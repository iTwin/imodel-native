/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/CacheSchema.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

//--------------------------------------------------------------------------------------+
// Schema definitions for WSCacheSchema ECSchema
//--------------------------------------------------------------------------------------+

#define SCHEMA_CacheSchema_Major                                    2
#define SCHEMA_CacheSchema_Minor                                    2

#define SCHEMA_CacheSchema                                          "WSCache"

#define SCHEMA_CacheLegacySupport_PREFIX                            "WSCacheLegacySupport"
#define CLASS_CacheLegacySupport_ParentToChildRelationship          "ParentToChildRelationship"

#define CLASS_Root                                                  "Root"
#define CLASS_Root_PROPERTY_Name                                    "name"
#define CLASS_Root_PROPERTY_Persistence                             "persistence"
#define CLASS_Root_PROPERTY_SyncDate                                "syncDate"

#define CLASS_NavigationBase                                        "NavigationBase"

#define CLASS_Sequence                                              "Sequence"
#define CLASS_Sequence_PROPERTY_LastChangeNumber                    "lastChangeNumber"
#define CLASS_Sequence_PROPERTY_LastCachedFileIndex                 "lastCachedFileIndex"

#define CLASS_CachedResponseInfo                                    "CachedResponseInfo"
#define CLASS_CachedResponseInfo_PROPERTY_Name                      "name"
#define CLASS_CachedResponseInfo_PROPERTY_AccessDate                "accessDate"
#define CLASS_CachedResponseInfo_PROPERTY_IsCompleted               "isCompleted"
#define CLASS_CachedResponseInfo_PROPERTY_Parent                    "parent"
#define CLASS_CachedResponseInfo_PROPERTY_Holder                    "holder"

#define CLASS_CachedResponsePageInfo                                "CachedResponsePageInfo"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheDate             "cacheDate"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheTag              "cacheTag"
#define CLASS_CachedResponsePageInfo_PROPERTY_Index                 "index"
#define CLASS_CachedResponsePageInfo_PROPERTY_IsPartial             "isPartial"
#define CLASS_CachedResponsePageInfo_PROPERTY_Response              "response"

#define CLASS_CachedRelationshipInfo                                "CachedRelationshipInfo"
#define CLASS_CachedRelationshipInfo_PROPERTY_RemoteId              "relRemoteId"
#define CLASS_CachedRelationshipInfo_PROPERTY_ClassId               "classId"
#define CLASS_CachedRelationshipInfo_PROPERTY_InstanceId            "instanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId         "sourceClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId      "sourceInstanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId         "targetClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId      "targetInstanceId"

#define CLASS_CachedObjectInfo                                      "CachedObjectInfo"
#define CLASS_CachedObjectInfo_PROPERTY_RemoteId                    "remoteId"
#define CLASS_CachedObjectInfo_PROPERTY_ClassId                     "classId"
#define CLASS_CachedObjectInfo_PROPERTY_InstanceId                  "instanceId"
#define CLASS_CachedObjectInfo_PROPERTY_InstanceState               "instanceState"
#define CLASS_CachedObjectInfo_PROPERTY_CacheDate                   "cacheDate"
#define CLASS_CachedObjectInfo_PROPERTY_CacheTag                    "cacheTag"

#define CLASS_CachedFileInfo                                        "CachedFileInfo"
#define CLASS_CachedFileInfo_PROPERTY_CacheDate                     "cacheDate"
#define CLASS_CachedFileInfo_PROPERTY_CacheTag                      "cacheTag"
#define CLASS_CachedFileInfo_PROPERTY_UpdateDate                    "updateDate"
#define CLASS_CachedFileInfo_PROPERTY_ObjectInfo                    "objectInfo"

#define CLASS_ChangeInfo                                            "ChangeInfo"
#define CLASS_ChangeInfo_PROPERTY_ChangeStatus                      "changeStatus"
#define CLASS_ChangeInfo_PROPERTY_SyncStatus                        "syncStatus"
#define CLASS_ChangeInfo_PROPERTY_ChangeNumber                      "changeNumber"
#define CLASS_ChangeInfo_PROPERTY_Revision                          "revision"
#define CLASS_ChangeInfo_PROPERTY_IsLocal                           "isLocal"

