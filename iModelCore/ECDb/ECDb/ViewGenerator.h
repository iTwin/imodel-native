/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $ 
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct ViewGenerator final
    {
    private:
        enum class ViewType
            {
            SelectFromView,
            ECClassView
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    12/2016
        //+===============+===============+===============+===============+===============+======
        struct Context
            {
        private:
            ViewType m_viewType;
            ECDbCR m_ecdb;

        protected:
            Context(ViewType viewType, ECDbCR ecdb) : m_viewType(viewType), m_ecdb(ecdb) {}

        public:
            virtual ~Context() {}

            ViewType GetViewType() const { return m_viewType; }
            ECDbCR GetECDb() const { return m_ecdb; }

            template<typename TContext>
            TContext& GetAs() { BeAssert(dynamic_cast<TContext*> (this) != nullptr); return static_cast<TContext&> (*this); }
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    12/2016
        //+===============+===============+===============+===============+===============+======
        struct SelectFromViewContext final : Context
            {
        private:
            ECSqlPrepareContext const& m_prepareCtx;
            bool m_isPolymorphicQuery;

        public:
            SelectFromViewContext(ECSqlPrepareContext const&, bool isPolymorphicQuery);
            ~SelectFromViewContext() {}

            ECSqlPrepareContext const& GetPrepareCtx() const { return m_prepareCtx; }
            bool IsPolymorphicQuery() const { return m_isPolymorphicQuery; }
            void SetPolymorphicQuery(bool isPolymorphic) { m_isPolymorphicQuery = isPolymorphic; }
            bool IsECClassIdFilterEnabled() const;
            bool IsInSelectClause(Utf8StringCR exp) const;
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    12/2016
        //+===============+===============+===============+===============+===============+======
        struct ECClassViewContext final : Context
            {
            private:
                bvector<Utf8StringCP> m_viewColumnNameList;
                bool m_captureViewColumnNames;
            public:
                explicit ECClassViewContext(ECDbCR ecdb) : Context(ViewType::ECClassView, ecdb), m_captureViewColumnNames(true) {}
                ~ECClassViewContext() {}

                bool MustCaptureViewColumnNames() const { return m_captureViewColumnNames; }
                void StopCaptureViewColumnNames() { m_captureViewColumnNames = false; }
                void AddViewColumnName(Utf8StringCR propAccessString) { BeAssert(MustCaptureViewColumnNames()); if (!MustCaptureViewColumnNames()) return;  BeAssert(!propAccessString.empty()); m_viewColumnNameList.push_back(&propAccessString); }

                bvector<Utf8StringCP> const& GetViewColumnNames() const { return m_viewColumnNameList; }
                
            };

        struct ToSqlVisitor final : IPropertyMapVisitor
            {
                enum class ColumnAliasMode
                    {
                    NoAlias = 0,
                    SystemPropertyName
                    };


                struct Result
                    {
                    private:
                        SingleColumnDataPropertyMap const* m_propertyMap = nullptr;
                        NativeSqlBuilder m_sql;
                        bool m_isLiteralSqlSnippet = false;

                    public:
                        Result() {}
                        explicit Result(SingleColumnDataPropertyMap const& propertyMap) :m_propertyMap(&propertyMap) {}

                        SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
                        DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
                        NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
                        NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
                        //indicates whether the added SQL snippet is a literal or a col name.
                        //This is necessary for calling code to determine whether it has to add a col alias or not
                        bool IsLiteralSqlSnippet() const { return m_isLiteralSqlSnippet; }
                        void SetIsLiteralSqlSnippet() { m_isLiteralSqlSnippet = true; }
                    };

            private:
                Context const& m_context;
                Utf8CP m_classIdentifier;
                DbTable const& m_tableFilter;
                ColumnAliasMode m_columnAliasMode = ColumnAliasMode::NoAlias;
                mutable bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_resultSetByAccessString;
                mutable std::vector<Result> m_resultSet;
                bool m_doNotAddColumnAliasForComputedExpression;
                BentleyStatus _Visit(SingleColumnDataPropertyMap const& propertyMap) const override;
                BentleyStatus _Visit(SystemPropertyMap const&) const override;

                BentleyStatus ToNativeSql(SingleColumnDataPropertyMap const&) const;
                BentleyStatus ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECInstanceIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECInstanceIdPropertyMap const&) const;
                Result& Record(SingleColumnDataPropertyMap const&) const;

            public:
                ToSqlVisitor(Context const& ctx, DbTable const& tableFilter, Utf8CP classIdentifier, ColumnAliasMode);
                ~ToSqlVisitor() {}
                std::vector<Result> const& GetResultSet() const { return m_resultSet; }
                void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); }
                void DoNotAddColumnAliasForComputedExpression() { m_doNotAddColumnAliasForComputedExpression = true; }
            };

        ViewGenerator();
        ~ViewGenerator();

        static BentleyStatus CreateECClassView(ECDbCR, ClassMapCR);

        static BentleyStatus GenerateViewSql(NativeSqlBuilder& viewSql, Context&, ClassMap const&);

        static BentleyStatus RenderPropertyMaps(NativeSqlBuilder& sqlView, Context&, bset<DbTable const*>& requireJoinTo, ClassMapCR classMap, DbTable const& contextTable, ClassMapCP baseClass, PropertyMap::Type filter, bool requireJoin = false);
        static BentleyStatus RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassEndTableMap const& relationMap);
        static BentleyStatus RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassLinkTableMap const& relationMap);
        static BentleyStatus DoRenderRelationshipClassMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const& sourceJoinInfo, ConstraintECClassIdJoinInfo const& targetJoinInfo, RelationshipClassLinkTableMap const* castInto = nullptr);
        static BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap, DbTable const& contextTable, ClassMapCP castAs = nullptr);
        static BentleyStatus RenderNullView(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderMixinClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderMixinClassMap(bmap<Utf8String, bpair<DbTable const*, bvector<ECN::ECClassId>>, CompareIUtf8Ascii>& selectClauses, Context& ctx, ClassMap const& mixInClassMap, ClassMap const& derivedClassMap);
        static BentleyStatus GenerateECClassIdFilter(Utf8StringR filterSqlExpression, ClassMap const&, DbTable const&, DbColumn const& classIdColumn, bool polymorphic);

    public:
        //! Generates a SQLite polymorphic SELECT query for a given classMap
        //! @param viewSql [out] Output SQL for view
        //! @param prepareContext [in] prepareContext from ECSQL
        //! @param classMap [in] Source classMap for which to generate view
        //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
        //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
        static BentleyStatus GenerateSelectFromViewSql(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, ClassMap const& classMap, bool isPolymorphicQuery);
        static BentleyStatus CreateECClassViews(ECDbCR, bvector<ECN::ECClassId> const&);
        static BentleyStatus CreateECClassViews(ECDbCR);
        static BentleyStatus DropECClassViews(ECDbCR);
    };

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       11/2016
+===============+===============+===============+===============+===============+======*/
struct ConstraintECClassIdJoinInfo final
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

        static DbTable const* RequiresJoinTo(ConstraintECClassIdPropertyMap const&, bool ignoreVirtualColumnCheck);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
