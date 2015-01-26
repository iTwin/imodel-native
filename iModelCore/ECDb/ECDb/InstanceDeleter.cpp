/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/InstanceDeleter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//************* InstanceDeleter ****************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeleter::InstanceDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, bool deleteDependentInstances)
: m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecProperty (nullptr), m_deleteDependentInstances (deleteDependentInstances), m_instanceFinder (nullptr)/*, m_ecPropertyIdForPersistence (0)*/
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeleter::InstanceDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCP ecProperty, ECPropertyId propertyIdForPersistence)
: m_ecDbMap (ecDbMap), m_ecClass (ecClass), m_ecProperty (ecProperty), m_deleteDependentInstances (false), m_propertyIdForPersistence (propertyIdForPersistence)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool InstanceDeleter::IsTopLevelDeleter()
    {
    return m_ecProperty == nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeleter::~InstanceDeleter ()
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus InstanceDeleter::Initialize()
    {
    return _Initialize();
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceFinder* InstanceDeleter::GetInstanceFinder()
    {
    if (m_instanceFinder == nullptr)
        m_instanceFinder = std::unique_ptr<ECInstanceFinder> (new ECInstanceFinder (m_ecDbMap.GetECDbR()));
    return m_instanceFinder.get();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus InstanceDeleter::_Initialize()
    {
    IClassMap const* classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (classMap != nullptr, MapStatus::Error);
    PRECONDITION (!classMap->IsUnmapped(), MapStatus::Error);

    ECDbSqlTable const& table = classMap->GetTable();
    if (!m_ecDbMap.GetECDbR().TableExists (table.GetName().c_str()))
        return MapStatus::Error;
        
    if (!IsTopLevelDeleter ())
        m_ecPropertyId = m_ecProperty->GetId();

    DbResult result = PrepareDeleteStatement();
    if (result != BE_SQLITE_OK)
        return MapStatus::Error;

    InitializeStructArrayDeleters (*classMap, table);

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceDeleter::GetDeleteSql (Utf8StringR deleteSql)
    {
    IClassMap const* classMap = m_ecDbMap.GetClassMap (m_ecClass);
    ECDbSqlTable const& table = classMap->GetTable();

    SqlDelete deleteBuilder;
    deleteBuilder.SetTable (table.GetName ().c_str ());

    if (IsTopLevelDeleter())
        {
        if (m_ecClass.GetIsStruct())
            {
            deleteBuilder.AddWhere (ECDB_COL_ECPropertyId, "is null");
            deleteBuilder.AddWhere (ECDB_COL_ECArrayIndex, "is null");
            }
        }
    else
        {
        // A child is always a struct
        deleteBuilder.AddWhere (ECDB_COL_ECPropertyId, " = ? ");
        }

    if (m_ecClass.GetIsStruct())
        deleteBuilder.AddWhere (ECDB_COL_OwnerECInstanceId, " = ?");
    else
        {
        auto ecInstanceIdPropMap = classMap->GetPropertyMap (L"ECInstanceId");
        BeAssert (ecInstanceIdPropMap != nullptr);
        auto nativeSqlSnippets = ecInstanceIdPropMap->ToNativeSql (nullptr, ECSqlType::Delete);
        if (nativeSqlSnippets.size () == 1)
            {
            //must call this overload, as other overload escapes lhs operand and ToNativeSql also
            //escapes lhs. Double escaping is not allowed in SQLite.
            Utf8String whereClause (nativeSqlSnippets[0].ToString ());
            whereClause.append (" = ?");
            deleteBuilder.AddWhere (whereClause.c_str ());
            }
        else
            BeAssert (false && "ECInstanceId prop map is expected to map to a single column.");
        }

    auto classIdColumn = table.FindColumnCP("ECClassId"); 
    if (nullptr != classIdColumn)
        deleteBuilder.AddWhere (classIdColumn->GetName().c_str(), Utf8PrintfString (" = %lld", classMap->GetClass().GetId()).c_str());

    bool status = deleteBuilder.GetSql (deleteSql);
    BeAssert (status);
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceDeleter::PrepareDeleteStatement()
    {
    Utf8String deleteSql;
    GetDeleteSql (deleteSql);

    m_deleteStatement = new CachedStatement (deleteSql.c_str());
    DbResult result = m_deleteStatement->Prepare (m_ecDbMap.GetECDbR(), deleteSql.c_str());
    if (result != BE_SQLITE_OK)
        m_deleteStatement = nullptr;
    return result;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult InstanceDeleter::GetDeleteStatement (CachedStatementPtr& deleteStatement)
    {
    DbResult result = BE_SQLITE_OK;

    if (m_deleteStatement.IsNull() /* Statement has not been created before */  ||
        m_deleteStatement->GetRefCount() > 1 /* Statement is currently in use */)
        {
        DbResult result = PrepareDeleteStatement();
        if (result != BE_SQLITE_OK)
            return result;
        }
    else
        {
        m_deleteStatement->Reset();
        m_deleteStatement->ClearBindings();
        }

    deleteStatement = m_deleteStatement;
    return result;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceDeleter::InitializeStructArrayDeleters (IClassMap const& classMap, ECDbSqlTable const& table)
    {
    classMap.GetPropertyMaps ().Traverse ([this, &classMap] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        PropertyMapToTableCP propertyMapToTable = propMap->GetAsPropertyMapToTable ();
        if (propertyMapToTable != nullptr)
            {
            ECPropertyId ecPropertyIdForPersistence = 0;
            ecPropertyIdForPersistence = propMap->GetECPropertyIdForPersistence (classMap.GetClass ().GetId (), m_ecDbMap.GetECDbR ());
            InstanceDeleterPtr deleter = CreateChildDeleter (m_ecDbMap, propertyMapToTable->GetElementType (), propertyMapToTable->GetProperty (), ecPropertyIdForPersistence);
            BeAssert (!deleter.IsNull ());
            m_propertyDeleters.push_back (deleter);
            }
        }, true);
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeleterPtr InstanceDeleter::CreateChildDeleter (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, ECN::ECPropertyId propertyIdForPersistence)
    {
    InstanceDeleterPtr deleter = new InstanceDeleter (ecDbMap, ecClass, &ecProperty, propertyIdForPersistence);
    MapStatus status = deleter->Initialize();
    return (MapStatus::Success == status) ? deleter : nullptr;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceDeleterPtr InstanceDeleter::Create (ECDbMapCR ecDbMap, ECN::ECClassCR ecClass, bool deleteDependentInstances)
    {
    InstanceDeleterPtr deleter;

    ECRelationshipClassCP ecRelClass = ecClass.GetRelationshipClassCP();
    if (ecRelClass == nullptr)
        deleter = new InstanceDeleter (ecDbMap, ecClass, deleteDependentInstances);
    else
        deleter = new RelationshipInstanceDeleter (ecDbMap, *ecRelClass);

    MapStatus status = deleter->Initialize();
    if (MapStatus::Success != status)
        return nullptr;
    
    return deleter;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler)
    {
    ECInstanceIdSet ecInstanceIdSet;
    ecInstanceIdSet.insert (ecInstanceId);
    int numDeleted = 0;
    return _Delete (numDeleted, ecInstanceIdSet, deleteHandler);
    }
    
/*---------------------------------------------------------------------------------------
* TO BE DEPRECATED
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::Delete (int* pNumDeleted, Utf8CP whereCriteria, ECDbDeleteHandlerP deleteHandler)
    {
    int ALLOW_NULL_OUTPUT (numDeleted, pNumDeleted);
    numDeleted = 0;
    
    // Find instances to be deleted
    ECInstanceIdSet instanceIds;
    BentleyStatus status = FindInstances (instanceIds, whereCriteria);
    if (SUCCESS != status)
        return status;

    return _Delete (numDeleted, instanceIds, deleteHandler);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::Delete (int* pNumDeleted, const ECInstanceIdSet& ecInstanceIds, ECDbDeleteHandlerP deleteHandler)
    {
    int ALLOW_NULL_OUTPUT (numDeleted, pNumDeleted);
    numDeleted = 0;
    return _Delete (numDeleted, ecInstanceIds, deleteHandler);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::FindOrphanedInstances 
(
ECInstanceKeyMultiMap& orphanedRelationshipInstances,
ECInstanceKeyMultiMap& orphanedInstances, 
ECClassId deletedClassId, 
ECInstanceId deletedInstanceId, 
ECDbR ecDb
)
    {
    ECInstanceKey seedInstanceKey (deletedClassId, deletedInstanceId);
    
    ECInstanceFinder* instanceFinder = GetInstanceFinder();

    // Add *all* relationship instances that relate the deleted instance (at either end)
    ECInstanceKeyMultiMap allRelationshipInstances;
    BentleyStatus status = instanceFinder->FindRelatedInstances (nullptr, &allRelationshipInstances, seedInstanceKey, ECInstanceFinder::RelatedDirection_All);
    POSTCONDITION (status == SUCCESS, status);
    orphanedRelationshipInstances.insert (allRelationshipInstances.begin(), allRelationshipInstances.end());

    // Add embedded children
    ECInstanceKeyMultiMap embeddedChildren;
    status = instanceFinder->FindRelatedInstances (&embeddedChildren, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_EmbeddedChildren);
    POSTCONDITION (status == SUCCESS, status);
    orphanedInstances.insert (embeddedChildren.begin(), embeddedChildren.end());

    // Add held relationship instances, and child instances (only if they don't have any other parents left)
    ECInstanceKeyMultiMap heldChildren;
    status = instanceFinder->FindRelatedInstances (&heldChildren, nullptr, seedInstanceKey, ECInstanceFinder::RelatedDirection_HeldChildren);
    POSTCONDITION (status == SUCCESS, status);
    for (ECInstanceKeyMultiMapConstIterator iter = heldChildren.begin(); iter != heldChildren.end(); iter++)
        {
        ECClassId relatedClassId = iter->first;
        ECInstanceId relatedInstanceId = iter->second;
        seedInstanceKey = ECInstanceKey (iter->first, iter->second);

        ECInstanceKeyMultiMap holdingParentRelationships;
        status = instanceFinder->FindRelatedInstances (nullptr, &holdingParentRelationships, seedInstanceKey, ECInstanceFinder::RelatedDirection_HoldingParents);
        POSTCONDITION (status == SUCCESS, status);
        if (holdingParentRelationships.size() < 2)
            {
            ECInstanceKeyMultiMapPair mapEntry (relatedClassId, relatedInstanceId);
            orphanedInstances.insert (mapEntry);
            }
        }

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::DeleteInstances (int& numDeleted, const ECInstanceKeyMultiMap& instanceMap, ECDbR ecDb, ECDbDeleteHandlerP deleteHandler)
    {
    numDeleted = 0;
    ECInstanceKeyMultiMapConstIterator instanceIdIter;
    for (ECInstanceKeyMultiMapConstIterator classIdIter = instanceMap.begin(); classIdIter != instanceMap.end(); classIdIter = instanceIdIter)
        {
        ECClassId classId = classIdIter->first;
        ECClassCP ecClass = ecDb.GetSchemaManager ().GetECClass (classId);
        if (ecClass == nullptr)
            {
            LOG.errorv ("Failed to get ECClass for ECClassId %x", classId);
            return ERROR;
            }

        ECPersistencePtr persistence = ecDb.GetImplR ().GetECPersistence (*ecClass);
        if (!persistence.IsValid())
            return ERROR;

        ECInstanceIdSet ecInstanceIdSet;
        bpair<ECInstanceKeyMultiMapConstIterator, ECInstanceKeyMultiMapConstIterator> keyRange = instanceMap.equal_range (classId);
        for (instanceIdIter = keyRange.first; instanceIdIter != keyRange.second; instanceIdIter++)
            ecInstanceIdSet.insert (instanceIdIter->second);

        int numChildDeleted;
        DeleteStatus deleteStatus = persistence->Delete (&numChildDeleted, ecInstanceIdSet, deleteHandler);
        if (deleteStatus != DELETE_Success)
            return ERROR;
        numDeleted += numChildDeleted;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::DeleteDependentInstances (int& numDeleted, const ECInstanceIdSet& deletedInstanceIds, ECDbDeleteHandlerP deleteHandler)
    {
    numDeleted = 0;
    ECDbR ecDb = m_ecDbMap.GetECDbR();
    for (ECInstanceId deletedInstanceId : deletedInstanceIds)
        {
        ECInstanceKeyMultiMap orphanedRelationshipInstances;
        ECInstanceKeyMultiMap orphanedInstances;

        BentleyStatus status = FindOrphanedInstances (orphanedRelationshipInstances, orphanedInstances, m_ecClass.GetId(), deletedInstanceId, ecDb);
        if (status != SUCCESS)
            return status;

        /* 
        * Note: Delete relationship instances first. Deleting instances that hold relationships as 
        * FKeys causes the relationship to get deleted automatically, messing up the count of 
        * number of items deleted. 
        */
        int numRelationshipsDeleted;
        status = DeleteInstances (numRelationshipsDeleted, orphanedRelationshipInstances, ecDb, deleteHandler);
        if (status != SUCCESS)
            return status;
        numDeleted += numRelationshipsDeleted;

        int numInstancesDeleted;
        status = DeleteInstances (numInstancesDeleted, orphanedInstances, ecDb, deleteHandler);
        if (status != SUCCESS)
            return status;
        numDeleted += numInstancesDeleted;
        }

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceDeleter::GetSelectSql (Utf8StringR selectSql, Utf8CP whereCriteria)
    {
    BeAssert (IsTopLevelDeleter());
    IClassMap const* classMap = m_ecDbMap.GetClassMap (m_ecClass);
    ECDbSqlTable const& table = classMap->GetTable();

    SqlSelect selectBuilder;

    selectBuilder.AddFrom (table.GetName ().c_str ());
    selectBuilder.AddSelect (ECDB_COL_ECInstanceId);
    if (m_ecClass.GetIsStruct())
        {
        selectBuilder.AddWhere (ECDB_COL_ECPropertyId, "is null");
        selectBuilder.AddWhere (ECDB_COL_ECArrayIndex, "is null");
        }

    auto classIdColumn = table.FindColumnCP("ECClassId"); 
    if (nullptr != classIdColumn)
        selectBuilder.AddWhere (classIdColumn->GetName ().c_str (), Utf8PrintfString (" = %lld", classMap->GetClass ().GetId ()).c_str ());

    if (!Utf8String::IsNullOrEmpty (whereCriteria))
        selectBuilder.AddWhere (whereCriteria);
        
    bool status = selectBuilder.GetSql (selectSql);
    BeAssert (status);
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::FindInstances (ECInstanceIdSet& instanceIds, Utf8CP whereCriteria)
    {
    //TODO: We should cache finalSql and see if whereCriteria have changed.
    BeAssert (IsTopLevelDeleter());
    Utf8String selectSql;
    GetSelectSql (selectSql, whereCriteria);

    // Find the ids to be used for deleting related data in other tables
    Statement stmt;
    DbResult r = stmt.Prepare (m_ecDbMap.GetECDbR(), selectSql.c_str());
    if (BE_SQLITE_OK != r)
        return ERROR;
    while ((r =  stmt.Step()) == BE_SQLITE_ROW)
        {
        ECInstanceId instanceId = stmt.GetValueId<ECInstanceId> (0);
        instanceIds.insert (instanceId);
        }

    return (r == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::_Delete (int& numDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler)
    {
    if (ecInstanceIdSet.empty())
        return SUCCESS;

    // Invoke any before-delete callbacks
    OnBeforeDelete (deleteHandler, ecInstanceIdSet);

    // Delete any struct array properties (entries in a different table)
    BentleyStatus status = DeleteStructArrays (ecInstanceIdSet);
    if (SUCCESS != status)
        return status;

    // Delete any dependencies
    if (m_deleteDependentInstances)
        {
        BeAssert (IsTopLevelDeleter());
        int numChildDeleted;
        status = DeleteDependentInstances (numChildDeleted, ecInstanceIdSet, deleteHandler);
        if (SUCCESS != status)
            {
            numDeleted = 0;
            return status;
            }
        numDeleted += numChildDeleted;
        }

    // Delete the entries in the main table
    CachedStatementPtr deleteStatement;
    DbResult result = GetDeleteStatement (deleteStatement);
    if (BE_SQLITE_OK != result)
        {
        numDeleted = 0;
        return ERROR;
        }
        
    ECDbR ecDb = m_ecDbMap.GetECDbR();
    for (ECInstanceId ecInstanceId : ecInstanceIdSet)
        {
        deleteStatement->Reset();
        deleteStatement->BindId (1, ecInstanceId);
        result = deleteStatement->Step();
        POSTCONDITION (result == BE_SQLITE_DONE, ERROR);
        numDeleted += ecDb.GetModifiedRowCount();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus InstanceDeleter::DeleteStructArrays (const ECInstanceIdSet& ecInstanceIdSet)
    {
    for (InstanceDeleterPtr deleter: m_propertyDeleters)
        {
        if (SUCCESS != deleter->DeleteStructArrays (ecInstanceIdSet))
            return ERROR;
        }

    if (!IsTopLevelDeleter())
        {
        CachedStatementPtr deleteStatement;
        DbResult result = GetDeleteStatement (deleteStatement);
        if (BE_SQLITE_OK != result)
            return ERROR;

        BeAssert (m_propertyIdForPersistence != 0);
        deleteStatement->BindInt64 (1, m_propertyIdForPersistence);
        for (ECInstanceId ecInstanceId : ecInstanceIdSet)
            {
            deleteStatement->Reset();
            deleteStatement->BindId (2, ecInstanceId);
            result = deleteStatement->Step();
            POSTCONDITION (result == BE_SQLITE_DONE, ERROR);
            }
        }

    return SUCCESS;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                09/2013
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceDeleter::OnBeforeDelete (ECDbDeleteHandlerP deleteHandler, const ECInstanceIdSet& instanceIds)
    {
    if (deleteHandler == nullptr)
        return;
    for (ECInstanceId ecInstanceId : instanceIds)
        deleteHandler->_OnBeforeDelete (m_ecClass, ecInstanceId, m_ecDbMap.GetECDbR ());
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void InstanceDeleter::_ClearCache()
    {
    m_deleteStatement = nullptr;
    m_instanceFinder = nullptr;
    }
    
//************* RelationshipInstanceDeleter ****************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipInstanceDeleter::RelationshipInstanceDeleter (ECDbMapCR ecDbMap, ECN::ECRelationshipClassCR ecRelClass)
: InstanceDeleter (ecDbMap, ecRelClass, false)
    {
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus RelationshipInstanceDeleter::_Initialize()
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    PRECONDITION (classMap != nullptr, MapStatus::Error);
    PRECONDITION (!classMap->IsUnmapped(), MapStatus::Error);

    if (classMap->GetClassMapType () == ClassMap::Type::RelationshipLinkTable)
        return InstanceDeleter::_Initialize();

    /* else */
    BeAssert (classMap->GetClassMapType () == ClassMap::Type::RelationshipEndTable &&
              dynamic_cast<RelationshipClassEndTableMapCP> (classMap) != nullptr);
    RelationshipClassEndTableMapCP relationshipClassMap = static_cast<RelationshipClassEndTableMapCP> (classMap);
    BeAssert (relationshipClassMap != nullptr);

    DbResult result = PrepareUpdateStatement ();
    if (result != BE_SQLITE_OK)
        return MapStatus::Error;

    // Setup deleting struct array property entries (these are rows in secondary property tables that are to be deleted)
    ECDbSqlTable const& endTable = relationshipClassMap->GetTable ();
    InitializeStructArrayDeleters (*relationshipClassMap, endTable);
    return MapStatus::Success;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipInstanceDeleter::GetUpdateSql (Utf8StringR updateSql)
    {
    RelationshipClassEndTableMapCP relationshipClassMap = static_cast<RelationshipClassEndTableMapCP> (m_ecDbMap.GetClassMap (m_ecClass));
    BeAssert (!relationshipClassMap->IsUnmapped());
    ECDbSqlTable const& endTable = relationshipClassMap->GetTable ();

    SqlUpdate updateBuilder;

    // UPDATE Table
    updateBuilder.SetTable (endTable.GetName ().c_str ());

    // RK_Column = NULL
    auto rkColumn = relationshipClassMap->GetOtherEndECInstanceIdPropMap ()->GetFirstColumn ();
    BeAssert (rkColumn != nullptr);
    updateBuilder.AddUpdateColumn (rkColumn->GetName ().c_str (), "NULL");

    // RC_Column = NULL
    auto rcColumn = relationshipClassMap->GetOtherEndECClassIdPropMap ()->GetFirstColumn();
    BeAssert (rcColumn != nullptr);
    if (rcColumn->GetPersistenceType() == PersistenceType::Persisted)
        updateBuilder.AddUpdateColumn (rcColumn->GetName ().c_str (), "NULL");

    // Set non-struct-array properties to NULL
    Bindings parameterBindings;
    int nextParameterIndex = 1;
    BentleyStatus s = relationshipClassMap->GenerateParameterBindings (parameterBindings, nextParameterIndex);
    if (s != SUCCESS)
        { BeAssert(false); }
    
    for (Binding& binding : parameterBindings)
        {
        if (!binding.m_column)
            continue;
        updateBuilder.AddUpdateColumn (binding.m_column->GetName().c_str(), "NULL");
        nextParameterIndex++;
        }

    auto thisEndKeyColumn = relationshipClassMap->GetThisEndECInstanceIdPropMap ()->GetFirstColumn ();
    BeAssert (thisEndKeyColumn != nullptr);
    updateBuilder.AddWhere (thisEndKeyColumn->GetName ().c_str (), " = ?");

    bool status = updateBuilder.GetSql (updateSql);
    BeAssert (status);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceDeleter::PrepareUpdateStatement()
    {
    Utf8String updateSql;
    GetUpdateSql (updateSql);

    m_updateStatement = new CachedStatement (updateSql.c_str());
    DbResult result = m_updateStatement->Prepare (m_ecDbMap.GetECDbR(), updateSql.c_str());
    if (result != BE_SQLITE_OK)
        m_updateStatement = nullptr;
    return result;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult RelationshipInstanceDeleter::GetUpdateStatement (CachedStatementPtr& updateStatement)
    {
    DbResult result = BE_SQLITE_OK;

    if (m_updateStatement.IsNull() /* Statement has not been created before */  ||
        m_updateStatement->GetRefCount() > 1 /* Statement is currently in use */)
        {
        DbResult result = PrepareUpdateStatement();
        if (result != BE_SQLITE_OK)
            return result;
        }
    else
        {
        m_updateStatement->Reset();
        m_updateStatement->ClearBindings();
        }

    updateStatement = m_updateStatement;
    return result;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInstanceDeleter::DeleteFromRelationshipEndTable (int& numDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler)
    {
    // Invoke any before-delete callbacks
    OnBeforeDelete (deleteHandler, ecInstanceIdSet);

    // Delete any struct array properties (entries in a different table)
    BentleyStatus status = DeleteStructArrays (ecInstanceIdSet);
    if (SUCCESS != status)
        return status;

    CachedStatementPtr updateStatement;
    DbResult result = GetUpdateStatement (updateStatement);
    if (BE_SQLITE_OK != result)
        {
        numDeleted = 0;
        return ERROR;
        }

    ECDbR ecDb = m_ecDbMap.GetECDbR();
    for (ECInstanceId ecInstanceId : ecInstanceIdSet)
        {
        updateStatement->Reset();
        updateStatement->BindId (1, ecInstanceId);
        result = updateStatement->Step();
        POSTCONDITION (result == BE_SQLITE_DONE, ERROR);
        numDeleted += ecDb.GetModifiedRowCount();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RelationshipInstanceDeleter::_Delete (int& numDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler)
    {
    auto classMap = m_ecDbMap.GetClassMap (m_ecClass);
    BeAssert (classMap != nullptr && !classMap->IsUnmapped());

    if (classMap->GetClassMapType () == ClassMap::Type::RelationshipLinkTable)
        return InstanceDeleter::_Delete (numDeleted, ecInstanceIdSet, deleteHandler);

    /* else */
    return DeleteFromRelationshipEndTable (numDeleted, ecInstanceIdSet, deleteHandler);
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                 Ramanujam.Raman     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipInstanceDeleter::_ClearCache()
    {
    InstanceDeleter::_ClearCache();
    m_updateStatement = nullptr;
    }




END_BENTLEY_SQLITE_EC_NAMESPACE
