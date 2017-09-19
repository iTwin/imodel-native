/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonInserter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(ECInstanceKey& newInstanceKey, JsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert(ecInstance.IsValid());
    if (SUCCESS != JsonECInstanceConverter::JsonToECInstance(*ecInstance, jsonValue, m_ecdb.GetClassLocater()))
        return BE_SQLITE_ERROR;

    return m_ecinstanceInserter.Insert(newInstanceKey, *ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(JsonValueR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert(ecInstance.IsValid());
    if (SUCCESS != JsonECInstanceConverter::JsonToECInstance(*ecInstance, jsonValue, m_ecdb.GetClassLocater()))
        return BE_SQLITE_ERROR;

    ECInstanceKey newInstanceKey;
    DbResult stat = m_ecinstanceInserter.Insert(newInstanceKey, *ecInstance);
    if (BE_SQLITE_OK != stat)
        return stat;

    jsonValue[ECJsonUtilities::json_id()] = newInstanceKey.GetInstanceId().ToHexStr();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonInserter::Insert(ECInstanceKey& newInstanceKey, RapidJsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    BeAssert(ecInstance.IsValid());
    if (SUCCESS != JsonECInstanceConverter::JsonToECInstance(*ecInstance, jsonValue, m_ecdb.GetClassLocater()))
        return BE_SQLITE_ERROR;

    return m_ecinstanceInserter.Insert(newInstanceKey, *ecInstance);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
