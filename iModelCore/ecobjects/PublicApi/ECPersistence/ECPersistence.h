/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPersistence/ECPersistence.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjects.h>

#ifdef __ECOBJECTS_BUILD__
    #define ECPERSISTENCE_EXPORT EXPORT_ATTRIBUTE
#else
    #define ECPERSISTENCE_EXPORT IMPORT_ATTRIBUTE
#endif

EC_TYPEDEFS(IECConnection);
EC_TYPEDEFS(IECSchemaManager);
EC_TYPEDEFS(IECStatement);

