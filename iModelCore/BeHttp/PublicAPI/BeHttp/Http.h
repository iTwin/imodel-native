/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>

//__PUBLISH_SECTION_END__
#undef BEHTTP_EXPORT
#ifdef __BEHTTP_BUILD__
    #define BEHTTP_EXPORT EXPORT_ATTRIBUTE
#else
//__PUBLISH_SECTION_START__
    #define BEHTTP_EXPORT IMPORT_ATTRIBUTE
//__PUBLISH_SECTION_END__
#endif
//__PUBLISH_SECTION_START__

#define BENTLEY_HTTP_NAMESPACE_NAME   Http
#define BEGIN_BENTLEY_HTTP_NAMESPACE  BEGIN_BENTLEY_NAMESPACE namespace BENTLEY_HTTP_NAMESPACE_NAME {
#define END_BENTLEY_HTTP_NAMESPACE    END_BENTLEY_NAMESPACE }
#define USING_NAMESPACE_BENTLEY_HTTP  using namespace BentleyApi::BENTLEY_HTTP_NAMESPACE_NAME;

#define LOGGER_NAMESPACE_BENTLEY_HTTP       "Bentley.Http"
#define LOGGER_NAMESPACE_BENTLEY_HTTP_TIMES "Bentley.Http.Times"

BEGIN_BENTLEY_HTTP_NAMESPACE
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Request)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Response)
    DEFINE_POINTER_SUFFIX_TYPEDEFS(HttpError)
END_BENTLEY_HTTP_NAMESPACE
