#pragma once

#ifdef HAVE_OPENGL
#include <ptengine/renderEffect.h>
#include <ptengine/renderContext.h>

namespace pointsengine
{

class RenderLayersEffectGL : public RenderEffectI
{
public:
	RenderLayersEffectGL();
	BufferValueType	valueType() const								{ return Buffer_Layers; };

	bool			compatibleEnvironment( RenderEnvironment e ) const;
	bool			isEnabled( const RenderSettings *settings ) const;
	uint			requiredBuffers() const;					

	const char*		shaderDefine() const							{ return "#define PT_E"; }

	/* fixed pipeline implementation */ 
	void			startFixedFuncFrame( const RenderContext *context );
	void			endFixedFuncFrame( const RenderContext *context );

	/* GLSL based implementation */ 
	void			startShaderFrame( const RenderContext *context, ShaderObj *shader );
	void			endShaderFrame( const RenderContext *context, ShaderObj *shader );
	void			startShaderBuffer( const RenderContext *context, ShaderObj *shader );
	void			endShaderBuffer( const RenderContext *context, ShaderObj *shader );

	uint			requiredStandardUniforms() const;

	bool			affectsGeometry() const							{ return true; }
	bool			affectsColor() const							{ return false; }

private:
	GLuint			generateLayerTexture( const RenderContext *context, bool forceUpdate = false );

	typedef std::map <pointsengine::ContextID, std::pair<GLuint, uint> >		TextureMap;	// context, (texid, vislayers)

	TextureMap										m_layerTextures;

	ubyte											m_lastVisLayersMask;
	GLuint											m_attribPos;
};

}
#endif