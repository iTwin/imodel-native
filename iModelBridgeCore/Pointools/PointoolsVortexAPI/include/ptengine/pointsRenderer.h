#pragma once

#include <ptengine/pointsScene.h>

#include <ptengine/renderEffect.h>
#include <ptengine/renderSettings.h>
#include <ptengine/renderFrameData.h>
#include <ptengine/renderVoxelBuffer.h>
#include <ptengine/renderPointsMethod.h>
#include <ptengine/renderMethodGLVertexArray.h>
#include <ptengine/renderPipelineGLFixed.h>
#include <ptcloud2/gradient.h>

#include <Loki/AssocVector.h>

#include <pt/timestamp.h>

namespace pointsengine
{
/** Renders points to the current graphics context */
/**
 * Only the pipeline is transparently graphics API dependent. Renderer itself 
 * encapsulates logic for rendering via the pipeline
 */
class RenderContext;

class PointsRenderer
{
public:
	PointsRenderer( RenderPipelineI *pipeline );
	~PointsRenderer();

	void						renderPoints( RenderContext *context, const pcloud::Scene *scene, bool dynamic, bool updateonly );

	const RenderFrameData		&frameData() const;
	RenderFrameData				&frameData();

	void						disable();
	void						enable();
	bool						isEnabled();

	void						updateColourConstants( const RenderSettings *settings );
	void						computeActualColour(ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb, const ubyte *layers);
	void						computeActualColour(ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb, const ubyte *layers);
	const PointsBufferI			*activeBuffer() const;

private:
	void						filterVoxelList( bool dismissRendered, const pcloud::Scene *scene=0 );
	void						renderDiagnostics();;
	int							fillAndRenderPointsBuffers( RenderContext *context, bool dynamic, int renderPass );

	PointsBufferI				*determineBufferToUse( const RenderContext *context, const pcloud::Voxel *vox );
	int							numVoxelPointsToRender( const pcloud::Voxel * vox, float minOutput );

	void						computeDynamicFPS( const pt::TimeStamp &t, float fps );
	
	void						flushBuffers( RenderContext *context, bool fullBuffersOnly, bool reset );
	void						resetBuffers();

	void						renderEditStackDebug();

private:
	bool						m_enabled;

	//pipeline
	RenderPipelineI				*m_pipeline;		
	
	// point buffers
	PointsBufferI              *m_activeBuffer;
	SelectedPointsBuffer		m_selBuffer;
	
	typedef						Loki::AssocVector<uint, PointsBufferI*>  BufferMap;

	BufferMap					m_ptBuffers;
	BufferMap					m_vxBuffer;

	// frame analytics and targets
	RenderFrameData				m_frameData;

	// list of voxels
	PointsScene::VOXELSLIST		m_voxlist;
	int							m_voxlistState;

	//col computation
	RenderSettings				m_colSettings;
	float						m_colGeomzscale;
	float						m_colContrast;
	float						m_colBrightness;
	pt::Gradient				*m_colIntGrad;
	pt::Gradient				*m_colGeomGrad;
};

}
