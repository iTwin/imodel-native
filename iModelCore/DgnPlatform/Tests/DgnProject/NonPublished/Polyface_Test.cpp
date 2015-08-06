/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/Polyface_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_DGN

static int s_printCoordinates = 0;

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
struct PolyfaceTest : public GenericDgnModelTestFixture
{
double m_uorScale;
DRange3d m_outputRange;
double m_fringeFraction;
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      01/10
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceTest()
    : GenericDgnModelTestFixture (__FILE__, true)
    {
    m_uorScale = 1000000.0;
    m_fringeFraction = 0.5;
    m_outputRange = DRange3d::From (DPoint3d::From (0,0,0));
    }

Transform GetPlacement (DRange3dCR rawDataRange, bool applyUorScale, TransformP stepTransform = NULL)
    {
    double scale = applyUorScale ? m_uorScale : 1.0;
    double dx = rawDataRange.XLength () * scale;
    DPoint3d targetOrigin = DPoint3d::From(m_outputRange.high.x, m_outputRange.low.y, m_outputRange.low.z);
    targetOrigin.x += dx * m_fringeFraction;
    Transform scaleAboutRawLow = Transform::FromFixedPointAndScaleFactors (rawDataRange.low, scale, scale, scale);
    Transform translation = Transform::From (DVec3d::FromStartEnd (rawDataRange.low, targetOrigin));
    Transform compositeTransform;
    compositeTransform.InitProduct (translation, scaleAboutRawLow);
    DRange3d transformedRange;
    compositeTransform.Multiply (transformedRange, rawDataRange);
    m_outputRange.Extend (transformedRange.low);
    m_outputRange.Extend (transformedRange.high);

    if (NULL != stepTransform)
        {
        double dy = transformedRange.high.y - transformedRange.low.y;
        *stepTransform = Transform::From (0.0, dy, 0.0);
        }
    return compositeTransform;
    }

void AddSegmentToModel (DPoint3dCR point0, DPoint3dCR point1)
    {
    DSegment3d segment = DSegment3d::From (point0, point1);
    CurveVectorPtr parent1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    parent1->push_back (ICurvePrimitive::CreateLine (segment));
    EditElementHandle eeh;
    if (SUCCESS == DraftingElementSchema::ToElement  (eeh, *parent1, NULL, TRUE, *GetDgnModelP ()))
        eeh.AddToModel ();
    }

void AddSegmentsToModel (bvector <DSegment3d> const &segments, TransformCR transform)
    {
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    DRange3d range;
    range.Init ();
    for(DSegment3d segment : segments)
        {
        ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (segment);
        parent->push_back (line);
        range.Extend (segment.point[0]);
        range.Extend (segment.point[1]);
        }
    EditElementHandle eeh;

    if (SUCCESS == DraftingElementSchema::ToElement (eeh, *parent, NULL, TRUE, *GetDgnModelP ()))
        {
        eeh.GetHandler().ApplyTransform (eeh, TransformInfo (transform));
        eeh.AddToModel ();
        }
    else
        {
        for(DSegment3d segment : segments)
            {
            CurveVectorPtr parent1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            parent1->push_back (ICurvePrimitive::CreateLine (segment));
            EditElementHandle eeh;
            if (SUCCESS == DraftingElementSchema::ToElement (eeh, *parent1, NULL, TRUE, *GetDgnModelP ()))
                {
                eeh.GetHandler().ApplyTransform (eeh, TransformInfo (transform));
                eeh.AddToModel ();
                }
            }
        }
    }

void HandleMeshOutput (PolyfaceHeaderR mesh, bool applyPlacementTransform, bool scaleToUors = true)
    {
    DPoint3d origin = mesh.GetPointCP ()[0];
    double area = mesh.SumFacetAreas ();
    bool closed = mesh.IsClosedByEdgePairing ();
    printf ("Mesh Output (origin %lg,%lg,%lf) (closed = %d) (area %lg)\n",
                    origin.x, origin.y, origin.z,
                    closed ? 1 : 0,
                    area);
    size_t numPoint0 = mesh.GetPointCount ();
    size_t numFacet0 = mesh.GetNumFacet ();
    printf ("    (PointCount %d) (Facets %d)\n", (int32_t)numPoint0, (int32_t)numFacet0);

    if (closed)
        {
        double volume = mesh.SumTetrahedralVolumes (origin);
        printf ("    (volume %le)\n", volume);
        }

    if (s_printCoordinates)
        {
        printf ("(FacetCoordinates\n");
        for (size_t i = 0; i < mesh.GetPointCount (); i++)
            {
            DPoint3d xyz = mesh.GetPointCP ()[i];
            printf ("xy=%lf,%lf,%lf\n", xyz.x, xyz.y, xyz.z);
            }
        }
    EditElementHandle meshElement;
    DRange3d rangeA = mesh.PointRange ();
    Transform stepTransform1;
    Transform transformA = applyPlacementTransform
                            ? GetPlacement (rangeA, scaleToUors, &stepTransform1)
                            : Transform::FromIdentity ();

    printf ("     (output range (%g,%g,%g) (%g,%g,%g)\n",
                    rangeA.low.x, rangeA.low.y, rangeA.low.z,
                    rangeA.high.x, rangeA.high.y, rangeA.high.z);

    //Transform edgeTransform = Transform::FromProductOf (stepTransform, transformA);

    mesh.Transform (transformA, false);
    DRange3d rangeB = mesh.PointRange ();
    Transform stepTransform = Transform::From (0.0, 1.2 * (rangeB.high.y - rangeB.low.y), 0.0);
    mesh.Transform (stepTransform, false);
    Transform edgeTransform = stepTransform;
    ASSERT_EQ (SUCCESS, MeshHeaderHandler::CreateMeshElement(meshElement, NULL, mesh, true, *GetDgnModelP ()));
    meshElement.AddToModel ();

    bvector<DSegment3d> segments0, segments1;
    mesh.CollectSegments (segments0, false);

    AddSegmentsToModel (segments0, edgeTransform);
    edgeTransform.InitProduct (stepTransform, edgeTransform);

    mesh.CollectSegments (segments1, true);
    AddSegmentsToModel (segments1, edgeTransform);
    edgeTransform.InitProduct (stepTransform, edgeTransform);

    mesh.Transform (edgeTransform, false);
    mesh.Triangulate ();
    EditElementHandle triangulatedMeshElement;
    ASSERT_EQ (SUCCESS, MeshHeaderHandler::CreateMeshElement(triangulatedMeshElement, NULL, mesh, true, *GetDgnModelP ()));
    triangulatedMeshElement.AddToModel ();
    
    }


void ShowMeshOrientation (PolyfaceHeaderR mesh)
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (mesh, false);
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        DPoint3d centroid, edgePoint;
        DVec3d normal;
        double area;
        if (visitor->TryGetFacetCentroidNormalAndArea (centroid, normal, area)
            && visitor->TryGetEdgePoint (0, 0.25, edgePoint))
            {
            double a = sqrt (area);
            printf ("a=%lf\n", a);

            //AddSegmentToModel, centroid, edgePoint);
            }
        }
    }



