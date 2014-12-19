/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECJsonPersistence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

using std::shared_ptr;

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECJsonPersistence::ECJsonPersistence (ECDbR ecDb, ECN::ECClassId ecClassId) 
    : m_ecDb (ecDb), m_ecClassId (ecClassId), m_ecClass (nullptr), 
    m_initialized (false), m_ecPersistence (nullptr)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonPersistence::_Initialize ()
    {
    if (m_initialized)
        return SUCCESS;

    m_ecDb.GetEC().GetSchemaManager().GetECClass (m_ecClass, m_ecClassId);
    PRECONDITION (m_ecClass != nullptr && "Could not retrieve class with specified id", ERROR);
    
    m_ecPersistence = m_ecDb.GetEC().GetECPersistence (nullptr, *m_ecClass);
    if (m_ecPersistence.IsNull())
        return ERROR;

    m_initialized = true;
    return SUCCESS;
    }
    
    
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 9/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECJsonInserter::ECJsonInserter (ECDbR ecDb, ECN::ECClassId ecClassId) 
    : ECJsonPersistence (ecDb, ecClassId)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonInserter::Insert (Json::Value& jsonValue)
    {
    // TODO: Need to return something like an InsertStatus here. I didn't want to introduce
    // InsertStatus now especially since it will get refactored as we deprecate ECPersistence.

    if (SUCCESS != _Initialize())
        return ERROR;
    BeAssert (m_ecClass != nullptr);
    BeAssert (m_ecPersistence.IsValid());

    IECInstancePtr ecInstance = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert (ecInstance.IsValid());
    StatusInt status = ECJsonCppUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue);
    if (status != SUCCESS)
        return status;

    InsertStatus insertStatus = m_ecPersistence->Insert (nullptr, *ecInstance);
    if (insertStatus == INSERT_Success)
        {
        ECInstanceId ecInstanceId;
        if (!GetECInstanceIdFromECInstance (ecInstanceId, *ecInstance))
            BeAssert (false);

        jsonValue["$ECInstanceId"] = BeJsonUtilities::StringValueFromInt64 (ecInstanceId.GetValue ());
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonInserter::Insert (ECInstanceId* instanceIdP, RapidJsonValueCR jsonValue)
    {
    if (nullptr != instanceIdP)
        *instanceIdP = ECInstanceId ();

    if (SUCCESS != _Initialize())
        return ERROR;

    BeAssert (nullptr != m_ecClass);
    BeAssert (m_ecPersistence.IsValid());

    IECInstancePtr instancePtr = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert (instancePtr.IsValid());

    if (SUCCESS != ECRapidJsonUtility::ECInstanceFromJsonValue (*instancePtr, jsonValue))
        return ERROR;

    InsertStatus insertStatus = m_ecPersistence->Insert (nullptr, *instancePtr);
    if (INSERT_Success == insertStatus)
        {
        WString instanceIdStr = instancePtr->GetInstanceId ();
        if (instanceIdStr.empty ())
            BeAssert (false && "ECInstance converted from JSON value does not have an InstanceId");

        ECInstanceId instanceId;
        bool hasInstanceId = GetECInstanceIdFromECInstance (instanceId, *instancePtr);  
        BeAssert (hasInstanceId);
        
        // optionally return ID of newly inserted instance
        if (hasInstanceId && nullptr != instanceIdP)
            *instanceIdP = instanceId;

        return SUCCESS;
        }

    return ERROR;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECJsonInserter::GetECInstanceIdFromECInstance (ECInstanceId& ecInstanceId, ECN::IECInstanceCR instance)
    {
    WString instanceId = instance.GetInstanceId ();
    if (instanceId.empty ())
        return false;

    return ECInstanceIdHelper::FromString (ecInstanceId, instanceId.c_str ());
    }


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 9/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECJsonUpdater::ECJsonUpdater (ECDbR ecDb, ECN::ECClassId ecClassId) 
    : ECJsonPersistence (ecDb, ecClassId)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonUpdater::Update (const Json::Value& jsonValue)
    {
    // TODO: Need to return something like an UpdateStatus here. I didn't want to introduce
    // UpdateStatus now especially since it will get refactored as we deprecate ECPersistence.

    if (SUCCESS != _Initialize())
        return ERROR;
    BeAssert (m_ecClass != nullptr);
    BeAssert (m_ecPersistence.IsValid());

    IECInstancePtr ecInstance = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert (ecInstance.IsValid());
    StatusInt status = ECJsonCppUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue);
    if (status != SUCCESS)
        return status;

    UpdateStatus updateStatus = m_ecPersistence->Update (*ecInstance);
    return (updateStatus == UPDATE_Success) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonUpdater::Update (const ECInstanceId& instanceId, RapidJsonValueCR jsonValue)
    {
    if (SUCCESS != _Initialize())
        return ERROR;

    BeAssert (nullptr != m_ecClass);
    BeAssert (m_ecPersistence.IsValid());

    IECInstancePtr instancePtr = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert (instancePtr.IsValid());

    if (SUCCESS != ECRapidJsonUtility::ECInstanceFromJsonValue (*instancePtr, jsonValue))
        return ERROR;

    if (SUCCESS != ECInstanceAdapterHelper::SetECInstanceId (*instancePtr, instanceId))
        return ERROR;

    return m_ecPersistence->Update (*instancePtr);
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 9/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECJsonDeleter::ECJsonDeleter (ECDbR ecDb, ECN::ECClassId ecClassId) 
    : ECJsonPersistence (ecDb, ecClassId)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 1/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECJsonDeleter::Delete (ECInstanceId ecInstanceId)
    {
    // TODO: Need to return something like an DeleteStatus here. I didn't want to introduce
    // DeleteStatus now especially since it will get refactored as we deprecate ECPersistence.
    
    if (SUCCESS != _Initialize())
        return ERROR;
    BeAssert (m_ecClass != nullptr);
    BeAssert (m_ecPersistence.IsValid());

    DeleteStatus deleteStatus = m_ecPersistence->Delete (ecInstanceId);
    return (deleteStatus == DELETE_Success) ? SUCCESS : ERROR;
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

END_BENTLEY_SQLITE_EC_NAMESPACE

