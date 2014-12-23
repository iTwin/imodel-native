#include <ptgl/glState.h>
#include <ptgl/glShader.h>
#include <ptengine/renderContext.h>
#include <ptengine/renderIntensityEffect.h>
#include <ptengine/renderShaderUniforms.h>

using namespace pointsengine;

/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderIntensityEffectGL::startFixedFuncFrame( const RenderContext *context )
{
	const RenderSettings *settings = context->settings();
	const ResourceAdaptorI *res = context->resources();

	ptgl::State::enable( GL_TEXTURE_1D );	//note this is not enough because another unit might be active when this is flushed

	/* create texture if not already available */ 
	int gradient = settings->intensityGradient();

	TextureMap::iterator i = m_gradients.find(gradient);
	Texture1D* texture = 0;

	if ( i== m_gradients.end())		//not created in this context
	{
		texture = res->createGradientTexture( gradient );
		if (texture)
		{
			m_gradients.insert( TextureMap::value_type(gradient,texture) );
		}
	}
	else texture = i->second;

	if (texture)
	{
		glActiveTexture( GL_TEXTURE0 );							// texture unit 0 is used for intensity shader
		glBindTexture( GL_TEXTURE_1D, texture->m_texid );

		float white = settings->intensityWhite();
		float black = settings->intensityBlack();

		// handle texture edge
		switch( settings->intensityGradientEdge())
		{
		case RepeatEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			break;
		case ClampEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			break;
		case BlackEdge:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // opengl 1.3+
			break;
		case MirroredRepeat:
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // opengl 1.4+
			
		}
		/*texture matrix to render short values and brightness/contrast setting */ 
		glMatrixMode(GL_TEXTURE);
		//glPushMatrix();
		glLoadIdentity();

		float contrast = black;
		float brightness = white;
		contrast *= 10.0f;
		brightness = (brightness -0.5f) * 2.0f;
		brightness += 0.5f;

		/* brightness / contrast */ 
		glTranslated(brightness, 0, 0);
		glScaled(contrast, 0, 0);
		glScaled(1.0/65536, 0, 0);

		glMatrixMode(GL_MODELVIEW);
	}
	

	ptgl::State::enable( GL_TEXTURE_1D );
}
/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void	RenderIntensityEffectGL::endFixedFuncFrame( const RenderContext *context )
{
	ptgl::State::disable( GL_TEXTURE_1D );

	glActiveTexture( GL_TEXTURE0 );	//todo, check for multitexturing support
	glMatrixMode(GL_TEXTURE);
	//glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}
/*****************************************************************************/
void	RenderIntensityEffectGL::startShaderFrame( const RenderContext *context, ShaderObj *shader  )
{
	startFixedFuncFrame( context );

	ptgl::Shader *shaderGL = reinterpret_cast<ptgl::Shader*>(shader);

	if (shaderGL)
	{
		shaderGL->setUniform1i( UNIFORM_INTEN_TEX, TEX_UNIT_INTEN );
	}
}

/*****************************************************************************/
void	RenderIntensityEffectGL::endShaderFrame( const RenderContext *context, ShaderObj *shader  )
{
	endFixedFuncFrame( context );
}
/*****************************************************************************/
/**
* @brief
* @param e
* @return bool
*/
/*****************************************************************************/
bool RenderIntensityEffectGL::compatibleEnvironment( RenderEnvironment e ) const
{
	if (e == RenderEnvOpenGL) return true;
	return false;
}

/*****************************************************************************/
/**
* @brief
* @param settings
* @return bool
*/
/*****************************************************************************/
bool RenderIntensityEffectGL::isEnabled( const RenderSettings *settings ) const
{
	return settings->isIntensityEnabled();
}
/*****************************************************************************/
/**
* @brief
* @param availableBuffers
* @return bool
*/
/*****************************************************************************/
uint RenderIntensityEffectGL::requiredBuffers() const
{
	return Buffer_TexCoord0;
}