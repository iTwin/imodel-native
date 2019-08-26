/*--------------------------------------------------------------------------*/ 
/*	Pointools Arcball class definition and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_ARCBALL_INTERFACE
#define POINTOOLS_ARCBALL_INTERFACE

#include "3dmath.h"
#include "ptgl.h"
#include <stdlib.h>

namespace pt { namespace datatree { class Branch; }}

namespace ptgl
{

class PTGL_API Point
{
public:
	Point() {};
	Point(int _x, int _y)
	{
		x = _x; y = _y;
	}
	int x;
	int y;

	inline void operator -= (const Point& v)	{
			x -= v.x;
			y -= v.y;
		}
	inline void operator += (const Point& v)	{
			x += v.x;
			y += v.y;
		}
};

// Auxiliary Type Definitions
enum AxisSet
{
	NO_AXES  = 0,
	CAMERA_AXES = 1,
	BODY_AXES = 2,
	OTHER_AXES = 3
};

class PTGL_API Arcball  
{
private:
	bool bDrawConstraints;
	gl_tmatrix bodyorientation;
	int angleKeyIncrement;
	void DrawConstraints();
	gl_vector* GetUsedAxisSet();

	bool bProjectionMethod2;
	bool bDrawBallArea;
	int GLdisplayList;
	gl_unitquaternion currentQuat;
	gl_unitquaternion previousQuat;
	real radius;
	real winWidth;
	real winHeight;
	real xprev;
	real yprev;
	Point center;
	bool mouseButtonDown;
	AxisSet whichConstraints;
	int currentAxisIndex;
	gl_vector cameraAxes[3];
	gl_vector bodyAxes[3];
	gl_vector* otherAxes;
	int otherAxesNum;

	void initVars(void);
	void ProjectOnSphere(gl_vector& v) const;
	gl_unitquaternion RotationFromMove(const gl_vector& vfrom,const gl_vector& vto);
	gl_vector ConstrainToAxis(const gl_vector& loose,const gl_vector& axis);
	int NearestConstraintAxis(const gl_vector& loose);
public:

	int GetAngleKeyIncrement();
	void SetAngleKeyIncrement(int ang);
	void UseConstraints(AxisSet constraints);
	void ToggleMethod();
	void SetAlternateMethod(bool flag=true);

	void ResetRotations();
	Arcball();
	Arcball(const real& rad);
	Arcball(const real& rad,const gl_unitquaternion& initialOrient);
	Arcball(const Arcball& other);
	~Arcball();

	const Arcball& operator=(const Arcball& other);
	void Resize(const real& newRadius);
	void ClientAreaResize(const int &width, const int &height);
	
	/*events*/ 
	void MouseDown(const int &x, const int &y);
	void MouseUp(const int &x, const int &y);
	void MouseMove(const int &x, const int &y);
	void IssueGLrotation() const;

	/*added by faraz ravi - pointools*/ 
	const Arcball& operator+=(const Arcball& other);
	const Arcball& operator-=(const Arcball& other);
	const Arcball& operator/=(const float &v);
	const Arcball& operator*=(const float &v);

	gl_tmatrix GetMatrix() { return currentQuat.getRotMatrix(); };
	void SetQuat(const gl_unitquaternion& q);
	void GetQuat(gl_unitquaternion& q) { q = currentQuat; };
	void setOtherAxesSet(const gl_vector *axes, int count);
	void writeBranch(pt::datatree::Branch *b) const;
	void readBranch(const pt::datatree::Branch *b);
};

//---------------------------------------------------------------------------
// inlines

inline Arcball::~Arcball()
{
	if(otherAxes) delete[] otherAxes;
}

inline Arcball::Arcball(const Arcball& other)
{
	*this=other;
}

inline void Arcball::Resize(const real& newRadius)
{
	radius=newRadius;
}

inline void Arcball::ClientAreaResize(const int &width, const int &height)
{
	winWidth=real(width);
	winHeight=real(height);
//	center=Point( (newSize.right-newSize.left)/2 , (newSize.bottom-newSize.top)/2);
}

inline Arcball::Arcball()
{
	initVars();
}

inline void Arcball::SetAlternateMethod(bool flag)
{
	bProjectionMethod2=flag;
}

inline void Arcball::ToggleMethod()
{
	if(bProjectionMethod2) bProjectionMethod2=false;
	else bProjectionMethod2=true;
}

inline void Arcball::UseConstraints(AxisSet constraints)
{
	whichConstraints=constraints;
}

inline int Arcball::GetAngleKeyIncrement()
{
	return angleKeyIncrement;
}

inline void Arcball::SetAngleKeyIncrement(int ang)
{
	angleKeyIncrement=abs(ang)%360;
}
};

#endif
