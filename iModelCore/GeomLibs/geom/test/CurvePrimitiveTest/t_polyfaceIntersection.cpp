/*--------------------------------------------------------------------------------------+

* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
#include <Mtg/MtgApi.h>

#define build_t_poylfaceIntersection_not


#ifdef build_t_poylfaceIntersection

static void Print (CharCP title, UsageSums &data)
    {
    printf ("(%s (sum %.8g) (n %g) (minMax %.8g %.8g)\n",
        title,
        data.Sum (),
        data.Count (),
        data.Min (),
        data.Max ()
        );
    }

static void NewLine (){printf ("\n");}
static void printGraphSummary (MTGGraphP graph, CharCP title, int noisy)
    {
    if (noisy > 0)
        printf ("  %s (N %d) (F %d) (V %d) (C %d)\n",
            title,
            (int)graph->GetActiveNodeCount (),
            (int)jmdlMTGGraph_collectAndNumberFaceLoops (graph, NULL, NULL),
            (int)jmdlMTGGraph_collectAndNumberVertexLoops (graph, NULL, NULL),
            (int)jmdlMTGGraph_collectAndNumberConnectedComponents (graph, NULL, NULL)
            );    
    }

static void PrintNonZero (char const *title, size_t n, bool newline = true)
    {
    if (n != 0)
        {
        printf ("  (%s %d)", title, (int)n);
        if (newline)
            printf ("\n");
        }
    }

static void Print (char const *title, double value, bool newline = true)
    {
    printf ("  (%s %.8g)", title, value);
    if (newline)
        printf ("\n");
    }

static void PrintPoint (char const *name, DPoint3dCR xyz, bool newline = true)
    {
    printf ("  (%s %.4g, %.4g, %.4g)", name, xyz.x, xyz.y, xyz.z);
    if (newline)
        printf ("\n");
    }

static void PrintNode (MTGNodeId node, DPoint3dCR xyz, bool newline = true)
    {
    printf (" (%d   %.4g, %.4g, %.4g)", node, xyz.x, xyz.y, xyz.z);
    if (newline)
        printf ("\n");
    }


static void PrintRange (char const *name, DRange3dCR range)
    {
    if (range.IsNull ())
        printf (" (%s NULLRANGE)\n", name);
    else
        {
        printf (" (%s", name);
        PrintPoint ("low", range.low, false);
        PrintPoint ("high", range.high, false);
        printf (" )\n");
        }
    }

static void PrintPolyfaceSummary (PolyfaceHeaderCR mesh, char const *title)
    {
    if (!Check::PrintDeepStructs ())
        return;
    printf ("\n** Polyface %s\n", title);
    size_t numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisible, numInvisible;
    mesh.CollectCounts (numVertex, numFacet, numQuad, numTriangle, numImplicitTriangle, numVisible, numInvisible);
    PrintNonZero ("#V", numVertex, false);
    PrintNonZero ("#F", numFacet, false);
    PrintNonZero ("#Quad", numQuad, false);
    PrintNonZero ("#Tri", numTriangle, false);
    Print ("Area", mesh.SumFacetAreas ());
    NewLine ();
    size_t numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, num0Vis, num10Vis;
    mesh.CountSharedEdges (numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, false, num0Vis, num10Vis);
    PrintNonZero (" PolygonEdges", numPolygonEdge, false);
    PrintNonZero (" matedPairs", numMatedPair, false);
    PrintNonZero (" num1", num1, false);
    PrintNonZero (" num2", num2, false);
    PrintNonZero (" num3", num3, false);
    PrintNonZero (" num4", num4, false);
    PrintNonZero (" numMore", numMore, false);
    PrintNonZero (" numCollapsed", numCollapsed, false);
    PrintNonZero (" num0Vis", num0Vis, false);
    PrintNonZero (" num10Vis", num10Vis, false);
    printf ("\n");

    DRange3d range = mesh.PointRange ();
    PrintRange ("mesh", range);
    }
static double s_absTol = 1.0e-8;
static double s_relTol = 1.0e-12;

DRange3d RangeOfNodes (MTGFacets *facets, bvector<MTGNodeId> &nodes, size_t &numFace, size_t &numVertex, double &volumeSum, int noisy)
    {
    DRange3d range;
    range.Init ();
    size_t numError = 0;
    DPoint3d xyz;
    numFace = 0;
    numVertex = 0;
    MTGMask faceMask = MTG_CONSTU_MASK;
    MTGMask vertexMask = MTG_CONSTV_MASK;
    MTGGraph * graph = jmdlMTGFacets_getGraph(facets);
    graph->SetMask (faceMask);
    graph->SetMask (vertexMask);
    // clear masks node by node ...
    for (MTGNodeId node : nodes)
        {
        graph->ClearMaskAt (node, faceMask);
        graph->ClearMaskAt (node, vertexMask);
        }

    // confirm component includes entire face and vertex loops ..
    for (MTGNodeId node : nodes)
        {
        size_t faceMaskNodes = graph->CountMaskAroundFace (node, faceMask);
        size_t vertexMaskNodes = graph->CountMaskAroundVertex (node, vertexMask);
        if (faceMaskNodes != 0)
            numError++;
        if (vertexMaskNodes != 0)
            numError++;
        }

    DPoint3d origin = DPoint3d::From (0,0,0);
    volumeSum = 0.0;
    for (MTGNodeId node : nodes)
        {
        if (jmdlMTGFacets_getNodeCoordinates (facets, &xyz, node))
            range.Extend (xyz);
        else
            numError++;
        if (MTG_NULL_MASK == graph->GetMaskAt (node, faceMask))
            {
            numFace++;
            graph->SetMaskAroundFace (node, faceMask);
            if (noisy > 9)
                printf ("FACE ");
            int numSector = 0;
            DPoint3d xyzEdges[3];
            MTGARRAY_FACE_LOOP (sector, graph, node)
                {
                DPoint3d xyz;
                jmdlMTGFacets_getNodeCoordinates (facets, &xyz, sector);
                if (noisy > 9)
                    PrintNode (sector, xyz, false);
                if (numSector < 2)
                    xyzEdges[numSector] = xyz;
                else
                    {

                    xyzEdges[2] = xyz;
                    double volume = - origin.TripleProductToPoints (xyzEdges[0], xyzEdges[1], xyzEdges[2]);
                    volumeSum += volume / 6.0;
                    xyzEdges[1] = xyzEdges[2];
                    }

                numSector++;
                }
            MTGARRAY_END_FACE_LOOP (sector, graph, node)
            if (noisy > 9)
                {
                if (numSector != 3)
                    printf ("  (nv=%d)", numSector);
                printf ("\n");
                }

            }
        if (MTG_NULL_MASK == graph->GetMaskAt (node, vertexMask))
            {
            numVertex++;
            graph->SetMaskAroundVertex (node, vertexMask);
            }
        }
    PrintNonZero ("ConnectedComponent Errors", numError);
    return range;
    }

bool SameMaskEverywhere (MTGGraph &graph, MTGMask maskA, MTGMask maskB)
    {
    MTGARRAY_SET_LOOP (node, &graph)
        {
        bool boolA = graph.GetMaskAt (node, maskA) != 0;
        bool boolB = graph.GetMaskAt (node, maskB) != 0;
        if (boolA != boolB)
            return false;
        }
    MTGARRAY_END_SET_LOOP (node, &graph)
    return true;
    }

// Verify that various connected component searches result in the compatible node sets ..
bool VerifyComponentSearch (MTGGraph &graph)
    {
    bvector<bvector<MTGNodeId>> searchAll, searchScopeNode, searchScopeFace, searchScopeVertex;
    graph.CollectConnectedComponents (searchAll);
    graph.CollectConnectedComponents (searchScopeNode, MTG_ScopeNode);
    graph.CollectConnectedComponents (searchScopeFace, MTG_ScopeFace);
    graph.CollectConnectedComponents (searchScopeVertex, MTG_ScopeVertex);

    size_t numErrors = 0;

    if (   Check::True (searchAll.size () == searchScopeNode.size ())
       &&  Check::True (searchAll.size () == searchScopeFace.size ())
       &&  Check::True (searchAll.size () == searchScopeVertex.size ())
       )
       {
        MTGMask maskAll = graph.GrabMask ();
        MTGMask maskNode = graph.GrabMask ();
        MTGMask maskFace = graph.GrabMask ();
        MTGMask maskVertex = graph.GrabMask ();

        Check::True (maskAll != 0);
        Check::True (maskNode != 0);
        Check::True (maskFace != 0);
        Check::True (maskVertex != 0);
        for (size_t i = 0; i < searchAll.size (); i++)
            {
            graph.ClearMask (maskAll);
            graph.ClearMask (maskNode);
            graph.ClearMask (maskFace);
            graph.ClearMask (maskVertex);


            // Apply masks as indicated by each search result:
            // The un-scoped node list should have nodes uniquely ..
            for (size_t j = 0; j < searchAll[i].size (); j++)
                if (graph.TestAndSetMaskAt (searchAll[i][j], maskAll))
                    numErrors++;
            // same for scoped by node .. .
            for (MTGNodeId node : searchScopeNode[i])
                if (graph.TestAndSetMaskAt (node, maskNode))
                    numErrors++;
            // one node per face ...
            for (MTGNodeId seed : searchScopeFace[i])
                {
                // This should be first visit to this face..
                if (0 != graph.CountMaskAroundFace (seed, maskFace))
                    numErrors++;
                graph.SetMaskAroundFace (seed, maskFace);
                }

            // one node per vertex
            for (MTGNodeId seed : searchScopeVertex[i])
                {
                // This should be first visit to this vertex..
                if (0 != graph.CountMaskAroundVertex (seed, maskVertex))
                    numErrors++;
                graph.SetMaskAroundVertex (seed, maskVertex);
                }

            if (!SameMaskEverywhere (graph, maskAll, maskNode))
                numErrors++;
            if (!SameMaskEverywhere (graph, maskAll, maskFace))
                numErrors++;
            if (!SameMaskEverywhere (graph, maskAll, maskVertex))
                numErrors++;
            }
        graph.DropMask (maskNode);
        graph.DropMask (maskAll);
        graph.DropMask (maskFace);
        graph.DropMask (maskVertex);
       }
    else
        {
        numErrors++;
        }
    return numErrors == 0;
    }

void TestMeshIntersection (PolyfaceHeaderPtr &meshA, PolyfaceHeaderPtr &meshB,
int maxPerFace,
int noisy
)
    {
    if (noisy > 1)
        {
        PrintPolyfaceSummary (*meshA, "MeshA");
        PrintPolyfaceSummary (*meshB, "MeshB");
        }
    bvector<DSegment3dSizeSize> segments;
    PolyfaceQuery::SearchIntersectionSegments (*meshA, *meshB, segments);

    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetMaxPerFace (maxPerFace);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);

    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder, *meshA, segments, true);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder, *meshB, segments, false);
    if (noisy > 1)
        PrintPolyfaceSummary (builder->GetClientMeshR (), "merged");

    double areaA = meshA->SumFacetAreas ();
    double areaB = meshB->SumFacetAreas ();
    double areaC = builder->GetClientMeshR ().SumFacetAreas ();
    size_t numFacetsA = meshA->GetNumFacet ();
    size_t numFacetsB = meshB->GetNumFacet ();
    size_t numFacetsC = builder->GetClientMeshR ().GetNumFacet ();
    Check::Near (areaA + areaB, areaC, "mesh imprint preserves area");
    Check::True (numFacetsC >= numFacetsA + numFacetsB, "imprint cannot decrease facet count");

    MTGFacets *mtgFacets = jmdlMTGFacets_new ();
    PolyfaceToMTG (mtgFacets,
          nullptr, nullptr,
          builder->GetClientMeshR (),
          false, s_absTol, s_relTol, 1
          );
    if (noisy > 1)
        printGraphSummary (jmdlMTGFacets_getGraph (mtgFacets), "MTG From Connectivity", noisy);

    bvector<bvector<MTGNodeId>> componentNodes, componentFaceNodes, componentVertexNodes;
    MTGGraph *graph = jmdlMTGFacets_getGraph (mtgFacets);
    graph->CollectConnectedComponents (componentNodes);
    graph->CollectConnectedComponents (componentFaceNodes, MTG_ScopeFace);
    graph->CollectConnectedComponents (componentVertexNodes, MTG_ScopeVertex);
    Check::True (VerifyComponentSearch (*graph), "MultiComponentGraphserach");
    UsageSums volumeSum;
    UsageSums volumeSum1;
    DPoint3d origin;
    origin.Zero ();
    DVec3d checkSum1;
    DRange3d range1;
    for (size_t componentIndex = 0; componentIndex < componentNodes.size (); componentIndex++)
        {
        size_t numFace, numVertex;
        double volume;
        DRange3d range = RangeOfNodes (mtgFacets, componentNodes[componentIndex], numFace, numVertex, volume, noisy);
        Check::Size (numFace, componentFaceNodes[componentIndex].size ());
        Check::Size (numVertex, componentVertexNodes[componentIndex].size ());
        volumeSum.Accumulate (volume);
        double volume1 = mtgFacets->SumTetrahedralVolumeFromNodesPerFace (origin, componentFaceNodes[componentIndex]);
        checkSum1 = mtgFacets->SumTetrahedralVolumeChecksumFromNodesPerFace (origin, componentFaceNodes[componentIndex]);
        range1 = mtgFacets->RangeFromNodesPerFace (componentFaceNodes[componentIndex]);
        volumeSum1.Accumulate (volume1);
        if (noisy > 1)
            {
            printf ("\n (Component %d) (NumNode %d) (volume %g)", (int)componentIndex, (int)componentNodes[componentIndex].size (), volume);
            PrintNonZero("componentFaces", numFace, false);
            PrintNonZero ("componentVerts", numVertex, true);
            PrintRange ("componentRange", range);
            }
        }
    jmdlMTGFacets_free (mtgFacets);
    if (noisy > 1)
        {
        Print ("VolumeSum", volumeSum);
        Print ("VolumeSum1", volumeSum1);
        }
    Check::Near (0.0, volumeSum.Sum (), "interior and exterior volumes cancel");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,Grid0)
{
PolyfaceHeaderPtr gridA = UnitGridPolyface (
            DPlane3dByVectors::FromOriginAndParallelToXY (DPoint3d::From (0,0,0), 2, 3),
            2,2, false);

PolyfaceHeaderPtr gridB = UnitGridPolyface (
            DPlane3dByVectors::FromOriginAndParallelToXZ (DPoint3d::From (-1,1,-1.0), 2, 2),
            2,2, false);
TestMeshIntersection (gridA, gridB, 10, 1);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,Grid1)
{
PolyfaceHeaderPtr gridA = UnitGridPolyface (
            DPlane3dByVectors::FromOriginAndParallelToXY (DPoint3d::From (0,0,0), 2, 3),
            6,2, false);

PolyfaceHeaderPtr gridB = UnitGridPolyface (
            DPlane3dByVectors::FromOriginAndParallelToXZ (DPoint3d::From (1.5,1,-1.0), -4.5, 2),
            5,2, false);
TestMeshIntersection (gridA, gridB, 10, 1);
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,Slice)
{
PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
bvector<DPoint3d> pointA;
pointA.push_back (DPoint3d::From (0,0,0));
pointA.push_back (DPoint3d::From (1,0,0));
pointA.push_back (DPoint3d::From (1,1,0));
pointA.push_back (DPoint3d::From (0,1,0));
meshA->AddPolygon (pointA);
Check::True (meshA->SweepToSolid (DVec3d::From (0,0,5), false));

bvector<DPoint3d> pointB;
double ax = -1.0;
double bx = 2.0;
double ay = -1.0;
double by = 2.0;
double  zSlice = 2.0;
pointB.push_back (DPoint3d::From (ax,ay,zSlice));
pointB.push_back (DPoint3d::From (bx,ay,zSlice));
pointB.push_back (DPoint3d::From (bx,by,zSlice));
pointB.push_back (DPoint3d::From (ax,by,zSlice));

PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateVariableSizeIndexed ();
meshB->AddPolygon (pointB);

TestMeshIntersection (meshA, meshB, 3, 1);
}

#endif

void SaveGridByVolume (bvector<PolyfaceHeaderPtr> &volumes, double dx, double dy)
    {
    DPoint3d origin = DPoint3d::FromZero ();
    int numPositive = 0;
    int numNegative = 0;
    Check::Shift (0,dy, 0);
    for (auto &v : volumes)
        {
        double signedVolume = v->SumTetrahedralVolumes (origin);
        double ix = 0.0;
        double iy = 0.0;
        if (signedVolume > 0)
            {
            iy = 1.0;
            ix = numPositive++;
            }
        else
            {
            iy = 2.0;
            ix = numNegative++;
            }
        Check::Shift (ix * dx, iy * dy, 0);
        Check::SaveTransformed (v);
        Check::Shift (-ix * dx, -iy * dy, 0);
        }
    }
void SaveSpread(bvector<PolyfaceHeaderPtr> &meshes)
    {
    auto range = DRange3d::NullRange ();
    for (auto &m : meshes)
        range.Extend (m->PointRange ());
    double dx = 1.1 * range.XLength ();

    for (size_t i = 0; i < meshes.size (); i++)
        {
        Check::SaveTransformed (meshes[i]);
        Check::Shift (i * dx, 0, 0);
        Check::SaveTransformed (meshes[i]);
        Check::Shift (-(i * dx), 0, 0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,Boxes)
    {
    PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> pointA;
    double a = 4.0;
    double b = 6.0;
    pointA.push_back (DPoint3d::From (0,0,0));
    pointA.push_back (DPoint3d::From (a,0,0));
    pointA.push_back (DPoint3d::From (a,a,0));
    pointA.push_back (DPoint3d::From (0,a,0));
    meshA->AddPolygon (pointA);
    static bool s_triangulate = false;
    // unused - static int s_maxPerFace = 3;
    Check::True (meshA->SweepToSolid (DVec3d::From (0,0,b), s_triangulate));

    PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateVariableSizeIndexed ();
    meshB->CopyFrom(*meshA);
    meshB->Transform (Transform::From (DVec3d::From (1,2,3)));

    //TestMeshIntersection (meshA, meshB, s_maxPerFace, 1);

    bvector<PolyfaceHeaderPtr> enclosedVolumes;
    PolyfaceQuery::MergeAndCollectVolumes (*meshA, *meshB, enclosedVolumes);
    Check::SaveTransformed (meshA);
    Check::SaveTransformed (meshB);
    SaveGridByVolume (enclosedVolumes, 3.0 * a, 3.0 * a);
    Check::ClearGeometry ("PolyfaceIntersection.Boxes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,MultipleBoxes)
    {
    PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> pointA;
    double a = 4.0;
    double b = 6.0;
    pointA.push_back (DPoint3d::From (0,0,0));
    pointA.push_back (DPoint3d::From (a,0,0));
    pointA.push_back (DPoint3d::From (a,a,0));
    pointA.push_back (DPoint3d::From (0,a,0));
    meshA->AddPolygon (pointA);
    static bool s_triangulate = false;
    // unused - static int s_maxPerFace = 3;
    Check::True (meshA->SweepToSolid (DVec3d::From (0,0,b), s_triangulate));

    PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateVariableSizeIndexed ();
    meshB->CopyFrom(*meshA);
    meshB->Transform (Transform::From (DVec3d::From (1,2,3)));

    PolyfaceHeaderPtr meshC = PolyfaceHeader::CreateVariableSizeIndexed ();
    meshC->CopyFrom(*meshA);
    meshC->Transform (Transform::From (DVec3d::From (1.5,2.5,3.5)));

    bvector<PolyfaceHeaderPtr> meshList;
    meshList.push_back (meshA);
    meshList.push_back (meshB);
    meshList.push_back (meshC);
    Check::SaveTransformed (meshA);
    Check::SaveTransformed (meshB);
    Check::SaveTransformed (meshC);
    bvector<PolyfaceHeaderPtr> enclosedVolumes;
    PolyfaceQuery::MergeAndCollectVolumes (meshList, enclosedVolumes);
    Check::Size (7, enclosedVolumes.size (), "3 box interesection produces 7 volumes");
    SaveGridByVolume (enclosedVolumes, 3.0 * a, 3.0 * a);
    Check::ClearGeometry ("PolyfaceIntersection.MultipleBoxes");
    }

// Carrier for a mesh, with xy, area, volume used for sorting and cut/fill splits.
struct CutFillPatch
{
private:
DVec3d m_centroid;
double m_area;
double m_volume;
PolyfaceHeaderPtr m_mesh;
size_t m_parent;
size_t m_cluster;
friend struct CutFillPatchArray;
public:
CutFillPatch (PolyfaceHeaderPtr &mesh, DVec3dCR centroid, double area, double volume, size_t parent)
  : m_centroid (centroid), m_area (area), m_mesh (mesh), m_parent(parent), m_volume(volume), m_cluster(SIZE_MAX)
  {

  }
void SetCluster (size_t clusterIndex = SIZE_MAX){m_cluster = clusterIndex;}
size_t GetCluster () const {return m_cluster;}
size_t IsClustered () {return m_cluster != SIZE_MAX;}

bool IsCentroidCloseXY (CutFillPatch const &other, double tolerance) const
    {
    return m_centroid.DistanceXY (other.m_centroid) <= tolerance;
    }
};

struct CutFillPatchArray : bvector<CutFillPatch>
  {
  bvector<bvector<size_t>> m_clusters;
  DPoint3d m_origin;

  CutFillPatchArray (DPoint3dCR origin) :m_origin(origin){}

  CutFillPatchArray ()
      {
      m_origin.Zero ();
      }
     
  void RegisterMeshes (bvector<PolyfaceHeaderPtr> &meshes, size_t parentIndex)
      {

      for (PolyfaceHeaderPtr &mesh : meshes)
          {
          DVec3d centroidX, centroidY, centroidZ, areaXYZ, volumeXYZ;
          mesh->DirectionalAreaAndVolume (m_origin, areaXYZ, volumeXYZ, centroidX, centroidY, centroidZ);
          push_back (CutFillPatch (mesh, centroidZ, areaXYZ.z, volumeXYZ.z, parentIndex));
          }
      }

  void AddToFinalCluster (size_t index)
      {
      if (index < size () && m_clusters.size () > 0)
          {
          m_clusters.back ().push_back (index);
          at (index).SetCluster (m_clusters.size () - 1);
          }
      }

  void CollectClusters ()
      {
      m_clusters.clear ();
      // Assign each patch as a singleton cluster
      double xyTolerance = Angle::SmallAngle ();
      for (size_t i = 0; i < size (); i++)
        at(i).SetCluster ();
      for (size_t i = 0; i < size (); i++)
        {
        if (!at(i).IsClustered ())
            {
            m_clusters.push_back (bvector<size_t> ());
            AddToFinalCluster (i);
            for (size_t j = i + 1; j < size (); j++)
                {
                if (  !at(j).IsClustered ()
                   && at(i).IsCentroidCloseXY (at(j), xyTolerance)
                   )
                    AddToFinalCluster (j);
                }
            }
        }
      }

  void Print (int noisy)
      {
      if (noisy > 0)
          {
          GEOMAPI_PRINTF ("\nStacked Meshes (total %d) (clusters %d)\n", (int)size (), (int)m_clusters.size ());
          if (noisy > 1)
              {
              for (size_t i = 0; i < size (); i++)
                  {
                  CutFillPatch &patch = at(i);
                  GEOMAPI_PRINTF ("Patch  %3d: (p %d) (xy %.17g,%.17g) (area %.17g) (volume (%.17g) (cluster %d)\n",
                            (int)i,
                            (int)patch.m_parent,
                            patch.m_centroid.x,
                            patch.m_centroid.y,
                            patch.m_area,
                            patch.m_volume,
                            (int)patch.m_cluster
                            );
                  }
              for (size_t i = 0; i < m_clusters.size (); i++)
                  {
                  GEOMAPI_PRINTF("Cluster %d:", (int)i);
                  for (size_t clusterIndex : m_clusters[i])
                    GEOMAPI_PRINTF("  %d", (int)clusterIndex);
                  GEOMAPI_PRINTF ("\n");
                  }
              }
          }
      }
  };

//extern void PrintPolyfaceXYZ (PolyfaceHeaderR mesh, char * title, size_t maxPrintSize);
void Print (bvector<PolyfaceHeaderPtr> &meshes, CharCP title, size_t maxPrintSize = 200)
    {
    if (!Check::PrintDeepStructs ())
        return;
    printf ("\n(bvector<%s> [%d]\n",title, (int)meshes.size ());
    DPoint3d origin = DPoint3d::FromZero ();
    for (size_t i = 0; i < meshes.size (); i++)
      {
      DVec3d centroidX, centroidY, centroidZ, areaXYZ, volumeXYZ;
      meshes[i]->DirectionalAreaAndVolume (origin, areaXYZ, volumeXYZ, centroidX, centroidY, centroidZ);
      printf ("\n mesh %d: (xy centroid %.17g,%.17g,%.17g) (xy area %.17g volume %.17g)\n",
                          (int)i,
                          centroidZ.x, centroidZ.y,centroidZ.z,
                          areaXYZ.z, volumeXYZ.z);
      //PrintPolyfaceXYZ (*meshes[i], "", maxPrintSize);
      }
    printf(")\n");
    }


void AppendImprintSegments (bvector<DSegment3dSizeSize> const &source, bvector<DSegment3dSizeSize> &dest, bool swapIndices)
    {
    if (!swapIndices)
        {
        for (DSegment3dSizeSize const &data : source)
            dest.push_back (data);
        }
    else
        {
        for (DSegment3dSizeSize data : source)  // data is a copy of the member!!!
            {
            data.SwapTags ();
            dest.push_back (data);
            }
        }

    }
static int s_imprintNoisy = 0;

/*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                     Earlin.Lutz  10/17
    +---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceIntersection,ImprintForCutFill)
    {
    PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> pointA;
    double a = 4.0;
    double dy = 2.0 * a;
    pointA.push_back (DPoint3d::From (0,0,0));
    pointA.push_back (DPoint3d::From (a,0,0));
    pointA.push_back (DPoint3d::From (a,a,0));
    pointA.push_back (DPoint3d::From (0,a,0));
    meshA->AddPolygon (pointA);
    meshA->Triangulate ();
    meshA->Compress ();
    meshA->MarkInvisibleEdges (0.01);
#define MeshBByTranslate
#ifdef MeshBByTranslate
    double c = 1.0;
    meshB->AddPolygon (pointA);
    double fx = 0.25;
    double fy = 0.5;
    Transform transform = Transform::From (fx * a, fy * a, c);
    meshB->Transform (transform, false);
#else
    bvector<DPoint3d> pointB;
    double b = 0.5 * a;

    pointB.push_back (DPoint3d::From (   -a, -b,0));
    pointB.push_back (DPoint3d::From (2.0*a, -b,0));
    pointB.push_back (DPoint3d::From (2.0*a,  b,0));
    pointB.push_back (DPoint3d::From (   -a,  b,0));

    meshB->AddPolygon (pointB);

#endif
    size_t numOpen, numClosed;
    Check::SaveTransformed (meshA);
    Check::SaveTransformed (meshB);
    auto boundaryA = meshA->ExtractBoundaryStrings (numOpen, numClosed);
    auto boundaryB = meshB->ExtractBoundaryStrings (numOpen, numClosed);
    Check::Shift (0, dy, 0);
    Check::SaveTransformed (*boundaryA);
    Check::SaveTransformed (*boundaryB);
    Check::Shift (0, dy, 0);
    if (s_imprintNoisy)
        {
        //PrintPolyfaceXYZ (*meshA, "meshA", 200);
        Check::Print (boundaryA);
        //PrintPolyfaceXYZ (*meshB, "meshB", 200);
        Check::Print (boundaryB);
        }
    PolyfaceSearchContext searchA (meshA, true, true, true);
    PolyfaceSearchContext searchB (meshB, true, true, true);

    bvector<DSegment3dSizeSize> imprintBonA, imprintAonB;
    searchA.DoDrapeXY (*boundaryB, imprintBonA);
    searchB.DoDrapeXY (*boundaryA, imprintAonB);

    if (s_imprintNoisy)
        {
        Check::Print (imprintBonA, "B on A");
        Check::Print (imprintAonB, "A on B");
        }

    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builderA = IPolyfaceConstruction::Create (*options);
    IPolyfaceConstructionPtr builderB = IPolyfaceConstruction::Create (*options);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builderA, *meshA, imprintBonA, false);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builderB, *meshB, imprintAonB, false);

    Check::SaveTransformed (builderA->GetClientMeshR ());
    Check::Shift (0, dy, 0);
    Check::SaveTransformed (builderB->GetClientMeshR ());
    Check::Shift (0, dy, 0);



    bvector<PolyfaceHeaderPtr> componentA, componentB;
    builderA->GetClientMeshR ().PartitionByConnectivity (2, componentA);
    builderB->GetClientMeshR ().PartitionByConnectivity (2, componentB);
    SaveSpread (componentA);
    Check::Shift (0, dy, 0);
    SaveSpread (componentB);
    Check::Shift (0, dy, 0);
    DPoint3d origin = DPoint3d::From (0,0,-3);
    CutFillPatchArray patches (origin);
    patches.RegisterMeshes (componentA, 0);
    patches.RegisterMeshes (componentB, 1);
    if (s_imprintNoisy > 10)
        {
        Print (componentA, "componentA");
        Print (componentB, "componentB");
        }
    patches.CollectClusters ();
    patches.Print (10);
    Check::ClearGeometry ("PolyfaceIntersection.ImprintForCutFill");
    }
#ifdef buildImprintFolded
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST(PolyfaceIntersection, ImprintFolded)
    {
    PolyfaceHeaderPtr meshA = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> pointA;
    double a = 4.0;
    pointA.push_back (DPoint3d::From (0,0,0));
    pointA.push_back (DPoint3d::From (a,0,0));
    pointA.push_back (DPoint3d::From (a,a,0));
    pointA.push_back (DPoint3d::From (0,a,0));
    meshA->AddPolygon (pointA);
    meshA->Triangulate ();
    meshA->Compress ();
    meshA->MarkTopologicalBoundariesVisible (false);

    IFacetOptionsPtr options = IFacetOptions::Create ();
    IPolyfaceConstructionPtr builderB0 = IPolyfaceConstruction::Create (*options);
    bvector<DPoint3d> pointB0;
    double x0 = 0.35 * a;
    double x1 = 0.9  * a;
    double x2 = 0.5  * a;
    double z0 = 0.5;
    double z1 = 1.0;
    double z2 = 2.0;
    double y0 = -0.3;
 
    pointB0.push_back (DPoint3d::From (x0, y0, z0));
    pointB0.push_back (DPoint3d::From (x1, y0, z1));
    pointB0.push_back (DPoint3d::From (x2, y0, z2));

    DVec3d sweepVector = DVec3d::From (0, 3.0, 0.0);
    builderB0->AddLinearSweep (pointB0, nullptr, sweepVector, false);
    PolyfaceHeaderPtr meshB = builderB0->GetClientMeshPtr ();
    meshB->MarkTopologicalBoundariesVisible (false);

    Check::SaveTransformed (meshA);
    Check::SaveTransformed (meshB);

    Check::Shift (2.0 * a, 0, 0);

    size_t numOpen, numClosed;
    auto boundaryA = meshA->ExtractBoundaryStrings (numOpen, numClosed);
    auto boundaryB = meshB->ExtractBoundaryStrings (numOpen, numClosed);

    PolyfaceSearchContext searchA (meshA, true, true, true);
    PolyfaceSearchContext searchB (meshB, true, true, true);

    bvector<DSegment3dSizeSize> imprintBonA, imprintAonB;
    searchA.DoDrapeXY (*boundaryB, imprintBonA);
    searchB.DoDrapeXY (*boundaryA, imprintAonB);

    IPolyfaceConstructionPtr builderA = IPolyfaceConstruction::Create (*options);
    IPolyfaceConstructionPtr builderB = IPolyfaceConstruction::Create (*options);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builderA, *meshA, imprintBonA, false);
    PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builderB, *meshB, imprintAonB, false);


    bvector<PolyfaceHeaderPtr> componentA, componentB;
    builderA->GetClientMeshR ().PartitionByConnectivity (2, componentA);
    builderB->GetClientMeshR ().PartitionByConnectivity (2, componentB);

    DPoint3d origin = DPoint3d::From (0,0,-3);
    CutFillPatchArray patches (origin);
    patches.RegisterMeshes (componentA, 0);
    patches.RegisterMeshes (componentB, 1);

    patches.CollectClusters ();
    patches.Print (10);
    Check::ClearGeometry ("PolyfaceIntersection.ImprintFolded");
    }
#endif

