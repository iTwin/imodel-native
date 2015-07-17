/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMMesh.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

BcDTMMesh::BcDTMMesh (DPoint3dCP meshPts, long numMeshPts, const long* meshFaces, long numMeshFaces)
    {
    m_points = meshPts;
    m_pointCount = numMeshPts;
    m_meshFaceIndices = meshFaces;
    m_meshFaceIndicesCount = numMeshFaces;
    }

BcDTMMesh::~BcDTMMesh()
    {
    if (m_points != nullptr)
        free ((void*)m_points);

    if (m_meshFaceIndices != nullptr)
        free ((void*)m_meshFaceIndices);
    }

DPoint3d BcDTMMesh::GetPoint (int index)
    {
    return m_points [index];
    }

int BcDTMMesh::GetPointCount ()
    {
    return m_pointCount;
    }

int BcDTMMesh::GetFaceCount ()
    {
    return m_meshFaceIndicesCount;
    }

BcDTMMeshFacePtr BcDTMMesh::GetFace (int index)
    {
    return BcDTMMeshFace::Create (this, &m_meshFaceIndices [index * 3], 3);
    }
