/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSRepository.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"

#define INFO_Serialized_RepositoryId            "repositoryId"
#define INFO_Serialized_Location                "location"
#define INFO_Serialized_Label                   "label"
#define INFO_Serialized_Description             "description"
#define INFO_Serialized_PluginId                "pluginId"
#define INFO_Serialized_ServerUrl               "serverUrl"
#define INFO_Serialized_PluginVersion           "pluginVersion"
#define INFO_Serialized_ServiceVersion          "serviceVersion"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository::WSRepository() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository::WSRepository(Utf8StringCR serialized) : WSRepository()
    {
    Json::Value json;
    if (!Json::Reader::Parse(serialized, json))
        return;

    m_id = json[INFO_Serialized_RepositoryId].asString();
    m_location = json[INFO_Serialized_Location].asString();
    m_label = json[INFO_Serialized_Label].asString();
    m_description = json[INFO_Serialized_Description].asString();
    m_pluginId = json[INFO_Serialized_PluginId].asString().c_str();
    m_serverUrl = json[INFO_Serialized_ServerUrl].asString();
    m_pluginVersion = BeVersion(json[INFO_Serialized_PluginVersion].asString().c_str());
    m_serviceVersion = BeVersion(json[INFO_Serialized_ServiceVersion].asString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetId() const
    {
    return m_id;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetId(Utf8String id)
    {
    m_id = std::move(id);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetLocation() const
    {
    return m_location;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetLocation(Utf8String location)
    {
    m_location = std::move(location);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetLabel() const
    {
    return m_label;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersionCR WSRepository::GetPluginVersion() const
    {
    return m_pluginVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeVersionCR WSRepository::GetServiceVersion() const
    {
    return m_serviceVersion;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetLabel(Utf8String label)
    {
    m_label = std::move(label);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetDescription() const
    {
    return m_description;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetDescription(Utf8String description)
    {
    m_description = std::move(description);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetPluginId() const
    {
    return m_pluginId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetPluginId(Utf8String pluginId)
    {
    m_pluginId = std::move(pluginId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSRepository::GetServerUrl() const
    {
    return m_serverUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetServerUrl(Utf8String url)
    {
    m_serverUrl = url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetPluginVersion(BeVersion version)
    {
    m_pluginVersion = version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WSRepository::SetServiceVersion(BeVersion version)
    {
    m_serviceVersion = version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Vilius.Kazlauskas    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSRepository::IsValid() const
    {
    if (m_serverUrl.empty() || m_id.empty())
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas   05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSRepository::ToString() const
    {
    Json::Value json;

    json[INFO_Serialized_RepositoryId] = m_id;
    json[INFO_Serialized_Location] = m_location;
    json[INFO_Serialized_Label] = m_label;
    json[INFO_Serialized_Description] = m_description;
    json[INFO_Serialized_PluginId] = m_pluginId;
    json[INFO_Serialized_ServerUrl] = m_serverUrl;
    json[INFO_Serialized_PluginVersion] = m_pluginVersion.ToString();
    json[INFO_Serialized_ServiceVersion] = m_serviceVersion.ToString();
    return Json::FastWriter::ToString(json);
    }
