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
*   Primitive entities used inside adjacency models.
+---------------+---------------+---------------+---------------+---------------+------*/

#include "Buffer.h"
#include "Vec4.h"

#define EDGE_BOGUS 0
#define EDGE_BORDER 1
#define EDGE_MANIFOLD 2
#define EDGE_NONMANIFOLD 3
#define VERTEX_INTERIOR 0
#define VERTEX_BORDER 1
#define VERTEX_BORDER_ONLY 2

class Vertex;
class Edge;
class Face;

typedef buffer<Vertex *> vert_buffer;
typedef buffer<Edge *> edge_buffer;
typedef buffer<Face *> face_buffer;

extern int ClassifyEdge(Edge *);
extern int ClassifyVertex(Vertex *);

/*---------------------------------------------------------------------------------**//**
*   NPrim and NTaggedPrim : used for modeling primitives (shared primitive elements).
+---------------+---------------+---------------+---------------+---------------+------*/
class NPrim
{
    public:
        int uniqID;
        inline bool IsValid()     { return uniqID >= 0; }
        inline void MarkInvalid() { if (uniqID >= 0) uniqID = -uniqID - 1; }
        inline void MarkValid()   { if (uniqID<0) uniqID = -uniqID - 1; }
        inline int  ValidID()     { return (uniqID<0) ? (-uniqID - 1) : uniqID; }
};

class NTaggedPrim : public NPrim
{
    public:
        int tempID;
        inline void Untag() { tempID = 0; }
        inline void Tag(int t = 1) { tempID = t; }
        inline bool IsTagged() { return tempID != 0; }
};

/*---------------------------------------------------------------------------------**//**
*   Bounds
+---------------+---------------+---------------+---------------+---------------+------*/
class Bounds
{
    public:
        Vec3 minVec;
        Vec3 maxVec;
        Vec3 center;
        double radius;
        unsigned int points;

        Bounds() { Reset(); }
        void Reset();
        void AddPoint(const Vec3&);
        void Complete();
};

/*---------------------------------------------------------------------------------**//**
*   Plane
+---------------+---------------+---------------+---------------+---------------+------*/
class Plane
{
        // A plane is defined by the equation:  n*p + d = 0
        Vec3 n;
        double d;

    public:

        Plane() : n(0, 0, 1) { d = 0; } // This will define the XY plane
        Plane(const Vec3& p, const Vec3& q, const Vec3& r) { CalcFrom(p, q, r); }
        Plane(const array<Vec3>& verts) { CalcFrom(verts); }
        Plane(const Plane& p) { n = p.n; d = p.d; }

        void CalcFrom(const Vec3& p, const Vec3& q, const Vec3& r);
        void CalcFrom(const array<Vec3>&);
        bool IsValid() const { return n[X] != 0.0 || n[Y] != 0.0 || n[Z] != 0.0; }
        void MarkInvalid() { n[X] = n[Y] = n[Z] = 0.0; }
        double DistTo(const Vec3& p) const { return n*p + d; }
        const Vec3& Normal() const { return n; }
        void Coeffs(double *a, double *b, double *c, double *dd) const { *a = n[X]; *b = n[Y]; *c = n[Z]; *dd = d;}
        Vec4 Coeffs() const { return Vec4(n, d); }
};

/*---------------------------------------------------------------------------------**//**
*   Face3
+---------------+---------------+---------------+---------------+---------------+------*/
class Face3
{
    protected:
        Plane P;

    private:

        void RecalcPlane() { P.CalcFrom(VertexPos(0), VertexPos(1), VertexPos(2)); }
        void RecalcPlane(const Vec3& a, const Vec3& b, const Vec3& c)
        {
            P.CalcFrom(a, b, c);
        }

    public:

        Face3(const Vec3& a, const Vec3& b, const Vec3& c) : P(a, b, c){ }
        virtual const Vec3& VertexPos(int i) const = 0;
        virtual void VertexPos(int i, const Vec3&) = 0;
        const Plane& Plane() { if (!P.IsValid()) RecalcPlane();   return P; }
        void InvalidatePlane() { P.MarkInvalid(); }
        const double DistTo(const Vec3& p);
        double Area();
};

/*---------------------------------------------------------------------------------**//**
*   Vertex
+---------------+---------------+---------------+---------------+---------------+------*/
class Vertex : public Vec3, public NTaggedPrim
{
        edge_buffer edge_uses;

    public:

        Vertex(double x, double y, double z) : Vec3(x, y, z), edge_uses(6) {}
        edge_buffer& EdgeUses() { return edge_uses; }
        void LinkEdge(Edge *);
};

/*---------------------------------------------------------------------------------**//**
*   Edge
+---------------+---------------+---------------+---------------+---------------+------*/
class Edge : public NPrim
{
    private:

        Vertex *v1;
        face_buffer *face_uses;
        Edge *twin;
        Edge(Edge *twin, Vertex *Org); // the twin constructor

    public:
        Edge(Vertex *, Vertex *);
        ~Edge();

        // Fundamental Edge accessors
        Vertex *Org()  { return v1; }       // Origin
        Vertex *Dest() { return twin->v1; } // Destination
        Edge *Sym()    { return twin; }     // Returns the same edge in the opposite direction.

        // Standard methods for all objects
        face_buffer& FaceUses() { return *face_uses; }

        // Basic primitives for manipulating model topology
        void LinkFace(Face *);
        void RemapEndpoint(Vertex *from, Vertex *to);
};

/*---------------------------------------------------------------------------------**//**
*   Face
+---------------+---------------+---------------+---------------+---------------+------*/
class Face : public Face3, public NTaggedPrim
{
        Edge *edges[3];

    public:

        Face(Edge *, Edge *, Edge *);

        // Accessors
        const Vec3& VertexPos(int i) const { return *edges[i]->Org(); }
        void VertexPos(int, const Vec3&) { /*fatal_error("Face: can't directly set Vertex position.");*/ }        
        Vertex *GetVertex(int i) { return edges[i]->Org(); }
        const Vertex *GetVertex(int i) const { return edges[i]->Org(); }
        Edge *GetEdge(int i) { return edges[i]; }

        void RemapEdge(Edge *from, Edge *to);
};



