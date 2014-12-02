/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnLinkage.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
typedef bool    (*LinkageFunc)(LinkageHeaderP linkage, CallbackArgP userArg);

#include <DgnPlatform/DgnCore/DgnElements.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef struct msStringLinkage
    {
    LinkageHeader           header;
    MSStringLinkageData     data;
    } MSStringLinkage;

END_BENTLEY_DGNPLATFORM_NAMESPACE

BEGIN_BENTLEY_API_NAMESPACE

typedef int (*LinkageProcessFuncP)
    (
    LinkageHeaderP  pOutLink,
    CallbackArgP    pParams,
    LinkageHeaderP  pInLink,
    DgnElementP      pElem
    );

typedef StatusInt (*VisitLinkageFunc) (LinkageHeaderCP, CallbackArgP arg);
typedef StatusInt (*T_LinkFunc)(LinkageHeaderCP);

DGNPLATFORM_EXPORT int        mdlElement_processLinkages (LinkageProcessFuncP, CallbackArgP pLinkFuncArg, DgnElementP);
DGNPLATFORM_EXPORT void       mdlElement_visitLinkages (VisitLinkageFunc, CallbackArgP arg, DgnElementCP);
DGNPLATFORM_EXPORT int        mdlElement_processLinkagesDirect (LinkageProcessFuncP, CallbackArgP pLinkFuncArg, DgnElementP);
DGNPLATFORM_EXPORT int        mdlElement_processLinkagesReadOnly (LinkageProcessFuncP, CallbackArgP pLinkFuncArg, DgnElementP);
DGNPLATFORM_EXPORT StatusInt  linkage_appendToElement (DgnElementP, LinkageHeaderP, void const* dataP, void* convRulesP);
DGNPLATFORM_EXPORT void*      linkage_extractFromElement (void* inLinkBuf, DgnElementCP, int reqID, void* convRulesP, LinkageFunc, CallbackArgP paramsP);
DGNPLATFORM_EXPORT int        linkage_deleteFromElement (DgnElementP, int reqID, void* convRulesP, LinkageFunc, CallbackArgP paramsP);
DGNPLATFORM_EXPORT void*      linkage_padToLinkageSize (void const* data, UInt32 nActualBytesData);
DGNPLATFORM_EXPORT void*      linkage_extractLinkageByIndex (void* inLinkBuf, DgnElementCP, int reqID, void* convRulesP, int nth);
DGNPLATFORM_EXPORT int        linkage_deleteLinkageByIndex (DgnElementP, int reqID, int nth);
DGNPLATFORM_EXPORT int        mdlLinkage_numLinkages (DgnElementP, int reqID);
DGNPLATFORM_EXPORT StatusInt  mdlLinkage_copy (DgnElementP pDestElement, DgnElementCP pSourceElement, int linkageId);
DGNPLATFORM_EXPORT int        elemUtil_appendLinkage (DgnElementP, LinkageHeaderCP);
DGNPLATFORM_EXPORT int        elemUtil_deleteLinkage (DgnElementP, int reqID);
DGNPLATFORM_EXPORT void*      elemUtil_extractLinkage (void* inLinkBuf, T_LinkFunc, DgnElementCP, int reqID);
DGNPLATFORM_EXPORT StatusInt  elemUtil_replaceLinkage (DgnElementP, UShort* pOldLinkage, UShort* pNewLinkage, bool allowSizeChange);

END_BENTLEY_API_NAMESPACE
