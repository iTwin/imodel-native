/*--------------------------------------------------------------------------*/ 
/*  Geometry.cpp															*/ 
/*	Geometry base class implementation										*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include <pt/Geometry.h>
#include <pt/memrw.h>
#include <pt/ptmath.h>

#include <gl/gl.h>

using namespace pt;

#define MAX_BOUND_VALUE 5000
namespace pt_geom
{
	vector3 g_basepoint(0,0,0);
	vector3d g_basepointd(0,0,0);

	typedef std::list<Geometry*> GEOMLIST;
	GEOMLIST g_instances;
}
using namespace pt_geom;

const vector3 &Geometry::basePoint() { return g_basepoint; }

/*--------------------------------------------------------------------------*/ 
/*	Constructor																*/ 
/*--------------------------------------------------------------------------*/ 
Geometry::Geometry()
{
	/*set to identity*/ 
	m_matrix = mmatrix4d::identity();
	m_imatrix = mmatrix4d::identity();
	
	m_translate.set(0.0f);
	m_rotate.set(0.0f);
	m_scale.set(1.0f);

	m_matrixdirty = false;
	m_componentsdirty = false;
	m_bbdirty = true;
	m_visible = true;

	m_matrix.getGLmatrix16(m_glm);
	m_glm[12] = matrix()(3, 0) - g_basepoint.x;
	m_glm[13] = matrix()(3, 1) - g_basepoint.y;
	m_glm[14] = matrix()(3, 2) - g_basepoint.z;	

	g_instances.push_back(this);
}
/*--------------------------------------------------------------------------*/ 
/*	Destructor																*/ 
/*--------------------------------------------------------------------------*/ 
Geometry::~Geometry()
{
	g_instances.remove(this);
	if (!g_instances.size())
		setBasePoint(vector3(0,0,0));
}
/*--------------------------------------------------------------------------*/ 
/*	setup Matrix															*/ 
/*--------------------------------------------------------------------------*/ 
void Geometry::setMatrix(	double a1, double a2, double a3, double a4,
							double b1, double b2, double b3, double b4,
							double c1, double c2, double c3, double c4,
							double d1, double d2, double d3, double d4)
{
	m_matrix(0,0) = a1;
	m_matrix(1,0) = a2;
	m_matrix(2,0) = a3;
	m_matrix(3,0) = a4;

	m_matrix(0,1) = b1;
	m_matrix(1,1) = b2;
	m_matrix(2,1) = b3;
	m_matrix(3,1) = b4;

	m_matrix(0,2) = c1;
	m_matrix(1,2) = c2;
	m_matrix(2,2) = c3;
	m_matrix(3,2) = c4;

	m_matrix(0,3) = d1;
	m_matrix(1,3) = d2;
	m_matrix(2,3) = d3;
	m_matrix(3,3) = d4;

	/*calc rotations + translations*/ 
	double tran[16];

	assert(0);
/*
	m_matrix.untransform(tran);

	m_scale.x = tran[U_SCALEY];
	m_scale.y = tran[U_SCALEY];
	m_scale.z = tran[U_SCALEY];

	m_rotate.x = tran[U_ROTATEX];
	m_rotate.y = tran[U_ROTATEY];
	m_rotate.z = tran[U_ROTATEZ];

	m_translate.x = tran[U_TRANSX];
	m_translate.y = tran[U_TRANSY];
	m_translate.z = tran[U_TRANSZ];
*/
	m_matrix.getGLmatrix16(m_glm);
	m_glm[12] = matrix()(3, 0) - g_basepointd.x;
	m_glm[13] = matrix()(3, 1) - g_basepointd.y;
	m_glm[14] = matrix()(3, 2) - g_basepointd.z;	


	/*inv matrix*/ 
	m_imatrix = m_matrix;
	m_imatrix.invert();

	m_matrixdirty = false;
	m_componentsdirty = false;

	normalizeCoordinateSystem();
}
/*--------------------------------------------------------------------------*/ 
/*	setup Matrix															*/ 
/*--------------------------------------------------------------------------*/ 
void Geometry::setMatrix(const mmatrix4d &mat)
{
	m_matrix = mat;

	/*calc rotations + translations*/ 
	double tran[16];

	m_matrix.untransform(tran);
/*
	m_scale.x = tran[U_SCALEX];
	m_scale.y = tran[U_SCALEY];
	m_scale.z = tran[U_SCALEZ];

	m_rotate.x = tran[U_ROTATEX];
	m_rotate.y = tran[U_ROTATEY];
	m_rotate.z = tran[U_ROTATEZ];

	m_translate.x = tran[U_TRANSX];
	m_translate.y = tran[U_TRANSY];
	m_translate.z = tran[U_TRANSZ];
*/
	/*inv matrix*/ 
	m_imatrix = m_matrix;
	m_imatrix.invert();

	/*setup gl*/ 
	m_matrix.getGLmatrix16(m_glm);
	m_glm[12] = matrix()(3, 0) - g_basepoint.x;
	m_glm[13] = matrix()(3, 1) - g_basepoint.y;
	m_glm[14] = matrix()(3, 2) - g_basepoint.z;	

	m_componentsdirty = false;
	m_matrixdirty = false;

	normalizeCoordinateSystem();
}
//
void Geometry::setBasePoint(const vector3d &bp)
{
	vector3d dbp = g_basepointd - bp;
	g_basepointd = bp;
	g_basepoint.set(bp);

	/*move bounding boxes of geometry*/ 
	GEOMLIST::iterator i = g_instances.begin();
	for (; i!=g_instances.end(); i++)
	{
		Geometry *g = (*i);
		
		/*update gl matrix*/ 
		g->m_glm[12] = g->matrix()(3, 0) - g_basepointd.x;
		g->m_glm[13] = g->matrix()(3, 1) - g_basepointd.y;
		g->m_glm[14] = g->matrix()(3, 2) - g_basepointd.z;	
	}
}
//
void Geometry::setBasePoint(const vector3 &bp)
{
	setBasePoint(vector3d(bp));
}
//
void Geometry::local2Normalized(const float *v, float * tr) const
{
	tr[0]	=	m_matrix(0,0) * v[0];
	tr[0]  +=	m_matrix(1,0) * v[1];
	tr[0]  +=	m_matrix(2,0) * v[2];
	tr[0]  +=   (m_matrix(3,0) - g_basepoint.x);	

	tr[1]	=	m_matrix(0,1) * v[0];
	tr[1]	+=	m_matrix(1,1) * v[1];
	tr[1]	+=	m_matrix(2,1) * v[2];
	tr[1]	+=	(m_matrix(3,1) - g_basepoint.y);	

	tr[2]	=	m_matrix(0,2) * v[0];
	tr[2]	+=	m_matrix(1,2) * v[1];
	tr[2]	+=	m_matrix(2,2) * v[2];
	tr[2]	+=	(m_matrix(3,2) - g_basepoint.z);	
}
//
void Geometry::normalized2Local(const float *v, float * tr) const
{
	vector3 v1(v);
	v1 += g_basepoint;

	tr[0]	=	m_imatrix(0,0) * v1.x;
	tr[0]	+=	m_imatrix(1,0) * v1.y;
	tr[0]	+=	m_imatrix(2,0) * v1.z;
	tr[0]	+=	m_imatrix(3,0);	

	tr[1]	=	m_imatrix(0,1) * v1.x;
	tr[1]	+=	m_imatrix(1,1) * v1.y;
	tr[1]	+=	m_imatrix(2,1) * v1.z;
	tr[1]	+=	m_imatrix(3,1);	

	tr[2]	=	m_imatrix(0,2) * v1.x;
	tr[2]	+=	m_imatrix(1,2) * v1.y;
	tr[2]	+=	m_imatrix(2,2) * v1.z;
	tr[2]	+=	m_imatrix(3,2);	
}
//
vector3 Geometry::local2Normalized(const float *v) const
{
	vector3 tr;
	local2Normalized(v, tr);
	return tr;	
}
//
void Geometry::local2Normalized(float *v) const
{
	vector3 tr;
	local2Normalized(v, tr);
	vector3::cast(v) = tr;	
}
//
vector3 Geometry::normalized2Local(const float *v) const
{
	vector3 tr;
	normalized2Local(v, tr);
	return tr;	
}
//
void Geometry::normalized2Local(float *v) const
{
	vector3 tr;
	normalized2Local(v, tr);
	vector3::cast(v) = tr;	
}
//
void Geometry::local2World(const float *v, float * tr) const
{
	m_matrix.vec3_multiply_mat4f(v, tr);
}
//
void Geometry::world2Local(const float *v, float * tr) const
{
	tr[0]	=	m_imatrix(0,0) * v[0]
			+	m_imatrix(1,0) * v[1]
			+	m_imatrix(2,0) * v[2]
			+	m_imatrix(3,0);	

	tr[1]	=	m_imatrix(0,1) * v[0]
			+	m_imatrix(1,1) * v[1]
			+	m_imatrix(2,1) * v[2]
			+	m_imatrix(3,1);	

	tr[2]	=	m_imatrix(0,2) * v[0]
			+	m_imatrix(1,2) * v[1]
			+	m_imatrix(2,2) * v[2]
			+	m_imatrix(3,2);	
}
//
vector3 Geometry::local2World(const float *v) const
{
	vector3 tr;
	local2World(v, tr);
	return tr;	
}
//
void Geometry::local2World(float *v) const
{
	vector3 tr;
	local2World(v, tr);
	vector3::cast(v) = tr;	
}
//
vector3 Geometry::world2Local(const float *v) const
{
	vector3 tr;
	world2Local(v, tr);
	return tr;	
}
//
void Geometry::world2Local(float *v) const
{
	vector3 tr;
	world2Local(v, tr);
	vector3::cast(v) = tr;	
}
//---------------------------------------------------------------------
// double versions
//---------------------------------------------------------------------
void Geometry::local2Normalized(const double *v, double * tr) const
{
	tr[0]	=	m_matrix(0,0) * v[0];
	tr[0]  +=	m_matrix(1,0) * v[1];
	tr[0]  +=	m_matrix(2,0) * v[2];
	tr[0]  +=   (m_matrix(3,0) - g_basepointd.x);	

	tr[1]	=	m_matrix(0,1) * v[0];
	tr[1]	+=	m_matrix(1,1) * v[1];
	tr[1]	+=	m_matrix(2,1) * v[2];
	tr[1]	+=	(m_matrix(3,1) - g_basepointd.y);	

	tr[2]	=	m_matrix(0,2) * v[0];
	tr[2]	+=	m_matrix(1,2) * v[1];
	tr[2]	+=	m_matrix(2,2) * v[2];
	tr[2]	+=	(m_matrix(3,2) - g_basepointd.z);	
}
//
void Geometry::normalized2Local(const double *v, double * tr) const
{
	vector3d v1(v);
	v1 += g_basepointd;

	tr[0]	=	m_imatrix(0,0) * v1.x;
	tr[0]	+=	m_imatrix(1,0) * v1.y;
	tr[0]	+=	m_imatrix(2,0) * v1.z;
	tr[0]	+=	m_imatrix(3,0);	

	tr[1]	=	m_imatrix(0,1) * v1.x;
	tr[1]	+=	m_imatrix(1,1) * v1.y;
	tr[1]	+=	m_imatrix(2,1) * v1.z;
	tr[1]	+=	m_imatrix(3,1);	

	tr[2]	=	m_imatrix(0,2) * v1.x;
	tr[2]	+=	m_imatrix(1,2) * v1.y;
	tr[2]	+=	m_imatrix(2,2) * v1.z;
	tr[2]	+=	m_imatrix(3,2);	
}
//
vector3d Geometry::local2Normalized(const double *v) const
{
	vector3d tr;
	local2Normalized(v, tr);
	return tr;	
}
//
void Geometry::local2Normalized(double *v) const
{
	vector3d tr;
	local2Normalized(v, tr);
	vector3d::cast(v) = tr;	
}
//
vector3d Geometry::normalized2Local(const double *v) const
{
	vector3d tr;
	normalized2Local(v, tr);
	return tr;	
}
//
void Geometry::normalized2Local(double *v) const
{
	vector3d tr;
	normalized2Local(v, tr);
	vector3d::cast(v) = tr;	
}
//
void Geometry::local2World(const double *v, double * tr) const
{
	m_matrix.vec3_multiply_mat4(v, tr);
}
//
void Geometry::world2Local(const double *v, double * tr) const
{
	tr[0]	=	m_imatrix(0,0) * v[0]
			+	m_imatrix(1,0) * v[1]
			+	m_imatrix(2,0) * v[2]
			+	m_imatrix(3,0);	

	tr[1]	=	m_imatrix(0,1) * v[0]
			+	m_imatrix(1,1) * v[1]
			+	m_imatrix(2,1) * v[2]
			+	m_imatrix(3,1);	

	tr[2]	=	m_imatrix(0,2) * v[0]
			+	m_imatrix(1,2) * v[1]
			+	m_imatrix(2,2) * v[2]
			+	m_imatrix(3,2);	
}
//
vector3d Geometry::local2World(const double *v) const
{
	vector3d tr;
	local2World(v, tr);
	return tr;	
}
//
void Geometry::local2World(double *v) const
{
	vector3d tr;
	local2World(v, tr);
	vector3d::cast(v) = tr;	
}
//
vector3d Geometry::world2Local(const double *v) const
{
	vector3d tr;
	world2Local(v, tr);
	return tr;	
}
//
void Geometry::world2Local(double *v) const
{
	vector3d tr;
	world2Local(v, tr);
	vector3d::cast(v) = tr;	
}

