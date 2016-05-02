#include "PointoolsVortexAPIInternal.h"
#include <gl/glew.h>
#include <ptgl/glShader.h>
#include <ptengine/renderContext.h>
#include <ptengine/renderShaderUniforms.h>
#include <ptengine/renderPipelineGLShader.h>
#include <ptengine/renderVoxelBuffer.h>
#include <ptengine/visibilityEngine.h>
#include <ptengine/engine.h>
#include <ptengine/pointLayers.h>
#include <ptengine/clipManager.h>
using namespace pointsengine;

//
// Shader Pipeline
//
// pass 1: General rendering
// pass 2: Point clouds with solid colour attribute set
//
void RenderPipeline_GLShader::setupShaderForFrame( const RenderContext *context, ptgl::Shader *shader, 
												  uint requiredUniforms )
{
	if (requiredUniforms & Uniform_EyeMatrixInverted)
	{
	// UNIFORM_INVERTED_EYE_MATRIX
		mmatrix4d Meye;
		mmatrix4 Meyef;
		Meye.loadGLmodel();
		Meye.invert();
		for (int i=0; i<16; i++) Meyef.data()[i] = Meye.data()[i];
		shader->setUniformMatrix4fv( UNIFORM_INVERTED_EYE_MATRIX, 1, GL_TRUE, Meyef.data());
	}
	if (requiredUniforms & Uniform_EyeMatrix)
	{
		// UNIFORM_EYE_MATRIX
		mmatrix4d Meye;
		mmatrix4 Meyef;
		Meye.loadGLmodel();
		for (int i=0; i<16; i++) Meyef.data()[i] = Meye.data()[i];
		shader->setUniformMatrix4fv( UNIFORM_EYE_MATRIX, 1, GL_FALSE, Meyef.data());

	}

	// SELECTION COL
	const ubyte *scol = context->settings()->selectionColour();
	GLfloat scol_f[] = { (float)scol[0]/255, (float)scol[1]/255, (float)scol[2]/255, 1.0f };
	shader->setUniform4fv( UNIFORM_SEL_COL, 1, scol_f );
}
/*****************************************************************************/
/**
* @brief
* @param buffer
* @param settings
* @param method
* @param effects
* @return void
*/
/*****************************************************************************/
void RenderPipeline_GLShader::setUpShaderForBuffer( const PointsBufferI *buffer, const RenderContext *context, 
												   ptgl::Shader *shader, uint requiredUniforms )
{
	const VoxelInPlaceBuffer *vbuffer = static_cast< const VoxelInPlaceBuffer* >( buffer );
	
	const pcloud::Voxel *voxel = vbuffer->voxel();

	if (voxel)
	{
		// QUANTIZE VALUES
		if (requiredUniforms & Uniform_Quantize_O)
		{
			const double *Qs = voxel->channel(pcloud::PCloud_Geometry)->scaler();
			const double *Qo = voxel->channel(pcloud::PCloud_Geometry)->offset();

			if (Qs && Qo)
			{
				float offset[] = { (float)Qo[0], (float)Qo[1], (float)Qo[2], 0 };
                float scaler[] = { (float)Qs[0], (float)Qs[1], (float)Qs[2], 1.0 };

				shader->setUniform4fv( UNIFORM_QUANTIZE_O, 1, offset );
				shader->setUniform4fv( UNIFORM_QUANTIZE_S, 1, scaler );
			}
		}
		if (requiredUniforms & Uniform_CloudToPrjMatrix)
		{
			// UNIFORM_REG_MATRIX
			const pcloud::PointCloud *pc = voxel->pointCloud();
			mmatrix4d mat;
			pc->registration().compileMatrix(mat);
			mat >>= pc->userTransformationMatrix();			
			
			GLfloat Mp[16];
			for (int i=0;i<16;i++) Mp[i] = (float)mat.data()[i];

			shader->setUniformMatrix4fv( UNIFORM_PRJ_MATRIX, 1, GL_FALSE, Mp );
		}
		if (requiredUniforms & Uniform_RegMatrix)
		{
			// UNIFORM_REG_MATRIX
			const pcloud::PointCloud *pc = voxel->pointCloud();
			mmatrix4d mat(pc->registration().matrix());
			
			GLfloat Mr[16];
			for (int i=0;i<16;i++) Mr[i] = (float)mat.data()[i];

			shader->setUniformMatrix4fv( UNIFORM_REG_MATRIX, 1, GL_FALSE, Mr );
		}
		
		// FULLY SELECTED FLAG
		shader->setUniform1f( UNIFORM_FULL_SEL, 
			voxel->flag( pcloud::WholeSelected ) ? 1.0 : 0 );

		// layer color when edit channel not available
		if (!voxel->channel(pcloud::PCloud_Filter))
		{
			ptgl::Color c = thePointLayersState().getLayerColor( voxel->layers(0) );

			shader->setUniform4fv( UNIFORM_LAYER_COL, 1, &c.r );
			//shader->setUniform1f( UNIFORM_LAYER_ALPHA, c.a );
		}
		// used for override colour if set
	}
}
/*****************************************************************************/
/**
* @brief		called before the start of frame to allow the renderer to
*				do any pre-frame setup. 
*/
/*****************************************************************************/
void	RenderPipeline_GLShader::initializeFrame( RenderContext *context )
{	
	/*
	if (context->settings()->clippingEnabled())
	{
		// clipping is enabled, use two passes, one with clipping
		// switched on in the shader and one with it switched off
		m_numRenderPasses = 2;		
	}
	*/
	m_numRenderPasses = 2; // pass 2 is for color overrides
}
/*****************************************************************************/
/**
* @brief		check if the passed voxel should be rendered given the current
*				render context and render pass. At the moment the only thing
*				we do an additional render pass for is clipped voxels that use
*				a different shader.
*/
/*****************************************************************************/
bool RenderPipeline_GLShader::renderOnThisPass( RenderContext* context, int renderPass, pcloud::Voxel* vox )
{
	if (!vox) 
		return false;

	if (vox->pointCloud()->isOverriderColorEnabled())
		return renderPass == OverridePass ? true : false;
	else 
		return renderPass == OverridePass ? false : true;
}
/*****************************************************************************/
/**
* @brief		notify pipeline of start of a frame. Between start and end frame 
*               there can be no context switch.
*				maybe better to enforce this by storing context on start frame
*               and removing it from the renderPoints parameter list
*/
/*****************************************************************************/
void RenderPipeline_GLShader::startFrame( RenderContext *context, int renderPass )
{
	m_lastShader = 0;
	m_avalBuffers = 0;
 
	// using multiple passes here for overrider color
	if (renderPass == OverridePass)
	{
		// turn off other effects
		// turn off color effects
		m_storeSettings = *context->settings();

		context->settings()->enableRGB(false);
		context->settings()->enableGeomShader(false);
		context->settings()->enableIntensity(false);
	}	
}
void RenderPipeline_GLShader::endFrame( RenderContext *context, int renderPass )
{
	if (m_lastShader)
	{
		m_lastShader->end();

		context->effectsMan()->endFrame( context, m_avalBuffers, (ShaderObj*)m_lastShader );
		m_lastShader = 0;
	}
	if (renderPass == OverridePass)
	{
		*context->settings() = m_storeSettings;
	}

    glUseProgram(0);

	m_lastShader = 0;
	m_avalBuffers = 0;	
}
/*****************************************************************************/
/**
* @brief		renders points in the buffer using context settings
*/
/*****************************************************************************/
void RenderPipeline_GLShader::renderPoints( PointsBufferI *buffer, RenderContext *context )
{
	uint reqUniforms=0;
	ptgl::Shader *shader = getShader( buffer, context, reqUniforms );
	
	if (!shader) return; //shader error

	// shader change
	if (shader != m_lastShader)
	{
		if (m_lastShader)
		{
			m_lastShader->end();
			context->effectsMan()->endFrame( context, m_avalBuffers, (ShaderObj*)m_lastShader );
		}

		shader->begin();
		setupShaderForFrame( context, shader, reqUniforms );
		m_avalBuffers = buffer->getAvailableBuffers();
		context->effectsMan()->startFrame( context, m_avalBuffers, (ShaderObj*)shader );
	}
	
	setUpShaderForBuffer( buffer, context, shader, reqUniforms );
	context->effectsMan()->startBuffer( context, m_avalBuffers, (ShaderObj*)shader );

	//glColor3fv( buffer->baseColor() );

	renderMethod()->renderPoints( buffer, context->settings() );

	context->effectsMan()->endBuffer( context, m_avalBuffers, (ShaderObj*)shader );

	m_lastShader = shader;
}

