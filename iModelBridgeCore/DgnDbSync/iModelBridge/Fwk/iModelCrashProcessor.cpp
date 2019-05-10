/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelCrashProcessor.h"
#include <WebServices/Client/ClientInfo.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelCrashProcessor::iModelCrashProcessor()
    :m_buddi (new BentleyApi::WebServices::BuddiClient())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelCrashProcessor::GetCrashReportUrl(Utf8StringR url)
    {
    BentleyApi::WebServices::BuddiUrlResult result = m_buddi->GetUrl("ErrorReporting")->GetResult();
    if (!result.IsSuccess())
        {
        return ERROR;
        }

    url = result.GetValue();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelCrashProcessor::SendCrashReport(BentleyApi::WebServices::ClientInfoPtr clientInfo, Utf8StringCR requestId)
    {
    Utf8String crashReportUrl;
    if (SUCCESS != GetCrashReportUrl(crashReportUrl))
        return ERROR;

    /*BeXmlDomPtr dom = BeXmlDom::CreateEmpty();
    if (dom.IsNull())
        return ERROR;*/
    /*
    Dim sSiteId : sSiteId = "" : sSiteId = g_oWShell.RegRead ("HKEY_LOCAL_MACHINE\SOFTWARE\Bentley\Licensing\1.1\SiteId")
    if (0 = len(sSiteId)) Then
        sSiteId = g_oWShell.RegRead ("HKEY_CURRENT_USER\SOFTWARE\Bentley\Licensing\1.1\SiteId")
    End if
    */
    Utf8PrintfString additionalInfo(R"xml(
        <CustomerComment></CustomerComment>
        <MessageGUID>%s</MessageGUID>
        <SiteId>%s</SiteId>
        )xml", requestId.c_str(),"Test-iModelBridge");

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                    <soap:Body>
                        <SubmitLog xmlns="http://tempuri.org/">
                            <productName>%s</productName>"
                            <productVersion>%s</productVersion>"
                            <replyEmailAddress></replyEmailAddress>"
                            <subject>Crash RequestId: %s</subject>"
                            <base64MiniDumpFile></base64MiniDumpFile>"
                            <base64ExceptionLog></base64ExceptionLog>"
                            <base64AdditionalInfo>
                            %s
                            </base64AdditionalInfo>"
                        </SubmitLog>"
                    </soap:Body>
                </soap:Envelope>)xml", clientInfo->GetApplicationName().c_str(), clientInfo->GetApplicationVersion().ToString().c_str(), requestId.c_str(), Base64Utilities::Encode(additionalInfo).c_str());

    Http::HttpClient client;
    Http::Request request = client.CreatePostRequest(crashReportUrl);
    request.SetValidateCertificate(true);
    request.SetRequestBody(Http::HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("text/xml; charset=utf-8");

    auto status = request.Perform().then([=] (Http::Response const& response)
                {
                if (response.GetConnectionStatus() != Http::ConnectionStatus::OK ||
                    response.GetHttpStatus() != Http::HttpStatus::OK)
                    {
                    return ERROR;
                    }

                return SUCCESS;
                });
    return status.get();
    }