#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{


class RenderIntensityEffectGL : public RenderEffectI
{
public:
	bool				compatibleEnvironment( RenderEnvironment e ) const;
	uint				requiredBuffers() const;
	bool				isEnabled( const RenderSettings *settings ) const;

	/* fixed pipeline implementation */ 
	void				startFixedFuncFrame( const RenderContext *context );
	void				endFixedFuncFrame( const RenderContext *context );

	/* GLSL based implementation */ 
	void				startShaderFrame( const RenderContext *context, ShaderObj *shader );
	void				endShaderFrame( const RenderContext *context, ShaderObj *shader );
	const char*			shaderDefine() const	{ return "#define PT_I"; }

private:
	typedef std::map <int, Texture1D*>	TextureMap;
	TextureMap							m_gradients;
};

}