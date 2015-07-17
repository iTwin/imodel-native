/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMMesh.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include < vcclr.h >
#include ".\dtmmesh.h"
#using <mscorlib.dll>

using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
BGEO::DPoint3d MeshFace::GetCoordinates(int index)
    {
    return m_mesh->GetPoint(m_indices[index]-1);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int MeshFace::GetMeshPointIndex(int index)
    {
    return m_indices[index];
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
MeshFace::MeshFace(Mesh ^mesh, array<int>^ indices)
    {
    m_mesh = mesh;
    m_indices = indices;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
MeshFace::~MeshFace()
    {
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Mesh::Mesh(BcDTMMeshP mesh)
    {
    m_mesh = mesh;
    m_mesh->AddRef();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
MeshFace^ Mesh::GetFace(int index)
    {
    if (m_mesh == NULL)
        {
        throw gcnew System::Exception("Mesh was disposed");
        }
    BcDTMMeshFacePtr unmanagedMeshFace = m_mesh->GetFace(index);
    array<int>^ indices = gcnew array<int>(3);
    indices[0] = unmanagedMeshFace->GetMeshPointIndex(0);
    indices[1] = unmanagedMeshFace->GetMeshPointIndex(1);
    indices[2] = unmanagedMeshFace->GetMeshPointIndex(2);
    MeshFace^ meshFace = gcnew MeshFace(this, indices);
    return meshFace;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int Mesh::FaceCount::get()
    {
    if (m_mesh == nullptr)
        {
        throw gcnew System::Exception("Mesh was disposed");
        }
    return m_mesh->GetFaceCount();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
BGEO::DPoint3d Mesh::GetPoint(int index)
    {
    if (m_mesh == nullptr)
        {
        throw gcnew System::Exception("Mesh was disposed");
        }
    DPoint3d pt = m_mesh->GetPoint(index);
    return BGEO::DPoint3d(pt.x, pt.y, pt.z);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int Mesh::PointCount::get()
    {
    if (m_mesh == nullptr)
        {
        throw gcnew System::Exception("Mesh was disposed");
        }
    return m_mesh->GetPointCount();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Mesh::~Mesh()
    {
    this->!Mesh();
    System::GC::SuppressFinalize(this);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
Mesh::!Mesh()
    {
    if (m_mesh != NULL)
        {
        m_mesh->Release();
        m_mesh = NULL;
        }
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int Mesh::EdgeCount::get()
    {
    BuildEdges();
    return m_edgeCount;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
array<BGEO::DPoint3d>^ Mesh::GetEdge(int index) 
    {
    BuildEdges();
    array<BGEO::DPoint3d>^ res = gcnew array<BGEO::DPoint3d>(2);
    res[0] = GetPoint(m_edges[index]->Vertex1-1);
    res[1] = GetPoint(m_edges[index]->Vertex2-1);
    return res;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
void Mesh::BuildEdges()
    {
    if (m_edges != nullptr)
        return;

    // Create an array which size is the number of points
    array<System::Collections::ArrayList^>^ arrayOfList = gcnew array<System::Collections::ArrayList^>(PointCount);

    // Initialize edge count
    m_edgeCount = 0;

    // Iterate on each face of the mesh, and for each edge of the face put the edge in the entry of the point
    // that has the lowest index.
    for (int iFace = 0; iFace < FaceCount; iFace++)
        {   
        // Get the first point.
        int vertex1 = GetFace(iFace)->GetMeshPointIndex(0);

        // Iterate on the point
        for (int iVertex = 0; iVertex < 3; iVertex++)
            {
            // Get the second vertex of the edge.
            int vertex2;
            if (iVertex < 2)
                vertex2 = GetFace(iFace)->GetMeshPointIndex(iVertex+1);
            else
                vertex2 = GetFace(iFace)->GetMeshPointIndex(0);

            // Put the edge in the vertex that has the lowest index.
            if (vertex1 < vertex2)
                {
                if (arrayOfList[vertex1] == nullptr)
                    {
                    // If there is no list at this index create a new list
                    arrayOfList[vertex1] = gcnew System::Collections::ArrayList();
                    // Insert the second vertex
                    arrayOfList[vertex1]->Add(vertex2);
                    m_edgeCount++;
                    }
                else
                    {
                    if (!arrayOfList[vertex1]->Contains(vertex2))
                        {
                        // If this list does not contain the second vertex, inset it
                        arrayOfList[vertex1]->Add(vertex2);
                        m_edgeCount++;
                        }
                    }
                }
            else
                {
                if (arrayOfList[vertex2] == nullptr)
                    {
                    arrayOfList[vertex2] = gcnew System::Collections::ArrayList();
                    arrayOfList[vertex2]->Add(vertex1);
                    m_edgeCount++;
                    }
                else
                    {
                    if (!arrayOfList[vertex2]->Contains(vertex1))
                        {
                        arrayOfList[vertex2]->Add(vertex1);
                        m_edgeCount++;
                        }
                    }
                }

            // Move forward.
            vertex1 = vertex2;
            }
        }

    // Now we make the list of edges, first create the arry of edges.
    m_edges = gcnew array<MeshEdge^>(m_edgeCount);

    // Iterate on the array and get the edges.
    int iCount = 0;
    for (int iPoint = 0; iPoint < arrayOfList->Length; iPoint++)
        {
        if (arrayOfList[iPoint] != nullptr)
            {
            // Iterate on the edges of the entry and store them.
            for (int iEdge = 0; iEdge < arrayOfList[iPoint]->Count; iEdge++)
                {
                // Store a edge which first point is the current index, and second point is the entry of the array list.
                m_edges[iCount] = gcnew MeshEdge(iPoint, (System::Int32)(arrayOfList[iPoint][iEdge]));
                iCount++;
                }
            }
        }

    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE
