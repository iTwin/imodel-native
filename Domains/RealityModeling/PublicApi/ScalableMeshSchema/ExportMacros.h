/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ExportMacros.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined (__SCALABLEMESHSCHEMA_BUILD__)
#   define SCALABLEMESHSCHEMA_EXPORT      EXPORT_ATTRIBUTE
#else
#   define SCALABLEMESHSCHEMA_EXPORT      IMPORT_ATTRIBUTE
#endif

