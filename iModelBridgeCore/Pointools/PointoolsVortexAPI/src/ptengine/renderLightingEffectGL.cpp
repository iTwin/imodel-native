#include <ptengine/renderLightingEffect.h>
#include <ptengine/renderContext.h>
#include <ptgl/glshader.h>
#include <ptgl/glstate.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief
* @param shader
* @return void
*/
/*****************************************************************************/
void RenderLightingEffectGL::setEffectShaderUniforms( const RenderContext *context, ShaderObj* shader ) const
{
	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);

	if (shaderGL)
	{
	}
}

uint			RenderLightingEffectGL::requiredStandardUniforms() const
{	
	return Uniform_EyeMatrixInverted;
}
/*****************************************************************************/
/**
* @brief
* @param settings
* @param res
* @return void
*/
/*****************************************************************************/
void RenderLightingEffectGL::startFixedFuncFrame( const RenderContext *context )
{
	const RenderSettings *settings = context->settings();
	const ResourceAdaptorI *res = context->resources();

	ptgl::State::enable( GL_LIGHTING );
	ptgl::State::enable( GL_LIGHT0 );

	float gloss = settings->materialGlossiness() * 128;

	glShadeModel(GL_FLAT);
	glMaterialfv(GL_FRONT, GL_AMBIENT, settings->lightAmbient());
	glMaterialfv(GL_FRONT, GL_DIFFUSE, settings->materialDiffuse());
	glMaterialfv(GL_FRONT, GL_SPECULAR, settings->materialSpecular());
	glMaterialfv(GL_FRONT, GL_SHININESS, &gloss);
}

void				RenderLightingEffectGL::startShaderFrame( const RenderContext *context, ShaderObj *shader  )
{
	startFixedFuncFrame( context );
}
void				RenderLightingEffectGL::endShaderFrame( const RenderContext *context, ShaderObj *shader  )
{
	endFixedFuncFrame( context );
}
/*****************************************************************************/
/**
* @brief
* @param settings
* @param res
* @return void
*/
/*****************************************************************************/
void RenderLightingEffectGL::endFixedFuncFrame( const RenderContext *context )
{
	ptgl::State::disable( GL_LIGHTING );
	ptgl::State::disable( GL_LIGHT0 );
}

/*****************************************************************************/
/**
* @brief
* @param e
* @return bool
*/
/*****************************************************************************/
bool RenderLightingEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	return ( e == RenderEnvOpenGL) ? true : false;
}


/*****************************************************************************/
/**
* @brief
* @param settings
* @return bool
*/
/*****************************************************************************/
bool RenderLightingEffectGL::isEnabled( const RenderSettings *settings ) const
{
	return settings->isLightingEnabled();
}

/*****************************************************************************/
/**
* @brief
* @param availableBuffers
* @return bool
*/
/*****************************************************************************/
uint RenderLightingEffectGL::requiredBuffers() const
{
	return Buffer_Normal;
}