/********************************************************************
	pointsRenderer.cpp

	created:	2009/12/28
	created:	28:12:2009   22:23
	filename: 	c:\PointoolsSVN\PointoolsVortexAPI\src\ptengine\pointsRenderer.cpp
	file path:	c:\PointoolsSVN\PointoolsVortexAPI\src\ptengine
	file base:	pointsRenderer
	file ext:	cpp
	author:		Faraz Ravi
	
	purpose:	Refactored points renderer
*********************************************************************/

#include <ptengine/renderContext.h>
#include <ptengine/renderFrameData.h>
#include <ptengine/pointLayers.h>
#include <ptengine/colourRamps.h>
#include <ptengine/renderDiagnostics.h>
#include <ptengine/pointspager.h>
#include <ptengine/userChannels.h>
#include <ptengine/clipManager.h>

#include <ptengine/engine.h>

#include <ptgl/glstate.h>

using namespace pointsengine;
using namespace pt;

bool g_showDebugInfo = false;

#define AGGREGATION_THRESHOLD 1500		//less that this number of points get aggregated into a single buffer

/*****************************************************************************/
/**
* @brief	PointsRenderer constructor
* @return 
*/
/*****************************************************************************/
PointsRenderer::PointsRenderer( RenderPipelineI *pipeline )
{
	m_voxlistState = 0;
	m_enabled = true;
	m_pipeline = pipeline;

	//col computation
	m_colGeomzscale = 1.0f;
	m_colContrast = 0.5f;
	m_colBrightness = 1.0f;
	m_colIntGrad = 0;
	m_colGeomGrad = 0;
	m_activeBuffer = 0;

	if (getenv("POINTOOLSDEBUG") 
		&& strcmp(getenv("POINTOOLSDEBUG"), "41111")==0)
	{
		g_showDebugInfo = false;	// not for release
	}
}
/*****************************************************************************/
/**
* @brief	PointsRenderer Destructor
* @return 
*/
/*****************************************************************************/
PointsRenderer::~PointsRenderer()
{
	BufferMap::iterator it;
	for (it = m_ptBuffers.begin(); it != m_ptBuffers.end(); it++)
	{
		if (PointsBufferI* ptb = it->second)
			delete ptb;
	}
	m_ptBuffers.clear();
}
/*****************************************************************************/
/**
* @brief
* @param millisecs
* @param front
* @param scene
* @return void
*/
/*****************************************************************************/
void PointsRenderer::renderPoints( RenderContext *context, const pcloud::Scene *scene, bool dynamic, bool updateonly )
{
	TimeStamp	t0;
	t0.tick();								// start timer

	PointsScene::UseSceneVoxels voxelslock( m_voxlist, m_voxlistState );	// Get the scene voxels
	int voxelcount = m_voxlist.size();
	
	if (!voxelcount || !m_enabled)  return;		// early exit if nothing to render
	
	m_frameData.clearFrameStats();
	m_frameData.isDynamic(dynamic);

	ptgl::State::clear();

	filterVoxelList( false, scene );
	
	if (g_showDebugInfo) renderDiagnostics();					// debug output	

											// see how many passes are required to render the current scene, scenes with
											// clipped voxels for example may require more than one rendering pass	
	m_pipeline->initializeFrame(context);

	for (int pass = 0; pass < m_pipeline->getNumRenderPasses(); pass++)
	{									
		m_pipeline->startFrame( context, pass );

		fillAndRenderPointsBuffers( context, dynamic, pass );

		m_pipeline->endFrame( context, pass );			
	}

	// debug output
	//renderDiagnostics();

	if (dynamic) computeDynamicFPS( t0, context->settings()->framesPerSec() );
}

