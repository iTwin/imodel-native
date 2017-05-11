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
        bset< DbColumn const*> n_columns;
        bset< Utf8CP, CompareIUtf8Ascii> m_newMappedColumns;
        bset<Utf8String, CompareIUtf8Ascii> m_strings;

    private:
        void Assert(Utf8CP accessString) const { BeAssert(m_maps.find(accessString) == m_maps.end()); }
        Utf8CP Copy(Utf8CP);

    public:
        ColumnMaps(){}
        ~ColumnMaps(){}
        bool IsNew(Utf8CP accessString) const { return m_newMappedColumns.find(accessString) != m_newMappedColumns.end(); }
        bmap<Utf8CP, DbColumn const*, CompareIUtf8Ascii> const& GetEntries() const { return m_maps; }
        bset< Utf8CP, CompareIUtf8Ascii>  const& GetNewlyAddedAccessStrings() const { return m_newMappedColumns; }
        bool IsColumnInUsed(DbColumn const& column) const;
        void Insert(Utf8CP accessString, DbColumn const& column, bool newlyMappedColumn = false);
        void Insert(SingleColumnDataPropertyMap const& propertyMap);
        DbColumn * FindP(Utf8CP accessString) const
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
        InheritedAndDerivedAndLocal = 3
        };
    private:
        static BentleyStatus QueryLocalColumnMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryInheritedColumnMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryDerivedColumnMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryEndTableRelationshipMaps(ColumnMaps&, ClassMap const&);
        static BentleyStatus QueryMixinColumnMaps(ColumnMaps&, ClassMap const&, std::vector<ECN::ECClassCP> const*);
        static BentleyStatus FindMixins(std::vector<ECN::ECClassCP>&, ECDbCR, ECN::ECClassId);
        static ClassMap const*  FindMixinImplementation(ECDbCR, ECN::ECClassCR, DbTableId, ECN::ECClassId);
        ColumnMapContext();
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter, ClassMap const* base );

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
            private:
                ColumnMaps m_columnMaps;
                ClassMap const& m_classMap;
                bool m_init;
            private:
                virtual void Fill(ColumnMaps& columnMaps, ClassMap const& classMap) = 0;

            public:
                ClassMap const& GetClassMap() const { return m_classMap; }
                ColumnMaps& GetColumnMaps();
                ColumnResolutionScope(ClassMap const& classMap);
                ~ColumnResolutionScope();
            };
        struct SharedColumnReservation:NonCopyableClass
            {
            private:
                ClassMapColumnFactory const& m_allocator;
            public:
                SharedColumnReservation(ClassMapColumnFactory const& allocator)
                    :m_allocator(allocator)
                    {}
            };
    private:
        ClassMap const& m_classMap;
        mutable DbTable* m_primaryOrJoinedTable;
        mutable DbTable* m_overflowTable;
        mutable std::shared_ptr<SharedColumnReservation> m_sharedColumnReservation;
        bool m_useSharedColumnStrategy;
        int m_maxSharedColumnCount;
        mutable ColumnResolutionScope* m_columnResolutionScope;

    private:
        DbTable* GetEffectiveTable() const;
        DbTable* GetOrCreateOverflowTable() const;
        DbColumn* ReuseOrCreateSharedColumn() const;
        bool IsColumnInUse(DbColumn const& column) const;
        ColumnMaps* GetColumMaps() const;
        ECDbCR GetECDb() const;
        DbColumn* RegisterColumnMap(Utf8CP accessString, DbColumn* column) const;
        bool IsCompatible(DbColumn const&, DbColumn::Type, DbColumn::CreateParams const&) const;
        DbColumn* AllocateColumn(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8CP) const;
        DbColumn* AllocatedSharedColumn(ECN::ECPropertyCR, DbColumn::CreateParams const&,Utf8CP) const;
        static int MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR);

    public:
        ClassMapColumnFactory(ClassMap const& classMap);
        ~ClassMapColumnFactory() {};
        bool UsesSharedColumnStrategy() const { return m_useSharedColumnStrategy; }
        std::weak_ptr <SharedColumnReservation> GetSharedColumnReservation() const { return std::weak_ptr <SharedColumnReservation>(m_sharedColumnReservation); }
        std::weak_ptr <SharedColumnReservation> CreateSharedColumnReservation(Utf8CP, int = 0) const;
        void  ReleaseSharedColumnReservation() const { m_sharedColumnReservation = nullptr; }
        DbColumn* Allocate(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8CP accessString) const;
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct ImportColumnResolutionScope : ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        virtual void Fill(ColumnMaps& columnMaps, ClassMap const& classMap) override;

    public:
        ImportColumnResolutionScope(ClassMap const& classMap)
        : ClassMapColumnFactory::ColumnResolutionScope(classMap) {}

    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct UpdateColumnResolutionScope : public  ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        virtual void Fill(ColumnMaps& columnMaps, ClassMap const& classMap) override;

    public:
        UpdateColumnResolutionScope(ClassMap const& classMap)
            :ClassMapColumnFactory::ColumnResolutionScope(classMap){}
    };

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//===============+===============+===============+===============+===============+======
struct EndTableRelationshipColumnResolutionScope : public  ClassMapColumnFactory::ColumnResolutionScope
    {
    private:
        virtual void Fill(ColumnMaps& columnMaps, ClassMap const& classMap) override;
        std::vector<ClassMap const*> m_releventMaps;
    public:
        EndTableRelationshipColumnResolutionScope(ClassMap const& classMap, std::vector<ClassMap const*> classMaps)
            : ClassMapColumnFactory::ColumnResolutionScope(classMap), m_releventMaps(classMaps) {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
