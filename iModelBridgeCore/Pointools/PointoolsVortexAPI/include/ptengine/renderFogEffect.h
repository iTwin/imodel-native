#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{


class RenderFogEffect : public RenderEffectI
{
public:
	BufferValueType	valueType() const								{ return Graphic_None; };

	const char*			shaderDefine() const							{ return "FOG"; }
	uint				shaderUniforms() const							{ return 0; }
	void				setEffectShaderUniforms( ptgl::Shader *shader )	{};

	void				startFrame( const RenderContext *context )	{};
	void				endframe( const RenderContext *context )	{};
};

}