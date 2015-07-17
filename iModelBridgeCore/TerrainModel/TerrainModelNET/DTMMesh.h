/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMMesh.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include ".\Bentley.Civil.DTM.h"
using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

//=======================================================================================
/// <summary>
/// Class that represents a mesh edge.
/// </summary>    
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
private ref struct MeshEdge
    {
    public: 
        int Vertex1;
        int Vertex2;

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the MeshEdge class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        MeshEdge (int vertex1, int vertex2)
            {
            Vertex1 = vertex1;
            Vertex2 = vertex2;
            }
    };

ref struct Mesh;

//=======================================================================================
/// <summary>
/// Class that represents a mesh face.
/// </summary>    
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref struct MeshFace
    {
    private: 
        array<int>^ m_indices;
        Mesh^ m_mesh;
        ~MeshFace();

    internal: 

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the MeshFace class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        MeshFace (Mesh^ mesh, array<int>^ indices);

    public: 

        //=======================================================================================
        /// <summary>
        /// Gets the coordinates of a point by index.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        BGEO::DPoint3d GetCoordinates(int index);

        //=======================================================================================
        /// <summary>
        /// Gets the index of a point by its index in the face.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        int GetMeshPointIndex (int index);
    };

//=======================================================================================
/// <summary>
/// Class that represents a mesh.
/// </summary>    
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref struct Mesh: System::IDisposable
    {
    private: 

        BcDTMMeshP m_mesh;
        ~Mesh();
        !Mesh();
        void BuildEdges();
        int m_edgeCount;
        array<MeshEdge^>^ m_edges;

    internal: 

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the Mesh class.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        Mesh (BcDTMMeshP mesh);

    public: 

        //=======================================================================================
        /// <summary>
        /// Gets the count of points in the mesh.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property int PointCount
            {
            int get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets the count of faces in the mesh.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property int FaceCount
            {
            int get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets a point by its index.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        BGEO::DPoint3d GetPoint (int index);

        //=======================================================================================
        /// <summary>
        /// Gets a face by its index.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        MeshFace^ GetFace(int index);

        //=======================================================================================
        /// <summary>
        /// Gets number of edges.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property int EdgeCount
            {
            int get();
            }

        //=======================================================================================
        /// <summary>
        /// Gets a edge by its index.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        array<BGEO::DPoint3d>^ GetEdge(int index);
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
