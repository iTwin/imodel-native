/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#if defined (__BASEGEOCOORD_BUILD__)
#   define BASEGEOCOORD_EXPORTED      EXPORT_ATTRIBUTE
#   define BASEMANAGEDGCS_EXPORTED    EXPORT_ATTRIBUTE
#else
#   define BASEGEOCOORD_EXPORTED      IMPORT_ATTRIBUTE
#   define BASEMANAGEDGCS_EXPORTED    IMPORT_ATTRIBUTE
#endif