void BuildAndMeshLinearSweep (EditElementHandleCR contour, DVec3dCR sweep, bool capped)
    {
    EditElementHandle sweptElement;
    DPoint3d origin = DPoint3d::From (0,0,0);
    ASSERT_EQ (SUCCESS,
            SurfaceOrSolidHandler::CreateProjectionElement (sweptElement, NULL, contour, origin, sweep, NULL, capped, *GetDgnModelP ())
            );

    DRange3d rangeA = *sweptElement.GetIndexRange();
    Transform transformA = GetPlacement (rangeA, true);
    sweptElement.GetHandler().ApplyTransform (sweptElement, TransformInfo (transformA));
    sweptElement.AddToModel ();
    MeshAndOutput (sweptElement, false, false);    
    }


void BuildAndMeshRotationalSweep (EditElementHandleCR contour, DPoint3dCR origin, DVec3dCR axis, double sweep, bool capped)
    {
    EditElementHandle sweptElement;
    ASSERT_EQ (SUCCESS,
            SurfaceOrSolidHandler::CreateRevolutionElement (sweptElement, NULL, contour, origin, axis, sweep, capped, *GetDgnModelP (), 8)
            );
    DRange3d rangeA = *sweptElement.GetIndexRange();
    Transform transformA = GetPlacement (rangeA, true);
    sweptElement.GetHandler().ApplyTransform (sweptElement, TransformInfo (transformA));
    sweptElement.AddToModel ();
    MeshAndOutput (sweptElement, false, false);
    }



void HandleMeshOutput (bvector<PolyfaceHeaderPtr> meshVector, bool applyPlacementTransform, bool scaleToUors)
    {
    for (size_t i = 0; i < meshVector.size (); i++)
        HandleMeshOutput (*meshVector[i], scaleToUors);
    }

void MeshAndOutput (EditElementHandleR source, bool applyPlacementTransform, bool scaleMeshToUors)
    {
    bvector<PolyfaceHeaderPtr> meshVector;
    IMeshQuery::ElementToApproximateFacets (source, meshVector, NULL);
    HandleMeshOutput (meshVector, applyPlacementTransform, scaleMeshToUors);
    }




