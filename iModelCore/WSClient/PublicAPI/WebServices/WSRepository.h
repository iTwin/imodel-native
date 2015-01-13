/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/WSRepository.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
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

    public:
        WS_EXPORT WSRepository ();

        WS_EXPORT Utf8StringCR GetId () const;
        WS_EXPORT Utf8StringCR GetLocation () const;
        WS_EXPORT Utf8StringCR GetLabel () const;
        WS_EXPORT Utf8StringCR GetDescription () const;
        WS_EXPORT Utf8StringCR GetPluginId () const;

        WS_EXPORT void SetId (Utf8String id);
        WS_EXPORT void SetLocation (Utf8String location);
        WS_EXPORT void SetLabel (Utf8String label);
        WS_EXPORT void SetDescription (Utf8String description);
        WS_EXPORT void SetPluginId (Utf8String type);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
