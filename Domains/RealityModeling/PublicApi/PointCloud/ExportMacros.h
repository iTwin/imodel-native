/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__POINTCLOUD_BUILD__)
#   define POINTCLOUD_EXPORT      EXPORT_ATTRIBUTE
#else
#   define POINTCLOUD_EXPORT      IMPORT_ATTRIBUTE
#endif

