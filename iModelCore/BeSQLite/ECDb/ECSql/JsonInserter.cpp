/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonInserter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonInserter::JsonInserter (ECDbR ecdb, ECN::ECClassCR ecClass)
: m_ecClass (ecClass), m_ecinstanceInserter (ecdb, ecClass)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonInserter::IsValid () const
    {
    return m_ecinstanceInserter.IsValid ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonInserter::Insert (ECInstanceKey& newInstanceKey, JsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler ()->CreateInstance (0);
    BeAssert (ecInstance.IsValid ());
    StatusInt status = ECJsonCppUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue);
    if (status != SUCCESS)
        return ERROR;

    return m_ecinstanceInserter.Insert (newInstanceKey, *ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonInserter::Insert (JsonValueR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler ()->CreateInstance (0);
    BeAssert (ecInstance.IsValid ());
    StatusInt status = ECJsonCppUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue);
    if (status != SUCCESS)
        return ERROR;

    ECInstanceKey newInstanceKey;
    auto stat = m_ecinstanceInserter.Insert (newInstanceKey, *ecInstance);
    if (stat != SUCCESS)
        return ERROR;

    jsonValue["$ECInstanceId"] = BeJsonUtilities::StringValueFromInt64 (newInstanceKey.GetECInstanceId ().GetValue ());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonInserter::Insert (ECInstanceKey& newInstanceKey, RapidJsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = m_ecClass.GetDefaultStandaloneEnabler ()->CreateInstance (0);
    BeAssert (ecInstance.IsValid ());
    if (SUCCESS != ECRapidJsonUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue))
        return ERROR;

    auto stat = m_ecinstanceInserter.Insert (newInstanceKey, *ecInstance);
    if (stat != SUCCESS)
        return ERROR;

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