/*****************************************************************************/
/**
* @brief
* @param dismissRendered
* @return void
*/
/*****************************************************************************/
void PointsRenderer::filterVoxelList( bool dismissRendered, const pcloud::Scene *scene )
{
	PointsScene::VoxIterator i = m_voxlist.begin();
	PointsScene::VoxIterator vend = m_voxlist.end();

	ubyte layers = (ubyte)thePointLayersState().visibleBitMask();

	int count = 0;

	while (i != vend)
	{
		pcloud::Voxel *vox = *i;
		
		if ( (dismissRendered && vox->flag(pcloud::Rendered))		// this voxel has been processed already so dismiss
			|| (scene && vox->pointCloud()->scene() != scene)		// scene is specified
			||	vox->flag(pcloud::WholeClipped)						// clip box
			|| !vox->flag(pcloud::Visible)							// in frustum
			|| !vox->pointCloud()->displayInfo().visible()			
			|| !(vox->layers(0) & layers || vox->layers(1) & layers))
		{
			vox->flag(pcloud::RenderDismiss, true);
		}
		else
		{
			vox->flag(pcloud::RenderDismiss, false);
		}
		++i;
	}
}
/*****************************************************************************/
/**
* @brief	predicate function for OOC voxel sorting
* @param a
* @param b
* @return int
*/
/*****************************************************************************/
inline static int voxPriorityCmp(const pcloud::Voxel *a, const pcloud::Voxel *b)
{	
	if ( a->priority() == b->priority()) return (a > b ? 1 : 0);
	return a->priority() > b->priority() ? 1 : 0;
}
/*****************************************************************************/
/**
* @brief		fills the points buffer and sends to render
*/
/*****************************************************************************/
int PointsRenderer::fillAndRenderPointsBuffers( RenderContext *context, bool dynamic, int renderPass )
{	
	PointsScene::VOXELSLIST OOCvoxels;

	int totalPnts = 0;

	uint requiredBuffers = context->effectsMan()->requiredBuffers( context );

	//static int numVoxels = 0;
	//if (numVoxels != m_voxlist.size())
	//{
	//	numVoxels = m_voxlist.size();
	//	thePointsPager().pause();				//try to flush out requests so something is drawn
	//	thePointsPager().unpause();		
	//	thePointsPager().completeRequests();

	//	numVoxels = m_voxlist.size();
	//}
	
	int pass = 0;	// first pass is in-core, second is out-of-core

	for (pass = 0; pass < 2; pass++)
	{
		PointsScene::VoxIterator i = !pass ? m_voxlist.begin() : OOCvoxels.begin();
		PointsScene::VoxIterator vend = !pass ? m_voxlist.end() : OOCvoxels.end();

		if (pass)	// OOC pass
		{
		//	OOCvoxels.sort( voxPriorityCmp );	//doin't, breaks everything!
		}
		while (i != vend)
		{ 
			pcloud::Voxel *vox = *i;
			++i;

			// check this voxel against the current context and render pass
			if (!m_pipeline->renderOnThisPass(context, renderPass, vox))			
				continue;			

			if ( !vox->flag(pcloud::RenderDismiss) )
			{
				boost::mutex::scoped_lock lock(vox->mutex(), boost::try_to_lock);		// lock since
				if (!lock.owns_lock()) continue;

				if (!pass && vox->flag(pcloud::OutOfCore))	
				{
					OOCvoxels.push_back(vox);
					continue;						// must happen after lock, pager thread could change
				}

				int numRenderPts = numVoxelPointsToRender( vox, context->settings()->minDynamicOutput() );

				VoxelLoader loadPoints(vox, pass ? (float)numRenderPts/ vox->fullPointCount() : 0, false, false);	//OOC handling
				
				if (numRenderPts > vox->lodPointCount()) numRenderPts = vox->lodPointCount();			// mainly check for OOC

				if (dynamic) 
				{
					double d_size = m_frameData.frameDensity() < 1 ?									//apply dynamic reduction
						numRenderPts * m_frameData.frameDensity() : numRenderPts;						//to maintain framerate
					numRenderPts = d_size < numRenderPts ? (d_size > 0 ? d_size : 1) : numRenderPts;
				}			
				numRenderPts *= context->settings()->overallDensity();

				PointsBufferI *ptsBuffer = determineBufferToUse( context, vox );
				m_activeBuffer = ptsBuffer;	// for low-level access needed by some effects

				ptsBuffer->setRequiredBuffers( requiredBuffers );

				int ptsAdded = ptsBuffer->addPoints( vox, 0, numRenderPts, &m_selBuffer );
				totalPnts += ptsAdded;
				
				while ( ptsBuffer->isFull() )
				{
					flushBuffers( context, true, true );
				
					ptsAdded = ptsBuffer->addPoints( vox, ptsAdded, numRenderPts-ptsAdded, &m_selBuffer );
					totalPnts += ptsAdded;
					if (!ptsAdded) break;
				}
			}
		}
		// voxelInPlace Buffer will do nothing here - no need to worry about mutex
		flushBuffers( context, false, true );
		m_activeBuffer = 0;
	}

	m_frameData.lastFramePointsRendered( totalPnts );
	return totalPnts;
}

