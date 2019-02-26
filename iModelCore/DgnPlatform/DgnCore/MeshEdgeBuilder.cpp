/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/MeshEdgeBuilder.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol) { if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false; }
#define COMPARE_VALUES(val0, val1) { if (val0 < val1) return true; if (val0 > val1) return false; }

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     05/2017
//=======================================================================================
struct MeshEdgesBuilder
{
private:

    struct  PointComparator 
        {
        bool operator()(QPoint3dCR lhs, QPoint3dCR rhs) const
            {
            COMPARE_VALUES(lhs.x, rhs.x);
            COMPARE_VALUES(lhs.y, rhs.y);
            return lhs.z < rhs.z; 
            }
        };

    struct  PointMap : bmap<QPoint3d, uint32_t, PointComparator> 
        {
        PointMap(DRange3dCR range) : m_range(range) { }

        uint32_t        m_next = 0;
        DRange3dCR      m_range;

        uint32_t GetIndex(QPoint3d point)
            {
            auto insertPair = Insert(point, m_next);
            if (insertPair.second)
                m_next++;

            return insertPair.first->second;
            }
        };


    struct  EdgeInfo
        {
        bool        m_visible;
        uint32_t    m_faceCount;
        uint32_t    m_faceIndices[2];
        DPoint3d    m_points[2];
        MeshEdge    m_edge;

        EdgeInfo () { }
        EdgeInfo(bool visible, uint32_t faceIndex, MeshEdgeCR edge, DPoint3dCR point0, DPoint3dCR point1) : m_visible(visible), m_faceCount(1), m_edge(edge) { m_faceIndices[0] = faceIndex; m_points[0] = point0; m_points[1] = point1; }

        void AddFace(bool visible, uint32_t faceIndex)
            {
            if (m_faceCount < 2)
                {
                m_visible |= visible;
                m_faceIndices[m_faceCount++] = faceIndex;
                }
            }
        };

