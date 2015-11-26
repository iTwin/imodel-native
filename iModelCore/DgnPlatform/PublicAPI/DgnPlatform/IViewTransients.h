/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/IViewTransients.h $
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
struct IViewTransients
{
//__PUBLISH_SECTION_END__
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

}; // IViewTransients

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
