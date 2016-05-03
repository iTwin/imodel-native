/*--------------------------------------------------------------------------*/ 
/*	Pointools GLarcball class implementation								*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifdef WIN32
/*truncation from 'double' to 'float'*/ 
#pragma warning ( disable:4305 ) 
#endif

#include "PointoolsVortexAPIInternal.h"


#include <ptgl/glArcball.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include <pt/datatree.h>

using namespace ptgl;

Arcball::Arcball(const real& rad)
{
	initVars();
	radius=clamped(rad,0.1,1);
}


Arcball::Arcball(const real& rad,const gl_unitquaternion& initialOrient)
{
	initVars();
	radius=clamped(rad,0.1,1);
	currentQuat=initialOrient;
}

const Arcball& Arcball::operator=(const Arcball& other)
{
	if(this==&other) return *this;
	initVars();
	currentQuat=other.currentQuat;
	previousQuat=other.previousQuat;
	radius=other.radius;
	winWidth=other.winWidth;
	winHeight=other.winHeight;
	otherAxesNum=other.otherAxesNum;
	otherAxes=new gl_vector[otherAxesNum];
	int i; for (i=0; i<otherAxesNum; i++)	otherAxes[i] = other.otherAxes[i];

	center = other.center;
	mouseButtonDown = other.mouseButtonDown;
	whichConstraints = other.whichConstraints;
	currentAxisIndex = other.currentAxisIndex;
	for (i=0; i<3; i++)
	{
		cameraAxes[i] = other.cameraAxes[i];
		bodyAxes[i] = other.bodyAxes[i];
	}
	return *this;
}
const Arcball& Arcball::operator+=(const Arcball& other)
{
	gl_quaternion cq(currentQuat.scalarPart(), currentQuat.vectorPart());
	cq += other.currentQuat;
	currentQuat = gl_unitquaternion(cq);

	gl_quaternion pq(previousQuat.scalarPart(), previousQuat.vectorPart());
	pq += other.previousQuat;
	previousQuat = gl_unitquaternion(pq);
	return *this;
}
const Arcball& Arcball::operator-=(const Arcball& other)
{
	gl_quaternion cq(currentQuat.scalarPart(), currentQuat.vectorPart());
	cq -= other.currentQuat;
	currentQuat = gl_unitquaternion(cq);

	gl_quaternion pq(previousQuat.scalarPart(), previousQuat.vectorPart());
	pq -= other.previousQuat;
	previousQuat = gl_unitquaternion(pq);	return *this;
}
const Arcball& Arcball::operator/=(const float &v)
{
	currentQuat  *= 1.0f/v;
	previousQuat *= 1.0f/v;
	return *this;
}
const Arcball& Arcball::operator*=(const float &v)
{
	currentQuat  *=v;
	previousQuat *=v;
	return *this;
}

void Arcball::MouseDown(const int &x, const int &y)
{
	xprev=(2*x-winWidth)/winWidth;
	yprev=(winHeight-2*y)/winHeight;
	previousQuat=currentQuat;
	mouseButtonDown=true;
	bDrawBallArea=bProjectionMethod2;// draw circle only if method 2 active

/*	codice per memorizzare il vettore iniziale dove uno clicca per rubberbanding

	real winWidth=clientArea.Width();
	real winHeight=clientArea.Height();
	vdown=gl_vector((2*location.x-winWidth)/winWidth,(winHeight-2*location.y)/winHeight,0);
	ProjectOnSphere2(vdown);
	vcurr=vdown;
*/
}
void Arcball::SetQuat(const gl_unitquaternion& q)
{ 
	previousQuat = currentQuat = q; 
	// save current rotation axes for bodyAxes constraint at next rotation
	bodyorientation=currentQuat.getRotMatrix();
	bodyAxes[0]=gl_vector(bodyorientation(0,0),bodyorientation(1,0),bodyorientation(2,0));
	bodyAxes[1]=gl_vector(bodyorientation(0,1),bodyorientation(1,1),bodyorientation(2,1));
	bodyAxes[2]=gl_vector(bodyorientation(0,2),bodyorientation(1,2),bodyorientation(2,2));
}


void Arcball::MouseUp(const int &x, const int &y)
{
	mouseButtonDown=false;
	xprev=yprev=0.0;
	bDrawBallArea=false;
	// save current rotation axes for bodyAxes constraint at next rotation
	bodyorientation=currentQuat.getRotMatrix();
	bodyAxes[0]=gl_vector(bodyorientation(0,0),bodyorientation(1,0),bodyorientation(2,0));
	bodyAxes[1]=gl_vector(bodyorientation(0,1),bodyorientation(1,1),bodyorientation(2,1));
	bodyAxes[2]=gl_vector(bodyorientation(0,2),bodyorientation(1,2),bodyorientation(2,2));

//	vdown=vcurr=ORIGIN;// per rubberbanding
}


