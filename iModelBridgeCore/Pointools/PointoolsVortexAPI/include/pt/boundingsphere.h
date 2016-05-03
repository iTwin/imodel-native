/*--------------------------------------------------------------------------*/ 
/*  BoundingSphere.h														*/ 
/*	Axis Aligned Bounding Sphere class definition							*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef COMMONCLASSESBOUNDINGSPHERE_INTERFACE
#define COMMONCLASSESBOUNDINGSPHERE_INTERFACE

#include <pt/geomtypes.h>
#include <pt/boundingbox.h>

namespace pt
{

template<typename T> class BSphere  
{
public:
	BSphere<T>()
	{
		clear();
	}
	BSphere<T>(const vec3<T>& cen, T rad)
	{
		_center = cen;
		_radius = rad;
	}
	virtual ~BSphere<T>()
	{
	}
    
	void clear() 
	{ 
		_radius = -1.0; 
	};
	void expandBy(const T *v)
	{
		const vec3<T> &v1 = *reinterpret_cast<const vec3<T>*>(v);

		if (valid())
		{
			vec3<T> dv = v1 - _center;
			T r = dv.length();
			if (r>_radius)
			{
				T dr = (r-_radius)*0.5;
				_center += dv*(dr/r);
				_radius += dr;
			}
		}
		else
		{
			_center = v;
			_radius = 0.0;
		}
	}
	void expandBy(const BBox<T>& bb)
	{
		if (bb.valid())
		{
			if (valid())
			{
				BBox<T> newbb(bb);

				for(unsigned int c=0;c<8;++c)
				{
					vec3<T> v;
					bb.getExtrema(c, v);
					v -= _center;		// get the direction vector from corner
					v.normalize();		// normalise it.
					v *= -_radius;		// move the vector in the opposite direction distance radius.
					v += _center;		// move to absolute position.
					newbb.expandBy(v);	// add it into the new bounding box.
				}

				_center = newbb.center();
				_radius = newbb.radius();

			}
			else
			{
				_center = bb.center();
				_radius = bb.radius();
			}
		}
	}
	void expandBy(const BSphere<T>& sh)
	{
		if (sh.valid())
		{
			if (valid())
			{
				vec3<T> dv = sh._center-_center;
				T dv_len = dv.length();
				if (dv_len+sh._radius>_radius)
				{
					vec3<T> e1 = _center-(dv*(_radius/dv_len));
					vec3<T> e2 = sh._center+(dv*(sh._radius/dv_len));
					_center = (e1+e2)*0.5;
					_radius = (e2-_center).length();
				}                   
			}
			else
			{
				_center = sh._center;
				_radius = sh._radius;
			}
		}
	}
	void expandRadiusBy(const T *v)
	{
		const vec3<T> &v1 = *reinterpret_cast<const vec3<T>*>(v);

		if (valid())
		{
			T r = (v1-_center).length();
			if (r>_radius) _radius = r;
		}
		else
		{
			_center = v1;
			_radius = 0.0;
		}
	}
	void expandRadiusBy(const BBox<T>& bb)
	{
		if (bb.valid())
		{
			if (valid())
			{
				for(unsigned int c=0;c<8;++c)
				{
					vec3<T> corner;
					bb.getExtrema(c, corner);
					expandRadiusBy(corner);
				}
			}
			else
			{
				_center = bb.center();
				_radius = bb.radius();
			}
		}	
	}
	void expandRadiusBy(const BSphere<T>& sh)
	{
		if (sh.valid())
		{
			if (valid())
			{
				T r = (sh._center-_center).length()+sh._radius;
				if (r>_radius) _radius = r;
			}
			else
			{
				_center = sh._center;
				_radius = sh._radius;
			}
		}
	}

	inline bool valid() const { return _radius>=0.0; }	

	inline const T &radius() const { return _radius; };
	inline T &radius() { return _radius; };
    inline T radius2() const { return _radius*_radius; }

	inline const vec3<T> &center() const { return _center; };
	inline vec3<T> &center() { return _center; };

	inline void operator = (const BSphere<T>& b)
	{
		_radius = b._radius;
		_center = b._center;
	}
    inline bool contains(const vec3<T>& v) const
    {
        return valid() && ((v-_center).length2()<=radius2());
    }
    inline bool intersects( const BSphere<T>& bs ) const
    {
        return valid() && bs.valid() &&
               ((_center - bs._center).length2() <= (_radius + bs._radius)*(_radius + bs._radius));
    }
private:
	vec3<T>	_center;
	T		_radius;
};

// typdefs to allow the old definition of BoundingSphere to work along side the new double version
typedef BSphere<float>	BoundingSphere;
typedef BSphere<double>	BoundingSphereD;

}
#endif
