#pragma once

#include <ptengine/pointsRenderer.h>
#include <ptengine/renderResourceManager.h>
#include <ptengine/colourRamps.h>
#include <ptengine/renderEnvironment.h>

namespace pointsengine
{
	typedef		__int64				ContextID;

	class RenderContext
	{
		RenderContext( RenderEnvironment env );
		virtual ~RenderContext();

		friend class RenderContextManager;

	public:		

		PointsRenderer				*renderer();
		const PointsRenderer		*renderer() const;

		RenderSettings				*settings();
		const RenderSettings		*settings() const;

		const ResourceAdaptorI		*resources() const;
		
		RenderEffectsManager		*effectsMan();
		const RenderEffectsManager	*effectsMan() const;

		RenderEnvironment			environment() const;
		ContextID					contextID() const;


	private:
		
		ResourceAdaptorI			*m_resources;
		RenderSettings				*m_settings;
		PointsRenderer				*m_renderer;
		RenderEffectsManager		*m_effects;

		ContextID					m_contextId;
		RenderEnvironment			m_environment;
	};


	class RenderContextManager
	{
		RenderContextManager();
		~RenderContextManager();

	public:
		static RenderContextManager *instance();

		RenderContext	*getRenderContext( RenderEnvironment env, ContextID context=0 );
		bool			destroyRenderContext( ContextID context );

#ifdef HAVE_OPENGL
        RenderContext	*getOpenGLContext( ContextID cid, bool forceFixedFunc=false );
#endif
		RenderContext	*getDirect3DContext( ContextID cid );
		RenderContext	*getOtherContext( ContextID cid );

	private:
#ifdef HAVE_OPENGL
        RenderContext	*createOpenGLContext( bool forceFixedFunc=false );
#endif
		RenderContext	*createDirect3DContext();
		RenderContext	*createOtherContext( ContextID cid );

		typedef std::map<ContextID, RenderContext*> ContextMap;
		ContextMap	m_contexts;
	};
}