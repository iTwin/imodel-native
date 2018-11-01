/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DynamicSelectClauseECClass.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::Initialize()
    {
    if (m_schema != nullptr)
        return ECSqlStatus::Success;

    if (ECObjectsStatus::Success != ECSchema::CreateSchema(m_schema, "ECDbSystemTemp_DynamicECSqlSelectClause", "tempdynecsqlselectclause", 1, 0, 0))
        return ECSqlStatus::Error;

    if (ECObjectsStatus::Success != m_schema->CreateEntityClass(m_class, "DynamicECSqlSelectClause"))
        return ECSqlStatus::Error;

    //is never instantiated
    m_class->SetClassModifier(ECClassModifier::Abstract);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::GeneratePropertyIfRequired(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext& ctx, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp)
    {
    ECSqlStatus stat = Initialize();
    if (!stat.IsSuccess())
        return stat;

    generatedProperty = nullptr;

    //A property for the select clause item is generated
    //- if the exp is no prop name exp or
    //- if the exp is a prop name exp and has a column alias or is a ref to an item in an inner select

    //if select clause items have the same names, the following rules apply:
    //-if for both items no prop has to be generated -> ok, duplicate names are fine
    //-if both items have to be generated and both don't have aliases -> prop name of second exp will get a _1 (and _2 etc) suffix
    // (prop names must be unique)
    //-if one of the items has an alias -> error as ECSQL author has to make sure that aliases are unambiguous
    Utf8String propName = selectClauseItemExp.GetName();
    Utf8StringCR columnAlias = selectClauseItemExp.GetColumnAlias();
    auto it = m_selectClauseNames.find(propName);
    bool isDuplicateName = false;
    if (it != m_selectClauseNames.end())
        {
        DerivedPropertyExp const* otherSelectClauseItem = it->second;
        if (!columnAlias.empty() || !otherSelectClauseItem->GetColumnAlias().empty())
            {
            ctx.Issues().ReportV("Alias '%s' used in the select clause is ambiguous.", propName.c_str());
            return ECSqlStatus::InvalidECSql;
            }

        isDuplicateName = true;
        }

    //exp that are no prop name exps (e.g. constants or A+B, prop refs (ref to property in a nested select) or alias items always need generated prop
    const bool needsToGenerate = selectClauseItemPropNameExp == nullptr || !columnAlias.empty() || selectClauseItemPropNameExp->IsPropertyRef();
    if (needsToGenerate)
        {
        if (isDuplicateName)
            {
            //generate a unique prop name
            int suffixNr = 1;
            Utf8String uniquePropName;
            do
                {
                uniquePropName.Sprintf("%s_%d", propName.c_str(), suffixNr);
                suffixNr++;

                if (suffixNr > 1000) //arbitrary threshold to avoid end-less loop
                    {
                    ctx.Issues().ReportV("Could not generate a unique select clause item name for the item '%s'. Try to avoid duplicate select clause items.",
                                                                           selectClauseItemExp.ToECSql().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }

                } while (m_selectClauseNames.find(uniquePropName) != m_selectClauseNames.end());

                propName = uniquePropName;
                //now the name is not duplicate anymore
                isDuplicateName = false;
            }

        BeAssert(m_selectClauseNames.find(propName) == m_selectClauseNames.end() && "at this point select clause item name should be unique");
        ECSqlStatus stat = AddProperty(generatedProperty, ctx, propName, selectClauseItemExp, selectClauseItemPropNameExp);
        if (!stat.IsSuccess())
            return stat;
        }

    if (!isDuplicateName)
        m_selectClauseNames[propName] = &selectClauseItemExp;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::AddProperty(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext& ctx, Utf8StringCR propName, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp)
    {
    ECSqlTypeInfo const& typeInfo = selectClauseItemExp.GetExpression()->GetTypeInfo();
    const ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();

    Utf8String encodedPropName;
    ECNameValidation::EncodeToValidName(encodedPropName, propName);

    ECPropertyP generatedPropertyP = nullptr;
    switch (typeKind)
        {
            case ECSqlTypeInfo::Kind::Null:
            {
            PrimitiveECPropertyP primProp = nullptr;
            if (ECObjectsStatus::Success != GetClass().CreatePrimitiveProperty(primProp, encodedPropName, PRIMITIVETYPE_Integer))
                return ECSqlStatus::Error;

            //indicate that this is really of type NULL (which does not exist in ECObjects)
            primProp->SetExtendedTypeName("NULL"); 
            generatedPropertyP = primProp;
            break;
            }

            case ECSqlTypeInfo::Kind::Primitive:
            {
            PrimitiveECPropertyP primProp = nullptr;
            if (typeInfo.IsEnum())
                {
                BeAssert(typeInfo.GetEnumerationType() != nullptr);
                ECSqlStatus stat = AddReferenceToPropertyTypeSchema(typeInfo.GetEnumerationType()->GetSchema());
                if (!stat.IsSuccess())
                    return stat;

                if (ECObjectsStatus::Success != GetClass().CreateEnumerationProperty(primProp, encodedPropName, *typeInfo.GetEnumerationType()))
                    return ECSqlStatus::Error;
                }
            else
                {
                if (ECObjectsStatus::Success != GetClass().CreatePrimitiveProperty(primProp, encodedPropName, typeInfo.GetPrimitiveType()))
                    return ECSqlStatus::Error;
                }

            //Extended types are preserved as well
            if (typeInfo.HasExtendedType())
                primProp->SetExtendedTypeName(typeInfo.GetExtendedTypeName().c_str());

            if (!FeatureManager::IsAvailable(ctx.GetECDb(), Feature::SystemPropertiesHaveIdExtendedType) && selectClauseItemPropNameExp != nullptr && selectClauseItemPropNameExp->GetSystemPropertyInfo().IsId())
                {
                // In 4.0.0.2 files, the respective system property has already the extended type name Id. So it
                // gets added to the dynamic property automatically by the above code. For older files
                // we have to do that explicitly here.
                BeAssert(primProp->GetType() == PRIMITIVETYPE_Long);
                primProp->SetExtendedTypeName(EXTENDEDTYPENAME_Id);
                }

            generatedPropertyP = primProp;
            break;
            }

            case ECSqlTypeInfo::Kind::Struct:
            {
            ECStructClassCR structType = typeInfo.GetStructType();
            ECSqlStatus stat = AddReferenceToPropertyTypeSchema(structType.GetSchema());
            if (!stat.IsSuccess())
                return stat;

            ECStructClassCP asStruct = structType.GetStructClassCP();
            if (nullptr == asStruct)
                return ECSqlStatus::Error;

            StructECPropertyP structProp = nullptr;
            if (ECObjectsStatus::Success != GetClass().CreateStructProperty(structProp, encodedPropName, *asStruct))
                return ECSqlStatus::Error;

            generatedPropertyP = structProp;
            break;
            }

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            {
            PrimitiveArrayECPropertyP arrayProp = nullptr;
            if (typeInfo.GetEnumerationType() != nullptr)
                {
                ECSqlStatus stat = AddReferenceToPropertyTypeSchema(typeInfo.GetEnumerationType()->GetSchema());
                if (!stat.IsSuccess())
                    return stat;

                if (ECObjectsStatus::Success != GetClass().CreatePrimitiveArrayProperty(arrayProp, encodedPropName, *typeInfo.GetEnumerationType()))
                    return ECSqlStatus::Error;
                }
            else
                {
                if (ECObjectsStatus::Success != GetClass().CreatePrimitiveArrayProperty(arrayProp, encodedPropName, typeInfo.GetPrimitiveType()))
                    return ECSqlStatus::Error;
                }

            //Extended types are preserved as well
            if (typeInfo.HasExtendedType())
                arrayProp->SetExtendedTypeName(typeInfo.GetExtendedTypeName().c_str());

            if (typeInfo.GetArrayMinOccurs() != nullptr)
                arrayProp->SetMinOccurs(typeInfo.GetArrayMinOccurs().Value());

            if (typeInfo.GetArrayMaxOccurs() != nullptr)
                arrayProp->SetMaxOccurs(typeInfo.GetArrayMaxOccurs().Value());

            generatedPropertyP = arrayProp;
            break;
            }
            case ECSqlTypeInfo::Kind::StructArray:
            {
            ECStructClassCR structType = typeInfo.GetStructType();
            ECSqlStatus stat = AddReferenceToPropertyTypeSchema(structType.GetSchema());
            if (!stat.IsSuccess())
                return stat;

            ECStructClassCP asStruct = structType.GetStructClassCP();
            if (nullptr == asStruct)
                return ECSqlStatus::Error;

            StructArrayECPropertyP structArrayProp = nullptr;
            if (ECObjectsStatus::Success != GetClass().CreateStructArrayProperty(structArrayProp, encodedPropName, *asStruct))
                return ECSqlStatus::Error;

            if (typeInfo.GetArrayMinOccurs() != nullptr)
                structArrayProp->SetMinOccurs(typeInfo.GetArrayMinOccurs().Value());

            if (typeInfo.GetArrayMaxOccurs() != nullptr)
                structArrayProp->SetMaxOccurs(typeInfo.GetArrayMaxOccurs().Value());

            generatedPropertyP = structArrayProp;
            break;
            }
            default:
                BeAssert("Adding dynamic select clause property failed because of unexpected and unhandled property type.");
                return ECSqlStatus::Error;
        }

    generatedPropertyP->SetDisplayLabel(propName);
    generatedPropertyP->SetIsReadOnly(true);

    generatedProperty = generatedPropertyP;
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::AddReferenceToPropertyTypeSchema(ECSchemaCR propertyTypeSchema) const
    {
    if (ECSchema::IsSchemaReferenced(GetSchema(), propertyTypeSchema))
        return ECSqlStatus::Success;

    if (ECObjectsStatus::Success != GetSchema().AddReferencedSchema(const_cast<ECSchemaR> (propertyTypeSchema)))
        return ECSqlStatus::Error;

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
