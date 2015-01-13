/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/WSInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/Utils/Http/HttpResponse.h>
#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSInfo
    {
    public:
        enum class Type
            {
            Unknown = 0,
            BentleyWSG = 1,
            BentleyConnect = 2
            };

    private:
        static const BeVersion s_serverR1From;
        static const BeVersion s_serverR2From;
        static const BeVersion s_serverR3From;
        static const BeVersion s_serverR4From;

        Type m_type;
        BeVersion m_version;

    private:
        static BentleyStatus ExtractTypeAndVersionFromAboutPage (Utf8StringCR body, Type& typeOut, BeVersion& versionOut);

    public:
        struct WebApiVersion;

        //! Create invalid info
        WS_EXPORT WSInfo ();
        //! Construct info with values
        WS_EXPORT WSInfo (BeVersion serverVersion, Type serverType);
        //! Create info from server response
        WS_EXPORT WSInfo (MobileDgn::Utils::HttpResponseCR response);
        //! Deserialize string info
        WS_EXPORT WSInfo (Utf8StringCR serialized);

        //! Returns true if server type and version is known
        WS_EXPORT bool IsValid () const;

        //! Returns type of server detected
        WS_EXPORT Type GetType () const;

        //! Returns version of WSG used
        WS_EXPORT BeVersionCR GetVersion () const;

        WS_EXPORT bool IsR2OrGreater () const;
        WS_EXPORT bool IsR3OrGreater () const;
        WS_EXPORT bool IsR4OrGreater () const;

        WS_EXPORT BeVersion GetWebApiVersion () const;
        WS_EXPORT bool IsWebApiSupported (BeVersionCR version) const;

        WS_EXPORT bool IsNavigationPropertySelectForAllClassesSupported () const;
        WS_EXPORT bool IsSchemaDownloadFullySupported () const;

        //! Serialize info to string
        WS_EXPORT Utf8String ToString () const;
    };

typedef WSInfo& WSInfoR;
typedef const WSInfo& WSInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
