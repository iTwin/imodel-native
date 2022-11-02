/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ContentQueryContracts.h"
#include "../Shared/Queries/CustomFunctions.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void AppendToVector(bvector<T>&) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename First, typename... Args> static void AppendToVector(bvector<T>& vec, First&& first, Args&&... args)
    {
    vec.push_back(std::forward<First>(first));
    AppendToVector(vec, std::forward<Args>(args)...);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename... Args> static bvector<Utf8String> CreateList(Args&&... args)
    {
    bvector<Utf8String> list;
    AppendToVector(list, std::forward<Args>(args)...);
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

Utf8CP ContentQueryContract::ContractIdFieldName = "/ContractId/";
Utf8CP ContentQueryContract::ECInstanceKeysFieldName = "/ECInstanceKeys/";
Utf8CP ContentQueryContract::InputECInstanceKeysFieldName = "/InputECInstanceKeys/";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryContract::ContentQueryContract(uint64_t id, ContentDescriptorCR descriptor, ECClassCP ecClass, IQueryInfoProvider const& queryInfo,
    PresentationQueryContractFieldPtr displayLabelField, bvector<RelatedClassPath> relatedInstancePaths, bool skipCompositePropertyFields, bool skipXToManyRelatedContentFields)
    : PresentationQueryContract(id), m_descriptor(&descriptor), m_class(ecClass), m_relationshipClass(nullptr), m_queryInfo(queryInfo), m_displayLabelField(displayLabelField),
    m_relatedInstancePaths(std::move(relatedInstancePaths)), m_skipCompositePropertyFields(skipCompositePropertyFields), m_skipXToManyRelatedContentFields(skipXToManyRelatedContentFields)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::GetCalculatedPropertyField(Utf8StringCR calculatedFieldName, Utf8StringCR calculatedPropertyValue) const
    {
    Utf8String value = "'";
    value += calculatedPropertyValue;
    value += "'";
    return PresentationQueryContractFunctionField::Create(calculatedFieldName.c_str(), FUNCTION_NAME_EvaluateECExpression,
        CreateFieldsList("ECClassId", "ECInstanceId", value), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentDescriptor::Property const* ContentQueryContract::FindMatchingProperty(ContentDescriptor::ECPropertiesField const& field, ECClassCP ecClass) const
    {
    if (nullptr == ecClass)
        ecClass = m_relationshipClass ? m_relationshipClass : m_class;

    bvector<ContentDescriptor::Property const*> const& matchingProperties = field.FindMatchingProperties(ecClass);
    if (matchingProperties.empty())
        return nullptr;

    bvector<Utf8CP> selectClasses = ecClass && ecClass->IsRelationshipClass() ? m_queryInfo.GetRelationshipAliases() : m_queryInfo.GetSelectAliases();
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
* @bsimethod
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

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static QueryClauseAndBindings PropertyValueSelectStringClause(Utf8CP prefix, Utf8CP propertyAccessString, PrimitiveType propertyType)
    {
    if (propertyType == PRIMITIVETYPE_Point2d || propertyType == PRIMITIVETYPE_Point3d)
        return QueryContractHelpers::CreatePointAsJsonStringSelectField(propertyAccessString, prefix, (propertyType == PRIMITIVETYPE_Point2d) ? 2 : 3)->GetSelectClause();

    Utf8String valueClause;
    if (nullptr != prefix && 0 != *prefix)
        valueClause.append(QueryHelpers::Wrap(prefix)).append(".");
    valueClause.append(propertyAccessString);
    return valueClause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr CreatePropertySelectField(Utf8CP fieldName, Utf8CP prefix, Utf8CP propertyAccessString,
    ECPropertyCR prop, bool isGroupingField)
    {
    PresentationQueryContractFieldPtr field;
    if (prop.GetIsPrimitive())
        {
        QueryClauseAndBindings valueSelectClause = PropertyValueSelectStringClause(prefix, propertyAccessString, prop.GetAsPrimitiveProperty()->GetType());
        field = PresentationQueryContractSimpleField::Create(fieldName, valueSelectClause, false);
        if (nullptr != prop.GetAsPrimitiveProperty()->GetEnumeration())
            field->SetResultType(PresentationQueryFieldType::Enum);
        else
            field->SetResultType(PresentationQueryFieldType::Primitive);
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        if (isGroupingField)
            {
            Utf8String displayValueSelectClause = PropertyDisplayValueSelectStringClause(prop, field->GetName());
            field->SetGroupingClause(displayValueSelectClause);
            }
#endif
        }
    else if (prop.GetIsNavigation())
        {
        field = PresentationQueryContractFunctionField::Create(fieldName, FUNCTION_NAME_GetNavigationPropertyValue, CreateFieldsList("ECClassId", "ECInstanceId"), true);
        field->SetResultType(PresentationQueryFieldType::NavigationPropertyValue);
        }
    else
        {
        field = PresentationQueryContractSimpleField::Create(fieldName, prop.GetName().c_str(), true);
        field->SetResultType(PresentationQueryFieldType::Primitive);
        }
    field->SetPrefixOverride(prefix);
    return field;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
                break;
            }
        }
    if (prop.GetIsNavigation())
        selectClause = "CAST(null AS TEXT)";

    if (selectClause.empty())
        selectClause = "CAST(null AS TEXT)";

    return PresentationQueryContractSimpleField::Create(fieldName, selectClause.c_str(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationQueryContractFieldCPtr CreateInstanceKeyField(Utf8CP fieldName, Utf8CP alias, ECClassId defaultClassId)
    {
    if (nullptr != alias)
        {
        Utf8String clause;
        clause.append(FUNCTION_NAME_GetInstanceKey "(");
        clause.append("IFNULL(").append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECClassId")).append(",").append(defaultClassId.ToString()).append("),");
        clause.append(QueryHelpers::Wrap(alias)).append(".").append(QueryHelpers::Wrap("ECInstanceId"));
        clause.append(")");
        return PresentationQueryContractSimpleField::Create(fieldName, clause.c_str(), false);
        }
    return PresentationQueryContractSimpleField::Create(fieldName, "CAST(null AS TEXT)", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldCPtr ContentQueryContract::CreateInputKeysField(Utf8CP selectAlias) const
    {
    if (!m_descriptor->HasContentFlag(ContentFlags::IncludeInputKeys))
        return CreateInstanceKeyField(InputECInstanceKeysFieldName, nullptr, ECClassId());

    if (m_inputInstanceKey.IsValid())
        {
        return PresentationQueryContractFunctionField::Create(InputECInstanceKeysFieldName, FUNCTION_NAME_GetInstanceKey,
            {
            PresentationQueryContractSimpleField::Create(nullptr, m_inputInstanceKey.GetClassId().ToString(), false),
            PresentationQueryContractSimpleField::Create(nullptr, m_inputInstanceKey.GetInstanceId().ToString(), false),
            }, false);
        }

    Utf8CP inputKeySelectAlias = m_inputClassAlias.empty() ? selectAlias : m_inputClassAlias.c_str();
    return CreateInstanceKeyField(InputECInstanceKeysFieldName, inputKeySelectAlias, ECClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentQueryContract::ShouldHandleRelatedContentField(ContentDescriptor::RelatedContentField const& field) const
    {
    if (!field.IsXToMany())
        return true;
    return !m_skipXToManyRelatedContentFields && (field.GetPathFromSelectToContentClass().GetTargetsCount().IsNull() || field.GetPathFromSelectToContentClass().GetTargetsCount().Value() <= 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentQueryContract::CreateContractFields(bvector<PresentationQueryContractFieldCPtr>& contractFields, bvector<ContentDescriptor::Field*> const& fields, ContentDescriptor::RelatedContentField const* parentField) const
    {
    bool didCreateNonNullField = false;
    for (ContentDescriptor::Field const* descriptorField : fields)
        {
        PresentationQueryContractFieldCPtr contractField;
        if (descriptorField->IsCalculatedPropertyField())
            {
            if (nullptr == descriptorField->AsCalculatedPropertyField()->GetClass() || m_class->Is(descriptorField->AsCalculatedPropertyField()->GetClass()))
                {
                contractField = GetCalculatedPropertyField(descriptorField->GetUniqueName(), descriptorField->AsCalculatedPropertyField()->GetValueExpression());
                didCreateNonNullField = true;
                }
            else
                {
                contractField = PresentationQueryContractSimpleField::Create(descriptorField->GetUniqueName().c_str(), "CAST(null AS TEXT)", false);
                }
            }
        else if (descriptorField->IsPropertiesField())
            {
            ContentDescriptor::ECPropertiesField const& propertiesField = *descriptorField->AsPropertiesField();
            ECClassCP propertyClass = nullptr;
            if (parentField)
                propertyClass = parentField->IsRelationshipField() ? &parentField->GetRelationshipClass() : &parentField->GetContentClass();
            else
                propertyClass = m_relationshipClass ? m_relationshipClass : m_class;

            ContentDescriptor::Property const* fieldPropertyForThisContract = FindMatchingProperty(propertiesField, propertyClass);
            if (propertiesField.IsCompositePropertiesField() && m_skipCompositePropertyFields)
                {
                didCreateNonNullField = (nullptr != fieldPropertyForThisContract);
                continue;
                }

            if (nullptr != fieldPropertyForThisContract)
                {
                Utf8String propertyAccessor = GetPropertySelectClauseFromAccessString(fieldPropertyForThisContract->GetProperty().GetName());
                ECPropertyCR ecProperty = fieldPropertyForThisContract->GetProperty();
                contractField = CreatePropertySelectField(propertiesField.GetUniqueName().c_str(), fieldPropertyForThisContract->GetPrefix(),
                    propertyAccessor.c_str(), ecProperty
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
                    , m_descriptor->OnlyDistinctValues()
#endif
                    );
                didCreateNonNullField = true;
                }
            else
                {
                ECPropertyCR ecProperty = propertiesField.GetProperties().front().GetProperty();
                contractField = CreateNullPropertySelectField(propertiesField.GetUniqueName().c_str(), ecProperty);
                }
            }
        else if (descriptorField->IsNestedContentField() && descriptorField->AsNestedContentField()->AsRelatedContentField())
            {
            if (!ShouldHandleRelatedContentField(*descriptorField->AsNestedContentField()->AsRelatedContentField()))
                continue;

            auto relatedContentField = descriptorField->AsNestedContentField()->AsRelatedContentField();
            Utf8PrintfString keyFieldName("/key/%s", relatedContentField->GetUniqueName().c_str());
            bvector<PresentationQueryContractFieldCPtr> nestedContractFields;
            if (CreateContractFields(nestedContractFields, relatedContentField->GetFields(), relatedContentField))
                {
                contractFields.push_back(CreateInstanceKeyField(keyFieldName.c_str(),
                    relatedContentField->IsRelationshipField() ? relatedContentField->GetRelationshipClassAlias().c_str() : relatedContentField->GetContentClassAlias().c_str(), ECClassId()));
                didCreateNonNullField = true;
                }
            else
                {
                contractFields.push_back(CreateInstanceKeyField(keyFieldName.c_str(), nullptr, ECClassId()));
                }
            ContainerHelpers::Push(contractFields, nestedContractFields);
            }

        if (contractField.IsNull())
            continue;

        contractFields.push_back(contractField);
        }
    return didCreateNonNullField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryContractFieldCPtr> ContentQueryContract::_GetFields() const
    {
    if (nullptr == m_fields)
        {
        m_fields = std::make_unique<bvector<PresentationQueryContractFieldCPtr>>();

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        if (!m_descriptor->OnlyDistinctValues())
            {
#endif
            // contract id
            if (0 != GetId())
                m_fields->push_back(PresentationQueryContractSimpleField::Create(ContractIdFieldName, std::to_string(GetId()).c_str(), false));
            else
                m_fields->push_back(PresentationQueryContractSimpleField::Create(nullptr, ContractIdFieldName, false));

            // primary instance key
            bvector<Utf8CP> selectAliases = m_relationshipClass ? bvector<Utf8CP>{m_relationshipClassAlias.c_str()} : m_queryInfo.GetSelectAliases(IQueryInfoProvider::SELECTION_SOURCE_From);
            Utf8CP selectAlias = selectAliases.empty() ? nullptr : selectAliases.front();
            m_fields->push_back(CreateInstanceKeyField(ECInstanceKeysFieldName, selectAlias, ECClassId()));

            // input instance key
            m_fields->push_back(CreateInputKeysField(selectAlias));
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
            }
#endif

        if (0 == ((int)ContentFlags::KeysOnly & m_descriptor->GetContentFlags()))
            {
            // related instances' info
            m_fields->push_back(CreateRelatedInstanceInfoField(m_relatedInstancePaths));

            if (m_displayLabelField.IsValid())
                m_fields->push_back(m_displayLabelField);

            // fields
            CreateContractFields(*m_fields, m_descriptor->GetAllFields(), nullptr);
            }
        }
    return *m_fields;
    }
