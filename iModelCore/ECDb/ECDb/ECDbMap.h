/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"
#include "SchemaImportContext.h"
#include "ECDbSql.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct ECDbMap :NonCopyableClass
{
typedef bmap<ECN::ECClassId, ClassMapPtr>       ClassMapDictionary;
typedef bmap<ECDbSqlTable*, MappedTablePtr>          ClustersByTable;

private:
    mutable BeCriticalSection   m_criticalSection;
    ECDbR                       m_ecdb;
    ECDbSQLManager              m_ecdbSqlManager;
    ClassMapDictionary          m_classMapDictionary;

    mutable bmap<Utf8CP, ECDbSqlTable*, CompareIUtf8>  m_tableCache; //table names must be case insenstive

    ClustersByTable             m_clustersByTable;
    mutable bvector<ECN::ECClassCP> m_classMapLoadTable;
    mutable int                 m_classMapLoadAccessCounter;

    bool                        TryGetClassMap (ClassMapPtr& classMap, ECN::ECClassCR ecClass, bool loadIfNotFound) const;
    ClassMapPtr                 DoGetClassMap (ECN::ECClassCR ecClass) const;
    ClassMapPtr                 LoadAddClassMap (ECN::ECClassCR ecClass);
    MapStatus                   DoMapSchemas (SchemaImportContext const& schemaImportContext, bvector<ECN::ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation);
    MapStatus                   MapClass (SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, bool forceRevaluationOfMapStrategy);
    MapStatus                   AddClassMap (ClassMapPtr& classMap);
    void                        RemoveClassMap (IClassMap const& classMap);
    CreateTableStatus           FinishTableDefinition (ECN::ECClassCR ecClass) const;
    CreateTableStatus           FinishTableDefinition (ECDbSqlTable& table) const;
    bool                        FinishTableDefinition () const;
    DbResult                    Save ();
    //! Create a table to persist ECInstances of the given ECClass in the Db
    CreateTableStatus           CreateOrUpdateRequiredTables ();
    void                        LoadAllMetadata () const;

public:                        
                                explicit ECDbMap (ECDbR ecdb);
                                ~ECDbMap ();

    ECDbSQLManager const&        GetSQLManager () const { return m_ecdbSqlManager; }
    ECDbSQLManager&              GetSQLManagerR () { return m_ecdbSqlManager; }


    ECN::ECClassCR              GetClassForPrimitiveArrayPersistence (ECN::PrimitiveType primitiveType) const;
    bool                        ContainsMappingsForSchema (ECN::ECSchemaCR ecSchema);
    ECDbR                       GetECDbR () const { return m_ecdb; }
    MapStatus                   MapSchemas (SchemaImportContext const& importSchemaContext, bvector<ECN::ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation);

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
    bool                        IsMappedToExistingTable (ECDbSqlTable& table) const;

    ECDbSqlTable*                  FindOrCreateTable (Utf8CP tableName, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToSecondaryTable, bool mapToExisitingTable, bool allowReplacingEmptyTableWithView) ;
    MappedTableP                GetMappedTable (ClassMapCR classMap, bool createMappedTableEntryIfNotFound = true);
    //! The values returned by GetPrimitiveTypeName are intended only for logging and debugging purposes
    static WCharCP              GetPrimitiveTypeName (ECN::PrimitiveType primitiveType);

    //!Loads the class maps if they were not loaded yet
    void                        GetClassMapsFromRelationshipEnd (bset<IClassMap const*>& endClassMaps, ECN::ECRelationshipConstraintCR relationshipEnd, bool loadIfNotFound) const;
    std::vector<ECN::ECClassCP> GetClassesFromRelationshipEnd (ECN::ECRelationshipConstraintCR relationshipEnd) const;
    size_t                      GetTablesFromRelationshipEnd (bset<ECDbSqlTable*>* tables, ECN::ECRelationshipConstraintCR relationshipEnd) const;
    void                        ClearCache();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
