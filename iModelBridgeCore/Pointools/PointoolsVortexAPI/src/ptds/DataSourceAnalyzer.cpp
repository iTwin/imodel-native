#include "PointoolsVortexAPIInternal.h"
#include <ptds/GraphManager.h>

#include <ptds/DataSourceAnalyzer.h>
#include <ptengine/StreamHost.h>


#include <iostream>
#include <fstream>

#define DATA_SOURCE_ONE_SECOND_MILLISECONDS					1000
#define DATA_SOURCE_ANALYZER_DEFAULT_DATA_SIZE				1024 * 256	// Default data size to stream while initializing early stages of analysis
#define DEFAULT_DATA_SOURCE_ANALYZER_MIN_LATENCY_ENTRIES	5			// Number of latency entries to monitor
#define DATA_SOURCE_ANALYZER_LATENCY_MEAN_MIN_PERIOD_COEF	6.5			// Minimum data size / latency ratio (approx 15%)
#define	DATA_SOURCE_ANALYZER_MIN_PERIOD						0.5			// Shortest period permitted to prevent high frequency calls to server
#define DATA_SOURCE_ANALYZER_LATENCY_SAMPLE_PERIOD			5			// Sample latency every few seconds
#define DATA_SOURCE_ANALYZER_NUM_LATENCY_SAMPLES			3			// Number of latency samples to include in moving average

