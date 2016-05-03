#include "PointoolsVortexAPIInternal.h"

#ifdef HAVE_OPENGL
#include <ptgl/glState.h>
#include <ptgl/glShader.h>
#endif
#include <ptengine/renderContext.h>
#include <ptengine/renderRGBEffect.h>
#include <ptengine/renderShaderUniforms.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderRGBEffectGL::startFixedFuncFrame( const RenderContext *context )
{
}
/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderRGBEffectGL::endFixedFuncFrame( const RenderContext *context )
{
}

/*****************************************************************************/
/**
* @brief
* @param e
* @return bool
*/
/*****************************************************************************/
bool RenderRGBEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	if (e == RenderEnvOpenGL) return true;
	return false;
}

/*****************************************************************************/
/**
* @brief
* @param settings
* @return bool
*/
/*****************************************************************************/
bool RenderRGBEffectGL::isEnabled( const RenderSettings *settings ) const
{
	return settings->isRGBEnabled();
}

/*****************************************************************************/
/**
* @brief
* @param availableBuffers
* @return bool
*/
/*****************************************************************************/
uint RenderRGBEffectGL::requiredBuffers() const
{
	return Buffer_RGB;
}