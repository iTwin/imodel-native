#pragma once

#include <pt/typedefs.h>

namespace pointsengine
{
	class RenderFrameData
	{
	public:

		RenderFrameData();

		int		frameDuration() const				{ return m_msFrameDuration; }
		void	frameDuration(int val)				{ m_msFrameDuration = val; }

		float	frameDensity() const				{ return m_frameDensity; }
		void	frameDensity(float val)				{ m_frameDensity = val; }

		int		lastFrameTime() const				{ return m_msLastFrameTime; }
		void	lastFrameTime(int val)				{ m_msLastFrameTime = val; }

		void	lastFramePointsRendered( int pnts )	{ m_ptsLastFrame = pnts; }
		uint	lastFramePointsRendered() const		{ return m_ptsLastFrame; }

		uint	lastFramePtsPerSecond() const;

		void	incAggregatedNodeCount()			{ ++m_numAggregatedNodes; }
		void	incDirectNodeCount()				{ ++m_numDirectNodes; }

		void	clearFrameStats();

		void	incFrameCounter();
		int		frameCounter() const				{ return m_frameCounter; }

		uint	averagePtsPerSecond() const;

		bool	isDynamic() const					{ return m_isDynamic; }
		void	isDynamic(bool val)					{ m_isDynamic = val; }

	private:
		//	framerate control
		int		m_msFrameDuration;			// target frame draw time
		float	m_frameDensity;				// density reduction to maintain framerate  

		//	analytics
		int		m_msLastFrameTime;		
		uint	m_ptsLastFrame;				// total num points rendered in lat frame

		//  buffer stats
		int		m_numAggregatedNodes;
		int		m_numDirectNodes;

		//  average
		int		m_frameCounter;
		int		m_msTotalFrameTime;
		int64_t	m_ptsTotal;

		bool	m_isDynamic;
	};
}