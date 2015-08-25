/*--------------------------------------------------------------------------------------+
|
|   Code partially derived from Michael Garland's demo application called "QSlim" 
|   (version 1.0) which intends to demonstrate an algorithm of mesh simplification 
|   based on Garland and Heckbert(1997) "Surface Simplification Using Quadric Error 
|   Metrics". The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshPCH.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Model.h"

Model::~Model()
{     
    for (int ind = 0; ind < faces.length(); ind++)
        {
        delete faces[ind];
        }

    for (int ind = 0; ind < edges.length(); ind++)
        {
        delete edges[ind];
        }

    for (int ind = 0; ind < vertices.length(); ind++)
        {
        delete vertices[ind];
        }        

    vertices.freeMemory();
    edges.freeMemory();
    faces.freeMemory();   
}

int Model::AddVertice(double x, double y, double z)
{
    Vec3 p;
    p[X] = x;
    p[Y] = y;
    p[Z] = z;
    
    Vertex *v = new Vertex(p[X], p[Y], p[Z]);
    v->uniqID = vertices.add(v);
    currentVertCount++;
    
    bounds.AddPoint(p);
    return InitialVerticeCount() - 1;
}

int Model::AddFace(int32_t indexA, int32_t indexB, int32_t indexC)
{
buffer<int32_t> points(3);
    points.add(indexA);
    points.add(indexB);
    points.add(indexC);

    Vertex *v1 = vertices(points(0) - 1);
    Vertex *v2 = vertices(points(1) - 1);
    Vertex *v3 = vertices(points(2) - 1);
   
    Edge *e0 = AddEdge(v1, v2);
    Edge *e1 = AddEdge(v2, v3);
    Edge *e2 = AddEdge(v3, v1);

    Face *t = new Face(e0, e1, e2);
    t->uniqID = faces.add(t);
    currentFaceCount++;
    
    return InitialFaceCount() - 1;
}

Edge *Model::AddEdge(Vertex *org, Vertex *v)
{
    edge_buffer& edge_uses = org->EdgeUses();

    for (int i = 0; i < edge_uses.length(); i++)
    {
        if (edge_uses(i)->Dest() == v)
        {
            return edge_uses(i);
        }
    }

    Edge *e = new Edge(org, v);
    e->uniqID = edges.add(e);
    e->Sym()->uniqID = e->uniqID;
    currentEdgeCount++;

    return e;
}

void Model::GetRemainingPoints(vector<DPoint3d>& points)
{
    int uniqVerts = 0;
    DPoint3d pt;

    for (int i = 0; i < this->InitialVerticeCount(); i++)
    {
        if (this->GetVertex(i)->IsValid())
        {
            this->GetVertex(i)->tempID = ++uniqVerts; 

            pt.x = (*this->GetVertex(i))[X];
            pt.y = (*this->GetVertex(i))[Y];
            pt.z = (*this->GetVertex(i))[Z];
            points.push_back(pt);
        }
        else
        {
            this->GetVertex(i)->tempID = -1;
        }
    }
}

void Model::GetRemainingFaceIndexes(vector<int32_t>& faceIndexes)
{
    for (int i = 0; i < this->InitialFaceCount(); i++)
    {
        if (this->GetFace(i)->IsValid())
        {
            Face *f = this->GetFace(i);

            Vertex *v0 = (Vertex *)f->GetVertex(0);
            Vertex *v1 = (Vertex *)f->GetVertex(1);
            Vertex *v2 = (Vertex *)f->GetVertex(2);
            
            int32_t indexA = (int32_t)v0->tempID;
            int32_t indexB = (int32_t)v1->tempID;
            int32_t indexC = (int32_t)v2->tempID;

            faceIndexes.push_back(indexA);
            faceIndexes.push_back(indexB);
            faceIndexes.push_back(indexC);
        }
    }
}

Vertex* Model::GetVertex(int i) 
{ 
    assert(i >= 0 && i < vertices.length());
    return vertices(i); 
}

Edge* Model::GetEdge(int i) 
{ 
    assert(i >= 0 && i < edges.length());
    return edges(i); 
}

Face* Model::GetFace(int i) 
{ 
    assert(i >= 0 && i < faces.length());
    return faces(i); 
}

/*---------------------------------------------------------------------------------**//**
*   Model Transmogrification (alteration):
*
*   These routines are the basic model munging procedures. All model simplification 
*   is implemented in terms of these transmogrification primitives.
+---------------+---------------+---------------+---------------+---------------+------*/
void Model::KillVertex(Vertex *v)
{
    if (v->IsValid())
    {
        v->MarkInvalid();
        v->EdgeUses().reset();
        currentVertCount--;
    }
}

