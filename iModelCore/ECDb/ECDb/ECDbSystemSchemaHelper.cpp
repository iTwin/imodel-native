/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
//static member variable initialization
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_noSystemProperty;
const ECSqlSystemPropertyInfo ECSqlSystemPropertyInfo::s_ecinstanceId = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECInstanceId);
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo const& sysPropInfo) const
    {
    ECClassCP systemClass = nullptr;
    Utf8CP propName = nullptr;

    switch (sysPropInfo.GetType())
        {
            case ECSqlSystemPropertyInfo::Type::Class:
            {
            systemClass = Schemas().GetClass(ECSCHEMA_ECDbSystem, ECDBSYS_CLASS_ClassECSqlSystemProperties);
            switch (sysPropInfo.GetClass())
                {
                    case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                        propName = ECDBSYS_PROP_ECInstanceId;
                        break;
                    case ECSqlSystemPropertyInfo::Class::ECClassId:
                        propName = ECDBSYS_PROP_ECClassId;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Relationship:
            {
            systemClass = Schemas().GetClass(ECSCHEMA_ECDbSystem, ECDBSYS_CLASS_RelationshipECSqlSystemProperties);
            switch (sysPropInfo.GetRelationship())
                {
                    case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                        propName = ECDBSYS_PROP_SourceECInstanceId;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                        propName = ECDBSYS_PROP_SourceECClassId;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                        propName = ECDBSYS_PROP_TargetECInstanceId;
                        break;
                    case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                        propName = ECDBSYS_PROP_TargetECClassId;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Navigation:
            {
            systemClass = Schemas().GetClass(ECSCHEMA_ECDbSystem, ECDBSYS_CLASS_NavigationECSqlSystemProperties);
            switch (sysPropInfo.GetNavigation())
                {
                    case ECSqlSystemPropertyInfo::Navigation::Id:
                        propName = ECDBSYS_PROP_NavPropId;
                        break;
                    case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                        propName = ECDBSYS_PROP_NavPropRelECClassId;
                        break;

                    default:
                        BeAssert(false);
                        return nullptr;
                }
            break;
            }

            case ECSqlSystemPropertyInfo::Type::Point:
            {
            systemClass = Schemas().GetClass(ECSCHEMA_ECDbSystem, ECDBSYS_CLASS_PointECSqlSystemProperties);
            switch (sysPropInfo.GetPoint())
                {
                    case ECSqlSystemPropertyInfo::Point::X:
                        propName = ECDBSYS_PROP_PointX;
                        break;
                    case ECSqlSystemPropertyInfo::Point::Y:
                        propName = ECDBSYS_PROP_PointY;
                        break;
                    case ECSqlSystemPropertyInfo::Point::Z:
                        propName = ECDBSYS_PROP_PointZ;
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ExtendedTypeHelper::ExtendedType ExtendedTypeHelper::GetExtendedType(Utf8StringCR extendedType) {
    static std::map<Utf8String, ExtendedType> map {
        { EXTENDEDTYPENAME_Id, ExtendedType::Id },
        { EXTENDEDTYPENAME_ClassId, ExtendedType::ClassId },
        { EXTENDEDTYPENAME_SourceId, ExtendedType::SourceId },
        { EXTENDEDTYPENAME_SourceClassId, ExtendedType::SourceClassId },
        { EXTENDEDTYPENAME_TargetId, ExtendedType::TargetId },
        { EXTENDEDTYPENAME_TargetClassId, ExtendedType::TargetClassId },
        { EXTENDEDTYPENAME_NavId, ExtendedType::NavId },
        { EXTENDEDTYPENAME_NavRelClassId, ExtendedType::NavRelClassId },
    };
    auto it  = map.find(extendedType);
    if (it == map.end())
        return ExtendedType::Unknown;

    return it->second;
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool ExtendedTypeHelper::TryGetSystemExtendedType(Utf8StringR extenedType, Utf8StringCR className, Utf8StringCR propertyName) {
    static std::map<Utf8String, std::map<Utf8String, Utf8String>> map {
        { 
            ECDBSYS_CLASS_ClassECSqlSystemProperties, {
                { ECDBSYS_PROP_ECInstanceId, EXTENDEDTYPENAME_Id}, 
                { ECDBSYS_PROP_ECClassId, EXTENDEDTYPENAME_ClassId}
            } 
        }, { 
            ECDBSYS_CLASS_RelationshipECSqlSystemProperties, {
                { ECDBSYS_PROP_SourceECInstanceId, EXTENDEDTYPENAME_SourceId}, 
                { ECDBSYS_PROP_SourceECClassId, EXTENDEDTYPENAME_SourceClassId},
                { ECDBSYS_PROP_TargetECInstanceId, EXTENDEDTYPENAME_TargetId}, 
                { ECDBSYS_PROP_TargetECClassId, EXTENDEDTYPENAME_TargetClassId}
            } 
        }, { 
            ECDBSYS_CLASS_NavigationECSqlSystemProperties, {
                { ECDBSYS_PROP_NavPropId, EXTENDEDTYPENAME_NavId},
                { ECDBSYS_PROP_NavPropRelECClassId, EXTENDEDTYPENAME_NavRelClassId}
            }
        }, 
    };
    auto classIt  = map.find(className);
    if (classIt == map.end())
        return false;
    
    auto& props = classIt->second;
    auto propIt = props.find(propertyName);
    if (propIt == props.end())
        return false;

    extenedType = propIt->second;
    return true;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
