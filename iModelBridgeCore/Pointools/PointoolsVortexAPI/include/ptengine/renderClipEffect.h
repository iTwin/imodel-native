#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{

class RenderClipEffectGL : public RenderEffectI
{
public:
	RenderClipEffectGL() : isCapable(-1)								{}
	BufferValueType		valueType() const								{ return Buffer_Pos; };

	bool				compatibleEnvironment( RenderEnvironment e ) const;
	bool				isEnabled( const RenderSettings *settings ) const;
	uint				requiredBuffers() const							{ return 0; }

	/* shader setup */ 
	const char*			shaderDefine() const							{ return "#define PT_C"; }

	/* fixed pipeline implementation */ 
	void				startFixedFuncFrame( const RenderContext *context );
	void				endFixedFuncFrame( const RenderContext *context );

	/* GLSL based implementation */ 
	void				startShaderFrame( const RenderContext *context, ShaderObj *shader  );
	void				endShaderFrame( const RenderContext *context, ShaderObj *shader  );
	uint				requiredStandardUniforms()const;

private:

	int					isCapable;
};
}