/*****************************************************************************/
/**
* @brief		returns hardware support status for pipeline, GLSL needs GL 2.0+
* @return		bool
*/
/*****************************************************************************/
bool RenderPipeline_GLShader::isSupportedOnPlatform() const
{
	glewInit();

	static bool isSupported = false;
	static bool isInit = false;

	if (!isInit)
	{
		isSupported = (GLEW_VERSION_2_0 == GL_TRUE) ? true : false;
		
		isSupported |= (GL_TRUE == glewGetExtension("GL_ARB_shader_objects"))
			? true : false;
		
		isInit = true;
	}
	return isSupported;
}
/*****************************************************************************/
/**
* @brief		Does this pipeline 
* @return		bool
*/
/*****************************************************************************/
bool RenderPipeline_GLShader::useAggregateBuffers( const RenderSettings *settings, 
												   const pcloud::Voxel *vox ) const
{
	return false;
}
/*****************************************************************************/
/**
* @brief
* @param buffer
* @param method
* @return void
*/
/*****************************************************************************/
void RenderPipeline_GLShader::renderSelPoints( PointsBufferI *buffer, RenderContext *context )
{
	static RenderSettings selectedSettings;
	selectedSettings = (*context->settings());
	selectedSettings.enableIntensity(false);
	selectedSettings.enableLighting(false);
	selectedSettings.enableRGB( false );
	selectedSettings.enableGeomShader( false );
	
	if (buffer->getNumPoints())								// no need for shader stuff
	{			
		glColor3ubv( context->settings()->selectionColour() );			// render in solid selection colour
		renderMethod()->renderPoints( buffer, &selectedSettings );
	}
}

