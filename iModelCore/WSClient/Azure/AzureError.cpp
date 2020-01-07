/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Azure/AzureError.h>
#include <BeXml/BeXml.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
#define XML_Azure_Error         "Error"
#define XML_Azure_Code          "Code"
#define XML_Azure_Message       "Message"


/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AzureError::ParseBody(Http::ResponseCR response)
    {
    Utf8String body = response.GetBody().AsString();

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromMemory(xmlStatus, body.c_str(), body.size());
    if (BeXmlStatus::BEXML_Success != xmlStatus)
        return;

    BeXmlNodeP rootNode = xmlDom->GetRootElement();
    if (nullptr == rootNode)
        return;

    if (Utf8String(XML_Azure_Error) != rootNode->GetName())
        return;

    BeXmlNodeP codeNode = rootNode->SelectSingleNode(XML_Azure_Code);
    if (nullptr != codeNode)
        codeNode->GetContent(m_code);

    BeXmlNodeP messageNode = rootNode->SelectSingleNode(XML_Azure_Message);
    if (nullptr != messageNode)
        messageNode->GetContent(m_message);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AzureError::AzureError() : HttpError()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AzureError::AzureError(Http::ResponseCR response) : HttpError(response)
    {
    Utf8String contentType = response.GetHeaders().GetContentType();
    if (contentType.find(REQUESTHEADER_ContentType_ApplicationXml) != Utf8String::npos)
        ParseBody(response);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AzureError::AzureError(Http::ConnectionStatus connectionStatus, Http::HttpStatus httpStatus,
    Utf8StringCR code, Utf8StringCR message, Utf8StringCR description) : HttpError(connectionStatus, httpStatus)
    {
    m_code = code;
    m_message = message;
    m_description = description;
    }
