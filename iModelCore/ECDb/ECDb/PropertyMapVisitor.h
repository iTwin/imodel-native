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
    private:
        mutable std::vector<DbColumn const*> m_columns;
        DbTable const* m_table;
        PropertyMap::Type m_filter;
        bool m_doNotSkipSystemPropertyMaps;
    private:

        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const override;

    public:
        GetColumnsPropertyMapVisitor(DbTable const& table, PropertyMap::Type filter = PropertyMap::Type::All)
            :m_table(&table), m_filter(filter), m_doNotSkipSystemPropertyMaps(false)
            {}
        GetColumnsPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All, bool doNotSkipHorizontalPropertyMaps = false)
            :m_table(nullptr), m_filter(filter), m_doNotSkipSystemPropertyMaps(doNotSkipHorizontalPropertyMaps)
            {}
        ~GetColumnsPropertyMapVisitor() {}
        void Reset() { m_columns.clear(); }
        std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
        bool AreResultingColumnsAreVirtual() const
            {
            BeAssert(!GetColumns().empty());
            bool isVirtual = true;
            for (DbColumn const* column : GetColumns())
                {
                isVirtual &= column->GetPersistenceType() == PersistenceType::Virtual;
                if (!isVirtual)
                    break;
                }

            return isVirtual;
            }
        DbColumn const* GetSingleColumn() const
            {
            BeAssert(GetColumns().size() == 1);
            if (GetColumns().size() != 1)
                {
                return nullptr;
                }

            return GetColumns().front();
            }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct GetTablesPropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        mutable std::set<DbTable const*> m_tables;
        PropertyMap::Type m_filter;
    private:

        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                m_tables.insert(&propertyMap.GetTable());

            return VisitorFeedback::Cancel;
            }
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                m_tables.insert(&propertyMap.GetTable());

            return VisitorFeedback::NextSibling;
            }
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                m_tables.insert(propertyMap.GetTables().begin(), propertyMap.GetTables().end());

            return VisitorFeedback::Cancel;
            }

    public:
        GetTablesPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All)
            : m_filter(filter)
            {}
        std::set<DbTable const*> GetTables() const { return m_tables; }
        DbTable const* GetSingleTable() const
            {
            BeAssert(!m_tables.empty());
            if (m_tables.size() != 1)
                return nullptr;

            return *(m_tables.begin());
            }
    };

