/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct RelationshipConstraintMap : NonCopyableClass
    {
    private:
        ECN::ECRelationshipConstraintCR m_constraint;
        ConstraintECInstanceIdPropertyMap const* m_ecInstanceIdPropMap;
        ConstraintECClassIdPropertyMap const* m_ecClassIdPropMap;

    public:
        explicit RelationshipConstraintMap(ECN::ECRelationshipConstraintCR constraint)
            :  m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr)
            {}
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

        RelationshipClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);
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
    friend struct ClassMapFactory;
    struct Partition : NonCopyableClass
        {
        private:
            const DbColumn &m_ecInstanceId, &m_ecClassId, &m_sourceId, *m_sourceClassId, &m_targetId, *m_targetClassId;
            uint64_t m_hashCode;

        private:
            static  uint64_t QuickHash64(Utf8CP str, uint64_t mix = 0);
            Partition(DbColumn const& ecInstanceId, DbColumn const& ecClassId, DbColumn const& sourceId, DbColumn const* sourceClassId, DbColumn const& targetId, DbColumn const* targetClassId);

        public:
            ~Partition(){}
            DbTable const& GetTable() const { return m_ecInstanceId.GetTable(); }
            DbColumn const& GetECInstanceId() const { return m_ecInstanceId; }
            DbColumn const& GetECClassId() const { return m_ecClassId; }
            DbColumn const& GetSourceECInstanceId() const { return m_sourceId; }
            DbColumn const* GetSourceECClassId() const { return m_sourceClassId; }
            DbColumn const& GetTargetECInstanceId() const { return m_targetId; }
            DbColumn const* GetTargetECClassId() const { return m_targetClassId; }
            DbColumn const& GetConstraintECInstanceId(ECN::ECRelationshipEnd end) const { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECInstanceId() : GetTargetECInstanceId();}
            DbColumn const* GetConstraintECClassId(ECN::ECRelationshipEnd end) const { return end == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? GetSourceECClassId() : GetTargetECClassId(); }
            uint64_t GetHashCode() const { return m_hashCode; }
            bool CanQuery() const { return GetTable().GetType() != DbTable::Type::Virtual && GetTargetECClassId() != nullptr && GetSourceECClassId() != nullptr; }
            static std::unique_ptr<Partition> Create(DbColumn const& ecInstanceId, DbColumn const& ecClassId, DbColumn const& sourceId, DbColumn const* sourceClassId, DbColumn const& targetId, DbColumn const* targetClassId, ECN::ECRelationshipEnd fromEnd);
        };

    struct PartitionView : NonCopyableClass
        {
        private:
            RelationshipClassEndTableMap const& m_relationshipMap;
            struct ComparePartition { bool operator()(std::unique_ptr<Partition> const& lhs, std::unique_ptr<Partition> const& rhs) const { return lhs->GetHashCode() < rhs->GetHashCode(); } };
            std::map <DbTableId, std::set <std::unique_ptr<Partition>, ComparePartition>> m_partitionMap;
            bool m_loadedPartitions;
            
            BentleyStatus InsertPartition(std::unique_ptr<Partition> partition, bool assertAndFailOnDuplicatePartition);
            BentleyStatus ResurrectPartition(std::vector<DbTable const*> const& tables, DbColumn const& navId, DbColumn const& navRelECClassId);
            BentleyStatus Load();
            ECN::ECRelationshipEnd GetToEnd() const;
            ECN::ECRelationshipEnd GetFromEnd() const;
            PartitionView(RelationshipClassEndTableMap const& relationshipMap);
            BentleyStatus AddDefaultPartition();
        public:
            const std::vector <DbTable const*> GetTables(bool skipVirtualPartition) const;
            const std::map <DbTable const*, std::vector<Partition const*>> GetPartitionMap() const;
            const std::vector<Partition const*> GetPartitions(bool skipVirtualPartition) const;
            const std::vector<Partition const*> GetPartitions(DbTable const& toEnd, bool skipVirtualPartition) const;
            static std::unique_ptr< PartitionView> Create(RelationshipClassEndTableMap const& relationMap);
            std::vector<Partition const*> GetPhysicalPartitions() const;
            static std::vector<DbTable const*> GetOtherEndTables(RelationshipClassEndTableMap const&);
        };

    private:
        mutable std::unique_ptr<PartitionView> m_partitionCollection;

    private:
        RelationshipClassEndTableMap(ECDb const& ecdb, ECN::ECClassCR relClass, MapStrategyExtendedInfo const& mapStrategy) : RelationshipClassMap(ecdb, Type::RelationshipEndTable, relClass, mapStrategy) {}
        ClassMappingStatus _Map(ClassMappingContext&) override;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        RelationshipClassEndTableMap const* GetBaseClassMap(SchemaImportContext * ctx = nullptr) const;
        ClassMappingStatus MapSubClass(RelationshipClassEndTableMap const& baseClassMap);
    public:
        ~RelationshipClassEndTableMap() {}
        ECN::ECRelationshipEnd GetForeignEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;
        PartitionView const& GetPartitionView() const;
        bool IsRelationshipSubclass() const { return GetClass().HasBaseClasses(); }
        void ResetPartitionCache() const { m_partitionCollection = nullptr; }
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
            SourceAndTarget
            };

    private:
        RelationshipClassLinkTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        ClassMappingStatus _Map(ClassMappingContext&) override;
        ClassMappingStatus MapSubClass(ClassMappingContext&, RelationshipMappingInfo const&);
        ClassMappingStatus CreateConstraintPropMaps(ClassMappingContext&, RelationshipMappingInfo const&, bool addSourceECClassIdColumnToTable, bool addTargetECClassIdColumnToTable);
        void AddIndices(ClassMappingContext&, bool allowDuplicateRelationship);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECRelationshipConstraintCR) const;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        DbColumn* ConfigureForeignECClassIdKey(ClassMappingContext&, RelationshipMappingInfo const&, ECN::ECRelationshipEnd);
        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4);
        static Utf8String DetermineConstraintECInstanceIdColumnName(LinkTableMappingType const&, ECN::ECRelationshipEnd);
        static Utf8String DetermineConstraintECClassIdColumnName(LinkTableMappingType const&, ECN::ECRelationshipEnd);
        static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);
    public:
        ~RelationshipClassLinkTableMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
