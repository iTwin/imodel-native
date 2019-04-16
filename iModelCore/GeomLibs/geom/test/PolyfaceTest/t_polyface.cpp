/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

static bool s_printHullSteps = false;
static int s_noisy = false;
// Callers ask for a transform and a size.
// Callers build geometry within the size, at origin.
// Transform moves the geometry to the assigned origin.
static double sGridStep = 10.0;
static double sCallerSize = 4.0;
static size_t  sGridRow = 0;
static size_t  sGridColumn = 0;
static size_t  sGridCounter = 3;
static DPoint3d sGridLocalOrigin;
#if defined (BENTLEY_WIN32_useOpenGeomTestOutputFile)
static double sGridTextSize = 0.4;
#endif
static size_t s_maxPolyfacePrint = 2;

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP void jmdlMTGGraph_printLoopCounts (MTGGraph  *pGraph);
END_BENTLEY_GEOMETRY_NAMESPACE

static void StartNewGridRow ()
    {
    sGridRow++;
    sGridColumn = 0;
    }

//! @return size for caller to apply to its constructions.
static double SetTransformToNewGridSpot (IPolyfaceConstruction &builder, bool newRow = false)
    {
    if (newRow)
        StartNewGridRow ();
    else
        sGridColumn++;

    double x0 = sGridColumn * sGridStep;
    double y0 = sGridRow    * sGridStep;

    sGridLocalOrigin = DPoint3d::FromXYZ (x0, y0, 0.0);
    Transform transform = Transform::FromRowValues (1,0,0,x0,   0,1,0,y0, 0,0,1, 0.0);
    builder.SetLocalToWorld (transform);
    sGridCounter++;
    return sCallerSize;
    }

#ifdef buildAllLocals
static void ShiftLocalOrigin (IPolyfaceConstruction &builder, double dx, double dy, double dz)
    {
    builder.ApplyLocalToWorld (Transform::From (dx, dy, dz));
    }
#endif

void PrintAttribute (char const* name, size_t value)
    {
    printf ("%s=\"%d\"", name, (int)value);
    }

void PrintPoint (char const* name, DPoint3dCR xyz)
    {
    printf ("    <%s>%g,%g,%g</%s>\n", name, xyz.x, xyz.y, xyz.z, name);
    }

static void PrintPolyfaceXYZ (PolyfaceHeader &mesh, char const* title, size_t maxPrintSize)
    {
    if (!Check::PrintDeepStructs ())
        return;
    if ((maxPrintSize > 0) && ((int)mesh.Point().size() > maxPrintSize))
        return;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (mesh);
    printf ("<PolyfaceXYZ><name> %s</name>\n", title);
    size_t faceCounter = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();faceCounter++)
        {
        printf ("<face>");
        PrintAttribute ("id", faceCounter);
        PrintAttribute ("edges", visitor->NumEdgesThisFace ());
        printf (">\n");
        for (size_t i = 0; i < visitor->NumEdgesThisFace (); i++)
            {
            if (visitor->Visible()[i])
              PrintPoint ("Vis", visitor->Point()[i]);
            else 
              PrintPoint ("Hid", visitor->Point()[i]);
            }
        printf ("  </face>\n");
        }
    printf ("</PolyfaceXYZ>\n");
    }

void PrintPolyfaceSummary (PolyfaceHeader& mesh, char const* title, FILE *file)
    {
    size_t numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisible, numInvisible;
    mesh.CollectCounts (numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisible, numInvisible);
    Check::PrintIndent (1); 
        Check::Print (title);
        Check::Print ((int)numVertex, "NV");
        Check::Print ((int)numFacet, "NF");
        Check::Print ((int)numTri, "NTri");
        Check::Print ((int)numQuad, "NQuad");

    if (numVertex < 2)
        return;
    DRange3d range = mesh.PointRange ();
    Check::Print (range.low, "Range.low");
    Check::Print (range.high, "Range.low");
    if (mesh.IsClosedByEdgePairing ())
        {
        double v0 = mesh.SumTetrahedralVolumes (range.low);
        double v1 = mesh.SumTetrahedralVolumes (range.high);
        Check::PrintIndent (1);
        Check::Print (v0, "V0");
        Check::Print (v1, "V1");
        }
    else
        Check::Print ("(OPEN)");

    double volume;
    DPoint3d centroid;
    RotMatrix axes;
    DVec3d momentxyz, normal;
    double area;
    if (!mesh.ComputePrincipalMomentsAllowMissingSideFacets (volume, centroid, axes, momentxyz, false))
        Check::Print ("(NoRaggedVolume)");
    else
        {
        Check::Print (volume, "RaggedVolume");
        }
    auto visitor = PolyfaceVisitor::Attach (mesh);
    UsageSums areaSums[3];
    double zTol = Angle::SmallAngle ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        if (visitor->TryGetFacetCentroidNormalAndArea (centroid, normal, area))
            {
// unused            int k = 0;
            if (fabs (normal.z) < zTol)
                areaSums[1].Accumulate (area);
            else if (normal.z > 0.0)
                areaSums[2].Accumulate (area * normal.z);
            else
                areaSums[0].Accumulate (area * normal.z);
            }
        }
    Check::PrintIndent (1);Check::Print(areaSums[0].Count (), "#down");Check::Print (areaSums[0].Sum(), "Area");
    Check::PrintIndent (1);Check::Print(areaSums[1].Count (), "#vert");Check::Print (areaSums[1].Sum(), "Area");
    Check::PrintIndent (1);Check::Print(areaSums[2].Count (), "#up  ");Check::Print (areaSums[2].Sum(), "Area");
    }

void PrintPolyface (PolyfaceHeader& mesh, char const* title, FILE *file, size_t maxPrintSize, bool suppressSecondaryData)
    {
    // TODO: Make these callers do Check::SaveTransformed and close their file.
    }
void PrintPolyface (PolyfaceHeader& mesh, char const *title)
    {
    // TODO: Make these callers do Check::SaveTransformed and close their file.
    }

void PrintVisitor (PolyfaceVisitor &visitor, char const *title)
    {
    uint32_t numEdges = visitor.NumEdgesThisFace ();
    for (size_t i = 0; i < numEdges; i++)
        {
        printf (" (xyz %g,%g,%g)", visitor.Point()[i].x, visitor.Point()[i].y, visitor.Point()[i].z);
        if (visitor.Param().size () > i)
            printf (" (uv %g,%g)", visitor.Param()[i].x, visitor.Param()[i].y);
        if (visitor.Normal().size () > i)
            printf (" (uv %g,%g,%g)", visitor.Normal()[i].x, visitor.Normal()[i].y, visitor.Normal()[i].z);
        printf ("\n");
        }
    }





//#define TEST_NOTNOW(__A,__B) static void __A##__B##NotNow ()
#define TEST_NOTNOW(__A,__B) TEST(__A,__B)

void CheckCounts (PolyfaceHeader &mesh,
size_t num0In,
size_t numPairIn,
size_t num1In,
size_t num2In,
size_t num3In,
size_t num4In,
size_t numMoreIn
)
    {
    size_t numPair, num0, num1, num2, num3, num4, numMore, numCollapsed;
    mesh.CountSharedEdges (num0, numPair, num1, num2, num3, num4, numMore, numCollapsed);
    printf ("(coedges %d)(pairs %d) (num1 %d) (num2 %d) (num3 %d) (num4 %d) (numMore %d)\n", (int)num0, (int)numPair, (int)num1, (int)num2, (int)num3, (int)num4, (int)numMore);
    CHECK_EQ (Size, numPairIn, numPair);
    CHECK_EQ (Size, num0In, num0);
    CHECK_EQ (Size, num1In, num1);
    CHECK_EQ (Size, num2In, num2);
    CHECK_EQ (Size, num3In, num3);
    CHECK_EQ (Size, num4In, num4);
    CHECK_EQ (Size, numMoreIn, numMore);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (COMMENT_OUT)
TEST (Polyface, Carrier)
    {
    DPoint3d xyz[4] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,1,0}
        };

    PolyfaceQueryCarrier carrier
            (1, false,
            0,
            4, xyz, NULL,               // points
            0, NULL, NULL,              // normals
            0, NULL, NULL,              // params
            0, NULL, NULL,              // colors
            NULL);

    PolyfaceHeaderPtr header = PolyfaceHeader::CreateVariableSizeIndexed ();
    }
#endif

bool FindPair (size_t i, size_t j, bvector<std::pair<size_t,size_t>> *omitXY)
    {
    if (omitXY == nullptr)
        return false;
    for (auto &omit : *omitXY)
        {
        if (omit.first == i && omit.second == j)
            return true;
        }
    return false;
    }
PolyfaceHeaderPtr CreateGridMesh (size_t nx, size_t ny, bool doNormals = true, bool doParams = true, bvector<std::pair<size_t,size_t>> *omitXY = nullptr)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh.get()->Normal ().SetActive (doNormals);
    mesh.get()->NormalIndex ().SetActive (doNormals);

    mesh.get()->Param ().SetActive (doParams);
    mesh.get()->ParamIndex ().SetActive (doParams);

    DPoint3d xyz[10];
    DVec3d   normal[10];
    DPoint2d param[10];
    double xScale = 1.0 / (nx + 1);
    double yScale = 1.0 / (ny + 1);
    // Create a quad grid.
    //  xy coordinates are on interger grid.
    //  params are xy coordinates scaled to 0..1
    //  normals are all 001
    for (size_t i = 0; i < nx; i++)
        {
        for (size_t j = 0; j < ny; j++)
            {
            if (FindPair (i,j,omitXY))
                continue;
            size_t n = 0;
            double ai = (double)i;
            double aj = (double)j;
            xyz[n++].Init (ai, aj, 0);
            xyz[n++].Init (ai+1, aj, 0);
            xyz[n++].Init (ai+1, aj+1, 0);
            xyz[n++].Init (ai, aj+1, 0);
            xyz[n++].Init (ai, aj, 0);
            for (size_t k = 0; k < n; k++)
                {
                normal[k].Init (0,0,1);
                param[k].Init (xyz[i].x * xScale, xyz[i].y * yScale);
                }
            mesh.get()->AddPolygon (xyz, n, normal, param);
            }
        }
    mesh->Compress ();
    return mesh;
    }

IFacetOptionsPtr CreateFacetOptions (bool normals = true, bool params = true, bool edgeChains = true)
    {
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetNormalsRequired (normals);
    options->SetParamsRequired (params);
    options->SetEdgeChainsRequired (edgeChains);
//    options->SetFaceIndexRequired (true);
    options->SetParamMode (FACET_PARAM_01BothAxes);
    return options;
    }
IPolyfaceConstructionPtr CreateBuilder (bool normals, bool params)
    {
    IFacetOptionsPtr options = CreateFacetOptions (normals, params);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    return builder;
    }

bool Inspect (PolyfaceQueryCR mesh, size_t numFacet0, size_t minVertex0, size_t maxVertex0, bool nonPlanar0, bool nonConvex0)
    {
    size_t numFacet1, minVertex1, maxVertex1;
    bool nonPlanar1, nonConvex1;
    mesh.InspectFaces (numFacet1, minVertex1, maxVertex1, nonPlanar1, nonConvex1);
    return Check::Size (numFacet0, numFacet1, "numFacet")
        && Check::Size(minVertex0, minVertex1, "minVertex")
        && Check::Size(maxVertex0, maxVertex1, "maxVertex")
        && Check::Bool(nonPlanar0, nonPlanar1, "nonPlanar")
        && Check::Bool(nonConvex0, nonConvex1, "nonConvex");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, TriangulateCounts)
    {

    PolyfaceHeaderPtr mesh = CreateGridMesh (4,3);
    double area0 = mesh->SumFacetAreas ();
    //PrintPolyface (*mesh, "Grid", stdout, 400, true);

    size_t numQuad0 = mesh->GetNumFacet ();
    Inspect (*mesh, 12, 4, 4, false, false);
    mesh->Triangulate (4);
    //PrintPolyface (*mesh, "Grid.Triangulate(4)", stdout, 400, true);
    size_t numQuad1 = mesh->GetNumFacet ();
    Check::Size (numQuad0, numQuad1, "Triangulate allow quads");
    mesh->Triangulate (3);
    Inspect(*mesh, 24, 3, 3, false, false);
    //PrintPolyface (*mesh, "Grid.Triangulate(3)", stdout, 400, true);
    size_t numTri2 = mesh->GetNumFacet ();
    Check::Size (2 * numQuad0, numTri2, "Triangulate in quads");
    mesh->Triangulate (3);
    //PrintPolyface (*mesh, "Grid.Triangulate(3) again", stdout, 400, true);
    size_t numTri3 = mesh->GetNumFacet ();
    Check::Size (numTri2, numTri3, "retriangulate in place.");
    double area1 = mesh->SumFacetAreas ();
    Check::Near (area0, area1, "Area after triangulation");
    }

void CheckPartition (char const* descr, size_t componentTarget, size_t faceTarget, PolyfaceHeaderPtr mesh, bvector<ptrdiff_t> indices)
    {
    size_t totalFaces = mesh->GetNumFacet ();
    //varunused size_t numThisBlock = 0;
    size_t blockStart = 0;
    BlockedVectorInt blockSize;
    for (size_t i = 0; i < indices.size (); i++)
        {
        if (indices[i] < 0)
            {
            blockSize.push_back ((int)(i - blockStart));
            blockStart = i + 1;
            }
        }
    size_t numBlock = blockSize.size ();
    int minSize, maxSize;
    blockSize.MinMax (minSize, maxSize);
    Check::True (minSize > 0, "partition minSize > 0");
    Check::True (maxSize > 0, "partition maxSize > 0");
    if (faceTarget > 0)
        Check::True ((size_t)maxSize <= faceTarget, "faceTarget confirm");

    if (componentTarget > 0)
        Check::True (numBlock >= componentTarget, "faceTarget confirm");


    Check::True (maxSize <= (int)totalFaces, "partition maxSize < total faces");
    Check::True (maxSize * numBlock >= totalFaces, "partition maxSize * numBlock >= totalFaces");

    bvector<PolyfaceHeaderPtr> fragmentMeshArray;
    mesh->CopyPartitions (indices, fragmentMeshArray);
    Check::Size (fragmentMeshArray.size (), numBlock);
    double masterArea = mesh->SumFacetAreas ();
    double fragmentArea = 0.0;
    for (size_t i = 0; i < fragmentMeshArray.size (); i++)
        fragmentArea += fragmentMeshArray[i]->SumFacetAreas ();
    Check::Near (masterArea, fragmentArea, "Summed area of fragment meshes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, PartitionByFaceTarget)
    {
    size_t nx = 4;
    size_t ny = 10;
    size_t totalFaces = nx * ny;
    for (size_t faceTarget = totalFaces; faceTarget > 4; faceTarget = (faceTarget * 2 ) / 3)
        {
        PolyfaceHeaderPtr mesh = CreateGridMesh (nx, ny);
        bvector<ptrdiff_t> indices;
        mesh->PartitionByXYRange (faceTarget, 0, indices);
        CheckPartition ("Face Target", 0, faceTarget, mesh, indices);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, PartitionByComponentTarget)
    {
    size_t nx = 4;
    size_t ny = 10;
    size_t totalFaces = nx * ny;
    for (size_t componentTarget = 1; componentTarget * 4 < totalFaces; componentTarget = (componentTarget * 5) / 2)
        {
        PolyfaceHeaderPtr mesh = CreateGridMesh (nx, ny);
        bvector<ptrdiff_t> indices;
        mesh->PartitionByXYRange (0, componentTarget, indices);
        CheckPartition ("ComponentTarget Target", componentTarget, 0, mesh, indices);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, Compress)
    {
    int nx = 3;
    int ny = 8;
    PolyfaceHeaderPtr mesh = CreateGridMesh (nx, ny);
    //varunused size_t numPointA  = mesh.get()->GetPointCount ();
    //varunused size_t numNormalA = mesh.get()->GetNormalCount ();
    //varunused size_t numParamA  = mesh.get()->GetParamCount ();
    mesh.get()->Compress ();
    //varunused size_t numPointB  = mesh.get()->GetPointCount ();
    //varunused size_t numNormalB = mesh.get()->GetNormalCount ();
    //varunused size_t numParamB  = mesh.get()->GetParamCount ();
    CHECK_EQ (Size, mesh.get()->GetPointCount (), (nx + 1) * (ny + 1));
    CHECK_EQ (Size, mesh.get()->GetNormalCount (), 1);
    //CHECK_EQ (Size, mesh.get()->GetParamCount (), (nx + 1) * (ny + 1));
    }

void PrintGraph (MTGGraph *pGraph, int vertexLabelOffset)
    {
    int partnerLabelOffset = jmdlMTGGraph_getLabelOffset (pGraph, MTG_NODE_PARTNER_TAG);
    int numActive = 0;
    int numExterior = 0;
    MTGMask faceVisited = jmdlMTGGraph_grabMask (pGraph);
    MTGMask vertexVisited = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, faceVisited | vertexVisited);
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        numActive++;
        MTGNodeId partnerNodeId = MTGGraph::NullNodeId;
        if (jmdlMTGGraph_getMask (pGraph, seedNodeId, MTG_EXTERIOR_MASK))
            numExterior++;
        jmdlMTGGraph_getLabel (pGraph, &partnerNodeId, seedNodeId, partnerLabelOffset);
        printf ("(%2d F %2d V %2d", seedNodeId,
                            jmdlMTGGraph_getFSucc (pGraph, seedNodeId),
                            jmdlMTGGraph_getVSucc (pGraph, seedNodeId));
        if (jmdlMTGGraph_isValidNodeId (pGraph, partnerNodeId))
            printf (" P %2d", partnerNodeId);
        printf ("%s",jmdlMTGGraph_getMask (pGraph, seedNodeId, MTG_EXTERIOR_MASK) ? "X" : " ");
        printf ("%s",jmdlMTGGraph_getMask (pGraph, seedNodeId, MTG_PRIMARY_EDGE_MASK) ? "P" : " ");
        printf (")\n");
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)


    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, faceVisited))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, faceVisited);
            printf ("(Face");
            MTGARRAY_FACE_LOOP (nodeId, pGraph, seedNodeId)
                {
                printf (" %2d", nodeId);
                int vertexIndex;
                if (jmdlMTGGraph_getLabel (pGraph, &vertexIndex, nodeId, vertexLabelOffset))
                    printf ("@%d", vertexIndex);
                }
            MTGARRAY_END_FACE_LOOP (nodeId, pGraph, seedNodeId)
            printf (")\n");
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)


    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, vertexVisited))
            {
            jmdlMTGGraph_setMaskAroundVertex (pGraph, seedNodeId, vertexVisited);
            printf (" (Vertex");
            MTGARRAY_VERTEX_LOOP (nodeId, pGraph, seedNodeId)
                {
                printf (" %2d", nodeId);
                }
            MTGARRAY_END_VERTEX_LOOP (nodeId, pGraph, seedNodeId)
            printf (")\n");
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)
    printf ("   (Active %d) (Exterior %d)\n", numActive, numExterior);
    jmdlMTGGraph_printLoopCounts (pGraph);
    jmdlMTGGraph_dropMask (pGraph, faceVisited);
    jmdlMTGGraph_dropMask (pGraph, vertexVisited);
    }

