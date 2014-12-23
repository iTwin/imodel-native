/******************************************************************************

Pointools Vortex API Examples

LoadMetricsTool.h

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_LOAD_METRICS_TOOL_H
#define POINTOOLS_EXAMPLE_APP_LOAD_METRICS_TOOL_H

#include "VortexExampleApp.h"

#include <vector>
#include <time.h>

class LoadMetrics : public Tool
{
public:
	LoadMetrics();

	void drawPostDisplay();
	void onIdle();

	void drawGraph();

private:
	struct Metric
	{
		Metric(int val) : value(val) { time(&timestamp); }
		int		value;
		time_t	timestamp;
	};
	int	m_pointsLoadedSinceLastDraw;
	int m_pointsShortFall;
	
	std::vector<Metric>	m_shortFallGraph;
	std::vector<Metric>	m_loadedGraph;
};

#endif

