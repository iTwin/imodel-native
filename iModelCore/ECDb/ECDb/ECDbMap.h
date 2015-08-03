/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "ECDbSql.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct StorageDescription;

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ECDbMap :NonCopyableClass
{
typedef bmap<ECN::ECClassId, ClassMapPtr> ClassMapDictionary;
typedef bmap<ECDbSqlTable*, MappedTablePtr> ClustersByTable;

public:
    struct LightWeightMapCache : NonCopyableClass
        {
        enum class RelationshipEnd
            {
            None = 0,
            Source = 1,
            Target = 2,
            Both = Source | Target
            };


        typedef  std::vector < ECN::ECClassId > ClassIds;
        typedef bmap<ECN::ECClassId, RelationshipEnd> ClassRelationshipEnds;
        typedef bmap <ECDbSqlTable const*, ClassIds> TableClasses;

        private:
            mutable bmap<ECN::ECClassId, ClassRelationshipEnds> m_relationshipEndsByClassId;
            mutable bmap<ECN::ECClassId, TableClasses> m_tablesByClassId;
            mutable ClassRelationshipEnds m_anyClassRelationships;
            mutable TableClasses m_classIdsByTable;
            mutable ClassIds m_anyClassReplacements;
            mutable ECN::ECClassId m_anyClass;
            mutable std::map<ECN::ECClassId, std::unique_ptr<StorageDescription>> m_storageDescriptions;
            mutable struct
                {
                bool m_relationshipEndsByClassIdIsLoaded : 1;
                bool m_tablesByClassIdIsLoaded : 2;
                bool m_anyClassRelationshipsIsLoaded : 3;
                bool m_classIdsByTableIsLoaded : 4;
                bool m_anyClassReplacementsLoaded : 5;
                } m_loadedFlags;

            ECDbMapCR m_map;
        private:

            void LoadDerivedClasses () const;
            void LoadClassTableClasses () const;
            void LoadAnyClassRelationships () const;
            void LoadClassRelationships (bool addAnyClassRelationships) const;
            void LoadAnyClassReplacements () const;

        public:
            LightWeightMapCache (ECDbMapCR map);

            ~LightWeightMapCache (){}
            ClassRelationshipEnds const& GetClassRelationships (ECN::ECClassId classId) const;
            ClassRelationshipEnds const& GetAnyClassRelationships () const;
            ClassIds const& GetClassesMapToTable (ECDbSqlTable const& table) const;
            TableClasses const& GetTablesMapToClass (ECN::ECClassId classId) const;
            ECN::ECClassId GetAnyClassId () const;
            ClassIds const& GetAnyClassReplacements () const;
            StorageDescription const& GetStorageDescription (ECN::ECClassId id)  const;
            void Load (bool forceReload);
            void Reset ();
        };



private:
    mutable BeMutex m_criticalSection;

    LightWeightMapCache         m_lightWeightMapCache;
    ECDbR                       m_ecdb;
    ECDbSQLManager              m_ecdbSqlManager;
    ClassMapDictionary          m_classMapDictionary;
    ClustersByTable             m_clustersByTable;
    mutable bvector<ECN::ECClassCP> m_classMapLoadTable;
    mutable int                 m_classMapLoadAccessCounter;
    SchemaImportContext*        m_schemaImportContext;
    bool                        TryGetClassMap (ClassMapPtr& classMap, ECN::ECClassCR ecClass, bool loadIfNotFound) const;
    ClassMapPtr                 DoGetClassMap (ECN::ECClassCR ecClass) const;
    ClassMapPtr                 LoadAddClassMap (ECN::ECClassCR ecClass);
    MapStatus                   DoMapSchemas (bvector<ECN::ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation);
    MapStatus                   MapClass (ECN::ECClassCR ecClass, bool forceRevaluationOfMapStrategy);
    MapStatus                   AddClassMap (ClassMapPtr& classMap);
    void                        RemoveClassMap (IClassMap const& classMap);
    bool                        FinishTableDefinition () const;
    BentleyStatus               Save();
    //! Create a table to persist ECInstances of the given ECClass in the Db
    BentleyStatus               CreateOrUpdateRequiredTables ();
    BentleyStatus               EvaluateDMLPolicyForEachClass ();
public:                        
                                explicit ECDbMap (ECDbR ecdb);
                                ~ECDbMap() {}

    ECDbSQLManager const&        GetSQLManager () const { return m_ecdbSqlManager; }
    ECDbSQLManager&              GetSQLManagerR () { return m_ecdbSqlManager; }

    bool IsImportingSchema () const;
    SchemaImportContext* GetSchemaImportContext() const;
    bool AssertIfIsNotImportingSchema() const;

    LightWeightMapCache const& GetLightWeightMapCache () const { return m_lightWeightMapCache; }
    LightWeightMapCache& GetLightWeightMapCacheR () { return m_lightWeightMapCache; }
    ECN::ECClassCR              GetClassForPrimitiveArrayPersistence (ECN::PrimitiveType primitiveType) const;
    bool                        ContainsMappingsForSchema (ECN::ECSchemaCR ecSchema);
    ECDbR                       GetECDbR () const { return m_ecdb; }
    MapStatus                   MapSchemas (SchemaImportContext& importSchemaContext, bvector<ECN::ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation);

    ClassMapPtr                 LoadClassMap (bmap<ECN::ECClassId, ECN::ECClassCP>& currentlyLoadingClasses, ECN::ECClassCR ecClass);

    //! Gets the class map for the specified ECClass.
    //! @remarks if @p loadIfNotFound is true, the method never returns null for ECClasses which had been 
    //! imported into the ECDb file. Even for classes that
    //! are not mapped to a table, a class map is returned (an UnmappedClassMap).
    //! So the method only returns nullptr if the ECSchema of the specified ecClass has not been imported into the ECDb file yet.
    //! @param[in] ecClass ECClass for which the class map is to be retrieved.
    //! @param[in] loadIfNotFound if true, the class map is loaded from the ECDb file if it wasn't loaded yet. if false,
    //! the class map is not loaded from the ECDb file. In this case only class maps are found that have already been loaded into memory.
    //! @return Class map or nullptr if @p ecClass's ECSchema was not imported into the ECDb file or if @p loadIfNotFound is true
    //! and the class map was not loaded yet from the ECDb file.
    IClassMap const*            GetClassMap (ECN::ECClassCR ecClass, bool loadIfNotFound = true) const;
    
    //! @copydoc ECDbMap::GetClassMap
    ClassMapP                   GetClassMapP (ECN::ECClassCR ecClass, bool loadIfNotFound = true) const;

    //! @copydoc ECDbMap::GetClassMap
    ClassMapCP                  GetClassMapCP (ECN::ECClassCR ecClass, bool loadIfNotFound = true) const;

    ECDbSqlTable*               FindOrCreateTable (Utf8CP tableName, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToSecondaryTable, bool mapToExisitingTable) ;
    MappedTableP                GetMappedTable (ClassMapCR classMap, bool createMappedTableEntryIfNotFound = true);
    //! The values returned by GetPrimitiveTypeName are intended only for logging and debugging purposes
    static WCharCP              GetPrimitiveTypeName (ECN::PrimitiveType primitiveType);

    //!Loads the class maps if they were not loaded yet
    void                        GetClassMapsFromRelationshipEnd (bset<IClassMap const*>& endClassMaps, ECN::ECRelationshipConstraintCR relationshipEnd, bool loadIfNotFound) const;
    std::vector<ECN::ECClassCP> GetClassesFromRelationshipEnd (ECN::ECRelationshipConstraintCR) const;
    size_t                      GetTableCountOnRelationshipEnd (ECN::ECRelationshipConstraintCR) const;
    void                        ClearCache();
    ClassMapCP GetClassMap (ECN::ECClassId ecClassId);
    RelationshipClassMapCP GetRelationshipClassMap (ECN::ECClassId ecRelationshipClassId);
    };


    //=======================================================================================
    //! Hold detail about how table partition is described for this class
    // @bsiclass                                               Affan.Khan           05/2015
    //+===============+===============+===============+===============+===============+======
    struct HorizontalPartition : NonCopyableClass
        {
        private:
            ECDbSqlTable const* m_table;
            std::vector<ECN::ECClassId> m_partitionClassIds;
            std::vector<ECN::ECClassId> m_inversedPartitionClassIds;
            bool m_hasInversedPartitionClassIds;

        public:
            explicit HorizontalPartition (ECDbSqlTable const& table) : m_table (&table), m_hasInversedPartitionClassIds (false) {}
            ~HorizontalPartition () {}
            HorizontalPartition (HorizontalPartition&& rhs);
            HorizontalPartition& operator=(HorizontalPartition&& rhs);

            ECDbSqlTable const& GetTable () const { return *m_table; }
            std::vector<ECN::ECClassId> const& GetClassIds () const { return m_partitionClassIds; }

            void AddClassId (ECN::ECClassId classId) { m_partitionClassIds.push_back (classId); }
            void GenerateClassIdFilter (std::vector<ECN::ECClassId> const& tableClassIds);

            bool NeedsClassIdFilter () const;
            void AppendECClassIdFilterSql (NativeSqlBuilder&) const;
        };


    //=======================================================================================
    //! Represents storage description for a given class map and its derived classes for polymorphic queries
    // @bsiclass                                               Affan.Khan           05/2015
    //+===============+===============+===============+===============+===============+======
    struct StorageDescription : NonCopyableClass
        {
        private:
            ECN::ECClassId m_classId;
            std::vector<HorizontalPartition> m_horizontalPartitions;
            std::vector<size_t> m_nonVirtualHorizontalPartitionIndices;
            size_t m_rootHorizontalPartitionIndex;

            explicit StorageDescription (ECN::ECClassId classId) : m_classId (classId), m_rootHorizontalPartitionIndex (0) {}

            size_t AddHorizontalPartition (ECDbSqlTable const& table, bool isRootPartition);
        public:
            ~StorageDescription (){}
            StorageDescription (StorageDescription&&);
            StorageDescription& operator=(StorageDescription&&);

            HorizontalPartition const* GetHorizontalPartition (size_t index) const;
            HorizontalPartition* GetHorizontalPartitionP (size_t index);
            std::vector<HorizontalPartition> const& GetHorizontalPartitions () const { return m_horizontalPartitions; }
            HorizontalPartition const& GetRootHorizontalPartition () const { return *GetHorizontalPartition (m_rootHorizontalPartitionIndex); }
            std::vector<size_t> const& GetNonVirtualHorizontalPartitionIndices () const { return m_nonVirtualHorizontalPartitionIndices; }
            ECN::ECClassId GetClassId () const { return m_classId; }

            static std::unique_ptr<StorageDescription> Create (ECN::ECClassId classId, ECDbMap::LightWeightMapCache const& lwmc);

        };
END_BENTLEY_SQLITE_EC_NAMESPACE
