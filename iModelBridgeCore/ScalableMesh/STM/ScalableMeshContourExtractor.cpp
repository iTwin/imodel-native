/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include "ScalableMeshContourExtractor.h"
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


ScalableMeshContourExtractor::ScalableMeshContourExtractor()
{
}

ScalableMeshContourExtractor::ScalableMeshContourExtractor(ContoursParameters p)
    : m_params(p)
{
}

void ScalableMeshContourExtractor::GetMajorContours(bvector<bvector<DPoint3d>>& contoursMajor, IScalableMeshMeshPtr& meshP, double lowZ, double highZ)
{
    return GetContours(contoursMajor, m_params.majorContourSpacing, meshP, lowZ, highZ);
}

void ScalableMeshContourExtractor::GetMinorContours(bvector<bvector<DPoint3d>>& contoursMinor, IScalableMeshMeshPtr& meshP, double lowZ, double highZ)
{
    return GetContours(contoursMinor, m_params.minorContourSpacing, meshP, lowZ, highZ);
}

void ScalableMeshContourExtractor::GetContours(bvector<bvector<DPoint3d>>& contours, double spacing, IScalableMeshMeshPtr& meshP, double begin, double end)
{
    for (double i = begin; i < end; i += spacing)
    {
        DPlane3d plane = DPlane3d::From3Points(DPoint3d::From(0, 0, i), DPoint3d::From(0, 1, i), DPoint3d::From(1, 0, i));
        bvector<DSegment3d> allSegments;

        meshP->CutWithPlane(allSegments, plane);

        bvector<bvector<DPoint3d>> polylines;
        for (auto& seg : allSegments)
            seg.point[0].z = seg.point[1].z = i;
        StitchSegmentsAtJunctions(polylines, allSegments);

        for (auto& line : polylines)
            contours.push_back(line);
    }
}

END_BENTLEY_SCALABLEMESH_NAMESPACE
