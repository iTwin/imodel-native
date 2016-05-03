#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderContext.h>
#include <ptengine/renderMethodGLPoints.h>
#include <ptengine/renderGLResourceAdaptor.h>
#include <ptengine/renderPipelineGLShader.h>
#include <ptengine/renderIntensityEffect.h>		//remove later, this is a test
#include <ptengine/renderPointsEffect.h>		//remove later, this is a test
#include <ptengine/renderPlaneEffect.h>
#include <ptengine/renderRGBEffect.h>
#include <ptengine/renderClipEffect.h>
#include <ptengine/renderLightingEffect.h>
#include <ptengine/renderLayersEffect.h>

using namespace pointsengine;
/*****************************************************************************/
/**
* @brief
* @param env
*/
/*****************************************************************************/
RenderContext::RenderContext( RenderEnvironment env )
{
	m_environment = env;
}
/*****************************************************************************/
/**
* @brief
*/
/*****************************************************************************/
RenderContext::~RenderContext( )
{
	delete m_renderer;		// also need to delete pipeline and method
	delete m_resources;
	delete m_settings;
}
/*****************************************************************************/
/**
* @brief
* @return PointsRenderer			*
*/
/*****************************************************************************/
PointsRenderer			* RenderContext::renderer()
{
	return m_renderer;
}
/*****************************************************************************/
/**
* @brief
* @return const PointsRenderer	*
*/
/*****************************************************************************/
const PointsRenderer	* RenderContext::renderer() const
{
	return m_renderer;
}
/*****************************************************************************/
/**
* @brief
* @return RenderSettings			*
*/
/*****************************************************************************/
RenderSettings			* RenderContext::settings()
{
	return m_settings;
}
/*****************************************************************************/
/**
* @brief const version
* @return const RenderSettings	*
*/
/*****************************************************************************/
const RenderSettings	* RenderContext::settings() const
{
	return m_settings;
}
/*****************************************************************************/
/**
* @brief
* @return ResourceAdapterI		*
*/
/*****************************************************************************/
const ResourceAdaptorI		* RenderContext::resources() const
{
	return m_resources;
}
/*****************************************************************************/
/**
* @brief
* @param env
* @return void
*/
/*****************************************************************************/
RenderContext *RenderContextManager::getRenderContext( RenderEnvironment env, ContextID context )
{
	switch (env)
	{
	case RenderEnvOpenGL:
#ifdef HAVE_OPENGL
        return getOpenGLContext( context );
#else
        return 0;  //not supported
#endif
		
	case RenderEnvDirect3D:
		return getDirect3DContext( context );

	case RenderEnvOther:
		return getOtherContext( context );
	}
	return 0;	//not supported
}
/*****************************************************************************/
/**
* @brief
* @param context
* @return bool
*/
/*****************************************************************************/
bool RenderContextManager::destroyRenderContext( ContextID context )
{
	ContextMap::iterator i = m_contexts.find( context );
	
	if (i == m_contexts.end()) return false;

	extern RenderContext *g_currentRenderContext;

	if (g_currentRenderContext == i->second)
		g_currentRenderContext = 0;

	delete i->second;
	m_contexts.erase( i );

	return true;
}
/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
RenderContextManager::RenderContextManager()
{

}
RenderContextManager::~RenderContextManager()
{

}
/*****************************************************************************/
/**
* @brief
* @return RenderContextManager *
*/
/*****************************************************************************/
RenderContextManager *RenderContextManager::instance()
{
	static RenderContextManager *rcm=0;

	if (!rcm)
	{
		rcm = new RenderContextManager;
	}	
	return rcm;
}
#ifdef HAVE_OPENGL
/*****************************************************************************/
/**
* @brief
* @return RenderContext	*
*/
/*****************************************************************************/
RenderContext	* RenderContextManager::getOpenGLContext( ContextID cid, bool forceFixedFunc )
{
	// instance per thread is ok, threads cannot share GL contexts
	ContextID context = cid ? cid : (ContextID)wglGetCurrentContext();

	if (!context) return 0;

	ContextMap::iterator i = m_contexts.find( context );

	if (i == m_contexts.end())
	{
		RenderContext* rc = createOpenGLContext( forceFixedFunc );
		m_contexts.insert( ContextMap::value_type( context, rc) );
		return rc;
	}
	else
	{
		return i->second;
	}
}
/*****************************************************************************/
/**
* @brief	Creates an OpenGL rendering object
* @detail	The OpenGL context for the rendering object must be current in order
*			for resources to be correctly created. 
* @return RenderContext *
*/
/*****************************************************************************/
RenderContext * RenderContextManager::createOpenGLContext(bool forceFixedFunc) 
{
	glewInit();

	RenderContext *rc = new RenderContext(RenderEnvOpenGL);
	RenderPipelineI *pipeline = 0;
	RenderMethodI *method = 0;
	RenderEffectsManager *effects = new RenderEffectsManager;

	method = new RenderMethod_GLVertexArray;
	pipeline  = new RenderPipeline_GLShader(method);	// try Shader based pipeline first

	if (!pipeline->isSupportedOnPlatform() || forceFixedFunc)
	{
		delete pipeline;
		pipeline = new RenderPipeline_GLFixed(method);	// try fixed function pipeline
		
		if (!pipeline->isSupportedOnPlatform())
		{
			delete pipeline;
			delete method;		
			pipeline = 0; 
		}
	}
	
	if (!pipeline) return 0;	// failure, big time

	rc->m_settings = new RenderSettings;
	rc->m_settings->setToDefault();
	
	extern RenderContext *g_currentRenderContext;

	if (g_currentRenderContext)		//copy current context settings
	{
		//*(rc->m_settings) = *g_currentRenderContext->settings();
	}

	rc->m_resources = new OpenGLResourceAdaptor;
	rc->m_renderer = new PointsRenderer( pipeline );
	rc->m_contextId = (ContextID)wglGetCurrentContext();	
	rc->m_effects = effects;
	rc->m_effects->addEffect( new RenderIntensityEffectGL );	// a test
	rc->m_effects->addEffect( new RenderRGBEffectGL );	// a test
	rc->m_effects->addEffect( new RenderPointsEffectGL );		// a test, better placed elsewhere
	rc->m_effects->addEffect( new RenderPlaneEffectGL );
	rc->m_effects->addEffect( new RenderLightingEffectGL );
	rc->m_effects->addEffect( new RenderClipEffectGL );
	rc->m_effects->addEffect( new RenderLayersEffectGL );
	return rc;
}
#endif

