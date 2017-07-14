/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbMappingManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "RelationshipClassMap.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SchemaImportContext;

//======================================================================================
// @bsiclass                                              Krischan.Eberle        07/2017
//======================================================================================
struct DbMappingManager final : NonCopyableClass
    {
    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2016
    //+===============+===============+===============+===============+===============+======
    struct Classes final
        {
        public:
            enum class PropertyMapInheritanceMode
                {
                NotInherited, //!< indicates that base property map is not inherited, but created from scratch
                Clone //! inherited property maps areGet cloned from the base class property map
                };

        private:
            struct Context final : NonCopyableClass
                {
                enum class Mode
                    {
                    ImportingSchema,
                    Loading
                    };

                SchemaImportContext* m_importCtx = nullptr;
                DbClassMapLoadContext const* m_loadCtx = nullptr;
                ClassMap& m_classMap;

                Context(SchemaImportContext& importCtx, ClassMap& classMap) : m_importCtx(&importCtx), m_classMap(classMap) {}
                Context(DbClassMapLoadContext const& loadCtx, ClassMap& classMap) : m_loadCtx(&loadCtx), m_classMap(classMap) {}

                Mode GetMode() const { return m_importCtx != nullptr ? Mode::ImportingSchema : Mode::Loading; }
                };

            static PropertyMap* ProcessProperty(Context&, ECN::ECPropertyCR);
            static RefCountedPtr<DataPropertyMap> MapPrimitiveProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* compoundPropMap);
            static RefCountedPtr<Point2dPropertyMap> MapPoint2dProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
            static RefCountedPtr<Point3dPropertyMap> MapPoint3dProperty(Context&, ECN::PrimitiveECPropertyCR, CompoundDataPropertyMap const* parentPropMap, Utf8StringCR accessString, DbColumn::CreateParams const&);
            static RefCountedPtr<PrimitiveArrayPropertyMap> MapPrimitiveArrayProperty(Context&, ECN::PrimitiveArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RefCountedPtr<StructPropertyMap> MapStructProperty(Context&, ECN::StructECPropertyCR, StructPropertyMap const* parentPropMap);
            static RefCountedPtr<StructArrayPropertyMap> MapStructArrayProperty(Context&, ECN::StructArrayECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RefCountedPtr<NavigationPropertyMap> MapNavigationProperty(Context&, ECN::NavigationECPropertyCR);
            static Utf8String ComputeAccessString(ECN::ECPropertyCR, CompoundDataPropertyMap const* parentPropMap);
            static RelationshipConstraintMap const& GetConstraintMap(ECN::NavigationECPropertyCR, RelationshipClassMapCR, NavigationPropertyMap::NavigationEnd);
            static BentleyStatus DetermineColumnInfoForPrimitiveProperty(DbColumn::CreateParams&, ClassMap const&, ECN::PrimitiveECPropertyCR, Utf8StringCR accessString);

        public:
            static PropertyMap* MapProperty(SchemaImportContext& importCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(importCtx, classMap); return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP This should be moved out of here. It seems like it doesn't have to do with mapping. And if it has, it needs to be renamed/refactored
            static PropertyMap* LoadPropertyMap(DbClassMapLoadContext const& loadCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(loadCtx, classMap);  return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP Moving the below MapNavigationProperty from elsewhere here makes evident that we now have 2 equally named methods
            //for mapping nav props, plus a third method related to nav props. This needs to be cleaned up.
            static ClassMappingStatus MapNavigationProperty(SchemaImportContext&, NavigationPropertyMap&);
            static BentleyStatus SetupNavigationPropertyMap(SchemaImportContext&, NavigationPropertyMap&);

            static BentleyStatus MapUserDefinedIndexes(SchemaImportContext& ctx, ClassMap const&);
            static BentleyStatus MapUserDefinedIndex(SchemaImportContext& ctx, ClassMap const&, DbIndexListCustomAttribute::DbIndex const&);

            static ECN::ECClassCP GetRootClass(ECN::ECClassCR);

            //! Rules:
            //! If MapStrategy != TPH: NotInherited
            //! Else: Clone
            static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
        };

    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct FkRelationships : NonCopyableClass
        {
        public:
            struct Collection final
                {
                private:
                    std::vector<std::unique_ptr<FkRelationships>> m_list;
                    bmap<ECN::ECClassId, FkRelationships*> m_byRootClassIdMap;

                public:
                    Collection() {}

                    FkRelationships* Get(ECN::ECClassId rootRelClassId) const 
                        {
                        auto it = m_byRootClassIdMap.find(rootRelClassId);
                        if (it == m_byRootClassIdMap.end())
                            return nullptr;

                        return it->second;
                        }

                    bool Contains(ECN::ECClassId rootRelClassId) const { return Get(rootRelClassId) != nullptr; }

                    std::vector<std::unique_ptr<FkRelationships>> const& Get() const { return m_list; }

                    void Add(SchemaImportContext& ctx, RelationshipClassEndTableMap const& relMap, ECN::ECClassId rootRelClassId)
                        {
                        std::unique_ptr<FkRelationships> newEntry = std::make_unique<DbMappingManager::FkRelationships>(ctx, relMap);
                        FkRelationships* newEntryP = newEntry.get();
                        m_list.push_back(std::move(newEntry));
                        m_byRootClassIdMap[rootRelClassId] = newEntryP;
                        }

                };
        private:
            struct PartitionInfo final
                {
                enum class ColumnId
                    {
                    ECInstanceId = 0,
                    ECClassId = 1,
                    SourceECInstanceId = 2,
                    SourceEClassId = 3,
                    TargetECInstanceId = 4,
                    TargetECClassId = 5
                    };

                private:
                    const DbColumn* m_cols[6];
                    bool m_isPersisted;

                public:
                    PartitionInfo() :m_isPersisted(false)
                        {
                        Clear();
                        }
                    PartitionInfo(RelationshipClassEndTableMap::Partition const& partition);
                    DbColumn const* Get(ColumnId id) const { return m_cols[Enum::ToInt(id)]; }
                    void Set(ColumnId id, DbColumn const* column);
                    void Clear();
                    bool IsValid() const;
                    bool IsPersisted() const { return m_isPersisted; }
                    static ColumnId ConstraintECInstanceId(ECN::ECRelationshipEnd end) { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnId::SourceECInstanceId : ColumnId::TargetECInstanceId; }
                    static ColumnId ConstraintECClassId(ECN::ECRelationshipEnd end) { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? ColumnId::SourceEClassId : ColumnId::TargetECClassId; }
                };

            struct ForeignKeyColumnInfo final
                {
                private:
                    Utf8String m_fkColName;
                    Utf8String m_relClassIdColName;

                    explicit ForeignKeyColumnInfo(Utf8StringCR fkColName) : m_fkColName(fkColName) {}

                    static Utf8String DetermineRelClassIdColumnName(Utf8StringCR fkColName);

                public:
                    ForeignKeyColumnInfo() {}
                    static ForeignKeyColumnInfo FromNavigationProperty(ECN::NavigationECPropertyCR);

                    Utf8StringCR GetFkColumnName() const { return m_fkColName; }
                    Utf8StringCR GetRelClassIdColumnName() const { return m_relClassIdColName; }
                };

            static Utf8CP RELECCLASSID_COLNAME_TOKEN;
            SchemaImportContext& m_ctx;
            RelationshipClassEndTableMap const& m_relationshipMap;
            std::map<DbTableId, std::vector<PartitionInfo>> m_partitions;
            std::unique_ptr<ForeignKeyMappingType> m_foreignKeyMappingType;

            BentleyStatus FinishMapping();

            const std::vector<DbColumn const*> GetPartitionColumns(PartitionInfo::ColumnId) const;
            bool PersistedEndHasNonVirtualForeignKeyColumn() const;
            bool TryGetPartition(ClassMapCR, std::vector<PartitionInfo*>&);
            PartitionInfo* CreatePartition(DbTableId);
            BentleyStatus AddIndexToRelationshipEnd(PartitionInfo const&);
            BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol);
            DbColumn* CreateForeignKeyColumn(DbTable& fkTable, NavigationPropertyMap const&, ForeignKeyColumnInfo&);
            ClassMappingStatus CreateForeignKeyConstraint(DbTable const& referencedTable);
            DbColumn* CreateRelECClassIdColumn(DbMap const&, ECN::ECClassCR, DbTable& fkTable, ForeignKeyColumnInfo const&, DbColumn const& fkCol, NavigationPropertyMap const&);
            ECN::ECRelationshipEnd GetForeignEnd() const;
            ECN::ECRelationshipEnd GetReferencedEnd() const;

            bool IsPhysicalForeignKey() const { if (m_foreignKeyMappingType == nullptr) return false; return m_foreignKeyMappingType->GetType() == RelationshipMappingType::Type::PhysicalForeignKey; }

        public:
            FkRelationships(SchemaImportContext&, RelationshipClassEndTableMap const&);

            BentleyStatus UpdatePersistedEnd(NavigationPropertyMap&);

            static BentleyStatus FinishMapping(SchemaImportContext&);
        };

    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      01/2016
    //+===============+===============+===============+===============+===============+======
    struct Tables final : NonCopyableClass
        {
        private:
            Tables();
            ~Tables();

            static DbTable* FindOrCreateTable(SchemaImportContext&, ClassMap const&, ClassMappingInfo const&, DbTable::Type, DbTable const* primaryTable);
            static DbTable* CreateTableForExistingTableStrategy(SchemaImportContext&, ClassMap const&, Utf8StringCR existingTableName, Utf8StringCR primaryKeyColName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId);
            static DbTable* CreateTableForOtherStrategies(SchemaImportContext&, ClassMap const&, Utf8StringCR tableName, DbTable::Type, Utf8StringCR primaryKeyColumnName, PersistenceType classIdColPersistenceType, ECN::ECClassId exclusiveRootClassId, DbTable const* primaryTable);
            static BentleyStatus CreateClassIdColumn(SchemaImportContext&, DbTable&, PersistenceType);
            static bool IsExclusiveRootClassOfTable(ClassMappingInfo const&);
            static BentleyStatus DetermineTablePrefix(Utf8StringR tablePrefix, ECN::ECClassCR);

        public:
            static BentleyStatus MapToTable(SchemaImportContext&, ClassMap&, ClassMappingInfo const&);
            static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
            static DbTable* CreateOverflowTable(SchemaImportContext&, DbTable const&);
            static BentleyStatus CreateIndex(SchemaImportContext&, DbTable&, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
        };
    };


END_BENTLEY_SQLITE_EC_NAMESPACE