namespace ptds
{

class DataSourceAnalyzerSeries : public Series<float, 2>	{};

pt::SimpleTimer	DataSourceAnalyzer::timeGlobal;

bool DataSourceAnalyzer::enableGraphs = false;


DataSourceAnalyzer::DataSourceAnalyzer(void)
{

	setMinLatencyEntries(DEFAULT_DATA_SOURCE_ANALYZER_MIN_LATENCY_ENTRIES);

	setSampleCounter(0);

	if(timeGlobal.getStarted() == false)
	{
		timeGlobal.start();
	}
}


DataSourceAnalyzer::~DataSourceAnalyzer(void)
{
	deleteGraphEntities();
}


void DataSourceAnalyzer::initializeGraphs(int positionOffsetX, int positionOffsetY)
{
	wchar_t		graphEnvValue[256];
	Graph	*	graph;


	if(::GetEnvironmentVariableW(L"PTVORTEX_PW_GRAPH", graphEnvValue, 256) > 0)
	{
		setEnableGraphs(false);

		swscanf_s(graphEnvValue, L"%d", &positionOffsetX);
															// Do not enable test graphs if window position offset is -1
		if(positionOffsetX != -1)
		{
			setEnableGraphs(true);
		}
	}

	if(getEnableGraphs() && graphManager.getNumGraphs() == 0)
	{
		Vector2i	graphSize(1850, 350);

		if(graph = graphManager.newGraph(L"Graph1"))
		{
			graph->createWindow(L"Pointools DataSourceAnalyzer (Budget, DataSize, Bandwidth)", positionOffsetX, positionOffsetY, graphSize[0], graphSize[1]);
		}

		if(graph = graphManager.newGraph(L"Graph2"))
		{
			graph->createWindow(L"Pointools DataSourceAnalyzer (Period, Latency)", positionOffsetX, (1 * graphSize[1]) + positionOffsetY, graphSize[0], graphSize[1]);
		}

		if(graph = graphManager.newGraph(L"Graph3"))
		{
			graph->createWindow(L"Pointools DataSourceAnalyzer (Nums)", positionOffsetX, (2 * graphSize[1]) + positionOffsetY, graphSize[0], graphSize[1]);
		}
	}
}


void DataSourceAnalyzer::deleteGraphEntities(void)
{
	graphManager.removeAllEntities();

	DataSourceAnalyzerSeries *	s;
	unsigned int				t;
	unsigned int				numSeries = getNumSeries();

	for(t = 0; t < numSeries; t++)
	{
		if(s = getSeries(static_cast<SeriesIndex>(t)))
		{
			delete s;
		}
	}
}

float DataSourceAnalyzer::getSizeKB(DataSize sizeBytes)
{
															// return given size in Kilo Bytes
	return static_cast<float>(sizeBytes) / (1024);
}


float DataSourceAnalyzer::getSizeKb(DataSize sizeBytes)
{
															// return given size in Kilo Bytes
	return static_cast<float>(sizeBytes) / (1024 / 8);
}


float DataSourceAnalyzer::getSizeMB(DataSize sizeBytes)
{
															// Return given size in Mega Bytes
	return static_cast<float>(sizeBytes) / (1024 * 1024);
}


float DataSourceAnalyzer::getSizeMb(DataSize sizeBytes)
{
															// Return given size in Mega Bits
	return static_cast<float>(sizeBytes) / ((1024 * 1024) / 8);
}


DataSourceAnalyzerSeries *DataSourceAnalyzer::createNewSeries(Graph *graph, SeriesIndex seriesIndex, const wchar_t *seriesName, unsigned int maxPoints, GraphEntityStyle &style)
{
	if(getSeries(seriesIndex) != NULL)
		return NULL;

	DataSourceAnalyzerSeries *	series;

	if((series = new DataSourceAnalyzerSeries()) == NULL)
		return NULL;

	setSeries(seriesIndex, series);

	series->setName(seriesName);

	series->setNumPointsMax(maxPoints);

	series->setStyle(style);

	if(graph)
	{
		graph->addEntity(*series);
	}

	return series;
}


bool DataSourceAnalyzer::initialize(void)
{
	DataSourceAnalyzerSeries	*	series;
	Graph						*	graph;
	GraphEntityStyle				style;

															// If Graph 1 defined, create optional period series
	if(graph = graphManager.getGraph(1))
	{
		createNewSeries(graph, SeriesPeriod, L"Period", 100, GraphEntityStyle(GraphEntityStyle::ColorRGB(0, 0, 255)));

		graph->setIncludeOriginY(true);
	}
															// Create mandatory latency series required for analysis
	createNewSeries(graph, SeriesLatency, L"Latency (s)", 3, GraphEntityStyle(GraphEntityStyle::ColorRGB(255, 255, 0)));

															// If Graph 0 defined, create various optional series
	if(graph = graphManager.getGraph(0))
	{
		if(series = createNewSeries(graph, SeriesBudget, L"Budget (KB)", 100, GraphEntityStyle(GraphEntityStyle::ColorRGB(0, 255, 0))))
		{
			series->setUnitScale(Vector2d(1.0, 1.0 / 1024.0));
		}

		if(series = createNewSeries(graph, SeriesDataSize, L"DataSize (KB)", 100, GraphEntityStyle(GraphEntityStyle::ColorRGB(0, 255, 255))))
		{
			series->setUnitScale(Vector2d(1.0, 1.0 / 1024.0));
		}

		graph->setIncludeOriginY(true);
	}

															// Create mandatory Bandwidth series required for analysis
	if(series = createNewSeries(graph, SeriesBandwidth, L"Bandwidth (-Latency) (KB/s)", 5, GraphEntityStyle(GraphEntityStyle::ColorRGB(255, 0, 0))))
	{
		series->setUnitScale(Vector2d(1.0, 1.0 / 1024.0));
	}


	if(graph = graphManager.getGraph(2))
	{
		if(series = createNewSeries(graph, SeriesNumMultiReads, L"MultiReads (n)", 100, GraphEntityStyle(GraphEntityStyle::ColorRGB(255, 255, 0))))
		{
			series->setUnitScale(Vector2d(1.0, 1.0));
		}

		if(series = createNewSeries(graph, SeriesNumReads, L"Reads (n)", 100, GraphEntityStyle(GraphEntityStyle::ColorRGB(255, 0, 255))))
		{
			series->setUnitScale(Vector2d(1.0, 1.0));
		}

		graph->setIncludeOriginY(true);
	}


															// Start timer specifying last latency sample
	timeLastLatencySample.start();

	return true;
}


void DataSourceAnalyzer::clear(void)
{
	if(getSeries(SeriesLatency))
	{
		getSeries(SeriesLatency)->clear();
	}

	if(getSeries(SeriesBandwidth))
	{
		getSeries(SeriesBandwidth)->clear();
	}
}


void DataSourceAnalyzer::deleteAll(void)
{
	unsigned int				n;
	DataSourceAnalyzerSeries *	s;

	for(n = 0; n < series.size(); n++)
	{
		if(s = series[n])
		{
			delete s;
		}
	}

	series.clear();
}


unsigned int DataSourceAnalyzer::getNumLatencyEntries(void)
{
	DataSourceAnalyzerSeries *	latencySeries;

	if(latencySeries = getSeries(SeriesLatency))
	{
		return latencySeries->getNumPoints();
	}

	return 0;
}


unsigned int DataSourceAnalyzer::getNumBandwidthEntries(void)
{
	DataSourceAnalyzerSeries * bandwidthSeries;

	if(bandwidthSeries = getSeries(SeriesBandwidth))
	{
		return bandwidthSeries->getNumPoints();
	}

	return 0;
}


DataSize DataSourceAnalyzer::getLatencySampleDataSize(void)
{
	return DEFAULT_LATENCY_SAMPLE_DATA_SIZE;
}


void DataSourceAnalyzer::beginPeriod(void)
{
	timePeriod.start();

}


void DataSourceAnalyzer::endPeriod(void)
{
	DataSourceAnalyzerSeries *periodSeries;

	if(periodSeries = getSeries(SeriesPeriod))
	{
		Period currentTime	= timeGlobal.getEllapsedTimeSeconds();
		Period period		= timePeriod.getEllapsedTimeSeconds();

		DataSourceAnalyzerSeries::Point	p(currentTime, period);

		periodSeries->add(p);		
	}

}


bool DataSourceAnalyzer::beginRead(unsigned int numMultiReads, unsigned int numReads, ptds::DataSize dataSize)
{
	time.start();

	if(getSeries(SeriesLatency) == NULL || getSeries(SeriesBandwidth) == NULL)
	{
		return false;
	}

	return true;
}

bool DataSourceAnalyzer::endRead(unsigned int numMultiReads, unsigned int numReads, ptds::DataSize dataSize)
{
	DataSourceAnalyzerSeries	*s;

	Period period	= time.getEllapsedTimeSeconds();
	Period	t		= timeGlobal.getEllapsedTimeSeconds();


	if(dataSize == 0)
	{
		if(s = getSeries(SeriesLatency))
		{
			DataSourceAnalyzerSeries::Point	p(t, period);

			s->add(p);
		}
	}
	else
	{
		DataSourceAnalyzerSeries::Point	latencyMean;

		if(s = getSeries(SeriesLatency))
		{
			latencyMean = s->calculateMean();
		}

		if(period > latencyMean[1])
		{
			DataSourceAnalyzerSeries::Point	p(t, static_cast<double>(dataSize) / (period - latencyMean[1]));

			if(s = getSeries(SeriesBandwidth))
			{
				s->add(p);
			}
		}

		if(s = getSeries(SeriesDataSize))
		{
			DataSourceAnalyzerSeries::Point	p(t, static_cast<double>(dataSize));

			s->add(p);
		}

		if(s = getSeries(SeriesNumMultiReads))
		{
			DataSourceAnalyzerSeries::Point	p(t, static_cast<double>(numMultiReads));
			s->add(p);
		}

		if(s = getSeries(SeriesNumReads))
		{
			DataSourceAnalyzerSeries::Point	p(t, static_cast<double>(numReads));
			s->add(p);
		}

	}

	return true;
}


DataSourceAnalyzer::Period DataSourceAnalyzer::calculateEfficientPeriod(Period targetPeriod)
{
	DataSourceAnalyzerSeries	*	latencySeries;
	Period							minPeriod;
	Period							usePeriod;

															// Calculate mean latency
	if((latencySeries = getSeries(SeriesLatency)) == NULL)
	{
		return targetPeriod;
	}
															// Calculate mean latency from at most last n items
	minPeriod = (latencySeries->calculateMeanLast(DATA_SOURCE_ANALYZER_NUM_LATENCY_SAMPLES))[1] * DATA_SOURCE_ANALYZER_LATENCY_MEAN_MIN_PERIOD_COEF;

															// Threshold to bottom limit of n times latency
	usePeriod = std::max(targetPeriod, minPeriod);
															// Limit smallest period to prevent high frequency refreshes
	usePeriod = std::max(usePeriod, DATA_SOURCE_ANALYZER_MIN_PERIOD);

															// Return latency
	return usePeriod;
}


DataSize DataSourceAnalyzer::getReadSizePerPeriod(Period targetPeriod, Period &usePeriod)
{

	if(getNumLatencyEntries() == 0 || getTimeSinceLastLatencySample() > DATA_SOURCE_ANALYZER_LATENCY_SAMPLE_PERIOD)
	{
		return sampleLatency(targetPeriod, usePeriod);
	}

	setSampleCounter(getSampleCounter() + 1);

	return sampleBandwidth(targetPeriod, usePeriod);
}


DataSize DataSourceAnalyzer::sampleLatency(Period targetPeriod, Period &usePeriod)
{
	usePeriod = targetPeriod;

	timeLastLatencySample.start();

	return getLatencySampleDataSize();
}


DataSize DataSourceAnalyzer::sampleBandwidth(Period targetPeriod, Period &usePeriod)
{
	DataSourceAnalyzerSeries			*	latencySeries;
	DataSourceAnalyzerSeries			*	bandwidthSeries;

	DataSourceAnalyzerSeries::Point			latencyMean;
	DataSourceAnalyzerSeries::Point			bandwidthMean;
	DataSourceAnalyzerSeries::Point			bandwidthVector;
	DataSize								dataSize = 0;

#ifndef NO_DATA_SOURCE_SERVER

	if((latencySeries = getSeries(SeriesLatency)) == NULL)
	{
		return 0;
	}

	if(getNumLatencyEntries() == 0)
	{
		return (DATA_SOURCE_ANALYZER_DEFAULT_DATA_SIZE);
	}

	if((bandwidthSeries = getSeries(SeriesBandwidth)) == NULL)
	{
		return (DATA_SOURCE_ANALYZER_DEFAULT_DATA_SIZE);
	}

	if(getNumBandwidthEntries() == 0)
	{
		return (DATA_SOURCE_ANALYZER_DEFAULT_DATA_SIZE);
	}


	usePeriod = calculateEfficientPeriod(targetPeriod);

	latencyMean		= latencySeries->calculateMean();
	bandwidthMean	= bandwidthSeries->calculateMean();

	dataSize = static_cast<DataSize>((usePeriod - latencyMean[1]) * bandwidthMean[1]);

															// Apply slight upward pressure on bandwidth by requesting more than otherwise would
	dataSize *= pointsengine::StreamHost::getStreamScalarDefault();
															// If enforcing minimum budget, clip to lower threshold
	if(pointsengine::StreamHost::getStreamMinDefaultEnforce())
	{
		dataSize = std::max(dataSize, pointsengine::StreamHost::getStreamMinDefault());
	}

	DataSourceAnalyzerSeries *budgetSeries;
	if(budgetSeries = getSeries(SeriesBudget))
	{
		double timeSeconds = timeGlobal.getEllapsedTimeSeconds();
		DataSourceAnalyzerSeries::Point	p(timeSeconds, static_cast<float>(dataSize));
		budgetSeries->add(p);
	}

#endif

	return dataSize;
}


DataSourceAnalyzer::Period DataSourceAnalyzer::getTimeSinceLastLatencySample(void)
{
	return timeLastLatencySample.getEllapsedTimeSeconds();
}





} // End ptds namespace
