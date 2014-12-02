/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/EldscrFuncs.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_NAMESPACE

DGNPLATFORM_EXPORT int appendLinkageToDescr (MSElementDescrH elmDscrPP, LinkageHeaderP linkHdrP, void const* dataP, void* convRulesPP);
DGNPLATFORM_EXPORT int appendLinkageToDescr (MSElementDescrH elmDscrPP, LinkageHeaderCP linkHdrP, void const* dataP, void* convRulesPP);

END_BENTLEY_API_NAMESPACE
