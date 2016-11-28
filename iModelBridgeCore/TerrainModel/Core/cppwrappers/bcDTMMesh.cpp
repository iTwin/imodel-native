/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMMesh.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDtmImpl.h"

USING_NAMESPACE_BENTLEY_TERRAINMODEL

int BcDTMMeshFace::GetMeshPointIndex (int index)
    {
    return m_points[index];
    }

BcDTMMeshFace::BcDTMMeshFace (BcDTMMeshP mesh, const long* meshFaces, long numEdges)
    { 
    m_points = meshFaces;
    m_mesh = mesh;
    }

DPoint3d BcDTMMeshFace::GetCoordinates (int index)
    {
    return m_mesh->GetPoint (m_points[index] - 1);
    }

BcDTMMeshFace::~BcDTMMeshFace()
    {
    }

BcDTMMesh::BcDTMMesh (bvector<DPoint3d>& meshPts, bvector<long>& meshFaces)
    {
    m_points = meshPts;
    m_meshFaceIndices = meshFaces;
    }

BcDTMMesh::~BcDTMMesh()
    {
    }

DPoint3d BcDTMMesh::GetPoint (int index)
    {
    return m_points [index];
    }

int BcDTMMesh::GetPointCount ()
    {
    return (int)m_points.size();
    }

int BcDTMMesh::GetFaceCount ()
    {
    return (int)m_meshFaceIndices.size();
    }

BcDTMMeshFacePtr BcDTMMesh::GetFace (int index)
    {
    return BcDTMMeshFace::Create (this, &m_meshFaceIndices [index * 3], 3);
    }
