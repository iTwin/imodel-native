#include <ptengine/renderPointsMethod.h>

using namespace pointsengine;

/*****************************************************************************/
SoftwarePipeline::SoftwarePipeline() : RenderPipelineI( new NullMethod )
{

}
/*****************************************************************************/
SoftwarePipeline::~SoftwarePipeline()
{

}

bool	SoftwarePipeline::useAggregateBuffers( const RenderSettings *settings, const pcloud::Voxel *voxel ) const
{
	return false;
}
/*****************************************************************************/
/**
* @brief
* @param method
* @return 
*/
/*****************************************************************************/
RenderPipelineI::RenderPipelineI( RenderMethodI *method )
{
	static NullMethod nullMethod;
	if (!method) method = &nullMethod;	// gracefully handle null pointer
	
	setMethod( method );
}
/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
RenderPipelineI::~RenderPipelineI()
{
}

/*****************************************************************************/
/**
* @brief			sets the rendering method with which points are rendered
* @param method
*/
/*****************************************************************************/
void RenderPipelineI::setMethod( RenderMethodI *method )
{
	m_method = method;
}
/*****************************************************************************/
/**
* @brief
* @return RenderMethodI	*
*/
/*****************************************************************************/
RenderMethodI	* RenderPipelineI::renderMethod()	//protected
{
	return m_method;
}

/*****************************************************************************/
/**
* @brief
* @param buffer
* @param settings
* @return void
*/
/*****************************************************************************/
void NullMethod::renderPoints( PointsBufferI *buffer, const RenderSettings *context)
{
	assert(0); //null method, you should use setMethod on pipeline to set a rendering method
}