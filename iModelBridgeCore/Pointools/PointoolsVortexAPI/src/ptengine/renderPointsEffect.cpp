#include <ptgl/glState.h>
#include <ptengine/renderPointsEffect.h>
#include <ptengine/renderFrameData.h>
#include <ptengine/renderContext.h>
#include <ptengine/renderShaderUniforms.h>
#include <ptengine/pointLayers.h>
#include <ptgl/glShader.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderPointsEffectGL::startFixedFuncFrame( const RenderContext *context )
{
	const RenderSettings *settings = context->settings();
	const RenderFrameData &frame =  context->renderer()->frameData();

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glColor3ubv( context->renderer()->activeBuffer()->baseColor() );

	float density = 1.0f;
	float pointSize = settings->pointSize(); 

	if (frame.isDynamic() && settings->dynamicAdaptivePntSize())
	{
		density = frame.frameDensity() * 0.75f;
		if (density > 1.0f) density = 1.0f;
	
		pointSize = settings->pointSize() * (1.0f/density) * 0.5;
	}
	if (pointSize > 6.0f) pointSize = 6.0f;
	else if (pointSize < 1.0f) pointSize = 1.0f;

	glPointSize( pointSize );
}
/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderPointsEffectGL::endFixedFuncFrame( const RenderContext *context )
{
}

/*****************************************************************************/
/**
* @brief
* @param e
* @return bool
*/
/*****************************************************************************/
bool RenderPointsEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	if (e == RenderEnvOpenGL) return true;
	return false;
}

uint RenderPointsEffectGL::requiredBuffers() const
{
	return Buffer_Pos;
}
void	RenderPointsEffectGL::startShaderFrame( const RenderContext *context, ShaderObj *shader )
{
	startFixedFuncFrame( context );	

	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);
}
void	RenderPointsEffectGL::endShaderFrame( const RenderContext *context, ShaderObj *shader )
{

}
