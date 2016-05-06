/*--------------------------------------------------------------------------*/ 
/*  BBox.h															*/ 
/*	Axis Aligned Bounding Box class definition								*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef COMMONCLASSESBOUNDINGBOX_INTERFACE
#define COMMONCLASSESBOUNDINGBOX_INTERFACE

#ifndef _DEBUG
#include <pt/ptmath.h>
#endif

#include <pt/classes.h>
#include <pt/Bounds.h>
#include <pt/geomtypes.h>

namespace pt
{
template<typename T> class BBox : public Bounds<3, T>
{
public:
    typedef Bounds<3, T> Base;
	BBox()
	{
		clear();
		for (int i=0; i<3; i++) 
		{
        Base::upper_bounds[i] = 0;
        Base::lower_bounds[i] = 0;
		}
	}

	template <typename F>
	BBox(const BBox<F> &box)
	{
		for (int i=0; i<3; i++) 
		{
        Base::upper_bounds[i] = (T)box.upper(i);
        Base::lower_bounds[i] = (T)box.lower(i);
		}
		_av.set( box.weightedCenter() );
        Base::_empty = false;
	}

	BBox(T ux, T lx, T uy, T ly, T uz, T lz)
	{
		setBox(ux,lx,uy,ly,uz,lz);
	}
	virtual ~BBox()
	{
	}

	inline void setBox(T ux, T lx, T uy, T ly, T uz, T lz)
	{
		clear();
        Base::lower_bounds[0] = lx;
        Base::lower_bounds[1] = ly;
        Base::lower_bounds[2] = lz;
        Base::upper_bounds[0] = ux;
        Base::upper_bounds[1] = uy;
        Base::upper_bounds[2] = uz;
        Base::_empty = false;
	}
	inline void setBox(const vec3<T> &lower, const vec3<T> &upper)
	{
		clear();
        Base::lower_bounds[0] = lower.x;
        Base::lower_bounds[1] = lower.y;
        Base::lower_bounds[2] = lower.z;
        Base::upper_bounds[0] = upper.x;
        Base::upper_bounds[1] = upper.y;
        Base::upper_bounds[2] = upper.z;
        Base::_empty = false;
	}
	inline void clear() { Base::_empty = true; _av.zero(); }
	
	/*expansion*/ 
	inline void expandBy(const BBox<T>& bb)
	{
		if (bb.isEmpty()) return;
		vec3<T> v;
		for (int i=0; i<7; i++)
		{
			bb.getExtrema(i, v);
			expandBy(v);
		}
        Base::_empty = false;
	}
	inline void expandBy(const vec3<T> &v) { expand(v); _av += v; _av /= static_cast<T>(2.0); }
	inline void expandBy(const double *v)
	{
		T vf[] = { (T)v[0], (T)v[1], (T)v[2] };
		expand(vf);
        Base::_empty = false;
	}

	void expandByCoef(T coef)
	{
		unsigned int t;

		T u_bounds[3];
		T l_bounds[3];

		for(t = 0; t < 3; t++)
		{
			float offset = Base::size(t) * 0.5 * coef;

			u_bounds[t] = Base::upper_bounds[t] + offset;
			l_bounds[t] = Base::lower_bounds[t] - offset;
		}

		setBox(u_bounds[0], l_bounds[0], u_bounds[1], l_bounds[1], u_bounds[2], l_bounds[2]);

	}

	void expandByOffset(T dx, T dy, T dz)
	{
		setBox(Base::upper(0) + dx, Base::lower(0) - dx, Base::upper(1) + dy, Base::lower(1) - dy, Base::upper(2) + dz, Base::lower(2) - dz);
	}

	
	/*accessors*/ 
	inline T dx() const		{ return Base::size(0); }
	inline T dy() const		{ return Base::size(1); }
	inline T dz() const		{ return Base::size(2); }
	inline const T &lx() const	{ return Base::lower_bounds[0]; }
	inline const T &ly() const	{ return Base::lower_bounds[1]; }
	inline const T &lz() const	{ return Base::lower_bounds[2]; }
	inline const T &ux() const	{ return Base::upper_bounds[0]; }
	inline const T &uy() const	{ return Base::upper_bounds[1]; }
	inline const T &uz() const	{ return Base::upper_bounds[2]; }

	inline T &lx() { return Base::lower_bounds[0]; }
	inline T &ly() { return Base::lower_bounds[1]; }
	inline T &lz() { return Base::lower_bounds[2]; }
	inline T &ux() { return Base::upper_bounds[0]; }
	inline T &uy() { return Base::upper_bounds[1]; }
	inline T &uz() { return Base::upper_bounds[2]; }

	inline const vec3<T> &weightedCenter() const { return _av; }
	inline vec3<T> center() const { return diagonal() / 2.0 + vec3<T>(Base::lower_bounds); }
	inline vec3<T> diagonal() const { return vec3<T>(Base::size(0), Base::size(1), Base::size(2)); }

	inline T maxDimensionSize(void)
	{
		return std::max(dx(), std::max(dy(), dz()));
	}

	inline BBox<T>& operator = (const BBox<T>& box)
	{
		if (this != &box)
		{
			for (int i=0; i<3; i++) 
			{
            Base::upper_bounds[i] = box.upper_bounds[i];
            Base::lower_bounds[i] = box.lower_bounds[i];
			}	
            Base::_empty = box._empty;
			_av = box._av;
			_valid = box._valid;
			memcpy(Base::_usebound, box._usebound, sizeof(Base::_usebound));
		}
		return *this;
	}
	
	inline bool operator == (const BBox<T> &b)
	{
		if (this == &b) return true;

		for (int i=0; i<3; i++) 
		{
			if (fabs(Base::upper_bounds[i] - b.upper_bounds[i]) > 0.00001) return false;
			if (fabs(Base::lower_bounds[i] - b.lower_bounds[i]) > 0.00001) return false;
		}
		return true;
	}
	inline bool operator != (const BBox<T> &b)
	{
		return !(*this == b);
	}

