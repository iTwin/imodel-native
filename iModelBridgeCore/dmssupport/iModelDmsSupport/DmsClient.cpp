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
bmap<WString, WString> DmsClient::_GetDownloadURLs(Utf8String token, Utf8String datasource)
    {
    Http::HttpClient client;
    bmap<WString, WString> downloadUrls;
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
        rapidjson::Value& results = document["data"]["item"];
        bool isfolder = results["isFolder"].GetBool();
        if (!isfolder)
            {
            // Store the value of the element in a vector
            Utf8String name = results["name"].GetString();
            Utf8String url = results["downloadUrl"].GetString();
            WString fileName(name.c_str(), 0);
            WString downloadUrl(url.c_str(), 0);
            downloadUrls.Insert(fileName, downloadUrl);
            }
        }
    else
        {
        rapidjson::Value& results = document["data"]["item"]["children"];
        for (rapidjson::SizeType i = 0; i < results.Size(); i++)
            {
            bool isfolder = results[i]["isFolder"].GetBool();
            if (!isfolder)
                {
                // Store the value of the element in a vector
                Utf8String name = results[i]["name"].GetString();
                Utf8String url = results[i]["downloadUrl"].GetString();
                url.ReplaceAll(" ", "%20");
                WString fileName(name.c_str(), 0);
                WString downloadUrl(url.c_str(), 0);
                downloadUrls.Insert(fileName, downloadUrl);
                }
            }
        }
    return downloadUrls;
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
Utf8PrintfString DmsClient::_CreateQuery(Utf8String datasource)
    {
    // PWDI query
    if (m_repositoryType.EqualsI(PWREPOSITORYTYPE))
        return Utf8PrintfString("query{item(input: {type: %s, location: \"%s\", id: \"%s\"}){id, name, downloadUrl, isFolder}}", PWREPOSITORYTYPE, datasource.c_str(), m_fileId.c_str());
    // Projectshare query
    return Utf8PrintfString("query{item(input: {type: %s, location : \"%s\", id : \"%s\"}){children{id, name, downloadUrl, isFolder}}}", PSREPOSITORYTYPE, m_projectId.c_str(), m_folderId.c_str());
    }