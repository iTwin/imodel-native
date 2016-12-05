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
struct ConstraintECClassIdJoinInfo;

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    {
    private:
        struct ToSqlVisitor final : IPropertyMapVisitor
            {
            struct Result
                {
                private:
                    SingleColumnDataPropertyMap const* m_propertyMap;
                    NativeSqlBuilder m_sql;

                public:
                    Result() : m_propertyMap(nullptr) {}
                    explicit Result(SingleColumnDataPropertyMap const& propertyMap) :m_propertyMap(&propertyMap) {}

                    Utf8CP GetAccessString() const { return GetPropertyMap().GetAccessString().c_str(); }
                    SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
                    NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
                    NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
                    Utf8CP GetSql() const { return m_sql.ToString(); }
                    DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
                    DbTable const& GetTable() const { return GetColumn().GetTable(); }
                };

            private:
                Utf8CP m_classIdentifier;
                DbTable const& m_tableFilter;
                bool m_usePropertyNameAsAliasForSystemPropertyMaps;
                mutable bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_resultSetByAccessString;
                mutable std::vector<Result> m_resultSet;
                bool m_forECClassView;

                virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const& propertyMap) const override { return ToNativeSql(propertyMap); }
                virtual BentleyStatus _Visit(SystemPropertyMap const&) const override;

                BentleyStatus ToNativeSql(SingleColumnDataPropertyMap const&) const;
                BentleyStatus ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECInstanceIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECInstanceIdPropertyMap const&) const;
                Result& Record(SingleColumnDataPropertyMap const&) const;

            public:
                ToSqlVisitor(DbTable const& tableFilter, Utf8CP classIdentifier, bool usePropertyNameAsAliasForSystemPropertyMaps, bool forECClassViews);
                ~ToSqlVisitor() {}
                std::vector<Result> const& GetResultSet() const { return m_resultSet; }
                void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); }
            };

    private:
        ECDb const& m_ecdb;
        bool m_optimizeByIncludingOnlyRealTables;
        ECSqlPrepareContext const* m_prepareContext;
        bool m_isPolymorphic;
        bool m_captureViewColumnNameList;
        std::unique_ptr<std::vector<Utf8String>> m_viewColumnNameList;
        bool m_asSubQuery;

        explicit ViewGenerator(ECDb const& ecdb, bool cacheViewColumnNameList = false, bool asSubQuery = true) 
            : m_ecdb(ecdb), m_optimizeByIncludingOnlyRealTables(true), m_prepareContext(nullptr), m_asSubQuery(asSubQuery), m_captureViewColumnNameList(true)
            {
            if (cacheViewColumnNameList)
                m_viewColumnNameList = std::unique_ptr<std::vector<Utf8String>>(new std::vector<Utf8String>());
            }

        static BentleyStatus CreateECClassView(ECDbCR, ClassMapCR);

        /*=====================System Polymorphic view===================== */
        static BentleyStatus CreateUpdatableViewIfRequired(ECDbCR, ClassMap const&);
        static BentleyStatus GenerateUpdateTriggerSetClause(NativeSqlBuilder& sql, ClassMap const& baseClassMap, ClassMap const& derivedClassMap);
        BentleyStatus GenerateViewSql(NativeSqlBuilder& viewSql, ClassMap const&, bool isPolymorphicQuery, ECSqlPrepareContext const*);
        /*=====================NEW API=================*/
        bool IsECClassIdFilterEnabled() const;
        void RecordPropertyMapIfRequried(PropertyMap const& accessString);
        BentleyStatus RenderPropertyMaps(NativeSqlBuilder& sqlView, DbTable const*& requireJoinTo, ClassMapCR classMap, DbTable const& contextTable, ClassMapCP baseClass = nullptr, PropertyMap::Type filter = PropertyMap::Type::Entity, bool requireJoin = false);
        BentleyStatus RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, RelationshipClassEndTableMap const& relationMap);
        BentleyStatus RenderRelationshipClassMap(NativeSqlBuilder& viewSql, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const& sourceJoinInfo, ConstraintECClassIdJoinInfo const& targetJoinInfo, RelationshipClassLinkTableMap const* castInto = nullptr) ;
        BentleyStatus RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, RelationshipClassLinkTableMap const& relationMap);
        BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap);
        BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, ClassMap const& classMap, DbTable const& contextTable, ClassMapCP castAs = nullptr);
        BentleyStatus RenderNullView(NativeSqlBuilder& viewSql, ClassMap const& classMap);
    public:
        //! Generates a SQLite polymorphic SELECT query for a given classMap
        //! @param viewSql [out] Output SQL for view
        //! @param classMap [in] Source classMap for which to generate view
        //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
        //! @param prepareContext [in] prepareContext from ECSQL
        //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
        static BentleyStatus GenerateSelectViewSql(NativeSqlBuilder& viewSql, ECDb const&, ClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext);
        static BentleyStatus CreateUpdatableViews(ECDbCR);
        static BentleyStatus DropUpdatableViews(ECDbCR);
        static BentleyStatus CreateECClassViews(ECDbCR, bvector<ECN::ECClassId> const&);
        static BentleyStatus CreateECClassViews(ECDbCR);
        static BentleyStatus DropECClassViews(ECDbCR);
    };

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       11/2016
+===============+===============+===============+===============+===============+======*/
struct ConstraintECClassIdJoinInfo
    {
    private:
        bool m_joinIsRequired;
        DbColumn const* m_primaryECInstanceIdCol;
        DbColumn const* m_primaryECClassIdCol;
        DbColumn const* m_foreignECInstanceIdCol;
        ConstraintECClassIdPropertyMap const* m_propertyMap;
        ConstraintECClassIdJoinInfo() : m_joinIsRequired(false), m_primaryECInstanceIdCol(nullptr), m_primaryECClassIdCol(nullptr), m_foreignECInstanceIdCol(nullptr), m_propertyMap(nullptr) {}
        Utf8CP GetSqlTableAlias()const;
        Utf8CP GetSqlECClassIdColumnAlias()const;
  
    public:

        static ConstraintECClassIdJoinInfo Create(ConstraintECClassIdPropertyMap const& propertyMap, DbTable const& contextTable);
        ~ConstraintECClassIdJoinInfo() {}

        bool RequiresJoin() const { return m_joinIsRequired; }
        ConstraintECClassIdPropertyMap const& GetConstraintECClassIdPropMap() const { BeAssert(RequiresJoin()); return *m_propertyMap; }
        DbColumn const& GetPrimaryECInstanceIdColumn() const { BeAssert(RequiresJoin()); return *m_primaryECClassIdCol; }
        DbColumn const& GetPrimaryECClassIdColumn() const { BeAssert(RequiresJoin()); return *m_primaryECClassIdCol; }
        DbColumn const& GetForignECInstanceIdColumn() const { BeAssert(RequiresJoin()); return *m_foreignECInstanceIdCol; }
        NativeSqlBuilder GetNativeConstraintECClassIdSql(bool appendAlias) const;
        NativeSqlBuilder GetNativeJoinSql() const;
        static DbTable const* RequiresJoinTo(ConstraintECClassIdPropertyMap const& propertyMap, bool ignoreVirtualColumnCheck = false);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
