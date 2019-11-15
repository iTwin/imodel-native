/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ptengine/renderEffect.h>

namespace pointsengine
{


class RenderRGBEffectGL : public RenderEffectI
{
public:
	bool				compatibleEnvironment( RenderEnvironment e ) const;
	uint				requiredBuffers() const;
	bool				isEnabled( const RenderSettings *settings ) const;

	const char*			shaderDefine() const								{ return "#define PT_RGB"; }
	uint				shaderUniforms() const								{ return 0; }

	/* fixed pipeline implementation */ 
	void				startFixedFuncFrame( const RenderContext *context );
	void				endFixedFuncFrame( const RenderContext *context );

	/* GLSL based implementation */ 
	void				startShaderFrame( const RenderContext *context, ShaderObj *shader )	{};
	void				endShaderFrame( const RenderContext *context, ShaderObj *shader )	{};
};

}
