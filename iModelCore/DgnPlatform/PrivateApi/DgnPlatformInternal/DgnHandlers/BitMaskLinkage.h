/*----------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnHandlers/BitMaskLinkage.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef struct MSBitMaskLinkage
    {
    LinkageHeader           header;
    MSBitMaskLinkageData    data;
    
    } MSBitMaskLinkage;

END_BENTLEY_DGNPLATFORM_NAMESPACE
