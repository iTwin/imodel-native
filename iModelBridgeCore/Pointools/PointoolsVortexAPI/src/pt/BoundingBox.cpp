/*--------------------------------------------------------------------------*/ 
/*  BoundingBox.cpp															*/ 
/*	Axis Aligned Bounding Box class implentation							*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include <pt/ptmath.h>
#include <pt/BoundingBox.h>
//#include <MGC/MgcIntr3DLinBox.h>

using namespace pt;

// generates compile error
NOTE: this file should not be used / built

/*--------------------------------------------------------------------------*/ 
/*	Constructor																*/ 
/*--------------------------------------------------------------------------*/ 
BoundingBox::BoundingBox()
{
	_empty = true; 
	_av.zero();
	_valid = false;

	for (int i=0; i<3; i++) 
	{
		upper_bounds[i] = 0;
		lower_bounds[i] = 0;
	}
}
/*--------------------------------------------------------------------------*/ 
/*	Copy Constructor														*/ 
/*--------------------------------------------------------------------------*/ 
BoundingBox::BoundingBox(const BoundingBox &box)
{
	for (int i=0; i<3; i++) 
	{
		upper_bounds[i] = box.upper_bounds[i];
		lower_bounds[i] = box.lower_bounds[i];
	}
	_av = box._av;
	_empty = false;
	_valid = box._valid;
}
/*--------------------------------------------------------------------------*/ 
/*	Value Constructor														*/ 
/*--------------------------------------------------------------------------*/ 
BoundingBox::BoundingBox(float uxv, float lxv, float uyv, float lyv, float uzv, float lzv)
{
	_valid = true;
	setBox(uxv,lxv,uyv,lyv,uzv,lzv);
}
/*--------------------------------------------------------------------------*/ 
/*	Destructor																*/ 
/*--------------------------------------------------------------------------*/ 
BoundingBox::~BoundingBox()
{}
/*--------------------------------------------------------------------------*/ 
/*	setBox axis planes														*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::setBox(float uxv, float lxv, float uyv, float lyv, float uzv, float lzv)
{
	clear();
	lower_bounds[0] = lxv;
	lower_bounds[1] = lyv;
	lower_bounds[2] = lzv;
	upper_bounds[0] = uxv;
	upper_bounds[1] = uyv;
	upper_bounds[2] = uzv;
	_empty = false;
}
/*--------------------------------------------------------------------------*/ 
/*	setBox axis planes														*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::setBox(const vector3 &lowerv, const vector3 &upperv)
{
	clear();
	lower_bounds[0] = lowerv.x;
	lower_bounds[1] = lowerv.y;
	lower_bounds[2] = lowerv.z;
	upper_bounds[0] = upperv.x;
	upper_bounds[1] = upperv.y;
	upper_bounds[2] = upperv.z;
	_empty = false;
}
/*--------------------------------------------------------------------------*/ 
/*	expand box to include point, call clear() to expand from nothing		*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::expandBy(const double *v)
{
	float vf[] = { (float)v[0], (float)v[1], (float)v[2] };
	expand(vf);
	_empty = false;
}
/*--------------------------------------------------------------------------*/ 
/*	expand box to include bbox, call clear() to expand from nothing			*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::expandBy(const BoundingBox *bb)
{
	if (bb->isEmpty()) return;
	vector3 v;
	for (int i=0; i<7; i++)
	{
		bb->getExtrema(i, v);
		expandBy(v);
	}
	_empty = false;
}
/*--------------------------------------------------------------------------*/ 
/*	expand box by specified coefficient of current dimensions
/*--------------------------------------------------------------------------*/ 
void BoundingBox::expandByCoef(float coef)
{
	unsigned int t;

	float u_bounds[3];
	float l_bounds[3];

	for(t = 0; t < 3; t++)
	{
		float offset = size(t) * 0.5 * coef;

		u_bounds[t] = upper_bounds[t] + offset;
		l_bounds[t] = lower_bounds[t] - offset;
	}

	setBox(u_bounds[0], l_bounds[0], u_bounds[1], l_bounds[1], u_bounds[2], l_bounds[2]);

}
/*--------------------------------------------------------------------------*/ 
/*	expand box by specified size on each axis
/*--------------------------------------------------------------------------*/ 
void BoundingBox::expandByOffset(float dx, float dy, float dz)
{
	setBox(upper(0) + dx, lower(0) - dx, upper(1) + dy, lower(1) - dy, upper(2) + dz, lower(2) - dz);
}

