/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/Properties/PropertiesAPI.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if defined (__PROPERTIES_BUILD__)
#define PROPERTIES_EXPORT EXPORT_ATTRIBUTE
#endif

#if !defined (PROPERTIES_EXPORT)
#define PROPERTIES_EXPORT IMPORT_ATTRIBUTE
#endif

#include "PropertiesProvider.h"



