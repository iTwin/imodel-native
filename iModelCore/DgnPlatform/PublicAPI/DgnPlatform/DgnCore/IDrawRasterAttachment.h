/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IDrawRasterAttachment.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface adopted by classes to allow display raster attachment.
// @bsiclass                                                     Stephane.Poulin     03/04
//=======================================================================================
struct     IDrawRasterAttachment
{
    //  IDrawRasterAttachment
    virtual StatusInt PreUpdate         (UpdateContextP pContext, DgnModelListP includeList, bool useUpdateSequence, bool includeRefs) = 0;
    virtual StatusInt PostUpdate        (UpdateContextP pContext, DgnModelListP includeList, bool useUpdateSequence, bool includeRefs) = 0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