//---------------------------------------------------------------------
void Geometry::pushGLMatrix() const
{
	/*update extents*/ 
	glPushMatrix();
	glMultMatrixd(m_glm);
}
//---------------------------------------------------------------------
// update bounding box
//---------------------------------------------------------------------
void Geometry::updateBB()
{
	/*lazy evaluation*/ 
	if (m_bbdirty)
	{
		m_bb.clear();
		m_xbb.clear();
		m_xbs.clear();
		
		vector3 v, tr;

		int n = getNumVertices();
		for (int i=0; i<n; i++)
		{
			v = getVertex(i);
			local2Normalized(v, tr);
			m_bb.expandBy(v);
			m_xbb.expandBy(tr);
			m_xbs.expandBy(tr);
		}
		m_bbdirty = false;
	}
}
//---------------------------------------------------------------------
// normalize coordinate system
//---------------------------------------------------------------------
void Geometry::normalizeCoordinateSystem()
{
	vector3d t(0,0,0);
	GEOMLIST::iterator it=g_instances.begin(); 
	
	int geom_count = g_instances.size();

	if (!geom_count)
	{
		setBasePoint(vector3d(0,0,0));
		return;
	}
	while (it != g_instances.end())
	{
		(*it)->update();
		const BoundingBox &bb = (*it)->getTBoundingBox();
		if (!bb.isEmpty() && bb.valid())
			t += vector3d(bb.center());
		++it;
	}
	t /= (int)g_instances.size();	

	if (t.length() > MAX_BOUND_VALUE)
	{
		setBasePoint(t);
	}
	else
	{
		setBasePoint(vector3d(0,0,0));
	}
}
//---------------------------------------------------------------------
// update matrix from transforms
//---------------------------------------------------------------------
void Geometry::updateMatrix()
{ 
	/*scale, eulerZYX rotate, move*/ 
	if (m_matrixdirty || m_componentsdirty)
	{
		double sc[] = { m_scale.x, m_scale.y, m_scale.z, 1.0 };
		double tr[] = { m_translate.x, m_translate.y, m_translate.z, 1.0 };
		double rt[] = { m_rotate.x, m_rotate.y, m_rotate.z, 1.0 };

		m_matrix = mmatrix4d::scale(sc)
			>> mmatrix4d::rotation(rt[0], rt[1], rt[2]) 
			>> mmatrix4d::translation(tr);

		m_imatrix = m_matrix;
		m_imatrix.invert();
		m_matrix.getGLmatrix16(m_glm);

		m_matrixdirty = false;
	}
}
//---------------------------------------------------------------------
// Persistance
//---------------------------------------------------------------------
int	Geometry::blockOut(byte *buffer) const
{
	/*bounding boxes*/ 
	/*don't just write whole bb out because might change definition*/ 
	float bb [] = { m_bb.lx(), m_bb.ly(), m_bb.lz(), 
					m_bb.ux(), m_bb.uy(), m_bb.uz(), 
					m_xbb.lx(), m_xbb.ly(), m_xbb.lz(), 
					m_xbb.ux(), m_xbb.uy(), m_xbb.uz() };
	
	memrw r;
	r.reset();
	r.set_wdata(buffer);
	r.write_array(bb, 12);

	/*matrix*/ 
	r.write(m_matrix);
	r.write(m_imatrix);
	r.write_array(m_glm, 16);

	/*tranforms*/ 
	r.write(m_translate);
	r.write(m_rotate);
	r.write(m_scale);

	/*flags*/ 
	r.write(m_bbdirty);
	r.write(m_componentsdirty);
	r.write(m_matrixdirty);
	return r.byte_size;
}
//
//
//
int Geometry::blockIn(const byte *buffer)
{
	memrw r;
	r.reset();
	r.set_rdata(buffer);
	float bb[12];
	int pos =0 ;

	r.read_array(bb, 12, pos);
	
	m_bb.set(bb, &bb[3]);
	m_xbb.set(&bb[6], &bb[9]);

	/*matrix*/ 
	r.read(m_matrix, pos);
	r.read(m_imatrix, pos);
	r.read_array(m_glm, 16, pos);

	/*tranforms*/ 
	r.read(m_translate, pos);
	r.read(m_rotate, pos);
	r.read(m_scale, pos);

	/*flags*/ 
	r.read(m_bbdirty, pos);
	r.read(m_componentsdirty, pos);
	r.read(m_matrixdirty, pos);

	m_componentsdirty = false;
	return pos;
}
int	Geometry::blockSize() const
{
	return (12*sizeof(float)) + (2 * sizeof(mmatrix4d)) + (16 * sizeof(double)) + (3 * sizeof(vector3)) + (3*sizeof(byte));
}

