/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonUpdater.cpp $
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
DbResult JsonUpdater::Update(ECInstanceId instanceId, JsonValueCR jsonValue) const
    {
    if (m_ecClass.GetRelationshipClassCP() != nullptr)
        {
        BeAssert(false && "Use the other Update override for relationships");
        return BE_SQLITE_ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyInstance(m_ecClass);

    if (SUCCESS != ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonValue))
        return BE_SQLITE_ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                10/2015
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonUpdater::Update(ECInstanceId instanceId, JsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        {
        BeAssert(false && "Use the other Update override for non-relationship instances");
        return BE_SQLITE_ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyRelInstance(*relClass, sourceKey, targetKey);

    if (SUCCESS != ECJsonUtilities::ECInstanceFromJson(*ecInstance, jsonValue))
        return BE_SQLITE_ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Shaun.Sewall                    01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonUpdater::Update(ECInstanceId instanceId, RapidJsonValueCR jsonValue) const
    {
    if (m_ecClass.GetRelationshipClassCP() != nullptr)
        {
        BeAssert(false && "Use the other Update override for relationships");
        return BE_SQLITE_ERROR;
        }

    IECInstancePtr ecInstance = ECInstanceAdapterHelper::CreateECInstance(m_ecClass);

    if (SUCCESS != ECRapidJsonUtilities::ECInstanceFromJson(*ecInstance, jsonValue))
        return BE_SQLITE_ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                10/2015
//+---------------+---------------+---------------+---------------+---------------+------
DbResult JsonUpdater::Update(ECInstanceId instanceId, RapidJsonValueCR jsonValue, ECInstanceKeyCR sourceKey, ECInstanceKeyCR targetKey) const
    {
    ECRelationshipClassCP relClass = m_ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        {
        BeAssert(false && "Use the other Update override for non-relationship instances");
        return BE_SQLITE_ERROR;
        }

    IECInstancePtr ecInstance = CreateEmptyRelInstance(*relClass, sourceKey, targetKey);

    if (SUCCESS != ECRapidJsonUtilities::ECInstanceFromJson(*ecInstance, jsonValue))
        return BE_SQLITE_ERROR;

    ECInstanceAdapterHelper::SetECInstanceId(*ecInstance, instanceId);

    return m_ecinstanceUpdater.Update(*ecInstance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2015
//+---------------+---------------+---------------+---------------+---------------+------
IECInstancePtr JsonUpdater::CreateEmptyInstance(ECInstanceKeyCR instanceKey) const
    {
    ECClassCP ecClass = m_ecdb.Schemas().GetClass(instanceKey.GetClassId());
    if (!ecClass)
        return nullptr;

    IECInstancePtr instance = CreateEmptyInstance(*ecClass);
    ECInstanceAdapterHelper::SetECInstanceId(*instance, instanceKey.GetInstanceId());
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


END_BENTLEY_SQLITE_EC_NAMESPACE
