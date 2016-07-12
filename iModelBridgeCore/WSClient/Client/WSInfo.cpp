/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"

#define INFO_ServerVersion              "serverVersion"
#define INFO_Serialized_ServerVersion   "version"
#define INFO_Serialized_WebApiVersion   "webApi"
#define INFO_Serialized_ServerType      "type"

const BeVersion WSInfo::s_serverR1From("01.00.00.00");
const BeVersion WSInfo::s_serverR2From("01.01.00.00");
const BeVersion WSInfo::s_serverR3From("01.02.00.00");
const BeVersion WSInfo::s_serverR4From("02.00.00.00");

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo() :
m_type(Type::Unknown)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo(BeVersion serverVersion, BeVersion webApiVersion, Type serverType) :
m_serverVersion(serverVersion),
m_webApiVersion(webApiVersion),
m_type(serverType)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo(Http::ResponseCR response) : WSInfo()
    {
    if (!response.IsSuccess())
        {
        return;
        }

    ParseHeaders(response.GetHeaders(), m_type, m_serverVersion, m_webApiVersion);
    if (IsValid())
        {
        return;
        }

    ParseInfoPage(response, m_type, m_serverVersion, m_webApiVersion);
    if (IsValid())
        {
        return;
        }

    ParseAboutPage(response, m_type, m_serverVersion, m_webApiVersion);
    if (IsValid())
        {
        return;
        }

    m_type = Type::Unknown;
    m_serverVersion = BeVersion();
    m_webApiVersion = BeVersion();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo(Utf8StringCR serialized) : WSInfo()
    {
    Json::Value json;
    if (!Json::Reader::Parse(serialized, json))
        {
        return;
        }

    m_serverVersion = BeVersion(json[INFO_Serialized_ServerVersion].asString().c_str());
    m_webApiVersion = BeVersion(json[INFO_Serialized_WebApiVersion].asString().c_str());
    m_type = static_cast<Type>(json[INFO_Serialized_ServerType].asInt());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSInfo::ToString() const
    {
    Json::Value json;

    json[INFO_Serialized_ServerVersion] = m_serverVersion.ToString();
    json[INFO_Serialized_WebApiVersion] = m_webApiVersion.ToString();
    json[INFO_Serialized_ServerType] = static_cast<int>(m_type);

    return Json::FastWriter::ToString(json);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSInfo::ParseHeaders(HttpResponseHeadersCR headers, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut)
    {
    Utf8CP serverHeader = headers.GetServer();
    if (nullptr == serverHeader)
        {
        return;
        }

    bvector<Utf8String> servers;
    BeStringUtilities::Split(serverHeader, ",", servers);

    serverVersionOut = BeVersion();
    webApiVersionOut = BeVersion();

    for (Utf8String& server : servers)
        {
        server.Trim();

        if (serverVersionOut.IsEmpty())
            {
            serverVersionOut = BeVersion(server.c_str(), "Bentley-WSG/%d.%d.%d.%d");
            }

        if (webApiVersionOut.IsEmpty())
            {
            webApiVersionOut = BeVersion(server.c_str(), "Bentley-WebAPI/%d.%d");
            }
        }

    if (!serverVersionOut.IsEmpty() && !webApiVersionOut.IsEmpty())
        {
        typeOut = Type::BentleyWSG;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSInfo::ParseInfoPage(Http::ResponseCR response, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut)
    {
    Utf8String contentType = response.GetHeaders().GetContentType();
    if (Utf8String::npos == contentType.find("application/json"))
        {
        return;
        }

    Json::Value infoJson;
    if (!Json::Reader::Parse(response.GetBody().AsString(), infoJson))
        infoJson = Json::Value::null;

    JsonValueCR serverVersionJson = infoJson[INFO_ServerVersion];
    if (!serverVersionJson.isString())
        {
        return;
        }

    typeOut = Type::BentleyWSG;
    serverVersionOut = BeVersion(serverVersionJson.asCString());
    webApiVersionOut = DeduceWebApiVersion(serverVersionOut);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSInfo::ParseAboutPage(Http::ResponseCR response, Type& typeOut, BeVersion& serverVersionOut, BeVersion& webApiVersionOut)
    {
    Utf8String contentType = response.GetHeaders().GetContentType();
    if (Utf8String::npos == contentType.find("text/html"))
        {
        return;
        }

    Utf8String body = response.GetBody().AsString();
    if (body.find(R"(<span id="productNameLabel">Bentley Web Services Gateway 01.00</span>)") != Utf8String::npos)
        {
        typeOut = Type::BentleyWSG;
        serverVersionOut = BeVersion(1, 0);
        webApiVersionOut = BeVersion(1, 1);
        return;
        }

    if (body.find(R"(Web Service Gateway for BentleyCONNECT)") != Utf8String::npos &&
        body.find(R"(<span id="versionLabel">1.1.0.0</span>)") != Utf8String::npos)
        {
        typeOut = Type::BentleyConnect;
        serverVersionOut = BeVersion(1, 0);
        webApiVersionOut = BeVersion(1, 1);
        return;
        }

    // TODO: remove WSG pre-release support after TMA resolves their configuration
    if (body.find(R"(<span id="productNameLabel">Bentley Web Services Gateway 02.00</span>)") != Utf8String::npos &&
        body.find(R"(<span id="serverVersionLabel">02.00.00.12</span>)") != Utf8String::npos)
        {
        typeOut = Type::BentleyWSG;
        serverVersionOut = BeVersion(2, 0);
        webApiVersionOut = BeVersion(2, 0);
        return;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion WSInfo::DeduceWebApiVersion(BeVersionCR serverVersion)
    {
    if (serverVersion >= s_serverR4From)
        {
        return BeVersion();
        }
    if (serverVersion >= s_serverR3From)
        {
        return BeVersion(1, 3);
        }
    if (serverVersion >= s_serverR2From)
        {
        return BeVersion(1, 2);
        }
    return BeVersion(1, 1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsValid() const
    {
    return
        !m_serverVersion.IsEmpty() &&
        !m_webApiVersion.IsEmpty() &&
        m_type != Type::Unknown;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::Type WSInfo::GetType() const
    {
    return m_type;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersionCR WSInfo::GetVersion() const
    {
    return m_serverVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  WSInfo::IsR2OrGreater() const
    {
    return m_serverVersion >= s_serverR2From;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  WSInfo::IsR3OrGreater() const
    {
    return m_serverVersion >= s_serverR3From;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersionCR WSInfo::GetWebApiVersion() const
    {
    return m_webApiVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsWebApiSupported(BeVersionCR version) const
    {
    return m_webApiVersion >= version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsNavigationPropertySelectForAllClassesSupported() const
    {
    return IsWebApiSupported(BeVersion(1, 3));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsSchemaDownloadFullySupported() const
    {
    // R2 supports schema download but schemas are not properly configured for display
    return m_serverVersion >= s_serverR3From;
    }
