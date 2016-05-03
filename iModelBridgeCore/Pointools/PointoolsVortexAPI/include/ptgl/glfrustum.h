/*--------------------------------------------------------------------------*/ 
/*	Pointools GLtext class definition and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_GLFRUSTUM
#define POINTOOLS_GLFRUSTUM

#include <ptgl/ptgl.h>
#include <pt/BoundingBox.h>
#include <pt/BoundingSphere.h>
#include <pt/plane.h>

namespace ptgl
{
class PTGL_API Frustum  
{
public:
	Frustum();
	~Frustum();
	enum FrustumIntersect
	{
		FRUSTUM_FULL,
		FRUSTUM_PARTIAL,
		FRUSTUM_CULLED
	};
#ifdef HAVE_OPENGL
    void buildFrustum();
#endif
	void buildFrustum(const double *projection_matrix, const double *modelview_matrix);
	
	template<typename T> 
	bool inFrustum(const pt::BBox<T> *box) const
	{
		int p;

		for( p = 0; p < 6; p++ )
		{
			if( m_frustum[p][0] * box->lx() + m_frustum[p][1] * box->ly() + m_frustum[p][2] * box->lz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->ux() + m_frustum[p][1] * box->ly() + m_frustum[p][2] * box->lz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->lx() + m_frustum[p][1] * box->uy() + m_frustum[p][2] * box->lz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->ux() + m_frustum[p][1] * box->uy() + m_frustum[p][2] * box->lz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->lx() + m_frustum[p][1] * box->ly() + m_frustum[p][2] * box->uz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->ux() + m_frustum[p][1] * box->ly() + m_frustum[p][2] * box->uz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->lx() + m_frustum[p][1] * box->uy() + m_frustum[p][2] * box->uz() + m_frustum[p][3] > 0 )
				continue;
			if( m_frustum[p][0] * box->ux() + m_frustum[p][1] * box->uy() + m_frustum[p][2] * box->uz() + m_frustum[p][3] > 0 )
				continue;
			return false;
		}
		return true;
	}
	bool inFrustum(const pt::BoundingSphere *sp) const;

	template<typename T> 
	bool inFrustum(const pt::vec3<T> *v) const
	{
		int p;

		for( p = 0; p < 6; p++ )
			if( m_frustum[p][0] * v->x + m_frustum[p][1] * v->y + m_frustum[p][2] * v->z + m_frustum[p][3] <= 0 )
				 return false;
		return true;
	}

	template<typename T>
	FrustumIntersect containedInFrustum(const pt::BBox<T> *bx) const
	{
		/*check all corners are contained*/ 
		pt::vec3<T> v;
		int c=0;
		for (int i=0; i<8; i++)
		{
			bx->getExtrema(i, v);
			if (inFrustum(&v)) c++;
		}
		return c ? ((c==8) ? FRUSTUM_FULL : FRUSTUM_PARTIAL ) : FRUSTUM_CULLED;
	}

	Frustum &operator = (const Frustum &f)
	{
		memcpy(this, &f, sizeof(Frustum));
		return *this;
	}

	void leftPlane( pt::Planed &plane ) const;
	void rightPlane( pt::Planed &plane ) const;
	void topPlane( pt::Planed &plane ) const;
	void bottomPlane( pt::Planed &plane ) const;
	void nearPlane( pt::Planed &plane ) const;
	void farPlane( pt::Planed &plane ) const;

private:
	double m_frustum[6][4];
};
}
#endif 
