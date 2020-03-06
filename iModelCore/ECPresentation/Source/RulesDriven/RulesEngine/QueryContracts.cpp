/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/GroupingRule.h>
#include <algorithm>
#include "QueryContracts.h"
#include "CustomFunctions.h"
#include "NavigationQuery.h"
#include "ECExpressionContextsProvider.h"
#include "QueryBuilderHelpers.h"

Utf8CP PresentationQueryContract::RelatedInstanceInfoFieldName = "/RelatedInstanceInfo/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr PresentationQueryContract::GetField(Utf8CP name) const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (0 == strcmp(name, field->GetDefaultName()))
            return field;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationQueryContract::IsAggregating() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (field->IsAggregateField())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationQueryContract::HasInnerFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (FieldVisibility::Inner == field->GetVisibility())
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> PresentationQueryContract::GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    if (IsAggregating())
        {
        std::stable_sort(fields.begin(), fields.end(), [](PresentationQueryContractFieldCPtr const& lhs, PresentationQueryContractFieldCPtr const& rhs)
            {
            return (!lhs->IsAggregateField() && rhs->IsAggregateField());
            });
        }
    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t PresentationQueryContract::GetIndex(Utf8CP fieldName) const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = GetFields();
    size_t unusedFieldsCount = 0;
    size_t aggregateFieldsCount = 0;
    size_t nonAggregateFieldsCount = 0;
    int aggregateFieldIndex = -1;
    for (size_t i = 0; i < fields.size(); ++i)
        {
        PresentationQueryContractFieldCR field = *fields[i];
        if (FieldVisibility::Inner == field.GetVisibility())
            {
            ++unusedFieldsCount;
            continue;
            }
        if (field.IsAggregateField())
            aggregateFieldsCount++;
        else if (-1 != aggregateFieldIndex)
            nonAggregateFieldsCount++;
        if (field.GetDefaultName() == fieldName || 0 == strcmp(field.GetDefaultName(), fieldName))
            {
            if (field.IsAggregateField())
                aggregateFieldIndex = (int)(i - unusedFieldsCount);
            else
                return (uint8_t)(i - unusedFieldsCount - aggregateFieldsCount);
            }
        }

    if (-1 != aggregateFieldIndex)
        {
        // note: all aggregate fields are put at the end of SELECT statement -
        // we have to consider that when determining the field index
        return (uint8_t)(aggregateFieldIndex + nonAggregateFieldsCount);
        }

    BeAssert(false);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8CP> PresentationQueryContract::GetGroupingAliases() const
    {
    bvector<Utf8CP> aliases;
    if (!IsAggregating())
        return aliases;

    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (FieldVisibility::Inner != field->GetVisibility() && !field->IsAggregateField())
            aliases.push_back(field->GetName());
        }

    return aliases;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelatedInstanceInfoClause(bvector<RelatedClassPath> const& relatedInstancePaths)
    {
    bool isFirst = true;
    Utf8String clause = "'[";
    bset<Utf8String> usedRelatedInstances;
    for (RelatedClassPathCR relatedInstancePath : relatedInstancePaths)
        {
        if (relatedInstancePath.empty())
            continue;

        if (usedRelatedInstances.end() != usedRelatedInstances.find(relatedInstancePath.back().GetTargetClassAlias()))
            continue;

        usedRelatedInstances.insert(relatedInstancePath.back().GetTargetClassAlias());
        for (RelatedClassCR relatedInstanceClass : relatedInstancePath)
            {
            if (!relatedInstanceClass.GetTargetClassAlias() || !*relatedInstanceClass.GetTargetClassAlias())
                continue;

            if (!isFirst)
                clause.append(",");
            isFirst = false;

            uint64_t fallbackClassId = relatedInstanceClass.GetTargetClass().GetClass().GetId().GetValue();
            clause.append("{");
            clause.append("\"Alias\":\"").append(relatedInstanceClass.GetTargetClassAlias()).append("\",");
            clause.append("\"ECClassId\":' || CAST(IFNULL([").append(relatedInstanceClass.GetTargetClassAlias()).append("].[ECClassId], ").append(std::to_string(fallbackClassId).c_str()).append(") AS TEXT) || ',");
            clause.append("\"ECInstanceId\":' || CAST(IFNULL([").append(relatedInstanceClass.GetTargetClassAlias()).append("].[ECInstanceId], 0) AS TEXT) || '");
            clause.append("}");
            }
        }
    clause.append("]'");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static T PrepareDisplayLabelField(T field)
    {
    field->SetGroupingClause(QueryBuilderHelpers::CreateDisplayLabelValueClause(field->GetName()));
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr PresentationQueryContract::CreateRelatedInstanceInfoField(bvector<RelatedClassPath> const& relatedInstancePaths)
    {
    return PresentationQueryContractSimpleField::Create(RelatedInstanceInfoFieldName, CreateRelatedInstanceInfoClause(relatedInstancePaths).c_str(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr PresentationQueryContract::CreateDisplayLabelField(Utf8CP name, PresentationQueryContractFieldCR classIdField,
    PresentationQueryContractFieldCR instanceIdField, ECClassCP ecClass, bvector<RelatedClassPath> const& relatedInstancePaths,
    bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs)
    {
    ECPropertyCP labelProperty = nullptr != ecClass ? ecClass->GetInstanceLabelProperty() : nullptr;
    Utf8CP labelClause = nullptr != labelProperty ? labelProperty->GetName().c_str() : "''";
    RefCountedPtr<PresentationQueryContractSimpleField> defaultPropertyValueField = PresentationQueryContractSimpleField::Create(nullptr, labelClause);

    PresentationQueryContractFieldPtr labelField = PresentationQueryContractFunctionField::Create(name, FUNCTION_NAME_GetECInstanceDisplayLabel,
        { &classIdField, &instanceIdField, defaultPropertyValueField, CreateRelatedInstanceInfoField(relatedInstancePaths) });

    if (!labelOverrideValueSpecs.empty())
        labelField = QueryBuilderHelpers::CreateInstanceLabelField(name, labelOverrideValueSpecs, labelField.get(), &instanceIdField, &classIdField);

    return PrepareDisplayLabelField(labelField);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryContractSimpleField::_GetSelectClause(Utf8CP prefix, bool) const
    {
    if (!AllowsPrefix() || Utf8String::IsNullOrEmpty(prefix) || QueryHelpers::IsLiteral(m_selectClause) || QueryHelpers::IsFunction(m_selectClause))
        return QueryHelpers::Wrap(m_selectClause);
    return QueryHelpers::Wrap(prefix).append(".").append(QueryHelpers::Wrap(m_selectClause));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryContractFunctionField::_GetSelectClause(Utf8CP prefix, bool useFieldNames) const
    {
    bool first = true;
    Utf8String clause = m_functionName;
    clause.append("(");
    for (RefCountedPtr<PresentationQueryContractField const> const& parameter : m_parameters)
        {
        if (!first)
            clause.append(", ");

        Utf8String paramClause = (useFieldNames && !Utf8String::IsNullOrEmpty(parameter->GetName())) ? parameter->GetName() : parameter->GetSelectClause(prefix, useFieldNames);
        if (!useFieldNames && parameter->IsPresentationQueryContractFunctionField())
            clause.append(paramClause);
        else if (QueryHelpers::IsLiteral(paramClause) || QueryHelpers::IsFunction(paramClause))
            clause.append(paramClause);
        else if (!useFieldNames || !AllowsPrefix() || Utf8String::IsNullOrEmpty(prefix))
            clause.append(QueryHelpers::Wrap(paramClause));
        else
            clause.append(QueryHelpers::Wrap(prefix)).append(".").append(QueryHelpers::Wrap(paramClause));
        first = false;
        }
    clause.append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryContractDynamicField::_GetSelectClause(Utf8CP prefix, bool) const
    {
    return m_getSelectClauseHandler(AllowsPrefix() ? prefix : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryMergeField::_GetSelectClause(Utf8CP prefix, bool useFieldNames) const
    {
    Utf8String argument = useFieldNames ? Utf8PrintfString("[%s]", m_mergedField->GetName()) : m_mergedField->GetSelectClause(prefix, useFieldNames);
    Utf8String clause(FUNCTION_NAME_GetMergedValue);
    clause.append("(");
    clause.append("CAST(").append(argument).append(" AS TEXT)");
    if (!m_mergedValueResult.empty())
        clause.append(", '").append(m_mergedValueResult).append("'");
    clause.append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void AppendToVector(bvector<T>&) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename First, typename... Args> static void AppendToVector(bvector<T>& vec, First&& first, Args&&... args)
    {
    vec.push_back(std::forward<First>(first));
    AppendToVector(vec, std::forward<Args>(args)...);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename... Args> static bvector<Utf8String> CreateList(Args&&... args)
    {
    bvector<Utf8String> list;
    AppendToVector(list, std::forward<Args>(args)...);
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename... Args>
static bvector<RefCountedPtr<PresentationQueryContractField const>> CreateFieldsList(Args&&... args)
    {
    bvector<Utf8String> list;
    AppendToVector(list, std::forward<Args>(args)...);

    bvector<RefCountedPtr<PresentationQueryContractField const>> fieldsList;
    for (Utf8String fieldName : list)
        {
        bool allowPrefix = !QueryHelpers::IsFunction(fieldName) && !QueryHelpers::IsLiteral(fieldName);
        fieldsList.push_back(PresentationQueryContractSimpleField::Create(fieldName.c_str(), fieldName.c_str(), allowPrefix));
        }

    return fieldsList;
    }

#ifdef wip_skipped_instance_keys_performance_issue
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RefCountedPtr<PresentationQueryContractField const>> CreateFieldsList(bvector<Utf8String> fieldNames, bool allowsPrefix)
    {
    bvector<RefCountedPtr<PresentationQueryContractField const>> fieldsList;
    for (Utf8String field : fieldNames)
        fieldsList.push_back(PresentationQueryContractSimpleField::Create(field.c_str(), field.c_str(), allowsPrefix));
    return fieldsList;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPrefixedClause(Utf8String clause, Utf8CP prefix)
    {
    bool isFunctionClause = (Utf8String::npos != clause.find("("));
    if (!isFunctionClause)
        clause = QueryHelpers::Wrap(clause);
    if (nullptr == prefix || 0 == *prefix)
        return clause;
    return QueryHelpers::Wrap(prefix).append(".").append(clause);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsManyToManyRelationship(ECRelationshipClassCR rel)
    {
    return (rel.GetSource().GetMultiplicity().GetUpperLimit() > 1
        && rel.GetTarget().GetMultiplicity().GetUpperLimit() > 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasNavigationProperty(RelatedClassCR related)
    {
    return nullptr != related.GetNavigationProperty();
    }

Utf8CP NavigationQueryContract::SkippedInstanceKeysFieldName = "/SkippedInstanceKeys/";
Utf8CP NavigationQueryContract::SkippedInstanceKeysInternalFieldName = NavigationQueryContract::SkippedInstanceKeysFieldName;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> NavigationQueryContract::_GetFields() const
    {
    if (m_skippedInstanceKeysInternalField.IsNull())
        {
        bvector<Utf8String> args;
        if (m_pathFromSelectToParentClass.size() > 1)
            {
            for (size_t i = 0; i < m_pathFromSelectToParentClass.size() - 1; ++i)
                {
                RelatedClassCR related = m_pathFromSelectToParentClass[i];
                if (!related.GetRelationship())
                    continue;

                Utf8String classIdClause, instanceIdClause;
                if (IsManyToManyRelationship(*related.GetRelationship()) || !HasNavigationProperty(related))
                    {
                    BeAssert(nullptr != related.GetRelationshipAlias() && 0 != *related.GetRelationshipAlias());
                    Utf8String sourceOrTarget = related.IsForwardRelationship() ? "Target" : "Source";
                    classIdClause = QueryHelpers::Wrap(related.GetRelationshipAlias()).append(".").append(QueryHelpers::Wrap(Utf8String(sourceOrTarget).append("ECClassId")));
                    instanceIdClause = QueryHelpers::Wrap(related.GetRelationshipAlias()).append(".").append(QueryHelpers::Wrap(Utf8String(sourceOrTarget).append("ECInstanceId")));
                    }
                else
                    {
                    BeAssert(nullptr != related.GetTargetClassAlias() && 0 != *related.GetTargetClassAlias());
                    classIdClause = QueryHelpers::Wrap(related.GetTargetClassAlias()).append(".").append(QueryHelpers::Wrap("ECClassId"));
                    instanceIdClause = QueryHelpers::Wrap(related.GetTargetClassAlias()).append(".").append(QueryHelpers::Wrap("ECInstanceId"));
                    }
                args.push_back(classIdClause);
                args.push_back(instanceIdClause);
                }
            }
#ifdef wip_skipped_instance_keys_performance_issue
        m_skippedInstanceKeysInternalField = PresentationQueryContractFunctionField::Create(SkippedInstanceKeysInternalFieldName, FUNCTION_NAME_ECInstanceKeysArray,
            CreateFieldsList(args, false), false, false, FieldVisibility::Inner);
#else
        m_skippedInstanceKeysInternalField = PresentationQueryContractSimpleField::Create(SkippedInstanceKeysInternalFieldName, "'[]'", false, false, FieldVisibility::Inner);
#endif
        }
    if (m_skippedInstanceKeysField.IsNull())
        {
#ifdef wip_skipped_instance_keys_performance_issue
        BeAssert(m_skippedInstanceKeysInternalField.IsValid());
        m_skippedInstanceKeysField = PresentationQueryContractFunctionField::Create(SkippedInstanceKeysFieldName, FUNCTION_NAME_AggregateJsonArray,
            {m_skippedInstanceKeysInternalField}, false, true, FieldVisibility::Both);
#else
        m_skippedInstanceKeysField = PresentationQueryContractSimpleField::Create(SkippedInstanceKeysInternalFieldName, "'[]'", false, true, FieldVisibility::Both);
#endif
        }
    return {m_skippedInstanceKeysField, m_skippedInstanceKeysInternalField};
    }

Utf8CP ECInstanceNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP ECInstanceNodesQueryContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP ECInstanceNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceNodesQueryContract::ECInstanceNodesQueryContract(ECClassCP ecClass, bvector<RelatedClassPath> const& relatedInstancePaths, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId");
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId");
    m_relatedInstanceInfoField = CreateRelatedInstanceInfoField(relatedInstancePaths);
    m_displayLabelField = CreateDisplayLabelField(DisplayLabelFieldName, *m_ecClassIdField, *m_ecInstanceIdField, ecClass, relatedInstancePaths, labelOverrideValueSpecs);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ECInstanceNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields;
    fields.push_back(m_ecInstanceIdField);
    fields.push_back(m_ecClassIdField);
    fields.push_back(m_displayLabelField);
    fields.push_back(m_relatedInstanceInfoField);

    bvector<PresentationQueryContractFieldCPtr> baseFields = NavigationQueryContract::_GetFields();
    std::move(baseFields.begin(), baseFields.end(), std::back_inserter(fields));

    return fields;
    }

Utf8CP MultiECInstanceNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP MultiECInstanceNodesQueryContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP MultiECInstanceNodesQueryContract::InstanceKeysFieldName = "/InstanceKey/";
Utf8CP MultiECInstanceNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MultiECInstanceNodesQueryContract::MultiECInstanceNodesQueryContract(ECClassCP ecClass, bool aggregateInstanceKeys, bvector<RelatedClassPath> const& relatedInstancePaths, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, FieldVisibility::Inner);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, FieldVisibility::Inner);
    m_relatedInstanceInfoField = CreateRelatedInstanceInfoField(relatedInstancePaths);
    m_displayLabelField = CreateDisplayLabelField(DisplayLabelFieldName, *m_ecClassIdField, *m_ecInstanceIdField, ecClass, relatedInstancePaths, labelOverrideValueSpecs);
    if (aggregateInstanceKeys)
        {
        m_instanceKeysField = PresentationQueryContractFunctionField::Create(InstanceKeysFieldName, FUNCTION_NAME_GetGroupedInstanceKeys, {m_ecClassIdField, m_ecInstanceIdField}, false, true);
        m_includeIdFields = true;
        }
    else
        {
        m_instanceKeysField = PresentationQueryContractFunctionField::Create(InstanceKeysFieldName, FUNCTION_NAME_GetInstanceKey, {m_ecClassIdField, m_ecInstanceIdField});
        m_includeIdFields = false;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiECInstanceNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiECInstanceNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> MultiECInstanceNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields;
    if (m_includeIdFields)
        {
        fields.push_back(m_ecInstanceIdField);
        fields.push_back(m_ecClassIdField);
        }
    fields.push_back(m_displayLabelField);
    fields.push_back(m_relatedInstanceInfoField);
    fields.push_back(m_instanceKeysField);

    bvector<PresentationQueryContractFieldCPtr> baseFields = NavigationQueryContract::_GetFields();
    std::move(baseFields.begin(), baseFields.end(), std::back_inserter(fields));

    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ECRelationshipGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    return fields;
    }

Utf8CP ECClassGroupingNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP ECClassGroupingNodesQueryContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP ECClassGroupingNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
Utf8CP ECClassGroupingNodesQueryContract::GroupedInstanceKeysFieldName = "/GroupedInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodesQueryContract::ECClassGroupingNodesQueryContract()
    {
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId");
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, FieldVisibility::Inner);
    m_dummyClassIdField = PresentationQueryContractSimpleField::Create("dummy", "0", false, false, FieldVisibility::Outer);
    m_displayLabelField = PrepareDisplayLabelField(PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECClassDisplayLabel,
        {m_ecClassIdField, PresentationQueryContractSimpleField::Create(nullptr, "COUNT(1)")},
        true, true, FieldVisibility::Outer));
    m_groupedInstanceKeysField = PresentationQueryContractFunctionField::Create(GroupedInstanceKeysFieldName,
        FUNCTION_NAME_GetGroupedInstanceKeys, {m_ecClassIdField, m_ecInstanceIdField}, false, true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClassGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClassGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ECClassGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    fields.push_back(m_ecInstanceIdField);
    fields.push_back(m_ecClassIdField);
    fields.push_back(m_dummyClassIdField);
    fields.push_back(m_displayLabelField);
    fields.push_back(m_groupedInstanceKeysField);
    return fields;
    }

Utf8CP DisplayLabelGroupingNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP DisplayLabelGroupingNodesQueryContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
Utf8CP DisplayLabelGroupingNodesQueryContract::GroupedInstanceKeysFieldName = "/GroupedInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayLabelGroupingNodesQueryContract::DisplayLabelGroupingNodesQueryContract(ECClassCP ecClass, bvector<RelatedClassPath> const& relatedInstancePaths,
    bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs)
    {
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, FieldVisibility::Inner);
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, FieldVisibility::Inner);
    m_groupedInstanceKeysField = PresentationQueryContractFunctionField::Create(GroupedInstanceKeysFieldName,
      FUNCTION_NAME_GetGroupedInstanceKeys, {m_ecClassIdField, m_ecInstanceIdField}, false, true);
    m_displayLabelField = CreateDisplayLabelField(DisplayLabelFieldName, *m_ecClassIdField, *m_ecInstanceIdField, ecClass, relatedInstancePaths, labelOverrideValueSpecs);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayLabelGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayLabelGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> DisplayLabelGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    fields.push_back(m_ecInstanceIdField);
    fields.push_back(m_ecClassIdField);
    fields.push_back(m_displayLabelField);
    fields.push_back(m_groupedInstanceKeysField);
    return fields;
    }

Utf8CP BaseECClassGroupingNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP BaseECClassGroupingNodesQueryContract::ECClassIdFieldName  = "/ECClassId/";
Utf8CP BaseECClassGroupingNodesQueryContract::BaseClassIdFieldName  = "/BaseECClassId/";
Utf8CP BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
Utf8CP BaseECClassGroupingNodesQueryContract::GroupedInstanceKeysFieldName = "/GroupedInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BaseECClassGroupingNodesQueryContract::BaseECClassGroupingNodesQueryContract(ECClassId id)
    : m_baseClassId(id)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, FieldVisibility::Inner);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, FieldVisibility::Inner);
    m_baseClassIdField = PresentationQueryContractSimpleField::Create(BaseClassIdFieldName, id.ToString().c_str(), false);
    m_displayLabelField = PrepareDisplayLabelField(PresentationQueryContractFunctionField::Create(DisplayLabelFieldName,
        FUNCTION_NAME_GetECClassDisplayLabel, CreateFieldsList(id.ToString().c_str(), "COUNT(1)"), false, true, FieldVisibility::Outer));
    m_groupedInstanceKeysField = PresentationQueryContractFunctionField::Create(GroupedInstanceKeysFieldName,
        FUNCTION_NAME_GetGroupedInstanceKeys, {m_ecClassIdField, m_ecInstanceIdField}, false, true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseECClassGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseECClassGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> BaseECClassGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    fields.push_back(m_ecInstanceIdField);
    fields.push_back(m_ecClassIdField);
    fields.push_back(m_baseClassIdField);
    fields.push_back(m_displayLabelField);
    fields.push_back(m_groupedInstanceKeysField);
    return fields;
    }

Utf8CP ECPropertyGroupingNodesQueryContract::ECInstanceIdFieldName = "/ECInstanceId/";
Utf8CP ECPropertyGroupingNodesQueryContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP ECPropertyGroupingNodesQueryContract::ECPropertyClassIdFieldName = "/ECPropertyClassId/";
Utf8CP ECPropertyGroupingNodesQueryContract::ECPropertyNameFieldName = "/PropertyName/";
Utf8CP ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName = "/DisplayLabel/";
Utf8CP ECPropertyGroupingNodesQueryContract::ImageIdFieldName = "/ImageId/";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupingValueFieldName = "/GroupingValue/";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupingValuesFieldName = "/GroupingValues/";
Utf8CP ECPropertyGroupingNodesQueryContract::IsRangeFieldName = "/IsRange/";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupedInstanceKeysFieldName = "/GroupedInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodesQueryContract::ECPropertyGroupingNodesQueryContract(ECClassCR ecClass, ECPropertyCR prop, Utf8String groupingPropertyClassAlias, PropertyGroupCR spec, ECClassCP foreignKeyClass)
    : m_property(prop), m_specification(spec), m_foreignKeyClass(foreignKeyClass), m_groupingPropertyClassAlias(groupingPropertyClassAlias)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, FieldVisibility::Inner);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, FieldVisibility::Inner);
    m_ecPropertyClassIdField = PresentationQueryContractSimpleField::Create(ECPropertyClassIdFieldName,
        ecClass.GetId().ToString().c_str(), false);
    m_ecPropertyNameField = PresentationQueryContractSimpleField::Create(ECPropertyNameFieldName,
        Utf8PrintfString("'%s'", m_property.GetName().c_str()).c_str(), false);
    m_groupingValuesField = PresentationQueryContractDynamicField::Create(GroupingValuesFieldName,
        std::bind(&ECPropertyGroupingNodesQueryContract::GetGroupingValuesClause, this, std::placeholders::_1), true, true);
    m_imageIdField = PresentationQueryContractDynamicField::Create(ImageIdFieldName,
        std::bind(&ECPropertyGroupingNodesQueryContract::GetImageIdClause, this, std::placeholders::_1), true);
    m_isRangeField = PresentationQueryContractSimpleField::Create(IsRangeFieldName, spec.GetRanges().empty() ? "false" : "true", false);
    m_groupedInstanceKeysField = PresentationQueryContractFunctionField::Create(GroupedInstanceKeysFieldName, FUNCTION_NAME_GetGroupedInstanceKeys,
        {m_ecClassIdField, m_ecInstanceIdField}, false, true);
    bool groupByValue = (PropertyGroupingValue::PropertyValue == m_specification.GetPropertyGroupingValue());
    if (prop.GetIsPrimitive() && (PRIMITIVETYPE_Point3d == prop.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_Point2d == prop.GetAsPrimitiveProperty()->GetType()))
        {
        m_groupingValueField = PresentationQueryContractDynamicField::Create(GroupingValueFieldName,
            std::bind(&ECPropertyGroupingNodesQueryContract::GetPropertyValueClause, this, std::placeholders::_1),
            true, false, groupByValue ? FieldVisibility::Both : FieldVisibility::Inner);
        }
    else
        {
        m_groupingValueField = PresentationQueryContractSimpleField::Create(GroupingValueFieldName, GetGroupingValueClause(prop).c_str(),
            true, false, groupByValue ? FieldVisibility::Both : FieldVisibility::Inner);
        }
    m_displayLabelField = CreateDisplayLabelField();

    m_groupingValueField->SetPrefixOverride(m_groupingPropertyClassAlias);
    m_displayLabelField->SetPrefixOverride(m_groupingPropertyClassAlias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryContractField> ECPropertyGroupingNodesQueryContract::CreateDisplayLabelField() const
    {
    PresentationQueryContractFieldPtr defaultLabelField, groupedInstancesCountField;
    bool groupByValue = (PropertyGroupingValue::PropertyValue == m_specification.GetPropertyGroupingValue());
    if (groupByValue)
        {
        defaultLabelField = m_groupingValueField;
        groupedInstancesCountField = PresentationQueryContractSimpleField::Create(nullptr, "COUNT(1)", false);
        }
    else
        {
        defaultLabelField = PresentationQueryContractDynamicField::Create(GroupingValueFieldName,
            std::bind(&ECPropertyGroupingNodesQueryContract::GetPropertyValueClause, this, std::placeholders::_1),
            true, false, groupByValue ? FieldVisibility::Both : FieldVisibility::Inner);
        groupedInstancesCountField = PresentationQueryContractSimpleField::Create(nullptr, "0", false);
        }

    return PrepareDisplayLabelField(PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECPropertyDisplayLabel,
        {m_ecPropertyClassIdField, m_ecPropertyNameField, m_ecInstanceIdField, defaultLabelField, groupedInstancesCountField},
        true, groupByValue, groupByValue ? FieldVisibility::Outer : FieldVisibility::Both));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetGroupingValueClause(ECPropertyCR prop)
    {
    Utf8String name = prop.GetName();
    if (prop.GetIsNavigation())
        return QueryHelpers::Wrap(name).append(".[Id]");
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPointAsJsonStringClause(Utf8StringCR propertyName, Utf8CP prefix, int dimensions)
    {
    BeAssert(2 == dimensions || 3 == dimensions);
    static const char axis[] = {'x', 'y', 'z'};

    Utf8String prefixAndProperty;
    if (nullptr != prefix && 0 != *prefix)
        prefixAndProperty.append(QueryHelpers::Wrap(prefix)).append(".");
    prefixAndProperty.append(QueryHelpers::Wrap(propertyName));

    Utf8String clause;
    clause.append(FUNCTION_NAME_GetPointAsJsonString).append("(");
    for (int i = 0; i < dimensions; ++i)
        {
        if (i > 0)
            clause.append(", ");
        clause.append("QUOTE(").append(prefixAndProperty).append(".").append(1, axis[i]).append(")");
        }
    clause.append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetPropertyValueClause(Utf8CP prefix) const
    {
    if (nullptr != m_foreignKeyClass)
        {
        PresentationQueryContractFieldPtr field = PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetNavigationPropertyLabel,
            {m_ecClassIdField, m_ecInstanceIdField});
        return field->GetSelectClause("parentInstance");
        }
    if (m_property.GetIsPrimitive() && PRIMITIVETYPE_Point3d == m_property.GetAsPrimitiveProperty()->GetType())
        return GetPointAsJsonStringClause(m_property.GetName(), prefix, 3);
    if (m_property.GetIsPrimitive() && PRIMITIVETYPE_Point2d == m_property.GetAsPrimitiveProperty()->GetType())
        return GetPointAsJsonStringClause(m_property.GetName(), prefix, 2);
    return GetPrefixedClause(m_property.GetName(), prefix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetImageIdClause(Utf8CP prefix) const
    {
    if (m_specification.GetRanges().empty())
        {
        if (m_specification.GetImageId().empty())
            return "''";

        return Utf8PrintfString("'%s'", Utf8String(m_specification.GetImageId().c_str()).c_str());
        }

    // GetRangeImageId(propertyValue)
    return Utf8PrintfString("%s(%s)", FUNCTION_NAME_GetRangeImageId, GetPropertyValueClause(prefix).c_str());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetGroupingValuesClause(Utf8CP prefix) const
    {
    Utf8String clause("group_concat(");
    if (m_specification.GetRanges().empty())
        clause.append("DISTINCT ").append(QueryHelpers::Wrap(GroupingValueFieldName));
    else
        clause.append(FUNCTION_NAME_GetRangeIndex).append("(").append(QueryHelpers::Wrap(GroupingValueFieldName)).append(")");
    clause.append(")");
    return clause;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ECPropertyGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    fields.push_back(m_ecInstanceIdField);
    fields.push_back(m_ecPropertyNameField);
    fields.push_back(m_ecClassIdField);
    fields.push_back(m_ecPropertyClassIdField);
    fields.push_back(m_displayLabelField);
    fields.push_back(m_imageIdField);
    fields.push_back(m_isRangeField);
    fields.push_back(m_groupingValueField);
    fields.push_back(m_groupingValuesField);
    fields.push_back(m_groupedInstanceKeysField);
    return fields;
    }

Utf8CP ContentQueryContract::ContractIdFieldName = "/ContractId/";
Utf8CP ContentQueryContract::ECInstanceKeysFieldName = "/ECInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryContract::ContentQueryContract(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo, bvector<RelatedClassPath> const& relatedInstancePaths, bool skipCompositePropertyFields)
    : PresentationQueryContract(id), m_descriptor(&descriptor), m_class(ecClass), m_queryInfo(queryInfo), m_skipCompositePropertyFields(skipCompositePropertyFields), m_relatedInstancePaths(relatedInstancePaths)
    {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractField const& ContentQueryContract::GetDisplayLabelField(ContentDescriptor::DisplayLabelField const& field) const
    {
    if (m_displayLabelField.IsNull())
        {
        PresentationQueryContractFieldCPtr classIdField = PresentationQueryContractSimpleField::Create("ECClassId", "ECClassId");
        PresentationQueryContractFieldCPtr instanceIdField = PresentationQueryContractSimpleField::Create("ECInstanceId", "ECInstanceId");
        bvector<InstanceLabelOverrideValueSpecification const*> labelOverrideValuesList =
            m_class ? QueryBuilderHelpers::SerializeECClassMapPolymorphically(field.GetOverrideValueSpecs(), *m_class) : bvector<InstanceLabelOverrideValueSpecification const*>();
        m_displayLabelField = CreateDisplayLabelField(field.GetUniqueName().c_str(), *classIdField, *instanceIdField, m_class, m_relatedInstancePaths, labelOverrideValuesList);
        }
    return *m_displayLabelField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::GetCalculatedPropertyField(Utf8StringCR calculatedFieldName, Utf8StringCR calculatedPropertyValue, bool isGroupingField) const
    {
    Utf8String value = "'";
    value += calculatedPropertyValue;
    value += "'";
    return PresentationQueryContractFunctionField::Create(calculatedFieldName.c_str(), FUNCTION_NAME_EvaluateECExpression,
        CreateFieldsList("ECClassId", "ECInstanceId", value), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Property const* ContentQueryContract::FindMatchingProperty(ContentDescriptor::ECPropertiesField const& field, ECClassCP ecClass) const
    {
    if (nullptr == ecClass)
        ecClass = m_class;

    bvector<ContentDescriptor::Property const*> const& matchingProperties = field.FindMatchingProperties(ecClass);
    if (matchingProperties.empty())
        return nullptr;

    bvector<Utf8CP> selectClasses = m_queryInfo.GetSelectAliases();
    for (ContentDescriptor::Property const* prop : matchingProperties)
        {
        if (nullptr == prop->GetPrefix() || 0 == *prop->GetPrefix())
            return prop;

        for (Utf8CP selectClass : selectClasses)
            {
            if (0 == strcmp(prop->GetPrefix(), selectClass))
                return prop;
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPropertySelectClauseFromAccessString(Utf8StringCR accessString)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(accessString.c_str(), ".", tokens);
    Utf8String clause;
    for (Utf8StringCR token : tokens)
        {
        if (!clause.empty())
            clause.append(".");
        clause.append(QueryHelpers::Wrap(token));
        }
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String PropertyDisplayValueSelectStringClause(ECPropertyCR ecProperty, Utf8StringCR valueAccessString)
    {
    Utf8String clause;
    clause.append(FUNCTION_NAME_GetPropertyDisplayValue).append("('");
    clause.append(ecProperty.GetClass().GetSchema().GetName());
    clause.append("', '");
    clause.append(ecProperty.GetClass().GetName());
    clause.append("', '");
    clause.append(ecProperty.GetName());
    clause.append("', ");
    clause.append(valueAccessString).append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String PropertyValueSelectStringClause(Utf8CP prefix, Utf8CP propertyAccessString, PrimitiveType propertyType)
    {
    Utf8String valueClause;
    if (nullptr != prefix && 0 != *prefix)
        valueClause.append(QueryHelpers::Wrap(prefix)).append(".");
    valueClause.append(propertyAccessString);

    if (propertyType == PRIMITIVETYPE_Double)
        valueClause = Utf8String("QUOTE(").append(valueClause).append(")");
    else if (propertyType == PRIMITIVETYPE_Point2d || propertyType == PRIMITIVETYPE_Point3d)
        valueClause = GetPointAsJsonStringClause(propertyAccessString, prefix, propertyType == PRIMITIVETYPE_Point2d ? 2 : 3);

    return valueClause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr CreatePropertySelectField(Utf8CP fieldName, Utf8CP prefix, Utf8CP propertyAccessString,
    ECPropertyCR prop, bool isGroupingField)
    {
    if (prop.GetIsPrimitive())
        {
        Utf8String valueSelectClause = PropertyValueSelectStringClause(prefix, propertyAccessString, prop.GetAsPrimitiveProperty()->GetType());
        PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create(fieldName, valueSelectClause.c_str(), false);
        if (isGroupingField)
            {
            Utf8String displayValueSelectClause = PropertyDisplayValueSelectStringClause(prop, field->GetName());
            field->SetGroupingClause(displayValueSelectClause);
            }
        return field;
        }
    if (prop.GetIsNavigation())
        {
        RefCountedPtr<PresentationQueryContractFunctionField> navigationLabelfield = PresentationQueryContractFunctionField::Create(Utf8String(fieldName).append("_inner").c_str(),
            FUNCTION_NAME_GetNavigationPropertyLabel, CreateFieldsList("ECClassId", "ECInstanceId"), true);
        navigationLabelfield->SetPrefixOverride(prefix);

        return PrepareDisplayLabelField(PresentationQueryContractFunctionField::Create(fieldName, FUNCTION_NAME_GetLabelDefinitionDisplayValue, { navigationLabelfield }));
        }
    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create(fieldName, prop.GetName().c_str(), true);
    field->SetPrefixOverride(prefix);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr CreateNullPropertySelectField(Utf8CP fieldName, ECPropertyCR prop)
    {
    Utf8String selectClause;
    if (prop.GetIsPrimitive())
        {
        switch (prop.GetAsPrimitiveProperty()->GetType())
            {
            case PRIMITIVETYPE_Boolean: selectClause = "CAST(null AS BOOLEAN)"; break;
            case PRIMITIVETYPE_DateTime: selectClause = "CAST(null AS DATETIME)"; break;
            case PRIMITIVETYPE_Double: selectClause = "CAST(null AS DOUBLE)"; break;
            case PRIMITIVETYPE_Integer: selectClause = "CAST(null AS INT)"; break;
            case PRIMITIVETYPE_Long: selectClause = "CAST(null AS LONG)"; break;
            case PRIMITIVETYPE_String: selectClause = "CAST(null AS TEXT)"; break;
            case PRIMITIVETYPE_Point2d: selectClause = "CAST(null AS TEXT)"; break;
            case PRIMITIVETYPE_Point3d: selectClause = "CAST(null AS TEXT)"; break;
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                BeAssert(false);
                break;
            }
        }
    if (prop.GetIsNavigation())
        selectClause = "CAST(null AS TEXT)";

    if (selectClause.empty())
        selectClause = "null";

    return PresentationQueryContractSimpleField::Create(fieldName, selectClause.c_str(), false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::CreateInstanceKeyField(Utf8CP fieldName, Utf8CP alias, ECClassId defaultClassId, bool isMerging) const
    {
    if (!isMerging)
        {
        Utf8String clause;
        clause.append(FUNCTION_NAME_GetInstanceKey "(");
        clause.append("IFNULL(").append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECClassId")).append(",").append(defaultClassId.ToString()).append("),");
        clause.append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECInstanceId"));
        clause.append(")");
        return PresentationQueryContractSimpleField::Create(fieldName, clause.c_str(), false);
        }

    Utf8String clause;
    clause.append(FUNCTION_NAME_GetGroupedInstanceKeys "(");
    if (nullptr != alias)
        {
        clause.append("IFNULL(").append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECClassId")).append(",").append(defaultClassId.ToString()).append("),");
        clause.append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECInstanceId"));
        }
    else
        {
        clause.append(QueryHelpers::Wrap(fieldName));
        }
    clause.append(")");
    return PresentationQueryContractSimpleField::Create(nullptr, clause.c_str(), false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::CreateInstanceKeyField(ContentDescriptor::ECInstanceKeyField const& field, bool isMerging) const
    {
    if (isMerging)
        {
        bvector<Utf8CP> joinClassAliases = m_queryInfo.GetSelectAliases(IQueryInfoProvider::SELECTION_SOURCE_All);
        if (joinClassAliases.empty())
            {
            // merging queries wrap merged ones, so we don't have access to field's instance class alias - in this
            // case we have to select by field alias
            return CreateInstanceKeyField(field.GetUniqueName().c_str(), nullptr, ECClassId(), true);
            }
        }

    ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(*field.GetKeyFields().front(), m_class);
    if (nullptr != fieldPropertyForThisContract)
        {
        return CreateInstanceKeyField(field.GetUniqueName().c_str(), fieldPropertyForThisContract->GetPrefix(),
            fieldPropertyForThisContract->GetPropertyClass().GetId(), isMerging);
        }

    return PresentationQueryContractSimpleField::Create(field.GetUniqueName().c_str(), "CAST(null AS TEXT)", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr GetNavigationInstanceIdField(Utf8String fieldName, Utf8CP prefix)
    {
    if (nullptr == prefix)
        return PresentationQueryContractSimpleField::Create(fieldName.c_str(), "CAST(null AS LONG)", false);

    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create(fieldName.c_str(), "ECInstanceId");
    field->SetPrefixOverride(prefix);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ContentQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> contractFields;

    bvector<Utf8CP> selectAliases = m_queryInfo.GetSelectAliases(IQueryInfoProvider::SELECTION_SOURCE_From);
    if (!m_descriptor->OnlyDistinctValues())
        {
        if (0 != GetId())
            contractFields.push_back(PresentationQueryContractSimpleField::Create(ContractIdFieldName, std::to_string(GetId()).c_str(), false));
        else
            contractFields.push_back(PresentationQueryContractSimpleField::Create(nullptr, ContractIdFieldName, false));
        contractFields.push_back(CreateInstanceKeyField(ECInstanceKeysFieldName, selectAliases.empty() ? nullptr : selectAliases.front(), ECClassId(), m_descriptor->MergeResults()));
        }

    if (0 == ((int)ContentFlags::KeysOnly & m_descriptor->GetContentFlags()))
        {
        contractFields.push_back(CreateRelatedInstanceInfoField(m_relatedInstancePaths));

        for (ContentDescriptor::Field const* descriptorField : m_descriptor->GetAllFields())
            {
            bool createMergeField = m_descriptor->MergeResults();
            bool isDisplayLabelField = false;
            PresentationQueryContractFieldCPtr contractField;
            if (descriptorField->IsDisplayLabelField() && m_descriptor->ShowLabels())
                {
                contractField = &GetDisplayLabelField(*descriptorField->AsDisplayLabelField());
                isDisplayLabelField = true;
                }
            else if (descriptorField->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const& propertiesField = *descriptorField->AsPropertiesField();
                if (!m_skipCompositePropertyFields || !propertiesField.IsCompositePropertiesField())
                    {
                    ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(propertiesField, m_class);
                    if (nullptr != fieldPropertyForThisContract)
                        {
                        Utf8String propertyAccessor = GetPropertySelectClauseFromAccessString(fieldPropertyForThisContract->GetProperty().GetName());
                        ECPropertyCR ecProperty = fieldPropertyForThisContract->GetProperty();
                        contractField = CreatePropertySelectField(propertiesField.GetUniqueName().c_str(), fieldPropertyForThisContract->GetPrefix(),
                            propertyAccessor.c_str(), ecProperty, m_descriptor->OnlyDistinctValues());
                        }
                    else
                        {
                        ECPropertyCR ecProperty = propertiesField.GetProperties().front().GetProperty();
                        contractField = CreateNullPropertySelectField(propertiesField.GetUniqueName().c_str(), ecProperty);
                        }
                    }
                }
            else if (descriptorField->IsCalculatedPropertyField())
                {
                if (nullptr == descriptorField->AsCalculatedPropertyField()->GetClass() || m_class->Is(descriptorField->AsCalculatedPropertyField()->GetClass()))
                    contractField = GetCalculatedPropertyField(descriptorField->GetUniqueName(), descriptorField->AsCalculatedPropertyField()->GetValueExpression(), m_descriptor->OnlyDistinctValues());
                else
                    contractField = PresentationQueryContractSimpleField::Create(descriptorField->GetUniqueName().c_str(), "CAST(null AS TEXT)", false);
                }
            else if (!m_descriptor->OnlyDistinctValues() && descriptorField->IsSystemField() && descriptorField->AsSystemField()->IsECInstanceKeyField())
                {
                ContentDescriptor::ECInstanceKeyField const& keyField = *descriptorField->AsSystemField()->AsECInstanceKeyField();
                contractField = CreateInstanceKeyField(keyField, createMergeField);
                createMergeField = false;
                }
            else if (!m_descriptor->OnlyDistinctValues() && descriptorField->IsSystemField() && descriptorField->AsSystemField()->IsECNavigationInstanceIdField())
                {
                ContentDescriptor::ECNavigationInstanceIdField const& idField = *descriptorField->AsSystemField()->AsECNavigationInstanceIdField();
                ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(idField.GetPropertiesField(), m_class);
                if (nullptr != fieldPropertyForThisContract)
                    contractField = GetNavigationInstanceIdField(idField.GetUniqueName(), fieldPropertyForThisContract->GetPrefix());
                else
                    contractField = GetNavigationInstanceIdField(idField.GetUniqueName(), nullptr);
                }

            if (contractField.IsNull())
                continue;

            if (createMergeField)
                {
                Utf8String mergedValueResult;
                if (isDisplayLabelField)
                    {
                    Utf8String multipleInstancesString = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_MultipleInstances().m_str);
                    mergedValueResult = LabelDefinition::Create(multipleInstancesString.c_str())->ToJsonString();
                    }
                contractField = PresentationQueryMergeField::Create(nullptr, *contractField, mergedValueResult);
                }

            contractFields.push_back(contractField);
            }
        }
    return contractFields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleQueryContract::SimpleQueryContract(bvector<PresentationQueryContractFieldCPtr> fields)
    : m_fields(fields)
    {}

Utf8CP CountQueryContract::CountFieldName = "RowsCount";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CountQueryContract::CountQueryContract()
    {
    AddField(*PresentationQueryContractSimpleField::Create(CountFieldName, "COUNT(1)", false, true, FieldVisibility::Both));
    }