const PointsBufferI			*PointsRenderer::activeBuffer() const
{
	return m_activeBuffer;
}	
/*****************************************************************************/
/**
* @brief
* @param t
* @return void
*/
/*****************************************************************************/
void PointsRenderer::computeDynamicFPS( const pt::TimeStamp &t0, float fps )
{
	pt::TimeStamp t1;
	t1.tick();

	double ms = TimeStamp::delta_ms(t0,t1);
	float frameDensity = m_frameData.frameDensity();

	float mstarget = 1000.0f / fps;

	if (ms > 0.01)	 frameDensity *= (mstarget / ms);

	if (frameDensity > 1000.0) frameDensity = 1000.0;
	if (frameDensity < 0.005f) frameDensity = 0.005f;

	// targets for next frame
	m_frameData.frameDensity( frameDensity );
	m_frameData.frameDuration( mstarget );		

	// analytics
	m_frameData.lastFrameTime( (int)ms );
	m_frameData.incFrameCounter();

	float avPtsSec = m_frameData.averagePtsPerSecond() / 1e6;

	if (g_showDebugInfo)
	{
		std::cout << "fps = " << fps << " ms=" << ms << " density=" << frameDensity << std::endl;
		std::cout << "Last pts/sec " << m_frameData.lastFramePtsPerSecond() << "\t Av Pts/Sec " <<  avPtsSec << std::endl;
	}
}

/*****************************************************************************/
/**
* @brief			Computes and returns the number of points that can be safely 
					rendered in the voxel. The voxel should have its mutex locked. 
					Note that this is not based on visibility, but rather on the 
					data that is available in the voxel's data channels
* @param vox		The voxel
* @return int		Number of points to render
*/
/*****************************************************************************/
int PointsRenderer::numVoxelPointsToRender( const pcloud::Voxel * vox, float minOutput )
{
	if (vox->pointCloud()->displayInfo().visible())
	{
		if ( vox->flag( pcloud::WholeHidden ) ) return 0;
		if ( !vox->layers(0) && !vox->channel( pcloud::PCloud_Filter ) ) return 0;	// part layer without a channel to support

		int reqSize = vox->getRequestLOD() * vox->fullPointCount();
		int editedSize = vox->numPointsEdited();

		if (!editedSize) editedSize = reqSize;

		int renderSize = min(reqSize, editedSize);
		float renderProp = (float)renderSize / vox->fullPointCount();

		if ( !vox->flag( pcloud::OutOfCore ) ) // don't check min available if ooc 
		{
			if ( renderSize > 0 && renderProp < minOutput)
				renderSize = minOutput * vox->fullPointCount();

			if (!renderSize) return 0;

			/*no data check - must do sanity check on channels to ensure data is available */ 
			/* this is done post OOC load, so is still valid for OOC nodes */ 
			bool nodata = false;
			const pcloud::DataChannel *dc=0;
			uint channelbit = 1;

			for (int i=1;i<MAX_CHANNELS; i++) 
			{
				dc = vox->channel(i);
				if (dc)
				{
					if (dc->size() && dc->size() < renderSize) 
						renderSize = dc->size();
				}
				channelbit <<= 1;
			}
			if (!renderSize) return 0;
		}
		return renderSize;
	}
	return 0;
}


