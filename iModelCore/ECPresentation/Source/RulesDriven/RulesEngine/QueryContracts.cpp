/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryContracts.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/GroupingRule.h>
#include "QueryContracts.h"
#include "CustomFunctions.h"
#include "NavigationQuery.h"
#include "ECExpressionContextsProvider.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr PresentationQueryContract::GetField(Utf8CP name) const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = GetFields();
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
    bvector<PresentationQueryContractFieldCPtr> fields = GetFields();
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
    bvector<PresentationQueryContractFieldCPtr> fields = GetFields();
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
bvector<PresentationQueryContractFieldCPtr> PresentationQueryContract::GetFields() const {return _GetFields();}

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

    bvector<PresentationQueryContractFieldCPtr> fields = GetFields();
    for (PresentationQueryContractFieldCPtr const& field : fields)
        {
        if (FieldVisibility::Inner != field->GetVisibility() && !field->IsAggregateField())
            aliases.push_back(field->GetName());
        }

    return aliases;
    }

enum class IsFunctionTestStage
    {
    Name,
    Space,
    OpenArgs,
    Args,
    CloseArgs,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsFunction(Utf8StringCR clause)
    {
    IsFunctionTestStage stage = IsFunctionTestStage::Name;
    for (size_t i = 0; i < clause.size(); ++i)
        {
        switch (stage)
            {
            case IsFunctionTestStage::Name:
                {
                if ('0' <= clause[i] && clause[i] <= '9'
                    || 'A' <= clause[i] && clause[i] <= 'Z'
                    || 'a' <= clause[i] && clause[i] <= 'z'
                    || '_' == clause[i])
                    {
                    continue;
                    }
                if (' ' == clause[i])
                    {
                    stage = IsFunctionTestStage::Space;
                    break;
                    }
                if ('(' == clause[i])
                    {
                    stage = IsFunctionTestStage::OpenArgs;
                    break;
                    }
                return false;
                }
            case IsFunctionTestStage::Space:
                {
                if (' ' == clause[i])
                    continue;
                if ('(' == clause[i])
                    {
                    stage = IsFunctionTestStage::OpenArgs;
                    break;
                    }
                return false;
                }
            }
        if (IsFunctionTestStage::OpenArgs == stage)
            break;
        }

    if (IsFunctionTestStage::OpenArgs != stage || ')' != clause[clause.size() - 1])
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsLiteral(Utf8StringCR clause)
    {
    BeAssert(!clause.empty());
    return '\'' == clause[0] || clause[0] > '0' && clause[0] < '9';
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsWrapped(Utf8StringCR clause)
    {
    BeAssert(!clause.empty());
    return clause.StartsWith("[") && clause.EndsWith("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String Wrap(Utf8StringCR str)
    {
    if (IsFunction(str) || IsLiteral(str) || IsWrapped(str))
        return str;

    return Utf8String("[").append(str).append("]");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryContractSimpleField::_GetSelectClause(Utf8CP prefix, bool) const
    {
    if (!AllowsPrefix() || nullptr == prefix || 0 == *prefix)
        return m_selectClause;

    return Wrap(prefix).append(".").append(Wrap(m_selectClause));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PresentationQueryContractFunctionField::_GetSelectClause(Utf8CP prefix, bool) const
    {
    bool first = true;
    Utf8String clause = m_functionName;
    clause.append("(");
    for (Utf8String const& parameterName : m_functionParameters)
        {
        if (!first)
            clause.append(", ");

        if (!AllowsPrefix() || nullptr == prefix || 0 == *prefix || parameterName.StartsWith("'") || parameterName.EqualsI("NULL") || IsFunction(parameterName))
            clause.append(parameterName);
        else
            clause.append(Utf8PrintfString("%s.%s", Wrap(prefix).c_str(), Wrap(parameterName).c_str()));

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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2017
+===============+===============+===============+===============+===============+======*/
struct SkippedInstanceKeysAggregateField : PresentationQueryContractFunctionField
{
private:
    PresentationQueryContractFieldCPtr m_internalField;
private:
    SkippedInstanceKeysAggregateField(Utf8CP name, PresentationQueryContractFieldCR internalField) 
        : PresentationQueryContractFunctionField(name, FUNCTION_NAME_AggregateJsonArray, {internalField.GetName()}, false, false, true, FieldVisibility::Both), 
        m_internalField(&internalField)
        {}
protected:
    Utf8String _GetSelectClause(Utf8CP prefix, bool hasNestedQuery) const override
        {
        if (!hasNestedQuery)
            const_cast<SkippedInstanceKeysAggregateField*>(this)->GetFunctionParametersR()[0] = m_internalField->GetSelectClause(prefix, hasNestedQuery);
        Utf8String clause = PresentationQueryContractFunctionField::_GetSelectClause(prefix, hasNestedQuery);
        if (!hasNestedQuery)
            const_cast<SkippedInstanceKeysAggregateField*>(this)->GetFunctionParametersR()[0] = m_internalField->GetName();
        return clause;
        }
public:
    static RefCountedPtr<SkippedInstanceKeysAggregateField> Create(Utf8CP name, PresentationQueryContractFieldCR internalField)
        {
        return new SkippedInstanceKeysAggregateField(name, internalField);
        }
};

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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPrefixedClause(Utf8StringCR clause, Utf8CP prefix)
    {
    bool isFunctionClause = (Utf8String::npos != clause.find("("));
    Utf8CP clausePattern = isFunctionClause ? "%s" : "[%s]";

    if (nullptr == prefix || 0 == *prefix)
        return Utf8PrintfString(clausePattern, clause.c_str());

    Utf8String pattern("[%s].");
    pattern.append(clausePattern);
    return Utf8PrintfString(pattern.c_str(), prefix, clause.c_str());
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

Utf8CP NavigationQueryContract::SkippedInstanceKeysFieldName = "SkippedInstanceKeys";
Utf8CP NavigationQueryContract::SkippedInstanceKeysInternalFieldName = NavigationQueryContract::SkippedInstanceKeysFieldName;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> NavigationQueryContract::_GetFields() const
    {
    if (m_skippedInstanceKeysInternalField.IsNull())
        {
        bvector<Utf8String> args;
        if (m_relationshipPath.size() > 1)
            {
            for (size_t i = 0; i < m_relationshipPath.size() - 1; ++i)
                {
                RelatedClassCR related = m_relationshipPath[i];
                Utf8String classIdClause, instanceIdClause;
                if (IsManyToManyRelationship(*related.GetRelationship()) || !HasNavigationProperty(related))
                    {
                    BeAssert(nullptr != related.GetRelationshipAlias() && 0 != *related.GetRelationshipAlias());
                    Utf8CP sourceOrTarget = related.IsForwardRelationship() ? "Target" : "Source";
                    classIdClause = Utf8String("[").append(related.GetRelationshipAlias()).append("].[").append(sourceOrTarget).append("ECClassId]");
                    instanceIdClause = Utf8String("[").append(related.GetRelationshipAlias()).append("].[").append(sourceOrTarget).append("ECInstanceId]");
                    }
                else
                    {
                    BeAssert(nullptr != related.GetTargetClassAlias() && 0 != *related.GetTargetClassAlias());
                    classIdClause = Utf8String("[").append(related.GetTargetClassAlias()).append("].[ECClassId]");
                    instanceIdClause = Utf8String("[").append(related.GetTargetClassAlias()).append("].[ECInstanceId]");
                    }
                args.push_back(classIdClause);
                args.push_back(instanceIdClause);
                }
            }
        m_skippedInstanceKeysInternalField = PresentationQueryContractFunctionField::Create(SkippedInstanceKeysInternalFieldName, FUNCTION_NAME_ECInstanceKeysArray,
            args, false, false, false, FieldVisibility::Inner);
        }
    if (m_skippedInstanceKeysField.IsNull())
        {
        BeAssert(m_skippedInstanceKeysInternalField.IsValid());
        m_skippedInstanceKeysField = SkippedInstanceKeysAggregateField::Create(SkippedInstanceKeysFieldName, *m_skippedInstanceKeysInternalField);
        }
    return {m_skippedInstanceKeysField, m_skippedInstanceKeysInternalField};
    }

Utf8CP ECInstanceNodesQueryContract::ECInstanceIdFieldName = "ECInstanceId";
Utf8CP ECInstanceNodesQueryContract::ECClassIdFieldName = "ECClassId";
Utf8CP ECInstanceNodesQueryContract::DisplayLabelFieldName = "DisplayLabel";
Utf8CP ECInstanceNodesQueryContract::RelatedInstanceInfoFieldName = "RelatedInstanceInfo";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceNodesQueryContract::ECInstanceNodesQueryContract(ECClassCP ecClass, bvector<RelatedClass> const& relatedClasses)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false);
    m_displayLabelField = CreateDisplayLabelField(ecClass, relatedClasses);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId");
    m_relatedInstanceInfoField = CreateRelatedInstanceInfoField(relatedClasses);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    m_displayLabelField->GetFunctionParametersR()[1] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    m_displayLabelField->GetFunctionParametersR()[0] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateRelatedInstanceInfoClause(bvector<RelatedClass> const& relatedClasses)
    {
    bool isFirst = true;
    Utf8String clause = "'[";
    for (RelatedClass const& relatedClass : relatedClasses)
        {
        if (!isFirst)
            {
            clause.append(",");
            isFirst = false;
            }
        uint64_t fallbackClassId = relatedClass.GetTargetClass()->GetId().GetValue();
        clause.append("{");
        clause.append("\"Alias\":\"").append(relatedClass.GetTargetClassAlias()).append("\",");
        clause.append("\"ECClassId\":' || CAST(IFNULL([").append(relatedClass.GetTargetClassAlias()).append("].[ECClassId], ").append(std::to_string(fallbackClassId).c_str()).append(") AS TEXT) || ',");
        clause.append("\"ECInstanceId\":' || CAST(IFNULL([").append(relatedClass.GetTargetClassAlias()).append("].[ECInstanceId], 0) AS TEXT) || '");
        clause.append("}");
        }
    clause.append("]'");
    return clause;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryContractFunctionField> ECInstanceNodesQueryContract::CreateDisplayLabelField(ECClassCP ecClass, bvector<RelatedClass> const& relatedClasses)
    {
    ECPropertyCP labelProperty = nullptr != ecClass ? ecClass->GetInstanceLabelProperty() : nullptr;
    Utf8CP labelClause = nullptr != labelProperty ? labelProperty->GetName().c_str() : "''";
    return PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECInstanceDisplayLabel,
        CreateList("ECClassId", "ECInstanceId", labelClause, CreateRelatedInstanceInfoClause(relatedClasses)));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryContractSimpleField> ECInstanceNodesQueryContract::CreateRelatedInstanceInfoField(bvector<RelatedClass> const& relatedClasses)
    {
    return PresentationQueryContractSimpleField::Create(RelatedInstanceInfoFieldName, CreateRelatedInstanceInfoClause(relatedClasses).c_str(), false);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ECRelationshipGroupingNodesQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = NavigationQueryContract::_GetFields();
    return fields;
    }

Utf8CP ECClassGroupingNodesQueryContract::ECInstanceIdFieldName = "ECInstanceId";
Utf8CP ECClassGroupingNodesQueryContract::ECClassIdFieldName = "ECClassId";
Utf8CP ECClassGroupingNodesQueryContract::DisplayLabelFieldName = "DisplayLabel";
Utf8CP ECClassGroupingNodesQueryContract::GroupedInstanceIdsFieldName = "GroupedInstanceIds";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassGroupingNodesQueryContract::ECClassGroupingNodesQueryContract()
    {
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId");
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, false, FieldVisibility::Inner);
    m_dummyClassIdField = PresentationQueryContractSimpleField::Create("dummy", "0", false, false, false, FieldVisibility::Outer);
    m_displayLabelField = PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, 
        FUNCTION_NAME_GetECClassDisplayLabel, CreateList("ECClassId", "COUNT(1)"), true, false, true, FieldVisibility::Outer);
    m_groupedInstanceIdsField = PresentationQueryContractFunctionField::Create(GroupedInstanceIdsFieldName, 
        FUNCTION_NAME_GetGroupedInstanceKeys, CreateList(ECClassIdFieldName, ECInstanceIdFieldName), false, false, true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClassGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    m_displayLabelField->GetFunctionParametersR()[0] = name;
    m_groupedInstanceIdsField->GetFunctionParametersR()[0] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClassGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    m_groupedInstanceIdsField->GetFunctionParametersR()[1] = name;
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
    fields.push_back(m_groupedInstanceIdsField);
    return fields;
    }

Utf8CP DisplayLabelGroupingNodesQueryContract::ECInstanceIdFieldName = "ECInstanceId";
Utf8CP DisplayLabelGroupingNodesQueryContract::ECClassIdFieldName = "ECClassId";
Utf8CP DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName = "DisplayLabel";
Utf8CP DisplayLabelGroupingNodesQueryContract::GroupedInstanceIdsFieldName = "GroupedInstanceIds";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayLabelGroupingNodesQueryContract::DisplayLabelGroupingNodesQueryContract(ECClassCP ecClass, bool includeGroupedInstanceIdsField, bvector<RelatedClass> const& relatedClasses)
    {
    m_displayLabelField = CreateDisplayLabelField(ecClass, relatedClasses);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, false, FieldVisibility::Inner);
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, false, FieldVisibility::Inner);
    if (includeGroupedInstanceIdsField)
        {
        m_groupedInstanceIdsField = PresentationQueryContractFunctionField::Create(GroupedInstanceIdsFieldName,
            FUNCTION_NAME_GetGroupedInstanceKeys, CreateList(ECClassIdFieldName, ECInstanceIdFieldName), false, false, true);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayLabelGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    m_displayLabelField->GetFunctionParametersR()[1] = name;
    if (m_groupedInstanceIdsField.IsValid())
        m_groupedInstanceIdsField->GetFunctionParametersR()[1] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayLabelGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    m_displayLabelField->GetFunctionParametersR()[0] = name;
    if (m_groupedInstanceIdsField.IsValid())
        m_groupedInstanceIdsField->GetFunctionParametersR()[0] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryContractFunctionField> DisplayLabelGroupingNodesQueryContract::CreateDisplayLabelField(ECClassCP ecClass, bvector<RelatedClass> const& relatedClasses)
    {
    ECPropertyCP labelProperty = nullptr != ecClass ? ecClass->GetInstanceLabelProperty() : nullptr;
    Utf8CP labelClause = nullptr != labelProperty ? labelProperty->GetName().c_str() : "''";
    return PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECInstanceDisplayLabel,
        CreateList("ECClassId", "ECInstanceId", labelClause, CreateRelatedInstanceInfoClause(relatedClasses)));
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
    if (m_groupedInstanceIdsField.IsValid())
        fields.push_back(m_groupedInstanceIdsField);
    return fields;
    }

Utf8CP BaseECClassGroupingNodesQueryContract::ECInstanceIdFieldName = "ECInstanceId";
Utf8CP BaseECClassGroupingNodesQueryContract::ECClassIdFieldName  = "ECClassId";
Utf8CP BaseECClassGroupingNodesQueryContract::BaseClassIdFieldName  = "BaseECClassId";
Utf8CP BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName = "DisplayLabel";
Utf8CP BaseECClassGroupingNodesQueryContract::GroupedInstanceIdsFieldName = "GroupedInstanceIds";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BaseECClassGroupingNodesQueryContract::BaseECClassGroupingNodesQueryContract(ECClassId id)
    : m_baseClassId(id)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, false, FieldVisibility::Inner);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, false, FieldVisibility::Inner);
    m_baseClassIdField = PresentationQueryContractSimpleField::Create(BaseClassIdFieldName, Utf8PrintfString("%" PRIu64, id.GetValue()).c_str(), false);
    m_displayLabelField = PresentationQueryContractFunctionField::Create(DisplayLabelFieldName,
        FUNCTION_NAME_GetECClassDisplayLabel, CreateList(Utf8PrintfString("%" PRIu64, id.GetValue()).c_str(), "COUNT(1)"), false, false, true, FieldVisibility::Outer);
    m_groupedInstanceIdsField = PresentationQueryContractFunctionField::Create(GroupedInstanceIdsFieldName,
        FUNCTION_NAME_GetGroupedInstanceKeys, CreateList(ECClassIdFieldName, ECInstanceIdFieldName), false, false, true);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseECClassGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    m_groupedInstanceIdsField->GetFunctionParametersR()[1] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseECClassGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_ecClassIdField->SetClause(name);
    m_ecClassIdField->SetName(name);
    m_groupedInstanceIdsField->GetFunctionParametersR()[0] = name;
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
    fields.push_back(m_groupedInstanceIdsField);
    return fields;
    }

Utf8CP ECPropertyGroupingNodesQueryContract::ECInstanceIdFieldName = "ECInstanceId";
Utf8CP ECPropertyGroupingNodesQueryContract::ECClassIdFieldName = "ECClassId";
Utf8CP ECPropertyGroupingNodesQueryContract::ECPropertyClassIdFieldName = "ECPropertyClassId";
Utf8CP ECPropertyGroupingNodesQueryContract::ECPropertyNameFieldName = "PropertyName";
Utf8CP ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName = "DisplayLabel";
Utf8CP ECPropertyGroupingNodesQueryContract::ImageIdFieldName = "ImageId";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupingValueFieldName = "RulesEngine_GroupingValue";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupingValuesFieldName = "GroupingValues";
Utf8CP ECPropertyGroupingNodesQueryContract::IsRangeFieldName = "IsRange";
Utf8CP ECPropertyGroupingNodesQueryContract::GroupedInstanceIdsFieldName = "GroupedInstanceIds";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyGroupingNodesQueryContract::ECPropertyGroupingNodesQueryContract(ECClassCR ecClass, ECPropertyCR prop, Utf8String groupingPropertyClassAlias, PropertyGroupCR spec, ECClassCP foreignKeyClass) 
    : m_property(prop), m_specification(spec), m_foreignKeyClass(foreignKeyClass), m_ecInstanceFieldName("ECInstanceId"), m_groupingPropertyClassAlias(groupingPropertyClassAlias)
    {
    m_ecInstanceIdField = PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId", true, false, false, FieldVisibility::Inner);
    m_ecClassIdField = PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId", true, false, false, FieldVisibility::Inner);
    m_ecPropertyClassIdField = PresentationQueryContractSimpleField::Create(ECPropertyClassIdFieldName, 
        Utf8PrintfString("%" PRIu64, ecClass.GetId().GetValue()).c_str(), false);
    m_ecPropertyNameField = PresentationQueryContractSimpleField::Create(ECPropertyNameFieldName, 
        Utf8PrintfString("'%s'", m_property.GetName().c_str()).c_str(), false);
    m_groupingValuesField = PresentationQueryContractDynamicField::Create(GroupingValuesFieldName, 
        std::bind(&ECPropertyGroupingNodesQueryContract::GetGroupingValuesClause, this, std::placeholders::_1), true, false, true);
    m_imageIdField = PresentationQueryContractDynamicField::Create(ImageIdFieldName, 
        std::bind(&ECPropertyGroupingNodesQueryContract::GetImageIdClause, this, std::placeholders::_1), true);
    m_isRangeField = PresentationQueryContractSimpleField::Create(IsRangeFieldName, spec.GetRanges().empty() ? "false" : "true", false);    
    m_groupedInstanceIdsField = PresentationQueryContractFunctionField::Create(GroupedInstanceIdsFieldName, FUNCTION_NAME_GetGroupedInstanceKeys, 
        CreateList(ECClassIdFieldName, ECInstanceIdFieldName), false, false, true);

    bool groupByValue = (PropertyGroupingValue::PropertyValue == m_specification.GetPropertyGroupingValue());

    m_displayLabelField = PresentationQueryContractDynamicField::Create(DisplayLabelFieldName, 
        std::bind(&ECPropertyGroupingNodesQueryContract::GetDisplayLabelClause, this, std::placeholders::_1), true, false, 
        groupByValue, groupByValue ? FieldVisibility::Outer : FieldVisibility::Both);

    if (prop.GetIsPrimitive() && (PRIMITIVETYPE_Point3d == prop.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_Point2d == prop.GetAsPrimitiveProperty()->GetType()))
        {
        m_groupingValueField = PresentationQueryContractDynamicField::Create(GroupingValueFieldName,
            std::bind(&ECPropertyGroupingNodesQueryContract::GetPropertyValueClause, this, std::placeholders::_1), true, false, false, groupByValue ? FieldVisibility::Both : FieldVisibility::Inner);
        }
    else
        {
        m_groupingValueField = PresentationQueryContractSimpleField::Create(GroupingValueFieldName, GetGroupingValueClause(prop).c_str(),
            true, false, false, groupByValue ? FieldVisibility::Both : FieldVisibility::Inner);
        }

    m_groupingValueField->SetPrefixOverride(m_groupingPropertyClassAlias);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetGroupingValueClause(ECPropertyCR prop)
    {
    Utf8String name = prop.GetName();
    if (prop.GetIsNavigation())
        return Wrap(name).append(".[Id]");
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPointAsJsonStringClause(Utf8StringCR propertyName, Utf8CP prefix)
    {
    Utf8String clause;
    clause.append(FUNCTION_NAME_GetPointAsJsonString).append("(");
    if (nullptr != prefix && 0 != *prefix)
        clause.append(Wrap(prefix)).append(".");

    clause.append(Wrap(propertyName)).append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetPropertyValueClause(Utf8CP prefix)
    {
    if (nullptr != m_foreignKeyClass)
        {
        ECPropertyCP foreignKeyClassLabelProperty = m_foreignKeyClass->GetInstanceLabelProperty();
        Utf8CP labelClause = nullptr != foreignKeyClassLabelProperty ? foreignKeyClassLabelProperty->GetName().c_str() : "''";
        PresentationQueryContractFieldPtr field = PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECInstanceDisplayLabel,
            CreateList("ECClassId", "ECInstanceId", labelClause, "NULL"));
        return field->GetSelectClause("parentInstance");
        }
    if (m_property.GetIsPrimitive() && (PRIMITIVETYPE_Point3d == m_property.GetAsPrimitiveProperty()->GetType() || PRIMITIVETYPE_Point2d == m_property.GetAsPrimitiveProperty()->GetType()))
        return GetPointAsJsonStringClause(m_property.GetName(), prefix);
 
    return GetPrefixedClause(m_property.GetName(), prefix);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetDisplayLabelClause(Utf8CP prefix)
    {
    if (nullptr != prefix && !m_groupingPropertyClassAlias.empty())
        prefix = m_groupingPropertyClassAlias.c_str();

    bool groupByValue = (PropertyGroupingValue::PropertyValue == m_specification.GetPropertyGroupingValue());

    // GetECPropertyDisplayLabel(ECClassId, 'PropertyName', ECInstanceId, PropertyValue, GroupedInstancesCount)
    return Utf8PrintfString("%s(%" PRIu64 ", '%s', %s, %s, %s)", FUNCTION_NAME_GetECPropertyDisplayLabel, m_property.GetClass().GetId().GetValue(), 
        m_property.GetName().c_str(), GetPrefixedClause(m_ecInstanceFieldName.c_str(), prefix).c_str(), 
        groupByValue ? Wrap(GroupingValueFieldName).c_str() : GetPropertyValueClause(prefix).c_str(), groupByValue ? "COUNT(1)" : "0");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECPropertyGroupingNodesQueryContract::GetImageIdClause(Utf8CP prefix)
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
Utf8String ECPropertyGroupingNodesQueryContract::GetGroupingValuesClause(Utf8CP prefix)
    {
    if (m_specification.GetRanges().empty())
        return Utf8PrintfString("group_concat(DISTINCT %s)", GroupingValueFieldName);
        
    // GetRangeIndex(propertyValue)
    return Utf8PrintfString("group_concat(%s(%s))", FUNCTION_NAME_GetRangeIndex, GroupingValueFieldName);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyGroupingNodesQueryContract::_SetECInstanceIdFieldName(Utf8CP name)
    {
    m_ecInstanceFieldName = name;
    m_ecInstanceIdField->SetClause(name);
    m_ecInstanceIdField->SetName(name);
    m_groupedInstanceIdsField->GetFunctionParametersR()[1] = name;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyGroupingNodesQueryContract::_SetECClassIdFieldName(Utf8CP name)
    {
    m_groupedInstanceIdsField->GetFunctionParametersR()[0] = name;
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
    fields.push_back(m_groupedInstanceIdsField);
    return fields;
    }

Utf8CP ContentQueryContract::ECInstanceKeysFieldName = "ECInstanceKeys";
Utf8CP ContentQueryContract::DisplayLabelFieldName = "DisplayLabel";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryContract::ContentQueryContract(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo)
    : PresentationQueryContract(id), m_descriptor(&descriptor), m_class(ecClass), m_queryInfo(queryInfo)
    {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFunctionField const& ContentQueryContract::GetDisplayLabelField() const
    {
    if (m_displayLabelField.IsNull())
        {
        ECPropertyCP labelProperty = nullptr;
        if (nullptr != m_class)
            labelProperty = m_class->GetInstanceLabelProperty();

        Utf8CP labelClause = nullptr != labelProperty ? labelProperty->GetName().c_str() : "''";
        m_displayLabelField = PresentationQueryContractFunctionField::Create(DisplayLabelFieldName, FUNCTION_NAME_GetECInstanceDisplayLabel,
            CreateList("ECClassId", "ECInstanceId", labelClause, "NULL"), true, m_descriptor->OnlyDistinctValues());
        }

    return *m_displayLabelField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::GetCalculatedPropertyField(Utf8StringCR calculatedFieldName, Utf8StringCR calculatedPropertyValue, bool isDistinct) const
    {
    Utf8String value = "'";
    value += calculatedPropertyValue;
    value += "'";
    return PresentationQueryContractFunctionField::Create(calculatedFieldName.c_str(), FUNCTION_NAME_EvaluateECExpression,
        CreateList("ECClassId", "ECInstanceId", value), true, isDistinct);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Property const* ContentQueryContract::FindMatchingProperty(ContentDescriptor::ECPropertiesField const& field, ECClassCP ecClass) const
    {    
    if (nullptr == ecClass)
        ecClass = m_class;

    bvector<ContentDescriptor::Property const*> matchingProperties = field.FindMatchingProperties(ecClass);
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
        clause.append("[").append(token).append("]");
        }
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr CreatePropertySelectField(Utf8CP fieldName, Utf8CP prefix, Utf8CP propertyAccessString, ECPropertyCR prop, bool isDistinct)
    {
    if (prop.GetIsPrimitive())
        {
        switch (prop.GetAsPrimitiveProperty()->GetType())
            {
            case PRIMITIVETYPE_Point2d:
            case PRIMITIVETYPE_Point3d:
                {
                Utf8String clause = GetPointAsJsonStringClause(propertyAccessString, prefix);
                return PresentationQueryContractSimpleField::Create(fieldName, clause.c_str(), false);
                }
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_String:
                {
                /*if (nullptr != primitiveProperty.GetEnumeration())
                    {
                    Utf8String enumSchema = Utf8String("'").append(primitiveProperty.GetEnumeration()->GetSchema().GetName()).append("'");
                    Utf8String enumClass = Utf8String("'").append(primitiveProperty.GetEnumeration()->GetName()).append("'");
                    field = PresentationQueryContractFunctionField::Create(propertiesField.GetName().c_str(), FUNCTION_NAME_GetECEnumerationValue,
                        {enumSchema, enumClass, propertyAccessor}, false);
                    break;
                    }*/
                }
            }
        }
    if (prop.GetIsNavigation())
        {
        PresentationQueryContractFieldPtr field = PresentationQueryContractFunctionField::Create(fieldName, FUNCTION_NAME_GetECInstanceDisplayLabel, 
            CreateList("ECClassId", "ECInstanceId", "NULL", "NULL"));
        field->SetPrefixOverride(prefix);
        return field;
        }
    PresentationQueryContractFieldPtr field = PresentationQueryContractSimpleField::Create(fieldName, prop.GetName().c_str(), true, isDistinct);
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
        clause.append("IFNULL(").append(Wrap(alias)).append(".").append("[ECClassId],").append(defaultClassId.ToString()).append("),");
        clause.append(Wrap(alias)).append(".").append("[ECInstanceId])");
        return PresentationQueryContractSimpleField::Create(fieldName, clause.c_str(), false);
        }

    Utf8String clause;
    clause.append(FUNCTION_NAME_GetGroupedInstanceKeys "(");
    if (nullptr != alias)
        {
        clause.append("IFNULL(").append(Wrap(alias)).append(".").append("[ECClassId],").append(defaultClassId.ToString()).append("),");
        clause.append(Wrap(alias)).append(".").append("[ECInstanceId]");
        }
    else
        {
        clause.append(Wrap(fieldName));
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
            return CreateInstanceKeyField(field.GetName().c_str(), nullptr, ECClassId(), true);
            }
        }

    ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(*field.GetKeyFields().front(), m_class);
    if (nullptr != fieldPropertyForThisContract)
        {
        return CreateInstanceKeyField(field.GetName().c_str(), fieldPropertyForThisContract->GetPrefix(), 
            fieldPropertyForThisContract->GetPropertyClass().GetId(), isMerging);
        }

    return PresentationQueryContractSimpleField::Create(field.GetName().c_str(), "CAST(null AS TEXT)", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr GetNavigationInstanceIdField(Utf8String fieldName, Utf8CP prefix)
    {
    fieldName.append("_Id");
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
        contractFields.push_back(PresentationQueryContractSimpleField::Create("ContractId", std::to_string(GetId()).c_str(), false));
        contractFields.push_back(CreateInstanceKeyField(ECInstanceKeysFieldName, selectAliases.empty() ? nullptr : selectAliases.front(), ECClassId(), m_descriptor->MergeResults()));
        }

    if (0 == ((int)ContentFlags::KeysOnly & m_descriptor->GetContentFlags()))
        {
        for (ContentDescriptor::Field const* descriptorField : m_descriptor->GetAllFields())
            {
            bool createMergeField = m_descriptor->MergeResults();
            bool isDisplayLabelField = false;
            PresentationQueryContractFieldCPtr contractField;
            if (descriptorField->IsDisplayLabelField())
                {
                BeAssert(m_descriptor->ShowLabels());
                contractField = &GetDisplayLabelField();
                isDisplayLabelField = true;
                }
            else if (descriptorField->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const& propertiesField = *descriptorField->AsPropertiesField();
                ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(propertiesField, m_class);
                if (nullptr != fieldPropertyForThisContract)
                    {
                    Utf8String propertyAccessor = GetPropertySelectClauseFromAccessString(fieldPropertyForThisContract->GetProperty().GetName());
                    ECPropertyCR ecProperty = fieldPropertyForThisContract->GetProperty();
                    contractField = CreatePropertySelectField(propertiesField.GetName().c_str(), 
                        fieldPropertyForThisContract->GetPrefix(), propertyAccessor.c_str(), ecProperty, m_descriptor->OnlyDistinctValues());
                    }
                else
                    {
                    ECPropertyCR ecProperty = propertiesField.GetProperties().front().GetProperty();
                    contractField = CreateNullPropertySelectField(propertiesField.GetName().c_str(), ecProperty);
                    }
                }
            else if (descriptorField->IsCalculatedPropertyField())
                {
                if (nullptr == descriptorField->AsCalculatedPropertyField()->GetClass() || m_class->Is(descriptorField->AsCalculatedPropertyField()->GetClass()))
                    contractField = GetCalculatedPropertyField(descriptorField->GetName(), descriptorField->AsCalculatedPropertyField()->GetValueExpression(), m_descriptor->OnlyDistinctValues());
                else
                    contractField = PresentationQueryContractSimpleField::Create(descriptorField->GetName().c_str(), "CAST(null AS TEXT)", false);
                }
            else if (descriptorField->IsSystemField() && descriptorField->AsSystemField()->IsECInstanceKeyField())
                {
                ContentDescriptor::ECInstanceKeyField const& keyField = *descriptorField->AsSystemField()->AsECInstanceKeyField();
                contractField = CreateInstanceKeyField(keyField, createMergeField);
                createMergeField = false;
                }
            else if (descriptorField->IsSystemField() && descriptorField->AsSystemField()->IsECNavigationInstanceIdField())
                {
                ContentDescriptor::ECNavigationInstanceIdField const& idField = *descriptorField->AsSystemField()->AsECNavigationInstanceIdField();
                ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(idField.GetPropertiesField(), m_class);
                if (nullptr != fieldPropertyForThisContract)
                    contractField = GetNavigationInstanceIdField(idField.GetPropertiesField().GetName(), fieldPropertyForThisContract->GetPrefix());
                else
                    contractField = GetNavigationInstanceIdField(idField.GetPropertiesField().GetName(), nullptr);
                }

            if (contractField.IsNull())
                continue;

            if (createMergeField)
                {
                Utf8String mergedValueResult;
                if (isDisplayLabelField)
                    mergedValueResult = PRESENTATION_LOCALIZEDSTRING(RulesEngineL10N::GetNameSpace().m_namespace, RulesEngineL10N::LABEL_General_MultipleInstances().m_str);
                contractField = PresentationQueryMergeField::Create(nullptr, *contractField, mergedValueResult);
                }

            contractFields.push_back(contractField);
            }
        }
    return contractFields;
    }

Utf8CP CountQueryContract::CountFieldName = "RowsCount";
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CountQueryContract::CountQueryContract()
    {
    m_countField = PresentationQueryContractSimpleField::Create(CountFieldName, "COUNT(1)", false, false, true, FieldVisibility::Both);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> CountQueryContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields;
    fields.push_back(m_countField);
    return fields;
    }
