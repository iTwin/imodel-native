#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{
	class RenderLightingEffectGL : public RenderEffectI
	{
	public:
		bool				compatibleEnvironment( RenderEnvironment e ) const;

		uint				requiredBuffers() const;
		bool				isEnabled( const RenderSettings *settings ) const;

		const char*			shaderDefine() const								{ return "#define PT_L"; }
		uint				shaderUniforms() const								{ return 0; }
		void				setEffectShaderUniforms( const RenderContext *context, 
													ShaderObj *shader ) const;

		/* fixed pipeline implementation */ 
		void				startFixedFuncFrame( const RenderContext *context );
		void				endFixedFuncFrame( const RenderContext *context );

		/* GLSL based implementation */ 
		void				startShaderFrame( const RenderContext *context, ShaderObj *shader );
		void				endShaderFrame( const RenderContext *context, ShaderObj *shader );
		uint				requiredStandardUniforms() const;
	};

}