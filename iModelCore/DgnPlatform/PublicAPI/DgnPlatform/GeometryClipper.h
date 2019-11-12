/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
private:
    ClipVectorCP m_clip;

    DGNPLATFORM_EXPORT static double SumIntervals(bvector<DSegment1d> &intervals, size_t iBegin, size_t iEnd);
    DGNPLATFORM_EXPORT void ProcessSegments(bvector<DSegment3d>& segsUnclippedOut, bvector<DSegment3d>& segsClippedOut, const bvector<DSegment3d>& segsIn);
    DGNPLATFORM_EXPORT void ProcessLine(Strokes::PointLists& pointListsOut, const Strokes::PointList& pointListIn);

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
        bool HasClipped() const { return !m_clipped.empty(); }
        bvector<PolyfaceQueryCP>& GetOutput() { return m_output; }
        bvector<PolyfaceHeaderPtr>& GetClipped() { return m_clipped; }
    };

    GeometryClipper(ClipVectorCP clip) : m_clip(clip) { }

    DGNPLATFORM_EXPORT void DoClipPoints(bvector<DPoint3d>& pointsOut, const bvector<DPoint3d>& pointsIn);
    DGNPLATFORM_EXPORT void DoClipStrokes(StrokesList& strokesOut, Strokes&& strokesIn);
    DGNPLATFORM_EXPORT void DoClipPolyface(PolyfaceList& polyfacesOut, PolyfaceCR polyfaceIn);
}; // GeometryClipper

END_BENTLEY_RENDER_PRIMITIVES_NAMESPACE

