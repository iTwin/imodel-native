/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSRepository
    {
    private:
        Utf8String m_id;
        Utf8String m_location;
        Utf8String m_label;
        Utf8String m_description;
        Utf8String m_pluginId;
        Utf8String m_serverUrl;
        BeVersion  m_pluginVersion;
        BeVersion  m_serviceVersion;
        uint64_t   m_maxUploadSize;

    public:
        WSCLIENT_EXPORT WSRepository();
        WSCLIENT_EXPORT WSRepository(Utf8StringCR serialized);

        Utf8StringCR GetId() const { return m_id; };
        Utf8StringCR GetLocation() const { return m_location; };
        Utf8StringCR GetLabel() const { return m_label; };
        Utf8StringCR GetDescription() const { return m_description; };
        Utf8StringCR GetPluginId() const { return m_pluginId; };
        Utf8StringCR GetServerUrl() const { return m_serverUrl; };
        BeVersionCR GetPluginVersion() const { return m_pluginVersion; };
        BeVersionCR GetServiceVersion() const { return m_serviceVersion; };
        uint64_t GetMaxUploadSize() const { return m_maxUploadSize; };

        void SetId(Utf8String id) { m_id = std::move(id); };
        void SetLocation(Utf8String location) { m_location = std::move(location); };
        void SetLabel(Utf8String label) { m_label = std::move(label); };
        void SetDescription(Utf8String description) { m_description = std::move(description); };
        void SetPluginId(Utf8String pluginId) { m_pluginId = std::move(pluginId); };
        void SetServerUrl(Utf8String url) { m_serverUrl = std::move(url); };
        void SetPluginVersion(BeVersion version) { m_pluginVersion = version; };
        void SetServiceVersion(BeVersion version) { m_serviceVersion = version; };
        void SetMaxUploadSize(uint64_t maxUploadSize) { m_maxUploadSize = maxUploadSize; };

        //! Check if WSRepository contains minimum information required - server URL and repository ID
        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT Utf8String ToString() const;
    };

typedef const WSRepository& WSRepositoryCR;
typedef WSRepository& WSRepositoryR;

END_BENTLEY_WEBSERVICES_NAMESPACE