void PrintGraphSummary (MTGGraphP graph, char const* title)
    {
    printf ("  %s (N %d)\n", title, (int)graph->GetActiveNodeCount ());
    jmdlMTGGraph_printLoopCounts (graph);
    }
void PrintNewLine (size_t i, size_t numTotal, size_t numPerLine)
    {
    if (i + 1 == numTotal
        || (i % numPerLine) == numPerLine - 1)
        printf ("\n");
    }

void PrintIndexVector (BlockedVectorInt &source, char const* name)
    {
    printf ("<%s Active=\"%s\">\n", name, source.Active () ? "true" : "false");

    size_t numThisFace = 0;
    size_t faceIndex = 0;
    for (size_t n = source.size (), i = 0; i < n; i++)
        {
        if (numThisFace == 0)
            printf ("    <Face  startPos=\"%d\">", (int)i);
        int index = source[i];
        if (numThisFace > 0)
            printf (",");
        printf ("%d", index);
        numThisFace++;
        if (index == 0 || i == (n - 1))
            {
            printf ("</Face>");
            PrintNewLine (faceIndex, i == (n-1) ? faceIndex + 1 : faceIndex + 2, 3);
            numThisFace = 0;
            faceIndex++;
            }
        }
    printf ("</%s>\n", name);
    }

void VerifyFaceData (PolyfaceHeader  &mesh, char const*  name)
    {
    if (mesh.FaceIndex ().Active ()
        && Check::Size (mesh.PointIndex ().size (), mesh.FaceIndex ().size (), "FaceIndex present and same size as PointIndex"))
        {
        //varunused bvector<FacetFaceData> &faceDataVector = mesh.FaceData ();
        BlockedVectorIntR paramIndexVector = mesh.ParamIndex ();
        BlockedVectorIntR normalIndexVector = mesh.NormalIndex ();
        BlockedVectorIntR pointIndexVector = mesh.PointIndex ();
        BlockedVectorIntR faceIndexVector = mesh.FaceIndex ();
        if (paramIndexVector.size () > 0)
            Check::Size (paramIndexVector.size (), pointIndexVector.size (), "point, param index counts match");
        if (normalIndexVector.size () > 0)
            Check::Size (normalIndexVector.size (), pointIndexVector.size (), "point, normal index counts match");
        //varunused int currFaceIndex1 = 0;
        FacetFaceData faceData;
        size_t errors = 0;
        // Verify face data is present at all non-zero indices..
        for (size_t i = 0; i < faceIndexVector.size (); i++)
            {
            if (faceIndexVector[i] == 0 || pointIndexVector[i] == 0)
                {
                if (faceIndexVector[i] != pointIndexVector[i])
                    errors++;
                }
            else
                {
                size_t faceDataIndex;
                if (!mesh.TryGetFacetFaceDataAtReadIndex (i, faceData, faceDataIndex))
                    errors++;
                }
                
            }
        Check::Size (0, errors, "faceIndex errors");
        
        }
    }


//static int s_pointTestCounter;
static int s_polyfaceCounter;
void VerifyPolyface (PolyfaceHeader  &meshVectors, PolyfaceQueryR meshQuery, char const* name)
    {
    s_polyfaceCounter++;
    PolyfaceVisitorPtr vectorVisitor = PolyfaceVisitor::Attach (meshVectors);
    PolyfaceVisitorPtr queryVisitor  = PolyfaceVisitor::Attach (meshQuery);
    VerifyFaceData (meshVectors, name);
#ifdef abc
    Check::Size (meshVectors.GetNumFacet (), meshQuery.GetNumFacet ());    
    Check::Bool (meshVectors.GetHasFacets (), meshQuery.HasFacets ());    
    Check::Bool (meshVectors.HasConvexFacets (), meshQuery.HasConvexFacets ());    
    Check::Bool (meshVectors.IsTriangulated (), meshQuery.IsTriangulated ());
#endif
    size_t numFacet0 = 0;
    for (vectorVisitor->Reset (); vectorVisitor->AdvanceToNextFace ();)
        {
        numFacet0++;
        Check::True (queryVisitor->AdvanceToNextFace ());
        Check::True (DPoint3dOps::Equal (queryVisitor->Point (), vectorVisitor->Point ()));
        Check::True (DPoint2dOps::Equal (queryVisitor->Param (), vectorVisitor->Param ()));
        Check::True (DVec3dOps::Equal   (queryVisitor->Normal(), vectorVisitor->Normal ()));
        }

    double f0 = 0.3;
    double f1 = 0.4;
    double f2 = 1.0 - f0 - f1;
    int outerFaceCount = 0;
    DPoint3d zero;
    zero.Zero ();
    //varunused double tolerance = meshVectors.GetMediumTolerance ();
    double s_relTol = 0.01;
    //varunused int failures = 0;
    int numEdges = 0;
    bvector<int>failureIndices;
    bvector<int>edgeFailureIndices;
    size_t numFacet = meshVectors.GetNumFacet ();
    size_t testPeriod = 1;
    if (numFacet <= 10)
        testPeriod = 1;
    else if (numFacet <= 50)
        testPeriod = numFacet / 5;
    else
        testPeriod = (size_t) sqrt ((double)numFacet);
    for (vectorVisitor->Reset (); vectorVisitor->AdvanceToNextFace (); outerFaceCount++)
        {
        if ((outerFaceCount % testPeriod) == 0)
            {
            if (vectorVisitor->NumEdgesThisFace () < 3)
                continue;

            // This is within the first triangle.
            // If it is a convex face (which we are not testing!!!) it is also within the facet..
            DPoint3d testPoint, facetPoint;

            DPoint3d point0 = vectorVisitor->Point()[0];
            DPoint3d point1 = vectorVisitor->Point()[1];
            DPoint3d point2 = vectorVisitor->Point()[2];
            // FACE HIT ...
            testPoint.SumOf (zero, point0, f0, point1, f1, point2, f2);
            double diagonal = point0.Distance (point1) + point1.Distance (point2);
            double tolerance1 = s_relTol * diagonal;
            PolyfaceVisitorPtr pointVisitor = PolyfaceVisitor::Attach (meshVectors);
            int innerCount = 0;
            for (pointVisitor->Reset ();pointVisitor->AdvanceToFacetBySearchPoint (testPoint, tolerance1, facetPoint);)
                {
                Check::True (testPoint.Distance (facetPoint) <= tolerance1, "facet search point close to space point");
                innerCount++;
                }
            if (innerCount == 1)
                {
                // expected interior case.
                }
            else if (innerCount == 2)
                {
                numEdges++;
                }
            else
                failureIndices.push_back (outerFaceCount);

            // EDGE HIT ...
            DPoint3d edgePoint, edgeTestPoint;
            size_t sourceEdgeIndex = 0;
            if ( vectorVisitor->Point()[0].Distance (vectorVisitor->Point()[1]) > tolerance1)
                {
                edgeTestPoint.Interpolate (point0, f0, point1);
                ptrdiff_t edgeIndex;
                double edgeFraction;
                int numFacetHit = 0;
                int numEdgeHit = 0;
                double edgeFractionTol = 1.0e-10;
                bool foundPrimaryTarget = false;
                for (pointVisitor->Reset (); pointVisitor->AdvanceToFacetBySearchPoint (edgeTestPoint, tolerance1, edgePoint, edgeIndex, edgeFraction);)
                    {
                    numFacetHit++;
                    if (edgeIndex >= 0)
                        numEdgeHit++;
                    if (vectorVisitor->IndexPosition()[0] == pointVisitor->IndexPosition()[0]
                        && edgeIndex == sourceEdgeIndex
                        && fabs (edgeFraction - f0) <= edgeFractionTol)
                        {
                        foundPrimaryTarget = true;
                        }
                    }
                if (numEdgeHit == 0 || !foundPrimaryTarget)
                    edgeFailureIndices.push_back (outerFaceCount);
                }
            }
        }
    if (failureIndices.size () > 0)
        {
        printf ("AdvanceToFacetBySearchPoint failed on faces ");
        for (size_t i = 0; i < failureIndices.size (); i++)
            printf (" %d", failureIndices[i]);
        printf ("\n (total faces %d) (test period %d)\n", (int)outerFaceCount, (int)testPeriod);
        PrintPolyface (meshVectors, name, stdout, 20);
        }

    if (edgeFailureIndices.size () > 0)
        {
        printf ("AdvanceToFacetBySearchPoint with edge data failed on faces ");
        for (size_t i = 0; i < edgeFailureIndices.size (); i++)
            printf (" %d", edgeFailureIndices[i]);
        printf ("\n (total faces %d) (test period %d)\n", (int)outerFaceCount, (int)testPeriod);
        PrintPolyface (meshVectors, name, stdout, 20);
        }

        {
        size_t numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisible, numInvisible;
        meshVectors.CollectCounts (numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisible, numInvisible);
        Check::Size (numFacet0, numFacet, "Facet Counts");
        }

    }

double MaxNormalDeviation (PolyfaceHeaderR meshWithNormals)
    {
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (meshWithNormals, false);

    visitor->Reset ();
    visitor->SetNumWrap (2);
    //varunused bvector <int> &pointIndex = visitor->PointIndex ();
    //varunused bvector <size_t> &readIndex = visitor->IndexPosition ();
    bvector <DPoint3d> &points = visitor->Point ();
    
    double thetaMax = 0.0;
    for (;visitor->AdvanceToNextFace ();)
        {
        uint32_t numEdges = visitor->NumEdgesThisFace ();
        DVec3d facetNormal = PolygonOps::AreaNormal (points, numEdges);
        facetNormal.Normalize ();
        for (size_t i = 0; i < (size_t)numEdges; i++)
            {
            DVec3d sectorNormal;
            sectorNormal.CrossProductToPoints (points[i+1], points[i+2], points[i]);
            if (sectorNormal.DotProduct (facetNormal) < 0.0)
                sectorNormal.Negate ();
            double theta = sectorNormal.AngleTo (facetNormal);
            DoubleOps::UpdateMax (thetaMax, theta);
            }
        }
    return thetaMax;
    }
static int s_checkMTGConnectivity = 0;
static int s_checkMTGCoordinates = 1;
void VerifyMTG (PolyfaceHeaderR mesh, char const*  title)
    {
    static bool s_dumpMTG = false;
    if (s_checkMTGConnectivity)
        {
        MTGFacets *mtgFacets = jmdlMTGFacets_new ();
        PolyfaceToMTG_FromPolyfaceConnectivity (mtgFacets, mesh);
        PrintGraphSummary (jmdlMTGFacets_getGraph (mtgFacets), "MTG From Connectivity");
        jmdlMTGFacets_free (mtgFacets);
        }
    if (s_checkMTGCoordinates)
        {
        MTGFacets *mtgFacets = jmdlMTGFacets_new ();
        bvector<MTGNodeId> vertexToNodeId;
        bvector<size_t> nodeIdToPolyfaceIndexPosition;

        double absTol = 1e-12;
        double relTol = 0.0;
        bool dropToSingleFace = true;
        Check::True ( PolyfaceToMTG (mtgFacets, &vertexToNodeId, &nodeIdToPolyfaceIndexPosition, mesh, dropToSingleFace,
                absTol, relTol));    
        
        if (s_dumpMTG)
            PrintGraphSummary (jmdlMTGFacets_getGraph (mtgFacets), "MTG From Coordinates");
        jmdlMTGFacets_free (mtgFacets);
        }
    }

// 
void CheckSurfaceMesh (MSBsplineSurfaceCR surface, IPolyfaceConstructionR builder, bool noisy = false)
    {
    PolyfaceHeaderR mesh = builder.GetClientMeshR ();
    if (noisy)
        PrintPolyface (mesh, "SurfaceMesh", stdout, 400, false);
    Transform meshTransform;
    builder.GetLocalToWorld (meshTransform);
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (mesh, true);
    bool ok = true;
    for (visitor->Reset (); ok && visitor->AdvanceToNextFace ();)
        {
        if (visitor->Param().size () > 0)
            {
            if (noisy)
                PrintVisitor (*visitor, "VisitorFace");
            for (size_t i = 0; ok && i < visitor->Param ().size (); i++)
                {
                DPoint2d uv = visitor->Param ()[i];
                DPoint3d xyz;
                DVec3d   dXdu, dXdv;
                surface.EvaluatePoint (xyz, dXdu, dXdv, uv.x, uv.y);
                meshTransform.Multiply (xyz);
                ok &= Check::Near (visitor->Point ()[i], xyz, "mesh uv to bsurf xyz");
                if (visitor->Normal ().size () > 0)
                    {
                    DVec3d normal = visitor->Normal ()[i];
                    ok &= Check::True (normal.IsPerpendicularTo (dXdu), "normal perp dXdu");
                    ok &= Check::True (normal.IsPerpendicularTo (dXdv), "normal perp dXdv");
                    }
                }
            }
        }
    }
    
void ExamineCounts (PolyfaceQueryCR mesh, char const* title)
    {
    size_t numEdgeUse;
    size_t numMatedPair;
    size_t num1, num2, num3, num4, numMore, numCollapsed, num0Vis, num1Vis;
    // hm .. don't know the right answer, but at least run the queries ...
    mesh.CountSharedEdges (numEdgeUse, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, false, num0Vis, num1Vis);

    bvector<DSegment3d> segments;
    mesh.CollectSegments (segments, false);

    PolyfaceHeaderPtr mesh1 = PolyfaceHeader::CreateVariableSizeIndexed ();  // But type will change on CopyFrom !!!
    mesh1->CopyFrom (mesh);
    mesh1->MarkInvisibleEdges (0.1);
    mesh1->MarkAllEdgesVisible ();
    mesh1->MarkTopologicalBoundariesVisible (true);
    if (mesh.IsClosedByEdgePairing ())
        {
        }
    }
    
