#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{

class RenderPlaneEffectGL : public RenderEffectI
{
public:
	BufferValueType	valueType() const								{ return Buffer_RGB; };

	bool			compatibleEnvironment( RenderEnvironment e ) const;
	bool			isEnabled( const RenderSettings *settings ) const;
	uint			requiredBuffers() const							{ return 0; }

	/* fixed pipeline implementation */ 
	void			startFixedFuncFrame( const RenderContext *context );
	void			endFixedFuncFrame( const RenderContext *context );

	/* GLSL based implementation */ 
	const char*		shaderDefine() const;
	void			startShaderFrame( const RenderContext *context, ShaderObj *shader );
	void			endShaderFrame( const RenderContext *context, ShaderObj *shader );
	uint			requiredStandardUniforms() const;

private:
	typedef std::map <int, Texture1D*>	TextureMap;
	TextureMap							m_gradients;
	ShaderEdgeMode						m_edge;

};
}