void SaveFile (WStringCR name)
    {
#if defined (NOT_SUPPORTED_DOESNT_ACCOMPLISH_ANYTHING_EITHER)
    m_tdm->SaveDgn();
    m_tdm->CopyFileTo(name);
#endif
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PolyfaceTest, MeshCone)
    {
    // Did you know ... The area of a 12-gon inscribed in a circle of radius r is 3 r^2?

    DPoint3d centerA = {0,0,0};
    DPoint3d centerB = {0,0,10};
    RotMatrix rMatrix = RotMatrix::FromIdentity();
    double radiusA = 2.0;
    double radiusB = 3.0;
    for (int i = 0; i < 2; i++)
        {
        EditElementHandle  source;
        ASSERT_EQ(SUCCESS, ConeHandler::CreateConeElement(source, NULL, radiusA, radiusB, centerA, centerB, rMatrix, i == 1, *GetDgnModelP ()));
        MeshAndOutput (source, true, true);
        SaveFile (L"MeshCone.dgn");
        }
    }

TEST_F (PolyfaceTest, MeshEllipse)
    {
    EditElementHandle source;
    EllipseHandler::CreateEllipseElement (source, NULL,
                    DPoint3d::From (1,1,1),
                    3.0, 3.0, 0.0, true, *GetDgnModelP());
    MeshAndOutput (source, true, true);
    SaveFile (L"MeshEllipse.dgn");
    }



TEST_F (PolyfaceTest, MeshShape)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From(1,2,0));
    points.push_back (DPoint3d::From(1,1,0));
    points.push_back (DPoint3d::From(2,1,0));
    points.push_back (DPoint3d::From(2,2,0));
    points.push_back (DPoint3d::From (1,2,0));
    CurveVectorPtr contour = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    contour->push_back (ICurvePrimitive::CreateLineString (points));
    EditElementHandle contourElement;
    DraftingElementSchema::ToElement (contourElement, *contour, NULL, true, *GetDgnModelP ());
    MeshAndOutput (contourElement, true, true);

    BuildAndMeshLinearSweep (contourElement, DVec3d::From (0,0,1), true );
    BuildAndMeshLinearSweep (contourElement, DVec3d::From (0,0,1), false);
    
    BuildAndMeshRotationalSweep (contourElement, DPoint3d::From (0,0,0), DVec3d::From (1,0,0), Angle::Pi (), true );
    BuildAndMeshRotationalSweep (contourElement, DPoint3d::From (0,0,0), DVec3d::From (1,0,0), Angle::Pi (), false);

    BuildAndMeshRotationalSweep (contourElement, DPoint3d::From (0,0,0), DVec3d::From (1,-1,0), Angle::PiOver2 (), false);


    SaveFile (L"MeshShape.dgn");
    }
//#include <ElementGeometry/ElementGeometry.h>

TEST_F (PolyfaceTest, Mesh_ComplexShape_HalfCircleOnSquare)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From(1,2,0));
    points.push_back (DPoint3d::From(1,1,0));
    points.push_back (DPoint3d::From(5,1,0));
    points.push_back (DPoint3d::From(5,2,0));
    ICurvePrimitivePtr base = ICurvePrimitive::CreateLineString (points);
    CurveVectorPtr shape = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    shape->push_back (base);
    shape->push_back (ICurvePrimitive::CreateArc (
                DEllipse3d::From (  3, 2, 0,
                                    2, 0, 0,
                                    0, 2, 0,
                                  0.0, Angle::Pi ())));
    EditElementHandle contour;
    DraftingElementSchema::ToElement (contour, *shape, NULL, true, *GetDgnModelP ());
    MeshAndOutput (contour, true, true);

    BuildAndMeshLinearSweep (contour, DVec3d::From (0,0,1), true );
    BuildAndMeshLinearSweep (contour, DVec3d::From (0,0,1), false);
    
    BuildAndMeshRotationalSweep (contour, DPoint3d::From (0,0,0), DVec3d::From (1,0,0), Angle::Pi (), false );
    BuildAndMeshRotationalSweep (contour, DPoint3d::From (0,0,0), DVec3d::From (1,0,0), Angle::Pi (), true);

    BuildAndMeshRotationalSweep (contour, DPoint3d::From (0,0,0), DVec3d::From (1,-1,0), Angle::PiOver2 (), false);


    BuildAndMeshRotationalSweep (contour, DPoint3d::From (1,1,0), DVec3d::From (1,0,0), Angle::PiOver2 (), false);
    BuildAndMeshRotationalSweep (contour, DPoint3d::From (1,1,0), DVec3d::From (1,0,0), Angle::PiOver2 (), true );
    SaveFile (L"ComplexShape_HalfCircleOnSquare.dgn");
    }

size_t CountFaces (PolyfaceHeaderPtr header)
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*header, false);
    size_t numFace = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
        {
        numFace++;
        }
    return numFace;
    }

TEST_F (PolyfaceTest, Grid)
    {
    size_t nx = 3;
    size_t ny = 4;
    PolyfaceHeaderPtr polyface = CreateUnitGrid (nx + 1, ny + 1, false);
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    ASSERT_EQ (nx * ny, CountFaces (polyface));
    ASSERT_EQ (nx * ny, polyface->GetNumFacet ());
    polyface->Triangulate ();
    ASSERT_EQ (2 * nx * ny, polyface->GetNumFacet ());
    }

#endif
