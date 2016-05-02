#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderPlaneEffect.h>
#include <ptengine/renderContext.h>
#include <ptengine/renderShaderUniforms.h>
#include <ptgl/glshader.h>
#include <ptgl/glstate.h>

using namespace pointsengine;

uint			RenderPlaneEffectGL::requiredStandardUniforms() const
{
	return Uniform_Quantize_O | Uniform_CloudToPrjMatrix;
}

/*****************************************************************************/
/**
* @brief
* @param settings
* @param res
* @return void
*/
/*****************************************************************************/
void RenderPlaneEffectGL::startFixedFuncFrame( const RenderContext *context )
{
	const RenderSettings *settings = context->settings();
	const ResourceAdaptorI *res = context->resources();

	ptgl::State::enable( GL_TEXTURE_1D );
	
	/* fixed func pipline version */
	/* in tex unit 2 */ 
	glActiveTexture( GL_TEXTURE1 );
	glEnable(GL_TEXTURE_1D);

	int gradient = settings->geomShaderGradient();

	TextureMap::iterator i = m_gradients.find(gradient);
	Texture1D* texture = 0;
	if ( i== m_gradients.end())		//not created in this context
	{
		texture = res->createGradientTexture( gradient );
		if (texture)
		{
			m_gradients.insert( TextureMap::value_type(gradient,texture) );
		}
	}
	else texture = i->second;

	if (texture)
	{
		glBindTexture( GL_TEXTURE_1D, texture->m_texid );

		// handle texture edge
		switch( settings->geomShaderEdge())
		{
		case RepeatEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			break;
		case ClampEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			break;
		case BlackEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // opengl 1.3+
			break;
		case MirroredRepeat:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // opengl 1.4+

		}
	}
	// texture coord generation
    // RB_VORTEX_TODO: should we really narrow down parameters to floats???
    GLfloat params [] = {	(GLfloat)settings->geomShaderParams()[0],
							(GLfloat)settings->geomShaderParams()[1],
							(GLfloat)settings->geomShaderParams()[2],
							(GLfloat)settings->geomShaderParams()[3] };

	// nice in theory but does not work because points might be in short values
	// with decompress matrix. These tex coords get generate at the original point
	// values and not the values after decomp matrix is applied.
	// cest probleme. So there is a hack for tex unit 1(2) that applies
	// a matrix to the text coords that are generated to correct them
	// but its not here
	glTexGenfv(GL_S, GL_OBJECT_PLANE, params );
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glEnable(GL_TEXTURE_GEN_S);
}

/*****************************************************************************/
/**
* @brief
* @param settings
* @param res
* @return void
*/
/*****************************************************************************/
void RenderPlaneEffectGL::endFixedFuncFrame( const RenderContext *context )
{
	glActiveTexture( GL_TEXTURE1 );
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_GEN_S);
}
void RenderPlaneEffectGL::startShaderFrame(  const RenderContext *context, ShaderObj *shader  )
{
	startFixedFuncFrame( context );
	glDisable(GL_TEXTURE_GEN_S);

	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);

	if (shaderGL)
	{
		const RenderSettings *settings = context->settings();

        // RB_VORTEX_TODO: should we really narrow down parameters to floats???
		GLfloat params [] = {	(GLfloat)settings->geomShaderParams()[0],
								(GLfloat)settings->geomShaderParams()[1],
								(GLfloat)settings->geomShaderParams()[2],
								(GLfloat)settings->geomShaderParams()[3] };

		shaderGL->setUniform1i( UNIFORM_PLANE_TEX, TEX_UNIT_PLANE );
		shaderGL->setUniform4fv( UNIFORM_PLANE, 1, params );
	}
}
void RenderPlaneEffectGL::endShaderFrame(  const RenderContext *context, ShaderObj *shader  )
{
	endFixedFuncFrame( context );
	glDisable(GL_TEXTURE_GEN_S);
}

bool RenderPlaneEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	return ( e == RenderEnvOpenGL) ? true : false;
}

bool RenderPlaneEffectGL::isEnabled( const RenderSettings *settings ) const
{
	ShaderEdgeMode &edge = const_cast<RenderPlaneEffectGL*>(this)->m_edge;
	edge = settings->geomShaderEdge(); //bit of a hack, we need this info later
	return settings->isGeomShaderEnabled();
}
const char*		RenderPlaneEffectGL::shaderDefine() const
{ 
	switch (m_edge)
	{
		case RepeatEdge: return "#define PT_P"; 
		case ClampEdge: return "#define PT_P\n#define PT_P_EDGE_CLAMP";
		case BlackEdge: return "#define PT_P\n#define PT_P_EDGE_STOP";
		case MirroredRepeat: return "#define PT_P\n#define PT_P_EDGE_MIRROR";
	}
	return "define PT_P";
}

