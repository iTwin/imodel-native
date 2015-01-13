/*--------------------------------------------------------------------------------------+
|
|     $Source: WebServices/WSInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesInternal.h"

#define INFO_ServerVersion              "serverVersion"
#define INFO_Serialized_ServerVersion   "version"
#define INFO_Serialized_ServerType      "type"

const BeVersion WSInfo::s_serverR1From ("01.00.00.00");
const BeVersion WSInfo::s_serverR2From ("01.01.00.00");
const BeVersion WSInfo::s_serverR3From ("01.02.00.00");
const BeVersion WSInfo::s_serverR4From ("02.00.00.00");

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo () :
m_type (Type::Unknown)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo (BeVersion serverVersion, Type serverType) :
m_version (serverVersion),
m_type (serverType)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo (HttpResponseCR response) : WSInfo ()
    {
    if (!response.IsSuccess ())
        {
        return;
        }

    Utf8String serverHeader = Utf8String (response.GetHeaders ().GetValue ("Server"));
    if (serverHeader.find (Utf8String ("Bentley-WebAPI/2.0")) != Utf8String::npos)
        {
        m_type = Type::BentleyWSG;
        m_version = BeVersion (2, 0, 0, 0);
        return;
        }

    Utf8String contentType = response.GetHeaders ().GetContentType ();

    if (contentType.find ("text/html") != Utf8String::npos)
        {
        ExtractTypeAndVersionFromAboutPage (response.GetBody ().AsString (), m_type, m_version);
        return;
        }

    Json::Value infoJson = response.GetBody ().AsJson ();
    JsonValueCR serverVersionJson = infoJson[INFO_ServerVersion];
    if (serverVersionJson.isString ())
        {
        m_type = Type::BentleyWSG;
        m_version = BeVersion (serverVersionJson.asCString ());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::WSInfo (Utf8StringCR serialized) : WSInfo ()
    {
    Json::Value json;
    if (!Json::Reader::Parse (serialized, json))
        {
        return;
        }

    m_version = BeVersion (json[INFO_Serialized_ServerVersion].asString ().c_str ());
    m_type = static_cast<Type>(json[INFO_Serialized_ServerType].asInt ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSInfo::ToString () const
    {
    Json::Value json;

    json[INFO_Serialized_ServerVersion] = m_version.ToString ();
    json[INFO_Serialized_ServerType] = static_cast<int>(m_type);

    return Json::FastWriter::ToString (json);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WSInfo::ExtractTypeAndVersionFromAboutPage (Utf8StringCR body, Type& typeOut, BeVersion& versionOut)
    {
    if (body.find (R"(<span id="productNameLabel">Bentley Web Services Gateway 01.00</span>)") != Utf8String::npos)
        {
        typeOut = Type::BentleyWSG;
        versionOut = BeVersion (1, 0);
        return SUCCESS;
        }

    if (body.find (R"(Web Service Gateway for BentleyCONNECT)") != Utf8String::npos &&
        body.find (R"(<span id="versionLabel">1.1.0.0</span>)") != Utf8String::npos)
        {
        typeOut = Type::BentleyConnect;
        versionOut = BeVersion (1, 0);
        return SUCCESS;
        }

    if (body.find (R"(<span id="productNameLabel">Bentley Web Services Gateway 02.00</span>)") != Utf8String::npos)
        {
        // TODO: This is temporary workarround.
        // WSG 2.0 does not yet have a way to get server info.
        typeOut = Type::BentleyWSG;
        versionOut = BeVersion (2, 0);
        return SUCCESS;
        }

    // Unknown server
    typeOut = Type::Unknown;
    versionOut = BeVersion ();
    return ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsValid () const
    {
    return m_version.major != 0 && m_type != Type::Unknown;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSInfo::Type WSInfo::GetType () const
    {
    return m_type;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersionCR WSInfo::GetVersion () const
    {
    return m_version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  WSInfo::IsR2OrGreater () const
    {
    return m_version >= s_serverR2From;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  WSInfo::IsR3OrGreater () const
    {
    return m_version >= s_serverR3From;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  WSInfo::IsR4OrGreater () const
    {
    return m_version >= s_serverR4From;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersion WSInfo::GetWebApiVersion () const
    {
    if (m_version >= s_serverR4From)
        {
        return BeVersion (2, 0);
        }
    if (m_version >= s_serverR3From)
        {
        return BeVersion (1, 3);
        }
    if (m_version >= s_serverR2From)
        {
        return BeVersion (1, 2);
        }
    return BeVersion (1, 1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsWebApiSupported (BeVersionCR version) const
    {
    return GetWebApiVersion () >= version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsNavigationPropertySelectForAllClassesSupported () const
    {
    return IsWebApiSupported (BeVersion (1, 3));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSInfo::IsSchemaDownloadFullySupported () const
    {
    // R2 supports schema download but schemas are not properly configured for display
    return m_version >= s_serverR3From;
    }
