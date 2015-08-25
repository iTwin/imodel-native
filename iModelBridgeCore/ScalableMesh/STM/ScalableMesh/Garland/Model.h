/*--------------------------------------------------------------------------------------+
|
|   Code derived from Michael Garland's demo application called "QSlim" (version 1.0)
|   which intends to demonstrate an algorithm of mesh simplification based on
|   Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*---------------------------------------------------------------------------------**//**
*   Ajacency model representation (vertices and mesh connectivity)
+---------------+---------------+---------------+---------------+---------------+------*/
#include <string>
#include "Geometry.h"
#include "Vec3.h"
#include "Mat4.h"

typedef buffer<char *> string_buffer;
const int MAXLINE = 4096;

typedef void (*read_cmd)(string_buffer& argv);

class Model 
{
    protected:
        vert_buffer vertices;
        edge_buffer edges;
        face_buffer faces;

    private:
        void UpdateFace(Face*);

    public:
        Model() { }
        ~Model();

        Bounds bounds;
        int currentVertCount;
        int currentEdgeCount;
        int currentFaceCount;

        // Basic model accessor functions
        Vertex *GetVertex(int i);
        Edge *GetEdge(int i);
        Face *GetFace(int i);

        int InitialVerticeCount() { return vertices.length(); }
        int InitialEdgeCount() { return edges.length(); }
        int InitialFaceCount() { return faces.length(); }

        int CurrentVerticeCount() { return currentVertCount; }
        int CurrentEdgeCount() { return currentEdgeCount; }
        int CurrentFaceCount() { return currentFaceCount; }

        vert_buffer& AllVertices() { return vertices; }
        edge_buffer& AllEdges()    { return edges; }
        face_buffer& AllFaces()    { return faces; }

        // Simplification primitives:
        void KillVertex(Vertex *);
        void UnlinkEdgeFromVertex(Vertex* v, Edge* e);
        void RemapVertex(Vertex *from, Vertex *to);
        void KillEdge(Edge *);
        void UnlinkFaceFromEdge(Edge* e, Face *f);
        void RemapEdge(Edge* e1, Edge* e2);
        void KillFace(Face *);
        
        // Simplification convenience procedures
        void AssignNewCoordToVertex(Vertex *, double, double, double);   
        void RemapConnectivityAndKillFirstVertex(Vertex* v1, Vertex * v2);
        void Contract(Vertex *v1, Vertex *v2, const Vec3& to, face_buffer& changed); 
        void RemoveDegeneracy(face_buffer& changed);
        void ContractionRegion(Vertex *v1, Vertex *v2, face_buffer& changed);
        void ContractionRegion(Vertex *v1, const vert_buffer& vertices, face_buffer& changed);

        // Functions that were before in AdjPrims.cxx now Settings.h:
        void UntagFaceLoop(Vertex *v);
        void CollectFaceLoop(Vertex *v, face_buffer& faces);

        
        int AddVertice(double x, double y, double z);
        int AddFace(int32_t indexA, int32_t indexB, int32_t indexC);
        Edge *AddEdge(Vertex *org, Vertex *v);
        void GetRemainingPoints(vector<DPoint3d>& points);
        void GetRemainingFaceIndexes(vector<int32_t>& faceIndexes);

        // For debug.
        void OutputToFile(string outputFilepath); 
};