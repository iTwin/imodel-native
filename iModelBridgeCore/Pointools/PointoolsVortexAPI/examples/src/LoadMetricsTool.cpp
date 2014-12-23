#include "LoadMetricsTool.h"

//-----------------------------------------------------------------------------
LoadMetrics::LoadMetrics() : Tool(0, 0)
//-----------------------------------------------------------------------------
{
	m_pointsLoadedSinceLastDraw = 0;
	m_pointsShortFall = 0;
}
//-----------------------------------------------------------------------------
void LoadMetrics::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	drawGraph();
}
//-----------------------------------------------------------------------------
void LoadMetrics::drawGraph()
//-----------------------------------------------------------------------------
{

}
//-----------------------------------------------------------------------------
void LoadMetrics::onIdle()
//-----------------------------------------------------------------------------
{
	int last_load = m_pointsLoadedSinceLastDraw;
	int last_shortfall = m_pointsShortFall;

	m_pointsLoadedSinceLastDraw = 
		ptPtsLoadedInViewportSinceDraw(0);
	
	m_pointsShortFall = ptPtsToLoadInViewport(0, true);

	if (last_load != m_pointsLoadedSinceLastDraw)
		std::cout << "Loaded = " << m_pointsLoadedSinceLastDraw << std::endl;
	
	if (last_shortfall != m_pointsShortFall)
		std::cout << "Shortfall = " << m_pointsShortFall << std::endl;

	m_shortFallGraph.push_back(Metric(m_pointsShortFall));
	m_loadedGraph.push_back(Metric(m_pointsLoadedSinceLastDraw));
}