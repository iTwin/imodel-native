#include <ptengine/renderFrameData.h>
#include <iostream>

using namespace pointsengine;

extern bool g_showDebugInfo;
/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
RenderFrameData::RenderFrameData()
{
	m_frameDensity = 0.5f;
	m_msFrameDuration = 80;
	m_msLastFrameTime = 80;
	m_ptsLastFrame = 0;
	m_frameCounter = 0;
}

/*****************************************************************************/
/**
* @brief
* @return uint
*/
/*****************************************************************************/
uint RenderFrameData::lastFramePtsPerSecond() const
{
	return m_msLastFrameTime > 0 ?
		(uint)(1000 * m_ptsLastFrame / m_msLastFrameTime )
		: 0;
}


/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void RenderFrameData::incFrameCounter()
{
	++m_frameCounter;
	m_msTotalFrameTime += m_msLastFrameTime;
	m_ptsTotal += m_ptsLastFrame;
}

/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void RenderFrameData::clearFrameStats()
{
	if (g_showDebugInfo)
	{
		std::cout << "Frame counter   : " << m_frameCounter << std::endl;
		std::cout << "Total frame time: " << m_msTotalFrameTime << std::endl;
		std::cout << "Total points    : " << m_ptsTotal << std::endl;
		std::cout << "Aggregated Nodes: " << m_numAggregatedNodes << std::endl;
		std::cout << "Direct Nodes    : " << m_numDirectNodes << std::endl;
	}
	m_frameCounter = 0;
	m_msTotalFrameTime = 0;
	m_ptsTotal = 0;
	m_numAggregatedNodes = 0;
	m_numDirectNodes = 0;
}

/*****************************************************************************/
/**
* @brief
* @return uint
*/
/*****************************************************************************/
uint	RenderFrameData::averagePtsPerSecond() const
{
	if (!m_msTotalFrameTime) return 0;			//avoid div by zero
	return (uint)(1000 * m_ptsTotal / m_msTotalFrameTime);
}