void ExaminePolyface (PolyfaceHeaderR mesh, char const* title)
    {
    static int s_print = 0;
    if (s_print)
        PrintPolyface (mesh, title, stdout, 400, false);
    if (mesh.Point().size () > 0)
        {
    VerifyPolyface (mesh, mesh, title);
    VerifyMTG(mesh, title);
    PolyfaceHeaderPtr compactee = PolyfaceHeader::CreateFixedBlockIndexed (4);
    mesh.CopyTo (*compactee);
    compactee->CompactIndexArrays ();
    VerifyPolyface (*compactee, *compactee, "compacted");
    

    PolyfaceHeaderPtr meshWithNormals = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh.CopyTo (*meshWithNormals);
    static double s_singleEdgeAngle = 0.1;
    static double s_multipleEdgeAngle = 0.1;
    static double s_planarDeviationAngle = 0.01;
    Check::True (meshWithNormals->BuildApproximateNormals (s_singleEdgeAngle, s_multipleEdgeAngle), "Build approximate normals");
    double theta = MaxNormalDeviation (*meshWithNormals);
    meshWithNormals->BuildPerFaceParameters (LOCAL_COORDINATE_SCALE_01RangeBothAxes);
    if (s_print)
        PrintPolyface (*meshWithNormals, "ComptuedNormals", stdout, 400, false);
    Check::True (theta <= s_multipleEdgeAngle * 1.01,
                "Computed normals close to face planes");

    // blast the normals away, replace with per face.
    meshWithNormals->BuildPerFaceNormals ();
    double thetaB = MaxNormalDeviation (*meshWithNormals);
    Check::True (thetaB <= s_planarDeviationAngle, 
                "Computed normals close to face planes");

    ExamineCounts (mesh, title);

    PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateUnifiedIndexMesh (*meshWithNormals);
    if (s_print)
        PrintPolyface (*meshB, "UnifiedNormals", stdout, 400, false);
    meshWithNormals->Triangulate ();
    PolyfaceHeaderPtr meshC = PolyfaceHeader::CreateUnifiedIndexMesh (*meshWithNormals);
    if (s_print)
        PrintPolyface (*meshC, "UnifiedNormalsTriangulated", stdout, 400, false);

    Check::Near (meshWithNormals->SumFacetAreas (), meshB->SumFacetAreas (), "Area sums -- unified");
    Check::Near (meshWithNormals->SumFacetAreas (), meshC->SumFacetAreas (), "Area sums -- unified triangles");
    Check::Size (meshB->Point ().size (), meshC->Point ().size (), "Triangulation should not change unified mesh point counts");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, StitchCube)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    static int s_printGraph = 0;
    DPoint3d cubeCorners[8];
    DRange3d range;
    range.InitFrom (0,0,0, 1,1,1);
    range.Get8Corners (cubeCorners);
    //
    //
    //     2------3
    //     |\    /|
    //     | 6--7 |
    //     | |  | |
    //     | 4--5 |
    //     |/    \|
    //     0------1
    //
    int index[6][4] =
        {
        {0,2,3,1},
        {0,1,5,4},
        {4,5,7,6},
        {1,3,7,5},
        {2,0,4,6},
        {6,7,3,2},
        };

    // Build with leading 1,2,3,4,5,6 faces of the cube.
    int expectedSingleSidedExteriorCount [] =
        {4,6,8,6,4,0};
    int expectedSingleSidedFaceCount [] =
        {2, 3, 4, 5, 6, 6};
    for (int boolState = 0; boolState < 2; boolState++)
        {
        bool dropToSingleFace = boolState > 0;
        for (int numPanel = 1; numPanel <= 6; numPanel++)
            {
            PolyfaceHeaderPtr polyfaceMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
            polyfaceMesh.get()->SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
            polyfaceMesh.get()->PointIndex ().SetActive (true);
            for (int face = 0; face < numPanel; face++)
                {
                DPoint3d xyz[4];
                for (int i = 0; i < 4; i++)
                    xyz[i] = cubeCorners[index[face][i]];
                polyfaceMesh.get()->AddPolygon (xyz, 4);
                }
            MTGFacets *mtgFacets = jmdlMTGFacets_new ();
            bvector<MTGNodeId> vertexToNodeId;
            bvector<size_t> nodeIdToPolyfaceIndexPosition;
            double absTol = 1e-12;
            double relTol = 0.0;
            polyfaceMesh.get()->Compress ();
            MTGGraphP pGraph = jmdlMTGFacets_getGraph (mtgFacets);
            Check::True ( PolyfaceToMTG (mtgFacets, &vertexToNodeId, &nodeIdToPolyfaceIndexPosition, *polyfaceMesh.get(), dropToSingleFace, absTol, relTol));    
 
            int num0, num1;
            jmdlMTGGraph_countMasksInSet (&num1, &num0, pGraph, MTG_EXTERIOR_MASK);
            int numFace = (int)pGraph->CountFaceLoops ();
            if (s_printGraph)
                {
               jmdlMTGGraph_printLoopCounts (pGraph);
               printf ("(Patches %d) (%s) (Ex %d)\n",
                        numPanel,
                        dropToSingleFace ? "SingleFace" : "DoubleFace",
                        num1);
                PrintGraph (pGraph, mtgFacets->vertexLabelOffset);
                }

            if (dropToSingleFace)
                {
                Check::Int (expectedSingleSidedExteriorCount[numPanel - 1], num1);
                Check::Int (expectedSingleSidedFaceCount[numPanel - 1], numFace);
                }
            else
                {
                Check::Int (4 * numPanel, num1);
                Check::Int (2 * numPanel, numFace);
                }

            PolyfaceHeaderPtr polyfacePtrB = PolyfaceHeader::CreateVariableSizeIndexed ();
            PolyfaceHeaderR polyfaceB = *polyfacePtrB.get();
            polyfaceB.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
            polyfaceB.PointIndex ().SetActive (true);
            size_t numFaceB = AddMTGFacetsToIndexedPolyface (mtgFacets, polyfaceB);
            Check::Size (numFaceB, numPanel);
            PolyfaceHeaderPtr polyfacePtrC = PolyfaceHeader::CreateVariableSizeIndexed ();
            PolyfaceHeaderR polyfaceC = *polyfacePtrB.get();
            polyfaceC.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
            polyfaceC.PointIndex ().SetActive (true);
            size_t numFaceC = AddMTGFacetsToIndexedPolyface (mtgFacets, polyfaceC, 0);
            if (dropToSingleFace)
                {
                //PrintGraph (jmdlMTGFacets_getGraph (mtgFacets), mtgFacets->vertexLabelOffset);
                Check::Size (numFaceC, expectedSingleSidedFaceCount[numPanel - 1]);
                }
            else
                {
                Check::Size (numFaceC, numPanel * 2);
                }                    
            jmdlMTGFacets_free (mtgFacets);
            }
        }
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ClipCube_TwoPlanesCut)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetMaxPerFace (4);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);



    builder->AddSweptNGon (4, 2.0, 0.0, 1.0, true, true);

    DPoint3d xyz0 = builder->MultiplyByLocalToWorld (DPoint3d::FromXYZ (0.5,0.5,0));
    DPoint3d xyz1 = builder->MultiplyByLocalToWorld (DPoint3d::FromXYZ (3,3,0));

    PolyfaceHeader & header0 = builder->GetClientMeshR ();
    Check::True (header0.IsClosedByEdgePairing ());
    CheckCounts (header0, 24, 12,0,12,0,0,0);
    ExaminePolyface (header0, "DiamondFrustumC");


    ClipPlaneSet clipper;
    clipper.push_back ( ConvexClipPlaneSet::FromXYBox (xyz0.x, xyz0.y, xyz1.x, xyz1.y));

    Check::SaveTransformed (header0);
   
    Check::Shift (0, 10, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            header0, clipper, true,
            &insideClip, &outsideClip);
    Check::SaveTransformed (*insideClip);
    Check::Shift (10,0, 0);
    Check::SaveTransformed (*outsideClip);

    Check::ClearGeometry ("Polyface.ClipCube_TwoPlanesCut");
    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ClipSphere)
    {
    int64_t allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;

    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = SetTransformToNewGridSpot (*builder, true);

    //varunused size_t numPerQuadrant = 1;
    PolyfaceHeader &header0 = builder->GetClientMeshR ();
    //varunused double radius = mySize;
    //varunused double zStep = 3.0 * mySize;

    for (size_t numPerQuadrant = 1; numPerQuadrant < 10; numPerQuadrant *= 4 )
        {
        builder->Clear ();
        builder->AddFullSphere (DPoint3d::FromXYZ(0,0,0), mySize, numPerQuadrant, numPerQuadrant);
        ExaminePolyface (header0,  "Sphere");
        SetTransformToNewGridSpot (*builder);
        }

    ClipPlaneSet clipper;
    clipper.push_back (ConvexClipPlaneSet::FromXYBox (-1, -5, 1, 5));


    Check::SaveTransformed (header0);
   
    Check::Shift (0, 10, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            header0, clipper, true,
            &insideClip, &outsideClip);
    Check::SaveTransformed (*insideClip);
    Check::Shift (10,0, 0);
    Check::SaveTransformed (*outsideClip);
    Check::ClearGeometry ("Polyface.ClipSphere");



    Check::Size ((size_t)allocationCounter, (size_t)BSIBaseGeom::GetAllocationDifference ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, Disk)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = SetTransformToNewGridSpot (*builder, true);
    for (
            double zz = 0.0, ra = mySize, rb = mySize;
            zz < 10.5;
            zz += 1.0, ra *= 0.9, rb *= 0.8
            )
        {
        DEllipse3d ellipse = DEllipse3d::From (0,0,zz, ra,0,0, 0,rb,0, 0.0, msGeomConst_2pi);
        builder->Clear ();
        builder->AddFullDisk (ellipse);
        ExaminePolyface (builder->GetClientMeshR (), "EllipticDisk");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, CylinderSides)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = SetTransformToNewGridSpot (*builder, true);
    double zSize = 0.75 * mySize;
    DEllipse3d ellipseA = DEllipse3d::From (0,0,0, mySize,0,0,   0,mySize,0, 0.0, msGeomConst_2pi);
    DEllipse3d ellipseB = DEllipse3d::From (0,0,zSize, mySize,0,0,   0,mySize,0, 0.0, msGeomConst_2pi);
    builder->AddRuled (ellipseA, ellipseB, false);
    ExaminePolyface (builder->GetClientMeshR (), "CylinderSides");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, CylinderCapped)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = SetTransformToNewGridSpot (*builder, true);
    double zSize = 0.75 * mySize;
    DEllipse3d ellipseA = DEllipse3d::From (0,0,0, mySize,0,0,   0,mySize,0, 0.0, msGeomConst_2pi);
    DEllipse3d ellipseB = DEllipse3d::From (0,0,zSize, mySize,0,0,   0,mySize,0, 0.0, msGeomConst_2pi);
    builder->AddRuled (ellipseA, ellipseB, true);
    ExaminePolyface (builder->GetClientMeshR (), "CappedCylinder");

    PolyfaceHeaderPtr closedCylinderMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    builder->GetClientMeshR ().CopyTo (*closedCylinderMesh);


    SetTransformToNewGridSpot (*builder);
    builder->Clear ();
    builder->AddRuled (ellipseA, ellipseB, false);
    ExaminePolyface (builder->GetClientMeshR (), "OpenCylinder");

    PolyfaceHeader &header = builder->GetClientMeshR ();
    DRange3d range = DPoint3dOps::Range (&header.Point ());
    DPlane3d sectionPlane = DPlane3d::FromOriginAndNormal (DPoint3d::FromInterpolate (range.low, 0.5, range.high),
                    DVec3d::From (0,0,1));
    CurveVectorPtr section0 = header.PlaneSlice (sectionPlane, true);

    
    size_t numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisibleA, numInvisible, numVisibleB, numVisibleC;
    closedCylinderMesh->Triangulate ();  // make sure there are interior edges in the caps.
    closedCylinderMesh->MarkAllEdgesVisible ();
    closedCylinderMesh->CollectCounts (numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisibleA, numInvisible);
    // really tight angle to get caps invisible ...
    closedCylinderMesh->MarkInvisibleEdges (1e-8);
    closedCylinderMesh->CollectCounts (numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisibleB, numInvisible);    
    // looser angle to hide sides ...
    closedCylinderMesh->MarkInvisibleEdges (1.0);
    closedCylinderMesh->CollectCounts (numVertex, numFacet, numTri, numQuad, numImplicitTri, numVisibleC, numInvisible);
    Check::True (numVisibleB < numVisibleA, "hide edges within caps");
    Check::True (numVisibleC < numVisibleB, "hide cone side edges");
    }

void TestSkewedCone (char const* title, bool capped)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetMaxPerFace (3);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = SetTransformToNewGridSpot (*builder);
    double zSize = 0.75 * mySize;
    DEllipse3d ellipseA = DEllipse3d::From (0,0,0, mySize,0,0,   0,mySize,0, 0.0, msGeomConst_2pi);
    DEllipse3d ellipseB = DEllipse3d::From (0,0,zSize,
                                0.25 * mySize,0.2*mySize,0,
                                0,0.3*mySize,0.1*mySize, 0.0,
                                msGeomConst_2pi);
    builder->AddRuled (ellipseA, ellipseB, capped);
    builder->GetClientMeshR ().Triangulate ();
    ExaminePolyface (builder->GetClientMeshR (), title);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, WackyCone)
    {
    TestSkewedCone ("Uncapped Skew cone", false);
    TestSkewedCone ("Capped Skew cone", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, BsplineSurface)
    {
    // ?? Somebody left it hanging ..
    Check::SetTransform (Transform::FromIdentity ());
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    double mySize = 4.0;
    bvector<DPoint3d> points;
    int numU = 5;
    int numV = 7;
    double dx = mySize / numU;
    double dy = mySize / numV;
    for (int j = 0; j < numV; j++)
        {
        for (int i = 0; i < numU; i++)
            {
            points.push_back (DPoint3d::FromXYZ (i * dx, j * dy, 0.0));
            }
        }
    auto surface = MSBsplineSurface::CreateFromPolesAndOrder (
            points, NULL,
            NULL, 3, numU, false,
            NULL, 4, numV, false,
            false);
    builder->Add (*surface);
    ExaminePolyface (builder->GetClientMeshR (), "BsplineSurface");
    CheckSurfaceMesh (*surface, *builder);
    Check::SaveTransformed (surface);
    Check::Shift (43.2, 50.0);      // Just to match regression
    Check::SaveTransformed (builder->GetClientMeshR ());
    Check::ClearGeometry ("PolyfaceConstruction.BsplineSurface");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, BsplineSurface1)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder);

    bvector<DPoint3d> points;

    points.push_back (DPoint3d::From (115.8875, -78.5876, 19.05));
    points.push_back (DPoint3d::From (115.8875, -78.5876, -19.05));
    points.push_back (DPoint3d::From (115.8875, -72.2376, -19.05));
    points.push_back (DPoint3d::From (115.8875, -72.2376, 19.05));
    points.push_back (DPoint3d::From (115.8875, -78.5876, 19.05));

    points.push_back (DPoint3d::From (77.7875, -78.5876, 19.05));
    points.push_back (DPoint3d::From (77.7875, -78.5876, -19.05));
    points.push_back (DPoint3d::From (84.1375, -72.2376, -19.05));
    points.push_back (DPoint3d::From (84.1375, -72.2376, 19.05));
    points.push_back (DPoint3d::From (77.7875, -78.5876, 19.05));

    points.push_back (DPoint3d::From (77.7875, 78.5876, 19.05));
    points.push_back (DPoint3d::From (77.7875, 78.5876, -19.05));
    points.push_back (DPoint3d::From (84.1375, 84.9376, -19.05));
    points.push_back (DPoint3d::From (84.1375, 84.9376, 19.05));
    points.push_back (DPoint3d::From (77.7875, 78.5876, 19.05));

    points.push_back (DPoint3d::From (58.7375, 78.5876, 19.05));
    points.push_back (DPoint3d::From (58.7375, 78.5876, -19.05));
    points.push_back (DPoint3d::From (58.7375, 84.9376, -19.05));
    points.push_back (DPoint3d::From (58.7375, 84.9376, 19.05));
    points.push_back (DPoint3d::From (58.7375, 78.5876, 19.05));
    // 20 Points

    int uPoleCount = 5;
    int vPoleCount = 4;
    bvector<double> weights;
    bvector<double> uknots;
    bvector<double> vknots;

    uknots.push_back (0);
    uknots.push_back (0);
    uknots.push_back (0.4285714285714290);
    uknots.push_back (0.5);
    uknots.push_back (0.928571428571429);
    uknots.push_back (1);
    uknots.push_back (1);

    vknots.push_back (0);
    vknots.push_back (0);
    vknots.push_back (0.177767243422612);
    vknots.push_back (0.911116378288694);
    vknots.push_back (1);
    vknots.push_back (1);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder (points, NULL,
                &uknots, (int)uknots.size () - uPoleCount, uPoleCount, false,
                &vknots, (int)vknots.size () - vPoleCount, vPoleCount, false, false);

    DMatrix4d products;
    surface->ComputeSecondMomentAreaProducts (products);
    builder->Add (*surface);
    ExaminePolyface (builder->GetClientMeshR (), "BsplineSurface");
//    CheckSurfaceMesh (*surface, *builder);        // This is a goofy surface -- don't check normals
    Check::SaveTransformed (surface);
    Check::ShiftToLowerRight ();
    Check::SaveTransformed (builder->GetClientMeshR ());
    Check::ClearGeometry ("PolyfaceConstruction.BsplineSurface1");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, BsplineSurface2)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder);

    bvector<DPoint3d> points;

    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,0,0));
    points.push_back (DPoint3d::From (10,0,0));


    points.push_back (DPoint3d::From (0,1,0));
    points.push_back (DPoint3d::From (1,1,0));
    points.push_back (DPoint3d::From (10,1,0));
    // 20 Points

    int uPoleCount = 3;
    int vPoleCount = 2;
    bvector<double> weights;
    bvector<double> uknots;
    bvector<double> vknots;

    uknots.push_back (0);
    uknots.push_back (0);
    uknots.push_back (0.4285714285714290);
    uknots.push_back (1);
    uknots.push_back (1);

    vknots.push_back (0);
    vknots.push_back (0);
    vknots.push_back (1);
    vknots.push_back (1);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder (points, NULL,
                &uknots, (int)uknots.size () - uPoleCount, uPoleCount, false,
                &vknots, (int)vknots.size () - vPoleCount, vPoleCount, false, false);

    DMatrix4d products;
    surface->ComputeSecondMomentAreaProducts (products);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, BsplineSurface3)
    {
    //varunused double mySize = SetTransformToNewGridSpot (*builder);

    bvector<DPoint3d> points;

    double ax = 6.0;
    double ay = 8.0;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (ax,0,0));
    points.push_back (DPoint3d::From (2.0 * ax,0,0));

    double b = 1.0; // mesh size
    points.push_back (DPoint3d::From (0,ay,0));
    points.push_back (DPoint3d::From (ax,ay,0));
    points.push_back (DPoint3d::From (2.0 * ax ,ay,0));
    // 20 Points

    int uPoleCount = 3;
    int vPoleCount = 2;
    bvector<double> weights;
    bvector<double> uknots;
    bvector<double> vknots;

    uknots.push_back (0);
    uknots.push_back (0);
    uknots.push_back (0);
    uknots.push_back (1);
    uknots.push_back (1);
    uknots.push_back (1);

    vknots.push_back (0);
    vknots.push_back (0);
    vknots.push_back (1);
    vknots.push_back (1);

    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder (points, NULL,
                &uknots, (int)uknots.size () - uPoleCount, uPoleCount, false,
                &vknots, (int)vknots.size () - vPoleCount, vPoleCount, false, false);

    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    options->SetMaxEdgeLength (b);
    builder->Add (*surface);    
    CheckSurfaceMesh (*surface, *builder);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, BsplineSurfaceEdgeHiding)
    {
        // Sort of a dtm ...
    size_t numX = 15;
    size_t numY = 20;
    auto surface = SurfaceWithSinusoidalControlPolygon (4, 5, numX, numY,    0.0, 0.3, 0.0, -0.25);
    DRange3d surfaceRange;
    surface->GetPoleRange (surfaceRange);
    double dX = surfaceRange.XLength () + 1;
    double dY = surfaceRange.YLength () + 2;
    Check::SaveTransformed (surface);
    IFacetOptionsPtr options = CreateFacetOptions ();
    Check::Shift (dX, 0, 0);
    for (bool smooth : bvector<bool> {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0,dY,0);
        options->SetSmoothTriangleFlowRequired (smooth);
        for (int hiding : bvector <int>{0,1,2})
            {
           SaveAndRestoreCheckTransform shifter (dX,0,0);
            options->SetBsplineSurfaceEdgeHiding (hiding);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            builder->Add (*surface);
            Check::SaveTransformed (builder->GetClientMeshR ());
            Check::ShiftToLowerRight (1.0);
            }
        }
    Check::ClearGeometry ("PolyfaceConstruction.BsplineSurfaceEdgeHiding");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, Bspline1)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (-5.76707, 2.71907, 13.8176));

    points.push_back (DPoint3d::From (-5.76707, 2.71907, 13.8092411228027));
    points.push_back (DPoint3d::From (-5.77820692307692, 2.73020692307692, 13.7950953306226));
    points.push_back (DPoint3d::From (-5.796026, 2.48026, 13.792523368408));

    points.push_back (DPoint3d::From (-5.803265, 2.755265, 13.7967028070067));

    points.push_back (DPoint3d::From (-5.810504, 2.762504, 13.8008822456053));

    points.push_back (DPoint3d::From (-5.81718615384615, 2.76918615384615, 13.8176));
    points.push_back (DPoint3d::From (-5.810504, 2.762504, 13.8343177543947));
    points.push_back (DPoint3d::From (-5.803265, 2.755265, 13.8384971929933));

    points.push_back (DPoint3d::From (-5.796026, 2.748026, 13.842676631592));
    points.push_back (DPoint3d::From (-5.77820692307692, 2.73020692307692, 13.8401046693774));
    points.push_back (DPoint3d::From (-5.76707, 2.71907, 13.8259588771973));

    points.push_back (DPoint3d::From (-5.76707, 2.71907, 13.8176));
    // 13 points
    bvector<double>weights;
    weights.push_back (1.0);
    weights.push_back (0.83333333333333326);
    weights.push_back (0.72222222222222221);
    weights.push_back (0.83333333333333326);
    weights.push_back (1.0);
    weights.push_back (0.83333333333333348);
    weights.push_back (0.7222222222222221);
    weights.push_back (0.83333333333333326);
    weights.push_back (1.0);
    weights.push_back (0.83333333333333326);
    weights.push_back (0.72222222222222221);
    weights.push_back (0.83333333333333326);
    weights.push_back (1.0);

    // 13 weights

    bvector<double>knots;
    knots.push_back (0.0);
    knots.push_back (0.0);
    knots.push_back (0.0);
    knots.push_back (0.11111111111111116);
    knots.push_back (0.22222222222222221);
    knots.push_back (0.33333333333333337);
    knots.push_back (0.33333333333333337);
    knots.push_back (0.44444444444444442);
    knots.push_back (0.55555555555555558);
    knots.push_back (0.66666666666666674);
    knots.push_back (0.66666666666666674);
    knots.push_back (0.77777777777777779);
    knots.push_back (0.88888888888888884);
    knots.push_back (1.0);
    knots.push_back (1.0);
    knots.push_back (1.0);
    // 16 knots

    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder(points, &weights, &knots, 3, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceHeader, BsplineCurve2)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0.0587375, 0.0785876, 0.01905));
    points.push_back (DPoint3d::From (0.0587375, 0.0849376, 0.01905));
    points.push_back (DPoint3d::From (0.0587375, 0.0849376, -0.01905));
    points.push_back (DPoint3d::From (0.0587375, 0.0785876, -0.01905));
    points.push_back (DPoint3d::From (0.0587375, 0.0785876, 0.01905));
    // 5 points

    bvector<double>weights;
    bvector<double>knots;
    knots.push_back (0.0);
    knots.push_back (0.0);
    knots.push_back (0.0714285714285714);
    knots.push_back (0.5);
    knots.push_back (0.5714285714285714);
    knots.push_back (1.0);
    knots.push_back (1.0);

    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (points, &weights, &knots, 2, false, false);

    //BSplineCurveElement::CreateBSplineCurveElement(dgnModel, null, curve);
    }



