/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IDgnOleDraw.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include    <DgnPlatform/DgnCore/ViewContext.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnOleInfo;

// WIP_NONPORT - don't use this Windows-specific data type
#ifndef _WINDEF_
typedef struct _RECTL {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} RECTL, *PRECTL;
#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IDgnOleDraw
{
    virtual RECTL const&        GetObjRect()  const = 0;
    virtual RECTL const&        GetDrawRect() const = 0;
    virtual DPoint3dCP          GetShapePts() const = 0;
    virtual TransformCR         GetTexelToViewTransform() const = 0;
    virtual DgnOleInfo const&   GetDgnOleInfo() const = 0;
    virtual void                RenderToBitmap () = 0;
    virtual bool                IsTransparentBackground () const = 0;
    virtual ~IDgnOleDraw() {}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
