/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static member variable initialization
Utf8CP const ECDbSystemSchemaHelper::ECDBSYSTEM_SCHEMANAME = "ECDb_System";
Utf8CP const ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME = "ECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::ECCLASSID_PROPNAME = "ECClassId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME = "SourceECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME = "SourceECClassId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME = "TargetECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME = "TargetECClassId";

Utf8CP const ECDbSystemSchemaHelper::ECSQLSYSTEMPROPERTIES_CLASSNAME = "ECSqlSystemProperties";

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECClassCP ECDbSystemSchemaHelper::GetECSqlSystemPropertiesClass(ECDbSchemaManagerCR schemaManager)
    {
    ECClassCP systemPropsClass = schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, ECSQLSYSTEMPROPERTIES_CLASSNAME);
    BeAssert(systemPropsClass != nullptr);
    return systemPropsClass;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind)
    {
    ECClassCP systemPropsClass = GetECSqlSystemPropertiesClass(schemaManager);
    if (systemPropsClass == nullptr)
        {
        //log and assert already done in callee
        return nullptr;
        }

    Utf8CP requiredPropName = GetPropertyName(kind);
    return GetECProperty(*systemPropsClass, requiredPropName);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::IsSystemProperty(ECPropertyCR ecProperty, ECSqlSystemProperty kind)
    {
    Utf8CP requiredPropName = GetPropertyName(kind);
    return ecProperty.GetName().Equals(requiredPropName) && ecProperty.GetClass().GetName().Equals(ECSQLSYSTEMPROPERTIES_CLASSNAME);
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                09/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECPropertyCP ECDbSystemSchemaHelper::GetECProperty(ECClassCR ecClass, Utf8CP propertyName)
    {
    auto prop = ecClass.GetPropertyP(propertyName);
    if (prop == nullptr)
        {
        LOG.fatalv("ECProperty '%s' not found in ECClass '%s' from system ECSchema '%s'.", propertyName, Utf8String(ecClass.GetName().c_str()).c_str(), ECDBSYSTEM_SCHEMANAME);
        BeAssert(false && "Fatal error. ECProperty not found in ECSqlSystemProperties ECClass from system ECSchema.");
        }

    return prop;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
Utf8CP ECDbSystemSchemaHelper::GetPropertyName(ECSqlSystemProperty kind)
    {
    switch (kind)
        {
            case ECSqlSystemProperty::ECInstanceId:
                return ECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::ECClassId:
                return ECCLASSID_PROPNAME;
            case ECSqlSystemProperty::SourceECInstanceId:
                return SOURCEECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::SourceECClassId:
                return SOURCEECCLASSID_PROPNAME;
            case ECSqlSystemProperty::TargetECInstanceId:
                return TARGETECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::TargetECClassId:
                return TARGETECCLASSID_PROPNAME;
            default:
                BeAssert(false && "ECSqlSystemProperty enum has new value. Update ECDbSystemSchemaHelper::GetPropertyName accordingly.");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::TryGetSystemPropertyKind(ECSqlSystemProperty& kind, ECN::ECPropertyCR ecProperty)
    {
    ECSqlSystemProperty candidateKind = ECSqlSystemProperty::ECInstanceId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    candidateKind = ECSqlSystemProperty::ECClassId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    candidateKind = ECSqlSystemProperty::SourceECInstanceId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    candidateKind = ECSqlSystemProperty::SourceECClassId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    candidateKind = ECSqlSystemProperty::TargetECInstanceId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    candidateKind = ECSqlSystemProperty::TargetECClassId;
    if (IsSystemProperty(ecProperty, candidateKind))
        {
        kind = candidateKind;
        return true;
        }

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
Utf8CP ECDbSystemSchemaHelper::ToString(ECSqlSystemProperty systemProperty)
    {
    return GetPropertyName(systemProperty);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
ECN::ECClassCP ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ECDbCR ecdb, ECN::PrimitiveType primitiveType)
    {
    ECDbSchemaManager const& schemaManager = ecdb.Schemas();

    switch (primitiveType)
        {
            case PRIMITIVETYPE_Binary:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "BinaryArray");
            case PRIMITIVETYPE_Boolean:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "BooleanArray");
            case PRIMITIVETYPE_DateTime:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "DateTimeArray");
            case PRIMITIVETYPE_Double:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "DoubleArray");
            case PRIMITIVETYPE_Integer:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "IntegerArray");
            case PRIMITIVETYPE_Long:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "LongArray");
            case PRIMITIVETYPE_Point2D:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "Point2dArray");
            case PRIMITIVETYPE_Point3D:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "Point3dArray");
            case PRIMITIVETYPE_String:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "StringArray");
            case PRIMITIVETYPE_IGeometry:
                return schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "GeometryArray");
            default:
                BeAssert(false && "Unsupported primitive type. Adjust this method for new value of ECN::PrimitiveType enum");
                return nullptr;
        }
    }
    
END_BENTLEY_SQLITE_EC_NAMESPACE
