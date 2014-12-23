#pragma once

#include <ptengine/renderPointsBuffer.h>
#include <ptengine/renderEffect.h>

namespace pointsengine
{
/** Encapsulates API dependent methods for rendering points */
/**
 *  Pure abstract class with a single override to render the points. 
 *  The implementation are rendering API (GL/DX) dependent. RenderMethod is 
 *  created by the RenderPipelineFactory only, hence the private empty constructor
 */

class RenderMethodI
{
public:
	virtual void	renderPoints( PointsBufferI *buffer, const RenderSettings *settings )=0;
protected:
	RenderMethodI()	{}
};


/** NullMethod is for error handling only */
/**
 *  Enables graceful handling of no specified method - will assert in debug
 *  , nothing is rendered in release.
 */
class NullMethod : public RenderMethodI
{
public:
	virtual void renderPoints( PointsBufferI *buffer, const RenderSettings *settings );
};

/** Encapsulates API dependent pipeline for rendering points */
class RenderPipelineI
{
public:
	RenderPipelineI( RenderMethodI *method );
	virtual			~RenderPipelineI();

	virtual void	setMethod( RenderMethodI *method );

	// abstracts
	virtual void	renderPoints( PointsBufferI *buffer, RenderContext *context )=0;
	virtual void	renderSelPoints( PointsBufferI *buffer, RenderContext *context )=0;

	virtual bool	isSupportedOnPlatform() const=0;
	virtual	bool	useAggregateBuffers( const RenderSettings *settings, const pcloud::Voxel *voxel ) const=0;

	virtual void	startFrame( RenderContext *context ){};
	virtual void	endFrame( RenderContext *context )	{};

protected:
	RenderMethodI	*renderMethod();

private:
	RenderMethodI	*m_method;
};

/** SoftwarePipeline for query based rendering */
class SoftwarePipeline : public RenderPipelineI
{
public:
	SoftwarePipeline();
	virtual			~SoftwarePipeline();

	// abstracts
	void	renderPoints( PointsBufferI *buffer, RenderContext *context ){};
	void	renderSelPoints( PointsBufferI *buffer, RenderContext *context ){};
	bool	isSupportedOnPlatform() const { return true; };
	bool	useAggregateBuffers( const RenderSettings *settings, const pcloud::Voxel *voxel ) const;
};

}