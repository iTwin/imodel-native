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

#include <ScalableMeshPCH.h>
#include <assert.h>
#include "Geometry.h"

/*---------------------------------------------------------------------------------**//**
*   Bounds : manage boundaries (or dimensions) of the whole model (dataset).
+---------------+---------------+---------------+---------------+---------------+------*/
void Bounds::Reset()
{
    minVec[X] = minVec[Y] = minVec[Z] = HUGE;
    maxVec[X] = maxVec[Y] = maxVec[Z] = -HUGE;
    center[X] = center[Y] = center[Z] = 0.0;
    radius = 0.0;
    points = 0;
}

void Bounds::AddPoint(const Vec3& v)
{
    if (v[X] < minVec[X]) minVec[X] = v[X];
    if (v[Y] < minVec[Y]) minVec[Y] = v[Y];
    if (v[Z] < minVec[Z]) minVec[Z] = v[Z];

    if (v[X] > maxVec[X]) maxVec[X] = v[X];
    if (v[Y] > maxVec[Y]) maxVec[Y] = v[Y];
    if (v[Z] > maxVec[Z]) maxVec[Z] = v[Z];

    center += v;
    points++;
}

void Bounds::Complete()
{
    center /= (double)points;
    Vec3 R1 = maxVec - center;
    Vec3 R2 = minVec - center;
    radius = max(length(R1), length(R2));
}

/*---------------------------------------------------------------------------------**//**
*   Plane
+---------------+---------------+---------------+---------------+---------------+------*/
void Plane::CalcFrom(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
    Vec3 v1 = p2 - p1;
    Vec3 v2 = p3 - p1;
    n = v1 ^ v2;
    unitize(n);
    d = -n*p1;
}

void Plane::CalcFrom(const array<Vec3>& verts)
{
    n[X] = n[Y] = n[Z] = 0.0;

    int i;
    for (i = 0; i<verts.length() - 1; i++)
    {
        const Vec3& cur = verts[i];
        const Vec3& next = verts[i + 1];
        n[X] += (cur[Y] - next[Y]) * (cur[Z] + next[Z]);
        n[Y] += (cur[Z] - next[Z]) * (cur[X] + next[X]);
        n[Z] += (cur[X] - next[X]) * (cur[Y] + next[Y]);
    }
    const Vec3& cur = verts[verts.length() - 1];
    const Vec3& next = verts[0];
    n[X] += (cur[Y] - next[Y]) * (cur[Z] + next[Z]);
    n[Y] += (cur[Z] - next[Z]) * (cur[X] + next[X]);
    n[Z] += (cur[X] - next[X]) * (cur[Y] + next[Y]);

    unitize(n);
    d = -n*verts[0];
}

/*---------------------------------------------------------------------------------**//**
*   Face3
+---------------+---------------+---------------+---------------+---------------+------*/
double TriangleArea(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
    Vec3 a = v2 - v1;
    Vec3 b = v3 - v1;
    return 0.5 * length(a ^ b);
}

double Face3::Area()
{
    return TriangleArea(VertexPos(0), VertexPos(1), VertexPos(2));
}

/*---------------------------------------------------------------------------------**//**
*   Face
+---------------+---------------+---------------+---------------+---------------+------*/
Face::Face(Edge *e0, Edge *e1, Edge *e2) : Face3(*e0->Org(), *e1->Org(), *e2->Org())
{
    edges[0] = e0;
    edges[1] = e1;
    edges[2] = e2;

    edges[0]->LinkFace(this);
    edges[1]->LinkFace(this);
    edges[2]->LinkFace(this);
}

void Face::RemapEdge(Edge *from, Edge *to)
{
    for (int i = 0; i<3; i++)
    {
        if (edges[i] == from)
        {
            edges[i] = to;
            to->LinkFace(this);
        }
        else if (edges[i] == from->Sym())
        {
            edges[i] = to->Sym();
            to->Sym()->LinkFace(this);
        }
    }
    InvalidatePlane();
}

/*---------------------------------------------------------------------------------**//**
*   Edge
+---------------+---------------+---------------+---------------+---------------+------*/
Edge::Edge(Vertex *a, Vertex *b)
{
    v1 = a;
    v1->LinkEdge(this);
    face_uses = new buffer<Face *>(2);
    twin = new Edge(this, b);
}

Edge::Edge(Edge *sibling, Vertex *Org)
{
    v1 = Org;
    v1->LinkEdge(this);
    face_uses = sibling->face_uses;
    twin = sibling;
}

Edge::~Edge()
{
    if (twin)
    {
        face_uses->freeMemory();
        delete face_uses;

        twin->twin = NULL;
        delete twin;
    }
}

void Edge::LinkFace(Face *face)
{
    face_uses->add(face);
}

void Edge::RemapEndpoint(Vertex *from, Vertex *to)
{
    if (Org() == from)
    {
        v1 = to;
        to->LinkEdge(this);
    }
    else if (Dest() == from)
    {
        twin->v1 = to;
        to->LinkEdge(twin);
    }

    // The cached Plane equations for the faces already attached may
    // no longer be valid (in general, chances are pretty slim that they're OK)
    for (int i = 0; i<face_uses->length(); i++)
    {
        face_uses->ref(i)->InvalidatePlane();
    }
}

/*---------------------------------------------------------------------------------**//**
*   Vertex
+---------------+---------------+---------------+---------------+---------------+------*/

void Vertex::LinkEdge(Edge *e)
{
    edge_uses.add(e);
}


/*---------------------------------------------------------------------------------**//**
*  
+---------------+---------------+---------------+---------------+---------------+------*/
int ClassifyEdge(Edge *e)
{
    int cls = e->FaceUses().length();
    if (cls>3) cls = 3;
    return cls;
}

int ClassifyVertex(Vertex *v)
{
    int border_count = 0;
    const edge_buffer& edges = v->EdgeUses();

    for (int i = 0; i<edges.length(); i++)
        if (ClassifyEdge(edges(i)) == 1)
            border_count++;

    if (border_count == edges.length())
        return VERTEX_BORDER_ONLY;
    else
        return (border_count > 0);
}