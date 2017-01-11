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
bool ECDbSystemSchemaHelper::TryGetSystemPropertyInfo(ECSqlSystemPropertyInfo& info, ECN::ECPropertyCR ecProperty) const
    {
    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME) == ecProperty.GetClass().GetId())
        {
        if (PropertyNameEquals(ecProperty, ECSqlSystemPropertyInfo::Class::ECInstanceId))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECInstanceId);
            return true;
            }

        if (PropertyNameEquals(ecProperty, ECSqlSystemPropertyInfo::Class::ECClassId))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Class::ECClassId);
            return true;
            }
        }

    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME) == ecProperty.GetClass().GetId())
        {
        if (ecProperty.GetName().EqualsIAscii(NAVPROP_ID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::Id);
            return true;
            }

        if (ecProperty.GetName().EqualsIAscii(NAVPROP_RELECCLASSID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Navigation::RelECClassId);
            return true;
            }
        }

    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, REL_ECSQLSYSTEMPROPERTIES_CLASSNAME) == ecProperty.GetClass().GetId())
        {
        if (ecProperty.GetName().EqualsIAscii(SOURCEECINSTANCEID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId);
            return true;
            }

        if (ecProperty.GetName().EqualsIAscii(SOURCEECCLASSID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::SourceECClassId);
            return true;
            }

        if (ecProperty.GetName().EqualsIAscii(TARGETECINSTANCEID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId);
            return true;
            }

        if (ecProperty.GetName().EqualsIAscii(TARGETECCLASSID_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Relationship::TargetECClassId);
            return true;
            }

        }

    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME) == ecProperty.GetClass().GetId())
        {
        if (ecProperty.GetName().EqualsIAscii(POINTPROP_X_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::X);
            return true;
            }

        if (ecProperty.GetName().EqualsIAscii(POINTPROP_Y_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Y);
            return true;
            }
        
        if (ecProperty.GetName().EqualsIAscii(POINTPROP_Z_PROPNAME))
            {
            info = ECSqlSystemPropertyInfo(ECSqlSystemPropertyInfo::Point::Z);
            return true;
            }
        }

    info = ECSqlSystemPropertyInfo();
    return false;
    }


//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
bool ECDbSystemSchemaHelper::Equals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Class sysProp) const
    {
    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME) == prop.GetClass().GetId())
        return PropertyNameEquals(prop, sysProp);

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
bool ECDbSystemSchemaHelper::Equals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Relationship sysProp) const
    {
    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, REL_ECSQLSYSTEMPROPERTIES_CLASSNAME) == prop.GetClass().GetId())
        return PropertyNameEquals(prop, sysProp);

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
bool ECDbSystemSchemaHelper::Equals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Navigation sysProp) const
    {
    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME) == prop.GetClass().GetId())
        return PropertyNameEquals(prop, sysProp);

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
bool ECDbSystemSchemaHelper::Equals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Point sysProp) const
    {
    if (m_schemaManager.GetECClassId(ECDBSYSTEM_SCHEMANAME, POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME) == prop.GetClass().GetId())
        return PropertyNameEquals(prop, sysProp);

    return false;
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::PropertyNameEquals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Class sysProp)
    {
    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                return prop.GetName().EqualsIAscii(ECINSTANCEID_PROPNAME);
            case ECSqlSystemPropertyInfo::Class::ECClassId:
                return prop.GetName().EqualsIAscii(ECCLASSID_PROPNAME);

            default:
                BeAssert(false);
                return false;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::PropertyNameEquals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Relationship sysProp)
    {
    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                return prop.GetName().EqualsIAscii(SOURCEECINSTANCEID_PROPNAME);
            case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                return prop.GetName().EqualsIAscii(SOURCEECCLASSID_PROPNAME);
            case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                return prop.GetName().EqualsIAscii(TARGETECINSTANCEID_PROPNAME);
            case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                return prop.GetName().EqualsIAscii(TARGETECCLASSID_PROPNAME);

            default:
                BeAssert(false);
                return false;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::PropertyNameEquals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Navigation sysProp)
    {
    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Navigation::Id:
                return prop.GetName().EqualsIAscii(NAVPROP_ID_PROPNAME);
            case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                return prop.GetName().EqualsIAscii(NAVPROP_RELECCLASSID_PROPNAME);

            default:
                BeAssert(false);
                return false;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
//static 
bool ECDbSystemSchemaHelper::PropertyNameEquals(ECN::ECPropertyCR prop, ECSqlSystemPropertyInfo::Point sysProp)
    {
    switch (sysProp)
        {
            case ECSqlSystemPropertyInfo::Point::X:
                return prop.GetName().EqualsIAscii(POINTPROP_X_PROPNAME);
            case ECSqlSystemPropertyInfo::Point::Y:
                return prop.GetName().EqualsIAscii(POINTPROP_Y_PROPNAME);
            case ECSqlSystemPropertyInfo::Point::Z:
                return prop.GetName().EqualsIAscii(POINTPROP_Z_PROPNAME);

            default:
                BeAssert(false);
                return false;
        }
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Class classSysProp) const
    {
    ECClassCP systemClass = m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, CLASS_ECSQLSYSTEMPROPERTIES_CLASSNAME);
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
//static 
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Relationship sysProp) const
    {
    ECClassCP systemClass = m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, REL_ECSQLSYSTEMPROPERTIES_CLASSNAME);
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
// @bsimethod                                 Krischan.Eberle                12/2016
//+---------------+---------------+---------------+---------------+---------------+-
ECPropertyCP ECDbSystemSchemaHelper::GetSystemProperty(ECSqlSystemPropertyInfo::Navigation sysProp) const
    {
    ECClassCP systemClass = m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, NAVIGATION_ECSQLSYSTEMPROPERTIES_CLASSNAME);
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
    ECClassCP systemClass = m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, POINT_ECSQLSYSTEMPROPERTIES_CLASSNAME);
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



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCP ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence(ECN::PrimitiveType primitiveType) const
    {
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Binary:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "BinaryArray");
            case PRIMITIVETYPE_Boolean:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "BooleanArray");
            case PRIMITIVETYPE_DateTime:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "DateTimeArray");
            case PRIMITIVETYPE_Double:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "DoubleArray");
            case PRIMITIVETYPE_Integer:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "IntegerArray");
            case PRIMITIVETYPE_Long:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "LongArray");
            case PRIMITIVETYPE_Point2d:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "Point2dArray");
            case PRIMITIVETYPE_Point3d:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "Point3dArray");
            case PRIMITIVETYPE_String:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "StringArray");
            case PRIMITIVETYPE_IGeometry:
                return m_schemaManager.GetECClass(ECDBSYSTEM_SCHEMANAME, "GeometryArray");
            default:
                BeAssert(false && "Unsupported primitive type. Adjust this method for new value of ECN::PrimitiveType enum");
                return nullptr;
        }
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
