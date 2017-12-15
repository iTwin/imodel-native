/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GeometryClipper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

// #define WIP_STROKE_CLIPPING
#if defined(WIP_STROKE_CLIPPING)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClip(StrokesList& strokesOut, StrokesCR strokesIn)
    {
    //if (nullptr != m_clip)
    //if (0)
        {

        ClipPlaneSetCP clipPlanes = nullptr;
        ClipPlaneSetCP maskPlanes = nullptr;
        if (nullptr != m_clip)
            {
            for (ClipPrimitivePtr const& primitive : *m_clip)
                {
                clipPlanes = primitive->GetClipPlanes();
                maskPlanes = primitive->GetMaskPlanes();
                }
            }

        Strokes::PointLists strokePoints;

        DisplayParamsCPtr displayParams = strokesIn.m_displayParams;
        bool isDisjoint = strokesIn.m_disjoint;
        bool isPlanar = strokesIn.m_isPlanar;

        for (auto& strokes : strokesIn.m_strokes)
            {
            bvector<DPoint3d> points;
            bvector<DSegment1d> intervals;
            DSegment3d parentSeg;

            // ###TODO: if isDisjoint, must iterate over points differently
            // m_disjoint = points instead of lines

            if (!isDisjoint)
                {
                for (auto pt = strokes.m_points.begin(); pt != strokes.m_points.end(); /**/)
                    {
                    DPoint3d v0 = *pt;  pt++;
                    DPoint3d v1 = *pt;

                    if (nullptr != clipPlanes)
                        {
                        parentSeg.point[0] = v0;  parentSeg.point[1] = v1;
                        intervals.clear();
                        clipPlanes->AppendIntervals(parentSeg, intervals);
                        }

                    if (!intervals.empty())
                        {
                        for (auto& interval : intervals)
                            {
                            DSegment3d childSeg;  childSeg.FromFractionInterval(parentSeg, interval);
                            //bvector<DPoint3d> childPts;  childPts.push_back(childSeg.point[0]);  childPts.push_back(childSeg.point[1]);
                            bvector<DPoint3d> childPts;  childPts.push_back(v0);  childPts.push_back(v1);
                            strokePoints.push_back(Strokes::PointList(std::move(childPts), strokes.m_rangeCenter));
#if 0 // put into new list
                            points.push_back(childSeg.point[0]);
                            points.push_back(childSeg.point[1]);
#endif
                            }
                        }
    //                else
//                        points.push_back(v0);
                    }
                }

//GEOMDLLIMPEXP void AppendIntervals (DSegment3dCR segment, bvector<DSegment1d> &intervals) const;
#if 0
//!
//! @description Return a segment defined by fractional start and end on a parent segment.
//! @param [in] parent existing segment.
//! @param [in] interval interval withs fractional start and end coordinates
static DSegment3d FromFractionInterval
(
DSegment3dCR parent,
DSegment1dCR interval
);
#endif

            strokePoints.push_back(Strokes::PointList(std::move(points), strokes.m_rangeCenter));
            }

        strokesOut.push_back(Strokes(*displayParams, std::move(strokePoints), isDisjoint, isPlanar));
        }
    //else
    //    strokesOut.push_back(strokesIn);
    }

#else // !defined(WIP_STROKE_CLIPPING)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClip(StrokesList& strokesOut, StrokesCR strokesIn)
    {
    strokesOut.push_back(strokesIn);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClip(PolyfaceList& polyfacesOut, PolyfaceCR polyfaceIn)
    {
    if (nullptr != m_clip)
        {
        PolyfaceClipper pfClipper;

        pfClipper.ClipPolyface(*polyfaceIn.m_polyface, m_clip, true);
        if (pfClipper.HasOutput())
            {
            DisplayParamsCPtr displayParams = polyfaceIn.m_displayParams;
            bool displayEdges = polyfaceIn.m_displayEdges;
            bool isPlanar = polyfaceIn.m_isPlanar;

            bvector<PolyfaceQueryCP>& clippedPolyfaceQueries = pfClipper.GetOutput();
            for (auto& clippedPolyfaceQuery : clippedPolyfaceQueries)
                {
                polyfacesOut.push_back(Polyface(*displayParams, *clippedPolyfaceQuery->Clone(), displayEdges, isPlanar));
                }
            }
        }
    else
        polyfacesOut.push_back(polyfaceIn);
    }

