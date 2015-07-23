/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IViewTransients.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
friend struct ViewController;
friend struct ViewTransientCaller;
friend struct RedrawElems;

//__PUBLISH_SECTION_START__
protected:

//! Override this method to draw the transient graphics.
//! @param context The ViewContext in which to draw. 
//! @param isPreUpdate true when called before drawing elements (underlay).
//! @note You must set the current symbology before drawing any geometry.
//! @see ViewContext::GetIDrawGeom
virtual void _DrawTransients (ViewContextR context, bool isPreUpdate) = 0;

//! Override this method to draw the HitDetail resulting from a locate of the transient graphics.
//! @param context The ViewContext in which to draw. 
//! @param hit The transient graphics HitDetail.
//! @note You must set the current symbology before drawing any geometry.
//! @see ViewContext::GetIDrawGeom
virtual void _DrawHitTransients (ViewContextR context, HitDetailCR hit) {_DrawTransients(context, false);}

public:

//! Computes the range of the transients by drawing them. A viewport must be supplied,
//! in order to specify the rotation and what transients are displayed and/or visible.
//! @param[out] range The computed range.
//! @param[in] vp The view in which transients are displayed.
DGNPLATFORM_EXPORT StatusInt ComputeRange (DRange3d& range, DgnViewportP vp);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
