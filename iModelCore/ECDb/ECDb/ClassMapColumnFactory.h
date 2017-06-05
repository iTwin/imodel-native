/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapColumnFactory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include "DbSchema.h"
#include <Bentley/NonCopyableClass.h>

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
        static BentleyStatus QueryEndTableRelationshipMaps(ColumnMaps&, ClassMap const&, RelationshpFilter);
        static BentleyStatus QueryMixinColumnMaps(ColumnMaps&, ClassMap const&, std::vector<ECN::ECClassCP> const*);
        static BentleyStatus FindMixins(std::vector<ECN::ECClassCP>&, ECDbCR, ECN::ECClassId);
        static ClassMap const*  FindMixinImplementation(ECDbCR, ECN::ECClassCR, DbTableId, ECN::ECClassId);
        static void AppendRelationshipColumnMaps(ColumnMaps& columnMaps, ClassMap const& classMap, ECN::ECClassId relationshipClassId);

        ColumnMapContext();
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter, ClassMap const* base);

    public:
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter);

    };


//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ClassMapColumnFactory : NonCopyableClass
    {
    public:
        struct ColumnResolutionScope
            {
            protected:
                ClassMap const& m_classMap;

            private:
                ColumnMaps m_columnMaps;
                bool m_init = false;

                virtual void _Fill(ColumnMaps&) = 0;

            public:
                explicit ColumnResolutionScope(ClassMap const&);
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

        DbTable* GetEffectiveTable() const;
        DbTable* GetOrCreateOverflowTable() const;
        DbColumn* ReuseOrCreateSharedColumn() const;
        bool IsColumnInUse(DbColumn const& column) const;
        ColumnMaps* GetColumnMaps() const;
        ECDbCR GetECDb() const;
        DbColumn* RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const;
        bool IsCompatible(DbColumn const&, DbColumn::Type, DbColumn::CreateParams const&) const;
        DbColumn* AllocateColumn(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR) const;
        DbColumn* AllocatedSharedColumn(ECN::ECPropertyCR, DbColumn::CreateParams const&, Utf8StringCR) const;

        static uint32_t MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR);

    public:
        explicit ClassMapColumnFactory(ClassMap const& classMap);
        ~ClassMapColumnFactory() {};
        bool UsesSharedColumnStrategy() const { return m_useSharedColumnStrategy; }
        void ReserveSharedColumns(Utf8StringCR propertyName) const;
        void ReserveSharedColumns(uint32_t columnsRequired) const;
        void ReleaseSharedColumnReservation() const { m_areSharedColumnsReserved = false; }
        DbColumn* Allocate(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bool forcePhysicalColum = false) const;
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

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct EndTableRelationshipColumnResolutionScope final : public  ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        std::vector<ClassMap const*> m_relevantMaps;

        void _Fill(ColumnMaps&) override;

    public:
        EndTableRelationshipColumnResolutionScope(ClassMap const& classMap, std::vector<ClassMap const*> classMaps)
            : ClassMapColumnFactory::ColumnResolutionScope(classMap), m_relevantMaps(classMaps) {}

        ~EndTableRelationshipColumnResolutionScope() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
