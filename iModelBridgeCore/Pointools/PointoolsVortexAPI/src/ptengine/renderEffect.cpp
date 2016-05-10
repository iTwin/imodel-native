#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderEffect.h>
#include <ptengine/renderContext.h>

using namespace pointsengine;


/*****************************************************************************/
/**
* @brief Adds a new effect to be managed by the manager
* @param effect
* @return int
*/
/*****************************************************************************/
int		RenderEffectsManager::addEffect(pointsengine::RenderEffectI *effect)
{
	m_disableColor = false;
	m_effects.push_back( effect );
	return static_cast<int>(m_effects.size()-1);
}


/*****************************************************************************/
/**
* @brief
* @return int
*/
/*****************************************************************************/
int RenderEffectsManager::numOfEffects() const
{
	return (int)m_effects.size();
}

/*****************************************************************************/
/**
* @brief 
* @param index
* @return PointsRenderEffectI		*
*/
/*****************************************************************************/
RenderEffectI		* RenderEffectsManager::renderEffect(int index)
{
	return index < m_effects.size() ? m_effects[ index ] : NULL;
}

/*****************************************************************************/
/**
* @brief 
* @param index
* @return PointsRenderEffectI		*
*/
/*****************************************************************************/
const RenderEffectI		* RenderEffectsManager::renderEffect(int index) const
{
	return index < m_effects.size() ? m_effects[ index ] : NULL;
}

/*****************************************************************************/
/**
* @brief			Sets up all enabled effects for a rendering batch
* @param context	The current rendering context
* @return uint		Bit mask of buffers required to render all enabled effects
*/
/*****************************************************************************/
void	RenderEffectsManager::startFrame(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader )
{
	EffectsContainer::iterator i = m_effects.begin();

	while (i != m_effects.end())
	{
		uint reqBuffers = (*i)->requiredBuffers();

		// is enabled and min reqs met
		if ( (*i)->isEnabled( context->settings() )
			&& (*i)->compatibleEnvironment( context->environment() )
			&& ((buffersAvailable & reqBuffers) == reqBuffers))
		{
			if (!shader)
				(*i)->startFixedFuncFrame( context );
			else
			{
				(*i)->startShaderFrame( context, shader );					
			}
		}
		++i;
	}
}


/*****************************************************************************/
/**
* @brief			Sets up all enabled effects for a rendering batch
* @param context	The current rendering context
* @return uint		Bit mask of buffers required to render all enabled effects
*/
/*****************************************************************************/
void	RenderEffectsManager::startBuffer(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader )
{
	if (!shader) return;

	EffectsContainer::iterator i = m_effects.begin();

	while (i != m_effects.end())
	{
		uint reqBuffers = (*i)->requiredBuffers();

		// is enabled and min reqs met
		if ( (*i)->isEnabled( context->settings() )
			&& (*i)->compatibleEnvironment( context->environment() )
			&& ((buffersAvailable & reqBuffers) == reqBuffers))
		{
			(*i)->startShaderBuffer( context, shader );					
		}
		++i;
	}
}


/*****************************************************************************/
/**
* @brief			Cleans up rendering effects set up for rendering batch
* @param context	The current rendering context
* @return void		
*/
/*****************************************************************************/
void	RenderEffectsManager::endFrame(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader )
{
	EffectsContainer::iterator i = m_effects.begin();

	while (i != m_effects.end())
	{
        // RB_VORTEX_TODO: Not sure about the third part of this conditional... what was the intention???
        // Before change, this was --> (buffersAvailable & (*i)->requiredBuffers() == (*i)->requiredBuffers()) (added parentheses)
		if ((*i)->isEnabled( context->settings() ) 
			&& (*i)->compatibleEnvironment( context->environment() )
			&& ((buffersAvailable & (*i)->requiredBuffers()) == (*i)->requiredBuffers()))
		{
			if (!shader)
				(*i)->endFixedFuncFrame( context );
			else
				(*i)->endShaderFrame( context, shader );
		}
		++i;
	}
}

/*****************************************************************************/
/**
* @brief			Cleans up rendering effects set up for rendering batch
* @param context	The current rendering context
* @return void		
*/
/*****************************************************************************/
void	RenderEffectsManager::endBuffer(  const RenderContext *context, uint buffersAvailable, ShaderObj *shader )
{
	if (!shader) return;

	EffectsContainer::iterator i = m_effects.begin();

	while (i != m_effects.end())
	{
        // RB_VORTEX_TODO: Not sure about the third part of this conditional... what was the intention???
        // Before change, this was --> (buffersAvailable & (*i)->requiredBuffers() == (*i)->requiredBuffers()) (added parentheses)
		if ((*i)->isEnabled( context->settings() ) 
			&& (*i)->compatibleEnvironment( context->environment() )
			&& ((buffersAvailable & (*i)->requiredBuffers()) == (*i)->requiredBuffers()))
		{
			(*i)->endShaderBuffer( context, shader );
		}
		++i;
	}
}
/*****************************************************************************/
/**
* @brief
* @param context
* @return uint
*/
/*****************************************************************************/
uint RenderEffectsManager::requiredBuffers( const RenderContext *context ) const
{
	EffectsContainer::const_iterator i = m_effects.begin();
	uint buffersNeeded = 0;
	while (i != m_effects.end())
	{
		if ((*i)->isEnabled( context->settings() ) 
			&& (*i)->compatibleEnvironment( context->environment() ))
		{
			buffersNeeded |= (*i)->requiredBuffers();
		}
		++i;
	}
	return buffersNeeded;
}