/*--------------------------------------------------------------------------*/ 
/*	Assignment																*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::operator = (const BoundingBox& box)
{
	if (&box == this) return;

	for (int i=0; i<3; i++) 
	{
		upper_bounds[i] = box.upper_bounds[i];
		lower_bounds[i] = box.lower_bounds[i];
	}	
	_empty = box._empty;
	_av = box._av;
	_valid = box._valid;
	memcpy( _usebound, box._usebound, sizeof(_usebound));
}
/*--------------------------------------------------------------------------*/ 
/*	Assignment																*/ 
/*--------------------------------------------------------------------------*/ 
void BoundingBox::operator = (const BoundingBox *box)
{
	if (box == this) return;

	for (int i=0; i<3; i++) 
	{
		upper_bounds[i] = box->upper_bounds[i];
		lower_bounds[i] = box->lower_bounds[i];
	}
	_empty = box->_empty;
	_av = box->_av;
	_valid = box->_valid;
	memcpy( _usebound, box->_usebound, sizeof(_usebound));
}
/*--------------------------------------------------------------------------*/ 
/*	Assignment																*/ 
/*--------------------------------------------------------------------------*/ 
bool BoundingBox::operator == (const BoundingBox &b)
{
	if (this == &b) return true;

	for (int i=0; i<3; i++) 
	{
		if (fabs(upper_bounds[i] - b.upper_bounds[i]) > 0.00001f) return false;
		if (fabs(lower_bounds[i] - b.lower_bounds[i]) > 0.00001f) return false;
	}
	return true;
}
bool BoundingBox::operator != (const BoundingBox &b)
{
	return !(*this == b);
}

#ifdef _DEBUG
float BoundingBox::radius() const { return sqrt(radius2()); }
#endif

//
// Box Ray intersection
//
/*
bool BoundingBox::intersectsRay(const Ray<float> &ray) const
{
	vector3 bext(upper_bounds[0]-lower_bounds[0],
		upper_bounds[1]-lower_bounds[1],
		upper_bounds[2]-lower_bounds[2]);

	bext *= 0.5f;

	vector3 diff, cen(center());

	diff.x = ray.origin.x - cen.x;
	if(fabsf(diff.x)>bext.x && diff.x*ray.direction.x>=0.0f)	return false;

	diff.y = ray.origin.y - cen.y;
	if(fabsf(diff.y)>bext.y && diff.y*ray.direction.y>=0.0f)	return false;

	diff.z = ray.origin.z - cen.z;
	if(fabsf(diff.z)>bext.z && diff.z*ray.direction.z>=0.0f)	return false;

	float fAWdU[3];
	fAWdU[0] = fabsf(ray.direction.x);
	fAWdU[1] = fabsf(ray.direction.y);
	fAWdU[2] = fabsf(ray.direction.z);

	float f;
	f = ray.direction.y * diff.z - ray.direction.z * diff.y;	if(fabsf(f)>bext.y*fAWdU[2] + bext.z*fAWdU[1])	return false;
	f = ray.direction.z * diff.x - ray.direction.x * diff.z;	if(fabsf(f)>bext.x*fAWdU[2] + bext.z*fAWdU[0])	return false;
	f = ray.direction.x * diff.y - ray.direction.y * diff.x;	if(fabsf(f)>bext.x*fAWdU[1] + bext.y*fAWdU[0])	return false;

	return true;
}
*/

