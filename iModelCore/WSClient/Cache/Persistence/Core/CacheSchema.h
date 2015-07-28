/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/CacheSchema.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// Schema definitions for DSCacheSchema.01.04.ecschema.xml
//--------------------------------------------------------------------------------------+

#define SCHEMA_CacheSchema                                          "DSCacheSchema"

#define CLASS_Root                                                  "Root"
#define CLASS_Root_PROPERTY_Name                                    "Name"
#define CLASS_Root_PROPERTY_Persistence                             "Persistence"
#define CLASS_Root_PROPERTY_SyncDate                                "SyncDate"

#define CLASS_NavigationBase                                        "NavigationBase"

#define CLASS_FileCacheInfo                                         "FileCacheInfo"
#define CLASS_FileCacheInfo_PROPERTY_LastCachedFileIndex            "LastCachedFileIndex"

#define CLASS_ChangeSequenceInfo                                    "ChangeSequenceInfo"
#define CLASS_ChangeSequenceInfo_PROPERTY_LastChangeNumber          "LastChangeNumber"

#define CLASS_CachedResponseInfo                                    "CachedResponseInfo"
#define CLASS_CachedResponseInfo_PROPERTY_Name                      "Name"
#define CLASS_CachedResponseInfo_PROPERTY_CacheDate                 "CacheDate"
#define CLASS_CachedResponseInfo_PROPERTY_CacheTag                  "CacheTag"
#define CLASS_CachedResponseInfo_PROPERTY_AccessDate                "AccessDate"

#define CLASS_CachedRelationshipInfo                                "CachedRelationshipInfo"
#define CLASS_CachedRelationshipInfo_PROPERTY_RemoteId              "RemoteId"
#define CLASS_CachedRelationshipInfo_PROPERTY_RelClassId            "RelClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_RelInstanceId         "RelInstanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId         "SourceClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId      "SourceInstanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId         "TargetClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId      "TargetInstanceId"

#define CLASS_CachedFileInfo                                        "CachedFileInfo"
#define CLASS_CachedFileInfo_PROPERTY_CacheDate                     "CacheDate"
#define CLASS_CachedFileInfo_PROPERTY_CacheTag                      "CacheTag"

#define CLASS_CachedObjectInfo                                      "CachedObjectInfo"
#define CLASS_CachedObjectInfo_PROPERTY_ClassId                     "ClassId"
#define CLASS_CachedObjectInfo_PROPERTY_LocalId                     "LocalId"
#define CLASS_CachedObjectInfo_PROPERTY_RemoteId                    "RemoteId"
#define CLASS_CachedObjectInfo_PROPERTY_InstanceState               "InstanceState"
#define CLASS_CachedObjectInfo_PROPERTY_CacheDate                   "CacheDate"
#define CLASS_CachedObjectInfo_PROPERTY_CacheTag                    "CacheTag"

#define CLASS_ChangeInfo                                            "ChangeInfo"
#define CLASS_ChangeInfo_PROPERTY_ChangeStatus                      "ChangeStatus"
#define CLASS_ChangeInfo_PROPERTY_SyncStatus                        "SyncStatus"
#define CLASS_ChangeInfo_PROPERTY_ChangeNumber                      "ChangeNumber"

//! Values for property CLASS_CachedObjectInfo_PROPERTY_InstanceState
enum class CachedInstanceState
    {
    Placeholder = 0,
    Partial = 1,
    Full = 2
    };

#define CLASS_REL_RootRelationship                                  "RootRelationship"
#define CLASS_REL_WeakRootRelationship                              "WeakRootRelationship"
#define CLASS_REL_CachedFileInfoRelationship                        "CachedFileInfoRelationship"
#define CLASS_REL_CachedObjectInfoRelationship                      "CachedObjectInfoRelationship"
#define CLASS_REL_CachedResponseInfoToResultRelationship            "CachedResponseInfoToResultRelationship"
#define CLASS_REL_CachedResponseInfoToResultWeakRelationship        "CachedResponseInfoToResultWeakRelationship"
#define CLASS_REL_CachedResponseInfoToParentRelationship            "CachedResponseInfoToParentRelationship"
#define CLASS_REL_CachedResponseInfoToHolderRelationship            "CachedResponseInfoToHolderRelationship"
#define CLASS_REL_CachedResponseInfoToCachedRelationshipInfo        "CachedResponseInfoToCachedRelationshipInfo"

#define ECSql_RootClass                                             "[DSC].[Root]"
#define ECSql_CachedObjectInfoClass                                 "[DSC].[CachedObjectInfo]"
#define ECSql_CachedObjectInfoRelationshipClass                     "[DSC].[CachedObjectInfoRelationship]"
#define ECSql_CachedResponseInfoClass                               "[DSC].[CachedResponseInfo]"
#define ECSql_CachedRelationshipInfoClass                           "[DSC].[CachedRelationshipInfo]"
#define ECSql_CachedFileInfoClass                                   "[DSC].[CachedFileInfo]"
#define ECSql_CachedFileInfoRelationshipClass                       "[DSC].[CachedFileInfoRelationship]"
#define ECSql_ChangeInfoClass                                       "[DSC].[ChangeInfo]"
#define ECSql_NavigationBaseClass                                   "[DSC].[NavigationBase]"

#define ECSql_CachedResponseInfoToResultRelationshipClass           "[DSC].[CachedResponseInfoToResultRelationship]"
#define ECSql_CachedResponseInfoToResultWeakRelationshipClass       "[DSC].[CachedResponseInfoToResultWeakRelationship]"

#define SCHEMA_CacheLegacySupportSchema                             "DSCacheLegacySupportSchema"
#define SCHEMA_CacheLegacySupportSchema_LegacyParentRelationship    "LegacyParentRelationship"

END_BENTLEY_WEBSERVICES_NAMESPACE
