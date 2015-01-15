/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/UrlProvider.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Brad.Hadden    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UrlProvider
    {
    public:
        // Used for array indexes! Values must increment from 0
        enum Environment
            {
            Dev = 0,
            Qa,
            Release
            };

    private:
        static const Utf8String s_punchListWsgUrl[3];
        static const Utf8String s_connectWsgUrl[3];
        static const Utf8String s_connectEulaUrl[3];
        static const Utf8String s_connectLearnStsAuthUri[3];

        static bool s_isInitialized;

    public:
        WS_EXPORT static void Initialize (Environment env);
        WS_EXPORT static bool IsInitialized ();

        WS_EXPORT static Utf8StringCR GetPunchlistWsgUrl ();
        WS_EXPORT static Utf8StringCR GetConnectWsgUrl ();
        WS_EXPORT static Utf8StringCR GetConnectEulaUrl ();
        WS_EXPORT static Utf8StringCR GetConnectLearnStsAuthUri ();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
