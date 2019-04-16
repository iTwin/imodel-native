/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#define INFO_Serialized_MaxUploadSize           "maxUploadSize"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSRepository::WSRepository() : m_maxUploadSize(0) {}

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
    m_maxUploadSize = json[INFO_Serialized_MaxUploadSize].asUInt64();
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
    json[INFO_Serialized_MaxUploadSize] = Json::Value(m_maxUploadSize);
    return Json::FastWriter::ToString(json);
    }