void Arcball::MouseMove(const int &x, const int &y)
{
	real xcurr=(2*x-winWidth)/winWidth;
	real ycurr=(winHeight-2*y)/winHeight;
	gl_vector vfrom(xprev,yprev,0);
	gl_vector vto(xcurr,ycurr,0);
	if(mouseButtonDown)
	{
// find the two points on sphere according to the projection method
		ProjectOnSphere(vfrom);
		ProjectOnSphere(vto);
// modify the vectors according to the active constraint
		if(whichConstraints != NO_AXES)
		{
			gl_vector* axisSet=GetUsedAxisSet();
			vfrom=ConstrainToAxis(vfrom,axisSet[currentAxisIndex]);
			vto=ConstrainToAxis(vto,axisSet[currentAxisIndex]);
		};
// get the corresponding gl_unitquaternion
		gl_unitquaternion lastQuat=RotationFromMove(vfrom,vto);
		currentQuat*=lastQuat;
		xprev=xcurr;
		yprev=ycurr;
	}
	else if(whichConstraints != NO_AXES)
		{
			ProjectOnSphere(vto);
			currentAxisIndex=NearestConstraintAxis(vto);
		};
}

void Arcball::IssueGLrotation() const
{
#ifdef REAL_IS_DOUBLE
	glMultMatrixd(currentQuat.getRotMatrix());
#else
	glMultMatrixf(currentQuat.getRotMatrix());
#endif
}

void Arcball::ProjectOnSphere(gl_vector& v) const
{
	real rsqr=radius*radius;
	real dsqr=v.x()*v.x()+v.y()*v.y();
	if(bProjectionMethod2)
	{
		// if inside sphere project to sphere else on plane
		if(dsqr>rsqr)
		{
			register real scale=(radius-.05)/sqrt(dsqr);
			v.x()*=scale;
			v.y()*=scale;
			v.z()=0;
		}
		else
		{
			v.z()=sqrt(rsqr-dsqr);
		}
	}
	else
	{
		// if relatively "inside" sphere project to sphere else on hyperbolic sheet
		if(dsqr<(rsqr*0.5))	v.z()=sqrt(rsqr-dsqr);
		else v.z()=rsqr/(2*sqrt(dsqr));
	};
}

