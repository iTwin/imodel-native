/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/CacheSchema.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// Schema definitions for DSCacheSchema ECSchema
//--------------------------------------------------------------------------------------+

#define SCHEMA_CacheSchema_Major                                    1
#define SCHEMA_CacheSchema_Minor                                    8

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
#define CLASS_CachedResponseInfo_PROPERTY_AccessDate                "AccessDate"
#define CLASS_CachedResponseInfo_PROPERTY_IsCompleted               "IsCompleted"

#define CLASS_CachedResponsePageInfo                                "CachedResponsePageInfo"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheDate             "CacheDate"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheTag              "CacheTag"
#define CLASS_CachedResponsePageInfo_PROPERTY_Index                 "Index"

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
#define CLASS_ChangeInfo_PROPERTY_Revision                          "Revision"
#define CLASS_ChangeInfo_PROPERTY_IsLocal                           "IsLocal"

#define CLASS_InstanceBackup                                        "InstanceBackup"
#define CLASS_InstanceBackup_PROPERTY_Instance                      "Instance"

//! Values for property CLASS_CachedObjectInfo_PROPERTY_InstanceState
enum class CachedInstanceState
    {
    Placeholder = 0,
    Partial = 1,
    Full = 2
    };

#define CLASS_RootRelationship                                      "RootRelationship"
#define CLASS_WeakRootRelationship                                  "WeakRootRelationship"
#define CLASS_CachedFileInfoToInstance                              "CachedFileInfoRelationship"
#define CLASS_CachedObjectInfoToInstance                            "CachedObjectInfoRelationship"
#define CLASS_ResponseInfoToParent                                  "CachedResponseInfoToParentRelationship"
#define CLASS_ResponseInfoToHolder                                  "CachedResponseInfoToHolderRelationship"
#define CLASS_ResponseToResponsePage                                "ResponseToResponsePage"
#define CLASS_ResponseToAdditionalInstance                          "ResponseToAdditionalInstance"
#define CLASS_ResponsePageToResult                                  "ResponsePageToResult"
#define CLASS_ResponsePageToResultWeak                              "ResponsePageToResultWeak"
#define CLASS_ResponsePageToRelationshipInfo                        "ResponsePageToRelationshipInfo"
#define CLASS_ChangeInfoToInstanceBackup                            "ChangeInfoToInstanceBackup"

#define ECSql_RootClass                                             "[DSC].[Root]"
#define ECSql_CachedObjectInfoClass                                 "[DSC].[CachedObjectInfo]"
#define ECSql_CachedObjectInfoToInstanceClass                       "[DSC].[CachedObjectInfoRelationship]"
#define ECSql_CachedResponseInfoClass                               "[DSC].[CachedResponseInfo]"
#define ECSql_CachedResponsePageInfoClass                           "[DSC].[CachedResponsePageInfo]"
#define ECSql_CachedRelationshipInfoClass                           "[DSC].[CachedRelationshipInfo]"
#define ECSql_CachedFileInfoClass                                   "[DSC].[CachedFileInfo]"
#define ECSql_CachedFileInfoToInstanceClass                         "[DSC].[CachedFileInfoRelationship]"
#define ECSql_ChangeInfoClass                                       "[DSC].[ChangeInfo]"
#define ECSql_NavigationBaseClass                                   "[DSC].[NavigationBase]"
#define ECSql_InstanceBackup                                        "[DSC].[InstanceBackup]"
#define ECSql_ChangeInfoToInstanceBackup                            "[DSC].[ChangeInfoToInstanceBackup]"
#define ECSql_ResponseToParentClass                                 "[DSC].[CachedResponseInfoToParentRelationship]"
#define ECSql_ResponseToResponsePageClass                           "[DSC].[ResponseToResponsePage]"
#define ECSql_ResponsePageToResultClass                             "[DSC].[ResponsePageToResult]"
#define ECSql_ResponsePageToResultWeakClass                         "[DSC].[ResponsePageToResultWeak]"

#define SCHEMA_CacheLegacySupportSchema                             "DSCacheLegacySupportSchema"
#define SCHEMA_CacheLegacySupportSchema_LegacyParentRelationship    "LegacyParentRelationship"

END_BENTLEY_WEBSERVICES_NAMESPACE