void Model::UnlinkEdgeFromVertex(Vertex* v, Edge* e)
{
    int index = v->EdgeUses().find(e);
    v->EdgeUses().remove(index);
    if (v->EdgeUses().length() <= 0)
    {
        KillVertex(v);
    }
}

void Model::RemapVertex(Vertex* from, Vertex * to)
{
    if (from != to)
    {
        for (int i = 0; i < from->EdgeUses().length(); i++)
        {
            assert(from->EdgeUses()[i]->Org() == from);
            from->EdgeUses()[i]->RemapEndpoint(from, to);
        }
        KillVertex(from);
    }
}

void Model::KillEdge(Edge *e)
{
    if (e->IsValid())
    {
        UnlinkEdgeFromVertex(e->Org(), e);
        UnlinkEdgeFromVertex(e->Dest(), e->Sym());
        e->MarkInvalid();
        e->Sym()->MarkInvalid();
        e->FaceUses().reset();
        currentEdgeCount--;
    }
}

void Model::UnlinkFaceFromEdge(Edge* e, Face *f)
{
    int index = e->FaceUses().find(f);
    e->FaceUses().remove(index);
    if (e->FaceUses().length() == 0)
    {
        KillEdge(e);
    }
}

void Model::RemapEdge(Edge* e1, Edge* e2)
{
    if (e1 != e2)
    {
        for (int i = 0; i < e1->FaceUses().length(); i++)
        {
            e1->FaceUses()[i]->RemapEdge(e1, e2);
        }
        KillEdge(e1); // Disconnect from all faces and vertices
    }
}

void Model::KillFace(Face *f)
{
    if (f->IsValid())
    {
        if (f->GetEdge(0)->IsValid()) UnlinkFaceFromEdge(f->GetEdge(0), f);
        if (f->GetEdge(1)->IsValid()) UnlinkFaceFromEdge(f->GetEdge(1), f);
        if (f->GetEdge(2)->IsValid()) UnlinkFaceFromEdge(f->GetEdge(2), f);
        f->MarkInvalid();
        currentFaceCount--;
    }
}

void Model::AssignNewCoordToVertex(Vertex *v, double x, double y, double z)
{
    v->set(x, y, z);
}

// Remap the connectivity of vertex v1
// on vertex v2 and kill v1.
void Model::RemapConnectivityAndKillFirstVertex(Vertex *v1, Vertex* v2)
{
    if (v1 != v2)
    {
        for (int i = 0; i < v1->EdgeUses().length(); i++)
        {
            assert(v1->EdgeUses()[i]->Org() == v1);
            v1->EdgeUses()[i]->RemapEndpoint(v1, v2);
        }
        KillVertex(v1);
    }
}

void Model::Contract(Vertex *v1, Vertex *v2, const Vec3& to, face_buffer& changed)
{
    //--------------------------------------
    //             to
    //             *            
    //     v1  *       * v2
    // 
    // The initial pair (v1, v2) will be  
    // collapsed into position "to".
    //--------------------------------------

    // Collect all the faces that are going to be changed:
    ContractionRegion(v1, v2, changed);

    // v1 gets the coordinates of "to":
    AssignNewCoordToVertex(v1, to[X], to[Y], to[Z]); 
    
    // Remap the connectivity of v2 onto v1 and kill v2:
    RemapConnectivityAndKillFirstVertex(v2, v1);

    // Cleanup faces around position "to":
    RemoveDegeneracy(changed);
}

