/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
WCharCP const ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME_W = L"ECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME = "SourceECInstanceId";
WCharCP const ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME_W = L"SourceECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME = "SourceECClassId";
WCharCP const ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME_W = L"SourceECClassId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME = "TargetECInstanceId";
WCharCP const ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME_W = L"TargetECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME = "TargetECClassId";
WCharCP const ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME_W = L"TargetECClassId";
Utf8CP const ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME = "ECPropertyPathId";
WCharCP const ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME_W = L"ECPropertyPathId";
Utf8CP const ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME = "ECArrayIndex";
WCharCP const ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME_W = L"ECArrayIndex";
Utf8CP const ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME = "ParentECInstanceId";
WCharCP const ECDbSystemSchemaHelper::PARENTECINSTANCEID_PROPNAME_W = L"ParentECInstanceId";
//Utf8CP const ECDbSystemSchemaHelper::ECPROPERTYID_PROPNAME = "ECPropertyId";
//WCharCP const ECDbSystemSchemaHelper::ECPROPERTYID_PROPNAME_W = L"ECPropertyId";
Utf8CP const ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME = "OwnerECInstanceId";
WCharCP const ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME_W = L"OwnerECInstanceId";

WCharCP const ECDbSystemSchemaHelper::ECSQLSYSTEMPROPERTIES_CLASSNAME = L"ECSqlSystemProperties";

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECClassCP ECDbSystemSchemaHelper::GetECSqlSystemPropertiesClass (ECDbSchemaManagerCR schemaManager)
    {
    //WIP_ECDb: Calling GetECClass and then GetSchema does not load all remaining classes. Instead
    // only the class requested with GetECClass is in the schema. Is this by design?

    //ECClassP systemPropsClass = nullptr;
    //auto stat = schemaManager.GetECClass (systemPropsClass, ECDBSYSTEM_SCHEMANAME, ECSQLSYSTEMPROPERTIES_CLASSNAME);
    //if (stat != SUCCESS)
    //        {
      //  BeAssert (false && "ECDbSystemSchemaHelper> Getting ECSqlSystemProperties class from ECDb_System schema should never fail.");
        //return nullptr;
        //}
    //return systemPropsClass;

    auto systemSchema = GetSchema (schemaManager);
    if (systemSchema == nullptr)
        {
        //log and assert already done in callee
        return nullptr;
        }

    //log and assert already done in callee
    return GetECClass (*systemSchema, ECSQLSYSTEMPROPERTIES_CLASSNAME);
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECSchemaCP ECDbSystemSchemaHelper::GetSchema (ECDbSchemaManagerCR schemaManager)
    {
    ECSchemaCP schema = schemaManager.GetECSchema (ECDBSYSTEM_SCHEMANAME);
    if (schema == nullptr)
        {
        LOG.errorv ("Could not load system ECSchema '%s'. Make sure the standard ECSchemas are deployed in the expected location.", ECDBSYSTEM_SCHEMANAME);
        BeAssert (false && "Could not load system ECSchema. Make sure the standard ECSchemas are deployed in the expected location.");
        return nullptr;
        }

    return schema;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty (ECDbSchemaManagerCR schemaManager, ECSqlSystemProperty kind)
    {
    auto systemPropsClass = GetECSqlSystemPropertiesClass (schemaManager);
    if (systemPropsClass == nullptr)
        {
        //log and assert already done in callee
        return nullptr;
        }

    Utf8CP requiredPropName = GetPropertyName (kind);
    return GetECProperty (*systemPropsClass, requiredPropName);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::IsSystemProperty (ECPropertyCR ecProperty, ECSqlSystemProperty kind)
    {
    WCharCP requiredPropName = GetPropertyNameW (kind);
    return ecProperty.GetName ().Equals (requiredPropName) && ecProperty.GetClass ().GetName ().Equals (ECSQLSYSTEMPROPERTIES_CLASSNAME);
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                09/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECClassCP ECDbSystemSchemaHelper::GetECClass (ECSchemaCR ecdbSystemSchema, WCharCP className)
    {
    auto ecClass = ecdbSystemSchema.GetClassCP (className);
    if (ecClass == nullptr)
        {
        LOG.fatalv ("ECClass '%s' not found in system ECSchema '%s'.", Utf8String (className).c_str (), ECDBSYSTEM_SCHEMANAME);
        BeAssert (false && "Fatal error. ECClass not found in system ECSchema.");
        }

    return ecClass;

    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                09/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
ECPropertyCP ECDbSystemSchemaHelper::GetECProperty (ECClassCR ecClass, Utf8CP propertyName)
    {
    auto prop = ecClass.GetPropertyP (propertyName);
    if (prop == nullptr)
        {
        LOG.fatalv ("ECProperty '%s' not found in ECClass '%s' from system ECSchema '%s'.", propertyName, Utf8String (ecClass.GetName ().c_str ()).c_str (), ECDBSYSTEM_SCHEMANAME);
        BeAssert (false && "Fatal error. ECProperty not found in ECSqlSystemProperties ECClass from system ECSchema.");
        }

    return prop;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
Utf8CP ECDbSystemSchemaHelper::GetPropertyName (ECSqlSystemProperty kind)
    {
    switch (kind)
        {
            case ECSqlSystemProperty::ECInstanceId:
                return ECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::SourceECInstanceId:
                return SOURCEECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::SourceECClassId:
                return SOURCEECCLASSID_PROPNAME;
            case ECSqlSystemProperty::TargetECInstanceId:
                return TARGETECINSTANCEID_PROPNAME;
            case ECSqlSystemProperty::TargetECClassId:
                return TARGETECCLASSID_PROPNAME;
            case ECSqlSystemProperty::ECPropertyPathId:
                return ECPROPERTYPATHID_PROPNAME;
            case ECSqlSystemProperty::ECArrayIndex:
                return ECARRAYINDEX_PROPNAME;
            case ECSqlSystemProperty::ParentECInstanceId:
                return PARENTECINSTANCEID_PROPNAME;
            default:
                BeAssert (false && "ECSqlSystemProperty enum has new value. Update ECDbSystemSchemaHelper::GetPropertyName accordingly.");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
WCharCP ECDbSystemSchemaHelper::GetPropertyNameW (ECSqlSystemProperty kind)
    {
    switch (kind)
        {
            case ECSqlSystemProperty::ECInstanceId:
                return ECINSTANCEID_PROPNAME_W;
            case ECSqlSystemProperty::SourceECInstanceId:
                return SOURCEECINSTANCEID_PROPNAME_W;
            case ECSqlSystemProperty::SourceECClassId:
                return SOURCEECCLASSID_PROPNAME_W;
            case ECSqlSystemProperty::TargetECInstanceId:
                return TARGETECINSTANCEID_PROPNAME_W;
            case ECSqlSystemProperty::TargetECClassId:
                return TARGETECCLASSID_PROPNAME_W;
            case ECSqlSystemProperty::ECPropertyPathId:
                return ECPROPERTYPATHID_PROPNAME_W;
            case ECSqlSystemProperty::ECArrayIndex:
                return ECARRAYINDEX_PROPNAME_W;
            case ECSqlSystemProperty::ParentECInstanceId:
                return PARENTECINSTANCEID_PROPNAME_W;
            default:
                BeAssert (false && "ECSqlSystemProperty enum has new value. Update ECDbSystemSchemaHelper::GetPropertyName accordingly.");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::TryGetSystemPropertyKind (ECSqlSystemProperty& kind, ECN::ECPropertyCR ecProperty)
    {
    std::vector<ECSqlSystemProperty> kindList { ECSqlSystemProperty::ECInstanceId, 
                        ECSqlSystemProperty::SourceECInstanceId, ECSqlSystemProperty::SourceECClassId, 
                        ECSqlSystemProperty::TargetECInstanceId, ECSqlSystemProperty::TargetECClassId, 
                        ECSqlSystemProperty::ECPropertyPathId, 
                        ECSqlSystemProperty::ECArrayIndex, 
                        ECSqlSystemProperty::ParentECInstanceId };
   
    for (auto candidateKind : kindList)
        {
        if (IsSystemProperty (ecProperty, candidateKind))
            {
            kind = candidateKind;
            return true;
            }
        }

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                07/2014
//+---------------+---------------+---------------+---------------+---------------+-
//static 
Utf8CP ECDbSystemSchemaHelper::ToString (ECSqlSystemProperty systemProperty)
    {
    return GetPropertyName (systemProperty);
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
