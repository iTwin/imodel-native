/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__RASTER_BUILD__)
#   define RASTER_EXPORT      EXPORT_ATTRIBUTE
#else
#   define RASTER_EXPORT      IMPORT_ATTRIBUTE
#endif

