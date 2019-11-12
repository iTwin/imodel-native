/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Bentley/BeVersion.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Jason.Wichert    6/2019
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ApplicationInfo> ApplicationInfoPtr;
typedef const struct  ApplicationInfo& ApplicationInfoCR;
typedef struct ApplicationInfo& ApplicationInfoR;

struct ApplicationInfo
    {
private:
    BeVersion m_version;
    Utf8String m_productId;
    Utf8String m_deviceId;
    Utf8String m_languageTag;

public:
    //! Create application info with mandatory fields and initialize deviceId and language automatically.
    //! There should be one ApplicationInfo created per application and shared to code that connects to web services.
    //! @param[in] applicationVersion - application build version could be used to identify application in server side.
    //! Usual ways to get build version:
    //! - Pass version numbers to code:
    //! \%include \$(SrcRoot)bsicommon/sharedmki/CreateBuildVersionHeader.mki
    //! \#define MY_PRODUCT_VERSION (REL_V "." MAJ_V "." MIN_V "." SUBMIN_V)
    //! - Pass whole version to code: 
    //! %include \$(SharedMki)stdversion.mki
    //! MY_PRODUCT_VERSION = \$(REL_V).\$(MAJ_V).\$(MIN_V).\$(SUBMIN_V)
    //! nameToDefine = MY_PRODUCT_VERSION = \\"\$(MY_PRODUCT_VERSION)\\"
    //! \%include cdefapnd.mki
    //! - Or just hardcode if used for testing, etc.
    //! @param[in] applicationProductId - registered 4-digit application product ID (e.g. "1234") used for IMS sign-in [optional otherwise]
    //! Given product ID is used for sign-in relying-party URI that is "sso://wsfed_desktop/1234" for Windows builds and 
    //! "sso://wsfed_mobile/1234" for iOS/Android/Linux builds. Both RPs need to be registered to IMS DEV/QA/PROD environments so 
    //! that your application would be allowed to sign-in by IMS.
    LICENSING_EXPORT static ApplicationInfoPtr Create
        (
        BeVersion applicationVersion,
        Utf8String applicationProductId = nullptr
        );

    //! Consider using ApplicationInfo::Create() instead. Create client info with custom values, only useful for testing.
    //! @param[in] applicationVersion - application build version could be used to identify application in server side.
    //! Usual ways to get build version:
    //! - Pass version numbers to code:
    //! \%include \$(SrcRoot)bsicommon/sharedmki/CreateBuildVersionHeader.mki
    //! \#define MY_PRODUCT_VERSION (REL_V "." MAJ_V "." MIN_V "." SUBMIN_V)
    //! - Pass whole version to code: 
    //! %include \$(SharedMki)stdversion.mki
    //! MY_PRODUCT_VERSION = \$(REL_V).\$(MAJ_V).\$(MIN_V).\$(SUBMIN_V)
    //! nameToDefine = MY_PRODUCT_VERSION = \\"\$(MY_PRODUCT_VERSION)\\"
    //! \%include cdefapnd.mki
    //! - Or just hardcode if used for testing, etc.
    //! @param[in] deviceId - unique device ID used for licensing. Should be different for different devices
    //! @param[in] applicationProductId - registered 4-digit application product ID (e.g. "1234") used for IMS sign-in [optional otherwise]
    //! Given product ID is used for sign-in relying-party URI that is "sso://wsfed_desktop/1234" for Windows builds and 
    //! "sso://wsfed_mobile/1234" for iOS/Android/Linux builds. Both RPs need to be registered to IMS DEV/QA/PROD environments so 
    //! that your application would be allowed to sign-in by IMS.
    LICENSING_EXPORT ApplicationInfo
        (
        BeVersion applicationVersion,
        Utf8String deviceId,
        Utf8String applicationProductId = nullptr
        );

    LICENSING_EXPORT virtual ~ApplicationInfo();

    //! Override user language. Values must follow IETF language tag standard (RFC 1766).
    //! Examples - "en", "en-US", "nan-Hant-TW", etc.
    LICENSING_EXPORT void SetLanguage(Utf8StringCR languageTag);
    LICENSING_EXPORT Utf8String GetLanguage() const;

    LICENSING_EXPORT BeVersion GetVersion() const;
    LICENSING_EXPORT Utf8String GetProductId() const;
    LICENSING_EXPORT Utf8String GetDeviceId() const;

#if defined (__ANDROID__)
    LICENSING_EXPORT static void CacheAndroidDeviceId(Utf8String deviceId);
#endif
    };

END_BENTLEY_LICENSING_NAMESPACE
