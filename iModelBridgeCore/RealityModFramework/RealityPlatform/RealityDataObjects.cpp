/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataObjects.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/RealityDataObjects.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

RealityDataProjectRelationship::RealityDataProjectRelationship(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("RealityDataId"))
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ProjectId"))
            m_projectId = jsonInstance["properties"]["ProjectId"].asCString();
        }
    }

Utf8StringCR RealityDataProjectRelationship::GetRealityDataId() const { return m_realityDataId; }

Utf8StringCR RealityDataProjectRelationship::GetProjectId() const { return m_projectId; }


RealityDataDocument::RealityDataDocument(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("ContainerName"))
            m_containerName = jsonInstance["properties"]["ContainerName"].asCString();
        if (jsonInstance["properties"].isMember("Name"))
            m_name = jsonInstance["properties"]["Name"].asCString(); 
        if (jsonInstance["properties"].isMember("Id"))
            m_id = jsonInstance["properties"]["Id"].asCString(); 
        if (jsonInstance["properties"].isMember("FolderId"))
            m_folderId = jsonInstance["properties"]["FolderId"].asCString(); 
        if (jsonInstance["properties"].isMember("AccessUrl"))
            m_accessUrl = jsonInstance["properties"]["AccessUrl"].asCString(); 
        if (jsonInstance["properties"].isMember("RealityDataId"))
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString(); 
        if (jsonInstance["properties"].isMember("ContentType"))
            m_contentType = jsonInstance["properties"]["ContentType"].asCString(); 
        if (jsonInstance["properties"].isMember("Size"))
            m_size = jsonInstance["properties"]["Size"].asCString(); 
        }
    }

Utf8StringCR RealityDataDocument::GetContainerName() const { return m_containerName; }

Utf8StringCR RealityDataDocument::GetName() const { return m_name; }

Utf8StringCR RealityDataDocument::GetId() const { return m_id; }

Utf8StringCR RealityDataDocument::GetFolderId() const { return m_folderId; }

Utf8StringCR RealityDataDocument::GetAccessUrl() const { return m_accessUrl; }

Utf8StringCR RealityDataDocument::GetRealityDataId() const { return m_realityDataId; }

Utf8StringCR RealityDataDocument::GetContentType() const { return m_contentType; }

Utf8StringCR RealityDataDocument::GetSize() const { return m_size; }

RealityDataFolder::RealityDataFolder(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("Name"))
            m_name = jsonInstance["properties"]["Name"].asCString();
        if (jsonInstance["properties"].isMember("RealityDataId"))
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ParentFolderId"))
            m_parentId = jsonInstance["properties"]["ParentFolderId"].asCString();
        }
    }

Utf8StringCR RealityDataFolder::GetName() const { return m_name; }

Utf8StringCR RealityDataFolder::GetParentId() const { return m_parentId; }

Utf8StringCR RealityDataFolder::GetRealityDataId() const { return m_realityDataId; }