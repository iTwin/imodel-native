/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECPersistence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <limits>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPersistence::ECPersistence (IClassMap const& classMap, BeRepositoryBasedIdSequenceR ecInstanceIdSequence)
    : m_ecdb (classMap.GetECDbMap().GetECDbR()), m_ecInstanceIdSequence (ecInstanceIdSequence),
                m_ecClass (classMap.GetClass()), m_ecClassId (classMap.GetClass().GetId())
    {
    // TODO: Further refactor this to just keep the class map and pass that around
    // TODO: Further refactor this to keep a const ref to Map - this shouldn't have to modify the map at all. 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InsertStatus ECPersistence::Insert (ECInstanceId* retECInstanceId, ECN::IECInstanceR instance)
    {
    ECInstanceId ALLOW_NULL_OUTPUT (ecInstanceId, retECInstanceId);
    if (!IsValidInput (instance))
        {
        LOG.errorv(L"ECPersistence::Insert - Input instance's ECClass ('%ls') doesn't match this ECPersistence. Expected ECClass: %ls.",
                    instance.GetClass ().GetFullName (), m_ecClass.GetFullName ());
        return INSERT_InvalidInputInstance;
        }

    if (m_instanceInserter.IsNull())
        {
        InsertStatus status = INSERT_Success;
        m_instanceInserter = InstanceInserter::Create (status, m_ecdb.GetEC ().GetECDbMap (), m_ecClass, m_ecInstanceIdSequence, nullptr);
        if (status != INSERT_Success)
            return status;
        BeAssert (!m_instanceInserter.IsNull());
        }

    InsertStatus status = m_instanceInserter->Insert (&ecInstanceId, instance);
    if (status != INSERT_Success)
        {
        if (status == INSERT_SqliteError || status == INSERT_ConstraintViolation)
            LogLastSqliteError (m_ecdb); //WIP_ECDB: be able to write LOG.errorv ("While doing %s encountered SQLite error %s", foo, LAST_SQLITE_ERROR);
        
        return status;
        }

    return INSERT_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DeleteStatus ECPersistence::Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler /*= nullptr*/)
    {
    ECInstanceIdSet ecInstanceIdSet;
    ecInstanceIdSet.insert (ecInstanceId);
    return Delete (nullptr, ecInstanceIdSet, deleteHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
InstanceDeleterP ECPersistence::GetInstanceDeleter()
    {
    if (m_instanceDeleter.IsNull())
        m_instanceDeleter = InstanceDeleter::Create (m_ecdb.GetEC ().GetECDbMap (), m_ecClass, true);
    return m_instanceDeleter.get();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DeleteStatus ECPersistence::Delete (int32_t* nDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler /*= nullptr*/)
    {
    InstanceDeleterP instanceDeleter = GetInstanceDeleter();
    if (nullptr == instanceDeleter)
        return DELETE_FailedToGenerateSql;

    BentleyStatus status = instanceDeleter->Delete (nDeleted, ecInstanceIdSet, deleteHandler);
    if (status != SUCCESS)
        {
        LogLastSqliteError (m_ecdb);
        return DELETE_SqliteError;
        }

    return DELETE_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateStatus ECPersistence::Update (ECN::IECInstanceR instance)
    {
    if (!IsValidInput (instance))
        {
        LOG.errorv (L"Cannot call ECPersistence::Update with ECInstance whose ECClass is not the same as the ECPersistence's ECClass. Expected ECClass: %ls - Actual ECClass: %ls",
            m_ecClass.GetFullName (), instance.GetClass ().GetFullName ());
        return UPDATE_MismatchedInputInstance;
        }

    //Classes without properties cannot be updated
    if (m_ecClass.GetPropertyCount (true) == 0)
        {
        LOG.errorv ("ECPersistence::Update is not supported for ECClasses without ECProperties. Updating an empty instances virtually is a no-op anyways.");
        return UPDATE_MismatchedInputInstance;
        }

    if (m_instanceUpdater.IsNull())
        m_instanceUpdater = InstanceUpdater::Create (m_ecdb.GetEC ().GetECDbMap (), m_ecClass, m_ecInstanceIdSequence);
    
    if (m_instanceUpdater.IsNull())
        return UPDATE_FailedToGenerateSql;

    //instance must have a instanceId//
    BentleyStatus status = m_instanceUpdater->Update (instance);
    if (status != SUCCESS)
        {
        LogLastSqliteError (m_ecdb);
        return UPDATE_SqliteError;
        }

    return UPDATE_Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECPersistence::IsValidInput (IECInstanceR ecInstance) const
    {
    auto const& inputClass = ecInstance.GetClass ();
    //Always compare by value (which uses the operator from ECSchemaComparers.h). Never compare
    //by reference as there might be different copies in memory of the same logical entity.
    return inputClass == m_ecClass;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassId  ECPersistence::GetClassId () const {return m_ecClassId;}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan        04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECClassCR  ECPersistence::GetClass() const {return m_ecClass;}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                05/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECPersistence::LogLastSqliteError (BeSQLiteDbR db)
    {
    DbResult result;
    Utf8CP details = db.GetLastError (&result);
    LOG.errorv ("DbResult=%s; %s", Db::InterpretDbResult (result), details);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