/*****************************************************************************/
/**
* @brief		Pipeline constructor
* @param method	The render method to use
* @return 
*/
/*****************************************************************************/
RenderPipeline_GLShader::RenderPipeline_GLShader( RenderMethodI *method ) : RenderPipelineI(method)
{	
	m_numRenderPasses = 1;
}
RenderPipeline_GLShader::~RenderPipeline_GLShader()
{
	ShaderMapType::iterator i = m_shaders.begin();

	while (i != m_shaders.end())
	{
		delete i->second->shader;	//ptgl::Shader
		delete i->second;			//ShaderInfo;
		++i;
	}	
}
/*****************************************************************************/
/**
* @brief				creates or fetches cached shader according to settings
* @param settings
* @return ptgl::Shader *
*/
/*****************************************************************************/
ptgl::Shader * RenderPipeline_GLShader::getShader( const PointsBufferI *buffer, const RenderContext *context, uint &requiredUniforms )
{
	// try to find previously created shader
	uint hash = hashSettings( context->settings(), buffer );

	ShaderMapType::iterator shaderIterator = m_shaders.find( hash );
	if (shaderIterator != m_shaders.end())
	{
		requiredUniforms = shaderIterator->second->uniforms;
		return shaderIterator->second->shader;
	}
	
	// not found so create shader
	int numEffects = context->effectsMan()->numOfEffects();
	requiredUniforms = 0;

	pt::String preprocessor_defines;
	
	for (int i=0; i<numEffects;i++)	// get pp defines, this is used to turn parts of the shader script on and off
	{
		const RenderEffectI *effect = context->effectsMan()->renderEffect(i);
		const char *preprocessor = effect->shaderDefine();
		uint reqBuffers = effect->requiredBuffers();

		if (effect->isEnabled( context->settings() ) &&		//is switched on
			((hash & reqBuffers) == reqBuffers)  && //has required buffers available
			preprocessor)	//has a define
		{
			// this is used in 2nd pass - its ugly here
			if (context->effectsMan()->areColorEffectsDisabled())
			{
				if (effect->affectsColor()) continue;
			}
			requiredUniforms |= effect->requiredStandardUniforms();
			preprocessor_defines += pt::String(preprocessor) +  pt::String("\n");
		}
	}

	// create shader obj
	ResourceAdaptorI *res = const_cast<ResourceAdaptorI*>( context->resources() );
	ptgl::Shader *shader = reinterpret_cast<ptgl::Shader*>(res->createShaderObj( "pshader", preprocessor_defines.c_str() ));
	
	if (shader)						//should not fail unless error in script or unsupported on hardware / driver
	{
		ShaderInfo *shaderInfo = new ShaderInfo;
		shaderInfo->shader = shader;
		shaderInfo->uniforms = requiredUniforms;
		m_shaders.insert( ShaderMapType::value_type( hash, shaderInfo ) );	// insert into map
	}
	return shader;
}

/*****************************************************************************/
/**
* @brief			creates a hash value for the current settings, used in map
* @param settings
* @return uint
*/
/*****************************************************************************/
uint RenderPipeline_GLShader::hashSettings( const RenderSettings *settings, const PointsBufferI *buffer ) const
{
	uint hash = 0;
	hash |= (settings->isRGBEnabled() && buffer->getBufferPtr( Buffer_RGB )) ? Buffer_RGB : 0;
	hash |= (settings->isIntensityEnabled() && buffer->getBufferPtr( Buffer_TexCoord0 )) ? Buffer_TexCoord0 : 0;
	hash |= (settings->isLightingEnabled() && buffer->getBufferPtr( Buffer_Normal )) ? Buffer_Normal : 0;
	hash |= (settings->isGeomShaderEnabled()) ? Buffer_Pos : 0;
	hash |= (buffer->getBufferPtr( Buffer_Layers )) ? Buffer_Layers : 0;

	if (settings->isGeomShaderEnabled())
	{
		hash |= 256 << (int)settings->geomShaderEdge();
	}
	if (ClipManager::instance().clippingEnabled()) hash |= 2048;

	return hash;

	// other settings will be added later
	// fog
	// lighting settings
	// edge condition handling
}