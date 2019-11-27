/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PWShareDmsSupport.h"
#include <rapidjson/document.h>
#include <WebServices/Configuration/UrlProvider.h>

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger(L"iModelBridge"))

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool   PWShareDmsSupport::_InitializeSession(Utf8String fileUrl)
    {
    return _ParseInputURL(fileUrl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool  PWShareDmsSupport::_UnInitializeSession()
    {
    m_projectId.clear();
    m_folderId.clear();
    m_fileId.clear();
    return true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Vishal.Shingare                11/2019
//-------------------------------------------------------------------------------------
bmap<WString, WString> PWShareDmsSupport::_GetDownloadURLs(Utf8String token)
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

    Utf8PrintfString body("query{item(input: {type: %s, location : \"%s\", id : \"%s\"}){children{id, name, downloadUrl, isFolder}}}", REPOSITORYTYPE, m_projectId.c_str(), m_folderId.c_str());
    Http::Request request = client.CreatePostRequest(graphqlRequestUrl);
    request.SetRequestBody(Http::HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("application/graphql; charset=utf-8");
    request.GetHeaders().SetAuthorization(token);

    auto response = request.Perform().get();
    if (!response.IsSuccess())
        {
        LOG.errorv("Error getting requested sas urls");
        return downloadUrls;
        }

    rapidjson::Document document;
    auto content = response.GetContent()->GetBody()->AsString();
    document.Parse(content.c_str());

    if (document["data"].IsNull())
        {
        LOG.errorv("Error getting requested sas urls");
        return downloadUrls;
        }

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
    return downloadUrls;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool PWShareDmsSupport::_ParseInputURL(Utf8String url)
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