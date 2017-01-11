/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBSYSTEM_SCHEMANAME "ECDb_System"

#define CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME "ClassECSqlSystemProperties"
#define REL_ECSQLSYSTEMPROPERTIES_CLASSNAME "RelationshipECSqlSystemProperties"
#define POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME "PointECSqlSystemProperties"
#define NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME "NavigationECSqlSystemProperties"

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                06/2013
//+---------------+---------------+---------------+---------------+---------------+-
//static member variable initialization
Utf8CP const ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME = "ECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::ECCLASSID_PROPNAME = "ECClassId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME = "SourceECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME = "SourceECClassId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME = "TargetECInstanceId";
Utf8CP const ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME = "TargetECClassId";
Utf8CP const ECDbSystemSchemaHelper::NAVPROP_ID_PROPNAME = "Id";
Utf8CP const ECDbSystemSchemaHelper::NAVPROP_RELECCLASSID_PROPNAME = "RelECClassId";
Utf8CP const ECDbSystemSchemaHelper::POINTPROP_X_PROPNAME = "X";
Utf8CP const ECDbSystemSchemaHelper::POINTPROP_Y_PROPNAME = "Y";
Utf8CP const ECDbSystemSchemaHelper::POINTPROP_Z_PROPNAME = "Z";


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
BentleyStatus ECDbSystemSchemaHelper::TryGetSystemPropertyInfo(ECSqlSystemPropertyInfo& info, ECN::ECPropertyCR ecProperty) const
    {
    if (SUCCESS != InitializeCache())
        return ERROR;

    if (!ecProperty.HasId())
        return ERROR;

    auto it = m_byPropIdCache.find(ecProperty.GetId());
    if (it == m_byPropIdCache.end())
        info = ECSqlSystemPropertyInfo();
    else
        info = it->second;

    return SUCCESS;
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
bool ECDbSystemSchemaHelper::Equals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo const& sysPropInfo) const
    {
    BeAssert(prop.HasId());
    auto it = m_byPropIdCache.find(prop.GetId());
    if (it == m_byPropIdCache.end())
        return false;

    return it->second == sysPropInfo;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Navigation sysProp) const
    {
    ECClassCP systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME);
    if (systemClass == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Navigation::Id:
                return systemClass->GetPropertyP(NAVPROP_ID_PROPNAME);

            case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                return systemClass->GetPropertyP(NAVPROP_RELECCLASSID_PROPNAME);

            default:
                BeAssert(false && "ECSqlSystemPropertyInfo::Navigation was changed. Need to adjust this code");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Point sysProp) const
    {
    ECClassCP systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME);
    if (systemClass == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Point::X:
                return systemClass->GetPropertyP(POINTPROP_X_PROPNAME);

            case ECSqlSystemPropertyInfo::Point::Y:
                return systemClass->GetPropertyP(POINTPROP_Y_PROPNAME);

            case ECSqlSystemPropertyInfo::Point::Z:
                return systemClass->GetPropertyP(POINTPROP_Z_PROPNAME);

            default:
                BeAssert(false && "ECSqlSystemPropertyInfo::Point was changed. Need to adjust this code");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Class classSysProp) const
    {
    ECClassCP systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME);
    if (systemClass == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    switch (classSysProp)
        {
            case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                return systemClass->GetPropertyP(ECINSTANCEID_PROPNAME);

            case ECSqlSystemPropertyInfo::Class::ECClassId:
                return systemClass->GetPropertyP(ECCLASSID_PROPNAME);

            default:
                BeAssert(false && "ECSqlSystemPropertyInfo::Class was changed. Need to adjust this code");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Relationship sysProp) const
    {
    ECClassCP systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, REL_ECSQLSYSTEMPROPERTIES_CLASSNAME);
    if (systemClass == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                return systemClass->GetPropertyP(SOURCEECINSTANCEID_PROPNAME);

            case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                return systemClass->GetPropertyP(SOURCEECCLASSID_PROPNAME);

            case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                return systemClass->GetPropertyP(TARGETECINSTANCEID_PROPNAME);

            case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                return systemClass->GetPropertyP(TARGETECCLASSID_PROPNAME);

            default:
                BeAssert(false && "ECSqlSystemPropertyInfo::Relation was changed. Need to adjust this code");
                return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ECN::PrimitiveType primitiveType) const
    {
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Binary:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "BinaryArray");
            case PRIMITIVETYPE_Boolean:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "BooleanArray");
            case PRIMITIVETYPE_DateTime:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "DateTimeArray");
            case PRIMITIVETYPE_Double:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "DoubleArray");
            case PRIMITIVETYPE_Integer:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "IntegerArray");
            case PRIMITIVETYPE_Long:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "LongArray");
            case PRIMITIVETYPE_Point2d:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "Point2dArray");
            case PRIMITIVETYPE_Point3d:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "Point3dArray");
            case PRIMITIVETYPE_String:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "StringArray");
            case PRIMITIVETYPE_IGeometry:
                return Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, "GeometryArray");
            default:
                BeAssert(false && "Unsupported primitive type. Adjust this method for new value of ECN::PrimitiveType enum");
                return nullptr;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2017
