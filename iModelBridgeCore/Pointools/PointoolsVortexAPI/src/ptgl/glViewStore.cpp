/*--------------------------------------------------------------------------*/ 
/*	Pointools GLviewstore class implementation								*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include "PointoolsVortexAPIInternal.h"


#include <gl\glu.h>
#include <ptgl\glviewstore.h>

using namespace ptgl;
 
Viewstore::Viewstore() {}
Viewstore::Viewstore(bool str) { if (str) store(); }
Viewstore::~Viewstore() { }

//-----------------------------------------------------------------------------
// Project
// 3d -> viewport transformation
// based on loaded viewport, model matrix and projection matrix
//
// v = real world vertex x, y, z - should be v[3]
// px = on screen pixel				- should be px[2]
// return value indicates onscreen
//-----------------------------------------------------------------------------
bool Viewstore::project3v(const float *v,int *px) const
{
	double x,y,z;

	int r = gluProject(	v[0],
						v[1],
						v[2],
						model_mat,
						proj_mat,
						viewport,
						&x, &y, &z);
	px[0] = (int)x;
	px[1] = (int)y;

	if (
			px[0] < viewport[0] 
		||	px[1] < viewport[1] 
		||	px[0] > viewport[2] + viewport[0] 
		||	px[1] > viewport[3] + viewport[1] 
		)
		return false;
	else 
		return true;
	/*return (r == GL_TRUE ? true : false);*/ 
}
//-----------------------------------------------------------------------------
bool Viewstore::project3v(const float *v,float *px) const
{
	double x,y,z;

	int r = gluProject(	v[0],
						v[1],
						v[2],
						model_mat,
						proj_mat,
						viewport,
						&x, &y, &z);
	px[0] = x;
	px[1] = y;
	px[2] = z;

	if (
			px[0] < viewport[0] 
		||	px[1] < viewport[1] 
		||	px[0] > viewport[2] + viewport[0] 
		||	px[1] > viewport[3] + viewport[1] 
		||  px[2] < 0 
		||  px[2] > 1.0
		)
		return false;
	else 
		return true;
	/*return (r == GL_TRUE ? true : false);*/ 
}
//
bool Viewstore::project3v(const double *v,int *px) const
{
	double x,y,z;

	int r = gluProject(	v[0],
						v[1],
						v[2],
						model_mat,
						proj_mat,
						viewport,
						&x, &y, &z);
	px[0] = (int)x;
	px[1] = (int)y;

	if (
			px[0] < viewport[0] 
		||	px[1] < viewport[1] 
		||	px[0] > viewport[2] + viewport[0] 
		||	px[1] > viewport[3] + viewport[1] 
		)
		return false;
	else 
		return true;
	/*return (r == GL_TRUE ? true : false);*/ 
}
//
bool Viewstore::project3v(const float *obj, GLdouble *win) const
{
	int r = gluProject(	obj[0],
								obj[1],
								obj[2],
								model_mat,
								proj_mat,
								viewport,
								&win[0], &win[1], &win[2]);

	
	if (
			win[0] < viewport[0]
		||	win[1] < viewport[1]
		||	win[0] > viewport[2] + viewport[0]
		||	win[1] > viewport[3] + viewport[1]
		||  win[2] < 0 
		||  win[2] > 1.0
		)
		return false;
	else 
		return true;
}
//
bool Viewstore::project3v(const double *obj, GLdouble *win) const
{
	int r = gluProject(	obj[0],
								obj[1],
								obj[2],
								model_mat,
								proj_mat,
								viewport,
								&win[0], &win[1], &win[2]);

	
	if (
			win[0] < viewport[0]
		||	win[1] < viewport[1]
		||	win[0] > viewport[2] + viewport[0]
		||	win[1] > viewport[3] + viewport[1]
		||  win[2] < 0 
		||  win[2] > 1.0
		)
		return false;
	else 
		return true;
}
//-----------------------------------------------------------------------------
// UnProject
// 2d->3d viewport transformation
// based on loaded viewport, model matrix and projection matrix
//
// v = real world vertex x, y, z - should be v[3]
// px = on screen pixel				- should be px[2]
// return value indicates onscreen
//-----------------------------------------------------------------------------
void Viewstore::unproject3v(const GLdouble *px,GLdouble *v) const
{
	int r = gluUnProject(px[0],
						px[1],
						px[2],
						model_mat,
						proj_mat,
						viewport,
						&v[0], &v[1], &v[2]);

}
//-----------------------------------------------------------------------------
void Viewstore::unproject3v(const GLfloat *px, GLdouble *v) const
{
	int r = gluUnProject(	(GLdouble)px[0],
							(GLdouble)px[1],
							(GLdouble)px[2],
							model_mat,
							proj_mat,
							viewport,
							&v[0], &v[1], &v[2]);
}
//-----------------------------------------------------------------------------
float Viewstore::unprojectLength(int pixels, float dist) const
{
	GLdouble mmat[16];
	GLdouble v[3];
	GLdouble v2[3];
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(0,0,-dist);

	glGetDoublev(GL_MODELVIEW_MATRIX,mmat);
	
	gluProject(-0.5f, 0, 0,
				mmat,
				proj_mat,
				viewport,
				&v[0], &v[1], &v[2]);
	gluProject(0.5f, 0, 0,
				mmat,
				proj_mat,
				viewport,
				&v2[0], &v2[1], &v2[2]);
	
	glPopMatrix();

	return  (float)((double)pixels / (v2[0] - v[0]));
}
//-----------------------------------------------------------------------------
void Viewstore::transform(const GLfloat *v, GLfloat *t) const
{

	t[0]	=	model_mat[0] * v[0]
			+	model_mat[4] * v[1]
			+	model_mat[8] * v[2]
			+   model_mat[12];	

	t[1]	=	model_mat[1] * v[0]
			+	model_mat[5] * v[1]
			+	model_mat[9] * v[2]
			+   model_mat[13];

	t[2]	=	model_mat[2] * v[0]
			+	model_mat[6] * v[1]
			+	model_mat[10] * v[2]
			+   model_mat[14];
}
