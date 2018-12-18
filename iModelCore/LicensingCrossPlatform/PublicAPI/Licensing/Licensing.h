/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/PublicAPI/Licensing/Licensing.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>

//__PUBLISH_SECTION_END__
#undef LICENSING_EXPORT
#ifdef __BELICENSING_BUILD__
    #define LICENSING_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
    #define LICENSING_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__ 
#endif
//__PUBLISH_SECTION_START__

#define BENTLEY_LICENSING_NAMESPACE_NAME   Licensing
#define BEGIN_BENTLEY_LICENSING_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace BENTLEY_LICENSING_NAMESPACE_NAME {
#define END_BENTLEY_LICENSING_NAMESPACE    END_BENTLEY_NAMESPACE }
#define USING_NAMESPACE_BENTLEY_LICENSING  using namespace BentleyApi::BENTLEY_LICENSING_NAMESPACE_NAME;

#define LOGGER_NAMESPACE_BENTLEY_LICENSING  "Bentley.LICENSING"