/*****************************************************************************/
/**
* @brief
* @return RenderContext *
*/
/*****************************************************************************/
RenderContext * RenderContextManager::createDirect3DContext() 
{
	throw("Not implemented yet");
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @return RenderContext	*
*/
/*****************************************************************************/
RenderContext	* RenderContextManager::getDirect3DContext( ContextID cid )
{
	throw("Not implemented yet");
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @return RenderEffectsManager	*
*/
/*****************************************************************************/
RenderEffectsManager		*RenderContext::effectsMan()
{
	return m_effects;
}
/*****************************************************************************/
/**
* @brief
* @return RenderEffectsManager	*
*/
/*****************************************************************************/
const RenderEffectsManager	*RenderContext::effectsMan() const
{
	return m_effects;
}
/*****************************************************************************/
/**
* @brief
* @return RenderContext::ContextID
*/
/*****************************************************************************/
ContextID	RenderContext::contextID() const
{
	return m_contextId;
}

/*****************************************************************************/
/**
* @brief
* @return pointsengine::RenderEnvironment
*/
/*****************************************************************************/
RenderEnvironment			RenderContext::environment() const
{
	return m_environment;
}


/*****************************************************************************/
/**
* @brief
* @param cid
* @return RenderContext	*
*/
/*****************************************************************************/
RenderContext	* RenderContextManager::getOtherContext( ContextID cid )
{
	typedef std::map<ContextID, RenderContext*> ContextMap;

	static ContextMap	contexts;

	// instance per thread is ok, threads cannot share GL contexts //TODO: Review here
	ContextMap::iterator i = contexts.find( cid );

	if (i == contexts.end())
	{
		RenderContext* rc = createOtherContext( cid );
		contexts.insert( ContextMap::value_type( cid, rc) );
		return rc;
	}
	else
	{
		return i->second;
	}	
}

/*****************************************************************************/
/**
* @brief
* @return RenderContext	*
*/
/*****************************************************************************/
RenderContext	* RenderContextManager::createOtherContext( ContextID cid )
{
	RenderContext *rc = new RenderContext(RenderEnvOther);
	
	rc->m_settings = new RenderSettings;
	rc->m_settings->setToDefault();

	rc->m_resources = 0;
	rc->m_renderer = new PointsRenderer( new SoftwarePipeline );	//needed for colour constants
	rc->m_contextId = cid;	
	
	return rc;
}



