/*--------------------------------------------------------------------------*/ 
/*	Pointools GLviewstore class definition									*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Dec 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLS_VIEWSTORE_DEFINITION
#define POINTOOLS_VIEWSTORE_DEFINITION

#ifdef WIN32
#include <windows.h>
#endif

#include "ptgl.h"
#include <pt/geomtypes.h>

#include <gl/gl.h>
#include <gl/glu.h>

namespace ptgl
{
class PTGL_API Viewstore  
{
public:
	Viewstore();
	Viewstore(bool store);
	~Viewstore();

	void store()
	{
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, model_mat);
		glGetDoublev(GL_PROJECTION_MATRIX, proj_mat);
		glGetDoublev(GL_DEPTH_RANGE, depth_range);

		int r, c, i;
		for (r=0;r<4;++r)
		{
			for(c=0; c<4; ++c)
			{
				matrix[r][c] = 0;
				for (i=0; i<4; ++i)
					matrix[r][c] += proj_mat[r+i*4] * model_mat[i+c*4];
			}
		}
	}

	void restore_viewport()
	{
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	}
	void restore_projection()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(proj_mat);

	}
	void restore_modelview()
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixd(model_mat);
	}
	void restore()
	{
		restore_viewport();
		restore_projection();
		restore_modelview();
	}

	inline void	transform(const GLfloat *v, GLfloat *t) const;
	
	inline void	transform(const pt::vector4d &v, pt::vector4d &res) const
	{
		res[0] = matrix[0].dot(v);
		res[1] = matrix[1].dot(v);
		res[2] = matrix[2].dot(v);
		res[3] = matrix[3].dot(v);
	}
	void project4v(const pt::vector4d &v, pt::vector4d &res) const
	{
		transform(v,res);
		const double rhw = 1 /res.w;
		res.x = (1 + res.x * rhw) * viewport[2] / 2 + viewport[0];
		res.y = (1 - res.y * rhw) * viewport[3] / 2 + viewport[1];
		res.z = (res.z * rhw) * (depth_range[1] - depth_range[0]) + depth_range[0];
		res.w = rhw;
	}
	bool project4vb(const pt::vector4d &v, pt::vector4d &res) const
	{
		transform(v,res);
		const double rhw = 1 /res.w;
		res.x = (1 + res.x * rhw) * viewport[2] / 2 + viewport[0];
		res.y = (1 - res.y * rhw) * viewport[3] / 2 + viewport[1];
		res.z = (res.z * rhw) * (depth_range[1] - depth_range[0]) + depth_range[0];
		res.w = rhw;

		return	res.x >= viewport[0] &&
				res.y >= viewport[1] &&
				res.x <= viewport[0] + viewport[2] &&
				res.y <= viewport[1] + viewport[3];
	}

	bool	project3v(const GLfloat *v, int *px) const;
	bool	project3v(const GLfloat *v, GLfloat *px) const;
	bool	project3v(const GLdouble *v, int *px) const;
	bool	project3v(const GLfloat *obj, GLdouble *win) const;
	bool	project3v(const GLdouble *obj, GLdouble *win) const;
	
	inline void	_project3v(const GLdouble *obj, GLdouble *win) const
	{
		gluProject(	obj[0],	obj[1],	obj[2],	model_mat, proj_mat,viewport, &win[0], &win[1], &win[2]);
	}

	void	unproject3v(const GLdouble *px, GLdouble *v) const;
	void	unproject3v(const GLfloat *px, GLdouble *v) const;
	float	unprojectLength(int pixels, GLfloat dist=0) const;

	GLint		viewport[4];
	GLdouble	model_mat[16];
	GLdouble	proj_mat[16];
	GLdouble	depth_range[2];
	pt::vector4d matrix[4];

#ifdef WIN32
	HDC			dc;
#endif
};
}
#endif
