#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderShaderUniforms.h>
#include <ptengine/renderClipEffect.h>
#include <ptengine/renderContext.h>
#include <ptengine/visibilityengine.h>
#include <ptengine/engine.h>
#include <ptengine/clipManager.h>
#include <ptgl/glshader.h>
#include <ptgl/glstate.h>

using namespace pointsengine;
using namespace pt;

/*****************************************************************************/
void RenderClipEffectGL::startFixedFuncFrame( const RenderContext *context )
{
	if (isCapable == -1)
	{
		glewInit();
		isCapable = glewIsExtensionSupported("GL_NV_texture_shader") ? 1 : 0;
	}
	if (isCapable && 0)
	{
		glEnable(GL_TEXTURE_SHADER_NV);
		glTexEnvi(GL_TEXTURE_SHADER_NV, GL_SHADER_OPERATION_NV, GL_CULL_FRAGMENT_NV);
	}
	else
	{					
		int numClipPlanes = ClipManager::instance().getNumClippingPlanes();		
		double plane_eq[4];

		for (int i=0; i<numClipPlanes; i++)
		{			
			if (ClipManager::instance().getClippingPlaneParameters(i, plane_eq[0], plane_eq[1], plane_eq[2], plane_eq[3]) == PTV_SUCCESS)
			{								
				glEnable(GL_CLIP_PLANE0+i);
				glClipPlane(GL_CLIP_PLANE0+i, plane_eq);			
			}
		}		
	}
}
/*****************************************************************************/
void RenderClipEffectGL::endFixedFuncFrame( const RenderContext *context )
{
	for (unsigned int i=0; i<ClipManager::instance().getNumClippingPlanes(); i++)
	{
		glDisable(GL_CLIP_PLANE0+i);
	}
}

/*****************************************************************************/
bool RenderClipEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	return ( e == RenderEnvOpenGL) ? true : false;
}
/*****************************************************************************/
bool RenderClipEffectGL::isEnabled( const RenderSettings *settings ) const
{
	return ClipManager::instance().clippingEnabled();
}
/*****************************************************************************/
void	RenderClipEffectGL::startShaderFrame( const RenderContext *context, ShaderObj *shader  )
{	
	int numClipPlanes = ClipManager::instance().getNumClippingPlanes();	
#ifdef _DEBUG		
	// only allow 6 planes for now, this is all that is supported in the shader
	assert(numClipPlanes == 6);
#endif	

	GLfloat* planes = new GLfloat[numClipPlanes*4];
	GLint* planeEnabled = new GLint[numClipPlanes];
	memset(planes, 0, sizeof(GLfloat)*numClipPlanes*4);
	double a,b,c,d;		

	for (int i = 0; i < numClipPlanes; i++)
	{		
		if (planeEnabled[i] = ClipManager::instance().isClippingPlaneEnabled(i))		
		{
			if (ClipManager::instance().getClippingPlaneParameters(i, a, b, c, d) == PTV_SUCCESS)
			{
				planes[i*4] = a;
				planes[(i*4)+1] = b;
				planes[(i*4)+2] = c;
				planes[(i*4)+3] = d;								
			}			
		}					
	}	

	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader *>(shader);
	shaderGL->setUniform4fv("pCP", numClipPlanes, planes);	
	shaderGL->setUniform1iv("pCPe", numClipPlanes, planeEnabled);	
	shaderGL->setUniform1i("CSt", ClipManager::instance().getClipStyle());	

	delete [] planeEnabled;
	delete [] planes;	
}
/*****************************************************************************/
void	RenderClipEffectGL::endShaderFrame( const RenderContext *context, ShaderObj *shader  )
{

}
/*****************************************************************************/
uint	RenderClipEffectGL::requiredStandardUniforms()const
{
	return Uniform_Quantize_O | Uniform_Quantize_S | Uniform_CloudToPrjMatrix | Uniform_ClipBoxMatrix;
}