gl_unitquaternion Arcball::RotationFromMove(const gl_vector& vfrom,const gl_vector& vto)
{
//	vcurr=vto;// per rubberbanding
	if(bProjectionMethod2)
	{
		gl_quaternion q;
		q.x()=vfrom.z()*vto.y()-vfrom.y()*vto.z();
		q.y()=vfrom.x()*vto.z()-vfrom.z()*vto.x();
		q.z()=vfrom.y()*vto.x()-vfrom.x()*vto.y();
		q.w()=vfrom*vto;
		return gl_unitquaternion(q);
	}
	else
	{
// calculate axis of rotation and correct it to avoid "near zero length" rot axis
		gl_vector rotaxis=(vto^vfrom);
		rotaxis.EpsilonCorrect(X_AXIS);
// find the amount of rotation
		gl_vector d(vfrom-vto);
		real t=d.length()/(2.0*radius);
		clamp(t,-1.0,1.0);
		real phi=2.0*asin(t);
		return gl_unitquaternion(phi,rotaxis);
	}
}
/*
void Arcball::Key(UINT nChar)
{
	switch(nChar)
	{
	case VK_UP:
	case VK_NUMPAD8:
		currentQuat*=gl_unitquaternion(DegToRad(angleKeyIncrement),X_AXIS);
		break;
	case VK_DOWN:
	case VK_NUMPAD2:
		currentQuat*=gl_unitquaternion(DegToRad(-angleKeyIncrement),X_AXIS);
		break;
	case VK_RIGHT:
	case VK_NUMPAD6:
		currentQuat*=gl_unitquaternion(DegToRad(angleKeyIncrement),Y_AXIS);
		break;
	case VK_LEFT:
	case VK_NUMPAD4:
		currentQuat*=gl_unitquaternion(DegToRad(-angleKeyIncrement),Y_AXIS);
		break;
	case VK_PRIOR:
	case VK_NUMPAD9:
		currentQuat*=gl_unitquaternion(DegToRad(angleKeyIncrement),Z_AXIS);
		break;
	case VK_HOME:
	case VK_NUMPAD7:
		currentQuat*=gl_unitquaternion(DegToRad(-angleKeyIncrement),Z_AXIS);
		break;
	case VK_DELETE:
	case VK_NUMPAD5:
		currentQuat=gl_unitquaternion(0,X_AXIS);
		break;
	case VK_ESCAPE:
		currentQuat=previousQuat;
		MouseUp(Point(-1,-1));
		break;
	case VK_TAB:
		if(mouseButtonDown && whichConstraints!=NO_AXES)
		{
			currentAxisIndex=(currentAxisIndex+1)%3;
			currentQuat=previousQuat;
		}
		break;
	};
}

void Arcball::SetColor(COLORREF col)
{
	BallColor.x()=GetRValue(col)/255.0;
	BallColor.y()=GetGValue(col)/255.0;
	BallColor.z()=GetBValue(col)/255.0;
}

void Arcball::SetColorV(gl_vector colvec)
{
	clamp(colvec,0,1);
	BallColor=colvec;
}
*/ 
gl_vector Arcball::ConstrainToAxis(const gl_vector& loose,const gl_vector& axis)
{
	gl_vector onPlane;
    register real norm;
    onPlane = loose-axis*(axis*loose);
    norm = onPlane.length();
    if (norm > 0)
	{
		if (onPlane.z() < 0.0) onPlane = -onPlane;
		return ( onPlane/=sqrt(norm) );
    };
    if (fabs(axis.z()) > 0.999f) onPlane = X_AXIS;
	else
	{
		onPlane = gl_vector(-axis.y(), axis.x(), 0);
		onPlane.normalize();
    }
    return (onPlane);
}
void Arcball::ResetRotations()
{
	xprev=yprev=0.0;
	previousQuat=currentQuat=gl_unitquaternion(0,X_AXIS);
	mouseButtonDown=bDrawBallArea=bProjectionMethod2=bDrawConstraints=false;
	cameraAxes[0]=bodyAxes[0]=X_AXIS;
	cameraAxes[1]=bodyAxes[1]=Y_AXIS;
	cameraAxes[2]=bodyAxes[2]=Z_AXIS;
	bodyorientation.loadIdentity();
}
void Arcball::initVars()
{
    const gl_vector X_AXIS(1,0,0);
    const gl_vector Y_AXIS(0,1,0);
    const gl_vector Z_AXIS(0,0,1);
	winWidth=winHeight=0;
	previousQuat=currentQuat=gl_unitquaternion(0,X_AXIS);

	mouseButtonDown=bDrawBallArea=bProjectionMethod2=bDrawConstraints=false;
	xprev=yprev=0.0;
	center.x = 0;
	center.y = 0;
	radius=0.6;
	GLdisplayList=currentAxisIndex=otherAxesNum=0;

	otherAxes=NULL;
	whichConstraints=NO_AXES;
	cameraAxes[0]=bodyAxes[0]=X_AXIS;
	cameraAxes[1]=bodyAxes[1]=Y_AXIS;
	cameraAxes[2]=bodyAxes[2]=Z_AXIS;
	bodyorientation.loadIdentity();
	angleKeyIncrement=5;
	mouseButtonDown = false;
}

int Arcball::NearestConstraintAxis(const gl_vector& loose)
{
	gl_vector* axisSet=GetUsedAxisSet();
	gl_vector onPlane;
	register float max, dot;
	register int i, nearest;
	max = -1; 
	nearest = 0;
	if(whichConstraints == OTHER_AXES)
	{
		for (i=0; i<otherAxesNum; i++)
		{
			onPlane = ConstrainToAxis(loose, otherAxes[i]);
			dot = onPlane*loose;
			if (dot>max) max = dot; nearest = i;
		}
	}
	else
	{
		for (i=0; i<3; i++)
		{
			onPlane = ConstrainToAxis(loose, axisSet[i]);
			dot = onPlane*loose;
			if (dot>max)
			{
				max = dot;
				nearest = i;
			};
		}
	};
	return (nearest);
}

gl_vector* Arcball::GetUsedAxisSet()
{
    gl_vector* axes=NULL;
	switch(whichConstraints)
	{
	case CAMERA_AXES: axes=cameraAxes;
		break;
	case BODY_AXES: axes=bodyAxes;
		break;
	case OTHER_AXES: axes=otherAxes;
		break;
	};
	return axes;
}