void Model::UpdateFace(Face *F)
{
    //--------------------------------------
    //             v0
    //             * 
    //       e2  *    *  e0  
    //         *        *  
    //       *            *
    //  v2  * * * * * * * * *  v1
    //             e1
    //--------------------------------------

    Vertex *v0 = F->GetVertex(0);
    Vertex *v1 = F->GetVertex(1);
    Vertex *v2 = F->GetVertex(2);
    Edge *e0 = F->GetEdge(0);
    Edge *e1 = F->GetEdge(1);
    Edge *e2 = F->GetEdge(2);

    bool a = (v0 == v1); // e0 has been removed. 
    bool b = (v0 == v2); // e2 has been removed.
    bool c = (v1 == v2); // e1 has been removed.

    if (a && c) 
    {
        // This triangle has been reduced to a point.
        KillEdge(e0);
        KillEdge(e1);
        KillEdge(e2);
        KillFace(F);
    }
    // In the following 3 cases, the triangle has become an edge
    else if (a)
    {
        KillEdge(e0);
        RemapEdge(e1, e2->Sym());
        KillFace(F); 
    }
    else if (b)
    {
        KillEdge(e2);
        RemapEdge(e0, e1->Sym());
        KillFace(F);
    }
    else if (c)
    {
        KillEdge(e1);
        RemapEdge(e0, e2->Sym());
        KillFace(F);
    }
    else
    {
        // This triangle remains non-degenerate
    }   
}

void Model::RemoveDegeneracy(face_buffer& changed)
{
    for (int i = 0; i < changed.length(); i++)
    {
        UpdateFace(changed(i));
    }        
}

void Model::ContractionRegion(Vertex *v1, const vert_buffer& vertices, face_buffer& changed)
{
    changed.reset();

    // First, reset the marks on all reachable vertices:
    int i;
    UntagFaceLoop(v1);

    for (i = 0; i < vertices.length(); i++)
    {
        UntagFaceLoop(vertices(i));
    }
        
    // Now, pick out all the unique reachable faces:
    CollectFaceLoop(v1, changed);
    for (i = 0; i < vertices.length(); i++)
    {
        CollectFaceLoop(vertices(i), changed);
    }        
}

void Model::ContractionRegion(Vertex *v1, Vertex *v2, face_buffer& changed)
{
    changed.reset();

    // Clear marks on all reachable faces
    UntagFaceLoop(v1);
    UntagFaceLoop(v2);

    // Collect all the unique reachable faces
    CollectFaceLoop(v1, changed);
    CollectFaceLoop(v2, changed);
}

void Model::UntagFaceLoop(Vertex *v)
{
    edge_buffer& edges = v->EdgeUses();

    for (int j = 0; j<edges.length(); j++)
    {
        face_buffer& faces = edges(j)->FaceUses();
        for (int k = 0; k<faces.length(); k++)
            faces(k)->Untag();
    }
}

void Model::CollectFaceLoop(Vertex *v, face_buffer& loop)
{
    edge_buffer& edges = v->EdgeUses();

    for (int j = 0; j<edges.length(); j++)
    {
    face_buffer& faces = edges(j)->FaceUses();
    for (int k = 0; k<faces.length(); k++)
        if (!faces(k)->IsTagged())
        {
            loop.add(faces(k));
            faces(k)->Tag();
        }
    }
}

/*---------------------------------------------------------------------------------**//**
*   Save the current model as an obj file.
+---------------+---------------+---------------+---------------+---------------+------*/
void Model::OutputToFile(string outputFilepath)
{
    int verticeCount = 0;
    int faceCount = 0;
    std::stringstream ss;
    int uniqVerts = 0;

    for (int i = 0; i < this->InitialVerticeCount(); i++)
    {
        if (this->GetVertex(i)->IsValid())
        {
            this->GetVertex(i)->tempID = ++uniqVerts;
            const Vertex& v = *this->GetVertex(i);
            ss << "v " << v[X] << " " << v[Y] << " " << v[Z] << endl;
            verticeCount++;
        }
        else
        {
            this->GetVertex(i)->tempID = -1;
        }
    }
    for (int i = 0; i < this->InitialFaceCount(); i++)
    {
        if (this->GetFace(i)->IsValid())
        {
            Face *f = this->GetFace(i);
            ss << "f ";
            Vertex *v0 = (Vertex *)f->GetVertex(0);
            Vertex *v1 = (Vertex *)f->GetVertex(1);
            Vertex *v2 = (Vertex *)f->GetVertex(2);
            ss << v0->tempID << " " << v1->tempID << " " << v2->tempID << endl;
            faceCount++;
        }
    }
   
    string fileBody = ss.str();
    string h = "";
    h += "# OBJ file format with ext.obj\n";
    h += "# vertex count = "; h += to_string(verticeCount); h += "\n";
    h += "# face count = "; h += to_string(faceCount); h += "\n";

    string file = h; file += fileBody;

    ofstream out;
    out.open(outputFilepath);

    if (out)
    {
        out << file;
        out.close();
    }       
}