#define CLASS_InstanceBackup                                        "InstanceBackup"
#define CLASS_InstanceBackup_PROPERTY_ObjectInfo                    "objectInfo"
#define CLASS_InstanceBackup_PROPERTY_Instance                      "instance"

#define CLASS_RootToNode                                            "RootToNode"
#define CLASS_RootToNodeWeak                                        "rootToNodeWeak"
#define CLASS_ResponseToParent                                      "responseToParent"
#define CLASS_ResponseToHolder                                      "responseToHolder"
#define CLASS_ResponseToResponsePage                                "responseToResponsePage"
#define CLASS_ResponseToAdditionalInstance                          "responseToAdditionalInstance"
#define CLASS_ResponsePageToResult                                  "responsePageToResult"
#define CLASS_ResponsePageToResultWeak                              "responsePageToResultWeak"
#define CLASS_ChangeInfoToInstanceBackup                            "changeInfoToInstanceBackup"
#define CLASS_ObjectInfoToCachedFileInfo                            "objectInfoToCachedFileInfo"
#define CLASS_CachedFileInfoToFileInfo                              "cachedFileInfoToFileInfo"
#define CLASS_CachedFileInfoToFileInfoOwnership                     "cachedFileInfoToFileInfoOwnership"

#define CLASS_ExtendedData                                          "ExtendedData"
#define CLASS_NodeToExtendedData                                    "NodeToExtendedData"

#define ECSql_Root                                                  "[WSC].[" CLASS_Root "]"
#define ECSql_NavigationBase                                        "[WSC].[" CLASS_NavigationBase "]"
#define ECSql_CachedResponseInfo                                    "[WSC].[" CLASS_CachedResponseInfo "]"
#define ECSql_CachedResponsePageInfo                                "[WSC].[" CLASS_CachedResponsePageInfo "]"
#define ECSql_CachedRelationshipInfo                                "[WSC].[" CLASS_CachedRelationshipInfo "]"
#define ECSql_CachedObjectInfo                                      "[WSC].[" CLASS_CachedObjectInfo "]"
#define ECSql_CachedFileInfo                                        "[WSC].[" CLASS_CachedFileInfo "]"
#define ECSql_ChangeInfo                                            "[WSC].[" CLASS_ChangeInfo "]"
#define ECSql_InstanceBackup                                        "[WSC].[" CLASS_InstanceBackup "]"
#define ECSql_RootToNode                                            "[WSC].[" CLASS_RootToNode "]"
#define ECSql_RootToNodeWeak                                        "[WSC].[" CLASS_RootToNodeWeak "]"
#define ECSql_ResponseToParent                                      "[WSC].[" CLASS_ResponseToParent "]"
#define ECSql_ResponseToHolder                                      "[WSC].[" CLASS_ResponseToHolder "]"
#define ECSql_ResponseToResponsePage                                "[WSC].[" CLASS_ResponseToResponsePage "]"
#define ECSql_ResponsePageToResult                                  "[WSC].[" CLASS_ResponsePageToResult "]"
#define ECSql_ResponsePageToResultWeak                              "[WSC].[" CLASS_ResponsePageToResultWeak "]"
#define ECSql_ChangeInfoToInstanceBackup                            "[WSC].[" CLASS_ChangeInfoToInstanceBackup "]"
#define ECSql_ObjectInfoToCachedFileInfo                            "[WSC].[" CLASS_ObjectInfoToCachedFileInfo "]"
#define ECSql_CachedFileInfoToFileInfo                              "[WSC].[" CLASS_CachedFileInfoToFileInfo "]"
#define ECSql_ObjectInfoToCachedFileInfoOwnership                   "[WSC].[" CLASS_CachedFileInfoToFileInfoOwnership "]"

//! Values for property CLASS_CachedObjectInfo_PROPERTY_InstanceState
enum class CachedInstanceState
    {
    Placeholder = 0,
    Partial = 1,
    Full = 2
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
