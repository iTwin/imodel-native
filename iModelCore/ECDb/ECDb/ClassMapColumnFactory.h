/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ClassMap;
struct RelationshipClassEndTableMap;
struct PropertyMap;
struct SingleColumnDataPropertyMap;
struct SchemaReservationColumnEntry;

//======================================================================================
// @bsiclass
//===============+===============+===============+===============+===============+======
struct ColumnMaps
    {
    typedef std::unique_ptr<ColumnMaps> Ptr;
    private:
        bmap<Utf8String, DbColumn const*, CompareIUtf8Ascii> m_maps;
        bset<DbColumn const*> m_columns;
        bset<Utf8String, CompareIUtf8Ascii> m_newMappedColumns;

    public:
        ColumnMaps() {}
        ~ColumnMaps() {}

        bool IsNew(Utf8CP accessString) const { return m_newMappedColumns.find(accessString) != m_newMappedColumns.end(); }
        bmap<Utf8String, DbColumn const*, CompareIUtf8Ascii> const& GetEntries() const { return m_maps; }
        bset<Utf8String, CompareIUtf8Ascii>  const& GetNewlyAddedAccessStrings() const { return m_newMappedColumns; }
        bool IsColumnInUse(DbColumn const& column) const;
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
                printf("[AccessString=%s] [Column=%s]\n", map.first.c_str(), map.second->GetName().c_str());
            }
    };

//======================================================================================
// @bsiclass
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
        static BentleyStatus QueryDerivedColumnMapsForClass(ECN::ECClassCR ecClass, ColumnMaps& columnMaps, ClassMap const& contextClassMap);
        static BentleyStatus QueryDerivedColumnMaps(ColumnMaps&, ClassMap const&);

        ColumnMapContext();
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter, ClassMap const* base);

    public:
        static BentleyStatus Query(ColumnMaps&, ClassMap const&, Filter);

    };


//======================================================================================
// @bsiclass
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
        mutable bool m_putCurrentPropertyToOverflow = false;
        mutable ColumnResolutionScope* m_columnResolutionScope = nullptr;

        //not copyable
        ClassMapColumnFactory(ClassMapColumnFactory const&) = delete;
        ClassMapColumnFactory& operator=(ClassMapColumnFactory const&) = delete;

        DbColumn* HandleOverflowColumn(DbColumn* column) const;
        DbTable* GetEffectiveTable(SchemaImportContext&) const;
        DbTable* GetOrCreateOverflowTable(SchemaImportContext&) const;
        DbColumn* ReuseOrCreateSharedColumn(SchemaImportContext&) const;
        //! When SchemaSync column reservation is active, resolve the reserved placement of the
        //! physical column identified by @p accessString (a leaf column access string such as
        //! "Geo.X"): whether it was reserved in this class's primary or overflow table, plus the
        //! reserved entry. @p prop is the ECProperty the access string belongs to (used only to
        //! detect navigation properties, which are not reserved). Returns false when no column
        //! reservation is active or the column has no reservation.
        bool TryGetReservedColumnPlacement(SchemaImportContext&, ECN::ECPropertyCR prop, Utf8StringCR accessString, bool& reservedInOverflow, SchemaReservationColumnEntry const*& outEntry) const;
        bool IsColumnUsedByAnyDerivedClass(DbColumn const&, SchemaImportContext&) const;
        ColumnMaps* GetColumnMaps() const;
        DbColumn* RegisterColumnMap(Utf8StringCR accessString, DbColumn* column) const;
        bool IsCompatible(DbColumn const&, DbColumn::Type, DbColumn::CreateParams const&) const;
        DbColumn* AllocateColumn(SchemaImportContext&, ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR) const;
        DbColumn* AllocateSharedColumn(SchemaImportContext&, ECN::ECPropertyCR, DbColumn::CreateParams const&, Utf8StringCR) const;

        static uint32_t MaxColumnsRequiredToPersistProperty(ECN::ECPropertyCR);

    public:
        explicit ClassMapColumnFactory(ClassMap const&);
        ~ClassMapColumnFactory() {};
        //! Maximum physical SQLite columns per primary/overflow shared-column table.
        //! Referenced by SchemaSync reservation logic to determine the overflow threshold.
        static constexpr uint32_t kMaxPhysicalColumnsPerTable = 63;
        //! Enumerates the leaf column access strings a property expands into, appending them to
        //! @p out. @p accessString is the property's own access string (its name at the top level,
        //! or the parent path for a nested struct member). Point3d/Point2d expand into ".X/.Y/.Z"
        //! columns, navigation into ".Id/.RelECClassId", structs recurse into members, and every
        //! other property contributes a single column equal to @p accessString. This mirrors the
        //! per-leaf access strings passed to Allocate during mapping, so the SchemaSync reserve
        //! walk and the mapping-time consumer agree on the reservation key for each column.
        static void CollectColumnAccessStrings(ECN::ECPropertyCR property, Utf8StringCR accessString, bvector<Utf8String>& out);
        //! Pure overflow-budget arithmetic shared by the mapping-time allocator
        //! (EvaluateIfPropertyGoesToOverflow) and the SchemaSync reserve walk, so the primary-vs-
        //! overflow decision is computed once and never duplicated. Returns true when a property
        //! needing @p columnsRequired physical columns must spill into the overflow table given the
        //! base table's current shared-column budget:
        //!  - @p availablePhysicalColumns: unused physical column slots left in the base table
        //!    (kMaxPhysicalColumnsPerTable minus the physical columns it already has);
        //!  - @p sharedColumnCount: number of (non-freed) shared columns the base table already has;
        //!  - @p reusableSharedColumnCount: how many of those shared columns can be reused for the
        //!    property's class (not in use by it, an ancestor, or a descendant);
        //!  - @p maxSharedColumnsBeforeOverflow: the ShareColumns CA budget (null = unlimited).
        static bool EvaluateOverflowFromBudget(uint32_t columnsRequired, uint32_t availablePhysicalColumns, uint32_t sharedColumnCount, uint32_t reusableSharedColumnCount, Nullable<uint32_t> const& maxSharedColumnsBeforeOverflow);
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
            void EnsurePropertyGoesToOverflow(Utf8StringCR propertyName, SchemaImportContext& ctx) const;
            void EvaluateIfPropertyGoesToOverflow(Utf8StringCR propertyName, SchemaImportContext& ctx) const;
            void EvaluateIfPropertyGoesToOverflow(uint32_t columnsRequired, SchemaImportContext& ctx) const;
            void ResetCurrentPropertyOverflowFlag() const { m_putCurrentPropertyToOverflow = false; }
            void Debug() const { GetColumnMaps()->Debug(); }
            DbColumn* Allocate(SchemaImportContext&, ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bool forcePhysicalColum = false) const;
    };

//======================================================================================
// @bsiclass
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
// @bsiclass
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
