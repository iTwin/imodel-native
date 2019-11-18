/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelBridgeSettings.h"
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

#include <Logging/bentleylogging.h>
#include <WebServices/iModelHub/Client/OidcTokenProvider.h>

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_IMODELHUB

#define LOG (*LoggingManager::GetLogger(L"iModelBridgeFwk"))

#define SETTINGS_JSON_Properties "properties"
#define SETTINGS_JSON_Documents "documents"
#define SETTINGS_JSON_Id "id"
#define SETTINGS_JSON_BriefcaseId "briefcaseId"
#define SETTINGS_JSON_Version "version"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSettings::iModelBridgeSettings(BentleyApi::WebServices::IConnectSignInManagerPtr mgr, Utf8StringCR requestGuid, Utf8StringCR iModelGuid, Utf8StringCR projectGuid)
    :m_signInMgr(mgr),m_requestGuid(requestGuid),m_iModelGuid(iModelGuid), m_connectProjectGuid(projectGuid)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      iModelBridgeSettings::GetDocumentMappingUrl()
    {
    Utf8String productSettingsUrl = UrlProvider::Urls::ConnectProductSettingsService.Get();
    if (productSettingsUrl.empty())
        {
        LOG.errorv("Unable to get the product settings url.");
        return productSettingsUrl;
        }
    //static int iModelFwk_GRPID = 2661;
    Utf8PrintfString url("%s/v1/Application/2661/Context/%s/iModel/%s/Setting/DocumentMapping/Documents", productSettingsUrl.c_str(), m_connectProjectGuid.c_str(), m_iModelGuid.c_str());
    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  iModelBridgeSettings::GetDocumentMappingJson(rapidjson::Document& document)
    {
    Utf8String productSettingsUrl = GetDocumentMappingUrl();
    if (productSettingsUrl.empty())
        return ERROR;

    Http::HttpClient client(nullptr, GetAuthHandler());
    Http::Request request = client.CreateGetJsonRequest(productSettingsUrl);
    request.GetHeaders().AddValue("X-Correlation-Id", m_requestGuid.c_str());
    auto response = request.Perform().get();
    if (!response.IsSuccess())
        return ERROR;

    document.Parse<0>(response.GetBody().AsString().c_str());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus GetDocumentEntryByGuid(rapidjson::Value& value, rapidjson::Document& document, Utf8StringCR documentGuid)
    {
    auto& propertiesVal = document[SETTINGS_JSON_Properties];

    auto& documents = propertiesVal[SETTINGS_JSON_Documents];
    if (!documents.IsArray())
        return ERROR;

    for (auto& doc : documents.GetArray())
        {
        auto& idJson = doc[SETTINGS_JSON_Id];
        if (!idJson.IsString())
            continue;

        if (0 != documentGuid.CompareToI(idJson.GetString()))
            continue;

        value = doc;
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeSettings::GetBriefCaseId(Utf8StringCR documentGuid, BeSQLite::BeBriefcaseId& briefcaseId)
    {
    if (documentGuid.empty())
        return ERROR;

    if (!IsValid())
        return ERROR;

    rapidjson::Document document;
    if (SUCCESS != GetDocumentMappingJson(document) || document.HasParseError())
        return ERROR;

    rapidjson::Value docEntry;
    if (SUCCESS != GetDocumentEntryByGuid(docEntry, document, documentGuid))
        return ERROR;

    auto& briefcaseIdJson = docEntry[SETTINGS_JSON_BriefcaseId];
    if (!briefcaseIdJson.IsInt64())
        return ERROR;

    briefcaseId = BeSQLite::BeBriefcaseId(briefcaseIdJson.GetInt64());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static  BentleyStatus FillEmptyDocumentMap (rapidjson::Document& document)
    {
    document.SetObject();
    auto& allocator = document.GetAllocator();
    rapidjson::Value propValue(rapidjson::kObjectType);
    propValue.AddMember(SETTINGS_JSON_Version, rapidjson::Value(1), allocator);
    rapidjson::Value documentsValue(rapidjson::kArrayType);
    propValue.AddMember(SETTINGS_JSON_Documents, documentsValue, allocator);
    document.AddMember(SETTINGS_JSON_Properties, propValue, allocator);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     IsValidJsonDocument(rapidjson::Document& document)
    {
    auto& propVal = document[SETTINGS_JSON_Properties];
    if (!propVal.IsObject())
        return false;
    
    auto& documents = propVal[SETTINGS_JSON_Documents];
    if (!documents.IsArray())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelBridgeSettings::SetBriefCaseId(Utf8StringCR documentGuid, BeSQLite::BeBriefcaseId const& briefcaseId)
    {
    if (documentGuid.empty())
        return ERROR;

    if (!IsValid())
        return ERROR;

    Utf8String productSettingsUrl = GetDocumentMappingUrl();
    if (productSettingsUrl.empty())
        return ERROR;

    rapidjson::Document document;
    if (SUCCESS != GetDocumentMappingJson(document) || document.HasParseError())
        {
        FillEmptyDocumentMap(document);
        }
    else if (!IsValidJsonDocument(document))
        {
        document.Clear();
        FillEmptyDocumentMap(document);
        }
    
    auto& allocator = document.GetAllocator();

    rapidjson::Value docEntry;
    if (SUCCESS != GetDocumentEntryByGuid(docEntry, document, documentGuid))
        {
        docEntry.SetObject();
        docEntry.AddMember(SETTINGS_JSON_Id, rapidjson::Value(documentGuid.c_str(), allocator), allocator);
        docEntry.AddMember(SETTINGS_JSON_BriefcaseId, briefcaseId.GetValue(), allocator);
        auto arrayVal = document[SETTINGS_JSON_Properties][SETTINGS_JSON_Documents].GetArray();
        arrayVal.PushBack(docEntry.Move(), document.GetAllocator());
        }
    else
        {
        auto arrayVal = document[SETTINGS_JSON_Properties][SETTINGS_JSON_Documents].GetArray();
        for (auto& arrayEntry : arrayVal)
            {
            auto& idJson = arrayEntry[SETTINGS_JSON_Id];
            if (!idJson.IsString())
                continue;

            if (0 != documentGuid.CompareToI(idJson.GetString()))
                continue;

            if (arrayEntry.HasMember(SETTINGS_JSON_BriefcaseId))
                return ERROR;//Cannot unset briefcase Id for an existing entry.

            arrayEntry[SETTINGS_JSON_BriefcaseId] = briefcaseId.GetValue();
            }
        }

    
    auto authHandler = GetAuthHandler();

    Http::HttpClient client(nullptr, authHandler);
    
    Http::Request request = client.CreateRequest(productSettingsUrl,"PUT");
    request.GetHeaders().AddValue("X-Correlation-Id", m_requestGuid.c_str());
    request.GetHeaders().SetAccept(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetContentType("application/json; charset=utf-8");
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    request.SetRequestBody(HttpStringBody::Create(buffer.GetString()));
    auto response = request.Perform().get();
    if (!response.IsSuccess())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool            iModelBridgeSettings::IsValid() const
    {
    if (m_iModelGuid.empty())
        return false;

    return !m_connectProjectGuid.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<BentleyApi::WebServices::ConnectAuthenticationHandler> iModelBridgeSettings::GetAuthHandler()
    {
    if (nullptr != m_authHandler)
        return m_authHandler;

    auto configurationHandler = UrlProvider::GetSecurityConfigurator(nullptr);

    Utf8String baseUrl = UrlProvider::Urls::ConnectProductSettingsService.Get();

    auto tokenProvider = m_signInMgr->GetTokenProvider(baseUrl);
    bool isSaml = true;
    if (NULL != dynamic_cast<OidcTokenProvider*>(tokenProvider.get()))
        isSaml = false;

    m_authHandler = ConnectAuthenticationHandler::Create
                        (
                            baseUrl,
                            tokenProvider,
                            configurationHandler,
                            isSaml ? TOKENPREFIX_Token : TOKENPREFIX_BEARER
                        );
    m_authHandler->EnableExpiredTokenRefresh();
    return m_authHandler;
    }