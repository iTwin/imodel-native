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
//=======================================================================================
// @bsiclass                                                Affan.Khan      07/2017
//+===============+===============+===============+===============+===============+======
struct ForeignKeyPartitions final : NonCopyableClass
    {
    enum class PersistedEnd
        {
        SourceTable,
        TargetTable
        };
    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct NavigationInfo final
        {
        private:
            DbColumn const& m_idColumn;
            DbColumn const& m_relECClassIdColumn;
        public:
            NavigationInfo(DbColumn const& id, DbColumn const& relECClassId)
                :m_idColumn(id), m_relECClassIdColumn(relECClassId)
                {}

            DbColumn const& GetIdColumn() const { return m_idColumn; }
            DbColumn const& GetRelECClassIdColumn() const { return m_relECClassIdColumn; }
            bool operator == (NavigationInfo const& rhs) const
                {
                return GetIdColumn().GetId() == rhs.GetIdColumn().GetId() && GetRelECClassIdColumn().GetId() == rhs.GetRelECClassIdColumn().GetId();
                }
        };
    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct Partition final : NonCopyableClass
        {
        friend struct ForeignKeyPartitions;
        private:
            enum  ColumnId
                {
                ECInstanceId = 0,
                ECClassId = 1,
                SourceECInstanceId = 2,
                SourceECClassId = 3,
                TargetECInstanceId = 4,
                TargetECClassId = 5
                };

            uint64_t m_hashCode;
            DbColumn const* m_cols[6];
            ForeignKeyPartitions const& m_fkInfo;
            bool m_persisted;
        private:
            explicit Partition(ForeignKeyPartitions const&);
            void UpdateHash();
            DbColumn const* GetColumn(ColumnId) const;
            BentleyStatus SetColumn(ColumnId, DbColumn const*);
            static uint64_t QuickHash64(Utf8CP, uint64_t);
            void SetFromECClassIdColumn(DbColumn const* column);
            void MarkPersisted() { m_persisted = true; }

        public:
            ~Partition() {}
            uint64_t GetHashCode() const { return m_hashCode; }
            //Relationship cannonical view
            DbColumn const& GetECInstanceIdColumn() const { return *GetColumn(ColumnId::ECInstanceId); }
            DbColumn const& GetECClassIdColumn() const { return *GetColumn(ColumnId::ECClassId); }
            DbColumn const& GetSourceECInstanceIdColumn() const { return *GetColumn(ColumnId::SourceECInstanceId); }
            DbColumn const* GetSourceECClassIdColumn() const { return GetColumn(ColumnId::SourceECClassId); }
            DbColumn const& GetTargetECInstanceIdColumn() const { return *GetColumn(ColumnId::TargetECInstanceId); }
            DbColumn const* GetTargetECClassIdColumn() const { return GetColumn(ColumnId::TargetECClassId); }
            //Usefull relationship view
            DbColumn const& GetFromECInstanceIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::TargetTable ? GetSourceECInstanceIdColumn() : GetTargetECInstanceIdColumn(); }
            DbColumn const* GetFromECClassIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::TargetTable ? GetSourceECClassIdColumn() : GetTargetECClassIdColumn(); }
            DbColumn const& GetToECInstanceIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::SourceTable ? GetSourceECInstanceIdColumn() : GetTargetECInstanceIdColumn(); }
            DbColumn const& GetToECClassIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::SourceTable ? *GetSourceECClassIdColumn() : *GetTargetECClassIdColumn(); }
            NavigationInfo GetNavigationColumns() const { return NavigationInfo(GetFromECInstanceIdColumn(), GetECClassIdColumn()); }
            bool IsConcrete() const;
            bool IsPhysical() const;
            bool IsPersisted() const { return m_persisted; }
            static std::unique_ptr<Partition> Create(ForeignKeyPartitions const&, DbColumn const&, DbColumn const&);

        };

    private:
        MapStrategy m_mapStrategy;
        DbColumn const* m_fromClassIdColumn;
        ECN::ECRelationshipClassCR m_relationshipClass;
        std::vector<std::unique_ptr<Partition>> m_partitions;
        ECDbCR m_ecdb;
        bool m_readonly;
        bool m_updateFromECClassIdColumnOnInsert;

    private:
        ForeignKeyPartitions(ECDbCR, ECN::ECRelationshipClassCR, MapStrategy);
        static std::unique_ptr<ForeignKeyPartitions> Create(ECDbCR, ECN::ECRelationshipClassCR, MapStrategy, bool);
        static BentleyStatus GetMapStrategy(MapStrategy &, ECDbCR, ECN::ECRelationshipClassCR);
        BentleyStatus TryGetFromECClassIdColumn(DbColumn const*& column) const;

    public:
        ~ForeignKeyPartitions() {}
        void UpdateFromECClassIdColumnOnInsert(bool enable) { m_updateFromECClassIdColumnOnInsert = enable; }
        PersistedEnd GetPersistedEnd() const;
        bool Readonly() const { return m_readonly; }
        bool Contains(Partition const&) const;
        BentleyStatus UpdateFromECClassIdColumn();
        Partition const* FindCompatiblePartiton(NavigationPropertyMap const&) const;
        const std::vector<Partition const*> GetPartitions(bool onlyPhysical = false, bool onlyConcrete = false) const;
        const std::vector<Partition const*> GetPartitions(DbTable const&, bool onlyPhysical = false, bool onlyConcrete = false) const;
        BentleyStatus Insert(std::unique_ptr<Partition> partition);
        static std::unique_ptr<ForeignKeyPartitions> CreateReadonly(ECDbCR, ECN::ECRelationshipClassCR);
        static std::unique_ptr<ForeignKeyPartitions> Create(ECDbCR, ECN::ECRelationshipClassCR, MapStrategy);
        static std::vector<DbTable const*> GetOtherEndTables(ECDbCR, ECN::ECRelationshipClassCR, MapStrategy);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan      07/2017
