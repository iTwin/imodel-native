/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonUpdater.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
JsonUpdater::JsonUpdater (ECDbCR ecdb, ECClassCR ecClass, Utf8CP ecsqlOptions)
: m_ecdb (ecdb), m_ecClass (ecClass), m_ecinstanceUpdater (ecdb, ecClass, ecsqlOptions)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonUpdater::IsValid () const
    {
    return m_ecinstanceUpdater.IsValid ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2015
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr JsonUpdater::CreateEmptyInstance(ECClassCR ecClass) const
    {
    return ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2015
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr JsonUpdater::CreateEmptyInstance(ECInstanceKeyCR instanceKey) const
    {
    ECClassCP ecClass = m_ecdb.Schemas().GetECClass(instanceKey.GetECClassId());
    if (!ecClass)
        return nullptr;

    IECInstancePtr instance = CreateEmptyInstance(*ecClass);
    ECInstanceAdapterHelper::SetECInstanceId(*instance, instanceKey.GetECInstanceId());
    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2015
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr JsonUpdater::CreateEmptyRelInstance(ECRelationshipClassCR ecRelClass, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const
    {
    IECInstancePtr sourceInst = CreateEmptyInstance(sourceKey);
    IECInstancePtr targetInst = CreateEmptyInstance(targetKey);
    if (sourceInst == nullptr || targetInst == nullptr)
        return nullptr;

    StandaloneECRelationshipInstancePtr relInst = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(ecRelClass)->CreateRelationshipInstance();
    relInst->SetSource(sourceInst.get());
    relInst->SetTarget(targetInst.get());

    return relInst.get();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update(JsonValueCR jsonValue) const
    {
    ECInstanceId instanceId = ECInstanceId((uint64_t) BeJsonUtilities::Int64FromValue(jsonValue["$ECInstanceId"]));
    if (!instanceId.IsValid())
        return ERROR;

    return Update(instanceId, jsonValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 9/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update(ECInstanceId const& instanceId, JsonValueCR jsonValue) const
    {
    if (m_ecClass.GetRelationshipClassCP() != nullptr)
        {
        BeAssert(false && "Use the other Update override for relationships");
        return ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyInstance(m_ecClass);

    if (SUCCESS != ECJsonUtilities::ECInstanceFromJsonValue (*ecInstance, jsonValue))
        return ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update (*ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                10/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update(ECInstanceId const& instanceId, JsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        {
        BeAssert(false && "Use the other Update override for non-relationship instances");
        return ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyRelInstance(*relClass, sourceKey, targetKey);

    if (SUCCESS != ECJsonUtilities::ECInstanceFromJsonValue(*ecInstance, jsonValue))
        return ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonUpdater::Update (ECInstanceId const& instanceId, RapidJsonValueCR jsonValue) const
    {
    if (m_ecClass.GetRelationshipClassCP() != nullptr)
        {
        BeAssert(false && "Use the other Update override for relationships");
        return ERROR;
        }

    IECInstancePtr ecInstance = ECInstanceAdapterHelper::CreateECInstance(m_ecClass);
    
    if (SUCCESS != ECRapidJsonUtilities::ECInstanceFromJsonValue(*ecInstance, jsonValue))
        return ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update (*ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                10/2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus  JsonUpdater::Update(ECInstanceId const& instanceId, RapidJsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        {
        BeAssert(false && "Use the other Update override for non-relationship instances");
        return ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyRelInstance(*relClass, sourceKey, targetKey);
   
    if (SUCCESS != ECRapidJsonUtilities::ECInstanceFromJsonValue(*ecInstance, jsonValue))
        return ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