#if defined(HAVE_OPENGL)
	void draw() const
	{
		float l;

		if (dy() <= dx())
		{
			if (dy() <= dz()) l = dy();
			else if (dz() < dx()) l = dz();
			else l = dx();
		}
		else if (dz() < dx()) l = dz();
		else l = dx();

		l /= 10.0f;

		glBegin(GL_LINES);
		glVertex3f(lx(),	ly(),		lz());
		glVertex3f(lx()+l, ly(),		lz());
		glVertex3f(lx(),	ly(),		lz());
		glVertex3f(lx(),	ly()+l,	lz());
		glVertex3f(lx(),	ly(),		lz());
		glVertex3f(lx(),	ly(),		lz()+l);

		glVertex3f(lx(),	ly(),		uz());
		glVertex3f(lx()+l, ly(),		uz());
		glVertex3f(lx(),	ly(),		uz());
		glVertex3f(lx(),	ly()+l,	uz());
		glVertex3f(lx(),	ly(),		uz());
		glVertex3f(lx(),	ly(),		uz()-l);

		glVertex3f(lx(),	uy(),		uz());
		glVertex3f(lx()+l, uy(),		uz());
		glVertex3f(lx(),	uy(),		uz());
		glVertex3f(lx(),	uy()-l,	uz());
		glVertex3f(lx(),	uy(),		uz());
		glVertex3f(lx(),	uy(),		uz()-l);

		glVertex3f(ux(),	uy(),		uz());
		glVertex3f(ux()-l, uy(),		uz());
		glVertex3f(ux(),	uy(),		uz());
		glVertex3f(ux(),	uy()-l,	uz());
		glVertex3f(ux(),	uy(),		uz());
		glVertex3f(ux(),	uy(),		uz()-l);

		glVertex3f(ux(),	uy(),		lz());
		glVertex3f(ux()-l, uy(),		lz());
		glVertex3f(ux(),	uy(),		lz());
		glVertex3f(ux(),	uy()-l,	lz());
		glVertex3f(ux(),	uy(),		lz());
		glVertex3f(ux(),	uy(),		lz()+l);

		glVertex3f(lx(),	uy(),		lz());
		glVertex3f(lx()+l, uy(),		lz());
		glVertex3f(lx(),	uy(),		lz());
		glVertex3f(lx(),	uy()-l,	lz());
		glVertex3f(lx(),	uy(),		lz());
		glVertex3f(lx(),	uy(),		lz()+l);

		glVertex3f(ux(),	ly(),		uz());
		glVertex3f(ux()-l, ly(),		uz());
		glVertex3f(ux(),	ly(),		uz());
		glVertex3f(ux(),	ly()+l,	uz());
		glVertex3f(ux(),	ly(),		uz());
		glVertex3f(ux(),	ly(),		uz()-l);

		glVertex3f(ux(),	ly(),		lz());
		glVertex3f(ux()-l, ly(),		lz());
		glVertex3f(ux(),	ly(),		lz());
		glVertex3f(ux(),	ly()+l,	lz());
		glVertex3f(ux(),	ly(),		lz());
		glVertex3f(ux(),	ly(),		lz()+l);	
		glEnd();
	}
	inline void drawSolid() const
	{
		glFrontFace(GL_CW);
		/*CCW*/ 
		glBegin(GL_QUADS); 
		//front
		glNormal3f(0,0,-1.0f);
		glVertex3f(lx(), uy(), lz());
		glVertex3f(lx(), ly(), lz());
		glVertex3f(ux(), ly(), lz());
		glVertex3f(ux(), uy(), lz());

		//back
		glNormal3f(0,0,1.0f);
		glVertex3f(ux(), uy(), uz());	
		glVertex3f(ux(), ly(), uz());
		glVertex3f(lx(), ly(), uz());
		glVertex3f(lx(), uy(), uz());

		// "right"
		glNormal3f(1.0f,0,0);
		glVertex3f(ux(), uy(), lz());
		glVertex3f(ux(), ly(), lz());
		glVertex3f(ux(), ly(), uz());
		glVertex3f(ux(), uy(), uz());

		// "_left"
		glNormal3f(-1.0f,0,0);
		glVertex3f(lx(), uy(), uz());
		glVertex3f(lx(), ly(), uz());
		glVertex3f(lx(), ly(), lz());
		glVertex3f(lx(), uy(), lz());


		// "top"
		glNormal3f(0,1.0f,0);
		glVertex3f(lx(), uy(), uz());
		glVertex3f(lx(), uy(), lz());
		glVertex3f(ux(), uy(), lz());
		glVertex3f(ux(), uy(), uz());

		// "bottom"
		glNormal3f(0,-1.0f,0);
		glVertex3f(ux(), ly(), uz());
		glVertex3f(ux(), ly(), lz());
		glVertex3f(lx(), ly(), lz());
		glVertex3f(lx(), ly(), uz());
		glEnd();
		glFrontFace(GL_CCW);
	}
