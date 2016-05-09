/*--------------------------------------------------------------------------*/ 
/*  Fence.h																	*/ 
/*	Fence class definition													*/ 
/*  (C) 2005 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 24 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLS_FENCE_CLASS
#define POINTOOLS_FENCE_CLASS 1
#include <pt/geomtypes.h>
#include <ptgeom/geom.h>
#include <pt/rect.h>
#include <vector>

namespace pt
{
template <class Real>
class Fence
{
public:
	enum FenceIntersectResult
	{
		None=0,
		Intersects=1,
		Contains=2
	};
	typedef pt::vec2< Real > PointType;

	Fence() { m_bounds.makeEmpty(); }
	Fence(const Fence< Real > &f)
	{
		for (int i=0; i< f.numPoints(); i++) addPoint(f[i]);
	}
	void addPoint(const PointType &v) 
	{ 
		if (!m_points.size() || v != m_points.back())
		{
			m_points.push_back(v); 
			m_bounds.expand(v); 
		}
	}
	void clear() { m_points.clear(); }

	int numPoints() const { return static_cast<int>(m_points.size()); }
	
	bool intersectsEdge(Real p0x, Real p0y, Real p1x, Real p1y) const
	{
		return intersectsEdge(PointType(p0x,p0y), PointType(p1x,p1y));
	}
#ifdef NEEDS_WORK_VORTEX_DGNDB 
	bool isConvex() const
	{
		int j=0; bool p = false; bool n = false;
		for (int i=0; i<size; i++)
		{	
			++j;
			const PointType &p0 = m_points[i];
			const PointType &p1 = m_points[(i+1)%size];
			const PointType &p2 = m_points[(i+2)%size];
			
			if (isLeft(p1,p1,p2))
			{
				if (n) return false;
				p = true;
			}
			else
			{
				if (p) return false;
				n = true;
			}
		}
		return true;
	}
#endif
	static bool isLeft(const PointType &p0, const PointType &p1, const PointType &p2 )
	{
		return ((p1.x - p0.x)*(p2.y - p0.y) - (p2.x - p0.x)*(p1.y - p0.y) > 0) ?
			true : false;
	}

	bool intersectsEdge(const PointType &p0, const PointType &p1) const
	{
		PointType a, b;
		int size = numPoints();
		int j=0;

		for (int i=0; i<size; i++)
		{	
			++j;
			a.set(m_points[i].x, m_points[i].y);
			b.set(m_points[j%size].x, m_points[j%size].y);

			if (isLeft(a, b, p0) != isLeft(a, b, p1) && 
				isLeft(p0, p1, a) != isLeft(p0, p1, b)) 
				return true;
		}		
		return false;
	}
	FenceIntersectResult intersectsRect(const Rect< Real > &r) const
	{		
		if (!m_bounds.intersects(&r) && !m_bounds.contains(&r)) return None;

		if (intersectsEdge( PointType(r.lx(), r.ly()), PointType(r.ux(), r.ly()) ) 
				|| intersectsEdge( PointType(r.ux(), r.ly()), PointType(r.ux(), r.uy()) ) 
				|| intersectsEdge( PointType(r.lx(), r.uy()), PointType(r.ux(), r.uy()) ) 
				|| intersectsEdge( PointType(r.lx(), r.ly()), PointType(r.ux(), r.uy()) )
				)
			return Intersects;
		return  pntInFence(PointType(r.lx(), r.ly())) ? Contains : None;
	}
	bool containsRect(const Rect< Real > &r) const
	{
		if (m_points.size() < 2 || !m_bounds.contains(&r))
			return false;

		return ( pntInFence( PointType(r.lx(), r.ly()) ) && !intersectsRect(r))
			? true : false;
	}
	bool pntInFence(const PointType &p) const
	{
		return pntInFence(p.x,p.y);
	}
	bool pntInFence(Real x, Real y) const
	{
		int size = numPoints();

		if (size > 2)
		{
			bool oddNodes = false;

			float x_i, x_j, y_i, y_j;

			int j = 0;

			for (int i=0; i<size; i++)
			{
				++j;
				
				x_i = static_cast<float>(m_points[i].x);
				y_i = static_cast<float>(m_points[i].y);
				x_j = static_cast<float>(m_points[j%size].x);
				y_j = static_cast<float>(m_points[j%size].y);

				if (y_i < y && y_j >= y || y_j < y && y_i >= y)
					if (x_i + (y - y_i) /(y_j - y_i) * (x_j - x_i) < x)
						oddNodes = !oddNodes;
			}
			return oddNodes;
		}
		else
			return false;		
	}
	const pt::Bounds<2, Real>			&bounds() const { return m_bounds; }
	
	inline const PointType &point(int i) const { return m_points[i]; }
	inline PointType &operator [] (int i) { return m_points[i]; }
	inline const PointType &operator [] (int i) const { return m_points[i]; }

private:
	std::vector< PointType >	m_points;
	pt::Bounds<2, Real>			m_bounds;
};
typedef Fence<float> Fencef;
typedef Fence<double> Fenced;
}
#endif