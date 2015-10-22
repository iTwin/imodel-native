/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonUpdater.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonUpdater::JsonUpdater (ECDbCR ecdb, ECN::ECClassCR ecClass)
: m_ecClass (ecClass), m_ecinstanceUpdater (ecdb, ecClass)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonUpdater::IsValid () const
    {
    return m_ecinstanceUpdater.IsValid ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update (JsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = ECInstanceAdapterHelper::CreateECInstance(m_ecClass);
    BeAssert (ecInstance.IsValid ());
    StatusInt status = ECJsonCppUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue);
    if (status != SUCCESS)
        return ERROR;

    return m_ecinstanceUpdater.Update (*ecInstance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update (ECInstanceId const& instanceId, RapidJsonValueCR jsonValue) const
    {
    IECInstancePtr ecInstance = ECInstanceAdapterHelper::CreateECInstance(m_ecClass);
    BeAssert (ecInstance.IsValid ());
    if (SUCCESS != ECRapidJsonUtility::ECInstanceFromJsonValue (*ecInstance, jsonValue))
        return ERROR;

    ECInstanceAdapterHelper::SetECInstanceId (*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update (*ecInstance);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
