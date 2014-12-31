/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DynamicSelectClauseECClass.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "DynamicSelectClauseECClass.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle 10/13
//---------------------------------------------------------------------------------------
//static
WCharCP const DynamicSelectClauseECClass::SCHEMANAME = L"ECSqlStatement";
WCharCP const DynamicSelectClauseECClass::CLASSNAME = L"ECSqlSelectClause";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass::DynamicSelectClauseECClass ()
: m_schema (nullptr), m_class (nullptr)
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass::DynamicSelectClauseECClass (DynamicSelectClauseECClass const& rhs)
    : m_schema (rhs.m_schema), m_class (rhs.m_class)
    {
    }

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
DynamicSelectClauseECClass::DynamicSelectClauseECClass (DynamicSelectClauseECClass&& rhs)
    : m_schema (move (rhs.m_schema)), m_class (move (rhs.m_class))
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
DynamicSelectClauseECClass& DynamicSelectClauseECClass::operator= (DynamicSelectClauseECClass&& rhs)
    {
    if (this != &rhs)
        {
        m_schema = move (rhs.m_schema);
        m_class = move (rhs.m_class);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool DynamicSelectClauseECClass::IsGeneratedProperty (ECPropertyCR selectClauseProperty) const
    {
    //pointer comparison as other ECSqlStatements could float around which would have class with same name
    //if class was not generated, select clause doesn't contain any generated items -> return false in that case.
    return m_class != nullptr && selectClauseProperty.GetClass () == GetClassR ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::Initialize ()
    {
    if (m_schema != nullptr)
        return ECSqlStatus::Success;

    if (ECOBJECTS_STATUS_Success != ECSchema::CreateSchema (m_schema, SCHEMANAME, 1, 0))
        return ECSqlStatus::ProgrammerError;

    if (ECOBJECTS_STATUS_Success == m_schema->CreateClass(m_class, CLASSNAME))
        return ECSqlStatus::Success;
    else
        return ECSqlStatus::ProgrammerError;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::ParseBackReferenceToPropertyPath(PropertyPath& propertyPath, ECPropertyCR generatedProperty, ECDbR ecdb)
    {
    auto map = ecdb.GetImplR().GetECDbMap();
    auto defMetaDataInst = generatedProperty.GetCustomAttribute (L"DefinitionMetaData");
    if (defMetaDataInst == nullptr)
        {
        return ECSqlStatus::ProgrammerError;
        }
    
    ECValue v;
    if (defMetaDataInst->GetValue(v, L"DefinitionBackReference") != ECOBJECTS_STATUS_Success)
        return ECSqlStatus::ProgrammerError;

    return PropertyPath::TryParseQualifiedPath(propertyPath, Utf8String(v.GetString()), ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::SetBackReferenceToPropertyPath (ECPropertyR generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbR ecdb)
    {
    if (selectClauseItemExp.GetExpression ()->GetType() != Exp::Type::PropertyName)
        {
        return ECSqlStatus::Success;
        }

    auto propertyNameExp = static_cast<PropertyNameExp const*>(selectClauseItemExp.GetExpression());
    auto ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(ecdb.GetSchemaLocater ());
    auto bscaKey = SchemaKey(L"Bentley_Standard_CustomAttributes", 1, 0);
    auto bsca = ctx->LocateSchema(bscaKey, SCHEMAMATCHTYPE_Latest);
    if (bsca.IsNull())
        {
        LOG.error("Failed to find Bentley_Standard_CustomAttributes schema");
        return ECSqlStatus::ProgrammerError;
        }

    auto defMetaData = bsca->GetClassCP(L"DefinitionMetaData");
    if(defMetaData == nullptr)
        {
        LOG.error("Failed to find class DefinitionMetaData in Bentley_Standard_CustomAttributes schema");
        return ECSqlStatus::ProgrammerError;
        }

    auto defMetaDataInst = defMetaData->GetDefaultStandaloneEnabler()->CreateInstance();
    BeAssert(defMetaDataInst != nullptr);
    
    Utf8String qualifiedPropertyPath;
    auto status = propertyNameExp->GetPropertyPath().TryGetQualifiedPath(qualifiedPropertyPath);
    if (status != ECSqlStatus::Success)
        return status;


    if (defMetaDataInst->SetValue(L"DefinitionBackReference",  ECValue(qualifiedPropertyPath.c_str(), false)) != ECOBJECTS_STATUS_Success)
        return ECSqlStatus::ProgrammerError;

    status = AddReferenceToStructSchema (*bsca);
    if (status != ECSqlStatus::Success)
        return status;

    if (generatedProperty.SetCustomAttribute(*defMetaDataInst) != ECOBJECTS_STATUS_Success)
        return ECSqlStatus::ProgrammerError;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::AddProperty (ECN::ECPropertyCP& generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbR ecdb)
    {
    auto stat = Initialize ();
    if (stat != ECSqlStatus::Success)
        return stat;

    auto propName = selectClauseItemExp.GetName ();
    WString propNameW (propName.c_str(), BentleyCharEncoding::Utf8);

    auto const& typeInfo = selectClauseItemExp.GetExpression ()->GetTypeInfo ();
    const auto typeKind = typeInfo.GetKind ();

    switch (typeKind)
        {
        case ECSqlTypeInfo::Kind::Primitive:
        case ECSqlTypeInfo::Kind::Null:
            {
            PrimitiveECPropertyP primProp = nullptr;
            auto ecstat = GetClassR ().CreatePrimitiveProperty (primProp, propNameW.c_str (), typeInfo.GetPrimitiveType ());
            if (ecstat != ECOBJECTS_STATUS_Success)
                return ECSqlStatus::ProgrammerError;

            generatedProperty = primProp;
            break;
            }

        case ECSqlTypeInfo::Kind::Struct:
            {
            auto const& structType = typeInfo.GetStructType ();
            auto stat = AddReferenceToStructSchema (structType.GetSchema ());
            if (stat != ECSqlStatus::Success)
                return stat;

            StructECPropertyP structProp = nullptr;
            auto ecstat = GetClassR ().CreateStructProperty (structProp, propNameW.c_str (), structType);
            if (ecstat != ECOBJECTS_STATUS_Success)
                return ECSqlStatus::ProgrammerError;

            generatedProperty = structProp;
            break;
            }

        case ECSqlTypeInfo::Kind::PrimitiveArray:
            {
            ArrayECPropertyP arrayProp = nullptr;
            auto ecstat = GetClassR ().CreateArrayProperty (arrayProp, propNameW.c_str (), typeInfo.GetPrimitiveType ());
            if (ecstat != ECOBJECTS_STATUS_Success)
                return ECSqlStatus::ProgrammerError;

            generatedProperty = arrayProp;
            break;
            }
        case ECSqlTypeInfo::Kind::StructArray:
            {
            auto const& structType = typeInfo.GetStructType ();
            auto stat = AddReferenceToStructSchema (structType.GetSchema ());
            if (stat != ECSqlStatus::Success)
                return stat;

            ArrayECPropertyP arrayProp = nullptr;
            auto ecstat = GetClassR ().CreateArrayProperty (arrayProp, propNameW.c_str (), &structType);
            if (ecstat != ECOBJECTS_STATUS_Success)
                return ECSqlStatus::ProgrammerError;

            generatedProperty = arrayProp;
            break;
            }
        default:
            BeAssert ("Adding dynamic select clause property failed because of unexpected and unhandled property type.");
            return ECSqlStatus::ProgrammerError;
        }

    return SetBackReferenceToPropertyPath (*const_cast<ECPropertyP>(generatedProperty), selectClauseItemExp, ecdb);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus DynamicSelectClauseECClass::AddReferenceToStructSchema (ECSchemaCR structSchema) const
    {
    if (ECSchema::IsSchemaReferenced (GetSchemaR (), structSchema))
        return ECSqlStatus::Success;

    auto stat = GetSchemaR ().AddReferencedSchema (const_cast<ECSchemaR> (structSchema));
    return stat == ECOBJECTS_STATUS_Success ? ECSqlStatus::Success : ECSqlStatus::ProgrammerError;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECClassR DynamicSelectClauseECClass::GetClassR() const
    {
    return *m_class;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaR DynamicSelectClauseECClass::GetSchemaR() const
    {
    return *m_schema;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
