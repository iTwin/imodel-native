#include <ptengine/renderLayersEffect.h>
#include <ptengine/renderShaderUniforms.h>
#include <ptengine/pointLayers.h>
#include <ptgl/glShader.h>
#include <ptengine/engine.h>

using namespace pointsengine;

RenderLayersEffectGL::RenderLayersEffectGL()
{
	m_attribPos = 1;
}
//-----------------------------------------------------------------------------
bool	RenderLayersEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	return true; 
}
//-----------------------------------------------------------------------------
bool	RenderLayersEffectGL::isEnabled( const RenderSettings *settings ) const
{
	return true;
}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::startFixedFuncFrame( const RenderContext *context )
{

}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::endFixedFuncFrame( const RenderContext *context )
{

}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::startShaderBuffer( const RenderContext *context, ShaderObj *shader )
{
	const PointsBufferI *buffer = context->renderer()->activeBuffer();
	const void *layersBuffer = buffer->getBufferPtr(pointsengine::Buffer_Layers);

	// not ATI attrib pos 1 may not be accepted
	if (layersBuffer)
	{
		ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);
		
		// some hacking around here, because layers effect pushs through an attribute channel
		// which is not possible in ffp GL and not handled by the framework, unlike intensity
		// values for example
		glEnableVertexAttribArray( m_attribPos );
		shaderGL->setVertexAttribPointer( m_attribPos, 1, GL_UNSIGNED_BYTE, GL_FALSE, 0, layersBuffer ); 
	}	
}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::endShaderBuffer( const RenderContext *context, ShaderObj *shader )
{

}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::startShaderFrame( const RenderContext *context, ShaderObj *shader )
{
	// set up uniforms
	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);
	shaderGL->setUniform1i( UNIFORM_LAYER_TEX, TEX_UNIT_LAYER );

	// set up texture
	glActiveTexture( GL_TEXTURE0_ARB+TEX_UNIT_LAYER );
	GLuint texID = generateLayerTexture( context, false );	
	glBindTexture( GL_TEXTURE_2D, texID );
	
	// bind attrib array then get actual bound index, not guaranteed to be as requested
	glBindAttribLocationARB( shaderGL->GetProgramObject(), 1, "pE" );
	m_attribPos = glGetAttribLocation( shaderGL->GetProgramObject(), "pE");

	// if we have a layer channel we need zero blend
	float layer_col_gl [] = { 1.0,1.0,1.0,0 };
	
	shaderGL->setUniform4fv( UNIFORM_LAYER_COL, 1, layer_col_gl );
	//shaderGL->setUniform1f( UNIFORM_LAYER_ALPHA, 0 );
}
//-----------------------------------------------------------------------------
void	RenderLayersEffectGL::endShaderFrame( const RenderContext *context, ShaderObj *shader )
{
	glActiveTexture( GL_TEXTURE0_ARB+TEX_UNIT_LAYER );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glDisableVertexAttribArray( m_attribPos );
}
//-----------------------------------------------------------------------------
uint	RenderLayersEffectGL::requiredStandardUniforms() const
{ 
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @param availableBuffers
* @return bool
*/
/*****************************************************************************/
uint RenderLayersEffectGL::requiredBuffers() const
{
	return Buffer_Layers;
}
//-----------------------------------------------------------------------------
// generates a texture that indicates the visibility of the 128 combinations of
// 7 layers 
//-----------------------------------------------------------------------------
GLuint	RenderLayersEffectGL::generateLayerTexture( const RenderContext *context, bool forceUpdate )
{
	static uint last_state_id = 0;

	pointsengine::ContextID contextId = context->contextID();
	ubyte					visLayers = thePointLayersState().pointVisLayerMask();
	uint					state_hash = thePointLayersState().stateHash();

	TextureMap::iterator	it = m_layerTextures.find( contextId );
	GLuint					texID = it == m_layerTextures.end() ? 0 : it->second.first;
	ubyte					texLayers = it == m_layerTextures.end() ? 0 : it->second.second;


	// if the layer setup state has changed this invalidates all layer textures ie. for all contexts
	if (last_state_id != thePointLayersState().stateHash())
	{
		last_state_id = thePointLayersState().stateHash();
		texLayers = 0;

		// change all the tex layer set-ups to force regeneration of texture
		for ( it=m_layerTextures.begin(); it!=m_layerTextures.end();it++)
		{
			it->second.second = 0;
		}
	}

	// check if texture is already available and up to date
	if (texID && !forceUpdate && visLayers == texLayers) 
		return texID;	
	
	ubyte *layerTexture = 0;

	try
	{
		layerTexture = new ubyte[ 4 * 256 * 256 ];

	}
	catch(std::bad_alloc) 
	{ 
		return 0;
	}
	
	glActiveTexture( GL_TEXTURE0_ARB+TEX_UNIT_LAYER );
	
	// delete existing texture
	if ( texID )
	{	
		glDeleteTextures(1, &texID );
	}
	// clear the texture
	memset( layerTexture, 0, 4 * 256 * 256 );

	uint i, j;
	for ( i=1; i<256; i++)
	{
		uint i4 = i * 4;

		for ( j=0; j<256; j++)	//set whole column to 255, in future rows could be used for attributes
		{
			uint j4 = j * 4;

			if (i & visLayers)
			{
				layerTexture[j4 * 256 + i4] = 255;		//non-selected point
				layerTexture[j4 * 256 + i4 +1] = 255;	//selected point
				layerTexture[j4 * 256 + i4 +2] = 255;	//selected point
			}
			layerTexture[j4 * 256 + i4 +3] = 255;		//selected point
				
		}
		ptgl::Color c = thePointLayersState().getLayerColor(i);

		// set up the layer colour
		if (i & visLayers)
		{
			layerTexture[4 * 256 + i4] = c.r*255;		//r
			layerTexture[4 * 256 + i4 +1] = c.g*255;	//g
			layerTexture[4 * 256 + i4 +2] = c.b*255;	//b
			layerTexture[4 * 256 + i4 +3] = c.a*255;
		}
	}

	// create the GL texture 
	glGenTextures(1, &texID );

	// store the ID
	if (it == m_layerTextures.end())
	{
		m_layerTextures.insert( 
			TextureMap::value_type( context->contextID(), std::pair<GLuint, uint>(texID, state_hash) ));
	}
	else 
	{
		it->second.first = texID;
		it->second.second = visLayers;
	}

	/* Bind the texture to the index and init the texture*/ 
	glBindTexture(GL_TEXTURE_2D, texID);

	/*texture quality		*/ 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, layerTexture);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}