/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSRepository.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    public:
        WSCLIENT_EXPORT WSRepository();
        WSCLIENT_EXPORT WSRepository(Utf8StringCR serialized);

        WSCLIENT_EXPORT Utf8StringCR GetId() const;
        WSCLIENT_EXPORT Utf8StringCR GetLocation() const;
        WSCLIENT_EXPORT Utf8StringCR GetLabel() const;
        WSCLIENT_EXPORT Utf8StringCR GetDescription() const;
        WSCLIENT_EXPORT Utf8StringCR GetPluginId() const;
        WSCLIENT_EXPORT Utf8StringCR GetServerUrl() const;
        WSCLIENT_EXPORT BeVersionCR GetPluginVersion() const;
        WSCLIENT_EXPORT BeVersionCR GetServiceVersion() const;

        WSCLIENT_EXPORT void SetId(Utf8String id);
        WSCLIENT_EXPORT void SetLocation(Utf8String location);
        WSCLIENT_EXPORT void SetLabel(Utf8String label);
        WSCLIENT_EXPORT void SetDescription(Utf8String description);
        WSCLIENT_EXPORT void SetPluginId(Utf8String type);
        WSCLIENT_EXPORT void SetServerUrl(Utf8String url);
        WSCLIENT_EXPORT void SetPluginVersion(BeVersion version);
        WSCLIENT_EXPORT void SetServiceVersion(BeVersion version);

        //! Check if WSRepository contains minimum information required - server URL and repository ID
        WSCLIENT_EXPORT bool IsValid() const;
        WSCLIENT_EXPORT Utf8String ToString() const;
    };

typedef const WSRepository& WSRepositoryCR;
typedef WSRepository& WSRepositoryR;

END_BENTLEY_WEBSERVICES_NAMESPACE
