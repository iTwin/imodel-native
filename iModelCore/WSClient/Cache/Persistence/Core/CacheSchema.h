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
// Schema definitions for WSCacheSchema ECSchema
//--------------------------------------------------------------------------------------+

#define SCHEMA_CacheSchema_Major                                    2
#define SCHEMA_CacheSchema_Minor                                    0

#define SCHEMA_CacheSchema                                          "WSCache"

#define SCHEMA_CacheLegacySupport_PREFIX                            "WSCacheLegacySupport"
#define CLASS_CacheLegacySupport_ParentToChildRelationship          "ParentToChildRelationship"

#define CLASS_Root                                                  "Root"
#define CLASS_Root_PROPERTY_Name                                    "Name"
#define CLASS_Root_PROPERTY_Persistence                             "Persistence"
#define CLASS_Root_PROPERTY_SyncDate                                "SyncDate"

#define CLASS_NavigationBase                                        "NavigationBase"

#define CLASS_Sequence                                              "Sequence"
#define CLASS_Sequence_PROPERTY_LastChangeNumber                    "LastChangeNumber"
#define CLASS_Sequence_PROPERTY_LastCachedFileIndex                 "LastCachedFileIndex"

#define CLASS_CachedResponseInfo                                    "CachedResponseInfo"
#define CLASS_CachedResponseInfo_PROPERTY_Name                      "Name"
#define CLASS_CachedResponseInfo_PROPERTY_AccessDate                "AccessDate"
#define CLASS_CachedResponseInfo_PROPERTY_IsCompleted               "IsCompleted"

#define CLASS_CachedResponsePageInfo                                "CachedResponsePageInfo"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheDate             "CacheDate"
#define CLASS_CachedResponsePageInfo_PROPERTY_CacheTag              "CacheTag"
#define CLASS_CachedResponsePageInfo_PROPERTY_Index                 "Index"

#define CLASS_CachedRelationshipInfo                                "CachedRelationshipInfo"
#define CLASS_CachedRelationshipInfo_PROPERTY_RemoteId              "RelRemoteId"
#define CLASS_CachedRelationshipInfo_PROPERTY_ClassId               "ClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_InstanceId            "InstanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceClassId         "SourceClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_SourceInstanceId      "SourceInstanceId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetClassId         "TargetClassId"
#define CLASS_CachedRelationshipInfo_PROPERTY_TargetInstanceId      "TargetInstanceId"

#define CLASS_CachedObjectInfo                                      "CachedObjectInfo"
#define CLASS_CachedObjectInfo_PROPERTY_RemoteId                    "RemoteId"
#define CLASS_CachedObjectInfo_PROPERTY_ClassId                     "ClassId"
#define CLASS_CachedObjectInfo_PROPERTY_InstanceId                  "InstanceId"
#define CLASS_CachedObjectInfo_PROPERTY_InstanceState               "InstanceState"
#define CLASS_CachedObjectInfo_PROPERTY_CacheDate                   "CacheDate"
#define CLASS_CachedObjectInfo_PROPERTY_CacheTag                    "CacheTag"

#define CLASS_CachedFileInfo                                        "CachedFileInfo"
#define CLASS_CachedFileInfo_PROPERTY_CacheDate                     "CacheDate"
#define CLASS_CachedFileInfo_PROPERTY_CacheTag                      "CacheTag"
#define CLASS_CachedFileInfo_PROPERTY_UpdateDate                    "UpdateDate"

#define CLASS_ChangeInfo                                            "ChangeInfo"
#define CLASS_ChangeInfo_PROPERTY_ChangeStatus                      "ChangeStatus"
#define CLASS_ChangeInfo_PROPERTY_SyncStatus                        "SyncStatus"
#define CLASS_ChangeInfo_PROPERTY_ChangeNumber                      "ChangeNumber"
#define CLASS_ChangeInfo_PROPERTY_Revision                          "Revision"
#define CLASS_ChangeInfo_PROPERTY_IsLocal                           "IsLocal"

#define CLASS_InstanceBackup                                        "InstanceBackup"
#define CLASS_InstanceBackup_PROPERTY_InfoId                        "InfoId"
#define CLASS_InstanceBackup_PROPERTY_Instance                      "Instance"

#define CLASS_RootToNode                                            "RootToNode"
#define CLASS_RootToNodeWeak                                        "RootToNodeWeak"
#define CLASS_ResponseToParent                                      "ResponseToParent"
#define CLASS_ResponseToHolder                                      "ResponseToHolder"
#define CLASS_ResponseToResponsePage                                "ResponseToResponsePage"
#define CLASS_ResponseToAdditionalInstance                          "ResponseToAdditionalInstance"
#define CLASS_ResponsePageToResult                                  "ResponsePageToResult"
#define CLASS_ResponsePageToResultWeak                              "ResponsePageToResultWeak"
#define CLASS_ChangeInfoToInstanceBackup                            "ChangeInfoToInstanceBackup"
#define CLASS_ObjectInfoToCachedFileInfo                            "ObjectInfoToCachedFileInfo"
#define CLASS_CachedFileInfoToFileInfo                              "CachedFileInfoToFileInfo"
#define CLASS_CachedFileInfoToFileInfoOwnership                     "CachedFileInfoToFileInfoOwnership"

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
