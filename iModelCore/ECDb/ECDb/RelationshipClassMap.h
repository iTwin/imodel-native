/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SystemPropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintMap
    {
    private:
        ECN::ECRelationshipConstraintCR m_constraint;
        ConstraintECInstanceIdPropertyMap const* m_ecInstanceIdPropMap = nullptr;
        ConstraintECClassIdPropertyMap const* m_ecClassIdPropMap = nullptr;

        //not copyable
        RelationshipConstraintMap(RelationshipConstraintMap const&) = delete;
        RelationshipConstraintMap& operator=(RelationshipConstraintMap const&) = delete;

    public:
        explicit RelationshipConstraintMap(ECN::ECRelationshipConstraintCR constraint) :  m_constraint(constraint) {}
        ConstraintECInstanceIdPropertyMap const* GetECInstanceIdPropMap() const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap(ConstraintECInstanceIdPropertyMap const* ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        ConstraintECClassIdPropertyMap const* GetECClassIdPropMap() const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap(ConstraintECClassIdPropertyMap const* ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }
        ECN::ECRelationshipConstraintCR GetRelationshipConstraint() const { return m_constraint; }
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassMap : ClassMap
    {
    protected:
        static Utf8CP const DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
        static Utf8CP const DEFAULT_SOURCEECCLASSID_COLUMNNAME;
        static Utf8CP const DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
        static Utf8CP const DEFAULT_TARGETECCLASSID_COLUMNNAME;

        RelationshipConstraintMap m_sourceConstraintMap;
        RelationshipConstraintMap m_targetConstraintMap;

        RelationshipClassMap(ECDb const&, TableSpaceSchemaManager const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        RelationshipConstraintMap& GetConstraintMapR(ECN::ECRelationshipEnd constraintEnd);

    public:
        virtual ~RelationshipClassMap() {}

        ECN::ECRelationshipClassCR GetRelationshipClass() const { return *(GetClass().GetRelationshipClassCP()); }
        RelationshipConstraintMap const& GetConstraintMap(ECN::ECRelationshipEnd constraintEnd) const;
        ConstraintECInstanceIdPropertyMap const* GetConstraintECInstanceIdPropMap(ECN::ECRelationshipEnd constraintEnd) const;
        ConstraintECClassIdPropertyMap const* GetConstraintECClassIdPropMap(ECN::ECRelationshipEnd) const;

        ConstraintECInstanceIdPropertyMap const* GetSourceECInstanceIdPropMap() const { return m_sourceConstraintMap.GetECInstanceIdPropMap(); }
        ConstraintECClassIdPropertyMap const* GetSourceECClassIdPropMap() const { return m_sourceConstraintMap.GetECClassIdPropMap(); }
        ConstraintECInstanceIdPropertyMap const* GetTargetECInstanceIdPropMap() const { return m_targetConstraintMap.GetECInstanceIdPropMap(); }
        ConstraintECClassIdPropertyMap const* GetTargetECClassIdPropMap() const { return m_targetConstraintMap.GetECClassIdPropMap(); }
    };

typedef RelationshipClassMap const& RelationshipClassMapCR;
/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap final : RelationshipClassMap
    {
    private:
        ClassMappingStatus _Map(ClassMappingContext&) override;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        RelationshipClassEndTableMap const* GetBaseClassMap(SchemaImportContext* ctx = nullptr) const;
        ClassMappingStatus MapSubClass(RelationshipClassEndTableMap const& baseClassMap);

    public:
        RelationshipClassEndTableMap(ECDb const& ecdb, TableSpaceSchemaManager const& manager, ECN::ECClassCR relClass, MapStrategyExtendedInfo const& mapStrategy) : RelationshipClassMap(ecdb, manager, Type::RelationshipEndTable, relClass, mapStrategy) {}
        ~RelationshipClassEndTableMap() {}
        ECN::ECRelationshipEnd GetForeignEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan      07/2017
//+===============+===============+===============+===============+===============+======
struct ForeignKeyPartitionView final
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
            bool operator==(NavigationInfo const& rhs) const { return GetIdColumn() == rhs.GetIdColumn() && GetRelECClassIdColumn() == rhs.GetRelECClassIdColumn(); }
            bool operator!=(NavigationInfo const& rhs) const { return !(*this == rhs); }
        };

    //=======================================================================================
    // @bsiclass                                                Affan.Khan      07/2017
    //+===============+===============+===============+===============+===============+======
    struct Partition final
        {
        friend struct ForeignKeyPartitionView;
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

            ForeignKeyPartitionView const& m_fkInfo;
            uint64_t m_hashCode = 0;
            DbColumn const* m_cols[6];
            bool m_persisted = false;

            explicit Partition(ForeignKeyPartitionView const&);
            //not copyable
            Partition(Partition const&) = delete;
            Partition& operator=(Partition const&) = delete;

            void UpdateHash();
            DbColumn const* GetColumn(ColumnId) const;
            BentleyStatus SetColumn(ColumnId, DbColumn const*);
            static uint64_t QuickHash64(Utf8CP, uint64_t);
            void SetFromECClassIdColumn(DbColumn const* column);
            void MarkPersisted() { m_persisted = true; }

        public:
            ~Partition() {}
            uint64_t GetHashCode() const { return m_hashCode; }
            //Relationship canonical view
            DbTable const& GetTable() const { return GetECInstanceIdColumn().GetTable(); }
            DbTable const* GetOtherEndTable() const { return GetFromECClassIdColumn() == nullptr ? nullptr : &GetFromECClassIdColumn()->GetTable(); }
            DbColumn const& GetECInstanceIdColumn() const { return *GetColumn(ColumnId::ECInstanceId); }
            DbColumn const& GetECClassIdColumn() const { return *GetColumn(ColumnId::ECClassId); }
            DbColumn const& GetSourceECInstanceIdColumn() const { return *GetColumn(ColumnId::SourceECInstanceId); }
            DbColumn const* GetSourceECClassIdColumn() const { return GetColumn(ColumnId::SourceECClassId); }
            DbColumn const& GetTargetECInstanceIdColumn() const { return *GetColumn(ColumnId::TargetECInstanceId); }
            DbColumn const* GetTargetECClassIdColumn() const { return GetColumn(ColumnId::TargetECClassId); }
            //Useful relationship view
            DbColumn const& GetFromECInstanceIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::TargetTable ? GetSourceECInstanceIdColumn() : GetTargetECInstanceIdColumn(); }
            DbColumn const* GetFromECClassIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::TargetTable ? GetSourceECClassIdColumn() : GetTargetECClassIdColumn(); }
            DbColumn const& GetToECInstanceIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::SourceTable ? GetSourceECInstanceIdColumn() : GetTargetECInstanceIdColumn(); }
            DbColumn const& GetToECClassIdColumn() const { return m_fkInfo.GetPersistedEnd() == PersistedEnd::SourceTable ? *GetSourceECClassIdColumn() : *GetTargetECClassIdColumn(); }
            NavigationInfo GetNavigationColumns() const { return NavigationInfo(GetFromECInstanceIdColumn(), GetECClassIdColumn()); }
            bool IsConcrete() const;
            bool IsPhysical() const;
            bool IsPersisted() const { return m_persisted; }
            static std::unique_ptr<Partition> Create(ForeignKeyPartitionView const&, DbColumn const&, DbColumn const&);

        };

    private:
        TableSpaceSchemaManager const& m_schemaManager;
        MapStrategy m_mapStrategy;
        DbColumn const* m_fromClassIdColumn = nullptr;
        ECN::ECRelationshipClassCR m_relationshipClass;
        std::vector<std::unique_ptr<Partition>> m_partitions;
        bool m_readonly = false;
        bool m_updateFromECClassIdColumnOnInsert = true;

        ForeignKeyPartitionView(TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR, MapStrategy);
        //not copyable
        ForeignKeyPartitionView(ForeignKeyPartitionView const&) = delete;
        ForeignKeyPartitionView& operator=(ForeignKeyPartitionView const&) = delete;

        static std::unique_ptr<ForeignKeyPartitionView> Create(TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR, MapStrategy, bool);

        BentleyStatus TryGetFromECClassIdColumn(DbColumn const*& column) const;

        static BentleyStatus GetMapStrategy(MapStrategy&, TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR);
        static ECN::ECRelationshipClassCR GetRootClass(ECN::ECRelationshipClassCR ecRelationshipClass);

    public:
        ~ForeignKeyPartitionView() {}

        static std::unique_ptr<ForeignKeyPartitionView> CreateReadonly(TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR);
        static std::unique_ptr<ForeignKeyPartitionView> Create(TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR, MapStrategy);

        MapStrategy GetMapStrategy() const { return m_mapStrategy; }
        bool Readonly() const { return m_readonly; }

        PersistedEnd GetPersistedEnd() const { return m_mapStrategy == MapStrategy::ForeignKeyRelationshipInSourceTable ? PersistedEnd::SourceTable : PersistedEnd::TargetTable; }

        void UpdateFromECClassIdColumnOnInsert(bool enable) { m_updateFromECClassIdColumnOnInsert = enable; }
        bool Contains(Partition const&) const;
        BentleyStatus UpdateFromECClassIdColumn();
        Partition const* FindCompatiblePartiton(NavigationPropertyMap const&) const;
        std::vector<Partition const*> GetPartitions(bool onlyPhysical = false, bool onlyConcrete = false) const;
        std::vector<Partition const*> GetPartitions(DbTable const&, bool onlyPhysical = false, bool onlyConcrete = false) const;
        BentleyStatus Insert(std::unique_ptr<Partition> partition);

        //! param[out] otherEndTable Other end table or nullptr if not found.
        //! @return SUCCESS in case of success (also if otherEndTable is returned as nullptr). ERROR in case of errors during the retrieval
        static BentleyStatus TryGetOtherEndTable(DbTable const*& otherEndTable, TableSpaceSchemaManager const&, ECN::ECRelationshipClassCR, MapStrategy);
    };

