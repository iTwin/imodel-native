/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMapVisitor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "PropertyMap.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct GetColumnsPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        DbTable const* m_table = nullptr;
        PropertyMap::Type m_filter = PropertyMap::Type::All;
        bool m_doNotSkipSystemPropertyMaps = false;
        mutable std::vector<DbColumn const*> m_columns;
        mutable uint32_t m_virtualColumnCount = 0;

        BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        GetColumnsPropertyMapVisitor(DbTable const& table, PropertyMap::Type filter = PropertyMap::Type::All)
            : IPropertyMapVisitor(), m_table(&table), m_filter(filter)
            {}
        GetColumnsPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All, bool doNotSkipSystemPropertyMaps = false)
            :IPropertyMapVisitor(), m_filter(filter), m_doNotSkipSystemPropertyMaps(doNotSkipSystemPropertyMaps)
            {}

        ~GetColumnsPropertyMapVisitor() {}

        std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
        DbColumn const* GetSingleColumn() const;

        uint32_t GetColumnCount() const { return (uint32_t) m_columns.size(); }
        uint32_t GetVirtualColumnCount() const { return m_virtualColumnCount ; }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct GetTablesPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        PropertyMap::Type m_filter;
        mutable std::set<DbTable const*> m_tables;

        BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit GetTablesPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All) : IPropertyMapVisitor(), m_filter(filter) {}
        ~GetTablesPropertyMapVisitor() {}

        std::set<DbTable const*> const& GetTables() const { return m_tables; }

    };

//=======================================================================================
// Search PropertyMap with a given type
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SearchPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        mutable std::vector<PropertyMap const*> m_foundPropertyMaps;
        std::function<bool(PropertyMap const&)> m_propertyMapFilterCallback;
        std::function<bool(CompoundDataPropertyMap const&)> m_recurseIntoCompoundPropertyMap;

        BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit SearchPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All, bool recurseIntoCompoundPropertyMaps = true)
            {
            SetCallbackPropertyMapFilter([filter](PropertyMap const& propertyMap) { return Enum::Contains(filter, propertyMap.GetType()); });
            SetCallbackRecurseIntoCompoundPropertyMap([recurseIntoCompoundPropertyMaps](CompoundDataPropertyMap const&) { return recurseIntoCompoundPropertyMaps; });
            }

        ~SearchPropertyMapVisitor() {}

        void SetCallbackPropertyMapFilter(std::function<bool(PropertyMap const&)> callback) { m_propertyMapFilterCallback = callback; }
        void SetCallbackRecurseIntoCompoundPropertyMap(std::function<bool(CompoundDataPropertyMap const&)> callback) { m_recurseIntoCompoundPropertyMap = callback; }

        std::vector<PropertyMap const*> const& Results() const { return m_foundPropertyMaps; }
    };


//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ToSqlPropertyMapVisitor final : IPropertyMapVisitor
    {
    enum class ECSqlScope
        {
        Select,
        NonSelectNoAssignmentExp,
        NonSelectAssignmentExp
        };

    struct Result
        {
        private:
            SingleColumnDataPropertyMap const* m_propertyMap;
            NativeSqlBuilder m_sql;

        public:
            Result() : m_propertyMap(nullptr) {}
            explicit Result(SingleColumnDataPropertyMap const& propertyMap) :m_propertyMap(&propertyMap) {}

            SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
            NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
            NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
            DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
        };

    private:
        ECSqlScope m_scope;
        Utf8String m_classIdentifier;
        DbTable const& m_tableFilter;
        bool m_wrapInParentheses;
        mutable std::vector<Result> m_resultSet;

        BentleyStatus _Visit(SingleColumnDataPropertyMap const& propertyMap) const override { return ToNativeSql(propertyMap); }
        BentleyStatus _Visit(SystemPropertyMap const&) const override;

        BentleyStatus ToNativeSql(SingleColumnDataPropertyMap const&) const;
        BentleyStatus ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const&, bool requiresCast) const;
        BentleyStatus ToNativeSql(ConstraintECInstanceIdPropertyMap const&) const;
        BentleyStatus ToNativeSql(ConstraintECClassIdPropertyMap const&) const;
        BentleyStatus ToNativeSql(ECClassIdPropertyMap const&) const;
        BentleyStatus ToNativeSql(ECInstanceIdPropertyMap const&) const;
        Result& Record(SingleColumnDataPropertyMap const&) const;

        //Casts are needed for shared columns in all cases except for SELECT and assignment expressions
        //to make sure shared columns behave the same as unshared columns.
        //Shared columns are of type BLOB which behaves differently in terms of type conversions prior to comparisons
        //(see https://sqlite.org/datatype3.html#type_conversions_prior_to_comparison)
        bool RequiresCast(SingleColumnDataPropertyMap const& propMap) const { return m_scope == ECSqlScope::NonSelectNoAssignmentExp && propMap.GetColumn().IsShared() && propMap.GetColumnDataType() != DbColumn::Type::Blob && propMap.GetColumnDataType() != DbColumn::Type::Any; }

    public:
        ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope target, bool wrapInParentheses = false);
        ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope target, Utf8StringCR classIdentifier, bool wrapInParentheses = false);
        ~ToSqlPropertyMapVisitor() {}
        std::vector<Result> const& GetResultSet() const { return m_resultSet; }
        void Reset() const { m_resultSet.clear(); }

        bool IsForAssignmentExpression() const { return m_scope == ECSqlScope::NonSelectAssignmentExp; }
    };

struct DbClassMapSaveContext;
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SavePropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        DbClassMapSaveContext& m_context;
        DbTable const* m_tableFilter;
        BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit SavePropertyMapVisitor(DbClassMapSaveContext& ctx, DbTable const* tableFilter = nullptr) : IPropertyMapVisitor(), m_context(ctx), m_tableFilter(tableFilter) {}
        ~SavePropertyMapVisitor() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
