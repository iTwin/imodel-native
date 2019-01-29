/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsCurl/WSGServices.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <curl/curl.h>
#include <iostream>

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include "../RealityPlatformTools/WSGServices.cpp"
#include <RealityPlatform/RealityPlatformAPI.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//* writes the body of a curl reponse to a Utf8String
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   03/2017
//* writes the body of a curl reponse to a file
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteFileCallback(void *contents, size_t size, size_t nmemb, BeFile *stream)
    {
    uint32_t bytesWritten = 0;

    if (stream->Write(&bytesWritten, contents, (uint32_t)(size*nmemb)) != BeFileStatus::Success)
        bytesWritten = 0;

    return bytesWritten;
    }

RequestStatus RawServerResponse::ValidateResponse()
    {
    if ((toolCode != CURLE_OK) || (responseCode > 399))
        status = RequestStatus::BADREQ;
    else
        status = RequestStatus::OK;

    return status;
    }

RequestStatus RawServerResponse::ValidateJSONResponse(Json::Value& instances, Utf8StringCR keyword)
    {
    if ((toolCode != CURLE_OK) || (responseCode > 399) || !Json::Reader::Parse(body, instances) || instances.isMember("errorMessage") || !instances.isMember(keyword))
        status = RequestStatus::BADREQ;
    else
        status = RequestStatus::OK;

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void* RequestConstructor::PrepareRequestBase(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file) const
    {
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        response.toolCode = CURLcode::CURLE_FAILED_INIT;
        return curl;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;
    if (wsgRequest.GetRequestType() == WSGURL::HttpRequestType::POST_Request)
        {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wsgRequest.GetRequestPayload().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, wsgRequest.GetRequestPayload().length());
        }
    else if (wsgRequest.GetRequestType() == WSGURL::HttpRequestType::DELETE_Request)
        {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, wsgRequest.GetRequestPayload().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, wsgRequest.GetRequestPayload().length());
        }

    Utf8String proxyUrl, proxyCreds;
    GetCurrentProxyUrlAndCredentials(proxyUrl, proxyCreds);

    if (!proxyUrl.empty())
        {
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        if (!proxyCreds.empty())
            {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyCreds.c_str());
            }
        }

    bvector<Utf8String> wsgHeaders = wsgRequest.GetRequestHeader();
    for (Utf8String header : wsgHeaders)
        headers = curl_slist_append(headers, header.c_str());
    if(response.toolCode != ServerType::Azure)
        headers = curl_slist_append(headers, GetToken().c_str());

    curl_easy_setopt(curl, CURLOPT_URL, wsgRequest.GetHttpRequestString().c_str());

    //curl_easy_setopt(curl, CURLOPT_CERTINFO, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, (verifyPeer ? 1L : 0));
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, (verifyPeer ? 2L : 0));

    auto info = curl_version_info(CURLVERSION_NOW);
    if (verifyPeer && nullptr != info && nullptr != info->ssl_version && nullptr == strstr(info->ssl_version, "WinSSL"))
        {
        if(!m_certificatePath.empty() || m_certificatePath.DoesPathExist())
            curl_easy_setopt(curl, CURLOPT_CAINFO, m_certificatePath.GetNameUtf8().c_str());
        }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    return curl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void* WSGRequest::PrepareRequest(const WSGURL& wsgRequest, RawServerResponse& responseObject, bool verifyPeer, BeFile* file) const
    {
    CURL* curl = static_cast<CURL*>(PrepareRequestBase(wsgRequest, responseObject, verifyPeer, file));

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(responseObject.header));

    if (file != nullptr && wsgRequest.GetRequestType() != WSGURL::HttpRequestType::PUT_Request)
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        }
    else
        {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(responseObject.body));
        }

    return curl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::_PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    CURL* curl = static_cast<CURL*>(PrepareRequest(wsgRequest, response, verifyPeer, file));

    if(response.toolCode == CURLcode::CURLE_FAILED_INIT)
        return;

    response.toolCode = (int)curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(response.responseCode));

    /*struct curl_certinfo *ci;
    curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ci);
    printf(wsgRequest.GetHttpRequestString().c_str());
    printf("%d certs!\n", ci->num_of_certs);

    for (size_t i = 0; i < ci->num_of_certs; i++) {
        struct curl_slist *slist;

        for (slist = ci->certinfo[i]; slist; slist = slist->next)
            printf("%s\n", slist->data);
        }*/

    curl_easy_cleanup(curl);

    if (response.body.Contains("Token is not valid") && retry)
        {
        WSGRequest::GetInstance().RefreshToken();
        response = RawServerResponse();
        return _PerformRequest(wsgRequest, response, verifyPeer, file, false);
        }

    if (response.toolCode != CURLE_OK)
        response.status = BADREQ; // Default error ... may be modified by caller based on toolCode
    else
        response.status = OK;

    }