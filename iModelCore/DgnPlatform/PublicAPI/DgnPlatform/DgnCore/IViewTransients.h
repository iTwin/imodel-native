/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IViewTransients.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
DGNPLATFORM_TYPEDEFS (RedrawElems)
DGNPLATFORM_TYPEDEFS (ViewTransientCaller)

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct     IViewTransients
{
//__PUBLISH_SECTION_END__
friend struct ViewTransientCaller;
friend struct RedrawElems;

//__PUBLISH_SECTION_START__
protected:

//! Override this method to draw the transient graphics.
//! @param context      The viewcontext in which to draw. 
//! @param isPreUpdate  WIP_DOC
//! @see ViewContext::GetIDrawGeom
//! @bsimethod
virtual void _DrawTransients (ViewContextR context, bool isPreUpdate) = 0;

public:

//! Computes the range of the transients by drawing them. A viewport must be supplied,
//! in order to specify the rotation and what transients are displayed and/or visible.
//! @param[out]         range       The computed range.
//! @param[in]          vp          The view in which transients are displayed.
//! @bsimethod
DGNPLATFORM_EXPORT StatusInt ComputeRange (DRange3d& range, ViewportP vp);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