#endif

	template<class Real> bool intersectsRay(const Ray<Real> &ray) const
	{
		vec3<T> bext(Base::upper_bounds[0]- Base::lower_bounds[0],
                     Base::upper_bounds[1]- Base::lower_bounds[1],
                     Base::upper_bounds[2]- Base::lower_bounds[2]);

		bext *= 0.5f;

		vec3<Real> diff, cen(center());

		diff.x = ray.origin.x - cen.x;
		if(fabsf(static_cast<float>(diff.x))>bext.x && diff.x*ray.direction.x>=0.0f)	return false;

		diff.y = ray.origin.y - cen.y;
		if(fabsf(static_cast<float>(diff.y))>bext.y && diff.y*ray.direction.y>=0.0f)	return false;

		diff.z = ray.origin.z - cen.z;
		if(fabsf(static_cast<float>(diff.z))>bext.z && diff.z*ray.direction.z>=0.0f)	return false;

		Real fAWdU[3];

		fAWdU[0] = fabs(ray.direction.x);
		fAWdU[1] = fabs(ray.direction.y);
		fAWdU[2] = fabs(ray.direction.z);

		Real f;

		f = ray.direction.y * diff.z - ray.direction.z * diff.y;	if(fabsf(static_cast<float>(f))>bext.y*fAWdU[2] + bext.z*fAWdU[1])	return false;
		f = ray.direction.z * diff.x - ray.direction.x * diff.z;	if(fabsf(static_cast<float>(f))>bext.x*fAWdU[2] + bext.z*fAWdU[0])	return false;
		f = ray.direction.x * diff.y - ray.direction.y * diff.x;	if(fabsf(static_cast<float>(f))>bext.x*fAWdU[1] + bext.y*fAWdU[0])	return false;

		return true;
	}


	bool intersectsSegment(const Segment<T> &segment) const
	{
		vec3<T> bext(Base::upper_bounds[0]- Base::lower_bounds[0],
                     Base::upper_bounds[1]- Base::lower_bounds[1],
                     Base::upper_bounds[2]- Base::lower_bounds[2]);

		bext *= 0.5f;

		vec3<T> diff, cen(center()), dir;

		float fAWdU[3];

		dir.x = 0.5f * (segment.b.x - segment.a.x);
		diff.x = (0.5f * (segment.b.x + segment.a.x)) - cen.x;
		fAWdU[0] = fabsf(static_cast<float>(dir.x));
		if(fabsf(static_cast<float>(diff.x))>bext.x + fAWdU[0])	return false;

		dir.y = 0.5f * (segment.b.y - segment.a.y);
		diff.y = (0.5f * (segment.b.y + segment.a.y)) - cen.y;
		fAWdU[1] = fabsf(static_cast<float>(dir.y));
		if(fabsf(static_cast<float>(diff.y))>bext.y + fAWdU[1])	return false;

		dir.z = 0.5f * (segment.b.z - segment.a.z);
		diff.z = (0.5f * (segment.b.z + segment.a.z)) - cen.z;
		fAWdU[2] = fabsf(static_cast<float>(dir.z));
		if(fabsf(static_cast<float>(diff.z))>bext.z + fAWdU[2])	return false;

		T f;
		f = dir.y * diff.z - dir.z * diff.y;	if(fabsf(static_cast<float>(f))>bext.y*fAWdU[2] + bext.z*fAWdU[1])	return false;
		f = dir.z * diff.x - dir.x * diff.z;	if(fabsf(static_cast<float>(f))>bext.x*fAWdU[2] + bext.z*fAWdU[0])	return false;
		f = dir.x * diff.y - dir.y * diff.x;	if(fabsf(static_cast<float>(f))>bext.x*fAWdU[1] + bext.y*fAWdU[0])	return false;

		return true;
	}

	inline T volume() const { return dx() * dy() * dz(); }

	
	
	// Calculate the maximum distance between a point in this bounding box and the given bounding box.
	// Note: Both are Axis Aligned Bounding Boxes (AABB).
	// Note: The result is valid for cases where boxes are separate, intersecting or containing

	T maxDistanceSquared(const BBox<T> &b)
	{
		T dcx, dcy, dcz;
		T xMax, yMax, zMax;
																// Calculate axial distances between box centers												
		dcx = fabs(((Base::upper(0) + Base::lower(0)) - (b.upper(0) + b.lower(0))) * 0.5);
		dcy = fabs(((Base::upper(1) + Base::lower(1)) - (b.upper(1) + b.lower(1))) * 0.5);
		dcz = fabs(((Base::upper(2) + Base::lower(2)) - (b.upper(2) + b.lower(2))) * 0.5);
																// Calculate axial spans between each box's opposite ends
		xMax = dcx + ((Base::upper(0) - Base::lower(0)) + (b.upper(0) - b.lower(0))) * 0.5;
		yMax = dcy + ((Base::upper(1) - Base::lower(1)) + (b.upper(1) - b.lower(1))) * 0.5;
		zMax = dcz + ((Base::upper(2) - Base::lower(2)) + (b.upper(2) - b.lower(2))) * 0.5;

																// Return the square of the diagonal distance
		return xMax*xMax + yMax*yMax + zMax*zMax;
	}

	// Calculates the minimum distance between this axis aligned bounding box (AABB) and the given AABB
	// Note: If one boxes are intersecting or containing, the distance returned is zero.
	// Note: An axis' delta only contributes to the measurement if the two ranges on the axis do not overlap
	// 0 overlaps is equivalent to a closest vertex-vertex distance
	// 1 overlaps is equivalent to a closest edge-edge distance
	// 2 overlaps is equivalent to a face-face distance
	// 3 overlaps is containment (zero)

	T minDistanceSquared(const BBox<T> &b)
	{
		T	result = 0;

		T	delta;
																// Calculate squared delta for X axis if ranges non overlapping
		if(Base::lower(0) > b.upper(0))
		{
			delta	= Base::lower(0) - b.upper(0);
			result += (delta * delta);
		}
		else
		if(b.lower(0) > Base::upper(0))
		{
			delta	= Base::upper(0) - b.lower(0);
			result	+= (delta * delta);
		}

																// Calculate squared delta for Y axis if ranges non overlapping
		if(Base::lower(1) > b.upper(1))
		{
			delta	= Base::lower(1) - b.upper(1);
			result += (delta * delta);
		}
		else
		if(b.lower(1) > Base::upper(1))
		{
			delta	= Base::upper(1) - b.lower(1);
			result	+= (delta * delta);
		}

																// Calculate squared delta for Z axis if ranges non overlapping
		if(Base::lower(2) > b.upper(2))
		{
			delta	= Base::lower(2) - b.upper(2);
			result += (delta * delta);
		}
		else
		if(b.lower(2) > Base::upper(2))
		{
			delta	= Base::upper(2) - b.lower(2);
			result	+= (delta * delta);
		}

																// Return sum of squares of contributing axes
		return result;
	}	



	inline T radius() const { return sqrt(radius2()); }
    inline T radius2() const
    {
       return static_cast<T>(0.25*(diagonal().length2()));
    }
private:
	vec3<T>	_av;
	bool	_valid;
};

// typdefs to enable the original float definition of BoundingBox to work along side a new double version
typedef BBox<float>		BoundingBox;
typedef BBox<double>	BoundingBoxD;

}
#endif 
