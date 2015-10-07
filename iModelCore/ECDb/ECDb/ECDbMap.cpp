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
: m_ecdb (ecdb), m_classMapLoadAccessCounter (0), m_ecdbSqlManager (ecdb), m_schemaImportContext (nullptr), m_lightweightCache (*this)
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   05/2015
//---------------+---------------+---------------+---------------+---------------+--------
SchemaImportContext* ECDbMap::GetSchemaImportContext() const
    {
    if (AssertIfIsNotImportingSchema())
        return nullptr;
    
    return m_schemaImportContext;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::IsImportingSchema () const
    {
    return m_schemaImportContext != nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      02/2015
//---------------+---------------+---------------+---------------+---------------+--------
bool ECDbMap::AssertIfIsNotImportingSchema() const
    {
    BeAssert(IsImportingSchema() && "ECDb is in currently in schema import mode. Which was not expected");
    return !IsImportingSchema();
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
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfBinary");
            break;
        case PRIMITIVETYPE_Boolean:                
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfBoolean");
            break;
        case PRIMITIVETYPE_DateTime:                  
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfDateTime");
            break;
        case PRIMITIVETYPE_Double:                    
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfDouble");
            break;
        case PRIMITIVETYPE_Integer:                   
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfInteger");
            break;
        case PRIMITIVETYPE_Long:                      
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfLong");
            break;
        case PRIMITIVETYPE_Point2D:                   
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfPoint2d");
            break;
        case PRIMITIVETYPE_Point3D:                   
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfPoint3d");
            break;
        case PRIMITIVETYPE_String:                    
            ecMapClass = ecdbSystemSchema->GetClassCP ("ArrayOfString");
            break;
        case PRIMITIVETYPE_IGeometry:
            ecMapClass = ecdbSystemSchema->GetClassCP("ArrayOfGeometry");
            break;
        default:
            BeAssert(0 && "Cannot map primitive type");
        }

    return *ecMapClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    04/2014
//+---------------+---------------+---------------+---------------+---------------+------
MapStatus ECDbMap::MapSchemas (SchemaImportContext& schemaImportContext, bvector<ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation)
    {
    if (m_schemaImportContext != nullptr)
        {
        BeAssert(false && "MapSchemas is expected to be called if no other schema import is running.");
        return MapStatus::Error;
        }

    m_schemaImportContext = &schemaImportContext;

    auto stat = DoMapSchemas (mapSchemas, forceMapStrategyReevaluation);
    if (MapStatus::Success != stat)
        {
        m_schemaImportContext = nullptr;
        return stat;
        }

    if (SUCCESS != CreateOrUpdateRequiredTables())
        {
        ClearCache ();
        m_schemaImportContext = nullptr;
        return MapStatus::Error;
        }

    if (SUCCESS != Save())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return MapStatus::Error;
        }

    m_lightweightCache.Reset();

    if (SUCCESS != m_ecdbSqlManager.GetDbSchema().CreateOrUpdateIndices())
        {
        ClearCache();
        m_schemaImportContext = nullptr;
        return MapStatus::Error;
        }

    std::set<ClassMap const*> classMaps;
    for (auto& key : m_classMapDictionary)
        {
        if (!key.second->GetMapStrategy ().IsNotMapped ())
            classMaps.insert (key.second.get ());
        }
    ECDbMapAnalyser mapAnalyser (*this);
    if (mapAnalyser.Analyser (true /*apply changes*/) != BentleyStatus::SUCCESS)
        return MapStatus::Error;

    m_schemaImportContext = nullptr;
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         06/2015
//---------------------------------------------------------------------------------------
ClassMapCP  ECDbMap::GetClassMapCP (ECN::ECClassId classId) const
    {

    auto ecClass = GetECDbR ().Schemas ().GetECClass (classId);
    if (ecClass == nullptr)
        {
        BeDataAssert (false && "Failed to find classmap with given ecclassid");
        return nullptr;
        }

    return static_cast<RelationshipClassMapCP>(GetClassMap (*ecClass));;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         06/2015
//---------------------------------------------------------------------------------------
RelationshipClassMapCP ECDbMap::GetRelationshipClassMap (ECN::ECClassId ecRelationshipClassId) const
    {
    auto ecClass = GetECDbR ().Schemas ().GetECClass (ecRelationshipClassId);
    if (ecClass == nullptr)
        {
        BeDataAssert (false && "Failed to find classmap with given ecclassid");
        return nullptr;
        }

    if (ecClass->GetRelationshipClassCP() == nullptr)
        {
        BeDataAssert (false && "Failed to find relationship classmap with given ecclassid");
        return nullptr;
        }

    return static_cast<RelationshipClassMapCP>(GetClassMap (*ecClass));;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan         09/2012
//---------------------------------------------------------------------------------------
MapStatus ECDbMap::DoMapSchemas (bvector<ECSchemaCP>& mapSchemas, bool forceMapStrategyReevaluation)
    {
    if (AssertIfIsNotImportingSchema ())
        return MapStatus::Error;

    StopWatch timer (true);


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
        status = MapClass (*rootClass, forceMapStrategyReevaluation);
        if (status == MapStatus::Error)
            return status;
        }

    if (!FinishTableDefinition ())
        return MapStatus::Error;

    BeAssert (status != MapStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (ECRelationshipClassCP rootRelationshipClass : rootRelationships)
        {
        status = MapClass (*rootRelationshipClass, forceMapStrategyReevaluation);
        if (status == MapStatus::Error)
            return status;
        }

    BeAssert (status != MapStatus::BaseClassesNotMapped && "Expected to resolve all class maps by now.");
    for (std::pair<ClassMap const*, std::unique_ptr<ClassMapInfo>> const& kvpair : GetSchemaImportContext()->GetClassMapInfoCache())
        {
        if (SUCCESS != kvpair.first->CreateUserProvidedIndices(*kvpair.second))
            return MapStatus::Error;
        }

    if (!FinishTableDefinition ())
        return MapStatus::Error;
     
    timer.Stop ();
    if (LOG.isSeverityEnabled (NativeLogging::LOG_DEBUG))

        LOG.debugv ("Mapped %d ECSchemas containing %d ECClasses and %d ECRelationshipClasses to the database in %.4f seconds",
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
            LOG.errorv ("Failed to add map for class %s", ecClass.GetFullName ());
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
MapStatus ECDbMap::MapClass (ECClassCR ecClass, bool forceRevaluationOfMapStrategy)
    {
    if (AssertIfIsNotImportingSchema ())
        return MapStatus::Error;

    if (!ecClass.HasId())
        {
        if (0 == ECDbSchemaManager::GetClassIdForECClassFromDuplicateECSchema(GetECDbR(), ecClass))
            {
            LOG.errorv("ECClass %s does not exist in ECDb. Import ECSchema containing the class first", ecClass.GetFullName());
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
        ClassMapPtr newClassMap = ClassMapFactory::Create (status, *GetSchemaImportContext(), ecClass, *this);

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
        status = MapClass (*childClass, forceRevaluationOfMapStrategy);
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
        LOG.errorv ("Attempted to add a second ClassMap for ECClass %s", ecClass.GetFullName());
        BeAssert(false && "Attempted to add a second ClassMap for the same ECClass");
        return MapStatus::Error;
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
    if (!classMap.GetMapStrategy().IsNotMapped())
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
    if (AssertIfIsNotImportingSchema ())
        return nullptr;

    BeMutexHolder lock (m_criticalSection);
    ECDbSqlTable* table = GetSQLManagerR ().GetDbSchemaR ().FindTableP (tableName);
    OwnerType ownerType = mapToExistingTable == false ? OwnerType::ECDb : OwnerType::ExistingTable;
    if (table != nullptr)
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

        auto column = table->CreateColumn (primaryKeyColumnName, ECDbSqlColumn::Type::Long, ECDbKnownColumns::ECInstanceId, PersistenceType::Persisted);
        if (table->GetPersistenceType () == PersistenceType::Persisted)
            {
            column->GetConstraintR ().SetIsNotNull (true);
            table->GetPrimaryKeyConstraint ()->Add (primaryKeyColumnName);
            }

        if (mapToSecondaryTable)
            {            
            column = table->CreateColumn (ECDB_COL_ParentECInstanceId, ECDbSqlColumn::Type::Long, ECDbKnownColumns::ParentECInstanceId, PersistenceType::Persisted);
            column = table->CreateColumn (ECDB_COL_ECPropertyPathId, ECDbSqlColumn::Type::Long, ECDbKnownColumns::ECPropertyPathId, PersistenceType::Persisted);
            column = table->CreateColumn (ECDB_COL_ECArrayIndex, ECDbSqlColumn::Type::Long, ECDbKnownColumns::ECArrayIndex, PersistenceType::Persisted);
            if (table->GetPersistenceType () == PersistenceType::Persisted)
                {
                //struct array indices don't get a class id
                Utf8String indexName("uix_");
                indexName.append(table->GetName()).append("_structarraykey");
                ECDbSqlIndex* index = table->CreateIndex(indexName.c_str(), true, ECClass::UNSET_ECCLASSID);
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

        table = GetSQLManagerR ().GetDbSchemaR ().CreateTableForExistingTableMapStrategy (GetECDbR (), tableName);
        if (table == nullptr)
            return nullptr;

        if (!Utf8String::IsNullOrEmpty (primaryKeyColumnName))
            {
            auto editMode = table->GetEditHandle ().CanEdit ();
            if (!editMode)
                table->GetEditHandleR ().BeginEdit ();

            auto systemColumn = table->FindColumnP (primaryKeyColumnName);
            if (systemColumn == nullptr)
                {
                LOG.errorv("Primary key column '%s' does not exist in table '%s' which was specified in ClassMap custom attribute together with ExistingTable MapStrategy. Specify the column name in the ECInstanceIdColumn property in the ClassMap custom attribute, if it is not ECDb's default primary key column name.", primaryKeyColumnName, tableName);
                return nullptr;
                }

            systemColumn->SetKnownColumnId (ECDbKnownColumns::ECInstanceId);
            if (!editMode)
                table->GetEditHandleR ().EndEdit ();
            }
        }

    return table;   
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTableP ECDbMap::GetMappedTable (ClassMapCR classMap, bool createMappedTableEntryIfNotFound)//ECDB_TODO this function name should be GetOrAddMappedTable()
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


#if defined (_MSC_VER)
    #pragma warning (pop)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::CreateOrUpdateRequiredTables ()
    {
    if (AssertIfIsNotImportingSchema())
        return ERROR;

    BeMutexHolder lock (m_criticalSection);
    m_ecdb.GetStatementCache ().Empty ();
    StopWatch timer(true);
    
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
            if (table->GetPersistenceManager().Create(GetECDbR()) != BentleyStatus::SUCCESS)
                return ERROR;

            nCreated++;
            }
        }

    timer.Stop();
    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        LOG.debugv("Created %d tables, skipped %d tables and updated %d table/view(s) in %.4f seconds", nCreated, nSkipped, nUpdated, timer.GetElapsedSeconds());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbMap::FinishTableDefinition () const
    {
    if (AssertIfIsNotImportingSchema ())
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
* Gets the count of tables at the specified end of a relationship class. 
* @param  relationshpEnd [in] Constraint at the end of the relationship
* @return Number of tables at the specified end of the relationship. Returns 
*         UINT_MAX if the end is AnyClass. 
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDbMap::GetTableCountOnRelationshipEnd(ECRelationshipConstraintCR relationshipEnd) const
    {
    bset<ECDbSqlTable const*> tables;
    std::vector<ECClassCP> classes = GetClassesFromRelationshipEnd(relationshipEnd);
    for (ECClassCP ecClass : classes)
        {
        if (ClassMap::IsAnyClass (*ecClass))
            return SIZE_MAX;

        IClassMap const* classMap = GetClassMap (*ecClass, false);
        if (classMap->GetMapStrategy ().IsNotMapped ())
            continue;

        tables.insert(&classMap->GetTable());
        }

    return tables.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Krischan.Eberle                      06/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbMap::GetClassMapsFromRelationshipEnd(bset<IClassMap const*>& endClassMaps, ECRelationshipConstraintCR relationshipEnd, bool loadIfNotFound) const
    {
    std::vector<ECClassCP> endClasses = GetClassesFromRelationshipEnd(relationshipEnd);
    for (auto endClass : endClasses)
        {
        if (ClassMap::IsAnyClass(*endClass))
            return;

        auto endClassMap = GetClassMap(*endClass, loadIfNotFound);
        if (endClassMap->GetMapStrategy().IsNotMapped())
            continue;

        endClassMaps.insert(endClassMap);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbMap::ClearCache()
    {
    BeMutexHolder lock(m_criticalSection);
    m_classMapDictionary.clear();
    m_clustersByTable.clear();
    GetSQLManagerR().Reset();
    m_lightweightCache.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* Save map
* @bsimethod                                 Affan Khan                          08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECDbMap::Save()
    {
    BeMutexHolder lock(m_criticalSection);
    StopWatch stopWatch(true);
    int i = 0;
    std::set<ClassMap const*> doneList;
    for (auto it =  m_classMapDictionary.begin(); it != m_classMapDictionary.end(); it++)
        {
        ClassMapPtr const& classMap = it->second;
        ECClassCR ecClass = classMap->GetClass();
        if (classMap->IsDirty())
            {
            i++;
            if (SUCCESS != classMap->Save (doneList))
                {
                m_ecdb.GetECDbImplR().GetIssueReporter().Report (ECDbIssueSeverity::Error, "Failed to save ECDbMap for ECClass %s: %s", ecClass.GetFullName(), m_ecdb.GetLastError());
                return ERROR;
                }
            }
        }

    stopWatch.Stop();
    if (SUCCESS != GetSQLManagerR().Save())
        return ERROR;

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        LOG.debugv ("Saving ECDbMap for %d ECClasses took %.4lf msecs.", i, stopWatch.GetElapsedSeconds () * 1000.0);

    return SUCCESS;
    }



//************************************************************************************
// LightweightCache
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadClassIdsPerTable () const
    {
    if (m_loadedFlags.m_classIdsPerTableIsLoaded)
        return;

    Utf8CP sql0 =
        "SELECT ec_Table.Id, ec_Class.Id ClassId, ec_Table.Name TableName "
        "     FROM ec_PropertyMap  "
        "         JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId AND ec_Column.KnownColumn != 2 "
        "         JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "         JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "         JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId  "
        "         JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "     WHERE ec_ClassMap.MapStrategy NOT IN (100, 101) "
        "    GROUP BY  ec_Table.Id, ec_Class.Id ";

    auto stmt = m_map.GetECDbR ().GetCachedStatement (sql0);
    ECDbTableId currentTableId = -1;
    ECDbSqlTable const* currentTable;
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        auto tableId = stmt->GetValueInt64 (0);
        ECClassId id = stmt->GetValueInt64 (1);
        if (currentTableId != tableId)
            {
            Utf8CP tableName = stmt->GetValueText (2);
            currentTable = m_map.GetSQLManager ().GetDbSchema ().FindTable (tableName);
            currentTableId = tableId;
            BeAssert (currentTable != nullptr);
            }

        m_classIdsPerTable[currentTable].push_back (id);
        }

    m_loadedFlags.m_classIdsPerTableIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadAnyClassRelationships () const
    {
    if (m_loadedFlags.m_anyClassRelationshipsIsLoaded)
        return;

    Utf8CP sql1 =
        " SELECT"
        "       RCC.RelationshipClassId,"
        "       RCC.RelationshipEnd"
        " FROM ec_RelationshipConstraintClass RCC"
        " WHERE RCC.ClassId IN (SELECT Id FROM ec_Class WHERE Name = 'AnyClass')";

    auto stmt1 = m_map.GetECDbR ().GetCachedStatement (sql1);
    while (stmt1->Step () == BE_SQLITE_ROW)
        {
        ECClassId id = stmt1->GetValueInt64 (0);
        RelationshipEnd filter = stmt1->GetValueInt (1) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;

        auto itor = m_anyClassRelationships.find (id);
        if (itor == m_anyClassRelationships.end ())
            m_anyClassRelationships.insert (make_bpair (id, filter));
        else
            {
            m_anyClassRelationships[id] = static_cast<RelationshipEnd>((int)(itor->second) & (int)(filter));
            }
        }

    m_loadedFlags.m_anyClassRelationshipsIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadAnyClassReplacements () const
    {
    if (m_loadedFlags.m_anyClassReplacementsLoaded)
        return;

    Utf8CP sql1 =
        "SELECT ec_Class.Id "
        "  FROM ec_PropertyMap "
        "       JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId "
        "       JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "       JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "       JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId  "
        "       JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
		"WHERE ec_ClassMap.MapStrategy <> 0 AND ec_Class.IsRelationship = 0 AND ec_Table.IsVirtual = 0 "
        "GROUP BY ec_Class.Id ";


    auto stmt1 = m_map.GetECDbR ().GetCachedStatement (sql1);
    while (stmt1->Step () == BE_SQLITE_ROW)
        {
        ECClassId id = stmt1->GetValueInt64 (0);
        m_anyClassReplacements.push_back (id);
        }

    m_loadedFlags.m_anyClassReplacementsLoaded = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadRelationshipCache () const
    {
    if (m_loadedFlags.m_relationshipCacheIsLoaded)
        return;

    Utf8CP sql0 =
        "WITH RECURSIVE "
        "  DerivedClassList(RelationshipClassId, RelationshipEnd, IsPolymorphic, CurrentClassId, DerivedClassId) "
        "  AS ( "
        "      SELECT "
        "            RCC.RelationshipClassId, "
        "            RCC.RelationshipEnd, "
        "            RC.IsPolymorphic, "
        "            RCC.ClassId, "
        "            RCC.ClassId "
        "      FROM ec_RelationshipConstraintClass RCC "
        "      INNER JOIN ec_RelationshipConstraint RC "
        "            ON RC.RelationshipClassId = RCC.RelationshipClassId AND RC.RelationshipEnd = RCC.[RelationshipEnd] "
        "    UNION "
        "        SELECT DCL.RelationshipClassId, DCL.RelationshipEnd, DCL.IsPolymorphic, BC.BaseClassId, BC.ClassId "
        "            FROM DerivedClassList DCL "
        "        INNER JOIN ec_BaseClass BC ON BC.BaseClassId = DCL.DerivedClassId "
        "        WHERE IsPolymorphic = 1 "
        "  ) "
        "  SELECT DerivedClassId, "
        "         RelationshipClassId, "
        "         RelationshipEnd "
        "  FROM DerivedClassList ";


    auto stmt0 = m_map.GetECDbR ().GetCachedStatement (sql0);
    while (stmt0->Step () == BE_SQLITE_ROW)
        {
        ECClassId constraintClassId = stmt0->GetValueInt64 (0);
        ECClassId relationshipId = stmt0->GetValueInt64 (1);
        BeAssert (!stmt0->IsColumnNull (2));
        RelationshipEnd end = stmt0->GetValueInt (2) == 0 ? RelationshipEnd::Source : RelationshipEnd::Target;

        RelationshipClassIds& relClassIds = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        auto relIt = relClassIds.find (relationshipId);
        if (relIt == relClassIds.end ())
            relClassIds[relationshipId] = end;
        else
            relClassIds[relationshipId] = static_cast<RelationshipEnd>((int)(relIt->second) | (int)(end));


        ConstraintClassIds& constraintClassIds = m_nonAnyClassConstraintClassIdsPerRelClassIds[relationshipId];
        auto constraintIt = constraintClassIds.find (constraintClassId);
        if (constraintIt == constraintClassIds.end ())
            constraintClassIds[constraintClassId] = end;
        else
            constraintClassIds[constraintClassId] = static_cast<RelationshipEnd>((int)(constraintIt->second) | (int)(end));
        }

    m_relationshipEndsByClassIdRev = m_nonAnyClassConstraintClassIdsPerRelClassIds;
    LoadAnyClassRelationships ();
    LoadAnyClassReplacements ();
    for (ECClassId constraintClassId : m_anyClassReplacements)
        {
        RelationshipClassIds& rels = m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
        for (bpair<ECClassId, RelationshipEnd> const& pair : m_anyClassRelationships)
            {
            ECClassId relId = pair.first;
            RelationshipEnd end = pair.second;
            auto relsIt = rels.find (relId);
            if (relsIt == rels.end ())
                rels[relId] = end;
            else
                rels[relId] = static_cast<RelationshipEnd>((int)(relsIt->second) | (int)(end));
            }
        }
    for (auto& pair1 : m_anyClassRelationships)
        {
        auto& classes = m_relationshipEndsByClassIdRev[pair1.first];
        for (auto id : m_anyClassReplacements)
            {
            //ECClassId id = pair1.first;
            auto itor1 = classes.find (id);
            if (itor1 == classes.end ())
                {
                classes[id] = pair1.second;
                }
            else
                {
                classes[id] = static_cast<RelationshipEnd>((int)(itor1->second) | (int)(pair1.second));
                }
            }
        }

    m_loadedFlags.m_relationshipCacheIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadVerticalPartitions()  const
    {
    if (m_loadedFlags.m_verticalPartitionsIsLoaded)
        return;

    Utf8CP sql0 =
        "SELECT  ec_Class.Id ClassId, ec_Table.Name "
        "    FROM ec_PropertyMap "
        "       JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId AND ec_Column.KnownColumn != 2 "
        "       JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "       JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "       JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId "
        "       JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "    WHERE ec_ClassMap.MapStrategy NOT IN (100, 101) "
        "    GROUP BY ec_Class.Id, ec_Table.Name ";

    CachedStatementPtr stmt = m_map.GetECDbR().GetCachedStatement(sql0);
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        auto classId = stmt->GetValueInt64(0);
        Utf8CP tableName = stmt->GetValueText(1);
        auto table = m_map.GetSQLManager().GetDbSchema().FindTable(tableName);
        BeAssert(table != nullptr);
        m_verticalPartitions[classId].insert(table);
        }

    m_loadedFlags.m_verticalPartitionsIsLoaded = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadHorizontalPartitions ()  const
    {
    if (m_loadedFlags.m_horizontalPartitionsIsLoaded)
        return;

    auto anyClassId = GetAnyClassId ();
    Utf8CP sql0 =
        "WITH RECURSIVE  "
        "   DerivedClassList(RootClassId, CurrentClassId, DerivedClassId) "
        "   AS ( "
        "       SELECT Id, Id, Id FROM ec_Class "
        "   UNION  "
        "       SELECT RootClassId,  BC.BaseClassId, BC.ClassId "
        "           FROM DerivedClassList DCL  "
        "       INNER JOIN ec_BaseClass BC ON BC.BaseClassId = DCL.DerivedClassId "
        "   ), "
        "   TableMapInfo "
        "   AS ( "
        "   SELECT  ec_Class.Id ClassId, ec_Table.Name TableName "
        "   FROM ec_PropertyMap  "
        "       JOIN ec_Column ON ec_Column.Id = ec_PropertyMap.ColumnId AND ec_Column.KnownColumn != 2 "
        "       JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId "
        "       JOIN ec_ClassMap ON ec_ClassMap.Id = ec_PropertyMap.ClassMapId "
        "       JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId  "
        "       JOIN ec_Table ON ec_Table.Id = ec_Column.TableId "
        "   WHERE ec_ClassMap.MapStrategy NOT IN (100, 101)  AND ec_Table.Name NOT LIKE '%_joinedTable'"
        "  GROUP BY  ec_Class.Id , ec_Table.Name "
        "   )  "
        "SELECT  DCL.RootClassId, DCL.DerivedClassId, TMI.TableName FROM DerivedClassList DCL  "
        "   INNER JOIN TableMapInfo TMI ON TMI.ClassId = DCL.DerivedClassId ORDER BY DCL.RootClassId, TMI.TableName,DCL.DerivedClassId";

    CachedStatementPtr stmt = m_map.GetECDbR ().GetCachedStatement (sql0);
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        auto rootClassId = stmt->GetValueInt64 (0);
        auto derivedClassId = stmt->GetValueInt64 (1);
        if (anyClassId == rootClassId)
            continue;

        Utf8CP tableName = stmt->GetValueText (2);
        auto table = m_map.GetSQLManager ().GetDbSchema ().FindTable (tableName);
        BeAssert (table != nullptr);
        auto& ids = m_horizontalPartitions[rootClassId][table];
        if (derivedClassId == rootClassId)
            {
            ids.insert (ids.begin (), derivedClassId);
            }
        else
            ids.push_back (derivedClassId);
        }

    m_loadedFlags.m_horizontalPartitionsIsLoaded = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::LoadRelationshipByTable ()  const
    {
    if (m_loadedFlags.m_relationshipPerTableLoaded)
        return;

    Utf8CP sql0 =
        " SELECT DISTINCT ec_Class.Id, ec_Table.Name, ec_ClassMap.MapStrategy"
        "    FROM ec_Column"
        "        INNER JOIN ec_Table ON ec_Table.Id = ec_Column.TableId"
        "        INNER JOIN ec_PropertyMap ON  ec_PropertyMap.ColumnId = ec_Column.Id"
        "        INNER JOIN ec_PropertyPath ON ec_PropertyPath.Id = ec_PropertyMap.PropertyPathId"
        "        INNER JOIN ec_Property ON ec_PropertyPath.RootPropertyId = ec_Property.Id"
        "        INNER JOIN ec_ClassMap ON ec_PropertyMap.ClassMapId = ec_ClassMap.Id"
        "        INNER JOIN ec_Class ON ec_Class.Id = ec_ClassMap.ClassId"
        "    WHERE ec_ClassMap.MapStrategy  <> 0 AND ec_Column.KnownColumn NOT IN (1, 2) AND ec_Class.IsRelationship = 1 AND ec_Table.IsVirtual = 0";
    // AND ec_Table.IsVirtual = 0

    auto stmt = m_map.GetECDbR ().GetCachedStatement (sql0);
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        auto relationshipClassId = stmt->GetValueInt64 (0);
        Utf8CP tableName = stmt->GetValueText (1);
        RelationshipType type = RelationshipType::Link;
        if (stmt->GetValueInt (2) == Enum::ToInt (RelationshipType::Source))
            type = RelationshipType::Source;
        else if (stmt->GetValueInt (2) == Enum::ToInt (RelationshipType::Target))
            type = RelationshipType::Target;

        auto table = m_map.GetSQLManager ().GetDbSchema ().FindTable (tableName);
        BeAssert (table != nullptr);
        m_relationshipPerTable[table][relationshipClassId] = type;       
        }

    m_loadedFlags.m_relationshipPerTableLoaded = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//--------------------------------------------------------------------------------------
ECN::ECClassId ECDbMap::LightweightCache::GetAnyClassId () const
    {
    if (m_anyClassId == ECClass::UNSET_ECCLASSID)
        {
        auto stmt = m_map.GetECDbR ().GetCachedStatement ("SELECT ec_Class.Id FROM ec_Class INNER JOIN ec_Schema ON ec_Schema.Id = ec_Class.SchemaId WHERE ec_Class.Name = 'AnyClass' AND ec_Schema.Name = 'Bentley_Standard_Classes'");
        if (stmt->Step () == BE_SQLITE_ROW)
            m_anyClassId = stmt->GetValueInt64 (0);
        }

    return m_anyClassId;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetRelationshipsForConstraintClass(ECN::ECClassId constraintClassId) const
    {
    LoadRelationshipCache ();
    return m_relationshipClassIdsPerConstraintClassIds[constraintClassId];
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetRelationships (ECN::ECClassId relationshipId) const
    {
    LoadRelationshipCache ();
    return m_relationshipEndsByClassIdRev[relationshipId];
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle 08/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::ConstraintClassIds const& ECDbMap::LightweightCache::GetConstraintClassesForRelationship(ECN::ECClassId relClassId) const
    {
    LoadRelationshipCache();
    return m_nonAnyClassConstraintClassIdsPerRelClassIds[relClassId];
   }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipClassIds const& ECDbMap::LightweightCache::GetAnyClassRelationships() const
    {
    LoadAnyClassRelationships ();
    return m_anyClassRelationships;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipTypeByClassId ECDbMap::LightweightCache::GetRelationshipsMapToTable (ECDbSqlTable const& table) const
    {
    LoadRelationshipByTable ();
    return m_relationshipPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::RelationshipPerTable ECDbMap::LightweightCache::GetRelationshipsMapToTables () const
    {
    LoadRelationshipByTable ();
    return m_relationshipPerTable;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetClassesForTable (ECDbSqlTable const& table) const
    {
    LoadClassIdsPerTable ();
    return m_classIdsPerTable[&table];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
std::vector<ECClassId> const& ECDbMap::LightweightCache::GetAnyClassReplacements() const
    {
    LoadAnyClassReplacements ();
    return m_anyClassReplacements;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
bset<ECDbSqlTable const*> const& ECDbMap::LightweightCache::GetVerticalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadVerticalPartitions();
    return m_verticalPartitions[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      10/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::ClassIdsPerTableMap const& ECDbMap::LightweightCache::GetHorizontalPartitionsForClass(ECN::ECClassId classId) const
    {
    LoadHorizontalPartitions();
    return m_horizontalPartitions[classId];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::Load(bool forceReload)
    {
    if (forceReload)
        Reset();

    LoadAnyClassRelationships();
    LoadRelationshipCache();
    LoadClassIdsPerTable();
    LoadHorizontalPartitions();
    LoadAnyClassReplacements();
    LoadVerticalPartitions();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
void ECDbMap::LightweightCache::Reset ()
    {
    m_loadedFlags.m_classIdsPerTableIsLoaded = 
        m_loadedFlags.m_relationshipPerTableLoaded =
        m_loadedFlags.m_horizontalPartitionsIsLoaded =
        m_loadedFlags.m_verticalPartitionsIsLoaded =
        m_loadedFlags.m_relationshipCacheIsLoaded =
        m_loadedFlags.m_anyClassReplacementsLoaded = 
        m_loadedFlags.m_anyClassRelationshipsIsLoaded = false;

    m_anyClassId = ECClass::UNSET_ECCLASSID;
    m_relationshipEndsByClassIdRev.clear ();
    m_horizontalPartitions.clear();
    m_classIdsPerTable.clear();
    m_relationshipClassIdsPerConstraintClassIds.clear();
    m_nonAnyClassConstraintClassIdsPerRelClassIds.clear();
    m_anyClassRelationships.clear ();
    m_anyClassReplacements.clear ();
    m_storageDescriptions.clear ();
    m_relationshipPerTable.clear ();   
    m_verticalPartitions.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
ECDbMap::LightweightCache::LightweightCache (ECDbMapCR map) : m_map(map) { Reset (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
StorageDescription const& ECDbMap::LightweightCache::GetStorageDescription (IClassMap const& classMap)  const
    {
    const ECClassId classId = classMap.GetClass().GetId();
    auto it = m_storageDescriptions.find(classId);
    if (it == m_storageDescriptions.end())
        {
        auto des = StorageDescription::Create(classMap, *this);
        auto desP = des.get ();
        m_storageDescriptions[classId] = std::move(des);
        return *desP;
        }

    return *(it->second.get());
    }


//****************************************************************************************
// StorageDescription
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription::StorageDescription(StorageDescription&& rhs)
    : m_classId(std::move(rhs.m_classId)), m_horizontalPartitions(std::move(rhs.m_horizontalPartitions)),
    m_nonVirtualHorizontalPartitionIndices(std::move(rhs.m_nonVirtualHorizontalPartitionIndices)),
    m_rootHorizontalPartitionIndex(std::move(rhs.m_rootHorizontalPartitionIndex))
    {}

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription& StorageDescription::operator=(StorageDescription&& rhs)
    {
    if (this != &rhs)
        {
        m_classId = std::move(rhs.m_classId);
        m_horizontalPartitions = std::move(rhs.m_horizontalPartitions);
        m_nonVirtualHorizontalPartitionIndices = std::move(rhs.m_nonVirtualHorizontalPartitionIndices);
        m_rootHorizontalPartitionIndex = std::move(rhs.m_rootHorizontalPartitionIndex);
        }

    return *this;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan    05 / 2015
//------------------------------------------------------------------------------------------
//static
std::unique_ptr<StorageDescription> StorageDescription::Create(IClassMap const& classMap, ECDbMap::LightweightCache const& lwmc)
    {
    const ECClassId classId = classMap.GetClass().GetId();
    std::unique_ptr<StorageDescription> storageDescription = std::unique_ptr<StorageDescription>(new StorageDescription(classId));

    if (classMap.GetClassMapType() == IClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassEndTableMap const& relClassMap = static_cast<RelationshipClassEndTableMap const&> (classMap);
        ECDbSqlTable const& endTable = relClassMap.GetTable();
        const ECDbMap::LightweightCache::RelationshipEnd thisEnd = relClassMap.GetThisEnd() == ECRelationshipEnd::ECRelationshipEnd_Source ? ECDbMap::LightweightCache::RelationshipEnd::Source : ECDbMap::LightweightCache::RelationshipEnd::Target;

        HorizontalPartition* hp = storageDescription->AddHorizontalPartition(endTable, true);

        for (bpair<ECClassId, ECDbMap::LightweightCache::RelationshipEnd> const& kvpair : lwmc.GetConstraintClassesForRelationship(classId))
            {
            ECClassId constraintClassId = kvpair.first;
            ECDbMap::LightweightCache::RelationshipEnd end = kvpair.second;

            if (end == ECDbMap::LightweightCache::RelationshipEnd::Both || end == thisEnd)
                hp->AddClassId(constraintClassId);
            }

        hp->GenerateClassIdFilter(lwmc.GetClassesForTable(endTable));
        }
    else
        {
        for (auto& kp : lwmc.GetHorizontalPartitionsForClass(classId))
            {
            auto table = kp.first;

            auto& deriveClassList = kp.second;
            if (deriveClassList.empty())
                continue;

            HorizontalPartition* hp = storageDescription->AddHorizontalPartition(*table, deriveClassList.front() == classId);
            for (ECClassId ecClassId : deriveClassList)
                {
                hp->AddClassId(ecClassId);
                }

            hp->GenerateClassIdFilter(lwmc.GetClassesForTable(*table));
            }
        }
    //add vertical partitions
    for (auto verticalPartitionTable : lwmc.GetVerticalPartitionsForClass(classId))
        {
        storageDescription->m_verticalPartitions.push_back(VerticalPartition(*verticalPartitionTable));
        }

    return std::move(storageDescription);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
HorizontalPartition const* StorageDescription::GetHorizontalPartition(size_t index) const
    {
    if (index >= m_horizontalPartitions.size())
        {
        BeAssert(false && "Index out of range");
        return nullptr;
        }

    return &m_horizontalPartitions[index];
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
HorizontalPartition* StorageDescription::AddHorizontalPartition(ECDbSqlTable const& table, bool isRootPartition)
    {
    const bool isVirtual = table.GetPersistenceType() == PersistenceType::Virtual;
    m_horizontalPartitions.push_back(HorizontalPartition(table));

    const size_t indexOfAddedPartition = m_horizontalPartitions.size() - 1;
    if (!isVirtual)
        m_nonVirtualHorizontalPartitionIndices.push_back(indexOfAddedPartition);

    if (isRootPartition)
        m_rootHorizontalPartitionIndex = indexOfAddedPartition;

    return &m_horizontalPartitions[indexOfAddedPartition];
    }

//****************************************************************************************
// VerticalPartition
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
VerticalPartition::VerticalPartition(VerticalPartition&& rhs)
    : m_table(std::move(rhs.m_table))    
    {
    //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
    //free the table (as it is now owned by HorizontalPartition). If the ownership ever changes,
    //this method is safe.
    rhs.m_table = nullptr;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
VerticalPartition& VerticalPartition::operator=(VerticalPartition&& rhs)
    {
    if (this != &rhs)
        {
        m_table = std::move(rhs.m_table);
        //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
        //free the table (as it is now owned by HorizontalPartition). If the ownership ever changes,
        //this method is safe.
        rhs.m_table = nullptr;
        }

    return *this;
    }

//****************************************************************************************
// HorizontalPartition
//****************************************************************************************
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
HorizontalPartition::HorizontalPartition(HorizontalPartition&& rhs)
    : m_table(std::move(rhs.m_table)), m_partitionClassIds(std::move(rhs.m_partitionClassIds)),
    m_inversedPartitionClassIds(std::move(rhs.m_inversedPartitionClassIds)), m_hasInversedPartitionClassIds(std::move(rhs.m_hasInversedPartitionClassIds))
    {
    //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
    //free the table (as it is now owned by HorizontalPartition). If the ownership ever changes,
    //this method is safe.
    rhs.m_table = nullptr;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
HorizontalPartition& HorizontalPartition::operator=(HorizontalPartition&& rhs)
    {
    if (this != &rhs)
        {
        m_table = std::move(rhs.m_table);
        m_partitionClassIds = std::move(rhs.m_partitionClassIds);
        m_inversedPartitionClassIds = std::move(rhs.m_inversedPartitionClassIds);
        m_hasInversedPartitionClassIds = std::move(rhs.m_hasInversedPartitionClassIds);

        //nulling out the RHS m_table pointer is defensive, even if the destructor doesn't
        //free the table (as it is now owned by HorizontalPartition). If the ownership ever changes,
        //this method is safe.
        rhs.m_table = nullptr;
        }

    return *this;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
void HorizontalPartition::GenerateClassIdFilter(std::vector<ECN::ECClassId> const& tableClassIds)
    {
    BeAssert(!m_partitionClassIds.empty());
    m_hasInversedPartitionClassIds = m_partitionClassIds.size() > tableClassIds.size() / 2;
    if (m_partitionClassIds.size() == tableClassIds.size())
        return;

    //tableClassIds list is already sorted
    std::sort(m_partitionClassIds.begin(), m_partitionClassIds.end());

    auto partitionClassIdsIt = m_partitionClassIds.begin();
    for (ECClassId candidateClassId : tableClassIds)
        {
        if (partitionClassIdsIt == m_partitionClassIds.end() || candidateClassId < *partitionClassIdsIt)
            m_inversedPartitionClassIds.push_back(candidateClassId);
        else
            ++partitionClassIdsIt;
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Krischan.Eberle    05 / 2015
//------------------------------------------------------------------------------------------
bool HorizontalPartition::NeedsClassIdFilter() const
    {
    BeAssert(!m_partitionClassIds.empty());
    //If class ids are not inversed, we always have a non-empty partition class id list. So filtering is needed.
    //if class ids are inversed, filtering is needed if the inversed list is not empty. If it is empty, it means
    //don't filter at all -> consider all class ids
    return !m_inversedPartitionClassIds.empty();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
void HorizontalPartition::AppendECClassIdFilterSql(NativeSqlBuilder& sqlBuilder) const
    {
    BeAssert(!m_partitionClassIds.empty());

    std::vector<ECClassId> const* classIds = nullptr;
    BooleanSqlOperator inOperator;
    if (m_hasInversedPartitionClassIds)
        {
        classIds = &m_inversedPartitionClassIds;
        if (classIds->empty())
            return; //no filter needed, as all class ids should be considered

        inOperator = BooleanSqlOperator::NotIn;
        }
    else
        {
        classIds = &m_partitionClassIds;
        inOperator = BooleanSqlOperator::In;
        }

    sqlBuilder.Append(inOperator);
    sqlBuilder.AppendParenLeft();
    bool isFirstItem = true;
    for (ECClassId const& classId : *classIds)
        {
        if (!isFirstItem)
            sqlBuilder.AppendComma(false);

        sqlBuilder.Append(classId);

        isFirstItem = false;
        }

    sqlBuilder.AppendParenRight();
    }
END_BENTLEY_SQLITE_EC_NAMESPACE