//=======================================================================================
// Search PropertyMap with a given type
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SearchPropertyMapVisitor final : IPropertyMapVisitor
    {

    private:
        mutable std::vector<PropertyMap const*> m_propertyMaps;
        PropertyMap::Type m_filter;
        bool m_traverseCompoundProperties;
    private:

        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                m_propertyMaps.push_back(&propertyMap);

            return VisitorFeedback::Next;
            }
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                {
                if (m_traverseCompoundProperties)
                    return VisitorFeedback::Next;

                m_propertyMaps.push_back(&propertyMap);
                return VisitorFeedback::NextSibling;
                }

            return VisitorFeedback::Next;
            }
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetType()))
                m_propertyMaps.push_back(&propertyMap);

            return VisitorFeedback::Next;
            }

    public:
        SearchPropertyMapVisitor(PropertyMap::Type filter = PropertyMap::Type::All, bool traverseCompoundProperties = false)
            :m_filter(filter), m_traverseCompoundProperties(traverseCompoundProperties)
            {}
        ~SearchPropertyMapVisitor() {}
        void Reset() { m_propertyMaps.clear(); }
        std::vector<PropertyMap const*> const& ResultSet() const { return m_propertyMaps; }
        static std::vector<PropertyMap const*> Accept(PropertyMap const& propertyMap, PropertyMap::Type filter = PropertyMap::Type::All, bool traverseCompoundProperties = false)
            {
            SearchPropertyMapVisitor visitor(filter, traverseCompoundProperties);
            propertyMap.AcceptVisitor(visitor);
            return visitor.ResultSet();
            }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ToSqlPropertyMapVisitor final : IPropertyMapVisitor
    {
    enum SqlTarget
        {
        View, //!Inline view is in from. Normally it happen only in SELECT statement where view has a contract.
        Table, //!Direct query against a table
        };
    struct Result
        {
        private:
            SingleColumnDataPropertyMap const* m_propertyMap;
            NativeSqlBuilder m_sql;
        public:
            Result(SingleColumnDataPropertyMap const& propertyMap)
                :m_propertyMap(&propertyMap)
                {}
            Result()
                :m_propertyMap(nullptr)
                {}
            ~Result() {}
            Utf8CP GetAccessString() const { return GetPropertyMap().GetAccessString().c_str(); }
            SingleColumnDataPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
            NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
            NativeSqlBuilder const& GetSqlBuilder() const { return m_sql; }
            Utf8CP GetSql() const { return m_sql.ToString(); }
            DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
            DbTable const& GetTable() const { return GetColumn().GetTable(); }
            bool  IsColumnPersisted() const { return GetColumn().GetPersistenceType() == PersistenceType::Persisted; }
            bool  IsTablePersisted() const { return GetTable().GetPersistenceType() == PersistenceType::Persisted; }
        };

    private:
        mutable bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_resultSetByAccessString;
        mutable std::vector<Result> m_resultSet;
        mutable BentleyStatus m_status;
        SqlTarget m_target;
        Utf8CP m_classIdentifier;
        DbTable const& m_tableFilter;
        bool m_wrapInParentheses;
        bool m_usePropertyNameAsAliasForSystemPropertyMaps;
    private:
        Result& Record(SingleColumnDataPropertyMap const& propertyMap) const;
        bool IsAlienTable(DbTable const& table) const;
        SingleColumnDataPropertyMap const* FindSystemPropertyMapForTable(SystemPropertyMap const& systemPropertyMap) const;
        VisitorFeedback ToNativeSql(SingleColumnDataPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(ECClassIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const;

    private:
        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const override;

    public:
        ToSqlPropertyMapVisitor(DbTable const& tableFilter, SqlTarget target, Utf8CP classIdentifier, bool wrapInParentheses = false, bool usePropertyNameAsAliasForSystemPropertyMaps = false)
            :m_tableFilter(tableFilter), m_target(target), m_classIdentifier(classIdentifier), m_wrapInParentheses(wrapInParentheses), m_status(SUCCESS), m_usePropertyNameAsAliasForSystemPropertyMaps(usePropertyNameAsAliasForSystemPropertyMaps)
            {
            if (m_usePropertyNameAsAliasForSystemPropertyMaps)
                {
                BeAssert(target == SqlTarget::Table);
                BeAssert(wrapInParentheses == false);
                }

            if (m_classIdentifier != nullptr && strlen(m_classIdentifier) == 0)
                m_classIdentifier = nullptr;
            }

        ~ToSqlPropertyMapVisitor() {}

        BentleyStatus GetStatus() const { return m_status; }
        std::vector<Result> const& GetResultSet() const { return m_resultSet; }
        const Result* Find(Utf8CP accessString) const;
        NativeSqlBuilder::List ToList() const
            {
            NativeSqlBuilder::List list;
            for (Result const& r : m_resultSet)
                list.push_back(r.GetSqlBuilder());

            return list;
            }
        void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); m_status = SUCCESS; }
    };

struct DbClassMapSaveContext;
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SavePropertyMapVisitor final : IPropertyMapVisitor
    {
    private:
        DbClassMapSaveContext& m_context;
        mutable BentleyStatus m_status;
        mutable PropertyMap const* m_failedMap;

    private:
        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const override;

    public:
        explicit SavePropertyMapVisitor(DbClassMapSaveContext& ctx) : m_context(ctx), m_status(SUCCESS), m_failedMap(nullptr) {}
        ~SavePropertyMapVisitor() {}
        BentleyStatus GetStatus() const { return m_status; }
        PropertyMap const* GetPropertyMapThatCausedError() const { return m_failedMap; }
        void Reset() { m_status = SUCCESS; m_failedMap = nullptr; }
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
