#include "PointoolsVortexAPIInternal.h"

#include <ptgl\glExtensions.h>
#include <ptgl\glClipBox.h>

using namespace ptgl;

#ifndef USE_MESA
static PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
#endif

static bool _glhasTextureShader = false;

ClipBox::ClipBox(const pt::BoundingBox *bb)
{
	_bb = bb;
	if (_bb)
	{
		static bool initialized = false;
		if (!initialized)
		{
			_glhasTextureShader = Ext::isSupported("GL_NV_texture_shader");

#ifndef USE_MESA
			glActiveTextureARB   = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
#endif
			initialized = true;
		}
		static double sPlane[4] = {1.0, 0.0, 0.0, 0.0};
		static double tPlane[4] = {0.0, 1.0, 0.0, 0.0};
		static double rPlane[4] = {0.0, 0.0, 1.0, 0.0};

		static double nsPlane[4] = {-1.0, 0.0, 0.0, 0.0};
		static double ntPlane[4] = {0.0, -1.0, 0.0, 0.0};
		static double nrPlane[4] = {0.0, 0.0, -1.0, 0.0};

		GLfloat texmatrix1[16] = 
			{	1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				-_bb->lx(), -_bb->ly(),  -_bb->lz(), 1.0 };

		GLfloat texmatrix2[16] = 
			{	1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				_bb->ux(), _bb->uy(), _bb->uz(), 1.0 };

		/*check capabilities, if no texture shaders available, use gl clipping planes (slow)*/ 
		if (_glhasTextureShader)
		{
			glEnable(GL_TEXTURE_SHADER_NV);
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glMatrixMode(GL_TEXTURE);

			glLoadMatrixf(texmatrix1);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

			glTexGendv(GL_S, GL_EYE_PLANE, sPlane);
			glTexGendv(GL_T, GL_EYE_PLANE, tPlane);
			glTexGendv(GL_R, GL_EYE_PLANE, rPlane);

			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
			GLint cullmode [] = { GL_GEQUAL, GL_GEQUAL, GL_GEQUAL, GL_GEQUAL };
			glTexEnviv(GL_TEXTURE_SHADER_NV, GL_CULL_MODES_NV, cullmode);

			/*upper clip*/ 
			glActiveTextureARB(GL_TEXTURE2_ARB);
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(texmatrix2);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

			glTexGendv(GL_S, GL_EYE_PLANE, nsPlane);
			glTexGendv(GL_T, GL_EYE_PLANE, ntPlane);
			glTexGendv(GL_R, GL_EYE_PLANE, nrPlane);

			glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
			GLint cullmode2 [] = { GL_GEQUAL, GL_GEQUAL, GL_GEQUAL, GL_GEQUAL };
			glTexEnviv(GL_TEXTURE_SHADER_NV, GL_CULL_MODES_NV, cullmode2);	

			glMatrixMode(GL_MODELVIEW);
		}
		else
		{
			/*use gl clipping planes (this can be much slower*/ 
			double peq0 [] = { 1.0, 0, 0, -_bb->lx()};
			double peq1 [] = { -1.0, 0, 0, _bb->ux()};
			double peq2 [] = { 0, 1.0, 0, -_bb->ly()};
			double peq3 [] = { 0, -1.0, 0, _bb->uy()};
			double peq4 [] = { 0, 0, 1.0, -_bb->lz()};
			double peq5 [] = { 0, 0, -1.0, _bb->uz()};

			glClipPlane(GL_CLIP_PLANE0, peq0);
			glClipPlane(GL_CLIP_PLANE1, peq1);
			glClipPlane(GL_CLIP_PLANE2, peq2);
			glClipPlane(GL_CLIP_PLANE3, peq3);
			glClipPlane(GL_CLIP_PLANE4, peq4);
			glClipPlane(GL_CLIP_PLANE5, peq5);

			glEnable(GL_CLIP_PLANE0);
			glEnable(GL_CLIP_PLANE1);
			glEnable(GL_CLIP_PLANE2);
			glEnable(GL_CLIP_PLANE3);
			glEnable(GL_CLIP_PLANE4);
			glEnable(GL_CLIP_PLANE5);
		}
	}
}
ClipBox::~ClipBox()
{
	if (_bb)
	{
		if (_glhasTextureShader)
		{
			glDisable(GL_TEXTURE_SHADER_NV);
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glActiveTextureARB(GL_TEXTURE0_ARB);
		}
		else
		{
			glDisable(GL_CLIP_PLANE0);
			glDisable(GL_CLIP_PLANE1);
			glDisable(GL_CLIP_PLANE2);
			glDisable(GL_CLIP_PLANE3);
			glDisable(GL_CLIP_PLANE4);
			glDisable(GL_CLIP_PLANE5);		
		}	
	}
}
