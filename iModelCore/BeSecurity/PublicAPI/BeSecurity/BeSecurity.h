/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Logging/bentleylogging.h>

//__PUBLISH_SECTION_END__
#undef BESECURITY_EXPORT
#ifdef __BESECURITY_BUILD__
    #define BESECURITY_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
    #define BESECURITY_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__
#endif
//__PUBLISH_SECTION_START__

#define SECURITY_NAMESPACE_NAME                            Security
#define BEGIN_BENTLEY_SECURITY_NAMESPACE                   BEGIN_BENTLEY_NAMESPACE namespace SECURITY_NAMESPACE_NAME {
#define END_BENTLEY_SECURITY_NAMESPACE                     END_BENTLEY_NAMESPACE }
#define USING_NAMESPACE_BENTLEY_SECURITY                   using namespace BentleyApi::SECURITY_NAMESPACE_NAME;

#define KEYTAR_NAMESPACE_NAME   keytar
#define BEGIN_KEYTAR_NAMESPACE  BEGIN_BENTLEY_SECURITY_NAMESPACE namespace KEYTAR_NAMESPACE_NAME {
#define END_KEYTAR_NAMESPACE    END_BENTLEY_SECURITY_NAMESPACE }
#define USING_NAMESPACE_KEYTAR  using namespace BentleyApi::SECURITY_NAMESPACE_NAME::KEYTAR_NAMESPACE_NAME;

#define LOGGER_NAMESPACE_SECURITY  "BeSecurity"

BEGIN_BENTLEY_SECURITY_NAMESPACE
END_BENTLEY_SECURITY_NAMESPACE
