#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{
	class RenderPointsEffectGL : public RenderEffectI
	{
	public:
		bool				compatibleEnvironment( RenderEnvironment e ) const;

		uint				requiredBuffers() const;
		bool				isEnabled( const RenderSettings *settings ) const	{ return true; }

		const char*			shaderDefine() const								{ return 0; }
		uint				shaderUniforms() const								{ return 0; }

		/* fixed pipeline implementation */ 
		void				startFixedFuncFrame( const RenderContext *context );
		void				endFixedFuncFrame( const RenderContext *context );

		/* GLSL based implementation */ 
		void				startShaderFrame( const RenderContext *context, ShaderObj *shader );
		void				endShaderFrame( const RenderContext *context, ShaderObj *shader );
	};

}