//+---------------+---------------+---------------+---------------+---------------+-
BentleyStatus ECDbSystemSchemaHelper::InitializeCache() const
    {
    std::vector<ECSqlSystemPropertyInfo> allSysPropInfos {
        ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECInstanceId), ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECClassId),
        ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId), ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECClassId),
        ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId), ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECClassId),
        ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::Id), ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::RelECClassId),
        ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::X), ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Y),ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Z)};

    for (ECSqlSystemPropertyInfo const& sysPropInfo : allSysPropInfos)
        {
        ECPropertyCP prop = nullptr;
        switch (sysPropInfo.GetType())
            {
                case ECSqlSystemPropertyInfo::Type::Class:
                    prop = GetSystemProperty(sysPropInfo.GetClass());
                    break;
                case ECSqlSystemPropertyInfo::Type::Relationship:
                    prop = GetSystemProperty(sysPropInfo.GetRelationship());
                    break;
                case ECSqlSystemPropertyInfo::Type::Navigation:
                    prop = GetSystemProperty(sysPropInfo.GetNavigation());
                    break;
                case ECSqlSystemPropertyInfo::Type::Point:
                    prop = GetSystemProperty(sysPropInfo.GetPoint());
                    break;
                default:
                BeAssert(false);
                return ERROR;
            }

        if (prop == nullptr || !prop->HasId())
            {
            BeAssert(false);
            return ERROR;
            }

        m_byPropIdCache[prop->GetId()] = sysPropInfo;
        }

    return SUCCESS;
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
int ECSqlSystemPropertyInfo::Compare(ECSqlSystemPropertyInfo const& lhs, ECSqlSystemPropertyInfo const& rhs)
    {
    if (lhs.m_type != rhs.m_type)
        return Enum::ToInt(lhs.m_type) - Enum::ToInt(rhs.m_type);

    switch (lhs.m_type)
        {
            case Type::Class:
                return Enum::ToInt(lhs.m_classKind) - Enum::ToInt(rhs.m_classKind);
            case Type::Relationship:
                return Enum::ToInt(lhs.m_relKind) - Enum::ToInt(rhs.m_relKind);
            case Type::Navigation:
                return Enum::ToInt(lhs.m_navKind) - Enum::ToInt(rhs.m_navKind);
            case Type::Point:
                return Enum::ToInt(lhs.m_pointKind) - Enum::ToInt(rhs.m_pointKind);
            default:
                BeAssert(false && "Type enum was changed. Code needs to be adjusted.");
                return -1;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECSqlSystemPropertyInfo::LessThan::operator()(ECSqlSystemPropertyInfo const& lhs, ECSqlSystemPropertyInfo const& rhs) const
    {
    return ECSqlSystemPropertyInfo::Compare(lhs, rhs) < 0;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