// holeSelect = (-1,0,1) for (CW,no,CCW) hole.
void testSweptLinestrings (size_t numContour, double z0, double zStep, bool capped, int numPerFace, int holeSelect)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetMaxPerFace (numPerFace);

    IPolyfaceConstructionPtr builder =  IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder);

    bvector<DPoint3d> points;
    double a0 = 0;
    double a1 = 3;
    double b0 = 1;
    double b1 = 2;
    points.push_back (DPoint3d::FromXYZ (a0,a0,z0));
    points.push_back (DPoint3d::FromXYZ (a1,a0,z0));
    points.push_back (DPoint3d::FromXYZ (a1,a1,z0));
    points.push_back (DPoint3d::FromXYZ (a0,a1,z0));
    points.push_back (DPoint3d::FromXYZ (a0,a0,z0));
    //points.push_back (DPoint3d::FromXYZ (3,2,z0));
    //points.push_back (DPoint3d::FromXYZ (3,0,z0));
    //points.push_back (DPoint3d::FromXYZ (1,0,z0));
    bvector<CurveVectorPtr> contours;
    Transform zTransform = Transform::From (0,0,zStep);
    if (holeSelect == 0)
        {
        for (size_t i = 0; i < numContour; i++)
            {
            contours.push_back (CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer));
            DPoint3dOps::Multiply (&points, zTransform);
            }
        }
    else
        {
        bvector<DPoint3d> holeXYZ;
        holeXYZ.push_back (DPoint3d::FromXYZ (b0,b0,z0));
        holeXYZ.push_back (DPoint3d::FromXYZ (b1,b0,z0));
        holeXYZ.push_back (DPoint3d::FromXYZ (b1,b1,z0));
        holeXYZ.push_back (DPoint3d::FromXYZ (b0,b1,z0));
        holeXYZ.push_back (DPoint3d::FromXYZ (b0,b0,z0));
        CurveVectorPtr outer = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
        if (holeSelect < 0)
            DPoint3dOps::Reverse (holeXYZ);
        CurveVectorPtr hole  = CurveVector::CreateLinear (holeXYZ, CurveVector::BOUNDARY_TYPE_Inner);
        CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
        parityRegion->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*outer));
        parityRegion->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*hole));

        for (size_t i = 0; i < numContour; i++)
            {
            contours.push_back (parityRegion);
            parityRegion = parityRegion->Clone ();
            parityRegion->TransformInPlace (zTransform);
            }

        }

    builder->AddRuledBetweenCorrespondingCurves (contours, capped);
    char message[2048];
    sprintf (message, "(Linestring (contours %d) (z0 %g) (zStep %g) (capped %d) (numPerFace %d)\n",
                (int)numContour, z0, zStep, capped ? 1 : 0, (int)numPerFace);
    PolyfaceHeaderR mesh = builder->GetClientMeshR ();

    PrintPolyface (mesh, message, stdout, s_maxPolyfacePrint, false);
    ExaminePolyface (mesh, "Linestring Contours");
    if (capped)
        {
        if (Check::True (mesh.IsClosedByEdgePairing (), "Closed sweep"))
            {
            DPoint3d origin = DPoint3d::From (0, 0, 0);
            double volume = mesh.SumTetrahedralVolumes (origin);
            double da = a1 - a0;
            double db = b1 - b0;
            double baseArea = da * da;
            if (holeSelect != 0)
                baseArea -= db * db;
            double expectedVolume = baseArea * (numContour - 1) * zStep;
            Check::Near (expectedVolume, volume, "volume");
            }                
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef abc
TEST(PolyfaceConstruction, Box)
    {
    bvector<DPoint3d> point0, point1;
    point0.push_back (DPoint3d::From(1000, 1000, 0));
    point0.push_back (DPoint3d::From(1000, -1000, 0));
    point0.push_back (DPoint3d::From(-1000, -1000, 0));
    point0.push_back (DPoint3d::From(-1000, 1000, 0));
    point0.push_back (DPoint3d::From(1000, 1000, 0));

    point1.push_back (DPoint3d::From(1000, 1000, 1000));
    point1.push_back (DPoint3d::From(1000, -1000, 1000));
    point1.push_back (DPoint3d::From(-1000, -1000, 1000));
    point1.push_back (DPoint3d::From(-1000, 1000, 1000));
    point1.push_back (DPoint3d::From(1000, 1000, 1000));

    bvector<CurveVectorPtr> curves;

    List<CurveVector> cList = new List<CurveVector>();
    CurveVector cv = null;
    ComplexShapeElement shape = (ComplexShapeElement)ComplexShapeElement.CreateChainHeaderElement(dgnModel, null, true);
    shape.AddComponentElement(LineStringElement.CreateLineStringElement(dgnModel, null, points));
    shape.AddComponentComplete();
    cv = shape.GetCurveVector();
    cList.Add(cv);
    shape.DeleteFromModel();
    shape = (ComplexShapeElement)ComplexShapeElement.CreateChainHeaderElement(dgnModel, null, true);
    shape.AddComponentElement(LineStringElement.CreateLineStringElement(dgnModel, null, points2));
    shape.AddComponentComplete();
    cv = shape.GetCurveVector();
    cList.Add(cv);
    shape.DeleteFromModel();

    pc.AddRuledBetweenCorrespondingCurves(cList, true);
    MeshHeaderElement mesh = MeshHeaderElement.CreateMeshElement(dgnModel, null, pc.GetClientMesh());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,SweptLinestring)
    {
    testSweptLinestrings (2, 0,1, false, 3, 0);
    testSweptLinestrings (2, 0,1, false, 4, 0);

//    testSweptLinestrings (2, 0,1, false, 3);
    testSweptLinestrings (2, 0,1, true, 4,  0);
    testSweptLinestrings (2, 0,1, true, 4, -1);
    testSweptLinestrings (2, 0,1, true, 4,  1);


    testSweptLinestrings (3, 0, 2, true, 3, 0);
    testSweptLinestrings (3, 0, 2, true, 4, 0);
    Check::ClearGeometry ("Polyface.SweptLineString");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, LinearSweep)
    {
    IFacetOptionsPtr options = CreateFacetOptions ();
    IPolyfaceConstructionPtr builder =  IPolyfaceConstruction::Create (*options);
    double mySize = 1.0;
    mySize = SetTransformToNewGridSpot (*builder, true);

    bvector<DPoint3d> point;
    bvector<DVec3d>tangent;
    DVec3d sweepVector = DVec3d::From (0,0, 1);
    int numPoint = 4;
    double dTheta = msGeomConst_pi * 0.25;
    for (int i = 0; i < numPoint; i++)
        {
        double theta = i * dTheta;
        DPoint3d xyz = DPoint3d::FromXYZ (cos (theta), sin(theta), 0.0);
        DVec3d dir = DVec3d::From (-sin(theta), cos(theta), 0.0);
        point.push_back (xyz);
        tangent.push_back (dir);
        if (    i > 0
            && (i % 3) == 0
            &&  i != numPoint - 1)
            {
            // make it hard by pushing some duplicates
            point.push_back (xyz);
            tangent.push_back (dir);
            }
        }
    builder->AddLinearSweep (point, tangent, sweepVector);
    ExaminePolyface (builder->GetClientMeshR (), "LinearSweep");
    }

void CheckStack
(
IPolyfaceConstruction &builder,
bool reverseIndices,
bool reverseNormals,
double localToWorldScale,
bool isTransformed
)
    {
    Check::Bool (reverseIndices, builder.GetReverseNewFacetIndexOrder ());
    Check::Bool (reverseNormals, builder.GetReverseNewNormals ());
    Check::Near (localToWorldScale, builder.GetLocalToWorldScale ());
    Check::Near (1.0, builder.GetLocalToWorldScale () * builder.GetWorldToLocalScale ());
    Check::Bool (isTransformed, builder.IsTransformed ());
    Transform localToWorld, worldToLocal;
    builder.GetLocalToWorld (localToWorld);
    builder.GetWorldToLocal (worldToLocal);
    Transform product;
    product.InitProduct (localToWorld, worldToLocal);
    Check::True (product.IsIdentity ());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (PolyfaceConstruction, Stack)
    {
    Transform translation = Transform::FromRowValues (
                1,0,0,1,
                0,1,0,2,
                0,0,1,3);
                
    double a = 2.0;
    Transform uniformScale = Transform::FromRowValues (
                a,0,0,0,
                0,a,0,0,
                0,0,a,0);
    //varunused double b = 5.0;
    //varunused Transform skew = Transform::FromRowValues (
    //varunused             a,a,0,0,
    //varunused             0,a,0,0,
    //varunused             0,0,b,0);



    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    Check::False (builder->PopState ());
    CheckStack (*builder, false, false, 1.0, false);
    builder->PushState (false);
    CheckStack (*builder, false, false, 1.0, false);
    builder->SetLocalToWorld (translation);
    CheckStack (*builder, false, false, 1.0, true);
    builder->SetLocalToWorld (uniformScale);
    CheckStack (*builder, false, false, a, true);
    builder->SetReverseNewFacetIndexOrder (true);
    CheckStack (*builder, true, false, a, true);
    builder->SetReverseNewNormals (true);
    CheckStack (*builder, true, true, a, true);
    builder->PushState ();
    builder->ApplyLocalToWorld (translation);
    builder->ApplyLocalToWorld (uniformScale);
    CheckStack (*builder, true, true, a * a, true);
    Check::True (builder->PopState ());
    Check::True (builder->PopState ());
    Check::False (builder->PopState ());
   }

static int s_numEdgePerSection = 6;
static int s_numSectionEdge = 4;

MSBsplineCurvePtr CreateTestCenterline ()
    {
    bvector<DPoint3d> poles;
    // Build a test centerline extending along x axis with a slide up to y1 along the way.
    double y1 = 3.0;
    double step = 10.0;
    poles.push_back (DPoint3d::From( 0,0,0));
    poles.push_back (DPoint3d::From(0,0,step));
    poles.push_back (DPoint3d::From(0,y1,2 * step));
    poles.push_back (DPoint3d::From(0,y1,3 * step));
    return MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 4, false, false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, Tube)
    {
    double circleRadius = 1.0;
    DVec3d centerlinePlaneNormal = DVec3d::From (0,0,1);
    MSBsplineCurvePtr centerlineCurve = CreateTestCenterline ();

    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetAngleTolerance (Angle::DegreesToRadians (15));
    IPolyfaceConstructionPtr builder = PolyfaceConstruction::Create (*options);
    bvector<CurveVectorPtr> sections;
    bvector<DPoint3d> pointGrid;
    for (int i = 0; i <= s_numSectionEdge; i++)
        {
        double fraction = i / (double)s_numSectionEdge;
        DPoint3d centerlinePoint;
        DVec3d   centerlineTangent;
        centerlineCurve->FractionToPoint (centerlinePoint, centerlineTangent, fraction);
        DVec3d vector90;
        vector90.NormalizedCrossProduct (centerlineTangent, centerlinePlaneNormal);
        DEllipse3d circle = DEllipse3d::FromScaledVectors (centerlinePoint, centerlinePlaneNormal, vector90, circleRadius, circleRadius, 0.0, Angle::TwoPi ());
        CurveVectorPtr sectionCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        sectionCurve->push_back (ICurvePrimitive::CreateArc (circle));
        sections.push_back (sectionCurve);

        for (int k = 0; k <= s_numEdgePerSection; k++)
            {
            DPoint3d xyz;
            circle.FractionParameterToPoint (xyz, (double)k/(double)s_numEdgePerSection);
            pointGrid.push_back (xyz);
            }
        }

//    builder->AddRuledBetweenCorrespondingCurves (sections, false);
    builder->AddRowMajorQuadGrid (&pointGrid[0], NULL, NULL, s_numEdgePerSection + 1, s_numSectionEdge + 1, true);
    PolyfaceHeaderR mesh = builder->GetClientMeshR ();
    double area = mesh.SumFacetAreas ();
    printf ("Tube Surface area %lf\n", area);
    printf (" Nominal area %lf\n", Angle::TwoPi () * circleRadius * centerlineCurve->Length ());
PrintPolyface (mesh, "Tube Surface", stdout, s_maxPolyfacePrint);
    }

//! Build a grid of quads using points order "along rows" in an array.
//! @param [in] builder mesh builder
//! @param [in] points array of points
//! @param [in] numPerRow number of points per row
//! @param [in] numRow number of rows
void AddRowMajorQuadGrid
(
IPolyfaceConstructionPtr builder,
DPoint3dCP points,
DVec3dCP normals,
size_t numPerRow,
size_t numRow
)
    {
    bvector<size_t> index;
    size_t n = numPerRow * numRow;
    index.reserve (n);
    builder->FindOrAddPoints (points, n, index);
    for (size_t j1 = 1; j1 < numRow; j1++)
        {
        size_t j0 = j1 - 1;
        for (size_t i1 = 1; i1 < numPerRow; i1++)
            {
            size_t i0 = i1 - 1;
            builder->AddPointIndexQuad (  index[j0 * numPerRow + i0], j0 == 0,
                                index[j0 * numPerRow + i1], i1 == numPerRow - 1,
                                index[j1 * numPerRow + i1], j1 == numRow - 1,
                                index[j1 * numPerRow + i0], i0 == 0
                                );
            }
        }

    if (normals != NULL)
        {
        size_t n = numPerRow * numRow;
        builder->FindOrAddNormals (normals, n, index);
        for (size_t j1 = 1; j1 < numRow; j1++)
            {
            size_t j0 = j1 - 1;
            for (size_t i1 = 1; i1 < numPerRow; i1++)
                {
                size_t i0 = i1 - 1;
                builder->AddNormalIndexQuad (  index[j0 * numPerRow + i0],
                                    index[j0 * numPerRow + i1],
                                    index[j1 * numPerRow + i1],
                                    index[j1 * numPerRow + i0]
                                    );
                }
            }
        }
    }


// Add facets to a mesh.
// Facets approximate a tube around a centerline.
// The centerline curve (bspline) should be planar or nearly so.
//  (If it is not, the successive circular sections may pinch in strange ways)
//! @param [in] builder mesh builder
//! @param [in] centerlineCurve tube centerline
//! @param [in] radius tube radius
//! @param [in] numEdgePerSection number of edges around each section circle
//! @param [in] numSectionEdge number of edges along curve.
void AddTubeMesh
(
IPolyfaceConstructionPtr builder,
MSBsplineCurveCR centerlineCurve,
double radius,
int numEdgePerSection,  // points per section
int numSectionEdge,     // sections
bool normals
)
    {
    DVec3d tangentA, tangentB;
    // Evalaute tangents at more or less random points to get normal to the plane of the curve.
    DPoint3d pointA, pointB;
    centerlineCurve.FractionToPoint (pointA, tangentA, 0.0);
    centerlineCurve.FractionToPoint (pointB, tangentB, 0.12318);
    

    DVec3d planeNormal;
    if (tangentA.IsParallelTo (tangentB))
        {
        DVec3d vecX, vecY, vecZ;
        tangentA.GetNormalizedTriad (vecX, vecY, vecZ);
        planeNormal = vecX;
        }
    else
        planeNormal.NormalizedCrossProduct (tangentA, tangentB);
    
    // Build the sections ...
    bvector<DPoint3d> pointGrid;
    bvector<DVec3d> normalGrid;
    // build grid of points on the circles.
    // Each circle gets numEdge + 1 points.  (The builder will find the duplicated wraparound point.)
    for (int i = 0; i <= numSectionEdge; i++)
        {
        double curveFraction = i / (double)(numSectionEdge);
        DPoint3d centerlinePoint;
        DVec3d   centerlineTangent;
        centerlineCurve.FractionToPoint (centerlinePoint, centerlineTangent, curveFraction);
        DVec3d vector0, vector90;
        vector90.NormalizedCrossProduct (centerlineTangent, planeNormal);
        vector0.NormalizedCrossProduct (vector90, centerlineTangent);
        DEllipse3d circle = DEllipse3d::FromScaledVectors (centerlinePoint, vector0, vector90, radius, radius, 0.0, Angle::TwoPi ());

        for (int k = 0; k <= numEdgePerSection; k++)
            {
            DPoint3d xyz;
            DVec3d sectionTangent, sectionBinormal;
            double hoopFraction = (double)k/(double)numEdgePerSection;
            circle.FractionParameterToDerivatives (xyz, sectionTangent, sectionBinormal, hoopFraction);
            DVec3d normal;
            normal.NormalizedCrossProduct (sectionTangent, centerlineTangent);
            pointGrid.push_back (xyz);
            normalGrid.push_back (normal);
            }

        // To adjust to space curvature, rebuild out-of-plane normal with local direction
        planeNormal = vector0;
        }
    AddRowMajorQuadGrid (builder, &pointGrid[0],
                normals ? &normalGrid[0] : NULL,
                numEdgePerSection + 1, numSectionEdge+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, Tube2)
    {
    double circleRadius = 1.0;
    //DVec3d centerlinePlaneNormal = DVec3d::From (0,0,1);
    MSBsplineCurvePtr centerlineCurve = CreateTestCenterline ();
    int numEdgePerSection = s_numEdgePerSection;
    int numSectionEdge = s_numSectionEdge;
    

    // Set up to build a mesh ...
    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetAngleTolerance (Angle::DegreesToRadians (15));
    IPolyfaceConstructionPtr builder = PolyfaceConstruction::Create (*options);
    PolyfaceHeaderR mesh = builder->GetClientMeshR ();

    

    builder->AddTubeMesh (*centerlineCurve, circleRadius, numEdgePerSection, numSectionEdge);


    double area = mesh.SumFacetAreas ();
    double centerlineLength = centerlineCurve->Length ();
    printf ("Mesh Area %lf\n", area);
    printf ("Nominal Area from Centerline * sectionLength %lf\n",
                Angle::TwoPi () * circleRadius * centerlineLength);
PrintPolyface (mesh, "Tube Surface", stdout, s_maxPolyfacePrint, false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static int s_checkNormals = 1;
TEST(PolyfaceConstruction, DimpleNormals)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &points = mesh->Point ();
    bvector<int> &indices = mesh->PointIndex ();
    //varunused bvector<DVec3d> &normals = mesh->Normal ();
    //varunused bvector<int> &normalIndices = mesh->NormalIndex ();

    double dz = 0.01;
    points.push_back (DPoint3d::From (1,1,dz)); // raised off plane, but smooth enough for normal averaging.
    points.push_back (DPoint3d::From (1,2,0));
    points.push_back (DPoint3d::From (0,2,0));
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (2,0,0));
    points.push_back (DPoint3d::From (2,1,0));
    for (int i = 3; i < 7; i++)
        {
        indices.push_back (1);
        indices.push_back (i-1);
        indices.push_back (i);
        indices.push_back (0);
        }
    
    Check::True (mesh->BuildApproximateNormals (dz * 50, dz * 200), "Build normals");
    if (s_checkNormals)
        Check::Size (mesh->Point ().size (), mesh->Normal().size (), "All normals smoothed");
    // Introduce a sharp angle
    points[5].z += 1.0;
    Check::True (mesh->BuildApproximateNormals (dz * 50, dz * 200), "build normals with sharp angle");
    if (s_checkNormals)
        Check::Size (mesh->Point ().size () + 2, mesh->Normal().size (), "One crinkle triangle");


    }


DPoint3d ParamToXYZ (DPoint2d uv, double du = 0.0, double dv = 0.0)
    {
    static double s_ax = 10.0;
    static double s_ay = 20.0;
    static double s_bxx = 1.0;
    static double s_by;
    double x = s_ax * (uv.x + du);
    double y = s_ay * (uv.y + dv);
    return DPoint3d::From (x, y, s_bxx * x * x + s_by * y);
    }

DVec3d ParamToNormal (DPoint2d uv, double delta)
    {
    DPoint3d xyzX0 = ParamToXYZ (uv, -delta, 0.0);
    DPoint3d xyzX1 = ParamToXYZ (uv,  delta, 0.0);

    DPoint3d xyzY0 = ParamToXYZ (uv, 0.0, -delta);
    DPoint3d xyzY1 = ParamToXYZ (uv, 0.0,  delta);
    DVec3d ddu = DVec3d::FromStartEnd (xyzX0, xyzX1);
    DVec3d ddv = DVec3d::FromStartEnd (xyzY0, xyzY1);
    DVec3d normal = DVec3d::FromCrossProduct (ddu, ddv);
    normal.Normalize ();
    return normal;
    }


PolyfaceHeaderPtr CreateCoordinateGridSample0 (int numPerFace, int numPerRow, int numRow, double du, double dv)
    {
    PolyfaceHeaderPtr mesh =
        numPerFace == 3 ? PolyfaceHeader::CreateTriangleGrid (numPerRow) : PolyfaceHeader::CreateQuadGrid (numPerRow);
    BlockedVectorDPoint3dR points  = mesh->Point ();
    BlockedVectorDPoint2dR params  = mesh->Param ();
    BlockedVectorDVec3dR   normals = mesh->Normal ();

    points.SetActive  (true);
    params.SetActive  (true);
    normals.SetActive (true);
    for (int row = 0; row < numRow; row++)
        {
        for (int col = 0; col < numPerRow; col++)
            {
            DPoint2d param = DPoint2d::From (du * (double)col, dv * (double)row);
            DPoint3d point = ParamToXYZ (param);
            DVec3d normal = ParamToNormal (param, 0.01);
            points.push_back (point);
            params.push_back (param);
            normals.push_back (normal);
            }
        }
    return mesh;
    }

bool IsRectangle (bvector<DPoint2d> &uv)
    {
    if (uv.size () < 4)
        return false;
    double dx = uv[1].x - uv[0].x;
    double dy = uv[3].y - uv[0].y;

    DPoint2d uv1 = DPoint2d::From (uv[0].x + dx, uv[0].y);
    DPoint2d uv2 = DPoint2d::From (uv[0].x + dx, uv[0].y + dy);
    DPoint2d uv3 = DPoint2d::From (uv[0].x, uv[0].y + dy);

    return
           DPoint2dOps::AlmostEqual (uv[1], uv1)
        && DPoint2dOps::AlmostEqual (uv[2], uv2)
        && DPoint2dOps::AlmostEqual (uv[3], uv3);
    }

void TestGridSample (int numPerFace, int numPerRow, int numRow)
    {
    // Build triangle grid.  This is supposed to interpolate REALLY cleanly.
    double du = 0.8;
    double dv = 0.9;
    PolyfaceHeaderPtr mesh = CreateCoordinateGridSample0 (numPerFace, 4, 3, du, dv);

    DVec2d uvSize;
    DVec2d xySize;
    Check::True (mesh->TryGetMaxSingleFacetLocalXYLength (xySize), "xysize");
    Check::True (mesh->TryGetMaxSingleFacetParamLength (uvSize), "uvSize");
    Check::Near (DVec2d::From (du, dv), uvSize, "uvSize");
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (*mesh, false);
    
    visitor->Reset ();
    visitor->SetNumWrap (2);
    BlockedVectorDPoint3dR points  = visitor->Point ();
    BlockedVectorDPoint2dR params  = visitor->Param ();
    BlockedVectorDVec3dR   normals = visitor->Normal ();

    double u = 0.25;
    double v = 0.35;
    double w = 1.0 - u - v;
    ptrdiff_t oldReadIndex = -1;
    ptrdiff_t readIndex;
    for (;visitor->AdvanceToNextFace (); oldReadIndex = readIndex)
        {
        readIndex = (ptrdiff_t)visitor->GetReadIndex ();
        Check::True (readIndex > oldReadIndex, "Read index advances");
        FacetLocationDetail detail;
        if (numPerFace == 3)
            {
            DPoint2d uv = DPoint2d::FromSumOf (params[0], u, params[1], v, params[2], w);
            DPoint3d xyz = DPoint3d::FromSumOf (points[0], u, points[1], v, points[2], w);
            DVec3d normal = DVec3d::FromSumOf (normals[0], u, normals[1], v, normals[2], w);
            if (Check::True (visitor->TryParamToFacetLocationDetail (uv, detail), "ParamToLocation"))
                {
                Check::Ptrdiff (readIndex, detail.GetReadIndex (), "Param detail readindex");
                Check::Near (uv, detail.param, "ParamToDetail:param");            
                Check::Near (xyz, detail.point, "ParamToDetail:param");

                double maxDelta = DoubleOps::MaxAbs (
                            normals[0].AngleTo (normals[1]),
                            normals[1].AngleTo (normals[2]),
                            normals[2].AngleTo (normals[3]));

                if (Check::True (normal.DotProduct (detail.normal) > 0.0, "normal direction")) // miniminal correctness
                    {
                    double delta0 = normal.AngleTo (detail.normal);
                    Check::True (delta0 <= maxDelta, "interpolated normal"); // Should we just compare to barycentric normal?
                    }
                }
            }
        else if (IsRectangle (params))
            {
            DPoint2d uv = DPoint2d::FromInterpolateBilinear (params[0], params[1], params[3], params[2], u, v);
            DPoint3d xyz = DPoint3d::FromInterpolateBilinear (points[0], points[1], points[3], points[2], u, v);
            DVec3d normal = DVec3d::FromInterpolateBilinear (normals[0], normals[1], normals[3], normals[2], u, v);
            if (Check::True (visitor->TryParamToFacetLocationDetail (uv, detail), "ParamToLocation"))
                {
                Check::Ptrdiff (readIndex, detail.GetReadIndex (), "Param detail readindex");

                Check::Near (uv, detail.param, "ParamToDetail:param");            
                Check::Near (xyz, detail.point, "ParamToDetail:param");

                double maxDelta = DoubleOps::MaxAbs (
                            normals[0].AngleTo (normals[1]),
                            normals[1].AngleTo (normals[2]),
                            normals[2].AngleTo (normals[3]));

                if (Check::True (normal.DotProduct (detail.normal) > 0.0, "normal direction")) // miniminal correctness
                    {
                    double delta0 = normal.AngleTo (detail.normal);
                    Check::True (delta0 <= maxDelta, "interpolated normal"); // Should we just compare to barycentric normal?
                    }

                DRay3d ray;
                if (detail.TryGetPoint (ray.origin) && detail.TryGetNormal (ray.direction))
                    {
                    FacetLocationDetail detail1;
                    if (Check::True (visitor->TryDRay3dIntersectionToFacetLocationDetail (ray, detail1)))
                        {
                        Check::Ptrdiff (readIndex, detail.GetReadIndex (), "Ray detail readindex");
                        Check::Near (detail.point, detail1.point, "uv, ray pick match xyz");
                        Check::Near (detail.param, detail1.param, "uv, ray pick match uv");
                        }
                    }
                }
            }
        }
    // force triangulation ..
    mesh->Triangulate ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_FRAGMENT(PolyfaceVisitor,ParamToDetail)
    {
    TestGridSample (3, 6, 5);
    TestGridSample (4, 3, 2);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, NGonFaces)
    {
    //varunused Int64 allocationCounter = BSIBaseGeom::GetAllocationDifference ();
    //static int s_printGraph = 0;
    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetMaxPerFace (4);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    //varunused double mySize = SetTransformToNewGridSpot (*builder, true);

    builder->AddSweptNGon (4, 2.0, 0.0, 1.0, true, true);
    ExaminePolyface (builder->GetClientMeshR (), "DiamondFrustum");
    PrintPolyface (builder->GetClientMeshR (), "DiamondMesh", stdout, s_maxPolyfacePrint, true);

    }
void TestReversal (PolyfaceHeaderR mesh)
    {
    DVec3d eyeVector = DVec3d::From (0,0,1);
    size_t numPositiveA, numPositiveB;
    size_t numPerpendicularA, numPerpendicularB;
    size_t numNegativeA, numNegativeB;
    double areaA = mesh.SumDirectedAreas (eyeVector, numPositiveA, numPerpendicularA, numNegativeA);
    if (s_noisy)
        PrintPolyface (mesh, "Before Reversal", stdout, s_maxPolyfacePrint, true);
    mesh.ReverseIndicesAllFaces (true, true, true, BlockedVectorInt::None);
    if (s_noisy)
        PrintPolyface (mesh, "After Reversal", stdout, s_maxPolyfacePrint, true);
    double areaB = mesh.SumDirectedAreas (eyeVector, numPositiveB, numPerpendicularB, numNegativeB);
    Check::Near (areaA, -areaB);
    Check::Size (numPositiveA, numNegativeB);
    Check::Size (numPositiveB, numNegativeA);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ReverseIndexed)
    {
    PolyfaceHeaderPtr header = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &points = header->Point ();
    bvector<int> &pointIndex = header->PointIndex ();
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,0,0));
    points.push_back (DPoint3d::From (1,1,0));
    points.push_back (DPoint3d::From (0,1,0));
    pointIndex.push_back (1);
    pointIndex.push_back (2);
    pointIndex.push_back (-3);
    pointIndex.push_back (0);
    pointIndex.push_back (-1);
    pointIndex.push_back (3);
    pointIndex.push_back (4);
    pointIndex.push_back (0);
    TestReversal (*header);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ReverseCoordinate)
    {
    for (int numPerFace : bvector<int> {3,4})
        {
        PolyfaceHeaderPtr header = PolyfaceHeader::CreateFixedBlockCoordinates (numPerFace);
        bvector<DPoint3d> &points = header->Point ();
        for (size_t i = 0; i < 10; i += 2)
            {
            double x0 = (double)i;
            double x1 = x0 + 1;
            points.push_back (DPoint3d::From (x0,0,0));
            points.push_back (DPoint3d::From (x1,0,0));
            points.push_back (DPoint3d::From (x1,1,0));
            if (numPerFace == 4)
                points.push_back (DPoint3d::From (x0,1,0));
            }
        TestReversal (*header);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, ReverseCoordinateGrid)
    {
    int numPerRow = 5;
    for (int numPerFace : bvector<int> {3,4})
        {
        PolyfaceHeaderPtr header =
            numPerFace == 4 ? PolyfaceHeader::CreateQuadGrid (numPerRow) : PolyfaceHeader::CreateTriangleGrid (numPerRow);
        bvector<DPoint3d> &points = header->Point ();
        for (int row = 0; row < 4; row++)
            {
            for (int i = 0; i < numPerRow; i += 1)
                {
                points.push_back (DPoint3d::From ((double)i,(double)row,0));
                }
            }
        TestReversal (*header);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Polyface, Reverse1)
    {
    PolyfaceHeaderPtr headerA = PolyfaceHeader::CreateFixedBlockIndexed (4);
    bvector<DPoint3d> &points = headerA->Point ();
    bvector<int> &pointIndex = headerA->PointIndex ();
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,0,0));
    points.push_back (DPoint3d::From (1,1,0));
    points.push_back (DPoint3d::From (0,1,0));
    points.push_back (DPoint3d::From (-0.5, 0.5, 0));
    points.push_back (DPoint3d::From (0,2,0));
    points.push_back (DPoint3d::From (1,2,0));
//
//            7----6
//            |    |
//            4----3
//           /|    |
//          5 |    |
//           \|    |
//            1----2
    // true quad facet ..
    pointIndex.push_back (1);
    pointIndex.push_back (2);
    pointIndex.push_back (3);
    pointIndex.push_back (4);
    // triangle with trailing 0
    pointIndex.push_back (1);
    pointIndex.push_back (4);
    pointIndex.push_back (5);
    pointIndex.push_back (0);
    // another quad
    pointIndex.push_back (3);
    pointIndex.push_back (7);
    pointIndex.push_back (6);
    pointIndex.push_back (4);



    PolyfaceHeaderPtr headerB = headerA->CreateFixedBlockIndexed (4);
    headerA->CopyTo (*headerB);
    headerB->ReverseIndicesAllFaces ();

    PolyfaceHeaderPtr headerC = headerB->CloneAsVariableSizeIndexed (*headerB);
    PolyfaceHeaderPtr headerD = PolyfaceHeader::CreateVariableSizeIndexed ();
    headerC->CopyTo (*headerD);
    headerD->ReverseIndicesAllFaces ();

    Check::Size (headerA->PointIndex ().size (), headerB->PointIndex ().size ());
    // hm... what is the size comparison before after?
    // trailing zeros go away.
    // 1 zero is added to each face.
    Check::Size (headerC->PointIndex ().size (), headerD->PointIndex ().size ());
    auto visitorA = PolyfaceVisitor::Attach (*headerA); // FORWARD, blocked
    auto visitorB = PolyfaceVisitor::Attach (*headerB); // REVERSED, blocked
    auto visitorC = PolyfaceVisitor::Attach (*headerC); // REVERSED, variable
    auto visitorD = PolyfaceVisitor::Attach (*headerD); // FORWARD, variable
    visitorA->Reset ();
    visitorB->Reset ();
    visitorC->Reset ();
    visitorD->Reset ();
    double areaA, areaB, areaC, areaD;
    DVec3d normalA, normalB, normalB1, normalC, normalC1, normalD;
    DPoint3d centroidA, centroidB, centroidC, centroidD;
    for (;visitorA->AdvanceToNextFace () && visitorB->AdvanceToNextFace () && visitorC->AdvanceToNextFace () && visitorD->AdvanceToNextFace ();)
        {
        Check::True (visitorA->TryGetFacetCentroidNormalAndArea (centroidA, normalA, areaA));
        Check::True (visitorB->TryGetFacetCentroidNormalAndArea (centroidB, normalB, areaB));
        Check::True (visitorC->TryGetFacetCentroidNormalAndArea (centroidC, normalC, areaC));
        Check::True (visitorD->TryGetFacetCentroidNormalAndArea (centroidD, normalD, areaD));
        Check::Near (centroidA, centroidB);
        Check::Near (centroidA, centroidC);
        Check::Near (centroidA, centroidD);
        normalB1.Negate (normalB);
        normalC1.Negate (normalC);
        Check::Near (normalA, normalB1);
        Check::Near (normalA, normalC1);
        Check::Near (normalA, normalD);
        Check::Near (areaA, areaB);
        Check::Near (areaA, areaC);
        Check::Near (areaA, areaD);
        }
    // confirm all were exhausted together ...
    Check::False (visitorA->AdvanceToNextFace ());
    Check::False (visitorB->AdvanceToNextFace ());
    Check::False (visitorC->AdvanceToNextFace ());
    Check::False (visitorD->AdvanceToNextFace ());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_FRAGMENT(TestInstanceMacros,Test1)
    {
    printf ("TestInstanceMacrosTest1\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CubeMoments)
    {
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->AddSweptNGon (4, 2.0, -1.0, 1.0, true, true);
    builder->GetClientMeshR ().Triangulate ();
    double scaleX = 2.0;
    double scaleY = 1.0;
    PolyfaceHeaderR mesh = builder->GetClientMeshR ();
    
    
    DPoint3d centroidIn = DPoint3d::From (1,2,3);
    Transform T1 = Transform::FromRowValues
        (
        scaleX, 0,0,0,
        0, scaleY,0,0,
        0,0,1,0
        );
    mesh.Transform (T1);
    Transform T2 = Transform::FromAxisAndRotationAngle (DRay3d::FromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (0,0,1)), acos (0.95));
    mesh.Transform (T2);
    Transform T3 = Transform::From (centroidIn);
    mesh.Transform (T3);
    Transform identity = Transform::FromIdentity ();
    double volume;
    DVec3d moment1, moment1A;
    RotMatrix product2;
    DPoint3d origin = DPoint3d::From (0,0,0);
    double volume1A = mesh.SumTetrahedralFirstMoments (origin, moment1A);
    mesh.SumTetrahedralMomentProducts (identity, volume, moment1, product2);
#ifdef PrintSlabDetails
    double volume0 = mesh.SumTetrahedralVolumes (origin);
    printf (" Diamond slab (simple volume %lg) \n", volume0);
    printf (" Diamond slab (volume %lg) (moment1 %lg %lg %lg)\n", volume, moment1.x, moment1.y, moment1.z);
    printf (" (products (xx %lg) (yy %lg) (zz %lg)\n", product2.form3d[0][0], product2.form3d[1][1], product2.form3d[2][2]);
    printf (" (products (xy %lg) (xz %lg) (yz %lg)\n", product2.form3d[0][1], product2.form3d[0][2], product2.form3d[1][3]);
#endif
    DVec3d moment1B;
    double volume1B = mesh.SumTetrahedralFirstMoments (DPoint3d::From (-2,-3,4), moment1B);
    Check::Near (volume1A, volume1B, "volumes from several origins");
    DVec3d diagonalMomentsC;
    RotMatrix principalAxesC;
    DPoint3d centroidC;
    double volumeC;
    Check::True (mesh.ComputePrincipalMoments (volumeC, centroidC, principalAxesC, diagonalMomentsC), "ComputePrincipalMoments");
    Check::Near (centroidIn, centroidC, "centroid");
    Transform localToWorld = Transform::From (principalAxesC, centroidC);
    Transform worldToLocal;
    worldToLocal.InverseOf (localToWorld);
    double volumeD;
    DVec3d moment1D;
    RotMatrix product2D;
    mesh.SumTetrahedralMomentProducts (worldToLocal, volumeD, moment1D, product2D);
    Check::True (product2D.IsDiagonal (), "mixed products are zero in local system");
    }


// Verify curvature evaluations on surface known to be a cylinder around given axis.
void CheckCylinderCurvature (MSBsplineSurfaceCR surface, double u, double v, DRay3dCR axis)
    {
    DPoint3d xyz;
    DVec3d unitA, unitB;
    double kA, kB;
    Check::True (surface.EvaluatePrincipalCurvature (xyz, unitA, kA, unitB, kB, u, v), "Evaluate Principal curvatures");
    DPoint3d axisPoint;
    double axisParameter;
    axis.ProjectPointUnbounded (axisPoint, axisParameter, xyz);
    DVec3d myNormal = DVec3d::FromStartEnd (axisPoint, xyz);
    double r = myNormal.Magnitude ();

    Check::True (unitA.IsPerpendicularTo (unitB), "curvatures perpendicular");
    Check::True (fabs (kB) <= fabs (kA), "curvature sorting");
    Check::Near (1.0 / r,fabs (kA), "large curvature");
    Check::Near (0.0, kB, "large curvature");
    Check::True (myNormal.IsPerpendicularTo (unitA), "surface normal");
    Check::True (myNormal.IsPerpendicularTo (unitB), "surface normal");
    Check::True (unitA.IsPerpendicularTo (axis.direction), "cone axis");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, ConeNormals)
    {
    DEllipse3d e0 = DEllipse3d::From (0,0,0, 1,0,0, 0,1,0, 0.0, Angle::TwoPi ());
    DVec3d sweepVector = DVec3d::From (0,0,1);
    MSBsplineCurve c0;
    c0.InitFromDEllipse3d (e0);
    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateLinearSweep (c0, sweepVector);
    c0.ReleaseMem ();
    
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (4);
    builder->GetFacetOptionsR ().SetAngleTolerance (atan (1.0));
    builder->Add (*surface);
    CheckSurfaceMesh (*surface, *builder);
    
    PrintPolyface (builder->GetClientMeshR (), "Unit Cylinder", stdout, s_maxPolyfacePrint, false);
    
    DRay3d zAxis = DRay3d::FromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (0,0,1));
    CheckCylinderCurvature (*surface, 0.5, 0.5, zAxis);
    CheckCylinderCurvature (*surface, 0.25, 0.3, zAxis);
    double rScale = 3.0;
    double zScale = 2.5;
    Transform scale = Transform::FromRowValues
        (
        rScale, 0, 0, 0,
        0, rScale, 0, 0,
        0, 0, zScale, 0
        );
    MSBsplineSurfacePtr surface1 = surface->CreateCopyTransformed (scale);
    
    CheckCylinderCurvature (*surface1, 0.5, 0.5, zAxis);
    CheckCylinderCurvature (*surface1, 0.25, 0.3, zAxis);
    double yScale = sqrt (0.5);


    // Carefully skew so axis points along (1,0,0)
    // When looking down the axis, the x direction is shortened by root(0.5)
    // Force the same reduction in y size so the skew is still a cylinder.
    Transform skew = Transform::FromRowValues
        (
        1, 0, 1, 0,
        0, yScale, 0, 0,
        0, 0, 1, 0
        );
    
    DRay3d skewAxis = DRay3d::FromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (1,0,1));
    MSBsplineSurfacePtr surface2 = surface->CreateCopyTransformed (skew);
    CheckCylinderCurvature (*surface2, 0.5, 0.5, skewAxis);
    CheckCylinderCurvature (*surface2, 0.25, 0.3, skewAxis);
    }

void BlindCopyFaceLoops (PolyfaceHeaderR mesh, bvector<DPoint3d> &points)
    {
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (mesh, false);
        visitor->Reset ();
    for (;visitor->AdvanceToNextFace ();)
        {
        for (size_t i = 0; i < visitor->Point().size (); i++)
            points.push_back (visitor->Point()[i]);
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceCoordinateQuads, Test1)
    {
    int numPerFace = 4;
    double du = 1.0;
    double dv = 2.0;
    int numU = 3;
    int numV = 2;
    PolyfaceHeaderPtr mesh      =  PolyfaceHeader::CreateFixedBlockCoordinates (numPerFace);
    PolyfaceHeaderPtr quadGrid  =  CreateCoordinateGridSample0 (numPerFace, numU, numV, du, dv);
    BlindCopyFaceLoops (*quadGrid, mesh->Point());
    // blast it to indexed ...
    mesh->Triangulate ();
    }
    

void AddQuads (IPolyfaceConstructionPtr &builder, DPoint3d xyz00, DPoint3d xyz11, size_t numX, size_t numY)
    {
    DPoint3d xyz10 = xyz00;
    DPoint3d xyz01 = xyz00;
    xyz10.x = xyz11.x;
    xyz01.y = xyz11.y;
    DBilinearPatch3d patch (xyz00, xyz10, xyz01, xyz11);
    bvector<DPoint3d> points;
    double du = 1.0 / (double)numX;
    double dv = 1.0 / (double)numY;
    double area = 0.0;
    for (size_t i = 0; i < numX; i++)
        {
        for (size_t j = 0; j < numY; j++)
            {
            points.clear ();
            double u0 = i * du;
            double u1 = (i + 1) * du;
            double v0 = j * dv;
            double v1 = (j + 1) * dv;
            points.push_back (patch.Evaluate (u0, v0));
            points.push_back (patch.Evaluate (u1, v0));
            points.push_back (patch.Evaluate (u1, v1));
            points.push_back (patch.Evaluate (u0, v1));
            builder->AddTriangulation (points);
            area = builder->GetClientMeshR ().SumFacetAreas ();
            }
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceSplit, Connectivity1)
    {
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    double area0 = 0.0;
    AddQuads (builder, DPoint3d::From (0,0,0), DPoint3d::From (1,1,0), 2, 4);
    double area1 = builder->GetClientMeshR ().SumFacetAreas ();
    // Vertex contact ...
    AddQuads (builder, DPoint3d::From (1,1,0), DPoint3d::From (2,2,0), 1, 3);
    //double area2 = builder->GetClientMeshR ().SumFacetAreas ();
    // Edge contact
    AddQuads (builder, DPoint3d::From (2,1,0), DPoint3d::From (5,2,0), 2, 3);
    double area3 = builder->GetClientMeshR ().SumFacetAreas ();
    PolyfaceHeader& parentMesh = builder->GetClientMeshR ();
    bvector <PolyfaceHeaderPtr> vertexParts, edgeParts;
    parentMesh.PartitionByConnectivity (0, vertexParts);
    parentMesh.PartitionByConnectivity (1, edgeParts);
    Check::Size (1, vertexParts.size (), "vertex connectivity");
    if (Check::Size (2, edgeParts.size (), "vertex connectivity"))
        {
        double d0 = area1 - area0;
        double d1 = area3 - area1;
        double e0 = edgeParts[0]->SumFacetAreas ();
        double e1 = edgeParts[1]->SumFacetAreas ();
        Check::Near (DoubleOps::Min (d0, d1), DoubleOps::Min (e0, e1), "edge connection");
        Check::Near (DoubleOps::Max (d0, d1), DoubleOps::Max (e0, e1), "edge connection");
        }
    }

PolyfaceHeaderPtr BoxMesh (DPoint3d xyz00, double ax, double ay, double az)
    {
    ISolidPrimitivePtr box = ISolidPrimitive::CreateDgnBox
        (DgnBoxDetail (
            xyz00,
            DPoint3d::From (xyz00.x, xyz00.y, xyz00.z + az),
            DVec3d::From (1,0,0),
            DVec3d::From (0,1,0),
            ax, ay,
            ax, ay,
            true));
    IPolyfaceConstructionPtr builder = CreateBuilder (false, false);
    builder->GetFacetOptionsR ().SetMaxPerFace (10);
    builder->AddSolidPrimitive (*box);
    return builder->GetClientMeshPtr ();
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ClipPlaneSet1)
    {
    PolyfaceHeaderPtr box = BoxMesh (DPoint3d::From (0,0,0), 5.0, 3.0, 2.0);
    ClipPlaneSet clipper;
    bvector<DPoint3d> xyz;
    bvector<bool>hiddenEdge;
    xyz.push_back (DPoint3d::From (1,-1,0));
    xyz.push_back (DPoint3d::From (1,1,0));
    hiddenEdge.push_back (false);
    clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (xyz, hiddenEdge, true));

    Check::SaveTransformed (box);
    Check::Shift (0, 10, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            *box, clipper, true,
            &insideClip, &outsideClip);
    Check::SaveTransformed (*insideClip);
    Check::SaveTransformed (*outsideClip);

    Check::ClearGeometry ("Polyface.ClipPlaneSet1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ClipPlaneSet2)
    {
    PolyfaceHeaderPtr box = BoxMesh (DPoint3d::From (0,0,0), 5.0, 3.0, 2.0);
    ClipPlaneSet clipper;
    bvector<DPoint3d> xyz;
    bvector<bool>hiddenEdge;
    xyz.push_back (DPoint3d::From (1,-1,0));
    xyz.push_back (DPoint3d::From (1,1,0));
    xyz.push_back (DPoint3d::From (-1,1,0));
    hiddenEdge.push_back (true);
    hiddenEdge.push_back (false);

    clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (xyz, hiddenEdge, true));
    hiddenEdge.clear ();
    xyz.clear ();
    xyz.push_back (DPoint3d::From (8,8,0));
    xyz.push_back (DPoint3d::From (1,1,0));
    xyz.push_back (DPoint3d::From (1,-1,0));
    hiddenEdge.push_back (false);
    hiddenEdge.push_back (true);
    clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (xyz, hiddenEdge, true));

    Check::SaveTransformed (box);
    Check::Shift (0, 10, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            *box, clipper, true,
            &insideClip, &outsideClip);
    Check::SaveTransformed (*insideClip);
    Check::SaveTransformed (*outsideClip);

    Check::ClearGeometry ("Polyface.ClipPlaneSet2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,ClipRoadSurface)
    {
    // Make a bit of road .....
    auto road = BoxMesh (DPoint3d::From (0,0,0), 100.0, 10.0, 0.40);
    road->Triangulate ();
    ClipPlaneSet clipper;
    // Make a stripe . ..
    bvector<DPoint3d> stripe
        {
        DPoint3d::From (10.0, 0.5, 0),
        DPoint3d::From (10.5, 0.5, 0),
        DPoint3d::From (10.5, 5.0, 0),
        DPoint3d::From (10.0, 5.0, 0),
        DPoint3d::From (10.0, 0.5, 0),
        };
    // have to fill up a vector saying all edges matter ...
    bvector<bool> isHidden;
    for (size_t iStripe = 0; iStripe < stripe.size(); ++iStripe)
        isHidden.push_back (false);
    // Make some copies of the stripe ...
    for (double dx = 0.0; dx <= 40.0; dx += 10.0)
        {
        bvector <DPoint3d> stripe1 = stripe;
        for (auto &xyz : stripe1)
            xyz.x += dx;
        clipper.push_back (ConvexClipPlaneSet::FromXYPolyLine (stripe1, isHidden, true));
        }

    // Create a mesh and supporting CoordinateMap to catch the output . . .
    PolyfaceHeaderPtr clippedMesh = PolyfaceHeader::CreateVariableSizeIndexed ();


    bvector<bvector<ptrdiff_t>> readIndices;
    road->PartitionReadIndicesByNormal (DVec3d::From (0,0,1), readIndices);
    bvector<PolyfaceHeaderPtr> upSideDown;
    road->CopyPartitions (readIndices, upSideDown);
    Check::SaveTransformed (road);
    Check::Shift (0, 30, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            *upSideDown[0], clipper, false,
            &insideClip, &outsideClip);
    Check::SaveTransformed (*insideClip);
    Check::Shift (0, 15, 0);
    Check::SaveTransformed (*outsideClip);


    Check::ClearGeometry ("Polyface.StripeOnRoad");    
    }
#ifdef TestPolygonDecomp
template<typename T>
void ToBvector (T *data, int n, bvector<T> &bdata)
    {
    bdata.clear ();
    for (int i = 0; i < n; i++)
        bdata.push_back (data[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polygon,Decomposition)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (10,10,0));
    points.push_back (DPoint3d::From (5,10,0));
    points.push_back (DPoint3d::From (5,5,0));
    points.push_back (DPoint3d::From (4,5,0));
    points.push_back (DPoint3d::From (3,10,0));
    points.push_back (DPoint3d::From (0,10,0));
    points.push_back (DPoint3d::From (0,5,0));
    points.push_back (DPoint3d::From (2,5,0));
    points.push_back (DPoint3d::From (2,1,0));
    points.push_back (DPoint3d::From (0,1,0));
//    points.push_back (DPoint3d::From (0,0,0));

    bvector<ptrdiff_t> bIndices;
    Check::Print (points, "Points for polygon decomp");
    double tol = Angle::SmallAngle ();
    for (size_t n : bvector<size_t> {3, 4, 7, 10, 11, 12})
        {
        bvector<DPoint3d> point1;
        TaggedPolygon polygon1 (point1);
       for (size_t i = 0;i < n; i++)
            point1.push_back (points[i]);
        ClipPlaneTree tree;
        bsiPolygon_decomposeXYToXOR (bIndices, point1, tree);
        Check::Print (bIndices, "Decomp");
        for (double x = -0.39878234; x < 12.0; x += 0.5)
            {
            for (double y = -0.78234123; y < 12.0; y += 0.5)
                {
                DPoint3d xyz = DPoint3d::From (x, y, 0.0);
                int parity = bsiGeom_XYPolygonParity (&xyz, &point1.front (), (int)point1.size (), tol);
                bool inside = tree.IsPointInOrOn (xyz);
                Check::Bool (inside, parity >= 0);
                }
            }
        }

    }   
#endif


void Print (DoubleEndedQueue<size_t> &queue, CharCP message)
    {
    size_t n = queue.Size ();
    printf ("(%s (%d)   ", message, (int)n);
    for (size_t i = 0; i < n; i++)
        printf (" %d", (int)queue.GetFromLeft (i));
    printf (")\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DoubleEndedQueue,ForwardInsert)
    {
    size_t baseValue = 100;
    size_t maxEntry = 12;
    // unused - size_t badValue = 9999; // To be inserted when we know it should be full.
    DoubleEndedQueue<size_t> queue (INT_MAX);
    DoubleEndedQueue<size_t> queue1 (INT_MAX);
    // build up and teardown -- we know this creates non0 m_i0 . ..
    queue1.AddRight (1);
    queue1.AddRight(2);
    queue1.AddRight (3);
    queue1.PopLeft ();
    queue1.PopLeft ();
    queue1.PopRight ();
    Check::True (queue1.IsEmpty (), "empty");
    for (size_t i = 0; i < maxEntry; i++)
        {
        Check::Size (i, queue.Size ());
        queue.AddRight (baseValue + i);
        queue1.AddLeft (baseValue + maxEntry - 1 - i);
        Check::False (queue.IsEmpty ());
        }
    Check::True (queue == queue1, "Matched entries with reverse growth.");
    Print (queue, "Sequential Entries");

    for (size_t i = 0; i < maxEntry; i++)
        {
        size_t value0 = baseValue + i;
        size_t value1 = baseValue + maxEntry - i - 1;
        Check::Size (value0, queue.GetFromLeft (i));
        Check::Size (value1, queue.GetFromRight (i));
        Check::Size (queue1.GetCyclicFromLeft (i), queue.GetFromLeft (i));
        Check::Size (queue1.GetCyclicFromLeft (i + maxEntry), queue.GetFromLeft (i));
        Check::Size (queue1.GetCyclicFromLeft ((ptrdiff_t)i -(ptrdiff_t)maxEntry), queue.GetFromLeft (i));
        }
    }

double CrossXY (bvector<DPoint3d> const &points, size_t i0, size_t i1, size_t i2)
    {
    DPoint3d xyz0 = points[i0];
    DPoint3d xyz1 = points[i1];
    DPoint3d xyz2 = points[i2];
    double a =
                 (xyz1.x - xyz0.x) * (xyz2.y - xyz1.y)
                -(xyz1.y - xyz0.y) * (xyz2.x - xyz1.x);
    if (s_printHullSteps)
        printf (" %d %d %d   %g\n", (int)i0, (int)i1, (int)i2, a);
    return a;
    }
void PolylineToHull (
bvector<DPoint3d> const &sourcePoints,  //!< [in] coordinate array
bvector<size_t> &sourceIndices,         //!< [in] indices of coordinates to use
bvector<size_t> &hullIndices,           //!< [out] indices on convex hull.
DoubleEndedQueue <size_t> &queue        //!< [in,out] work queue
)
    {
    hullIndices.clear ();
    queue.Clear ();
    if (sourceIndices.size () < 2)
        return;
    // The queue has points on the hull, with first/last repeated.
    if (CrossXY (sourcePoints,
            sourceIndices[2],
            sourceIndices[1],
            sourceIndices[0]
            ) < 0.0)
        {
        queue.AddRight (sourceIndices[2]);
        queue.AddRight (sourceIndices[0]);
        queue.AddRight (sourceIndices[1]);
        queue.AddRight (sourceIndices[2]);
        }
    else
        {
        queue.AddRight (sourceIndices[2]);
        queue.AddRight (sourceIndices[1]);
        queue.AddRight (sourceIndices[0]);
        queue.AddRight (sourceIndices[2]);
        }
    for (size_t k = 3; k < sourceIndices.size (); k++)
        {
        size_t iNew = sourceIndices[k];
        size_t numLeft = 0;
        size_t numRight = 0;
        double aLeft = CrossXY (sourcePoints, iNew, queue.GetFromLeft(0), queue.GetFromLeft (1));
        double aRight = CrossXY (sourcePoints, iNew, queue.GetFromRight(0), queue.GetFromRight (1));
        if (aLeft > 0.0 && aRight < 0.0)
            continue;
        while (queue.Size () >= 4)
            {
            double a = CrossXY (sourcePoints, iNew, queue.GetFromLeft(0), queue.GetFromLeft (1));
            if (a >= 0.0)
                break;
            numLeft++;
            queue.PopLeft ();
            }
        while (queue.Size () >= 4)
            {
            double a = CrossXY (sourcePoints, iNew, queue.GetFromRight(0), queue.GetFromRight (1));
            if (a <= 0.0)
                break;
            numRight++;
            queue.PopRight ();
            }

    // no pops ==> new point is inside
    // pops ==> new point becomes the doubled point of the hull
    if (numLeft + numRight > 0)
            {
            queue.AddLeft (iNew);
            queue.AddRight (iNew);
            }
        }
    for (size_t i = 0, n = queue.Size () - 1; i < n; i++)
        hullIndices.push_back (queue.GetFromLeft (i));
    }

TaggedPolygon ExtractPolygon (bvector<DPoint3d> const &point, bvector<size_t> const &index)
    {
    TaggedPolygon polygon;
    for (auto i : index)
        polygon.Add (point[i]);
    return polygon;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConvexHull,FromPolyline)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (10,10,0));
    points.push_back (DPoint3d::From (5,10,0));
    points.push_back (DPoint3d::From (5,5,0));
    points.push_back (DPoint3d::From (4,5,0));
    points.push_back (DPoint3d::From (3,10,0));
    points.push_back (DPoint3d::From (0,10,0));
    points.push_back (DPoint3d::From (0,4,0));
    points.push_back (DPoint3d::From (2,4,0));

    points.push_back (DPoint3d::From (2,6,0));
    points.push_back (DPoint3d::From (3,6,0));
    points.push_back (DPoint3d::From (3,4,0));
    points.push_back (DPoint3d::From (8,4,0));
    points.push_back (DPoint3d::From (8,1,0));
    points.push_back (DPoint3d::From (0,1,0));
//    points.push_back (DPoint3d::From (0,0,0));

    Check::Print (points, "Points for polygon decomp");
    // unused - double tol = Angle::SmallAngle ();
    for (size_t n : bvector<size_t> {3, 4, 7, 10, 11, 12})
        {
        if (n >= points.size ())
            continue;

       bvector<size_t> index0, hullIndex;
       for (size_t i = 0;i < n; i++)
            index0.push_back (i);
        DoubleEndedQueue<size_t> queue (SIZE_MAX);
        PolylineToHull (points, index0, hullIndex, queue);

        Check::Print (hullIndex, "hull");
        }
    size_t n = points.size ();
    DRange1d areaRange;
    for (size_t i0 = 0;  i0 < n; i0++)
        {
        bvector<size_t> indices, hullIndex;
        for (size_t i = 0; i < n; i++)
            indices.push_back ((i0 + i) % n);
        DoubleEndedQueue<size_t> queue (SIZE_MAX);
        PolylineToHull (points, indices, hullIndex, queue);
        auto polygon = ExtractPolygon (points, hullIndex);
        double a = polygon.AreaXY ();
          areaRange.Extend (a);
        printf ("(i0 %d)", (int)i0);
        Check::Print (a, "area");
        Check::Print (hullIndex, "hull");
        }
    // unused - double a = areaRange.High () - areaRange.Low ();
    Check::Near (areaRange.High (), areaRange.Low (), "Polyline hull area indpenendent of start point");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,CollectLinearGeometry)
    {
    CurveVectorPtr rectangleA = CurveVector::CreateRectangle(0,0,   100,100, 0);
    CurveVectorPtr rectangleA1 = CurveVector::CreateRectangle (10,10, 20,20, 0);

    CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add(rectangleA);
    parityRegion->Add (rectangleA1);

    CurveVectorPtr rectangleB = CurveVector::CreateRectangle(110,10,  120,20, 0);

    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    parent->Add (parityRegion);
    parent->Add (rectangleB);


    bvector<bvector<DPoint3d>> point2A, point2B;
    bvector<bvector<bvector<DPoint3d>>> point3A, point3B;
    parent->CollectLinearGeometry (point2A);
    parent->CollectLinearGeometry (point3A);

    Check::Print (point2A, "2 level rectangles");
    Check::Print (point3A, "3 level rectangles");
    auto splitOptions = IFacetOptions::CreateForCurves ();
    splitOptions->SetMaxEdgeLength (20);
    parent->CollectLinearGeometry (point2B, splitOptions.get ());
    parent->CollectLinearGeometry (point3B, splitOptions.get ());
    Check::Print (point2B, "2 level split frectangles");
    Check::Print (point3B, "3 level split rectangles");
    }

double ZSweepVolume (PolyfaceHeaderCR mesh, double planeZ)
    {
    DPoint3dCR origin = DPoint3d::From (0,0,planeZ);
    DVec3d areaXYZ, volumeXYZ, centroidX, centroidY, centroidZ;
    mesh.DirectionalAreaAndVolume (origin, areaXYZ, volumeXYZ, centroidX, centroidY, centroidZ);
    return volumeXYZ.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,DragFaces)
    {
    double dx = 1.0;
    double dy = 2.0;
    DPoint3dDVec3dDVec3d plane (DPoint3d::From (0,0,0), DVec3d::From (dx,0,0), DVec3d::From (0,dy,0));
    int numX = 5;
    int numY = 3;
    auto mesh = UnitGridPolyface (plane, numX,numY, false);
    mesh->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    // grid mesh is row-major x order.
    int indicesPerFacet = 5;
    int verticalStripeXIndex = 3;

    bvector<size_t> verticalStripe;
    for (int j = 0; j < numY; j++)
        verticalStripe.push_back ((size_t)(indicesPerFacet * (verticalStripeXIndex + j * indicesPerFacet)));
    double zSweep = 1.5;
    DVec3d shiftVector = DVec3d::From (0,0,zSweep);
    bvector<size_t> activePointIndex, fringeFacetIndex;
    mesh->CollectAdjacentFacetAndPointIndices (verticalStripe, fringeFacetIndex, activePointIndex);
    Check::Size (2 * numY, fringeFacetIndex.size ());
    Check::Size (2 * (numY + 1), activePointIndex.size ());
    double z0 = -1.0;
    double volume0 = ZSweepVolume (*mesh, z0);
    mesh->TranslateSelectedFacets (verticalStripe, shiftVector,
            PolyfaceHeader::FacetTranslationMode::JustTranslatePoints);
    double xyAreaPerFacet = dx * dy;
    Check::Near (numX * numY * xyAreaPerFacet * fabs (z0), volume0, "Simple initial volume");
    double volume1 = ZSweepVolume (*mesh, z0);
    // volume increases ...
    //    ... area * sweepZ       for each swept facet.
    //    ... half of that for each of the two neighbors.
    Check::Near (volume0 + 2.0 * numY * xyAreaPerFacet * zSweep, volume1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,AddPolygon)
    {
    auto builder = CreateBuilder (true, true);
    bvector<DPoint3d> points
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (0,2,0),
        };

    //builder->AddPolygon (points);
    builder->AddTriangulation (points);


    }

void CheckIndexedPurge (
PolyfaceHeaderPtr &mesh1,
PolyfaceHeaderPtr &mesh2,
size_t num1,    // expected facets in mesh1
size_t num2,    // expected facets in mesh2
size_t num3,    // simple merge vertex count
size_t numPurged,    // expected facets after merge and purge
double dx,
double dy
)
    {
    IFacetOptionsPtr options = IFacetOptions::CreateForSurfaces();
    options->SetMaxPerFace (10);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->AddPolyface (*mesh1);
    builder->AddPolyface (*mesh2);
    auto mergedMesh = builder->GetClientMeshPtr ();
    auto purgedMesh = mergedMesh->CloneWithIndexedDuplicatesRemoved ();

    auto baseTransform = Check::GetTransform ();
    Check::SaveTransformed (*mesh1);    Check::Shift (dx,0,0);
    Check::SaveTransformed (*mesh2);    Check::Shift (dx,0,0);
    Check::SaveTransformed (*mergedMesh);     Check::Shift (dx,0,0);
    Check::SaveTransformed (*purgedMesh);    Check::Shift (dx, 0,0);
    Check::SetTransform (baseTransform);    Check::Shift (0, dy, 0);

    Check::Size(num1, mesh1->GetNumFacet(), "small mesh facet count");
    Check::Size (num2, mesh2->GetNumFacet (), "large mesh facet count");
    Check::Size(num1 + num2, mergedMesh->GetNumFacet(), "combined mesh facet count");
    Check::Size (num3, mergedMesh->Point().size (), "combined mesh vertex count");
    Check::Size(num1 + num2, mergedMesh->GetNumFacet(), "combined mesh facet count");
    Check::Size(numPurged, purgedMesh->GetNumFacet(), "combined mesh facet count");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceBuilder,DuplicateGrid)
    {
    IFacetOptionsPtr options = IFacetOptions::CreateForSurfaces();
    options->SetMaxPerFace (10);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);


    int numX = 2, numY = 3;

    double dx = numX + 3.0;
    double dy = numY + 5.0;

    // Make 2 overlapping meshes
    DPoint3dDVec3dDVec3d frame; // Identity frame !!!
    auto mesh1 = CreateGridMesh(numX, numY, false, false);
    auto mesh2 = CreateGridMesh(numX+1, numY+1, false, false);
    auto num1 = (size_t)(numX * numY);              // facet count in smaller grid.
    auto num2 = (size_t)((numX + 1) * (numY + 1));  // facet count in larger grid, vertex count in smaller grid
    auto num3 = (size_t)((numX + 2) * (numY + 2));  // vertex count in larger grid.

    CheckIndexedPurge (mesh1, mesh2, num1, num2, num3, num2, dx, dy);

    // indexed purge is orientation independent ..
    mesh2->ReverseIndicesAllFaces ();

    CheckIndexedPurge (mesh1, mesh2, num1, num2, num3, num2, dx, dy);
    Check::ClearGeometry ("PolyfaceBuilder.DuplicateGrid");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MaximalPlanarFaces0)
    {
    //    7
    //    |  \
    //    |    \
    //    |      \
    //    |        \
    //    4---------5----6
    //    |       / ||   |
    //    |     /   ||   |
    //    |   /     8    |
    //    | /       ||\  |
    //    |/        ||  \|
    //    1---------2----3
    //    |\       /|
    //    | \     / |
    //    |  \   /  |
    //    |   10    |
    //    |  /  \   |
    //    | /    \  |
    //    |/      \ |
    //    11--------9
    auto mesh0 = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh0->Point ().push_back (DPoint3d::From (0,0,0));
    mesh0->Point ().push_back (DPoint3d::From (2,0,0));
    mesh0->Point ().push_back (DPoint3d::From (4,0,1));  
    mesh0->Point ().push_back (DPoint3d::From (0,1,0));
    mesh0->Point ().push_back (DPoint3d::From (2,1,0));
    mesh0->Point ().push_back (DPoint3d::From (4,1,1));

    mesh0->Point ().push_back (DPoint3d::From (0,2,0));
    mesh0->Point ().push_back (DPoint3d::From (2, 0.5, 0));  // mid edge along the fold

    // Everything to the left of the 9285 line is coplanar ...
    mesh0->PointIndex ().push_back (1);
    mesh0->PointIndex ().push_back (2);
    mesh0->PointIndex ().push_back (8);
    mesh0->PointIndex ().push_back (5);
    mesh0->PointIndex ().push_back (0);

    mesh0->PointIndex ().push_back (4);
    mesh0->PointIndex ().push_back (1);
    mesh0->PointIndex ().push_back (5);
    mesh0->PointIndex ().push_back (0);

    mesh0->PointIndex ().push_back (2);
    mesh0->PointIndex ().push_back (3);
    mesh0->PointIndex ().push_back (8);
    mesh0->PointIndex ().push_back (0);

    mesh0->PointIndex ().push_back (6);
    mesh0->PointIndex ().push_back (5);
    mesh0->PointIndex ().push_back (8);
    mesh0->PointIndex ().push_back (3);
    mesh0->PointIndex ().push_back (0);


    mesh0->PointIndex ().push_back (4);
    mesh0->PointIndex ().push_back (5);
    mesh0->PointIndex ().push_back (7);
    mesh0->PointIndex ().push_back (0);  // another one in the base plane

    if (Check::PrintDeepStructs ())
        PrintPolyfaceXYZ (*mesh0, "Original mesh -- facet counts 34433, interior split edge at 1based vertex 8", 1000);


    if (Check::False (mesh0->HasIndexErrors ()))
        {
        double area0 = mesh0->SumFacetAreas ();
        size_t numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges;
        mesh0->CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges);
        Check::Size (5, numFacet, "Original facet count");
        PolyfaceHeaderPtr mesh1;
        mesh1 = mesh0->CloneWithMaximalPlanarFacets (true, false);
        if (Check::True (mesh1.IsValid ()))
            {
            bvector <int> foo;
            foo.push_back (1);
            if (Check::PrintDeepStructs ())
                PrintPolyfaceXYZ (*mesh1, "Before CollectCounts", 1000);
            mesh1->CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges);
            double area1 = mesh1->SumFacetAreas ();
            Check::Size (2, numFacet, "Facets after planar merge");
            Check::Near (area0, area1, "Facet areas before/after coplanar merge");
            }
        // Add lower triangles, which include a fully-interior point for the larger planar facet
        mesh0->Point ().push_back (DPoint3d::From (2, -2,0));
        mesh0->Point ().push_back (DPoint3d::From (1, -1,0));
        mesh0->Point ().push_back (DPoint3d::From (0, -2,0));

        mesh0->PointIndex ().push_back (2);
        mesh0->PointIndex ().push_back (1);
        mesh0->PointIndex ().push_back (10);
        mesh0->PointIndex ().push_back (0);

        mesh0->PointIndex ().push_back (1);
        mesh0->PointIndex ().push_back (11);
        mesh0->PointIndex ().push_back (10);
        mesh0->PointIndex ().push_back (0);

        mesh0->PointIndex ().push_back (11);
        mesh0->PointIndex ().push_back (9);
        mesh0->PointIndex ().push_back (10);
        mesh0->PointIndex ().push_back (0);

        mesh0->PointIndex ().push_back (9);
        mesh0->PointIndex ().push_back (2);
        mesh0->PointIndex ().push_back (10);
        mesh0->PointIndex ().push_back (0);

        if (Check::False (mesh0->HasIndexErrors ()))
            {
            // UNUSED: double area0 = mesh0->SumFacetAreas ();
            PolyfaceHeaderPtr mesh1 = mesh0->CloneWithMaximalPlanarFacets (true, false);
            if (Check::True (mesh1.IsValid ()))
                {
                size_t numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges;
                mesh1->CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges);
                Check::Size (2, numFacet, "Facets after planar merge");
                Check::Size (10, numVertex, "Vertex after planar merge with interior point stripped");
                }
            PolyfaceHeaderPtr mesh2 = mesh0->CloneWithMaximalPlanarFacets (true, true);
            if (Check::True (mesh1.IsValid ()))
                {
                size_t numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges;
                mesh2->CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges);
                Check::Size (2, numFacet, "Facets after planar merge");
                Check::Size (7, numVertex, "Vertex after planar merge with interior point stripped and edge joined");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,MaximalPlanarFaces1)
    {
    static int numX = 3;
    static int numY = 3;
    bvector<std::pair<size_t,size_t>> omitList
        {
        std::pair<size_t, size_t>(1,1)
        };
    auto mesh0 = CreateGridMesh(numX, numY, false, false, &omitList);

    size_t numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges;
    mesh0->CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisibleEdges, numInvisibleEdges);


    Check::Size (numX * numY - omitList.size (), numFacet, "Facets in mesh with holes");

    PolyfaceHeaderPtr mesh1 = mesh0->CloneWithMaximalPlanarFacets (true, false);
    if (Check::True (mesh1.IsValid ()))
        {
        if (Check::PrintDeepStructs ())
            PrintPolyfaceXYZ (*mesh1, "Planar merge with hole", 1000);

        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,ConvexSetPunch)
    {
    // Sort of a dtm ...
    size_t numX = 15;
    size_t numY = 20;
    auto dtm = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.3, 0.0, -0.25);
    dtm->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    Check::SaveTransformed (*dtm);
    auto base = CurveVector::CreateRectangle (0,0, 2,2, 4.0);
    double b = -6;
    auto solid = ISolidPrimitive::CreateDgnExtrusion (DgnExtrusionDetail (base, DVec3d::From (0,0,b), true));
    auto options = IFacetOptions::Create ();
    options->SetMaxPerFace (4);
    auto solidFacets = solid->Facet (options);
    //Check::SaveTransformed (*solid);
    double a = 8.0;
    double az = -12.0;
    Transform transform = Transform::FromLineAndRotationAngle (DPoint3d::From (a,0,az), DPoint3d::From (0,a,az), Angle::DegreesToRadians (40.0));
    solidFacets->Transform (transform);
    ConvexClipPlaneSet clipper0;
    solidFacets->BuildConvexClipPlaneSet (clipper0);
    ClipPlaneSet clipper1;
    clipper1.push_back (clipper0);

    Check::SaveTransformed (*solidFacets);
   
    Check::Shift (0, (double)numY, 0);
    PolyfaceHeaderPtr insideClip, outsideClip;
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (
            *solidFacets, clipper0, true,
            &insideClip, &outsideClip);
    Check::Shift ((double)numX, 0,0);
    Check::SaveTransformed (*insideClip);
    Check::Shift (0, (double)numY,0);
    Check::SaveTransformed (*outsideClip);

    Check::ClearGeometry ("Polyface.ConvexSetPunch");

    }


//! 
//!<ul>
//!<li>Input:
//!  <ul>
//!  <li> the original loop of a culvert front
//!  <li>the ClipPlaneSet used to punch a dtm
//!  <li>the (possibly multiple) loops of the punch
//!  </ul>
//!<li>Among the multiple loops, choose the one with centroid closest to the centroid of the culvert front
//!<li>Reorder points in both the centroid front and the closest clip loop so that the starting points of each 
//!    of those two loops are on an intersection edge within the planes.  (i.e. they both correspond to the same culvert front point)
//!</ul>
static bool AlignLoops
(
ConvexClipPlaneSet const &planes,   //! [in] ConvexClipPlaneSet whose first numLoopPlanes are a built from sweeps of edges of a a loop.
size_t numLoopPlanes,   //! [in] number of points in the original loop for the clip plaens
DVec3d const &forwardVector,    //! [in] forward vector used in the original loop.  Outputs are oriented so their normal is forward
bvector<DPoint3d> const &loop0, //! [in] a loop, probably the generator for the planes
bvector<bvector<DPoint3d>> const &loop1Candidates,  //! [in] additional loops, probably the loops where the planes cut a mesh
bvector<DPoint3d> &loop0A,  //! [out] points from loop0, but reorderd for both (a) normal orientation and (b) front () point is on two planes.
bvector<DPoint3d> &loop1A,  //! [out] points from the loop in loop1Candidates (with centroid closest to loop0 centroid), but reorderd for both (a) normal orientation and (b) front () point is on two planes.
size_t &candidateIndex      //! [out] the index of the selected candidate.
)
    {
    double tolerance = DoubleOps::SmallCoordinateRelTol (); // yes, treat it as an abstol.
    // Find the loop1 loop with closest centroid to loop0 .
    DPoint3d centroid0, centroid1;
    DVec3d normal0, normal1;
    double   area0, area1;
    candidateIndex = SIZE_MAX;
    double dMin = DBL_MAX;
    PolygonOps::CentroidNormalAndArea (loop0, centroid0, normal0, area0);
    for (size_t i = 0; i < loop1Candidates.size (); i++)
        {
        if (loop1Candidates[i].size () < 4)
            continue;
        PolygonOps::CentroidNormalAndArea (loop1Candidates[i], centroid1, normal1, area1);
        double d = centroid0.Distance (centroid1);
        if (d < dMin)
            {
            candidateIndex = i;
            dMin = d;
            }
        }
    if (candidateIndex == SIZE_MAX)
        return false;

    // Find some pair of consecutive planes for which each loop has a point on the line of intersection of the planes.
    // shuffle loop points to make those points the start and end.
    for (size_t i = 0; i < numLoopPlanes; i++)
        {
        ClipPlane planeX = planes[i];
        ClipPlane planeY = planes[(i+1) % numLoopPlanes];
        auto i0 = ClipPlane::FindPointOnBothPlanes (loop0, planeX, planeY, tolerance);
        if (i0.IsValid ())
            {
            auto i1 = ClipPlane::FindPointOnBothPlanes (loop1Candidates[candidateIndex], planeX, planeY, tolerance);
            if (i1.IsValid ())
                {
                PolylineOps::CopyCyclicPointsFromStartIndex (loop0, i0.Value (), loop0A);
                PolylineOps::CopyCyclicPointsFromStartIndex (loop1Candidates[candidateIndex], i1.Value (), loop1A);
                PolygonOps::ReverseForPreferedNormal (loop0A, forwardVector);
                PolygonOps::ReverseForPreferedNormal (loop1A, forwardVector);
                return true;
                }
            }
        }
    return false;
    }

void DoClip (PolyfaceHeaderR facets, ConvexClipPlaneSetCR planes, DVec3dCR viewVector, bvector<DPoint3d> &triangulationMate)
    {
    ClipPlaneSet clipper;
    clipper.push_back (planes);

    PolyfaceHeaderPtr insideMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceHeaderPtr outsideMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr insideMap = PolyfaceCoordinateMap::Create (*insideMesh);
    PolyfaceCoordinateMapPtr outsideMap = PolyfaceCoordinateMap::Create (*outsideMesh);

    bvector<bvector<DPoint3d>> loops, chains;

    bool badCuts;
    PolyfaceCoordinateMap::AddClippedPolyface (facets, insideMap.get (), outsideMap.get (), badCuts, &clipper, false, nullptr, &loops, &chains);



    DRange3d range = facets.PointRange ();

    Check::SaveTransformed (facets);
    Check::SaveTransformed (triangulationMate);
    double dx = range.XLength ();
    double dy = range.YLength ();
    auto frame = Check::GetTransform ();
    Check::Shift (1.5 * dx, 0,0);
    Check::SaveTransformed (*insideMesh);
    Check::Shift (0, dy,0);
    Check::SaveTransformed (*outsideMesh);

    Check::Shift (0, dy,0);
    for (auto &data : loops)
        Check::SaveTransformed (data);
    for (auto &data: chains)
        Check::SaveTransformed (data);



    bvector<DPoint3d> loopA, loopB;
    size_t indexB;
    if (AlignLoops (
            planes, triangulationMate.size () - 1, 
            viewVector,
            triangulationMate, loops,
            loopA, loopB, indexB)
        )
        {
        Check::Shift (0, dy,0);
        Check::SaveTransformed (loopA);
        Check::SaveTransformed (loopB);
        Check::True (loopA.size () > 0, "Aligned loop is nontrivial");


        Check::Shift (0, dy,0);
        Check::SaveTransformed (triangulationMate);
        Check::SaveTransformed (loopA);
#define UseGreedyTriangles
#ifdef UseGreedyTriangles
        bvector<int> indices;
        bvector<DTriangle3d> triangles;
        PolylineOps::GreedyTriangulationBetweenLinestrings (loopB, loopA, triangles, &indices, Angle::FromDegrees (10.0));
        Check::SaveTransformed (triangles, true);
        Check::Shift (0, dy, 0);
        outsideMap->AddPolygon (loopA);
        for (auto &t : triangles)
            outsideMap->AddPolygon (3, t.point);

#else
        MTGFacets facets (MTG_Facets_VertexOnly);
        if (jmdlMTGFacets_ruledPatternDPoint3dArrayBoundaries (
                            &facets,
                            &loopA[0], (int)loopA.size (),
                            &loopB[0], (int)loopB.size ()
                            ))
            {
            /*size_t numFaceB = */AddMTGFacetsToIndexedPolyface (&facets, *outsideMesh);
            outsideMesh->AddPolygon (&triangulationMate[0], triangulationMate.size ());
            outsideMesh->Compress ();

            }
#endif
        Check::SaveTransformed (*outsideMesh);
        }

    Check::SetTransform (frame);
    Check::Shift (4.0 * dx, 0, 0);
    }


void AddShiftedPoints (bvector<DPoint3d> &xyz0, bvector<DPoint3d> const &xyz1, DVec3dCR shift)
    {
    for(auto &xyz : xyz1)
        xyz0.push_back (xyz + shift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CulvertPunchA)
    {
    double dy = 20.0;
    bvector<DPoint3d> wave {DPoint3d::From (0,0,0), DPoint3d::From (0,0,10), DPoint3d::From (5,0,10), DPoint3d::From (5,0,0)};
    bvector<DPoint3d> sectionPoints;
    AddShiftedPoints (sectionPoints, wave, DVec3d::From (0,0,0));
    AddShiftedPoints (sectionPoints, wave, DVec3d::From (10,0,0));
    AddShiftedPoints (sectionPoints, wave, DVec3d::From (20,0,0));

    auto sectionVector = CurveVector::CreateLinear (sectionPoints, CurveVector::BOUNDARY_TYPE_Open);
    auto surface = ISolidPrimitive::CreateDgnExtrusion (
            DgnExtrusionDetail (sectionVector, DVec3d::From (0,dy,0), false));

    auto options = IFacetOptions::Create ();
    options->SetMaxPerFace (4);
    auto surfaceFacets = surface->Facet (options);

    bvector<DPoint3d> rectangle {
        DPoint3d::From (7,3,2),
        DPoint3d::From (7,8,2),
        DPoint3d::From (8,8,5),
        DPoint3d::From (8,3,5),
        DPoint3d::From (7,3,2)
        };
    DVec3d viewVector = DVec3d::From (-5,0,-1);
    PolygonOps::ReverseForPreferedNormal (rectangle, viewVector);
    auto tilt = Angle::FromDegrees (5.0);
    double forwardClip = 1.0;
    double reverseClip = -5.0;
    ConvexClipPlaneSet planes;
    planes.AddSweptPolyline (rectangle, viewVector, tilt);

    DoClip (*surfaceFacets, planes, viewVector, rectangle);

    planes.Add (ClipPlane::FromPointsAndDistanceAlongPlaneNormal (rectangle, viewVector, forwardClip, true));
    planes.Add (ClipPlane::FromPointsAndDistanceAlongPlaneNormal (rectangle, viewVector, reverseClip, true));
    DoClip (*surfaceFacets, planes, viewVector, rectangle);
    Check::ClearGeometry ("Polyface.CulvertPunchA");
    }



//! Build a mesh that "connects" an above-ground culvert face to the nearby ground.
//! The connector mesh can be perpendicular to the face, or graded at some angle from perpendicular
PolyfaceHeaderPtr AddConvexCulvertToDTM
(
PolyfaceHeaderR facets,         //!< [in] DTM to be punched
bvector<DPoint3d> forwardFaceIn,  //!< [in] exposed face of culvert.  !! This must be convex
double maxDistanceBelow,        //!< [in] max plausible distance from culvert front face to ground contact.
Angle tiltAngle                 //!< [in] angle to tilt away from perpendicular of the forward face.
)
    {
    bool ok = false;
    ConvexClipPlaneSet planes;
    auto forwardFace = forwardFaceIn;
    PolygonOps::ReverseForPreferedNormal (forwardFace, DVec3d::From (0,0,1));
    DVec3d viewVector = PolygonOps::AreaNormal (forwardFace);
    planes.AddSweptPolyline (forwardFace, viewVector, tiltAngle);
    size_t numSidePlanes = planes.size ();

    ClipPlaneSet clipper;
    clipper.push_back (planes);

    PolyfaceHeaderPtr insideMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceHeaderPtr outsideMesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr insideMap = PolyfaceCoordinateMap::Create (*insideMesh);
    PolyfaceCoordinateMapPtr outsideMap = PolyfaceCoordinateMap::Create (*outsideMesh);

    bvector<bvector<DPoint3d>> loops, chains;

    bool badCuts;
    PolyfaceCoordinateMap::AddClippedPolyface (facets, insideMap.get (), outsideMap.get (), badCuts, &clipper, false, nullptr, &loops, &chains);

    bvector<DPoint3d> loopA, loopB;
    auto planeAngle = Angle::FromDegrees (10.0);
    size_t indexB;
    if (AlignLoops (planes, numSidePlanes,
        viewVector,
        forwardFace, loops,
        loopA, loopB, indexB)
        )
        {
        bvector<int> indices;
        bvector<DTriangle3d> triangles;
        PolylineOps::GreedyTriangulationBetweenLinestrings (loopB, loopA, triangles, &indices, planeAngle);
#define CheckCallsInCulvert_not
#ifdef CheckCallsInCulvert
        DRange3d facetRange = facets.PointRange ();
        DRange3d loopRange = DRange3d::From (forwardFace);
            {   // indent for scoped var ..
            SaveAndRestoreCheckTransform transformSaver (0, facetRange.YLength (), 0);
            for (auto &loop : loops)
                {
                Check::SaveTransformed (loop);
                loopRange.Extend (loop);
                }
            Check::SaveTransformed (forwardFace);
            Check::Shift (0, 2.0 * loopRange.YLength (), 0.0);
            Check::SaveTransformed (loopA);
            double dz = 0.1 * loopRange.ZLength ();
            Check::SaveTransformed (loopB);
            Check::Shift (0,0, dz);
            Check::SaveTransformed (triangles, true);
            Check::Shift (0,0, dz);
            Check::SaveTransformed (loopA);
            }
#endif
        outsideMap->AddPolygon (loopA);
        for (auto &t : triangles)
            outsideMap->AddPolygon (3, t.point);
        ok = true;
        }
    return ok ? outsideMesh : nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,CulvertPunchB)
    {
    // Sort of a dtm ...
    size_t numX = 8;
    size_t numY = 14;
    double shiftX = (double) numX;
    double shiftY = (double) numY;
    auto dtm = PolyfaceWithSinusoidalGrid (numX, numY,    0.0, 0.3, 0.0, -0.25);

    for (auto zData : bvector<DPoint2d>
            {
            DPoint2d::From (0.1, 0.2)
            ,DPoint2d::From (-1.5, -0.5)
            }
            )
        {
        double z0 = zData.x;
        double z1 = zData.y;
        // KNOWN PROBLEM: if the rectangle xy are exactly on integers, puncher sees overlapping edges instead of loops.
        bvector<DPoint3d> rectangle {
            DPoint3d::From (2.1,5.2, z0),
            DPoint3d::From (2.1,6.2, z1),
            DPoint3d::From (4.1,6.2, z1),
            DPoint3d::From (4.1,5.2, z0),
            DPoint3d::From (2.1,5.2, z0),
            };

        for (Angle tilt : bvector<Angle>{ Angle::FromDegrees (0.0), Angle::FromDegrees (5.0), Angle::FromDegrees (15.0)})
            {
            auto baseFrame = Check::GetTransform ();
            for (double dz : bvector<double> {0.0, 2.0, 4.0})
                {
                bvector<DPoint3d> rectangle1 = rectangle;
                DPoint3dOps::Add (rectangle1, DVec3d::From (0,0,dz));

                auto dtm1 = AddConvexCulvertToDTM (*dtm, rectangle1, 10.0, tilt);
                Check::SaveTransformed (*dtm);
                Check::SaveTransformed (rectangle1);
                if (dtm1.IsValid ())
                    {
                    Check::Shift (0, shiftY, 0);
                    Check::SaveTransformed (*dtm1);
                    }
                Check::Shift (0, 2.0 * shiftY, 0);
                }
                Check::SetTransform (baseFrame);
                Check::Shift (2.0 * shiftX, 0,0);
            }
        Check::Shift (shiftX, 0, 0);
        }
    Check::ClearGeometry ("Polyface.CulvertPunchB");
    }

void ExercisePunchAndMaximal (PolyfaceHeaderPtr &clipper, PolyfaceHeaderPtr & target)
    {
    PolyfaceHeaderPtr inside, outside;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*clipper, *target, &inside, &outside);
    double a = 20.0;
    Check::SaveTransformed (*target);
    Check::SaveTransformed (*clipper);
    Check::Shift (a,0,0);
    Check::SaveTransformed (*inside);
    Check::SaveTransformed (*outside);

    Check::Shift (a,0,0);
    bvector<bool> trueFalse { true, false};
    for (auto mergeEdges : trueFalse)
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        for (auto mergeFaces : trueFalse)
            {
            SaveAndRestoreCheckTransform shifter (a,0,0);
            auto meshB = outside->CloneWithMaximalPlanarFacets (mergeFaces, mergeEdges);
            Check::SaveTransformed (*meshB);

            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PunchOverlap)
    {
    PolyfaceHeaderPtr inside1 = PolyfaceHeader::CreateVariableSizeIndexed();
    inside1->AddPolygon({ { 15,17.5 },{ 2.5,17.5 },{ 2.5,2.5 },{ 14.3263953224568,2.49999999999999 },{ 19.3263953224568,12.5 },{ 14.9999999999999,12.5 },{ 15,17.5 }});

    PolyfaceHeaderPtr clipper = PolyfaceHeader::CreateVariableSizeIndexed();
    clipper->AddPolygon({ { 2.5,2.5 },{ 15,2.5 },{ 15,10 },{ 2.5,10 },{ 2.5,2.5 } });

    ExercisePunchAndMaximal (clipper, inside1);
    Check::ClearGeometry("Polyface.PunchOverlap");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface,PunchOverlapB)
    {
    PolyfaceHeaderPtr inside1 = PolyfaceHeader::CreateVariableSizeIndexed();
    inside1->AddPolygon({ { 15,17.5 },{ 2.5,17.5 },{ 2.5,2.5 },{ 14.3263953224568,2.49999999999999 },{ 19.3263953224568,12.5 },{ 14.9999999999999,12.5 },{ 15,17.5 }});

    // Punch out holes:
    PolyfaceHeaderPtr clipper = PolyfaceHeader::CreateVariableSizeIndexed();
    clipper->AddPolygon({ { 2.5,2.5 },{ 15,2.5 },{ 15,10 },{ 2.5,10 },{ 2.5,2.5 } });
    clipper->AddPolygon({ { 15,8 },{ 15,5 },{ 18,5 },{ 18,4 },{ 19,4 },{ 19,8 },{ 15,8 } });
    clipper->AddPolygon({ { 15,2.5 },{ 18,2.5 },{ 18,5 },{ 15,5 },{ 15,2.5 } });
    ExercisePunchAndMaximal (clipper, inside1);
    Check::ClearGeometry ("Polyface.PunchOverlapB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz    11/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(Polyface,PunchOverlapC)
    {
    PolyfaceHeaderPtr target = PolyfaceHeader::CreateVariableSizeIndexed();
    static int s_numTargetFace = 1;
    target->AddPolygon({{15.000000000000002, 17.500000000000000},
                        {2.5000000000000000, 17.500000000000000},
                        {2.5000000000000018, 2.5000000000000000},
                        {14.376355634263669, 2.5000000000000036},
                        {19.376355634263668, 12.500000000000000},
                        {14.999999999999998, 12.500000000000000},
                        {15.000000000000002, 17.500000000000000}});
    if (s_numTargetFace > 1)
        target->AddPolygon({{20.000000000000000, 12.500000000000000},
                        {19.376355634263668, 12.500000000000000},
                        {14.376355634263669, 2.5000000000000036},
                        {20.000000000000000, 2.5000000000000000},
                        {20.000000000000000, 12.500000000000000}});

    // Punch out holes:
    PolyfaceHeaderPtr clipper = PolyfaceHeader::CreateVariableSizeIndexed();
    clipper->AddPolygon({{2.5,2.5}, {15,2.5}, {15,10}, {2.5,10}, {2.5,2.5}});
    clipper->AddPolygon({{15,8}, {15,5}, {18,5}, {18,4}, {19,4}, {19,8}, {15,8}});
    clipper->AddPolygon({{15,2.5}, {18,2.5}, {18,5}, {15,5}, {15,2.5}});
#define JustPunchnot
#ifdef JustPunch
    double dX = 20;
    double dY = 20;
    PolyfaceHeaderPtr inside, outside, debugPolygons;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*clipper, *target, &inside, &outside, nullptr, false);
    Check::SaveTransformed (clipper);
    Check::Shift (0, dY, 0);
    Check::SaveTransformed (target);
    Check::Shift (dX, -dY, 0);
    Check::SaveTransformed (inside);
    Check::SaveTransformed (outside);
    Check::Shift (0, dY, 0);
    Check::SaveTransformed (debugPolygons);
#elif defined(PunchAndExpand)
    PolyfaceHeaderPtr inside, outside;
    PolyfaceHeader::ComputePunchXYByPlaneSets(*clipper, *target, &inside, &outside);
    PolyfaceHeaderPtr outsideCloneTrue = outside->CloneWithMaximalPlanarFacets(false, true);
    PolyfaceHeaderPtr outsideCloneFalse = outside->CloneWithMaximalPlanarFacets(false, false);
#else
    ExercisePunchAndMaximal (clipper, target);
#endif
    Check::ClearGeometry("Polyface.PunchOverlapC");

    }


bvector<DPoint3d> SweepPoints (bool clockwise)
    {
    const double Elevation = 0.8;
    const double HalfWidth = 1.0;
    const double Height = 4.0;
    if (clockwise)
        {
        return bvector<DPoint3d>
            {
            DPoint3d::From (0.0, Elevation),
            DPoint3d::From (-HalfWidth, Elevation),
            DPoint3d::From (-HalfWidth, Height),
            DPoint3d::From (HalfWidth, Height),
            DPoint3d::From (HalfWidth, Elevation),
            DPoint3d::From (0.0, Elevation)
            };
        }
    else
        {
        return bvector<DPoint3d>
            {
            DPoint3d::From (0.0, Elevation),
            DPoint3d::From (HalfWidth, Elevation),
            DPoint3d::From (HalfWidth, Height),
            DPoint3d::From (-HalfWidth, Height),
            DPoint3d::From (-HalfWidth, Elevation),
            DPoint3d::From (0.0, Elevation)
            };
        }
    }

RotMatrix Frame1 ()
    {
    auto rotationCCWAroundX = RotMatrix::FromAxisAndRotationAngle(0, Angle::DegreesToRadians (90.0));
    auto rotationCCWAroundZ = RotMatrix::FromAxisAndRotationAngle(2, Angle::DegreesToRadians(90.0));
    return rotationCCWAroundZ *rotationCCWAroundX;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz    11/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(Polyface,PiotrSweep)
    {
    for (auto axes :
        {
        RotMatrix::FromIdentity (),
        Frame1 (),
        })
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        for (bool clockwise : {false, true})
            {
            SaveAndRestoreCheckTransform shifter (0,20,0);
            auto points = SweepPoints (clockwise);
            Check::SaveTransformed (points);
            IFacetOptionsPtr options = CreateFacetOptions ();
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            Transform placement = Transform::From (axes);
            DVec3d direction;
            axes.GetColumn (direction, 2);
            placement.Multiply (points, points);
            builder->AddLinearSweep (points, nullptr, direction, true);
            Check::SaveTransformed (builder->GetClientMeshR ());
            }
        }
    Check::ClearGeometry ("Polyface.PiotrSweep");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz    11/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(Polyface,FilledDisk)
    {
    static int s_clearMode = 2;
    double radiusA = 5.0;
    IFacetOptionsPtr options = CreateFacetOptions ();
    options->SetParamsRequired (true); 
    options->SetNormalsRequired (true); 
    int counter = 0;
    char title[1024];
    for (double maxEdgeLength : {10.0, 5.0, 1.0})
        {
        SaveAndRestoreCheckTransform shifter (3.0 * radiusA, 0, 0);
        options->SetMaxEdgeLength (maxEdgeLength);
        for (double radiusB : {5.0, 4.0, 1.0, 8.0, 12.0})
            {
            counter++;
            SaveAndRestoreCheckTransform shifter (0.0, 8.0 * radiusB, 0);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
            auto ellipse = DEllipse3d::From (
                    radiusA, radiusB, 0,
                    radiusA, 0, 0,
                    0, radiusB, 0,
                    0, Angle::TwoPi ());
            builder->AddFullDiskTriangles (ellipse, 4);
            Check::SaveTransformed (builder->GetClientMeshR ());
            if (s_clearMode == 1)
                {
                sprintf (title, "Polyface.FilledDiskTriangles%d", counter);
                Check::ClearGeometry (title);
                }

            Check::Shift (0, 3.0 * radiusB, 0);
            IPolyfaceConstructionPtr builderB = IPolyfaceConstruction::Create (*options);
            builderB->AddFullDisk (ellipse, 4);
            Check::SaveTransformed (builderB->GetClientMeshR ());
            if (s_clearMode == 1)
                {
                sprintf (title, "Polyface.FilledDisk%d", counter);
                Check::ClearGeometry (title);
                }
            }
        }
    Check::ClearGeometry ("Polyface.FilledDisk");
    }
