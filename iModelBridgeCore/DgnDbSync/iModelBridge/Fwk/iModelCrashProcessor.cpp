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
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   iModelCrashProcessor::GetCrashReportUrl(Utf8StringR url)
    {
    if (nullptr == m_buddi)
        m_buddi = BentleyApi::WebServices::IBuddiClientPtr(new BentleyApi::WebServices::BuddiClient());

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
BentleyStatus   iModelCrashProcessor::SendCrashReport(Utf8StringCR exceptionString, WCharCP dmpFile)
    {
    if (nullptr == m_clientInfo)//TODO Construct a dummy client Info
        return ERROR;

    Utf8String crashReportUrl;
    if (SUCCESS != GetCrashReportUrl(crashReportUrl))
        return ERROR;

    /*BeXmlDomPtr dom = BeXmlDom::CreateEmpty();
    if (dom.IsNull())
        return ERROR;*/
    /*
    TODO: Get the site id.
    Dim sSiteId : sSiteId = "" : sSiteId = g_oWShell.RegRead ("HKEY_LOCAL_MACHINE\SOFTWARE\Bentley\Licensing\1.1\SiteId")
    if (0 = len(sSiteId)) Then
        sSiteId = g_oWShell.RegRead ("HKEY_CURRENT_USER\SOFTWARE\Bentley\Licensing\1.1\SiteId")
    End if
    */
    Utf8PrintfString additionalInfo(R"xml(
        <CustomerComment>%s</CustomerComment>
        <MessageGUID>%s</MessageGUID>
        <SiteId>%s</SiteId>
        )xml", m_requestGuid.c_str(), m_requestGuid.c_str(), m_jobRunGuid.c_str());

    Utf8String dumpStr;
    BeFile dumpFile;
    if (BeFileStatus::Success == dumpFile.Open(dmpFile, BeFileAccess::Read))
        {
        //size_t fileSize = 0;
        //dumpFile.GetSize(fileSize);
        //size_t nEncodedBytes = (size_t)(4.0 * ((fileSize + 2) / 3.0 ));
        //dumpStr.reserve(nEncodedBytes);
       /* Byte buffer[1024];
        uint32_t bytesRead;
        while (BeFileStatus::Success == dumpFile.Read(buffer, &bytesRead, 1024) && bytesRead > 0)
            {
            Base64Utilities::Encode(dumpStr, buffer, bytesRead);
            }*/
        bvector<Byte> buffer;
        dumpFile.ReadEntireFile(buffer);
        Base64Utilities::Encode(dumpStr, &buffer[0], buffer.size());
        dumpFile.Close();
        }

    Utf8PrintfString body(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                               xmlns:xsd="http://www.w3.org/2001/XMLSchema"
                               xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
                    <soap:Body>
                        <SubmitLog xmlns="http://tempuri.org/">
                            <productName>%s</productName>
                            <productVersion>%s</productVersion>
                            <replyEmailAddress></replyEmailAddress>
                            <subject>%s %s exception</subject>
                            <base64MiniDumpFile>%s</base64MiniDumpFile>
                            <base64ExceptionLog>%s</base64ExceptionLog>
                            <base64AdditionalInfo>%s</base64AdditionalInfo>
                        </SubmitLog>
                    </soap:Body>
                </soap:Envelope>)xml", m_clientInfo->GetApplicationName().c_str(), m_clientInfo->GetApplicationVersion().ToString().c_str(),
        m_clientInfo->GetApplicationName().c_str(), m_clientInfo->GetApplicationVersion().ToString().c_str(), dumpStr.c_str(), Base64Utilities::Encode(exceptionString).c_str(),
                                    Base64Utilities::Encode(additionalInfo).c_str());

    T_Utf8StringVector urlParts;
    BeStringUtilities::Split(crashReportUrl.c_str(), "/", urlParts);
    Http::HttpClient client;
    Http::Request request = client.CreatePostRequest(crashReportUrl);
    request.SetValidateCertificate(true);
    request.SetRequestBody(Http::HttpStringBody::Create(body));
    request.GetHeaders().SetContentType("text/xml; charset=utf-8");
    //request.GetHeaders().SetValue("Man", "POST /errorreporting/errorreporting.asmx HTTP/1.1");
    request.GetHeaders().SetValue("SOAPAction", "http://tempuri.org/SubmitLog");
    request.GetHeaders().SetValue("Host", urlParts[1]);
//    xmlhttpReport.setRequestHeader "Host", split(urlReport, "/")(2)


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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
iModelCrashProcessor& iModelCrashProcessor::GetInstance()
    {
    static iModelCrashProcessor s_Instance;
    return s_Instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelCrashProcessor::SetClientInfo(BentleyApi::WebServices::ClientInfoPtr clientInfo)
    {
    m_clientInfo = clientInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void            iModelCrashProcessor::SetRunInfo(Utf8StringCR jobRunGuid, Utf8StringCR requestGuid)
    {
    m_jobRunGuid = jobRunGuid;
    m_requestGuid = requestGuid;
    }
