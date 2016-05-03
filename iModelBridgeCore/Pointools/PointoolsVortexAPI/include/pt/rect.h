/*--------------------------------------------------------------------------*/
/*  BoundingBox.h															*/
/*	Axis Aligned Bounding Rect class definition								*/
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/
/*																			*/
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/
/*--------------------------------------------------------------------------*/

#ifndef COMMONCLASSESRECT_INTERFACE
#define COMMONCLASSESRECT_INTERFACE

#include <pt/bounds.h>
#include <pt/geomtypes.h>

namespace pt
{
template <class REAL>
class Rect : public Bounds<2, REAL>
{
public:
	typedef Rect<REAL> RT;

	Rect(){};
	Rect(REAL x, REAL y, REAL x2, REAL y2)
	{
		lower_bounds[0] = x;
		lower_bounds[1] = y;
		upper_bounds[0] = x2;
		upper_bounds[1] = y2;
	};

	~Rect(){};

	void set(REAL x, REAL y, REAL x2, REAL y2)
	{
		lower_bounds[0] = x;
		lower_bounds[1] = y;
		upper_bounds[0] = x2;
		upper_bounds[1] = y2;
	};

	/*accessors*/
	inline REAL dx() const		{ return size(0); }
	inline REAL dy() const		{ return size(1); }
	inline void width(REAL w)	{ upper_bounds[0] = lower_bounds[0] + w; }
	inline void height(REAL h)	{ upper_bounds[1] = lower_bounds[1] + h; }

	inline const REAL &lx() const	{ return lower_bounds[0]; }
	inline const REAL &ly() const	{ return lower_bounds[1]; }
	inline const REAL &ux() const	{ return upper_bounds[0]; }
	inline const REAL &uy() const	{ return upper_bounds[1]; }
	inline  REAL &lx() 	{ return lower_bounds[0]; }
	inline  REAL &ly() 	{ return lower_bounds[1]; }
	inline  REAL &ux() 	{ return upper_bounds[0]; }
	inline  REAL &uy() 	{ return upper_bounds[1]; }

	void operator += (const vec2<REAL> &t)
	{
		lx() += t.x;
		ly() += t.y;
		ux() += t.x;
		uy() += t.y;
	};
	void operator -= (const vec2<REAL> &t)
	{
		lx() -= t.x;
		ly() -= t.y;
		ux() -= t.x;
		uy() -= t.y;
	};
	void operator += (const RT &box)
	{
		if (box.lx() < lx()) lx() = box.lx();
		if (box.ly() < ly()) ly() = box.ly();
		if (box.ux() > ux()) ux() = box.ux();
		if (box.uy() > uy()) uy() = box.uy();
	};
	RT& operator += (const REAL *p)
	{
		expand(p);
		return (*this);
	};
	template <class T>
	void operator *= (T s)
	{
		for (int i=0;i<2;i++)
		{
			lower_bounds[i] *= s;
			upper_bounds[i] *= s;
		}
	};
	template <class T>
	void operator /= (T s)
	{
		for (int i=0;i<2;i++)
		{
			lower_bounds[i] /= s;
			upper_bounds[i] /= s;
		}
	};
	inline REAL midX() const { return mid(0); };
	inline REAL midY() const { return mid(1); };

	bool intersection(const RT &rect)
	{
		if (intersects(&rect)) return false;

		if (rect.lx() > lx()) lx() = rect.lx();
		if (rect.ly() > ly()) ly() = rect.ly();

		if (rect.ux() < ux()) ux() = rect.ux();
		if (rect.uy() < uy()) uy() = rect.uy();

		return true;
	}
	RT& operator = (const RT &rect)
	{
		memcpy(this, &rect, sizeof(RT));
		return (*this);
	};
	inline bool operator == (const RT &rect) const
	{
		return (lx() == rect.lx() 
			&& ly() == rect.ly() 
			&& ux() == rect.ux()
			&& uy() == rect.uy());
	};
	inline bool operator != (const RT &rect) const
	{
		return (lx() != rect.lx() 
			|| ly() != rect.ly() 
			|| ux() != rect.ux()
			|| uy() != rect.uy());
	};
	inline REAL area() const { return dx() * dy(); }
	template <class T>
	Rect<T> cast(const T &v) const
	{
		Rect<T> r;
		r.lx() = (T)lx();
		r.ly() = (T)ly();
		r.ux() = (T)ux();
		r.uy() = (T)uy();
		return r;
	}
	inline void inflate(REAL amount)
	{
		lx() -= amount;
		ly() -= amount;
		ux() += amount;
		uy() += amount;
	}
	inline void inflate(REAL ax, REAL ay)
	{
		lx() -= ax;
		ly() -= ay;
		ux() += ax;
		uy() += ay;
	}
	inline void deflate(REAL amount)
	{
		inflate(-amount);
	}
	inline void deflate(REAL ax, REAL ay)
	{
		inflate(-ax, -ay);
	}
};

typedef Rect<double>	Rectd;
typedef Rect<float>		Rectf;
typedef Rect<int>		Recti;

}
#endif