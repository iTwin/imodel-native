/*--------------------------------------------------------------------------*/ 
/*  Plane.cpp																*/ 
/*	Geometry base class implementation										*/ 
/*  (C) 2004 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 16 Feb 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#include <pt/Plane.h>
#include <Mgc/MgcCore3d.pkg>
#include <Mgc/MgcIntersection3d.pkg>
#include <Mgc/MgcMath.h>

#define MGCVEC(a) *reinterpret_cast<Mgc::Vector3*>((void*)&a)
#define PTVEC(a) *reinterpret_cast<pt::vector3*>((void*)&a)

using namespace pt;
//
// projectToPlane
//
bool Plane::projectToPlane(vector3 &v) const
{
	vector3 o(v);
	return intersectRay(o, m_normal, v);
}
//
// intersectRay
//
bool Plane::intersectRay(const Mgc::Ray3 &r, vector3 &res) const
{
	float t = -1;
	float d = m_normal.dot(PTVEC(r.Direction()));

	if (d != 0)
	{
		vector3 plane_origin = m_normal * m_constant;
		t = m_normal.dot(plane_origin - PTVEC(r.Origin())) / d;
		res = PTVEC(r.Origin()) + PTVEC(r.Direction()) * t;
		return true;
	}
	return false;
}
//
bool Plane::intersectRay(const vector3 &origin, const vector3 &dir, vector3 &res) const
{
	/*create ray and intersect with plane*/ 

	float t = -1;
	float d = m_normal.dot(dir);

	if (d != 0)
	{
		vector3 plane_origin = m_normal * m_constant;
		t = m_normal.dot(plane_origin - origin) / d;
		res = origin + dir * t;
		return true;
	}
	return false;
}
//
// intersect with a line
//
bool Plane::intersectLine(const vector3 &a, const vector3 &b, vector3 &i) const
{
	/*check endpoints are either side of plane, then do ray intersection*/ 
	if (whichSide(a) != whichSide(b)) 
	{
		return intersectRay(a, b - a, i);
	}
	return false;
}
//
// from 3 points
//
void Plane::from3points(const vector3 &a, const vector3 &b, const vector3 &c)
{
    vector3 edge1 = b - a;
    vector3 edge2 = c - a;
    m_normal = edge1.cross(edge2);
	m_normal.normalize();
    m_constant = m_normal.dot(a);
	m_base = a;
	m_transform_dirty = true;
}
//
// calculate Transformation matrix (Rotation)
//
void Plane::updateTransformData()
{
	/*make sure plane is normalized before doing this*/ 
	vector3 u, v;
	getUV(v,u);

	/*u,v and n form columns of rotation matrix*/ 
	m_transform[0][0] = u.x;
	m_transform[1][0] = u.y;
	m_transform[2][0] = u.z;

	m_transform[0][1] = v.x;
	m_transform[1][1] = v.y;
	m_transform[2][1] = v.z;

	m_transform[0][2] = m_normal.x;
	m_transform[1][2] = m_normal.y;
	m_transform[2][2] = m_normal.z;

	m_transform_dirty = false;
}
//
// get U and V vectors
//
void Plane::getUV(vector3 &u, vector3 &v) const
{
	const vector3 &n = m_normal;

	if (fabs(n.x) >= fabs(n.y))
	{
		/* n.x or n.z is the largest magnitude component*/ 
		float invLength = 1 / sqrt(n.x * n.x + n.z * n.z);
		u.x = n.z * invLength;
		u.y = 0;
		u.z = -n.x * invLength;
	}
	else
	{
		/*n.y or n.z is the largest magnitude component*/ 
		float invLength = 1 / sqrt(n.y * n.y + n.z * n.z);
		u.x = 0;
		u.y = n.z * invLength;
		u.z = -n.y * invLength;
	}

	v = n.cross(u);
}
//
// Transform point into Plane's coordinate system
//
void Plane::transformIntoPlaneCS(vector3 &v) const
{
	if (m_transform_dirty) const_cast<Plane*>(this)->updateTransformData();

	Mgc::Vector3 _v = m_transform * MGCVEC(v);
	v = PTVEC(_v);
	v += m_base;
}
//
// Plane from Transform Matrix
//
void Plane::fromTransformMatrix(const mmatrix4 &mat)
{
	/*columns of matrix = u, v, n*/ 
	mmatrix3 m;
	int i, j;
	for (i=0; i<3; i++)
		for (j=0; j<3; j++)
			m(i,j) = mat(i,j);

	m.orthoNormalize();
	normal(vector3(m(2,0), m(2,1), m(2,2)));
	base(vector3(mat(3,0), mat(3,1), mat(3,2)));
}
//
// whichSide
//
int Plane::whichSide(const vector3 &v) const
{
    float d = dist(v);

    if ( d < 0.0f ) return -1;
    if ( d > 0.0f ) return 1;
    return 0;
}
//
// to 3d
//
void Plane::to3D(const float *p, float *t) const
{
	if (m_transform_dirty) const_cast<Plane*>(this)->updateTransformData();

	/*use u,v basis vectors for 2d frame*/ 
	vector3 u(PTVEC(m_transform.GetColumn(0)));
	vector3 v(PTVEC(m_transform.GetColumn(1)));

	u *= p[0];
	v *= p[1];

	/*reuse u for effieciency*/ 
	u += v;
	u += m_base;
	t[0] = u.x;
	t[1] = u.y;
	t[2] = u.z;
}
//
// to 2d
//
void Plane::to2D(const float *q, float *t) const
{
	if (m_transform_dirty) const_cast<Plane*>(this)->updateTransformData();

	vector3 u(PTVEC(m_transform.GetColumn(0)));
	vector3 v(PTVEC(m_transform.GetColumn(1)));
	vector3 QP(q);
	QP -= m_base;

	//convert point Q
	//solve Q = P + xu + yv;
	//=>  x = u.(Q-P)
	//and y = v.(Q-P)

	t[0] = u.dot(QP);
	t[1] = v.dot(QP);
}
//
// assignment
//
const Plane &Plane::operator = (const Plane& p)
{
	normal(p.normal());
	base(p.base());
	return (*this);
}
//
// axis aligned box intersection
//
bool Plane::intersects(const BoundingBox &bb) const
{
	vector3 v;

	bb.getExtrema(0, v);
	int side = whichSide(v);

	for (int i=1; i< 8; i++)
	{
		bb.getExtrema(i, v);
		if (whichSide(v) != side) return true;
	}
	return false;
}	
