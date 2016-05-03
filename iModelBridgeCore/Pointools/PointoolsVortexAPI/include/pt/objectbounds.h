#ifndef POINTOOLS_OBJECT_BOUNDS_H	
#define POINTOOLS_OBJECT_BOUNDS_H

#include <pt/flags.h>

namespace pt
{
//-------------------------------------------------------------------------
/// Bounds3D encapsulates axis aligned bounds and bounds calculations
//-------------------------------------------------------------------------
/*!
 * The Bounds3D class holds axis aligned bounds information of its owner 
 * Object and can be used to compute bounds via the boundsComputer callback
 */
template<typename T> class Bounds3DBase
{
public:
    typedef std::function<void()> ComputeFunction;

	Bounds3DBase() : m_boundsComputer(), m_flags(BoundsDirty) {};
	Bounds3DBase(ComputeFunction compute) : m_boundsComputer(compute), m_flags(BoundsDirty) {};
	~Bounds3DBase(){};

	void operator = (const Bounds3DBase<T> &b) 
	{
		m_bb = b.m_bb;
		m_bs = b.m_bs;
		m_flags = b.m_flags;
	}
	enum FlagValues { BoundsDirty = 1, BoundsUpdated = 2 };

	const BBox<T> &bounds() const { if (m_flags.flag(BoundsDirty)) computeBounds(); return m_bb; }
	const BSphere<T> &sphericalBounds() const { if (m_flags.flag(BoundsDirty)) computeBounds(); return m_bs; }

	/* bounds shift */ 
	void translate(const pt::vec3<T> &v) { m_bb.translateBy(v); m_bs.center() += v; m_flags.set(BoundsUpdated); }

	/* bounds dirty and update tracking */ 
	inline void dirtyBounds() const { const_cast<Bounds3DBase<T>*>(this)->m_flags.set(BoundsDirty); }
	inline void undirtyBounds() const { const_cast<Bounds3DBase<T>*>(this)->m_flags.clear(BoundsDirty); }
	inline bool areBoundsDirty() const { return m_flags.flag(BoundsDirty); }
	inline bool areBoundsUpdated() const { return m_flags.flag(BoundsUpdated);}
	
	inline bool isValid() const { return !bounds().isEmpty(); }

	/* bounds computation */ 
	void computeBounds() const { if (m_boundsComputer) m_boundsComputer(); }
	void clearBounds() { m_bb.clear(); m_bs.clear(); }
	inline void expandBounds(const pt::vec3<T> &v) 
	{ 
		m_bb.expand(v); 
		m_bs.expandBy(v); 
	};
	void expandBounds(const BBox<T> &b) 
	{ 
		m_bb.expandBy(b); 
		m_bs.expandBy(b); 
	};

	void transformedBounds(const mmatrix4d &tr, Bounds3DBase<double> &b) const
	{
		if (m_flags.flag(BoundsDirty)) computeBounds();
		if (!isValid()) return;

		b.clearBounds(); 
		pt::vector3d v, vt;		
		for (int i=0; i<8; i++)	{ 
			m_bb.getExtrema(i,v);	
			tr.vec3_multiply_mat4(v,vt); 
			b.expandBounds(vt);	
		}
	}
	void transformedBounds(const mmatrix4d &tr, Bounds3DBase<float> &b) const
	{
		if (m_flags.flag(BoundsDirty)) computeBounds();
		if (!isValid()) return;

		b.clearBounds(); 
		pt::vector3 v, vt;		
		for (int i=0; i<8; i++)	{ 
			m_bb.getExtrema(i,v);	
			tr.vec3_multiply_mat4f(v,vt); 
			b.expandBounds(vt);	
		}
	}
	void transformBounds(const mmatrix4d &tr)
	{
		Bounds3DBase<T> b;
		transformedBounds(tr, b);
		*this = b;
		undirtyBounds();
	}
protected:
	BBox<T>			m_bb;
	BSphere<T>		m_bs;

	Flags			m_flags;
    ComputeFunction	m_boundsComputer;
};


typedef Bounds3DBase<float>		Bounds3D;
typedef Bounds3DBase<double>	Bounds3DD;

}
#endif