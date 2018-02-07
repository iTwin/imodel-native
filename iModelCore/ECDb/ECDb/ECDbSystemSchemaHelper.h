/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSystemSchemaHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSCHEMA_ECDbSystem "ECDbSystem"
#define ECSCHEMA_ALIAS_ECDbSystem "ecdbsys"

#define ECDBSYS_CLASS_ClassECSqlSystemProperties "ClassECSqlSystemProperties"
#define ECDBSYS_CLASS_RelationshipECSqlSystemProperties "RelationshipECSqlSystemProperties"
#define ECDBSYS_CLASS_PointECSqlSystemProperties "PointECSqlSystemProperties"
#define ECDBSYS_CLASS_NavigationECSqlSystemProperties "NavigationECSqlSystemProperties"

#define ECDBSYS_PROP_ECInstanceId "ECInstanceId"
#define ECDBSYS_PROPALIAS_Id "Id"

#define ECDBSYS_PROP_ECClassId "ECClassId"
#define ECDBSYS_PROP_SourceECInstanceId "SourceECInstanceId"
#define ECDBSYS_PROPALIAS_SourceId "SourceId"
#define ECDBSYS_PROP_SourceECClassId "SourceECClassId"
#define ECDBSYS_PROP_TargetECInstanceId "TargetECInstanceId"
#define ECDBSYS_PROPALIAS_TargetId "TargetId"
#define ECDBSYS_PROP_TargetECClassId "TargetECClassId"
#define ECDBSYS_PROP_NavPropId "Id"
#define ECDBSYS_PROP_NavPropRelECClassId "RelECClassId"
#define ECDBSYS_PROP_PointX "X"
#define ECDBSYS_PROP_PointY "Y"
#define ECDBSYS_PROP_PointZ "Z"


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2016
//+===============+===============+===============+===============+===============+======
struct ECSqlSystemPropertyInfo final
    {
public:
    enum class Type
        {
        None,
        Class,
        Relationship,
        Point,
        Navigation
        };

    enum class Class
        {
        ECInstanceId,
        ECClassId
        };

    enum class Relationship
        {
        SourceECInstanceId,
        SourceECClassId,
        TargetECInstanceId,
        TargetECClassId
        };

    enum class Point
        {
        X, Y, Z
        };

    enum class Navigation
        {
        Id, RelECClassId
        };

    struct LessThan
        {
        bool operator()(ECSqlSystemPropertyInfo const& lhs, ECSqlSystemPropertyInfo const& rhs) const
            {
            return ECSqlSystemPropertyInfo::Compare(lhs, rhs) < 0;
            }
        };
private:
    static const ECSqlSystemPropertyInfo s_noSystemProperty;
    static const ECSqlSystemPropertyInfo s_ecinstanceId;
    static const ECSqlSystemPropertyInfo s_ecclassid;
    static const ECSqlSystemPropertyInfo s_sourceECInstanceId;
    static const ECSqlSystemPropertyInfo s_sourceECClassId;
    static const ECSqlSystemPropertyInfo s_targetECInstanceId;
    static const ECSqlSystemPropertyInfo s_targetECClassId;
    static const ECSqlSystemPropertyInfo s_navigationId;
    static const ECSqlSystemPropertyInfo s_navigationRelECClassId;
    static const ECSqlSystemPropertyInfo s_pointX;
    static const ECSqlSystemPropertyInfo s_pointY;
    static const ECSqlSystemPropertyInfo s_pointZ;

    Type m_type;
    union
        {
        Class m_classKind;
        Relationship m_relKind;
        Point m_pointKind;
        Navigation m_navKind;
        };

    ECSqlSystemPropertyInfo() : m_type(Type::None) {}
    explicit ECSqlSystemPropertyInfo(Class kind) : m_type(Type::Class), m_classKind(kind) {}
    explicit ECSqlSystemPropertyInfo(Relationship kind) : m_type(Type::Relationship), m_relKind(kind) {}
    explicit ECSqlSystemPropertyInfo(Point kind) : m_type(Type::Point), m_pointKind(kind) {}
    explicit ECSqlSystemPropertyInfo(Navigation kind) : m_type(Type::Navigation), m_navKind(kind) {}

    static int Compare(ECSqlSystemPropertyInfo const& lhs, ECSqlSystemPropertyInfo const& rhs);

public:
    bool operator==(ECSqlSystemPropertyInfo const& rhs) const { return Compare(*this, rhs) == 0; }
    bool operator!=(ECSqlSystemPropertyInfo const& rhs) const { return !(*this == rhs); }
    
    bool IsSystemProperty() const { return m_type != Type::None; }
    Type GetType() const { return m_type; }

    Class GetClass() const { BeAssert(m_type == Type::Class); return m_classKind; }
    Relationship GetRelationship() const { BeAssert(m_type == Type::Relationship); return m_relKind; }
    Point GetPoint() const { BeAssert(m_type == Type::Point); return m_pointKind; }
    Navigation GetNavigation() const { BeAssert(m_type == Type::Navigation); return m_navKind; }
    //Indicates whether the system property is of an id type
    bool IsId() const { return IsSystemProperty() && m_type != Type::Point; }

    static ECSqlSystemPropertyInfo const& ECInstanceId() { return s_ecinstanceId; }
    static ECSqlSystemPropertyInfo const& ECClassId() { return s_ecclassid; }
    static ECSqlSystemPropertyInfo const& SourceECInstanceId() { return s_sourceECInstanceId; }
    static ECSqlSystemPropertyInfo const& SourceECClassId() { return s_sourceECClassId; }
    static ECSqlSystemPropertyInfo const& TargetECInstanceId() { return s_targetECInstanceId; }
    static ECSqlSystemPropertyInfo const& TargetECClassId() { return s_targetECClassId; }
    static ECSqlSystemPropertyInfo const& NavigationId() { return s_navigationId; }
    static ECSqlSystemPropertyInfo const& NavigationRelECClassId() { return s_navigationRelECClassId; }
    static ECSqlSystemPropertyInfo const& PointX() { return s_pointX; }
    static ECSqlSystemPropertyInfo const& PointY() { return s_pointY; }
    static ECSqlSystemPropertyInfo const& PointZ() { return s_pointZ; }
    static ECSqlSystemPropertyInfo const& NoSystemProperty() { return s_noSystemProperty; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
struct ECDbSystemSchemaHelper final
    {
    public:
        ECDb const& m_ecdb;
        mutable bmap<ECN::ECPropertyId, ECSqlSystemPropertyInfo const*> m_byPropIdCache;

        //not copyable
        ECDbSystemSchemaHelper(ECDbSystemSchemaHelper const&) = delete;
        ECDbSystemSchemaHelper& operator=(ECDbSystemSchemaHelper const&) = delete;

        BentleyStatus InitializeCache() const;

        SchemaManager const& Schemas() const { return m_ecdb.Schemas(); }

    public:
        explicit ECDbSystemSchemaHelper(ECDb const& ecdb) : m_ecdb(ecdb) {}

        //! @return System property or nullptr in case of errors
        ECN::ECPropertyCP GetSystemProperty(ECSqlSystemPropertyInfo const&) const;
        ECSqlSystemPropertyInfo const& GetSystemPropertyInfo(ECN::ECPropertyCR) const;
        void ClearCache() const { m_byPropIdCache.clear(); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
