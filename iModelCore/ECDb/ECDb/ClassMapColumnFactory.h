/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ClassMap;
struct RelationshipClassEndTableMap;
struct PropertyMap;
struct SingleColumnDataPropertyMap;

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnMaps
    {
    typedef std::unique_ptr<ColumnMaps> Ptr;
    private:
        bmap<Utf8CP, DbColumn const*, CompareIUtf8Ascii> m_maps;
        bset<DbColumn const*> n_columns;
        bset<Utf8CP, CompareIUtf8Ascii> m_newMappedColumns;
        bset<Utf8String, CompareIUtf8Ascii> m_strings;

        Utf8StringCR Copy(Utf8StringCR);

    public:
        ColumnMaps() {}
        ~ColumnMaps() {}

        bool IsNew(Utf8CP accessString) const { return m_newMappedColumns.find(accessString) != m_newMappedColumns.end(); }
        bmap<Utf8CP, DbColumn const*, CompareIUtf8Ascii> const& GetEntries() const { return m_maps; }
        bset< Utf8CP, CompareIUtf8Ascii>  const& GetNewlyAddedAccessStrings() const { return m_newMappedColumns; }
        bool IsColumnInUsed(DbColumn const& column) const;
        void Insert(Utf8StringCR accessString, DbColumn const& column, bool newlyMappedColumn = false);
        void Insert(SingleColumnDataPropertyMap const& propertyMap);
        DbColumn* FindP(Utf8CP accessString) const
            {
            auto itor = m_maps.find(accessString);
            if (itor != m_maps.end())
                return const_cast<DbColumn*>(itor->second);

            return nullptr;
            }
        void Debug() const
            {
            for (auto const& map : m_maps)
                printf("[AccessString=%s] [Column=%s]\n", map.first, map.second->GetName().c_str());
            }
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ColumnMapContext
    {
    enum class Filter
        {
        InheritedAndLocal = 1,
        DerivedAndLocal = 2,
        Full = 3
        };
    private:
        enum class RelationshpFilter
            {
            Direct,
            All
            };

        static BentleyStatus QueryLocalColumnMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryInheritedColumnMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryDerivedColumnMaps(ColumnMaps&, ClassMap const&);

        ColumnMapContext();
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter, ClassMap const* base);

    public:
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter);

    };


//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ClassMapColumnFactory final
    {
    public:
        struct ColumnResolutionScope
            {
            private:
                ColumnMaps m_columnMaps;
                bool m_init = false;

            protected:
                ClassMap const& m_classMap;

            private:
                virtual void _Fill(ColumnMaps&) = 0;

            protected:
                explicit ColumnResolutionScope(ClassMap const&);

            public:
                virtual ~ColumnResolutionScope();

                ClassMap const& GetClassMap() const { return m_classMap; }
                ColumnMaps& GetColumnMaps();
            };
        
    private:
        ClassMap const& m_classMap;
        mutable DbTable* m_primaryOrJoinedTable = nullptr;
        mutable DbTable* m_overflowTable = nullptr;
        bool m_useSharedColumnStrategy = false;
        Nullable<uint32_t> m_maxSharedColumnCount;
        mutable bool m_areSharedColumnsReserved = false;
        mutable ColumnResolutionScope* m_columnResolutionScope = nullptr;

        //not copyable
        ClassMapColumnFactory(ClassMapColumnFactory const&) = delete;
        ClassMapColumnFactory& operator=(ClassMapColumnFactory const&) = delete;

        DbColumn* HandleOverflowColumn(DbColumn* column) const;
        DbTable* GetEffectiveTable(SchemaImportContext&) const;
        DbTable* GetOrCreateOverflowTable(SchemaImportContext&) const;
        DbColumn* ReuseOrCreateSharedColumn(SchemaImportContext&) const;
        ColumnMaps* GetColumnMaps() const;
        DbColumn* RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const;
        bool IsCompatible(DbColumn const&, DbColumn::Type, DbColumn::CreateParams const&) const;
        DbColumn* AllocateColumn(SchemaImportContext&, ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR) const;
        DbColumn* AllocatedSharedColumn(SchemaImportContext&, ECN::ECPropertyCR, DbColumn::CreateParams const&, Utf8StringCR) const;

        static uint32_t MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR);

    public:
        explicit ClassMapColumnFactory(ClassMap const&);
        ~ClassMapColumnFactory() {};
        bool UsesSharedColumnStrategy() const { return m_useSharedColumnStrategy; }
        bool IsColumnInUse(DbColumn const& column) const;
        bool MarkNavPropertyMapColumnUsed(NavigationPropertyMap const& map) const
            {
            PRECONDITION(map.IsComplete(), false);
            GetColumnMaps()->Insert(map.GetIdPropertyMap());
            GetColumnMaps()->Insert(map.GetRelECClassIdPropertyMap());
            return true;
            }
        DbColumn const* FindColumn(Utf8CP accessString) const 
            {
            return GetColumnMaps()->FindP(accessString);
            }
        void ReserveSharedColumns(Utf8StringCR propertyName) const;
        void ReserveSharedColumns(uint32_t columnsRequired) const;
        void ReleaseSharedColumnReservation() const { m_areSharedColumnsReserved = false; }
        void Debug() const { GetColumnMaps()->Debug(); }
        DbColumn* Allocate(SchemaImportContext&, ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bool forcePhysicalColum = false) const;
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ImportColumnResolutionScope final : ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        void _Fill(ColumnMaps&) override;

    public:
        explicit ImportColumnResolutionScope(ClassMap const& classMap) : ClassMapColumnFactory::ColumnResolutionScope(classMap) {}
        ~ImportColumnResolutionScope() {}

    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct UpdateColumnResolutionScope final : public  ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        void _Fill(ColumnMaps&) override;

    public:
        explicit UpdateColumnResolutionScope(ClassMap const& classMap) : ClassMapColumnFactory::ColumnResolutionScope(classMap) {}
        ~UpdateColumnResolutionScope() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
