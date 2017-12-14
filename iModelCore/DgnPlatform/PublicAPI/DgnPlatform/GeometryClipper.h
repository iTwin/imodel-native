/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeometryClipper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/RenderPrimitives.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Clip various types of geometry for output to renderer. 
// @bsiclass                                                     Mark.Schlosser  12/2017
//=======================================================================================
struct GeometryClipper
{
protected:
    ClipVectorCP m_clip;

public:
    GeometryClipper(ClipVectorCP clip) : m_clip(clip) { }

    DGNPLATFORM_EXPORT void DoClip(bvector<BentleyApi::Dgn::Render::Primitives::Strokes>& strokesOut, BentleyApi::Dgn::Render::Primitives::StrokesCR strokesIn);
    DGNPLATFORM_EXPORT void DoClip(bvector<BentleyApi::Dgn::Render::Primitives::Polyface>& polyfacesOut, BentleyApi::Dgn::Render::Primitives::PolyfaceCR polyfaceIn);
}; // GeometryClipper

END_BENTLEY_DGN_NAMESPACE
