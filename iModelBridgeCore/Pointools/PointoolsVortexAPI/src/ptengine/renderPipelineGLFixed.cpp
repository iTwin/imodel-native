#include <ptengine/renderPipelineGLFixed.h>
#include <ptengine/renderContext.h>
#include <ptengine/userChannels.h>

using namespace pointsengine;
#define AGGREGATION_THRESHOLD 1500

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
void RenderPipeline_GLFixed::renderPoints( PointsBufferI *buffer, RenderContext *context )
{
	context->effectsMan()->startFrame( context, buffer->getAvailableBuffers(), 0 );

	renderMethod()->renderPoints( buffer, context->settings() );

	context->effectsMan()->endFrame( context, buffer->getAvailableBuffers(), 0 );
}

/*****************************************************************************/
/**
* @brief
* @return bool
*/
/*****************************************************************************/
bool RenderPipeline_GLFixed::isSupportedOnPlatform() const
{
	return true;
}
/*****************************************************************************/
/**
* @brief		Does this pipeline 
* @return		bool
*/
/*****************************************************************************/
bool RenderPipeline_GLFixed::useAggregateBuffers( const RenderSettings *settings, 
												   const pcloud::Voxel *voxel  ) const
{
	if (	voxel->flag(pcloud::PartSelected)  ||
			voxel->flag(pcloud::PartHidden)    ||
			voxel->lodPointCount() < AGGREGATION_THRESHOLD ||
			settings->isLightingEnabled()	||
			settings->isGeomShaderEnabled() ||
			UserChannelManager::instance()->renderOffsetChannel() )
	{
		return true;
	}
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
void RenderPipeline_GLFixed::renderSelPoints( PointsBufferI *buffer, RenderContext *context )
{
	static RenderSettings selectedSettings;
	selectedSettings = (*context->settings());
	selectedSettings.enableIntensity(false);
	selectedSettings.enableLighting(false);
	selectedSettings.enableRGB( false );
	selectedSettings.enableGeomShader( false );

	if (buffer->getNumPoints())
	{
		glColor3ubv( context->settings()->selectionColour() );
		renderMethod()->renderPoints( buffer, &selectedSettings );
	}
}

RenderPipeline_GLFixed::RenderPipeline_GLFixed( RenderMethodI *method ) : RenderPipelineI(method)
{	

}