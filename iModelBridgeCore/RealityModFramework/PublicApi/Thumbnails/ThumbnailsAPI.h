/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Thumbnails/ThumbnailsAPI.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined (__THUMBNAILS_BUILD__)
#define THUMBNAILS_EXPORT EXPORT_ATTRIBUTE
#endif

#if !defined (THUMBNAILS_EXPORT)
#define THUMBNAILS_EXPORT IMPORT_ATTRIBUTE
#endif

#include "ThumbnailsProvider.h"



