/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/MarkupHandlers.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define     TYPE106_ATTROFFSET              56

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

// Minor Ids for Major Id XATTRIBUTEID_Markup
enum MarkupHandlerMinorId
    {
    MINORID_Placemark           = 0,
    MINORID_ContentArea         = 1,    // Handler
    MINORID_TemplateContent     = 2,    // XAttribute
    MINORID_ContentAreaVersion  = 3,    // XAttribute
    MINORID_ContentAreaArea     = 4,    // XAttribute
    };


END_BENTLEY_DGNPLATFORM_NAMESPACE