    bmap <MeshEdge, EdgeInfo>       m_edgeMap;
    bvector<FVec3d>                 m_triangleNormals;
    MeshEdgeCreationOptionsCR       m_options;

public:    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
* Build maps from a MeshBuilder::Polyface...
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesBuilder (DRange3dCR tileRange, MeshCR mesh, MeshBuilder::Polyface const& builderPolyface) : m_options(builderPolyface.m_edgeOptions)
    {
    TriangleList const&     triangles = mesh.Triangles();
    bool                    anyHidden = false;
    
    // We need to detect the edge pairs -- Can't do that from the Mesh indices as these are not shared - so we'll
    // assume that the polyface indices are properly shared, this should be true as a seperate index array is used
    // for Polyfaces.

    for (size_t triangleIndex = builderPolyface.m_baseTriangleIndex; triangleIndex < triangles.Count(); triangleIndex++)
        {
        auto const&     triangle = triangles.GetTriangle(triangleIndex);
        DPoint3d        polyfacePoints[3];
        uint32_t        polyfaceIndices[3];      // Use (unquantized) polyface points to calculate triangle normals.
        bool            indexNotFound = false;

        for (size_t j=0; j<3; j++)
            {
            auto const&       foundPolyfaceIndex = builderPolyface.m_vertexIndexMap.find(triangle.m_indices[j]);

            if (foundPolyfaceIndex == builderPolyface.m_vertexIndexMap.end())
                {
                BeAssert(false);
                indexNotFound = true;
                continue;
                }

            polyfacePoints[j] = builderPolyface.m_polyface.GetPointCP()[polyfaceIndices[j] = foundPolyfaceIndex->second];
            }
        if (indexNotFound)
            continue;

        for (size_t j=0; j<3; j++)
            {
            size_t          jNext = (j+1)%3;
            uint32_t        triangleNormalIndex = static_cast<uint32_t>(m_triangleNormals.size());
            MeshEdge        meshEdge(triangle.m_indices[j], triangle.m_indices[jNext]), polyfaceEdge(polyfaceIndices[j], polyfaceIndices[jNext]);
            EdgeInfo        edgeInfo(triangle.GetEdgeVisible(j), triangleNormalIndex, meshEdge, polyfacePoints[j], polyfacePoints[jNext]);
            auto            insertPair = m_edgeMap.Insert(polyfaceEdge, edgeInfo);

            anyHidden |= !edgeInfo.m_visible;

            if (!insertPair.second)
                insertPair.first->second.AddFace(edgeInfo.m_visible, triangleNormalIndex);
            }

        m_triangleNormals.push_back(FVec3d::From(DVec3d::FromNormalizedCrossProductToPoints(polyfacePoints[0], polyfacePoints[1], polyfacePoints[2])));
        }

    if (!anyHidden)
        CalculateEdgeVisibility (tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
* Build maps from TriMeshArgs  - Could be used by a reality source...
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesBuilder (TriMeshArgsCR args, DRange3dCR tileRange, MeshEdgeCreationOptionsCR options) : m_options (options)
    {
    PointMap            pointMap(tileRange);
    uint32_t            triangleIndex = 0;
    bool                anyHidden = false;

    for (uint32_t i = 0; i < args.m_numIndices; i+= 3, triangleIndex++)
        {
        uint32_t const* triIndex = args.m_vertIndex + i;

        DPoint3d dpts[3] =
            {
            args.m_points[triIndex[0]].Unquantize(args.m_pointParams),
            args.m_points[triIndex[1]].Unquantize(args.m_pointParams),
            args.m_points[triIndex[2]].Unquantize(args.m_pointParams)
            };

        for (size_t j=0; j<3; j++)
            {
            MeshEdge        meshEdge(triIndex[j], triIndex[(j+1)%3]);
            EdgeInfo        edgeInfo(true, triangleIndex, meshEdge, dpts[j], dpts[(j+1) %3]);

            if (m_options.GenerateAllEdges())
                meshEdge = MeshEdge(pointMap.GetIndex(args.m_points[meshEdge.m_indices[0]]),            // Remap the edge indices to indices based on the PointMap -- An expensive operation - if "AllOptions" is set We'll skip it and just use
                                    pointMap.GetIndex(args.m_points[meshEdge.m_indices[1]]));           // The raw indices - which is fast but will generate duplicate edges.

            auto            insertPair = m_edgeMap.Insert(meshEdge, edgeInfo);

            anyHidden |= !edgeInfo.m_visible;

            if (!insertPair.second)
                insertPair.first->second.AddFace(edgeInfo.m_visible, triangleIndex);
            }

        m_triangleNormals.push_back(FVec3d::From(DVec3d::FromNormalizedCrossProductToPoints(dpts[0], dpts[1], dpts[2])));
        }

    if (!anyHidden)
        CalculateEdgeVisibility (tileRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculateEdgeVisibility(DRange3dCR tileRange)
    {
    if (m_options.GenerateAllEdges())
        return;

    double  minEdgeDot = cos(m_options.m_minCreaseAngle);

    // If there is no visibility indication in the mesh, infer from the mesh geometry.
    for (auto& edge : m_edgeMap)
        {
        if (!tileRange.IsContained(edge.second.m_points[0]) &&
            !tileRange.IsContained(edge.second.m_points[1]))
            {
            // An edge that is outside the tile should never be displayed. (avoid incorrect sheet edge detection on triangles overlapping tile boundary).
            edge.second.m_visible = false;
            }
        else if (2 == edge.second.m_faceCount)
            {
            FVec3d const&  normal0 = m_triangleNormals[edge.second.m_faceIndices[0]];
            FVec3d const&  normal1 = m_triangleNormals[edge.second.m_faceIndices[1]];
                                         
            if (fabs(normal0.DotProduct(normal1)) > minEdgeDot)
                edge.second.m_visible = false;
            }
        else if (m_options.GenerateSheetEdges())
            {
            edge.second.m_visible = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildPolylineFromEdgeChain(MeshEdgesR edges, PolyfaceEdgeChain const& chain, MeshBuilder::Polyface const& builderPolyface, bmap<uint32_t, uint32_t> const& inverseVertexIndexMap) const
    {
    double              startDistance = 0.0;
    bvector<uint32_t>   indices;
    DPoint3dCP          polyfacePoints = builderPolyface.m_polyface.GetPointCP();
    int32_t const*      chainIndices = chain.GetIndexCP();

    for (size_t i=0; i<chain.GetIndexCount(); i++)
        {
        auto const&   builderIndex = inverseVertexIndexMap.find((uint32_t) chainIndices[i]-1);
                 
        if (i > 0 && 0 != chainIndices[i] && 0 != chainIndices[i-1])
            startDistance += polyfacePoints[chainIndices[i]-1].Distance(polyfacePoints[chainIndices[i-1]-1]);

        if (builderIndex == inverseVertexIndexMap.end())
            {
            // This vertex is outside the tile (or perhaps decimated?)
            if (!indices.empty())
                {
                if (indices.size() > 1)
                    edges.m_polylines.push_back(MeshPolyline(startDistance, std::move(indices)));

                indices.clear();
                }
           }
        else
            {
            if (indices.empty())
                startDistance = 0.0;

            indices.push_back(builderIndex->second);
            }
        }

    if (indices.size() > 1)
        edges.m_polylines.push_back(MeshPolyline(startDistance, std::move(indices)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildEdges (MeshEdgesR edges, MeshBuilder::Polyface const* builderPolyface)
    {
    // Do not produce visible edges if polyface already has edge chains.
    bool hasEdgeChains = nullptr != builderPolyface && 0 != builderPolyface->m_polyface.GetEdgeChainCount();
    for (auto& edge : m_edgeMap)
        {
        if (edge.second.m_visible)
            {
            if (!hasEdgeChains && !m_options.CreateEdgeChains())
                edges.m_visible.push_back(edge.second.m_edge);
            }
        else
            {
            if (2 == edge.second.m_faceCount)
                {
                static  double    s_maxPlanarDot = .999999;

                FVec3d const&  normal0 = m_triangleNormals[edge.second.m_faceIndices[0]];
                FVec3d const&  normal1 = m_triangleNormals[edge.second.m_faceIndices[1]];

                if (fabs(normal0.DotProduct(normal1)) < s_maxPlanarDot)
                    {
                    // Potential silhouettes.ru
                    edges.m_silhouette.push_back(edge.second.m_edge);
                    edges.m_silhouetteNormals.push_back(OctEncodedNormalPair(OctEncodedNormal::From(normal0), OctEncodedNormal::From(normal1)));
                    }
                }
            }
        }

    if (nullptr != builderPolyface)
        {
        bmap <uint32_t, uint32_t>   inverseVertexIndexMap;

        for (auto& vertexIndex : builderPolyface->m_vertexIndexMap)
            inverseVertexIndexMap.Insert(vertexIndex.second, vertexIndex.first);

        if (0 != builderPolyface->m_polyface.GetEdgeChainCount())
            {
            for (size_t i=0; i<builderPolyface->m_polyface.GetEdgeChainCount(); i++)
                BuildPolylineFromEdgeChain(edges, *(builderPolyface->m_polyface.GetEdgeChainCP() + i), *builderPolyface, inverseVertexIndexMap);
            }
        else if (m_options.CreateEdgeChains())
            {
            BuildPolylineFromEdgeChain(edges, PolyfaceEdgeChain(CurveTopologyId(), builderPolyface->m_polyface), *builderPolyface, inverseVertexIndexMap);
            }
        }
    }
};  // MeshEdgesBuilder



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::BeginPolyface(PolyfaceQueryCR polyface, MeshEdgeCreationOptionsCR options) 
    { 
    m_currentPolyface = (options.GenerateNoEdges() && 0 == polyface.GetEdgeChainCount()) ? nullptr : new Polyface (polyface, options, m_mesh->Triangles().Count()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::EndPolyface()
    {
    if (!m_currentPolyface.IsValid())
        return;

    if (!m_mesh->m_edges.IsValid())
        m_mesh->m_edges = new MeshEdges();

    MeshEdgesBuilder(m_tileRange, *m_mesh, *m_currentPolyface).BuildEdges(*m_mesh->m_edges, m_currentPolyface.get());
    }

