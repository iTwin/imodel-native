/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ExtendedDataDelegate.h"
#include "../Core/CacheSchema.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ExtendedDataDelegate::GetExtendedDataClass(ECInstanceKeyCR ownerKey)
    {
    return m_dbAdapter.GetECClass(SCHEMA_CacheSchema, CLASS_ExtendedData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ExtendedDataDelegate::GetExtendedDataRelationshipClass(ECInstanceKeyCR ownerKey)
    {
    return m_dbAdapter.GetECRelationshipClass(SCHEMA_CacheSchema, CLASS_NodeToExtendedData);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKey ExtendedDataDelegate::GetHolderKey(ECInstanceKeyCR ownerKey)
    {
    ECClassCP ownerClass = m_dbAdapter.GetECClass(ownerKey);
    if (nullptr == ownerClass || ownerClass->GetSchema().GetName() == SCHEMA_CacheSchema)
        {
        return ownerKey;
        }

    if (ownerClass->IsRelationshipClass())
        {
        return m_relationshipInfoManager.ReadCachedRelationshipKey(ownerKey).GetInfoKey();
        }

    return m_objectInfoManager.ReadCachedInstanceKey(ownerKey).GetInfoKey();
    }
