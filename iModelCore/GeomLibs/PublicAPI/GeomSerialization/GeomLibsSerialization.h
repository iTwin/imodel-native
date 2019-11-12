/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <Bentley/Bentley.h>

#if defined(__GEOMLIBS_SERIALIZATION_BUILD__)
    #define GEOMLIBS_SERIALIZATION_EXPORT EXPORT_ATTRIBUTE
#else
    #define GEOMLIBS_SERIALIZATION_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_GEOMETRY_NAMESPACE BEGIN_BENTLEY_NAMESPACE
#define END_BENTLEY_GEOMETRY_NAMESPACE   END_BENTLEY_NAMESPACE
