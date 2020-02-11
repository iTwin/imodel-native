/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DmsClient.h"
#include <rapidjson/document.h>
#include <WebServices/Configuration/UrlProvider.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool   DmsClient::_InitializeSession(Utf8String fileUrl, Utf8String repositoryType)
    {
    m_repositoryType = repositoryType;
    if(m_repositoryType.EqualsI(PWREPOSITORYTYPE))
        return _ParseInputUrlForPW(fileUrl);
    return _ParseInputUrlForPS(fileUrl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool  DmsClient::_UnInitializeSession()
    {
    m_projectId.clear();
    m_folderId.clear();
    m_fileId.clear();
    return true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Vishal.Shingare                11/2019
//-------------------------------------------------------------------------------------
bvector<DmsResponseData> DmsClient::_GetDownloadURLs(Utf8String token, Utf8String datasource)
    {
    Http::HttpClient client;
    bvector<DmsResponseData> downloadUrls;
    Utf8String graphqlRequestUrl = UrlProvider::Urls::ProjectWiseDocumentService.Get();
    if (graphqlRequestUrl.empty())
        {
        LOG.errorv("Error getting ProjectWiseDocumentService url");
        return downloadUrls;
        }
    graphqlRequestUrl.append("graphql");

    Utf8PrintfString body = _CreateQuery(datasource);
    Http::Request request = client.CreatePostRequest(graphqlRequestUrl);
    request.SetRequestBody(Http::HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("application/graphql; charset=utf-8");
    request.GetHeaders().SetAuthorization(token);

    auto response = request.Perform().get();
    if (!response.IsSuccess())
        {
        LOG.errorv("Error getting requested file urls");
        return downloadUrls;
        }

    rapidjson::Document document;
    auto content = response.GetContent()->GetBody()->AsString();
    document.Parse(content.c_str());
    if (document["data"].IsNull())
        {
        LOG.errorv("Error getting requested file urls");
        return downloadUrls;
        }
    if (m_repositoryType.EqualsI(PWREPOSITORYTYPE))
        {
        DmsResponseData dmsResponse;
        rapidjson::Value& results = document["data"]["item"];
        bool isfolder = results["isFolder"].GetBool();
        if (!isfolder)
            {
            // Store the value of the element in a vector
            Utf8String name = results["name"].GetString();
            Utf8String url = results["downloadUrl"].GetString();
            Utf8String parentId = results["parentId"].GetString();
            Utf8String fileId = results["id"].GetString();
            dmsResponse.fileName = WString(name.c_str(), 0);
            dmsResponse.downloadURL = WString(url.c_str(), 0);
            dmsResponse.parentFolderId = WString(parentId.c_str(), 0);
            dmsResponse.fileId = WString(fileId.c_str(), 0);
            downloadUrls.push_back(dmsResponse);
            }
        }
    else
        {
        rapidjson::Value& results = document["data"]["item"]["children"];
        for (rapidjson::SizeType i = 0; i < results.Size(); i++)
            {
            DmsResponseData dmsResponse;
            bool isfolder = results[i]["isFolder"].GetBool();
            if (!isfolder)
                {
                // Store the value of the element in a vector
                Utf8String name = results[i]["name"].GetString();
                Utf8String url = results[i]["downloadUrl"].GetString();
                Utf8String parentId = results[i]["parentId"].GetString();
                Utf8String fileId = results[i]["id"].GetString();
                url.ReplaceAll(" ", "%20");

                dmsResponse.fileName = WString(name.c_str(), 0);
                dmsResponse.downloadURL = WString(url.c_str(), 0);
                dmsResponse.parentFolderId = WString(parentId.c_str(), 0);
                dmsResponse.fileId = WString(fileId.c_str(), 0);
                downloadUrls.push_back(dmsResponse);
                }
            }
        }
    return downloadUrls;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Vishal.Shingare                01/2020
//-------------------------------------------------------------------------------------
bvector<DmsResponseData> DmsClient::_GetWorkspaceFiles(Utf8String token, Utf8String datasource, DmsResponseData& cfgData)
    {
    Http::HttpClient client;
    bvector<DmsResponseData> files;
    if (datasource.empty())
        {
        LOG.warningv("Error datasource url is NULL");
        return files;
        }
    Utf8String graphqlRequestUrl = UrlProvider::Urls::ProjectWiseDocumentService.Get();
    if (graphqlRequestUrl.empty())
        {
        LOG.errorv("Error getting ProjectWiseDocumentService url");
        return files;
        }
    graphqlRequestUrl.append("graphql");

    // get workspace related files
    Utf8PrintfString body = _CreateQuery(datasource, true);

    Http::Request request = client.CreatePostRequest(graphqlRequestUrl);
    request.SetRequestBody(Http::HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("application/graphql; charset=utf-8");
    request.GetHeaders().SetAuthorization(token);
    auto response = request.Perform().get();
    if (!response.IsSuccess())
        {
        LOG.errorv("Error getting workspace related files");
        return files;
        }

    rapidjson::Document document;
    auto content = response.GetContent()->GetBody()->AsString();
    document.Parse<0>(content.c_str());

    if (document["data"].IsNull() || document["data"]["pwWorkspaces"].IsNull())
        {
        LOG.warningv("Error getting data or workspace related instances");
        return files;
        }
    cfgData.fileId = WString(document["data"]["pwWorkspaces"][0]["id"].GetString(), 0);
    cfgData.parentFolderId = WString(L"Workspace");
    cfgData.downloadURL = WString(document["data"]["pwWorkspaces"][0]["content"].GetString(), 0);

    rapidjson::Value& results = document["data"]["pwWorkspaces"][0]["relatedDocuments"];

    for (rapidjson::SizeType i = 0; i < results.Size(); i++)
        {
        DmsResponseData wsgResponse;

        // Store the value of the element in a vector
        Utf8String fileId = results[i]["id"].GetString();
        Utf8String fileName = results[i]["name"].GetString();
        Utf8String folderName = results[i]["relativeDirectory"].GetString();
        Utf8String downloadURL = results[i]["downloadUrl"].GetString();
        wsgResponse.fileId = WString(fileId.c_str(), 0);
        wsgResponse.fileName = WString(fileName.c_str(), 0);
        wsgResponse.parentFolderId = WString(folderName.c_str(), 0);
        wsgResponse.downloadURL = WString(downloadURL.c_str(), 0);

        files.push_back(wsgResponse);

        }

    return files;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsClient::_ParseInputUrlForPS(Utf8String url)
    {
    bvector<Utf8String> guids;
    BeStringUtilities::Split(url.c_str(), "/", guids);
    if (guids.size() < 3)
        return false;
    m_projectId = guids[0];
    m_folderId = guids[1];
    m_fileId = guids[2];
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DmsClient::_ParseInputUrlForPW(Utf8String url)
    {
    bvector<Utf8String> guids;
    BeStringUtilities::Split(url.c_str(), "{", guids);
    if (guids.size() < 2)
        return false;
    m_fileId = guids[1].Trim("}");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8PrintfString DmsClient::_CreateQuery(Utf8String datasource, bool isWSQuery)
    {
    if (m_repositoryType.EqualsI(PWREPOSITORYTYPE))
        {
        // WS query
        if (isWSQuery)
            return Utf8PrintfString("query{pwWorkspaces(application: \"%s\", location: \"%s\", documentId: \"%s\"){id, content, relatedDocuments {id, name, relativeDirectory, downloadUrl}}}", APPLICATIONTYPE, datasource.c_str(), m_fileId.c_str());
        // PWDI query
        return Utf8PrintfString("query{item(input: {type: %s, location: \"%s\", id: \"%s\"}){id, parentId, name, downloadUrl, isFolder}}", PWREPOSITORYTYPE, datasource.c_str(), m_fileId.c_str());
        }
    // Projectshare query
    return Utf8PrintfString("query{item(input: {type: %s, location : \"%s\", id : \"%s\"}){children{id, parentId, name, downloadUrl, isFolder}}}", PSREPOSITORYTYPE, m_projectId.c_str(), m_folderId.c_str());
    }