/*****************************************************************************/
/**
* @brief		determines the buffer to use based on capabilities and batch size
* @param vox
* @return PointsBufferI				*
*/
/*****************************************************************************/
PointsBufferI		*PointsRenderer::determineBufferToUse( const RenderContext *context, const pcloud::Voxel *vox )
{
	bool useAggregation = false;
	PointsBufferI* ptsBuffer = 0;

	RenderFrameData &fdata = const_cast<RenderContext*>(context)->renderer()->frameData();
	
	useAggregation = m_pipeline->useAggregateBuffers( context->settings(), vox );	

	/* hash the channels and get the buffer */ 
	uint buffer = Buffer_Pos;
	buffer |= (vox->channel( pcloud::PCloud_Intensity )) ? Buffer_TexCoord0 : 0;
	buffer |= (vox->channel( pcloud::PCloud_RGB )) ? Buffer_RGB : 0;
	buffer |= (vox->channel( pcloud::PCloud_Normal )) ? Buffer_Normal : 0;

	if (vox->flag( pcloud::WholeSelected ) && !vox->flag( pcloud::PartHidden ))
	{
		BufferMap::iterator i = m_ptBuffers.find( 0 );	// 0 == WholeSelected

		if (i == m_ptBuffers.end())
		{
			ptsBuffer = new SelectedVoxelPointsBuffer( context->settings()->selectionColour() );
			m_ptBuffers.insert( BufferMap::value_type( 0, ptsBuffer ) );
		}
		else ptsBuffer = i->second;		

		fdata.incDirectNodeCount();
	}
	else if (useAggregation)
	{
		BufferMap::iterator i = m_ptBuffers.find( buffer );

		if (i == m_ptBuffers.end())
		{
			ptsBuffer = new PointsBuffer;
			m_ptBuffers.insert( BufferMap::value_type( buffer, ptsBuffer) );
		}
		else ptsBuffer = i->second;
		
		// selection buffer may be used with PointsBuffer rendering
		// so make sure selection Col is updated
		m_selBuffer.setBaseColour( context->settings()->selectionColour() );

		fdata.incAggregatedNodeCount();
	}
	else
	{
		buffer |= (vox->channel( pcloud::PCloud_Filter )) ? Buffer_Layers : 0;
		BufferMap::iterator i = m_ptBuffers.find( 0x10000 | buffer );

		if (i == m_ptBuffers.end())
		{
			ptsBuffer = new VoxelInPlaceBuffer;
			m_ptBuffers.insert( BufferMap::value_type( buffer | 0x10000, ptsBuffer));
		}
		else
		{
			ptsBuffer = i->second;
		}
		fdata.incDirectNodeCount();
	}
	return ptsBuffer;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointsRenderer::resetBuffers()
{
	BufferMap::iterator i = m_ptBuffers.begin();

	while (i != m_ptBuffers.end())
	{
		i->second->reset();
		++i;
	}
}

/*****************************************************************************/
/**
* @brief
* @param context			The current rendering context
* @param fullBuffersOnly	Only flush full buffers
* @param reset				Reset buffer counters after rendering flush
*/
/*****************************************************************************/
void		PointsRenderer::flushBuffers( RenderContext *context, bool fullBuffersOnly, bool reset )
{
	BufferMap::iterator i = m_ptBuffers.begin();

	while (i != m_ptBuffers.end())
	{
		PointsBufferI *buffer = i->second;
		
		if (!fullBuffersOnly || buffer->isFull())
		{
			m_activeBuffer = buffer;
			m_pipeline->renderPoints( buffer, context );		
			if (reset) buffer->reset();
		}
		++i;
	}
	if (m_selBuffer.getNumPoints() && (!fullBuffersOnly || m_selBuffer.isFull()))
	{
		m_pipeline->renderSelPoints( &m_selBuffer, context );
		m_selBuffer.clear();
	}
}
/*****************************************************************************/
/**
* @brief
* @return const RenderFrameData		&
*/
/*****************************************************************************/
const RenderFrameData		& PointsRenderer::frameData() const
{
	return m_frameData;
}
/*****************************************************************************/
/**
* @brief
* @return const RenderFrameData		&
*/
/*****************************************************************************/
RenderFrameData		& PointsRenderer::frameData()
{
	return m_frameData;
}

/*****************************************************************************/
/**
* @brief
* @param settings
* @return void
*/
/*****************************************************************************/
void PointsRenderer::updateColourConstants( const RenderSettings *settings )
{
	m_colSettings = *settings;

	m_colGeomzscale = vector3d(m_colSettings.geomShaderParams()).length();

	if (fabs(m_colGeomzscale) < 1) m_colGeomzscale = 1.0;
	m_colGeomzscale = 1.0 / m_colGeomzscale;

	m_colContrast = m_colSettings.intensityBlack();
	m_colBrightness = m_colSettings.intensityWhite();;

	m_colContrast *= 10.0f;
	m_colBrightness = (m_colBrightness -0.5f) * 2.0f;
	m_colBrightness += 0.5f;

	ColourGradient *grad =
		RenderResourceManager::instance()->gradientManager()->
		getGradientByIndex( m_colSettings.intensityGradient() );

	m_colIntGrad = grad ? &(grad->m_gradient) : 0;
	
	grad = RenderResourceManager::instance()->gradientManager()->
		getGradientByIndex( m_colSettings.geomShaderGradient() );

	m_colGeomGrad = grad ? &(grad->m_gradient) : 0;
}


/*****************************************************************************/
/**
* @brief
* @param col
* @param pnt
* @param intensity
* @param rgb
* @return void
*/
/*****************************************************************************/
void PointsRenderer::computeActualColour(ubyte *col, const float *pnt, const short *intensity, const ubyte *rgb, const ubyte *layers)
{
	double dpnt[] = {pnt[0],pnt[1],pnt[2]};
	computeActualColour(col, dpnt, intensity, rgb, layers);
}

/*****************************************************************************/
/**
* @brief
* @param col
* @param pnt
* @param intensity
* @param rgb
* @return void
*/
/*****************************************************************************/
void PointsRenderer::computeActualColour(ubyte *col, const double *pnt, const short *intensity, const ubyte *rgb, const ubyte *layers)
{
	/* move into integer space from float gave 3x performance boost */ 
	int pcol [] = { 256, 256, 256 };
	int components = 0;

	bool doGeom			= (m_colSettings.isGeomShaderEnabled()) ? true : false;
	bool doIntensity	= intensity && (m_colSettings.isIntensityEnabled()) ? true : false;
	bool doRGB			= rgb && (m_colSettings.isRGBEnabled()) ? true : false;

	/*if (layers && *layers & 0x80) //not wporking as expected and selection color is applied by calling code
	{
		col[0] = m_colSettings.selectionColour()[0];
		col[1] = m_colSettings.selectionColour()[1];
		col[2] = m_colSettings.selectionColour()[2];

		return;
	}*/
	
	/* calculate colour contributions */ 
	if (doRGB)
	{
		if (!doIntensity && !doGeom) /* handle rgb quickly */ 
		{
			col[0] = rgb[0];
			col[1] = rgb[1];
			col[2] = rgb[2];
			return;
		}
		pcol[0] *= rgb[0];
		pcol[1] *= rgb[1];
		pcol[2] *= rgb[2];
		pcol[0] /= 256;
		pcol[1] /= 256;
		pcol[2] /= 256;

		++components;
	}
	if (doIntensity)
	{		
		const ubyte *img = m_colIntGrad->img();

		/* todo: move into integer space also for performance */ 
		double _in = (*intensity);
		_in /= 65536;
		_in *= m_colContrast;
		_in += m_colBrightness;

		/* clamp */ 
		if (_in > 1.0f) _in = 1.0f;
		else if (_in < 0) _in = 0;

		int in = (_in * (m_colIntGrad->imgWidth()-1));

		pcol[2] *= img[in*4];
		pcol[1] *= img[in*4+1];
		pcol[0] *= img[in*4+2];
		pcol[0] /= 256;
		pcol[1] /= 256;
		pcol[2] /= 256;
	}
	if (doGeom)
	{
		const ubyte *img = m_colGeomGrad->img();

		vector3 ps( m_colSettings.geomShaderParams());
		vector3 gz(pnt[0],pnt[1], pnt[2]);

		float z = gz.dot(ps) + m_colSettings.geomShaderParams()[3];
		bool isblack = false;

		/*edge behaviour*/ 
		if (z > 1.0 || z < 0)
		{
			switch (m_colSettings.geomShaderEdge())
			{
			case ClampEdge:
				if (z > 1.0f) z = 1.0f;
				else if (z < 0) z = 0;
				break;
			case BlackEdge:
				pcol[0] = pcol[1] = pcol[2] = 0;
				isblack = true;
				break;
			case MirroredRepeat:		//needs to be implemented correctly
				z = fmod(z, 1.0f);				
				while (z<0) z += 1.0f;
				break;

			default: 
				z = fmod(z, 1.0f);				
				while (z<0) z += 1.0f;
				break;

			}
		}
		if (!isblack)
		{
			int zp = (z * m_colGeomGrad->imgWidth());

			pcol[2] *= img[zp*4];
			pcol[1] *= img[zp*4+1];
			pcol[0] *= img[zp*4+2];
			pcol[0] /= 256;
			pcol[1] /= 256;
			pcol[2] /= 256;
		}
	}
	else if (!doIntensity && !doRGB)
	{
		col[0] = 255;
		col[1] = 255;
		col[2] = 255;
		return;
	}

	/* assign and clamp */ 
	col[0] = pcol[0];
	col[1] = pcol[1];
	col[2] = pcol[2];
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void PointsRenderer::renderDiagnostics()
{
	/* voxel edit / layers state */ 
	/*----------------------------------------------------------*/ 
	
	RenderVoxelDiagnosticInfo::beginVoxelEditStateRender();

	PointsScene::VoxIterator i = m_voxlist.begin();
	PointsScene::VoxIterator vend = m_voxlist.end();

	ubyte layers = (ubyte)thePointLayersState().visibleBitMask();

	while (i != vend)
	{
		pcloud::Voxel *vox = *i;

		if (!vox->flag( pcloud::RenderDismiss ))
		{
			RenderVoxelDiagnosticInfo::renderVoxelEditState( vox ) ;

			if (vox->flag( pcloud::PartHidden ))
			{
				vox->flag( pcloud::DebugShowGreen, true, false );
				RenderVoxelDiagnosticInfo::renderVoxelOutline( vox );
			}
			else
			{
				vox->flag( pcloud::DebugShowGreen, false, false );		
			}
		}
		++i;
	}	
	RenderVoxelDiagnosticInfo::endVoxelEditStateRender();
}
void	PointsRenderer::renderEditStackDebug()
{
	// to do :)

}
