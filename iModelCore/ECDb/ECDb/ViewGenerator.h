/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlPrepareContext;

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    {
    private:
        struct ViewMember
            {
            private:
                std::vector<ClassMap const*> m_classMaps;
                DbSchema::EntityType m_storageType;
            public:
                ViewMember() :m_storageType(DbSchema::EntityType::Table) {}
                ViewMember(DbSchema::EntityType storageType, ClassMap const& classMap) :m_storageType(storageType) { m_classMaps.push_back(&classMap); }

                void AddClassMap(ClassMap const& classMap) { m_classMaps.push_back(&classMap); }
                std::vector<ClassMap const*> const& GetClassMaps() const { return m_classMaps; }
                DbSchema::EntityType GetStorageType() const { return m_storageType; }
            };

        typedef bmap<DbTable const*, ViewMember> ViewMemberByTable;

        ECDbMap const& m_map;
        bool m_optimizeByIncludingOnlyRealTables;
        ECSqlPrepareContext const* m_prepareContext;
        bool m_isPolymorphic;
        std::unique_ptr<std::vector<Utf8String>> m_viewAccessStringList;
        bool m_asSubQuery;
        bool m_captureViewAccessStringList;

        explicit ViewGenerator(ECDbMap const& map, bool returnViewAccessStringList = false, bool asSubQuery = true) :
            m_map(map), m_optimizeByIncludingOnlyRealTables(true), m_prepareContext(nullptr), m_asSubQuery(asSubQuery), m_captureViewAccessStringList(true)
            {
            if (returnViewAccessStringList)
                m_viewAccessStringList = decltype(m_viewAccessStringList)(new std::vector<Utf8String>());
            }

        static BentleyStatus CreateECClassView(ClassMapCR);
        static BentleyStatus CreateUpdatableViewIfRequired(ECDbCR, ClassMap const&);
        BentleyStatus GenerateViewSql(NativeSqlBuilder& viewSql, ClassMap const&, bool isPolymorphicQuery, ECSqlPrepareContext const*);

        BentleyStatus ComputeViewMembers(ViewMemberByTable& viewMembers, ECN::ECClassCR, bool ensureDerivedClassesAreLoaded);
        BentleyStatus GetRootClasses(std::vector<ClassMap const*>& rootClasses, ECDbCR);
        BentleyStatus GetViewQueryForChild(NativeSqlBuilder& viewSql, DbTable const&, const std::vector<ClassMap const*>& childClassMap, ClassMap const& baseClassMap);
        //! Relationship polymorphic query
        BentleyStatus CreateViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap);
        BentleyStatus CreateViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap, ClassMap const& baseClassMap);
        BentleyStatus CreateViewForRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR, ClassMap const& baseClassMap);
        BentleyStatus CreateViewForRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassEndTableMap const&, ClassMap const& baseClassMap);
        BentleyStatus CreateNullViewForRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR, ClassMap const& baseClassMap);
        BentleyStatus CreateNullViewForRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassMapCR, ClassMap const& baseClassMap);
        BentleyStatus CreateNullViewForRelationship(NativeSqlBuilder& viewSql, ClassMap const& relationMap, ClassMap const& baseClassMap);
        BentleyStatus CreateNullView(NativeSqlBuilder& viewSql, ClassMap const&);
        Utf8CP GetECClassIdPrimaryTableAlias(ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }

        BentleyStatus BuildRelationshipJoinIfAny(NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, DbTable const& contextTable);
        //! Append view prop map list separated by comma.
        BentleyStatus AppendViewPropMapsToQuery(NativeSqlBuilder& viewSql, DbTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView = false);

        BentleyStatus AppendSystemPropMaps(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, DbTable const& contextTable);
        BentleyStatus AppendSystemPropMapsToNullView(NativeSqlBuilder& viewSql, RelationshipClassMapCR relationMap, bool endWithComma);
        BentleyStatus AppendConstraintClassIdPropMap(NativeSqlBuilder& viewSql, RelationshipConstraintPropertyMap const& propMap, RelationshipClassMapCR relationMap, ECN::ECRelationshipConstraintCR constraint, DbTable const& contextTable);

        //! Return prop maps of child base on parent map. So only prop maps that make up baseClass properties are selected.
        BentleyStatus GetPropertyMapsOfDerivedClassCastAsBaseClass(std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ClassMap const& baseClassMap, ClassMap const& childClassMap, bool skipSystemProperties);

        static BentleyStatus GenerateUpdateTriggerSetClause(NativeSqlBuilder& sql, ClassMap const& baseClassMap, ClassMap const& derivedClassMap);

    public:
        //! Generates a SQLite polymorphic SELECT query for a given classMap
        //! @param viewSql [out] Output SQL for view
        //! @param classMap [in] Source classMap for which to generate view
        //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
        //! @param prepareContext [in] prepareContext from ECSQL
        //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
        static BentleyStatus GenerateSelectViewSql(NativeSqlBuilder& viewSql, ClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext);

        static BentleyStatus CreateUpdatableViews(ECDbCR ecdb);
        static BentleyStatus DropUpdatableViews(ECDbCR ecdb);

        static BentleyStatus CreateECClassViews(ECDbCR ecdb);
        static BentleyStatus DropECClassViews(ECDbCR ecdb);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
