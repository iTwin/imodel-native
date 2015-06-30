/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <vector>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbMap::ECDbMap (ECDbR ecdb) 
: m_ecdb(ecdb), m_classMapLoadAccessCounter(0), m_ecdbSqlManager(ecdb), m_mapContext(nullptr)
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------+---------------+---------------+---------------+---------------+--------
ECDbMap::MapContext* ECDbMap::GetMapContext() const
    {
    if (AssertIfNotMapping())
        return nullptr;
    
    return m_mapContext.get();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
void ECDbMap::BeginMapping ()
    {
    AssertIfMapping();
    m_mapContext = std::unique_ptr<MapContext>(new MapContext());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
void ECDbMap::EndMapping ()
    {
    AssertIfNotMapping();
    m_mapContext = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::IsMapping () const
    {
    return m_mapContext != nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::AssertIfNotMapping () const
    {
    BeAssert(IsMapping() && "ECDb is in currently is not in mapping mode. Which was not expected");
    return !IsMapping();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::AssertIfMapping () const
    {
    BeAssert (!IsMapping () && "ECDb is in currently in mapping mode. Which was not expected");
    return IsMapping();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::IsExclusivelyStored (ECN::ECClassId ecClassId) const
    {
    return m_exclusivelyStoredClasses.find (ecClassId) != m_exclusivelyStoredClasses.end ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCR ECDbMap::GetClassForPrimitiveArrayPersistence (PrimitiveType primitiveType) const
    {
    ECSchemaCP ecdbSystemSchema = ECDbSystemSchemaHelper::GetSchema (m_ecdb.Schemas());
    EXPECTED_CONDITION (ecdbSystemSchema != nullptr);
 
    //WIP_ECDB: The hard coded names should become constants in ECDbSystemSchemaHelper eventually
    ECClassCP ecMapClass = nullptr;
    switch(primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfBinary");
            break;
        case PRIMITIVETYPE_Boolean:                
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfBoolean");
            break;
        case PRIMITIVETYPE_DateTime:                  
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfDateTime");
            break;
        case PRIMITIVETYPE_Double:                    
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfDouble");
            break;
        case PRIMITIVETYPE_Integer:                   
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfInteger");
            break;
        case PRIMITIVETYPE_Long:                      
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfLong");
            break;
        case PRIMITIVETYPE_Point2D:                   
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfPoint2d");
            break;
        case PRIMITIVETYPE_Point3D:                   
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfPoint3d");
            break;
        case PRIMITIVETYPE_String:                    
            ecMapClass = ecdbSystemSchema->GetClassCP (L"ArrayOfString");
            break;
        case PRIMITIVETYPE_IGeometry:
            ecMapClass = ecdbSystemSchema->GetClassCP(L"ArrayOfGeometry");
            break;
        default:
            BeAssert(0 && "Cannot map primitive type");
        }

    return *ecMapClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus ECDbMap::MapSchemas (SchemaImportContext const& schemaImportContext, bvector<ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation)
    {
    if (AssertIfMapping ())
        return MapStatus::Error;

    BeginMapping ();
    auto stat = DoMapSchemas (schemaImportContext, mapSchemas, forceMapStrategyReevaluation);
    if (stat != MapStatus::Success)
        {
        EndMapping ();
        return stat;
        }

    if (CreateOrUpdateRequiredTables() != SUCCESS)
        {
        schemaImportContext.GetIssueListener ().Report (ECDbSchemaManager::IImportIssueListener::Severity::Error,
            "Failed to import ECSchemas. Data tables could not be created or updated. Please see log for details.");

        ClearCache ();
        EndMapping ();
        return MapStatus::Error;
        }

    if (BE_SQLITE_DONE != Save ())
        {
        ClearCache ();
        schemaImportContext.GetIssueListener ().Report (ECDbSchemaManager::IImportIssueListener::Severity::Error,
            "Failed to import ECSchemas. Mappings of ECSchema to tables could not be saved. Please see log for details.");

        EndMapping ();
        return MapStatus::Error;
        }

    EndMapping ();
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
MapStatus ECDbMap::DoMapSchemas (SchemaImportContext const& schemaImportContext, bvector<ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation)
    {
    StopWatch timer (L"", true);
    if (AssertIfNotMapping ())
        return MapStatus::Error;

    // Identify root classes/relationship-classes
    bvector<ECClassCP> rootClasses;
    bvector<ECRelationshipClassCP> rootRelationships;
    int nClasses = 0;
    int nRelationshipClasses = 0;
    for (ECSchemaCP schema : mapSchemas)
        {
        SupplementalSchemaMetaDataPtr supplementalSchemaMetaData = nullptr;
        if (SupplementalSchemaMetaData::TryGetFromSchema (supplementalSchemaMetaData, *schema) && supplementalSchemaMetaData != nullptr)
            continue; // Don't map any supplemental schemas

        for (ECClassCP ecClass : schema->GetClasses ())
            {
            ECRelationshipClassCP relationshipClass = ecClass->GetRelationshipClassCP ();
            if (ecClass->GetBaseClasses ().size () == 0)
                {
                if (nullptr == relationshipClass)
                    rootClasses.push_back (ecClass);
                else
                    rootRelationships.push_back (relationshipClass);
                }

            if (relationshipClass)
                nRelationshipClasses++;
            else
                nClasses++;
            }
        }

    if (forceMapStrategyReevaluation)
        ClearCache ();

    // Starting with the root, recursively map the entire class hierarchy. 
    MapStatus status = MapStatus::Success;
    for (ECClassCP rootClass : rootClasses)
        {
        status = MapClass (schemaImportContext, *rootClass, forceMapStrategyReevaluation);
        if (status == MapStatus::Error)
            return status;
        }
    
    if (!FinishTableDefinition ())
        return MapStatus::Error;

    BeAssert (status != MapStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationships)
        {
        status = MapClass (schemaImportContext, *rootRelationshipClass, forceMapStrategyReevaluation);
        if (status == MapStatus::Error)
            return status;
        }
    
    BeAssert (status != MapStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (auto& key : m_classMapDictionary)
        {
        key.second->CreateIndices ();
        }

    if (!FinishTableDefinition ())
        return MapStatus::Error;

    timer.Stop ();
    LOG.infov ("Mapped %d ECSchemas containing %d ECClasses and %d ECRelationshipClasses to db in %.4f seconds",
        mapSchemas.size (), nClasses, nRelationshipClasses, timer.GetElapsedSeconds ());

    return MapStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Casey.Mullen        11/2011
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapPtr ECDbMap::LoadAddClassMap (ECClassCR ecClass)
    {
    BeMutexHolder lock (m_criticalSection);
    BeAssert (GetClassMap (ecClass, false) == nullptr);
    MapStatus mapStatus;
    if (!GetSQLManagerR ().IsLoaded ())
        {
        if (GetSQLManagerR ().Load () != BentleyStatus::SUCCESS)
            {
            BeAssert (false && "Failed to map information");
            return nullptr;
            }
        }


    auto classMapPtr = ClassMapFactory::Load (mapStatus, ecClass, *this);
    if (classMapPtr != nullptr)
        {
        if (MapStatus::Error == AddClassMap (classMapPtr))
            {
            LOG.errorv (L"Failed to add map for class %ls", ecClass.GetFullName ());
            return nullptr;
            }

        m_classMapLoadTable.push_back (&ecClass);
        return classMapPtr;
        }

    return nullptr;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                   Ramanujam.Raman                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ECDbMap::MapClass (SchemaImportContext const& schemaImportContext, ECClassCR ecClass, bool forceRevaluationOfMapStrategy)
    {
    if (AssertIfNotMapping ())
        return MapStatus::Error;

    if (!ecClass.HasId())
        {
        if (0 == ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(GetECDbR(), ecClass))
            {
            LOG.errorv(L"ECClass %ls does not exist in ECDb. Import ECSchema containing the class first", ecClass.GetFullName());
            BeAssert(false);
            return MapStatus::Error;
            }
        }

    /*
     * Note: forceRevaluationOfMapStrategy is normally set to TRUE for the case the schemas
     * are getting upgraded. If classes have new properties, loading the previous class map
     * updates the class map with new property maps. If the schema has new classes, the fall
     * through the code below creates the new class maps 
     */
    auto classMap = GetClassMap (ecClass);
    bool revaluateMapStrategy = (classMap != nullptr && forceRevaluationOfMapStrategy);

    MapStatus status = classMap == nullptr ? MapStatus::Success : MapStatus::AlreadyMapped;
    if (status != MapStatus::AlreadyMapped)
        {
        ClassMapPtr newClassMap = ClassMapFactory::Create (status, schemaImportContext, ecClass, *this);

        //error (and no reevaluation)
        if ((status == MapStatus::BaseClassesNotMapped || status == MapStatus::Error) && !revaluateMapStrategy)
            return status;

        if (newClassMap == nullptr)
            {
            BeAssert(false && "ClassMapPtr is not expected to be nullptr at this point.");
            return MapStatus::Error; // this can happen if the ECDbMap custom attributes had invalid values
            }

        status = AddClassMap (newClassMap);
        if (status == MapStatus::Error)
            return status;
        }


    for (ECClassP childClass : ecClass.GetDerivedClasses())
        {
        status = MapClass (schemaImportContext, *childClass, forceRevaluationOfMapStrategy);
        if (status == MapStatus::Error)
            return status;
        } 

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus ECDbMap::AddClassMap (ClassMapPtr& classMap)
    {
    BeMutexHolder lock (m_criticalSection);
    ECClassCR ecClass = classMap->GetClass();
    if (m_classMapDictionary.end() != m_classMapDictionary.find(ecClass.GetId()))
        {
        LOG.errorv (L"Attempted to add a second ClassMap for ECClass %ls", ecClass.GetFullName());
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return MapStatus::Error;
        }
    if (classMap->GetMapStrategy ().IsExclusiveyStoreInThisTable ())
        {
        if (classMap->GetMapStrategy ().IsMapped () && classMap->GetTable ().GetPersistenceType () == PersistenceType::Persisted)
            m_exclusivelyStoredClasses.insert (classMap->GetClass ().GetId ());
        else
            {
            LOG.warningv (L"Exclusively stored class (ECClass %ls) is either not mapped or is mapped to a virtual table", ecClass.GetFullName ());
            BeAssert (false && "Exclusively stored class is either not mapped or is mapped to a virtual table");
            }
        }

    m_classMapDictionary[ecClass.GetId()]= classMap;
    MappedTableP mappedTable = GetMappedTable (*classMap);
    BeAssert (nullptr != mappedTable);
    return mappedTable ? MapStatus::Success : MapStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbMap::RemoveClassMap (IClassMap const& classMap)
    {
    BeMutexHolder lock (m_criticalSection);
    ECClassCR ecClass = classMap.GetClass();
    if (classMap.GetMapStrategy().IsMapped())
        m_clustersByTable.erase (&classMap.GetTable());

    m_classMapDictionary.erase(ecClass.GetId());
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
IClassMap const* ECDbMap::GetClassMap (ECN::ECClassCR ecClass, bool loadIfNotFound) const
    {
    ClassMapPtr classMap = nullptr;
    if (TryGetClassMap (classMap, ecClass, loadIfNotFound))
        {
        BeAssert (classMap != nullptr);
        return classMap.get ();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapCP ECDbMap::GetClassMapCP (ECN::ECClassCR ecClass, bool loadIfNotFound) const
    {
    ClassMapPtr classMap = nullptr;
    if (TryGetClassMap (classMap, ecClass, loadIfNotFound))
        {
        BeAssert (classMap != nullptr);
        return classMap.get ();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
ClassMapP ECDbMap::GetClassMapP (ECN::ECClassCR ecClass, bool loadIfNotFound) const
    {
    ClassMapPtr classMap = nullptr;
    if (TryGetClassMap (classMap, ecClass, loadIfNotFound))
        {
        BeAssert (classMap != nullptr);
        return classMap.get ();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//* @bsimethod                                                    Krischan.Eberle   02/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbMap::TryGetClassMap (ClassMapPtr& classMap, ECN::ECClassCR ecClass, bool loadIfNotFound) const
    {
    BeMutexHolder lock (m_criticalSection);
    if (!ecClass.HasId ())
        {
        ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema (m_ecdb, ecClass);
        BeAssert (ecClass.HasId () && "ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema was not able to retrieve a class id.");
        }

    classMap = DoGetClassMap (ecClass);
    const bool found = classMap != nullptr;
    if (found || !loadIfNotFound)
        return found;

    //lazy loading the class map implemented with const-casting the actual loading so that the 
    //get method itself can remain const (logically const)
    classMap = const_cast<ECDbMap*> (this)->LoadAddClassMap (ecClass);
    return classMap != nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan         08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ClassMapPtr ECDbMap::DoGetClassMap (ECClassCR ecClass) const
    {
    auto it = m_classMapDictionary.find (ecClass.GetId());
    if (m_classMapDictionary.end () == it)
        return nullptr;
    else
        return it->second;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbSqlTable* ECDbMap::FindOrCreateTable (Utf8CP tableName, bool isVirtual, Utf8CP primaryKeyColumnName, bool mapToSecondaryTable, bool mapToExistingTable) 
    {
    if (AssertIfNotMapping ())
        return nullptr;

    BeMutexHolder lock (m_criticalSection);
    auto table = GetSQLManagerR ().GetDbSchemaR ().FindTableP (tableName);
    auto ownerType = mapToExistingTable == false ? OwnerType::ECDb : OwnerType::ExistingTable;
    if (table)
        {

        //if virtuality and empty table handling mismatches, change the table to the stronger
        //option so that both needs are met. (does some logging)
        //existingTable->TryAssign (isVirtual, allowReplacingEmptyTableWithView);        
        if (table->GetOwnerType () != ownerType)
            {
            std::function<Utf8CP (bool)> toStr = [] (bool val) {return val ? "true" : "false"; };
            LOG.warningv ("Multiple classes are mapped to the table %s although the classes require mismatching table metadata: "
                "Metadata IsMappedToExistingTable: Expected=%s - Actual=%s. Actual value is ignored.",
                tableName,
                toStr (mapToExistingTable), toStr (table->GetOwnerType () != OwnerType::ECDb));
            BeAssert (false && "ECDb uses a table for two classes although the classes require mismatching table metadata.");
            }

        return table;
        }

    if (ownerType == OwnerType::ECDb)
        {
        table = GetSQLManagerR ().GetDbSchemaR ().CreateTable (tableName, isVirtual ? PersistenceType::Virtual : PersistenceType::Persisted);
        if (Utf8String::IsNullOrEmpty (primaryKeyColumnName))
            primaryKeyColumnName = ECDB_COL_ECInstanceId;

        auto column = table->CreateColumn (primaryKeyColumnName, ECDbSqlColumn::Type::Long, ECDbSystemColumnECInstanceId, PersistenceType::Persisted);
        if (table->GetPersistenceType () == PersistenceType::Persisted)
            {
            column->GetConstraintR ().SetIsNotNull (true);
            table->GetPrimaryKeyConstraint ()->Add (primaryKeyColumnName);
            }

        if (mapToSecondaryTable)
            {            
            column = table->CreateColumn (ECDB_COL_ParentECInstanceId, ECDbSqlColumn::Type::Long, ECDbSystemColumnParentECInstanceId, PersistenceType::Persisted);
            column = table->CreateColumn (ECDB_COL_ECPropertyPathId, ECDbSqlColumn::Type::Long, ECDbSystemColumnECPropertyPathId, PersistenceType::Persisted);
            column = table->CreateColumn (ECDB_COL_ECArrayIndex, ECDbSqlColumn::Type::Long, ECDbSystemColumnECArraryIndex, PersistenceType::Persisted);
            if (table->GetPersistenceType () == PersistenceType::Persisted)
                {
                auto index = table->CreateIndex ((table->GetName() + "_StructArrayIndex").c_str());
                index->SetIsUnique (true);
                index->Add (ECDB_COL_ParentECInstanceId);
                index->Add (ECDB_COL_ECPropertyPathId);
                index->Add (ECDB_COL_ECArrayIndex);
                index->Add (primaryKeyColumnName);
                }
            }
        }
    else
        {
        BeAssert (mapToSecondaryTable == false);
        if (mapToSecondaryTable)
            return nullptr;

        table = GetSQLManagerR ().GetDbSchemaR ().CreateTableUsingExistingTableDefinition (GetECDbR (), tableName);
        if (!Utf8String::IsNullOrEmpty (primaryKeyColumnName))
            {
            auto editMode = table->GetEditHandle ().CanEdit ();
            if (!editMode)
                table->GetEditHandleR ().BeginEdit ();

            auto systemColumn = table->FindColumnP (primaryKeyColumnName);
            if (systemColumn == nullptr)
                {
                BeAssert (false && "Failed to find user provided primary key column");
                return nullptr;
                }

            systemColumn->SetUserFlags (ECDbSystemColumnECInstanceId);
            if (!editMode)
                table->GetEditHandleR ().EndEdit ();
            }
        }

    return table;   
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTableP ECDbMap::GetMappedTable (ClassMapCR classMap, bool createMappedTableEntryIfNotFound)//ECDB_TODO this funtion name should be GetOrAddMappedTable()
    {
    BeMutexHolder lock (m_criticalSection);

    ClustersByTable::const_iterator it = m_clustersByTable.find (&classMap.GetTable());
    if (m_clustersByTable.end() != it)
        {
        MappedTableP mappedTable = it->second.get();
        if (createMappedTableEntryIfNotFound)
            mappedTable->AddClassMap (classMap);
        return mappedTable;
        }
    if (createMappedTableEntryIfNotFound)
        {
        MappedTablePtr mappedTable = MappedTable::Create (*this, classMap);
        m_clustersByTable[&classMap.GetTable()] = mappedTable;
        return mappedTable.get();
        }
    return nullptr;
    }

#if defined (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable:4063)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static
WCharCP ECDbMap::GetPrimitiveTypeName (ECN::PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        // the values are intended only for logging and debugging purposes
        case PRIMITIVETYPE_String  : return L"String";
        case PRIMITIVETYPE_Integer : return L"Integer(32)";
        case PRIMITIVETYPE_Long    : return L"Long(64)";
        case PRIMITIVETYPE_Double  : return L"Double";
        case PRIMITIVETYPE_DateTime: return L"DateTime";
        case PRIMITIVETYPE_Binary  : return L"Binary";
        case PRIMITIVETYPE_Boolean : return L"Boolean";
        case PRIMITIVETYPE_Point2D : return L"Point2D";
        case PRIMITIVETYPE_Point3D : return L"Point3D";
        default:                     return L"<unknown>";
        }
    }

#if defined (_MSC_VER)
    #pragma warning (pop)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::CreateOrUpdateRequiredTables ()
    {
    if (AssertIfNotMapping ())
        return ERROR;

    BeMutexHolder lock (m_criticalSection);
    m_ecdb.GetStatementCache ().Empty ();
    StopWatch timer(L"", true);
    
    int nCreated = 0;
    int nUpdated = 0;
    int nSkipped = 0;
    ClustersByTable::iterator it = m_clustersByTable.begin();    
    for (; it != m_clustersByTable.end(); ++it)
        {
        MappedTablePtr & mappedTable = it->second;
        ECDbSqlTable* table = it->first;
                
        if (!mappedTable->IsFinished())
            {
            if (mappedTable->FinishTableDefinition() != SUCCESS)
                return ERROR;
            }

        if (table->GetPersistenceType () == PersistenceType::Virtual || table->GetOwnerType () == OwnerType::ExistingTable)           
            continue; 
        
        if (GetECDbR().TableExists(table->GetName().c_str()))
            {
            if (GetSQLManager ().IsTableChanged (*table))
                {
                if (table->GetPersistenceManager ().CreateOrUpdate (GetECDbR ()) != BentleyStatus::SUCCESS)
                    return ERROR;
                nUpdated++;
                }
            else
                nSkipped++;
            }
        else
            {
            if (table->GetPersistenceManager ().Create (GetECDbR ()) != BentleyStatus::SUCCESS)
                return ERROR;

            nCreated++;
            }
        }

    timer.Stop();
    LOG.infov("Created %d tables, Skipped %d and updated %d table/view(s) in %.4f seconds", nCreated, nSkipped, nUpdated, timer.GetElapsedSeconds());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbMap::FinishTableDefinition () const
    {
    if (AssertIfNotMapping ())
        return false;

    BeMutexHolder aGurad (m_criticalSection);
    auto it = m_clustersByTable.begin();
    for (; it != m_clustersByTable.end(); ++it)
        {
        MappedTablePtr const& mappedTable = it->second;
        if (!mappedTable->IsFinished() && mappedTable->FinishTableDefinition() != SUCCESS)
            return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
std::vector<ECClassCP> ECDbMap::GetClassesFromRelationshipEnd (ECRelationshipConstraintCR relationshipEnd) const
    {
    //for recursive lambdas, iOS requires us to define the lambda variable before assigning the actual function to it.
    std::function<void (std::vector<ECClassCP>&, ECClassP, bool)> gatherClassesDelegate;
    gatherClassesDelegate =
        [this, &gatherClassesDelegate] (std::vector<ECClassCP>& classes, ECClassP ecClass, bool includeSubclasses)
        {
        classes.push_back (ecClass);
        if (includeSubclasses)
            {
            for (auto childClass : ecClass->GetDerivedClasses ())
                { 
                gatherClassesDelegate (classes, childClass, includeSubclasses);
                }
            }
        };

    bool isPolymorphic = relationshipEnd.GetIsPolymorphic();
    std::vector<ECClassCP> classes;
    for (ECClassCP ecClass : relationshipEnd.GetClasses ())
        {
        ECClassP ecClassP = const_cast<ECClassP> (ecClass);
        gatherClassesDelegate (classes, ecClassP, isPolymorphic);
        }
    
    return std::move (classes);
    }
    
/*---------------------------------------------------------------------------------**//**
* Gets the (count of) tables at the specified end of a relationship class. 
* @param  tables [out] The tables that are at the specified end of the relationship. Can be nullptr
* @param  relationshpEnd [in] Constraint at the end of the relationship
* @return Number of tables at the specified end of the relationship. Returns 
*         UINT_MAX if the end is AnyClass. 
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbMap::GetTablesFromRelationshipEnd (bset<ECDbSqlTable*>* tables, ECRelationshipConstraintCR relationshipEnd) const
    {
    bset<ECDbSqlTable*> tmpTables;
    bset<ECDbSqlTable*>* outTables = (tables == nullptr) ? &tmpTables : tables;
    
    auto classes = GetClassesFromRelationshipEnd (relationshipEnd);
    for (ECClassCP ecClass : classes)
        {
        if (ClassMap::IsAnyClass (*ecClass))
            {
            outTables->clear();
            return SIZE_MAX;
            }

        auto classMap = GetClassMap (*ecClass, false);
        if (classMap->GetMapStrategy ().IsDoNotMap() || classMap->GetMapStrategy ().IsDoNotMapHierarchy())
            continue;

        BeAssert (classMap->GetMapStrategy().IsMapped());
        ECDbSqlTable&  table = classMap->GetTable();
        outTables->insert(&table);
        }
    return outTables->size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                      06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMap::GetClassMapsFromRelationshipEnd (bset<IClassMap const*>& endClassMaps, ECRelationshipConstraintCR relationshipEnd, bool loadIfNotFound) const
    {
    auto endClasses = GetClassesFromRelationshipEnd (relationshipEnd);
    for (auto endClass : endClasses)
        {
        if (ClassMap::IsAnyClass (*endClass))
            {
            return;
            }

        auto endClassMap = GetClassMap (*endClass, loadIfNotFound);
        if (endClassMap->GetMapStrategy().IsUnmapped())
            continue;

        endClassMaps.insert (endClassMap);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbMap::ClearCache ()
    {
    BeMutexHolder aGurad (m_criticalSection);
    m_classMapDictionary.clear();
    m_clustersByTable.clear();
    m_exclusivelyStoredClasses.clear ();
    GetSQLManagerR ().Reset ();
    }

/*---------------------------------------------------------------------------------**//**
* Save map
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbMap::Save()
    {
    BeMutexHolder aGurad (m_criticalSection);
    StopWatch stopWatch("", true);
    int i = 0;
    std::set<ClassMap const*> doneList;
    for (auto it =  m_classMapDictionary.begin(); it != m_classMapDictionary.end(); it++)
        {
        ClassMapPtr const& classMap = it->second;
        ECClassCR ecClass = classMap->GetClass();
        if (classMap->IsDirty())
            {
            i++;
            auto r = classMap->Save (doneList);
            if (r != BentleyStatus::SUCCESS)
                {
                LOG.errorv ("Failed to save ECDbMap for ECClass %s. db error: %s", Utf8String (ecClass.GetFullName ()).c_str (), GetECDbR ().GetLastError ());
                return BE_SQLITE_ERROR;
                }
            }
        }

    stopWatch.Stop();
    GetSQLManagerR ().Save ();
    LOG.infov (L"Saving EC to db mappings took %.4lf seconds to save %d classes", stopWatch.GetElapsedSeconds (), i);

    return BE_SQLITE_DONE;
    }

//************************************************************************************
// ECDbMap::MapContext
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------------------------------------------------------------------------------
void ECDbMap::MapContext::AddClassIdFilteredIndex(ECDbSqlIndex const& index, ECClassId classId)
    {
    BeAssert(m_classIdFilteredIndices.find(&index) == m_classIdFilteredIndices.end());
    m_classIdFilteredIndices[&index] = classId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------------------------------------------------------------------------------
bool ECDbMap::MapContext::TryGetClassIdToIndex (ECClassId& classId, ECDbSqlIndex const& index) const
    {
    auto it = m_classIdFilteredIndices.find(&index);
    if (it == m_classIdFilteredIndices.end())
        return false;

    classId = it->second;
    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

