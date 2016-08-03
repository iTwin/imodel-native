/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/IMSSearch/IMSSearch.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CWSCCInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_GetIMSUserInfo(CWSCCHANDLE apiHandle, CWSCCDATABUFHANDLE* imsUserBuffer)
    {
    VERIFY_API

    if (imsUserBuffer == NULL)
        {
        api->SetStatusMessage("The imsUserBuffer parameter is NULL.");
        api->SetStatusDescription("The imsUserBuffer parameter passed into ConnectWebServicesClientC_GetIMSUserInfo is invalid.");
        return INVALID_PARAMETER;
        }

    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token"; //TODO: UrlProvider::Urls::ImsSearch.Get()
    Utf8String collection = "IMS/User";
    if (api->m_solrClients.find(searchApiUrl + collection) == api->m_solrClients.end())
        {
        api->CreateSolrClient
            (
            searchApiUrl,
            collection
            );
        }

    auto client = api->m_solrClients.find(searchApiUrl + collection)->second;
    auto result = client->SendGetRequest()->GetResult();
    if (!result.IsSuccess())
        return httperrorToConnectWebServicesClientStatus(api, result.GetError().GetHttpStatus(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());
    
    CWSCCBUFFER* buf = (CWSCCBUFFER*) calloc(1, sizeof(CWSCCBUFFER));
    if (buf == nullptr)
        {
        free(buf);
        api->SetStatusMessage("Memory failed to initialize interally.");
        api->SetStatusDescription("Failed to calloc memory for CWSCCBUFFER.");
        return INTERNAL_MEMORY_ERROR;
        }

    LPCWSCCIMSUSERBUFFER bufToFill = new CWSCCIMSUSERBUFFER;
    ImsUser_BufferStuffer(bufToFill, result.GetValue());
    buf->lItems.push_back(bufToFill);

    buf->lCount = buf->lItems.size();
    buf->lClassType = BUFF_TYPE_IMSUSER;
    buf->lSchemaType = SCHEMA_TYPE_IMSSEARCH;
    buf->isWSGBuffer = false;
    *imsUserBuffer = (CWSCCDATABUFHANDLE) buf;

    api->SetStatusMessage("Success!");
    api->SetStatusDescription("The IMS user info was successfully retreived.");
    return SUCCESS;
    }