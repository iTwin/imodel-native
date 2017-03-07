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

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataProjectRelationship::RealityDataProjectRelationship(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ProjectId") && !jsonInstance["properties"]["ProjectId"].isNull())
            m_projectId = jsonInstance["properties"]["ProjectId"].asCString();
        }
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataProjectRelationship::GetRealityDataId() const { return m_realityDataId; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataProjectRelationship::GetProjectId() const { return m_projectId; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataDocument::RealityDataDocument(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("ContainerName") && !jsonInstance["properties"]["ContainerName"].isNull())
            m_containerName = jsonInstance["properties"]["ContainerName"].asCString();
        if (jsonInstance["properties"].isMember("Name") && !jsonInstance["properties"]["Name"].isNull())
            m_name = jsonInstance["properties"]["Name"].asCString(); 
        if (jsonInstance["properties"].isMember("Id") && !jsonInstance["properties"]["Id"].isNull())
            m_id = jsonInstance["properties"]["Id"].asCString(); 
        if (jsonInstance["properties"].isMember("FolderId") && !jsonInstance["properties"]["FolderId"].isNull())
            m_folderId = jsonInstance["properties"]["FolderId"].asCString(); 
        if (jsonInstance["properties"].isMember("AccessUrl") && !jsonInstance["properties"]["AccessUrl"].isNull())
            m_accessUrl = jsonInstance["properties"]["AccessUrl"].asCString(); 
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString(); 
        if (jsonInstance["properties"].isMember("ContentType") && !jsonInstance["properties"]["ContentType"].isNull())
            m_contentType = jsonInstance["properties"]["ContentType"].asCString(); 
        if (jsonInstance["properties"].isMember("Size") && !jsonInstance["properties"]["Size"].isNull())
            m_size = jsonInstance["properties"]["Size"].asCString(); 
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataDocument::GetContainerName() const { return m_containerName; }

Utf8StringCR RealityDataDocument::GetName() const { return m_name; }

Utf8StringCR RealityDataDocument::GetId() const { return m_id; }

Utf8StringCR RealityDataDocument::GetFolderId() const { return m_folderId; }

Utf8StringCR RealityDataDocument::GetAccessUrl() const { return m_accessUrl; }

Utf8StringCR RealityDataDocument::GetRealityDataId() const { return m_realityDataId; }

Utf8StringCR RealityDataDocument::GetContentType() const { return m_contentType; }

Utf8StringCR RealityDataDocument::GetSize() const { return m_size; }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
RealityDataFolder::RealityDataFolder(Json::Value jsonInstance)
    {
    if (jsonInstance.isMember("properties"))
        {
        if (jsonInstance["properties"].isMember("Name") && !jsonInstance["properties"]["Name"].isNull())
            m_name = jsonInstance["properties"]["Name"].asCString();
        if (jsonInstance["properties"].isMember("RealityDataId") && !jsonInstance["properties"]["RealityDataId"].isNull())
            m_realityDataId = jsonInstance["properties"]["RealityDataId"].asCString();
        if (jsonInstance["properties"].isMember("ParentFolderId") && !jsonInstance["properties"]["ParentFolderId"].isNull())
            m_parentId = jsonInstance["properties"]["ParentFolderId"].asCString();
        }
    }
//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            02/2017
//-------------------------------------------------------------------------------------
Utf8StringCR RealityDataFolder::GetName() const { return m_name; }

Utf8StringCR RealityDataFolder::GetParentId() const { return m_parentId; }

Utf8StringCR RealityDataFolder::GetRealityDataId() const { return m_realityDataId; }