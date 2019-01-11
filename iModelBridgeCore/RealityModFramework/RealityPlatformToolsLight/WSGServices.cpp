/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatformToolsLight/WSGServices.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <iostream>

#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include "../RealityPlatformTools/WSGServices.cpp"
#include <RealityPlatform/RealityPlatformAPI.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

RequestStatus RawServerResponse::ValidateResponse()
    {
    if ((toolCode != 0) || (responseCode > 399))
        status = RequestStatus::BADREQ;
    else
        status = RequestStatus::OK;

    return status;
    }

RequestStatus RawServerResponse::ValidateJSONResponse(Json::Value& instances, Utf8StringCR keyword)
    {
    if ((toolCode != 0) || (responseCode > 399) || !Json::Reader::Parse(body, instances) || instances.isMember("errorMessage") || !instances.isMember(keyword))
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
    return nullptr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void* WSGRequest::PrepareRequest(const WSGURL& wsgRequest, RawServerResponse& responseObject, bool verifyPeer, BeFile* file) const
    {
    return nullptr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                02/2017
//-------------------------------------------------------------------------------------
void WSGRequest::_PerformRequest(const WSGURL& wsgRequest, RawServerResponse& response, bool verifyPeer, BeFile* file, bool retry) const
    {
    if(m_requestCallback)
        return m_requestCallback(wsgRequest, response, verifyPeer, file);
    else
        {
        response.body = "Curl is not included with this version of RealityPlatformTools."
                        "In order to perform requests, you must first set WSGRequest::GetInstance().SetRequestCallback(...)";
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason                10/2017
//-------------------------------------------------------------------------------------
void ConnectTokenManager::RefreshToken() const
    {
    if(m_tokenCallback)
        return m_tokenCallback(m_token, m_tokenRefreshTimer);
    }