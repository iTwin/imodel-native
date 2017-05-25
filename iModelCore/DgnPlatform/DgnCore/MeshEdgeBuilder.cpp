/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/MeshEdgeBuilder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

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
        MeshEdge    m_edge;

        EdgeInfo () { }
        EdgeInfo(bool visible, uint32_t faceIndex, MeshEdgeCR edge) : m_visible(visible), m_faceCount(1), m_edge(edge) { m_faceIndices[0] = faceIndex; }

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
    bvector<FPoint3d>               m_triangleNormals;
    MeshEdgeCreationOptionsCR       m_options;

public:    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesBuilder (MeshCR mesh, DRange3dCR tileRange, MeshEdgeCreationOptionsCR options) : m_options(options)
    {
    PointMap                    pointMap(tileRange);
    TriangleList const&         triangles = mesh.Triangles();
    QPoint3dListCR              points = mesh.Points();
    bool                        anyHidden = false;

    for (uint32_t triangleIndex = 0; triangleIndex < triangles.size(); triangleIndex++)
        {
        auto const&   triangle = triangles[triangleIndex];
        for (size_t j=0; j<3; j++)
            {
            MeshEdge        meshEdge(triangle.m_indices[j], triangle.m_indices[(j+1)%3]);
            EdgeInfo        edgeInfo(triangle.GetEdgeVisible(j), triangleIndex, meshEdge);
            MeshEdge        pointMapEdge(pointMap.GetIndex(points[meshEdge.m_indices[0]]), 
                                         pointMap.GetIndex(points[meshEdge.m_indices[1]]));
            auto            insertPair = m_edgeMap.Insert(pointMapEdge, edgeInfo);

            if (!insertPair.second)
                insertPair.first->second.AddFace(edgeInfo.m_visible, triangleIndex);

            anyHidden |= !edgeInfo.m_visible;
            }

        m_triangleNormals.push_back (FPoint3d::From(mesh.GetTriangleNormal(triangle)));
        }

    if (!anyHidden)
        CalculateEdgeVisibility (&mesh.Points().front(), tileRange, mesh.Points().GetParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesBuilder (TriMeshArgsCR args, DRange3dCR tileRange, MeshEdgeCreationOptionsCR options) : m_options (options)
    {
    PointMap                    pointMap(tileRange);
    uint32_t                    triangleIndex = 0;

    for (int32_t i = 0; i < args.m_numIndices; i+= 3, triangleIndex++)
        {
        int32_t const* triIndex = args.m_vertIndex + i;

        for (size_t j=0; j<3; j++)
            {
            MeshEdge        meshEdge(triIndex[j], triIndex[(j+1)%3]);
            EdgeInfo        edgeInfo(true, triangleIndex, meshEdge);

            if (!m_options.m_generateAllEdges)
                meshEdge = MeshEdge(pointMap.GetIndex(args.m_points[meshEdge.m_indices[0]]),            // Remap the edge indices to indices based on the PointMap -- An expensive operation - if "AllOptions" is set We'll skip it and just use
                                    pointMap.GetIndex(args.m_points[meshEdge.m_indices[1]]));           // The raw indices - which is fast but will generate duplicate edges.

            auto            insertPair = m_edgeMap.Insert(meshEdge, edgeInfo);

            if (!insertPair.second)
                insertPair.first->second.AddFace(edgeInfo.m_visible, triangleIndex);
            }

        DPoint3d dpts[3] =
            {
            args.m_points[triIndex[0]].Unquantize(args.m_pointParams),
            args.m_points[triIndex[1]].Unquantize(args.m_pointParams),
            args.m_points[triIndex[2]].Unquantize(args.m_pointParams)
            };

        m_triangleNormals.push_back(FPoint3d::From(DVec3d::FromNormalizedCrossProductToPoints(dpts[0], dpts[1], dpts[2])));
        }
    CalculateEdgeVisibility (args.m_points, tileRange, args.m_pointParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculateEdgeVisibility(QPoint3dCP points, DRange3dCR tileRange, QPoint3d::ParamsCR qparams)
    {
    if (m_options.m_generateAllEdges)
        return;

    double  minEdgeDot = cos(m_options.m_minCreaseAngle);

    // If there is no visibility indication in the mesh, infer from the mesh geometry.
    for (auto& edge : m_edgeMap)
        {
        DPoint3d const&  point0  = points[edge.second.m_edge.m_indices[0]].Unquantize(qparams);
        DPoint3d const&  point1  = points[edge.second.m_edge.m_indices[1]].Unquantize(qparams);
        static bool s_hideSheetEdges = true; // Possibly an option (for reality mesh tile edges??).

        if (!tileRange.IsContained(point0) &&
            !tileRange.IsContained(point1))
            {
            // An edge that is outside the tile should never be displayed. (avoid incorrect sheet edge detection on triangles overlapping tile boundary).
            edge.second.m_visible = false;
            }
        else if (2 == edge.second.m_faceCount)
            {
            FPoint3d const&  normal0 = m_triangleNormals[edge.second.m_faceIndices[0]];
            FPoint3d const&  normal1 = m_triangleNormals[edge.second.m_faceIndices[1]];

            if (fabs(DPoint3d::From(normal0).DotProduct(DPoint3d::From(normal1))) > minEdgeDot)
                edge.second.m_visible = false;
            }
        else if (m_options.m_generateSheetEdges)
            {
            edge.second.m_visible = false;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildEdges (MeshEdgesR edges) const
    {
    for (auto& edge : m_edgeMap)
        {
        if (edge.second.m_visible)    
            {
            edges.m_visible.push_back(edge.second.m_edge);
            }
        else
            {
            if (2 == edge.second.m_faceCount)
                {
                FPoint3d const&  normal0 = m_triangleNormals[edge.second.m_faceIndices[0]];
                FPoint3d const&  normal1 = m_triangleNormals[edge.second.m_faceIndices[1]];

                if (!DPoint3d::From(normal0).IsParallelTo(DPoint3d::From(normal1)))
                    {
                    // Potential silhouettes.
                    edges.m_silhouette.push_back(edge.second.m_edge);
                    edges.m_silhouetteNormals0.Add(DPoint3d::From(normal0));
                    edges.m_silhouetteNormals1.Add(DPoint3d::From(normal1));
                    }
                }
            }
        }
    }
};  // MeshEdgesBuilder


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdges::MeshEdges(TriMeshArgsCR mesh, DRange3dCR tileRange, MeshEdgeCreationOptionsCR options)     
    {
    MeshEdgesBuilder (mesh, tileRange, options).BuildEdges(*this);
    }                        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MeshEdgesPtr    Mesh::GetEdges(DRange3dCR tileRange, MeshEdgeCreationOptionsCR options) const
    {
    if (!m_edges.IsValid())
        {
        m_edges = new MeshEdges();
        MeshEdgesBuilder (*this, tileRange, options).BuildEdges(*m_edges);
        }

    return m_edges;
    }

