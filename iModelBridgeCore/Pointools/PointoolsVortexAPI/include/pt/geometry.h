#ifndef POINTOOLS_GEOMETRY3D_TEMPLATE_H
#define POINTOOLS_GEOMETRY3D_TEMPLATE_H

#include <pt/scenegraph.h>

namespace pt
{
template <class ObjectType, class VertexType>
class ArrayVertexRange
{
public:
	ArrayVertexRange(const ObjectType*o, VertexType &hint, pt::CoordinateSpace cs)
	{
		b = &o->vertex(0);
		e = b + o->numVertices();
		i = o->numVertices() ? b : e;
	}
	ArrayVertexRange(const ObjectType*o)
	{
		b = &o->vertex(0);
		e = b + o->numVertices();
		i = o->numVertices() ? b : e;
	}

	typedef VertexType* Iterator;
	typedef VertexType Type;
	typedef ObjectType ObjType;

	const Iterator begin() { return b; }
	const Iterator end() { return e; }
	
private:
	const VertexType *i;
	const VertexType *b;
	const VertexType *e;
};
//-------------------------------------------------------------------------
// Geometry3D
/// Simplifies some overloading of Object3D
//-------------------------------------------------------------------------
template<class ConstVertexRange>
class Geometry3D	: public Object3D
{
public:
	Geometry3D(const wchar_t *id=0, const Object3D* parent=0) : Object3D(id, parent) {}

	double findNearestPoint(const vector3d &pnt, vector3d &nearest, CoordinateSpace = ProjectSpace) const
	{
		double dist, d;
		ConstVertexRange r(static_cast<ConstVertexRange::ObjType>(this), pnt, cs);
		ConstVertexRange::Iterator i = r.begin();
		if (i == r.end()) return -1;

		if (cs == LocalSpace)
		{		
			dist = pnt.dist2(*i);

			while (++i != r.end())
			{
				d = pnt.dist2(*i);
				if (d < dist) { dist = d; nearest.set(*i); }
			}
			return sqrt(dist);
		}
		else
		{
			m_transform.setCoordinateSpace(cs);
			ConstVertexRange::Type v;
				
			m_transform.transformCS((*i), v); dist = pnt.dist2(v);

			while (++i != r.end())
			{
				m_transform.transformCS((*i), v); d = pnt.dist2(v); 
				if (d < dist) { dist = d; nearest.set(v); }
			}
			return sqrt(dist);
		}
	};
	void computeGeometryBounds(Bounds3D &bounds, CoordinateSpace cs = ProjectSpace)
	{
		bounds.clearBounds();

		ConstVertexIterator::iterator i = ConstVertexIterator::begin(this);
		if (cs == LocalSpace)
		{
			do { bounds.expandBounds(*i); }
			while (++i != ConstVertexIterator::end(this));
		}
		else 
		{
			VertexType v;
			m_transform.compileMatrix(cs);
			do 	{
				m_transform.transformCS((*i), v);
				bounds.expandBounds(v);
			}
			while (++i != ConstVertexIterator::end(this));
		}
		bounds.undirtyBounds();
	}
};
}
#endif