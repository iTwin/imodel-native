/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/RefAttachHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/bvector.h>
#include <DgnPlatform/DgnHandlers/IEcPropertyHandler.h>
#include <DgnPlatform/DgnHandlers/ElementECProvider.h>
#include <DgnPlatform/DgnCore/ITransactionHandler.h>

#include "DelegateMaps.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef enum
    {
    REFDIFF_NoDifference        = 0,
    REFDIFF_Parameters          = 1,
    REFDIFF_LevelMask           = 2,
    REFDIFF_LevelMaskRefTime    = 3,
    REFDIFF_Overrides           = 4,
    } ReferenceDiff;

END_BENTLEY_DGNPLATFORM_NAMESPACE
