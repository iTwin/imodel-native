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

#define ECDBSYSTEM_SCHEMANAME "ECDbSystem"

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
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo const& sysPropInfo) const
    {
    ECClassCP systemClass = nullptr;
    Utf8CP propName = nullptr;

    switch (sysPropInfo.GetType())
        {
            case ECSqlSystemPropertyInfo::Type::Class:
            {
            systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME);
            switch (sysPropInfo.GetClass())
                {
                    case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                        propName = ECINSTANCEID_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Class::ECClassId:
                        propName = ECCLASSID_PROPNAME;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Relationship:
            {
            systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, REL_ECSQLSYSTEMPROPERTIES_CLASSNAME);
            switch (sysPropInfo.GetRelationship())
                {
                    case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                        propName = SOURCEECINSTANCEID_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                        propName = SOURCEECCLASSID_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                        propName = TARGETECINSTANCEID_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                        propName = TARGETECCLASSID_PROPNAME;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Navigation:
            {
            systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME);
            switch (sysPropInfo.GetNavigation())
                {
                    case ECSqlSystemPropertyInfo::Navigation::Id:
                        propName = NAVPROP_ID_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                        propName = NAVPROP_RELECCLASSID_PROPNAME;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Point:
            {
            systemClass = Schemas().GetECClass(ECDBSYSTEM_SCHEMANAME, POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME);
            switch (sysPropInfo.GetPoint())
                {
                    case ECSqlSystemPropertyInfo::Point::X:
                        propName = POINTPROP_X_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Point::Y:
                        propName = POINTPROP_Y_PROPNAME;
                        break;
                    case ECSqlSystemPropertyInfo::Point::Z:
                        propName = POINTPROP_Z_PROPNAME;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            default:
                BeAssert(false);
                return nullptr;
        }

    if (systemClass == nullptr || propName == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    return systemClass->GetPropertyP(propName);
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
        ECPropertyCP prop = GetSystemProperty(*sysPropInfo);
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

END_BENTLEY_SQLITE_EC_NAMESPACE
