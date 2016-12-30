/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMapVisitor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    public:
        enum class ContainsState
            {
            None,
            Some,
            All
            };
    private:
        DbTable const* m_table = nullptr;
        PropertyMap::Type m_filter = PropertyMap::Type::All;
        bool m_doNotSkipSystemPropertyMaps = false;
        mutable std::vector<DbColumn const*> m_columns;
        mutable ContainsState m_containsVirtualColumns = ContainsState::None;
        mutable ContainsState m_containsOverflowColumns = ContainsState::None;

        virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(SystemPropertyMap const&) const override;

        static ContainsState UpdateContainsState(ContainsState previous, bool matchIsContained);

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

        ContainsState ContainsVirtualColumns() const { return m_containsVirtualColumns; }
        ContainsState ContainsOverflowColumns() const { return m_containsOverflowColumns; }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct GetTablesPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        PropertyMap::Type m_filter;
        mutable std::set<DbTable const*> m_tables;

        virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit GetTablesPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All) : IPropertyMapVisitor(), m_filter(filter) {}
        ~GetTablesPropertyMapVisitor() {}

        std::set<DbTable const*> const& GetTables() const { return m_tables; }
        DbTable const* GetSingleTable() const;
    };

//=======================================================================================
// Search PropertyMap with a given type
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SearchPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        PropertyMap::Type m_filter;
        bool m_recurseIntoCompoundPropertyMaps;
        mutable std::vector<PropertyMap const*> m_foundPropertyMaps;

        virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(CompoundDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit SearchPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All, bool recurseIntoCompoundPropertyMaps = true)
            : IPropertyMapVisitor(), m_filter(filter), m_recurseIntoCompoundPropertyMaps(recurseIntoCompoundPropertyMaps)
            {}
        ~SearchPropertyMapVisitor() {}

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

            Utf8CP GetAccessString() const { return GetPropertyMap().GetAccessString().c_str(); }
            SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
            NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
            NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
            Utf8CP GetSql() const { return m_sql.ToString(); }
            DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
            DbTable const& GetTable() const { return GetColumn().GetTable(); }
        };

    private:
        ECSqlScope m_scope;
        Utf8CP m_classIdentifier;
        DbTable const& m_tableFilter;
        bool m_wrapInParentheses;
        mutable bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_resultSetByAccessString;
        mutable std::vector<Result> m_resultSet;

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
        ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope target, Utf8CP classIdentifier, bool wrapInParentheses = false);
        ~ToSqlPropertyMapVisitor() {}
        std::vector<Result> const& GetResultSet() const { return m_resultSet; }
        const Result* Find(Utf8CP accessString) const;
        void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); }

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

        virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const&) const override;
        virtual BentleyStatus _Visit(SystemPropertyMap const&) const override;

    public:
        explicit SavePropertyMapVisitor(DbClassMapSaveContext& ctx) : IPropertyMapVisitor(), m_context(ctx) {}
        ~SavePropertyMapVisitor() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
