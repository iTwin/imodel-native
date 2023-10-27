/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "QueryContracts.h"
#include "CustomFunctions.h"

Utf8CP PresentationQueryContract::RelatedInstanceInfoFieldName = "/RelatedInstanceInfo/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> PresentationQueryContract::GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields = _GetFields();
    std::stable_sort(fields.begin(), fields.end(), [](PresentationQueryContractFieldCPtr const& lhs, PresentationQueryContractFieldCPtr const& rhs)
        {
        return (!lhs->IsAggregateField() && rhs->IsAggregateField());
        });
    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        if (0 == strcmp(field.GetName(), fieldName))
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

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to find requested field in contract: '%s'", fieldName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> PresentationQueryContract::GetGroupingAliases() const
    {
    bvector<Utf8String> aliases;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryFieldType PresentationQueryContract::_GetFieldType(Utf8StringCR name) const
    {
    auto field = GetField(name.c_str());
    if (field.IsValid())
        return field->GetResultType();
    return PresentationQueryFieldType::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

        if (usedRelatedInstances.end() != usedRelatedInstances.find(relatedInstancePath.back().GetTargetClass().GetAlias()))
            continue;

        usedRelatedInstances.insert(relatedInstancePath.back().GetTargetClass().GetAlias());
        for (RelatedClassCR relatedInstanceClass : relatedInstancePath)
            {
            if (relatedInstanceClass.GetTargetClass().GetAlias().empty())
                continue;

            if (!isFirst)
                clause.append(",");
            isFirst = false;

            uint64_t fallbackClassId = relatedInstanceClass.GetTargetClass().GetClass().GetId().GetValue();
            clause.append("{");
            clause.append("\"Alias\":\"").append(relatedInstanceClass.GetTargetClass().GetAlias()).append("\",");
            clause.append("\"ECClassId\":' || CAST(IFNULL([").append(relatedInstanceClass.GetTargetClass().GetAlias()).append("].[ECClassId], ").append(std::to_string(fallbackClassId).c_str()).append(") AS TEXT) || ',");
            clause.append("\"ECInstanceId\":' || CAST(IFNULL([").append(relatedInstanceClass.GetTargetClass().GetAlias()).append("].[ECInstanceId], 0) AS TEXT) || '");
            clause.append("}");
            }
        }
    clause.append("]'");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr PresentationQueryContract::CreateRelatedInstanceInfoField(bvector<RelatedClassPath> const& relatedInstancePaths)
    {
    return PresentationQueryContractSimpleField::Create(RelatedInstanceInfoFieldName, CreateRelatedInstanceInfoClause(relatedInstancePaths), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryClauseAndBindings PresentationQueryContractSimpleField::_GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const&) const
    {
    if (!AllowsPrefix() || Utf8String::IsNullOrEmpty(prefix) || QueryHelpers::IsLiteral(m_selectClause.GetClause()) || QueryHelpers::IsFunction(m_selectClause.GetClause()))
        return QueryClauseAndBindings(QueryHelpers::Wrap(m_selectClause.GetClause()), m_selectClause.GetBindings());

    return QueryClauseAndBindings(QueryHelpers::Wrap(prefix).append(".").append(QueryHelpers::Wrap(m_selectClause.GetClause())), m_selectClause.GetBindings());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings CreateFieldClause(PresentationQueryContractField const& field, Utf8CP fieldPrefix, std::function<bool(Utf8CP)> const& useFieldName)
    {
    Utf8String clause;
    bool useFieldNameForParam = (useFieldName && useFieldName(field.GetName()));
    QueryClauseAndBindings paramClause = useFieldNameForParam ? QueryClauseAndBindings(field.GetName()) : field.GetSelectClause(fieldPrefix, useFieldName);
    if (!useFieldNameForParam && field.IsPresentationQueryContractFunctionField())
        clause.append(paramClause.GetClause());
    else if (QueryHelpers::IsLiteral(paramClause.GetClause()) || QueryHelpers::IsFunction(paramClause.GetClause()))
        clause.append(paramClause.GetClause());
    else if (!useFieldNameForParam || Utf8String::IsNullOrEmpty(fieldPrefix))
        clause.append("(").append(QueryHelpers::Wrap(paramClause.GetClause())).append(")");
    else
        clause.append(QueryHelpers::Wrap(fieldPrefix)).append(".").append(QueryHelpers::Wrap(paramClause.GetClause()));
    return QueryClauseAndBindings(clause, paramClause.GetBindings());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryClauseAndBindings PresentationQueryContractFunctionField::_GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const& useFieldName) const
    {
    bool first = true;
    Utf8String clause = m_functionName;
    BoundQueryValuesList bindings;
    clause.append("(");
    if (m_distinctArguments)
        clause.append("DISTINCT ");
    for (RefCountedPtr<PresentationQueryContractField const> const& parameter : m_parameters)
        {
        if (!first)
            clause.append(", ");

        auto paramClause = CreateFieldClause(*parameter, AllowsPrefix() ? prefix : nullptr, useFieldName);
        clause.append(paramClause.GetClause());
        ContainerHelpers::Push(bindings, paramClause.GetBindings());

        first = false;
        }
    clause.append(")");
    return QueryClauseAndBindings(clause, bindings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryClauseAndBindings PresentationQueryContractBinaryOpField::_GetSelectClause(Utf8CP prefix, std::function<bool(Utf8CP)> const& useFieldName) const
    {
    auto lhsClause = CreateFieldClause(*m_lhs, AllowsPrefix() ? prefix : nullptr, useFieldName);
    auto rhsClause = CreateFieldClause(*m_rhs, AllowsPrefix() ? prefix : nullptr, useFieldName);

    BoundQueryValuesList bindings;
    ContainerHelpers::Push(bindings, lhsClause.GetBindings());
    ContainerHelpers::Push(bindings, rhsClause.GetBindings());

    return QueryClauseAndBindings(
        Utf8String(lhsClause.GetClause()).append(" ").append(m_operator).append(" ").append(rhsClause.GetClause()),
        bindings
        );
    }

#ifdef wip_skipped_instance_keys_performance_issue
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RefCountedPtr<PresentationQueryContractField const>> CreateFieldsList(bvector<Utf8String> fieldNames, bool allowsPrefix)
    {
    bvector<RefCountedPtr<PresentationQueryContractField const>> fieldsList;
    for (Utf8String field : fieldNames)
        fieldsList.push_back(PresentationQueryContractSimpleField::Create(field.c_str(), field.c_str(), allowsPrefix));
    return fieldsList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsManyToManyRelationship(ECRelationshipClassCR rel)
    {
    return (rel.GetSource().GetMultiplicity().GetUpperLimit() > 1
        && rel.GetTarget().GetMultiplicity().GetUpperLimit() > 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HasNavigationProperty(RelatedClassCR related)
    {
    return nullptr != related.GetNavigationProperty();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleQueryContract::SimpleQueryContract(bvector<PresentationQueryContractFieldCPtr> fields)
    : m_fields(fields)
    {}

Utf8CP CountQueryContract::CountFieldName = "RowsCount";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CountQueryContract::CountQueryContract(Utf8CP groupingFieldName)
    {
    bvector<PresentationQueryContractFieldCPtr> params;

    bool wantDistinct = false;
    if (groupingFieldName && *groupingFieldName)
        {
        params.push_back(PresentationQueryContractSimpleField::Create("arg", groupingFieldName, false));
        wantDistinct = true;
        }
    else
        {
        params.push_back(PresentationQueryContractSimpleField::Create("arg", "1", false));
        }

    auto field = PresentationQueryContractFunctionField::Create(CountFieldName, "COUNT", params, false, true);
    field->SetDistinctArguments(wantDistinct);
    AddField(*field);
    }

Utf8CP InstanceKeysSelectContract::ECClassIdFieldName = "/ECClassId/";
Utf8CP InstanceKeysSelectContract::ECInstanceIdFieldName = "/ECInstanceId/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> InstanceKeysSelectContract::_GetFields() const
    {
    bvector<PresentationQueryContractFieldCPtr> fields;
    fields.push_back(PresentationQueryContractSimpleField::Create(ECClassIdFieldName, "ECClassId"));
    fields.push_back(PresentationQueryContractSimpleField::Create(ECInstanceIdFieldName, "ECInstanceId"));
    return fields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr QueryContractHelpers::CreatePointAsJsonStringSelectField(Utf8StringCR propertyName, Utf8StringCR prefix, int dimensions)
    {
    static const char axis[] = { 'x', 'y', 'z' };

    Utf8String prefixAndProperty;
    if (!prefix.empty())
        prefixAndProperty.append(QueryHelpers::Wrap(prefix)).append(".");
    prefixAndProperty.append(QueryHelpers::Wrap(propertyName));

    auto field = PresentationQueryContractFunctionField::Create("", FUNCTION_NAME_GetPointAsJsonString, {});
    for (int i = 0; i < dimensions; ++i)
        field->GetFunctionParameters().push_back(PresentationQueryContractSimpleField::Create("", Utf8PrintfString("%s.[%c]", prefixAndProperty.c_str(), axis[i]).c_str(), false));

    return field;
    }