//+===============+===============+===============+===============+===============+======
struct FkRelationshipMappingInfo final : NonCopyableClass
    {
    public:
        struct Collection final
            {
            private:
                std::map<ECN::ECClassId, std::unique_ptr<FkRelationshipMappingInfo>> m_fkRelMappingInfos;
                std::vector<FkRelationshipMappingInfo const*> m_orderedList;

            public:
                Collection() {}

                std::vector<FkRelationshipMappingInfo const*> const& Get() const { return m_orderedList; }

                bool TryGet(FkRelationshipMappingInfo*& fkRelMappingInfo, ECN::ECClassId relClassId) const 
                    {
                    auto it = m_fkRelMappingInfos.find(relClassId);
                    if (it != m_fkRelMappingInfos.end())
                        {
                        fkRelMappingInfo = it->second.get();
                        return true;
                        }

                    fkRelMappingInfo = nullptr;
                    return false;
                    }

                FkRelationshipMappingInfo& Add(ECN::ECRelationshipClassCR relClass)
                    {
                    BeAssert(m_fkRelMappingInfos.find(relClass.GetId()) == m_fkRelMappingInfos.end());

                    std::unique_ptr<FkRelationshipMappingInfo> newFkRelMappingInfo = std::make_unique<FkRelationshipMappingInfo>(relClass);
                    FkRelationshipMappingInfo& fkRelMappingInfoR = *newFkRelMappingInfo;
                    m_fkRelMappingInfos[relClass.GetId()] = std::move(newFkRelMappingInfo);

                    m_orderedList.push_back(&fkRelMappingInfoR);
                    return fkRelMappingInfoR;
                    }
            };

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
                bool m_isPersisted = false;

            public:
                PartitionInfo(){ Clear(); }
                explicit PartitionInfo(RelationshipClassEndTableMap::Partition const& partition);

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

    private:
        static Utf8CP RELECCLASSID_COLNAME_TOKEN;

        ECN::ECRelationshipClassCR m_relClass;
        RelationshipClassEndTableMap const* m_relClassMap = nullptr;
        std::map<DbTableId, std::vector<PartitionInfo>> m_partitions;
        ForeignKeyConstraintCustomAttribute m_fkConstraintCA;
        bool m_fkConstraintCAIsRead = false;

    public:
        explicit FkRelationshipMappingInfo(ECN::ECRelationshipClassCR relClass) : m_relClass(relClass) {}
        void AssignClassMap(RelationshipClassEndTableMap const&);

        void SetModified() { BeAssert(m_relClassMap != nullptr); const_cast<RelationshipClassEndTableMap*>(m_relClassMap)->Modified(); }
        ECN::ECRelationshipClassCR GetRelClass() const { return m_relClass; }
        RelationshipClassEndTableMap const& GetRelClassMap() const { BeAssert(m_relClassMap != nullptr);  return *m_relClassMap; }
        bool IsFkConstraintCARead() const { return m_fkConstraintCAIsRead; }
        ForeignKeyConstraintCustomAttribute const& GetFkConstraintCA() const { return m_fkConstraintCA; }
        void ReadFkConstraintCA(SchemaImportContext&, ECN::NavigationECPropertyCR);

        bool IsPhysicalForeignKey() const { return m_fkConstraintCA.IsValid(); }

        ECN::ECRelationshipEnd GetFkEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;

        PartitionInfo* CreatePartition(DbTableId);
        std::map<DbTableId, std::vector<PartitionInfo>> const& GetPartitions() const { return m_partitions; }
        const std::vector<DbColumn const*> GetPartitionColumns(PartitionInfo::ColumnId) const;
        bool TryGetPartition(ClassMapCR, std::vector<PartitionInfo const*>&) const;
    };

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

            Classes();
            ~Classes();

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

            static BentleyStatus MapUserDefinedIndex(SchemaImportContext&, ClassMap const&, DbIndexListCustomAttribute::DbIndex const&);

        public:
            static PropertyMap* MapProperty(SchemaImportContext& importCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(importCtx, classMap); return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP This should be moved out of here. It seems like it doesn't have to do with mapping. And if it has, it needs to be renamed/refactored
            static PropertyMap* LoadPropertyMap(DbClassMapLoadContext const& loadCtx, ClassMap& classMap, ECN::ECPropertyCR prop) { Context ctx(loadCtx, classMap);  return ProcessProperty(ctx, prop); }
            //WIP_CLEANUP Moving the below MapNavigationProperty from elsewhere here makes evident that we now have 2 equally named methods
            //for mapping nav props, plus a third method related to nav props. This needs to be cleaned up.
            static ClassMappingStatus MapNavigationProperty(SchemaImportContext&, NavigationPropertyMap&);

            static BentleyStatus MapUserDefinedIndexes(SchemaImportContext&, ClassMap const&);

            static BentleyStatus TryDetermineRelationshipMappingType(RelationshipMappingType&, SchemaImportContext const&, ECN::ECRelationshipClassCR);
            static BentleyStatus FailIfConstraintClassIsNotMapped(SchemaImportContext&, ECN::ECRelationshipClassCR);

            //! Rules:
            //! If MapStrategy != TPH: NotInherited
            //! Else: Clone
            static PropertyMapInheritanceMode GetPropertyMapInheritanceMode(MapStrategyExtendedInfo const&);
        };

    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct FkRelationships final
        {
        private:
            FkRelationships();
            ~FkRelationships();

            static BentleyStatus AddIndexToRelationshipEnd(SchemaImportContext&, FkRelationshipMappingInfo const&, FkRelationshipMappingInfo::PartitionInfo const&);
            static DbColumn* CreateForeignKeyColumn(FkRelationshipMappingInfo::ForeignKeyColumnInfo&, SchemaImportContext&, FkRelationshipMappingInfo const&, DbTable& fkTable, NavigationPropertyMap const&);
            static BentleyStatus CreateForeignKeyConstraint(SchemaImportContext&, FkRelationshipMappingInfo const&, DbTable const& referencedTable);
            static DbColumn* CreateRelECClassIdColumn(SchemaImportContext&, FkRelationshipMappingInfo const&, FkRelationshipMappingInfo::ForeignKeyColumnInfo const&, DbTable& fkTable, DbColumn const& fkCol, NavigationPropertyMap const&);
            static BentleyStatus FinishMapping(SchemaImportContext&, FkRelationshipMappingInfo const&);
            static BentleyStatus ValidateForeignKeyColumn(SchemaImportContext&, FkRelationshipMappingInfo const&, DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol);
            static BentleyStatus ValidateRelClass(SchemaImportContext&, ECN::ECRelationshipClassCR);

        public:
            static BentleyStatus Initialize(SchemaImportContext&, FkRelationshipMappingInfo&);

            static BentleyStatus UpdatePersistedEnd(SchemaImportContext&, FkRelationshipMappingInfo&, NavigationPropertyMap&);

            static BentleyStatus FinishMapping(SchemaImportContext&);

            static BentleyStatus TryDetermineFkEnd(ECN::ECRelationshipEnd&, ECN::ECRelationshipClassCR, IssueReporter const&);

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
            static BentleyStatus CreateVirtualTableForFkRelationship(SchemaImportContext&, RelationshipClassEndTableMap&, ClassMappingInfo const&);
            static BentleyStatus DetermineTableName(Utf8StringR tableName, ECN::ECClassCR, Utf8CP tablePrefix = nullptr);
            static DbTable* CreateOverflowTable(SchemaImportContext&, DbTable const&);
            static BentleyStatus CreateIndex(SchemaImportContext&, DbTable&, Utf8StringCR indexName, bool isUnique, std::vector<DbColumn const*> const&, bool addIsNotNullWhereExp, bool isAutoGenerated, ECN::ECClassId, bool applyToSubclassesIfPartial = true);
        };
    };


END_BENTLEY_SQLITE_EC_NAMESPACE