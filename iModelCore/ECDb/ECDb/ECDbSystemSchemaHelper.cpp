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
// @bsimethod                                 Krischan.Eberle                01/2017
//+---------------+---------------+---------------+---------------+---------------+-
//static member variable initialization
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_noSystemProperty;
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_ecinstanceid = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECInstanceId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_ecclassid = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECClassId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_sourceECInstanceId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_sourceECClassId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECClassId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_targetECInstanceId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_targetECClassId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECClassId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_navigationId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::Id);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_navigationRelECClassId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::RelECClassId);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_pointX = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::X);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_pointY = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Y);
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_pointZ = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Z);


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
ECSqlSystemPropertyInfo const& ECDbSystemSchemaHelper::GetSystemPropertyInfo(ECN::ECPropertyCR ecProperty) const
    {
    if (!ecProperty.HasId() || SUCCESS != InitializeCache())
        {
        BeAssert(false);
        return ECSqlSystemPropertyInfo::NoSystemProperty();
        }

    auto it = m_byPropIdCache.find(ecProperty.GetId());
    if (it == m_byPropIdCache.end())
        return ECSqlSystemPropertyInfo::NoSystemProperty();

    return *it->second;
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



//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2017
//+---------------+---------------+---------------+---------------+---------------+-
BentleyStatus ECDbSystemSchemaHelper::InitializeCache() const
    {
    if (!m_byPropIdCache.empty())
        return SUCCESS;

    std::vector<ECSqlSystemPropertyInfo const*> allSysPropInfos {
        &ECSqlSystemPropertyInfo::ECInstanceId(), &ECSqlSystemPropertyInfo::ECClassId(),
        &ECSqlSystemPropertyInfo::SourceECInstanceId(), &ECSqlSystemPropertyInfo::SourceECClassId(),
        &ECSqlSystemPropertyInfo::TargetECInstanceId(), &ECSqlSystemPropertyInfo::TargetECClassId(),
        &ECSqlSystemPropertyInfo::NavigationId(), &ECSqlSystemPropertyInfo::NavigationRelECClassId(),
        &ECSqlSystemPropertyInfo::PointX(), &ECSqlSystemPropertyInfo::PointY(), &ECSqlSystemPropertyInfo::PointZ()};

    for (ECSqlSystemPropertyInfo const* sysPropInfo : allSysPropInfos)
        {
        ECPropertyCP prop = nullptr;
        switch (sysPropInfo->GetType())
            {
                case ECSqlSystemPropertyInfo::Type::Class:
                    prop = GetSystemProperty(sysPropInfo->GetClass());
                    break;
                case ECSqlSystemPropertyInfo::Type::Relationship:
                    prop = GetSystemProperty(sysPropInfo->GetRelationship());
                    break;
                case ECSqlSystemPropertyInfo::Type::Navigation:
                    prop = GetSystemProperty(sysPropInfo->GetNavigation());
                    break;
                case ECSqlSystemPropertyInfo::Type::Point:
                    prop = GetSystemProperty(sysPropInfo->GetPoint());
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

END_BENTLEY_SQLITE_EC_NAMESPACE
