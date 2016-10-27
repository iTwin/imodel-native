/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DynamicSelectClauseECClass.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "DynamicSelectClauseECClass.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define SCHEMANAME "ECSqlStatement"
#define CLASSNAME "ECSqlSelectClause"

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass::DynamicSelectClauseECClass(DynamicSelectClauseECClass const& rhs)
    : m_schema(rhs.m_schema), m_class(rhs.m_class)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass& DynamicSelectClauseECClass::operator= (DynamicSelectClauseECClass const& rhs)
    {
    if (this != &rhs)
        {
        m_schema = rhs.m_schema;
        m_class = rhs.m_class;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass::DynamicSelectClauseECClass(DynamicSelectClauseECClass&& rhs)
    : m_schema(std::move(rhs.m_schema)), m_class(std::move(rhs.m_class))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass& DynamicSelectClauseECClass::operator= (DynamicSelectClauseECClass&& rhs)
    {
    if (this != &rhs)
        {
        m_schema = std::move(rhs.m_schema);
        m_class = std::move(rhs.m_class);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::Initialize()
    {
    if (m_schema != nullptr)
        return ECSqlStatus::Success;

    if (ECObjectsStatus::Success != ECSchema::CreateSchema(m_schema, SCHEMANAME, SCHEMANAME, 1, 0, 0))
        return ECSqlStatus::Error;

    if (ECObjectsStatus::Success != m_schema->CreateEntityClass(m_class, CLASSNAME))
        return ECSqlStatus::Error;

    //is never instantiated
    m_class->SetClassModifier(ECClassModifier::Abstract);
    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::SetBackReferenceToPropertyPath(ECPropertyR generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbCR ecdb)
    {
    if (selectClauseItemExp.GetExpression()->GetType() != Exp::Type::PropertyName)
        {
        return ECSqlStatus::Success;
        }

    auto propertyNameExp = static_cast<PropertyNameExp const*>(selectClauseItemExp.GetExpression());
    if (propertyNameExp->IsPropertyRef())
        {
        auto endPointPropertyName = propertyNameExp->GetPropertyRef()->GetEndPointPropertyNameIfAny();
        if (endPointPropertyName == nullptr)
            return ECSqlStatus::Success;

        propertyNameExp = endPointPropertyName;
        }


    auto ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(ecdb.GetSchemaLocater());
    auto bscaKey = SchemaKey("Bentley_Standard_CustomAttributes", 1, 0);
    auto bsca = ctx->LocateSchema(bscaKey, SchemaMatchType::Latest);
    if (bsca.IsNull())
        {
        LOG.error("Failed to find Bentley_Standard_CustomAttributes schema");
        return ECSqlStatus::Error;
        }

    auto defMetaData = bsca->GetClassCP("DefinitionMetaData");
    if (defMetaData == nullptr)
        {
        LOG.error("Failed to find class DefinitionMetaData in Bentley_Standard_CustomAttributes schema");
        return ECSqlStatus::Error;
        }

    auto defMetaDataInst = defMetaData->GetDefaultStandaloneEnabler()->CreateInstance();
    BeAssert(defMetaDataInst != nullptr);

    Utf8String qualifiedPropertyPath;
    if (SUCCESS != propertyNameExp->GetPropertyPath().TryGetQualifiedPath(qualifiedPropertyPath))
        return ECSqlStatus::Error;


    if (defMetaDataInst->SetValue("DefinitionBackReference", ECValue(qualifiedPropertyPath.c_str(), false)) != ECObjectsStatus::Success)
        return ECSqlStatus::Error;

    ECSqlStatus status = AddReferenceToStructSchema(*bsca);
    if (!status.IsSuccess())
        return status;

    if (generatedProperty.SetCustomAttribute(*defMetaDataInst) != ECObjectsStatus::Success)
        return ECSqlStatus::Error;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2015
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::GeneratePropertyIfRequired(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext& ctx, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp, ECDbCR ecdb)
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
            ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Alias '%s' used in the select clause is ambiguous.", propName.c_str());
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
                    ctx.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not generate a unique select clause item name for the item '%s'. Try to avoid duplicate select clause items.",
                                                                           selectClauseItemExp.ToECSql().c_str());
                    return ECSqlStatus::InvalidECSql;
                    }

                } while (m_selectClauseNames.find(uniquePropName) != m_selectClauseNames.end());

                propName = uniquePropName;
                //now the name is not duplicate anymore
                isDuplicateName = false;
            }

        BeAssert(m_selectClauseNames.find(propName) == m_selectClauseNames.end() && "at this point select clause item name should be unique");
        ECSqlStatus stat = AddProperty(generatedProperty, ctx, propName, selectClauseItemExp, ecdb);
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
ECSqlStatus DynamicSelectClauseECClass::AddProperty(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext& ctx, Utf8StringCR propName, DerivedPropertyExp const& selectClauseItemExp, ECDbCR ecdb)
    {
    ECSqlTypeInfo const& typeInfo = selectClauseItemExp.GetExpression()->GetTypeInfo();
    const ECSqlTypeInfo::Kind typeKind = typeInfo.GetKind();

    Utf8String encodedPropName;
    ECNameValidation::EncodeToValidName(encodedPropName, propName);

    switch (typeKind)
        {
            case ECSqlTypeInfo::Kind::Primitive:
            case ECSqlTypeInfo::Kind::Null:
            {
            PrimitiveECPropertyP primProp = nullptr;
            if (ECObjectsStatus::Success != GetClassR().CreatePrimitiveProperty(primProp, encodedPropName, typeInfo.GetPrimitiveType()))
                return ECSqlStatus::Error;

            generatedProperty = primProp;
            break;
            }

            case ECSqlTypeInfo::Kind::Struct:
            {
            ECStructClassCR structType = typeInfo.GetStructType();
            ECSqlStatus stat = AddReferenceToStructSchema(structType.GetSchema());
            if (!stat.IsSuccess())
                return stat;

            ECStructClassCP asStruct = structType.GetStructClassCP();
            if (nullptr == asStruct)
                return ECSqlStatus::Error;

            StructECPropertyP structProp = nullptr;
            if (ECObjectsStatus::Success != GetClassR().CreateStructProperty(structProp, encodedPropName, *asStruct))
                return ECSqlStatus::Error;

            generatedProperty = structProp;
            break;
            }

            case ECSqlTypeInfo::Kind::PrimitiveArray:
            {
            PrimitiveArrayECPropertyP arrayProp = nullptr;
            if (ECObjectsStatus::Success != GetClassR().CreatePrimitiveArrayProperty(arrayProp, encodedPropName, typeInfo.GetPrimitiveType()))
                return ECSqlStatus::Error;

            generatedProperty = arrayProp;
            break;
            }
            case ECSqlTypeInfo::Kind::StructArray:
            {
            ECStructClassCR structType = typeInfo.GetStructType();
            ECSqlStatus stat = AddReferenceToStructSchema(structType.GetSchema());
            if (!stat.IsSuccess())
                return stat;

            ECStructClassCP asStruct = structType.GetStructClassCP();
            if (nullptr == asStruct)
                return ECSqlStatus::Error;

            StructArrayECPropertyP structArrayProp = nullptr;
            if (ECObjectsStatus::Success != GetClassR().CreateStructArrayProperty(structArrayProp, encodedPropName, asStruct))
                return ECSqlStatus::Error;

            generatedProperty = structArrayProp;
            break;
            }
            default:
                BeAssert("Adding dynamic select clause property failed because of unexpected and unhandled property type.");
                return ECSqlStatus::Error;
        }

    const_cast<ECPropertyP>(generatedProperty)->SetDisplayLabel(propName);

    return SetBackReferenceToPropertyPath(*const_cast<ECPropertyP>(generatedProperty), selectClauseItemExp, ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::AddReferenceToStructSchema(ECSchemaCR structSchema) const
    {
    if (ECSchema::IsSchemaReferenced(GetSchemaR(), structSchema))
        return ECSqlStatus::Success;

    auto stat = GetSchemaR().AddReferencedSchema(const_cast<ECSchemaR> (structSchema));
    return stat == ECObjectsStatus::Success ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
