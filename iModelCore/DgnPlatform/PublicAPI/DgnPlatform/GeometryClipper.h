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

BEGIN_BENTLEY_RENDER_PRIMITIVES_NAMESPACE

//=======================================================================================
//! Clip various types of geometry for output to renderer. 
// @bsiclass                                                     Mark.Schlosser  12/2017
//=======================================================================================
struct GeometryClipper
{
protected:
    ClipVectorCP m_clip;

public:
    struct PolyfaceClipper : PolyfaceQuery::IClipToPlaneSetOutput
    {
    private:
        bvector<PolyfaceQueryCP> m_output;
        bvector<PolyfaceHeaderPtr> m_clipped;

        StatusInt _ProcessUnclippedPolyface(PolyfaceQueryCR mesh) override { m_output.push_back(&mesh);  return SUCCESS; }
        StatusInt _ProcessClippedPolyface(PolyfaceHeaderR mesh) override { m_output.push_back(&mesh);  m_clipped.push_back(&mesh);  return SUCCESS; }

    public:
        void ClipPolyface(PolyfaceQueryCR mesh, ClipVectorCP clip, bool triangulate) { clip->ClipPolyface(mesh, *this, triangulate); }
        bool HasOutput() const { return !m_output.empty(); }
        bvector<PolyfaceQueryCP>& GetOutput() { return m_output; }
    };

    GeometryClipper(ClipVectorCP clip) : m_clip(clip) { }

    DGNPLATFORM_EXPORT void DoClip(StrokesList& strokesOut, StrokesCR strokesIn);
    DGNPLATFORM_EXPORT void DoClip(PolyfaceList& polyfacesOut, PolyfaceCR polyfaceIn);
}; // GeometryClipper

END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE

