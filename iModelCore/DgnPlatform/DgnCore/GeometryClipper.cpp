/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometryClipper::SumIntervals(bvector<DSegment1d> &intervals, size_t iBegin, size_t iEnd)
    {
    double s = 0.0;
    for (size_t i = iBegin; i < iEnd; i++)
        s += intervals[i].Delta();
    return s;
    }

#define TARGET_INTERVAL_SUM (0.99999999)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::ProcessSegments(bvector<DSegment3d>& segsUnclippedOut, bvector<DSegment3d>& segsClippedOut, const bvector<DSegment3d>& segsIn)
    {
    DSegment3d clipSeg;
    bvector<DSegment1d> clipIntervals;

    for (auto seg : segsIn) // clip each segment against each clipPrimitive
        {
        bool isClipped = false;

        if (seg.IsAlmostSinglePoint()) // avoid degenerate cases - don't even try to clip almost zero-length segments
            {
            isClipped = true; // don't draw them either - they are almost zero-length, so this is a valid response.
            }
        else
            {
            for (ClipPrimitivePtr const& clipPrimitive : *m_clip)
                {
                auto clipPlaneSet = clipPrimitive->GetClipPlanes();

                clipIntervals.clear();
                clipPlaneSet->AppendIntervals(seg, clipIntervals); // find intervals for current segment (intervals = resulting segments from clip)

                // if fractional intervals sum to less than roughly 1, we know that the line has been made shorter (clipped).
                if (SumIntervals(clipIntervals, 0, clipIntervals.size()) < TARGET_INTERVAL_SUM)
                    {
                    for (auto clipInterval : clipIntervals)
                        {
                        clipSeg = DSegment3d::FromFractionInterval(seg, clipInterval);
                        segsClippedOut.push_back(clipSeg); // needs potential further clipping (against other planes)
                        }
                    isClipped = true;
                    break;
                    }
                }
            }

        if (!isClipped)
            {
            segsUnclippedOut.push_back(seg); // guaranteed needs no further clipping (ready for final output, checked against all primitives)
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::ProcessLine(Strokes::PointLists& pointListsOut, const Strokes::PointList& pointListIn)
    {
    DSegment3d clipSeg;
    bvector<DSegment3d> clipSegsFinal;
    bvector<DSegment3d> clipSegsTemp0;
    bvector<DSegment3d> clipSegsTemp1;

    for (size_t i = 0; i + 1 < pointListIn.m_points.size(); i++)
        {
        clipSeg.Init(pointListIn.m_points[i], pointListIn.m_points[i + 1]);
        clipSegsTemp0.push_back(clipSeg);
        }

    ProcessSegments(clipSegsFinal, clipSegsTemp1, clipSegsTemp0);

    while (!clipSegsTemp1.empty())
        {
        clipSegsTemp0.clear();
        ProcessSegments(clipSegsFinal, clipSegsTemp0, clipSegsTemp1);
        clipSegsTemp1 = std::move(clipSegsTemp0);
        }

    bvector<DPoint3d> clipPts;
    bool canDecimate = pointListIn.m_canDecimate;
    for (auto seg : clipSegsFinal)
        {
        if (clipPts.empty())
            { // always add first point in a new pointList (clipPts is empty)
            clipPts.push_back(seg.point[0]);
            }
        else
            {
            if (!clipPts.back().AlmostEqual(seg.point[0]))
                { // submit previous pointList because points mismatch (will begin new pointList)
                pointListsOut.push_back(Strokes::PointList(std::move(clipPts), canDecimate));
                clipPts.push_back(seg.point[0]); // add first point to new pointList
                }
            // else AlmostEqual() and will skip outputting seg.point[0] in order to continue previous line string
            }

        clipPts.push_back(seg.point[1]);
        }

    if (!clipPts.empty())
        { // submit final pointList, if available
        pointListsOut.push_back(Strokes::PointList(std::move(clipPts), canDecimate));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClipPoints(bvector<DPoint3d>& pointsOut, const bvector<DPoint3d>& pointsIn)
    {
    if (nullptr != m_clip)
        {
        for (auto ptIn : pointsIn)
            {
            if (m_clip->PointInside(ptIn))
                {
                pointsOut.push_back(ptIn);
                }
            }
        }
    else
        {
        for (auto ptIn : pointsIn)
            {
            pointsOut.push_back(ptIn);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClipStrokes(StrokesList& strokesOut, Strokes&& strokesIn)
    {
    if (nullptr != m_clip)
        {
        Strokes::PointLists newStrokePts;

        DisplayParamsCPtr displayParams = strokesIn.m_displayParams;
        bool isDisjoint = strokesIn.m_disjoint;
        bool isPlanar = strokesIn.m_isPlanar;
        double decimationTolerance = strokesIn.m_decimationTolerance;

        if (isDisjoint) // clip as individual points
            {
            bvector<DPoint3d> newPts;

            for (auto& strokePts : strokesIn.m_strokes)
                {
                DoClipPoints(newPts, strokePts.m_points);

                if (!newPts.empty())
                    {
                    newStrokePts.emplace_back(Strokes::PointList(std::move(newPts), false));
                    newPts.clear();
                    }
                }

            if (!newStrokePts.empty())
                strokesOut.emplace_back(Strokes(*displayParams, std::move(newStrokePts), isDisjoint, isPlanar, decimationTolerance));
            }
        else // clip as line strings
            {
            for (auto& strokePts : strokesIn.m_strokes)
                {
                ProcessLine(newStrokePts, strokePts);
                }

            if (!newStrokePts.empty())
                strokesOut.emplace_back(Strokes(*displayParams, std::move(newStrokePts), isDisjoint, isPlanar, decimationTolerance));
            }
        }
    else
        {
        strokesOut.emplace_back(std::move(strokesIn));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryClipper::DoClipPolyface(PolyfaceList& polyfacesOut, PolyfaceCR polyfaceIn, bool clipRasterText)
    {
    // Raster text clipping is deferred because UV params must be fixed up after polyface collection.
    if (nullptr != m_clip && (clipRasterText || nullptr == polyfaceIn.GetGlyphImage()))
        {
        PolyfaceClipper pfClipper;

        pfClipper.ClipPolyface(polyfaceIn.GetPolyface(), m_clip, true);
        if (pfClipper.HasOutput())
            {
            bvector<PolyfaceQueryCP>& clippedPolyfaceQueries = pfClipper.GetOutput();
            for (auto& clippedPolyfaceQuery : clippedPolyfaceQueries)
                polyfacesOut.push_back(polyfaceIn.Clone(*clippedPolyfaceQuery->Clone()));
            }
        }
    else
        {
        polyfacesOut.push_back(polyfaceIn);
        }
    }