//
// Box Segment intersection
//
bool BoundingBox::intersectsSegment(const Segment<float> &segment) const
{
	vector3 bext(upper_bounds[0]-lower_bounds[0],
		upper_bounds[1]-lower_bounds[1],
		upper_bounds[2]-lower_bounds[2]);

	bext *= 0.5f;

	vector3 diff, cen(center()), dir;

	float fAWdU[3];

	dir.x = 0.5f * (segment.b.x - segment.a.x);
	diff.x = (0.5f * (segment.b.x + segment.a.x)) - cen.x;
	fAWdU[0] = fabsf(dir.x);
	if(fabsf(diff.x)>bext.x + fAWdU[0])	return false;

	dir.y = 0.5f * (segment.b.y - segment.a.y);
	diff.y = (0.5f * (segment.b.y + segment.a.y)) - cen.y;
	fAWdU[1] = fabsf(dir.y);
	if(fabsf(diff.y)>bext.y + fAWdU[1])	return false;

	dir.z = 0.5f * (segment.b.z - segment.a.z);
	diff.z = (0.5f * (segment.b.z + segment.a.z)) - cen.z;
	fAWdU[2] = fabsf(dir.z);
	if(fabsf(diff.z)>bext.z + fAWdU[2])	return false;

	float f;
	f = dir.y * diff.z - dir.z * diff.y;	if(fabsf(f)>bext.y*fAWdU[2] + bext.z*fAWdU[1])	return false;
	f = dir.z * diff.x - dir.x * diff.z;	if(fabsf(f)>bext.x*fAWdU[2] + bext.z*fAWdU[0])	return false;
	f = dir.x * diff.y - dir.y * diff.x;	if(fabsf(f)>bext.x*fAWdU[1] + bext.y*fAWdU[0])	return false;

	return true;
}
#ifndef POINTOOLS_POD_API
//
//
//
void BoundingBox::draw() const
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
//
//
//
void BoundingBox::drawSolid() const
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

// Calculate the maximum distance between a point in this bounding box and the given bounding box.
// Note: Both are Axis Aligned Bounding Boxes (AABB).
// Note: The result is valid for cases where boxes are separate, intersecting or containing

float BoundingBox::maxDistanceSquared(const BoundingBox &b)
{
	float dcx, dcy, dcz;
	float xMax, yMax, zMax;
															// Calculate axial distances between box centers												
	dcx = fabs(((upper(0) + lower(0)) - (b.upper(0) + b.lower(0))) * 0.5);
	dcy = fabs(((upper(1) + lower(1)) - (b.upper(1) + b.lower(1))) * 0.5);
	dcz = fabs(((upper(2) + lower(2)) - (b.upper(2) + b.lower(2))) * 0.5);
															// Calculate axial spans between each box's opposite ends
	xMax = dcx + ((upper(0) - lower(0)) + (b.upper(0) - b.lower(0))) * 0.5;
	yMax = dcy + ((upper(1) - lower(1)) + (b.upper(1) - b.lower(1))) * 0.5;
	zMax = dcz + ((upper(2) - lower(2)) + (b.upper(2) - b.lower(2))) * 0.5;

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

float BoundingBox::minDistanceSquared(const BoundingBox &b)
{
	float	result = 0;

	float	aMin, aMax;
	float	bMin, bMax;
	float	delta;
															// Calculate squared delta for X axis if ranges non overlapping
	if(lower(0) > b.upper(0))
	{
		delta	= lower(0) - b.upper(0);
		result += (delta * delta);
	}
	else
	if(b.lower(0) > upper(0))
	{
		delta	= upper(0) - b.lower(0);
		result	+= (delta * delta);
	}

															// Calculate squared delta for Y axis if ranges non overlapping
	if(lower(1) > b.upper(1))
	{
		delta	= lower(1) - b.upper(1);
		result += (delta * delta);
	}
	else
	if(b.lower(1) > upper(1))
	{
		delta	= upper(1) - b.lower(1);
		result	+= (delta * delta);
	}

															// Calculate squared delta for Z axis if ranges non overlapping
	if(lower(2) > b.upper(2))
	{
		delta	= lower(2) - b.upper(2);
		result += (delta * delta);
	}
	else
	if(b.lower(2) > upper(2))
	{
		delta	= upper(2) - b.lower(2);
		result	+= (delta * delta);
	}

															// Return sum of squares of contributing axes
	return result;
}





#endif