/*==========================================================================
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassLinkTableMap final : RelationshipClassMap
    {
    friend struct ClassMapFactory;

    private:
        enum class RelationshipIndexSpec
            {
            Source,
            Target,
            SourceAndTarget, 
            SourceAndTargetAndClassId
            };

    private:
        ClassMappingStatus _Map(ClassMappingContext&) override;
        ClassMappingStatus MapSubClass(ClassMappingContext&);
        ClassMappingStatus CreateConstraintPropMaps(SchemaImportContext&, LinkTableRelationshipMapCustomAttribute const&, bool addSourceECClassIdColumnToTable, bool addTargetECClassIdColumnToTable);
        void AddIndices(SchemaImportContext&, bool allowDuplicateRelationship);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECRelationshipConstraintCR) const;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        DbColumn* ConfigureForeignECClassIdKey(SchemaImportContext&, LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4, DbColumn const* col5);
        
        static Utf8String DetermineConstraintECInstanceIdColumnName(LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        static Utf8String DetermineConstraintECClassIdColumnName(LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        //AllowDuplicateRelationships flag is inherited from root rel class to actual rel class
        static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);

        static bool GetAllowDuplicateRelationshipsFlag(Nullable<bool> const& allowDuplicateRelationshipFlag) { return allowDuplicateRelationshipFlag.IsNull() ? false : allowDuplicateRelationshipFlag.Value(); }
        void AddDefaultIndexes(SchemaImportContext& ctx);
    public:
        RelationshipClassLinkTableMap(ECDb const&, TableSpaceSchemaManager const&, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        ~RelationshipClassLinkTableMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
