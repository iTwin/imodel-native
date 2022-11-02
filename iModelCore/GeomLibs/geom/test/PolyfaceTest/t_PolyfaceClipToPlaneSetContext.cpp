/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

void AddFacetFaceData(PolyfaceHeaderR mesh, double dx, double dy)
    {
    auto rangeA = DRange2d::From (mesh.Param ());
    auto rangeB = DRange2d::From (0,0,dx,dy);
    FacetFaceData faceData;
    faceData.Init ();
    faceData.m_paramRange = rangeA;
    faceData.m_paramDistanceRange = rangeB;
    mesh.FaceData ().SetActive (true);
    mesh.FaceIndex ().SetActive (true);
    mesh.FaceData ().clear ();
    int faceIndex = 1;      // one based !?
    mesh.FaceIndex ().clear ();
    mesh.FaceData ().push_back (faceData);
    mesh.FaceData().push_back(faceData);

    for (int q : mesh.PointIndex())
        {
        if (q == 0)
            mesh.FaceIndex ().push_back (0);
        else
            mesh.FaceIndex ().push_back (faceIndex);
        }
    }
struct MyClipOutputHandler : PolyfaceQuery::IClipToPlaneSetOutput
{
StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override
    {
    auto p = dynamic_cast <PolyfaceHeaderCP> (&polyfaceQuery);
    if (p != nullptr)
        Check::SaveTransformed (*p); 
    return SUCCESS;
    }
StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override
    {
    Check::SaveTransformed(polyfaceHeader);
    return SUCCESS;
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceClipToPlaneSetContext, ClipToRange)
    {
    PolyfaceHeaderPtr polyface = PolyfaceWithSinusoidalGrid(20, 16, 0.05, 0.05, 0.8, 3, true, true);
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
    AddFacetFaceData(*polyface, 10, 10);
    double creaseRadians = 0.4;
    double shortEdgeTolerance = 0.001;
    polyface->BuildNormalsFast(creaseRadians, shortEdgeTolerance);
    polyface->MarkInvisibleEdges(0.4, nullptr);
    polyface->AddEdgeChains(0);
    auto polyfaceRange = DRange3d::From (polyface->Point ());
    auto range1 = DRange3d::From (
        polyfaceRange.LocalToGlobal (0.21231212, 0.29, -1),
        polyfaceRange.LocalToGlobal(0.73564, 1.243524, 0.9));
    auto range2 = DRange3d::From(
        polyfaceRange.LocalToGlobal(-0.2, -0.2, -1),
        polyfaceRange.LocalToGlobal(1.1, 1.2, 2));
    MyClipOutputHandler handler;
    for (auto range : { range1, range2 })
        {
        SaveAndRestoreCheckTransform (range.XLength () * 2.0, 0, 0);
        Check::SaveTransformed(*polyface);
        Check::SaveTransformedEdges(range);
        Check::Shift(0, 2.0 * polyfaceRange.YLength(), 0);
        Check::SaveTransformedEdges(range);
        polyface->PolyfaceQuery::ClipToRange(range, handler, true);
        }

    PolyfaceHeaderPtr emptyPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
    Check::True(SUCCESS == emptyPolyface->ClipToRange(range1, handler, true), "Clip empty polyface succeeds");
    DRange3d nullRange = DRange3d::NullRange();
    Check::True(SUCCESS == polyface->ClipToRange(nullRange, handler, true), "Clip polyface to null range succeeds");

    Check::ClearGeometry ("PolyfaceClipToPlaneSetContext.ClipToRange");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceClipToPlaneSetContext, ClipToIntersection)
    {
    PolyfaceHeaderPtr polyface = PolyfaceWithSinusoidalGrid(20, 16, 0.05, 0.05, 0.8, 3, true, true);
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
    AddFacetFaceData (*polyface, 10,10);
    double creaseRadians = 0.4;
    double shortEdgeTolerance = 0.001;
    polyface->BuildNormalsFast(creaseRadians, shortEdgeTolerance);
    polyface->MarkInvisibleEdges(0.4, nullptr);
    polyface->AddEdgeChains(0);
    auto polyfaceRange = DRange3d::From(polyface->Point());
    DPoint3d centerA = polyfaceRange.LocalToGlobal (0.6, 0.6, 0.5);
    DPoint3d centerB = polyfaceRange.LocalToGlobal(0.5, 0.5, 0.5);
    DPlane3d planeA0 = DPlane3d::FromOriginAndNormal(centerA, DVec3d::From (-1,0,0));
    DPlane3d planeA1 = DPlane3d::FromOriginAndNormal(centerA, DVec3d::From(0, -1, 0));
    double a = sqrt (0.5);
    DPlane3d planeB = DPlane3d::FromOriginAndNormal(centerB, DVec3d::From (a,a,0));
    ConvexClipPlaneSet convexSetA0, convexSetA1;
    ConvexClipPlaneSet convexSetB;
    convexSetA0.push_back(ClipPlane(planeA0));
    convexSetA1.push_back(ClipPlane(planeA1));
    convexSetB.push_back(ClipPlane(planeB));

    ClipPlaneSet setA, setB;
    setA.push_back (convexSetA0);
    setA.push_back(convexSetA1);
    setB.push_back(convexSetB);

    T_ClipPlaneSets manySets;
    manySets.push_back (setA);
    manySets.push_back (setB);
    MyClipOutputHandler handler;
    Check::SaveTransformed(*polyface);
    Check::Shift(0, 2.0 * polyfaceRange.YLength(), 0);
    polyface->PolyfaceQuery::ClipToPlaneSetIntersection(manySets, handler, true);

    PolyfaceHeaderPtr emptyPolyface = PolyfaceHeader::CreateVariableSizeIndexed();
    Check::True(SUCCESS == emptyPolyface->ClipToPlaneSetIntersection(manySets, handler, true), "Clip empty polyface succeeds");
    T_ClipPlaneSets nullSets;
    Check::True(SUCCESS == polyface->ClipToPlaneSetIntersection(nullSets, handler, true), "Clip polyface to empty ClipPlaneSets succeeds");

    Check::ClearGeometry("PolyfaceClipToPlaneSetContext.ClipToIntersection");
    }
