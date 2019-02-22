/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $ 
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include "DbSchema.h"
#include "ECDbSqlFunctions.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct TableSpaceSchemaManager;
struct ECSqlPrepareContext;
struct ConstraintECClassIdJoinInfo;
struct MemberFunctionCallExp;

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
            TableSpaceSchemaManager const& m_schemaManager;

        protected:
            Context(ViewType viewType, ECDbCR ecdb, TableSpaceSchemaManager const& manager) : m_viewType(viewType), m_ecdb(ecdb), m_schemaManager(manager) {}

        public:
            virtual ~Context() {}

            ViewType GetViewType() const { return m_viewType; }
            ECDbCR GetECDb() const { return m_ecdb; }
            TableSpaceSchemaManager const& GetSchemaManager() const { return m_schemaManager; }
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
            bool m_isPolymorphicQuery = false;
            MemberFunctionCallExp const* m_memberFunctionCallExp = nullptr;
        public:
            SelectFromViewContext(ECSqlPrepareContext const&, TableSpaceSchemaManager const& manager, bool isPolymorphicQuery, MemberFunctionCallExp const*);
            ~SelectFromViewContext() {}

            ECSqlPrepareContext const& GetPrepareCtx() const { return m_prepareCtx; }
            bool IsPolymorphicQuery() const { return m_isPolymorphicQuery; }
            MemberFunctionCallExp const* GetMemberFunctionCallExp() const { return m_memberFunctionCallExp; }

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
                explicit ECClassViewContext(ECDbCR ecdb) : Context(ViewType::ECClassView, ecdb, ecdb.Schemas().Main()), m_captureViewColumnNames(true) {}
                ~ECClassViewContext() {}

                bool MustCaptureViewColumnNames() const { return m_captureViewColumnNames; }
                void StopCaptureViewColumnNames() { m_captureViewColumnNames = false; }
                void AddViewColumnName(Utf8StringCR propAccessString) { BeAssert(MustCaptureViewColumnNames()); if (!MustCaptureViewColumnNames()) return;  BeAssert(!propAccessString.empty()); m_viewColumnNameList.push_back(&propAccessString); }

                bvector<Utf8StringCP> const& GetViewColumnNames() const { return m_viewColumnNameList; }
                
            };

        struct ToSqlVisitor final : IPropertyMapVisitor
            {
                struct Result final
                    {
                    public: 
                        enum class SqlExpressionType
                            {
                            PropertyName,
                            Literal,
                            Computed
                            };

                    private:
                        SingleColumnDataPropertyMap const* m_propertyMap = nullptr;
                        NativeSqlBuilder m_sql;
                        SqlExpressionType m_sqlExpType = SqlExpressionType::PropertyName;

                    public:
                        Result(SingleColumnDataPropertyMap const& propertyMap, SqlExpressionType sqlExpType) :m_propertyMap(&propertyMap), m_sqlExpType(sqlExpType) {}

                        SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
                        DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
                        NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
                        NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
                        //indicates what type the generated SQL snippet is.
                        //This is necessary for calling code to determine whether it has to add a col alias or not
                        SqlExpressionType GetSqlExpressionType() const { return m_sqlExpType; }
                    };

            private:
                Context const& m_context;
                Utf8String m_classIdentifier;
                DbTable const& m_tableFilter;
                mutable std::vector<Result> m_resultSet;

                BentleyStatus _Visit(SingleColumnDataPropertyMap const& propertyMap) const override { return ToNativeSql(propertyMap); }
                BentleyStatus _Visit(SystemPropertyMap const&) const override;

                BentleyStatus ToNativeSql(SingleColumnDataPropertyMap const&) const;
                BentleyStatus ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECInstanceIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ConstraintECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECClassIdPropertyMap const&) const;
                BentleyStatus ToNativeSql(ECInstanceIdPropertyMap const&) const;

                Result& AddResult(SingleColumnDataPropertyMap const&, Result::SqlExpressionType) const;

                //Casts are needed for shared columns to make sure shared columns behave the same as unshared columns.
                //Shared columns are of type BLOB which behaves differently in terms of type conversions prior to comparisons
                //(see https://sqlite.org/datatype3.html#type_conversions_prior_to_comparison)
                bool RequiresCast(SingleColumnDataPropertyMap const& propMap) const
                    { 
                    return m_context.GetViewType() == ViewType::ECClassView && propMap.GetColumn().IsShared() && propMap.GetColumnDataType() != DbColumn::Type::Any && propMap.GetColumnDataType() != DbColumn::Type::Blob;
                    }

            public:
                ToSqlVisitor(Context const& ctx, DbTable const& tableFilter, Utf8StringCR classIdentifier) : IPropertyMapVisitor(), m_context(ctx), m_tableFilter(tableFilter), m_classIdentifier(classIdentifier) {}
                ~ToSqlVisitor() {}
                std::vector<Result> const& GetResultSet() const { return m_resultSet; }
                void Reset() const { m_resultSet.clear(); }
            };

        ViewGenerator();
        ~ViewGenerator();
        static BentleyStatus GenerateViewSql(NativeSqlBuilder& viewSql, Context&, ClassMap const&);
        static BentleyStatus GenerateChangeSummaryViewSql(NativeSqlBuilder&, SelectFromViewContext&, ClassMap const&);

        static BentleyStatus CreateECClassView(ECDbCR, ClassMapCR);

        static BentleyStatus RenderPropertyMaps(NativeSqlBuilder& sqlView, Context&, bset<DbTable const*>& requireJoinTo, ClassMapCR classMap, DbTable const& contextTable, ClassMap const* baseClass, PropertyMap::Type filter, bool requireJoin = false);
        static BentleyStatus RenderRelationshipClassEndTableMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassEndTableMap const& relationMap);
        static BentleyStatus RenderRelationshipClassLinkTableMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassLinkTableMap const& relationMap);
        static BentleyStatus DoRenderRelationshipClassMap(NativeSqlBuilder& viewSql, Context&, RelationshipClassMap const& relationMap, DbTable const& contextTable, ConstraintECClassIdJoinInfo const& sourceJoinInfo, ConstraintECClassIdJoinInfo const& targetJoinInfo, RelationshipClassLinkTableMap const* castInto = nullptr);
        static BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderEntityClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap, DbTable const& contextTable, ClassMap const* castAs = nullptr);
        static BentleyStatus RenderNullView(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderMixinClassMap(NativeSqlBuilder& viewSql, Context&, ClassMap const& classMap);
        static BentleyStatus RenderMixinClassMap(bmap<Utf8String, bpair<DbTable const*, bvector<ECN::ECClassId>>, CompareIUtf8Ascii>& selectClauses, Context& ctx, ClassMap const& mixInClassMap, ClassMap const& derivedClassMap);
        static BentleyStatus GenerateECClassIdFilter(Utf8StringR filterSqlExpression, ClassMap const&, DbTable const&, DbColumn const& classIdColumn, bool polymorphic);
    public:
        static BentleyStatus GenerateSelectFromViewSql(NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, ClassMap const& classMap, bool isPolymorphicQuery, MemberFunctionCallExp const* memberFunctionCallExp = nullptr);
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