void Arcball::setOtherAxesSet(const gl_vector *axes, int count)
{
	otherAxesNum = count;
	if (otherAxes) delete otherAxes;
	otherAxes = new gl_vector[count];

	for (int i=0; i<count; i++) otherAxes[i] = axes[i];
}
/*
const Arcball &Arcball::operator(const Arcball &a)
{
	bDrawConstraints a.bDrawConstraints;
	bodyorientation = a.bodyorientation;
	angleKeyIncrement = a.angleKeyIncrement;

	BallColor = a.BallColor;
	bProjectionMethod2 = a.bProjectionMethod2;
	bDrawBallArea = a.bDrawBallArea;

	GLdisplayList = a.GLdisplayList;

	currentQuat = a.currentQuat;
	previousQuat = a.previousQuat;
	radius = a.radius;
	winWidth = a.winWidth;
	winHeight = a.winHeight; 
	xprev = a.xprev;
	yprev = a.yprev;
	center = a.center;

	mouseButtonDown = a.mouseButtonDown;

	whichConstraints = a.whichConstraints;
	currentAxisIndex = a.currentAxisIndex;

	int i;
	for (i=0; i<3; i++)
	{
		cameraAxes[i] = a.cameraAxes[i];
		bodyAxes[i] = a.bodyAxes[i];
	}
	otherAxesNum = a.otherAxesNum;

	for (i=0; i<otherAxesNum; i++)
	{
		otherAxes[i] = a.otherAxes[i];
	}
	return (*this);
}
*/
static void writeVector(const gl_vector &v, pt::datatree::NodeID nid, pt::datatree::Branch *b)
{
	char _nid[32];
	char id[32];
	nid.get(_nid);

	sprintf(id, "%s_x", _nid);
	b->addNode(id, v.x());
	sprintf(id, "%s_y", _nid);
	b->addNode(id, v.y());
	sprintf(id, "%s_z", _nid);
	b->addNode(id, v.z());
}
static void readVector(gl_vector &v, pt::datatree::NodeID nid, const pt::datatree::Branch *b)
{
	char _nid[32];
	char id[32];
	nid.get(_nid);

	sprintf(id, "%s_x", _nid);
	b->getNode(id, v.x());
	sprintf(id, "%s_y", _nid);
	b->getNode(id, v.y());
	sprintf(id, "%s_z", _nid);
	b->getNode(id, v.z());
}
void Arcball::writeBranch(pt::datatree::Branch *b) const
{	
	/*current quat*/ 
	gl_vector	v = currentQuat.vectorPart();
	double		s = currentQuat.scalarPart();

	writeVector(v, "current_quat", b);
	b->addNode("current_quat_s", s);

	/*previous Quat*/ 
	v = previousQuat.vectorPart();
	s = previousQuat.scalarPart();

	writeVector(v, "prev_quat", b);
	b->addNode("prev_quat_s", s);

	b->addNode("radius", radius);
	b->addNode("viewport_width", winWidth);
	b->addNode("viewport_height", winHeight);
	b->addNode("num_other_axis", otherAxesNum);

	char axis[32];

	int i; for (i=0; i<otherAxesNum; i++)
	{
		sprintf(axis, "axis%d", i);
		writeVector(otherAxes[i], axis, b);
	}
	
	b->addNode("center_x", center.x);
	b->addNode("center_y", center.y);

	b->addNode("contrainsts", (int)whichConstraints);
	b->addNode("current_axis", currentAxisIndex);

	writeVector(cameraAxes[0], "camera_axis_0", b);
	writeVector(cameraAxes[1], "camera_axis_1", b);
	writeVector(cameraAxes[2], "camera_axis_2", b);

	writeVector(bodyAxes[0], "body_axis_0", b);
	writeVector(bodyAxes[1], "body_axis_1", b);
	writeVector(bodyAxes[2], "body_axis_2", b);
}
void Arcball::readBranch(const pt::datatree::Branch *b)
{
	initVars();

	/*current quat*/ 
	gl_vector	v;
	double		s;

	readVector(v, "current_quat", b);
	b->getNode("current_quat_s", s);

	currentQuat = gl_unitquaternion(gl_quaternion(s, v));

	/*previous Quat*/ 
	readVector(v, "prev_quat", b);
	b->getNode("prev_quat_s", s);

	previousQuat = gl_unitquaternion(gl_quaternion(s, v));

	b->getNode("radius", radius);
	b->getNode("viewport_width", winWidth);
	b->getNode("viewport_height", winHeight);
	b->getNode("num_other_axis", otherAxesNum);

	char axis[32];

	int i; for (i=0; i<otherAxesNum; i++)
	{
		sprintf(axis, "axis%d", i);
		readVector(otherAxes[i], axis, b);
	}
	
	b->getNode("center_x", center.x);
	b->getNode("center_y", center.y);

	int constraints;
	b->getNode("contrainsts", constraints);
	whichConstraints = (AxisSet)constraints;

	b->getNode("current_axis", currentAxisIndex);

	readVector(cameraAxes[0], "camera_axis_0", b);
	readVector(cameraAxes[1], "camera_axis_1", b);
	readVector(cameraAxes[2], "camera_axis_2", b);

	readVector(bodyAxes[0], "body_axis_0", b);
	readVector(bodyAxes[1], "body_axis_1", b);
	readVector(bodyAxes[2], "body_axis_2", b);
}
