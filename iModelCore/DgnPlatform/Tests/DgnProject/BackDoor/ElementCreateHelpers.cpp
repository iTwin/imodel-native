/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/ElementCreateHelpers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnProject/ElementCreateHelpers.h>
#include <UnitTests/BackDoor/DgnProject/DgnElementHelpers.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateGrid (bvector<DPoint3d> &points, size_t rowSize, bool triangulate)
    {
    PolyfaceHeaderPtr header0 = PolyfaceHeader::New ();
    header0->SetMeshStyle ( triangulate ? MESH_ELM_STYLE_TRIANGLE_GRID : MESH_ELM_STYLE_QUAD_GRID);
    header0->SetTwoSided (false);
    header0->SetNumPerRow ((UInt32)rowSize);
 
    header0->Point().SetActive (true);
    header0->PointIndex().SetActive (true);

    for (size_t i = 0; i < points.size (); i++)
            header0->Point ().push_back (points[i]);
    header0->SetNumPerFace (triangulate ? 3 : 4);
    header0->Point ().SetStructsPerRow ((UInt32)rowSize);
    return header0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateUnitGrid (size_t numXPoint, size_t numYPoint, bool triangulate)
    {
    bvector<DPoint3d>points;
    for (size_t iy = 0; iy < numYPoint; iy++)
        {
        for (size_t ix = 0; ix < numXPoint; ix++)
            {
            points.push_back (DPoint3d::From ((double)ix, (double)iy, 0.0));
            }
        }
    return CreateGrid (points, numXPoint, triangulate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
/* Create a polyface mesh for a triangulated unit square from (1,1) to (2,2), with the diagonal hidden. */
PolyfaceHeaderPtr CreateMesh00 (EditElementHandleR eeh, DgnModelR model)
    {
    PolyfaceHeaderPtr header0 = PolyfaceHeader::New ();
    header0->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    header0->SetTwoSided (false);
    header0->SetNumPerFace (0);
    DPoint3d points[] =
        {
        {1,1,0},
        {2,1,0},
        {2,2,0},
        {1,2,0},
        };
    int indices [] =
        {
        1,2,-3,0,
        3,4,-1,0
        };

    header0->Point().SetActive (true);
    header0->PointIndex().SetActive (true);
    for (int i = 0; i < _countof (indices); i++)
        header0->PointIndex().push_back (indices[i]);

    for (int i = 0; i < _countof (points); i++)
        header0->Point ().push_back (points[i]);
    return header0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatchedIndexVectors (BlockedVectorIntR vector0, BlockedVectorIntR vector1)
    {
    size_t n0 = vector0.size ();
    size_t n1 = vector1.size ();
    int errors = 0;

    if (vector0.Active () != vector1.Active ())
        errors++;

    if (n0 != n1)
        {
        errors ++;
        }
    else
        {
        for (size_t i = 0; i < n0; i++)
            {
            if (vector0[i] != vector1[i])
                errors++;
            }
        }
    return errors == 0;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellSetUp (EditElementHandleR m_eeh, DgnModelR model, bool is3d)
    {
    DPoint3d origin[] = {0.0, 0.0, 0.0};
    DPoint3d scale[] = {1.0, 1.0, 1.0};

    RotMatrix       rMatrix;

    rMatrix.InitIdentity ();

    SharedCellHandler::CreateSharedCellElement (m_eeh, NULL, L"test", origin, &rMatrix, scale, is3d, model);
    SharedCellHandler::CreateSharedCellComplete (m_eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateDef (DgnModelR model, bool is3d)
    {
    size_t const NUM_LINES = 3;
    size_t const NUM_POINTS = 2;

    EditElementHandle defHandle;
    EditElementHandle Lines[NUM_LINES];
    DPoint3d            m_points[NUM_LINES][NUM_POINTS];

    for (size_t i = 0; i < NUM_LINES; i++)
        {
        GeneratePoints (m_points[i], NUM_POINTS);
        for (size_t j = 0; j < NUM_POINTS; j++)
            {
            m_points[i][j].x *= i+7;
            m_points[i][j].y *= i+5;
            m_points[i][j].z *= i+6;
            }
        }

    SharedCellDefHandler::CreateSharedCellDefElement (defHandle, L"test", is3d, model);

    for (size_t j = 0; j < NUM_LINES; j++)
        {
        DSegment3d  segment;
        if (j +1 < NUM_LINES)
            segment.Init (*m_points[j], *m_points[j+1]);
        else
            segment.Init (*m_points[j], *m_points[j]);

        LineHandler::CreateLineElement (Lines[j], NULL, segment, is3d, model);
        ASSERT_EQ (SUCCESS, SharedCellDefHandler::AddChildElement (defHandle, Lines[j]));
        }

    SharedCellDefHandler::AddChildComplete (defHandle);
    defHandle.AddToModel (/*&model*/);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void InitPoints (DPoint3dP points, size_t size)
    {
    double const SCALE = 100.0;
    for (size_t i = 0; i < size; i++)
        {
        points[i].x = i*SCALE;
        points[i].y = i*SCALE;
        points[i].z = i*SCALE;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateElement(DPT_CreateElement::ElementType type, EditElementHandleR eeh, DgnModelR model, bool is3d)
    {
    if(type == DPT_CreateElement::Line)
        {
        static size_t const NV = 2;
        DSegment3d  segment;

        InitPoints (segment.point, NV);
        EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (eeh, NULL, segment, is3d, model));
        }
    if(type == DPT_CreateElement::Cone)
        {
        DPoint3d top = {0, 10, 0};
        DPoint3d bottom = {0, 0, 0};

        double const TOP_RADIUS = 5.0;
        double const BOTTOM_RADIUS = 8.0;

        RotMatrix           rMatrix;
        rMatrix.InitIdentity ();

        ASSERT_EQ(SUCCESS, ConeHandler::CreateConeElement(eeh, NULL, TOP_RADIUS, BOTTOM_RADIUS, top, bottom, rMatrix, is3d, model));
        }
    if(type == DPT_CreateElement::Curve)
        {
        static size_t const NV = 6;
        DPoint3d            m_points [NV];
        InitPoints(m_points, NV);
        ASSERT_EQ (SUCCESS, CurveHandler::CreateCurveElement (eeh, NULL, m_points, NV, is3d, model));
        }
    if(type == DPT_CreateElement::nArc)
        {
        DPoint3d point[] = {0, 0, 0};

        double start = 0;
        double sweep = PI/6;

        ASSERT_EQ(SUCCESS, ArcHandler::CreateArcElement(eeh, NULL, *point, PI/4, PI/5, 0, start, sweep, is3d, model));
        }
    if(type == DPT_CreateElement::nEllipse)
        {
        DPoint3d point[] = {0, 0, 0};
        double axis1 = PI/4;
        double axis2 = PI/6;
        double rotAngle = PI/8;

        ASSERT_EQ(SUCCESS, EllipseHandler::CreateEllipseElement(eeh, NULL, *point, axis1, axis2, rotAngle, is3d, model));
        }
    if(type == DPT_CreateElement::LineString)
        {
        static size_t const NV = 5;
        DPoint3d        points[NV];
        GeneratePoints(points, NV);

        ASSERT_EQ(SUCCESS, LineStringHandler::CreateLineStringElement(eeh, NULL, points, NV, is3d, model));
        }
    if(type == DPT_CreateElement::PointString)
        {
        static size_t const NV = 2;
        DPoint3d        points[NV];
        GeneratePoints(points, NV);

        ASSERT_EQ(SUCCESS, PointStringHandler::CreatePointStringElement(eeh, NULL, points, NULL, NV, true, false, model));
        }
    if(type == DPT_CreateElement::Mesh)
        {
        size_t errors = 0;
        PolyfaceHeaderPtr header0 = CreateMesh00 (eeh, model);
        ASSERT_TRUE (header0 != NULL);

        /*StatusInt stat1 =*/ MeshHeaderHandler::CreateMeshElement (eeh, NULL, *header0, true, model);

        PolyfaceHeaderPtr header1;
        if (SUCCESS == BackDoor::MeshHeaderHandler::PolyfaceFromElement (header1, eeh))
            {
            bool a =  (MatchedIndexVectors (header0->PointIndex (), header1->PointIndex()));
            ASSERT_TRUE (a);
            PolyfaceVisitorPtr visitor0 = PolyfaceVisitor::Attach (*header0.get ());
            PolyfaceVisitorPtr visitor1 = PolyfaceVisitor::Attach (*header1.get ());
            for (visitor0->Reset (), visitor1->Reset ();
                visitor0->AdvanceToNextFace () && visitor1->AdvanceToNextFace ();
                )
                {
                size_t n0 = visitor0->Point ().size ();
                size_t n1 = visitor1->Point ().size ();
                if (n0 != n1)
                    errors++;
                else
                    {
                    for (size_t i = 0; i < n0; i++)
                        {
                        if (visitor0->ClientPointIndex()[i] != visitor1->ClientPointIndex()[i])
                            errors++;
                        }
                    }
                }
            ASSERT_TRUE (errors == 0);
            ASSERT_FALSE (visitor0->AdvanceToNextFace ());
            ASSERT_FALSE (visitor1->AdvanceToNextFace ());
                }
            }
        if (type == DPT_CreateElement::nSharedCell)
            {
            CreateDef(model, is3d);
            SharedCellSetUp(eeh, model, is3d);
            }
        if(type == DPT_CreateElement::nShape)
            {
            size_t const NUM_POINTS = 20;

            DPoint3d        points[NUM_POINTS];
            InitPoints(points, NUM_POINTS);

            ASSERT_EQ (SUCCESS, ShapeHandler::CreateShapeElement (eeh, NULL, points, NUM_POINTS, is3d, model));
            }
        if(type == DPT_CreateElement::Complex)
            {
            ChainHeaderHandler::CreateChainHeaderElement (eeh, NULL, true, is3d, model);
            static size_t const NV = 2;
            EditElementHandle line;
            DSegment3d  segment;

            InitPoints (segment.point, NV);
            EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (line, NULL, segment, is3d, model));
            ChainHeaderHandler::AddComponentElement (eeh, line);
            ChainHeaderHandler::AddComponentComplete(eeh);
            line.Invalidate();
            }
        if(type == DPT_CreateElement::BSpline)
            {
            MSBsplineCurve spline;
            size_t const NUM_POINTS = 4;

            DPoint3d        poles[NUM_POINTS];
            InitPoints(poles, NUM_POINTS);

            spline.InitFromPoints (poles, _countof(poles));
            ASSERT_EQ (SUCCESS, BSplineCurveHandler::CreateBSplineCurveElement (eeh, NULL, spline, is3d, model));
            spline.ReleaseMem();
            }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void isPerp(DVec3dR a, DPoint3dR b)
    {
        ASSERT_EQ(0, (a.x*b.x)+(a.y*b.y)+(a.z*b.z));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PushIndices (bvector<int>&dest, int *indices, size_t count, bool padToMaxFaceSize)
    {
    size_t numPerFace = 0;
    if (padToMaxFaceSize)
        {
        // Copying terminated to padded
        size_t maxCount = 0;
        size_t currCount = 0;
        for (size_t i = 0; i < count; i++)
            {
            if (indices[i] == 0)
                {
                if (currCount > maxCount)
                    maxCount = currCount;
                currCount = 0;
                }
            else
                currCount++;
            }
        numPerFace = maxCount;

        currCount = 0;
        for (size_t i = 0; i < count; i++)
            {
            if (indices[i] == 0)
                {
                while (currCount++ < maxCount)
                    dest.push_back (0);
                currCount = 0;
                }
            else
                {
                currCount++;
                dest.push_back (indices[i]);
                }
            }
        }
    else
        {
        // Copying terminated-to-terminated
        for (size_t i = 0; i < count; i++)
            dest.push_back (indices[i]);
        numPerFace = 0;
        }
    return numPerFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateCube (bool variableSize, double x0, double y0, double z0, double a, double b, double c)
    {
    PolyfaceHeaderPtr header0 = PolyfaceHeader::New ();
    header0->SetMeshStyle(MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    header0->SetTwoSided (false);

    double x1 = x0 + a;
    double y1 = y0 + b;
    double z1 = z0 + c;
    DPoint3d points[] =
        {
        {x0,y0,z0},
        {x1,y0,z0},
        {x0,y1,z0},
        {x1,y1,z0},
        {x0,y0,z1},
        {x1,y0,z1},
        {x0,y1,z1},
        {x1,y1,z1},
        };

    int indices [] =
        {
        2,1,3,4,0,
        5,6,8,7,0,
        2,4,8,6,0,
        1,5,7,3,0,
        6,5,1,2,0,
        8,4,3,7,0,
        };

    header0->Point().SetActive (true);
    header0->PointIndex().SetActive (true);
    for (size_t i = 0; i < _countof (points); i++)
            header0->Point ().push_back (points[i]);
    UInt32 numPerFace = (UInt32) PushIndices (header0->PointIndex (), indices, _countof (indices), variableSize);
    header0->SetNumPerFace ((UInt32)numPerFace);
    header0->PointIndex ().SetStructsPerRow (numPerFace <= 1 ? 1 : (UInt32)numPerFace);
    return header0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateSweep (DPoint3d basePoints[], int numBasePoints, DVec3dR sweepVector)
    {
    PolyfaceHeaderPtr header0 = PolyfaceHeader::New ();
    header0->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
    header0->SetTwoSided (false);

    DPoint3d points[1000];
    int indices [1000];

    for (int i = 0; i < numBasePoints; i++)
        points[i] = basePoints[i];
    for (int i = 0; i < numBasePoints; i++)
        points[i + numBasePoints].SumOf (points[i], sweepVector);
    size_t numPoints = (size_t)(2 * numBasePoints);
    int m = 0;
    // Base polygon in reverse order
    for (int i = 0; i < numBasePoints; i++)
        indices[m++] = numBasePoints - i;
    indices[m++] = 0;
    // top as is ..
    for (int i = 0; i < numBasePoints; i++)
        indices[m++] = i + numBasePoints + 1;
    indices[m++] = 0;
    // sides
    for (int i = 0; i < numBasePoints; i++)
        {
        int j = (i + 1) % numBasePoints;
        indices[m++] = 1 + i;
        indices[m++] = 1 + j;
        indices[m++] = 1 + j + numBasePoints;
        indices[m++] = 1 + i + numBasePoints;
        indices[m++] = 0;
        }

    header0->Point().SetActive (true);
    header0->PointIndex().SetActive (true);
    //BackDoor::GeomLibs::DPoint3dOps::Copy (&header0->Point (), points, numPoints);
    //header0->AddPolygon(points, numPoints);
    for (size_t i = 0; i < numPoints; i++)
            header0->Point ().push_back (points[i]);
    size_t numPerFace = PushIndices (header0->PointIndex (), indices, m, true);
    header0->SetNumPerFace ((UInt32)numPerFace);
    header0->PointIndex ().SetStructsPerRow (numPerFace <= 1 ? 1 : (UInt32)numPerFace);
    return header0;
    }

END_DGNDB_UNIT_TESTS_NAMESPACE
