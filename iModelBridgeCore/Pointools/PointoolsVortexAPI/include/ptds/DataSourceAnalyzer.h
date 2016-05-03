
#pragma once

#include <ptds/ptds.h>
#include <pt/timestamp.h>

#define DEFAULT_LATENCY_SAMPLE_DATA_SIZE	0


namespace ptds
{

class DataSource;

class DataSourceAnalyzerSeries;
class GraphEntityStyle;

class Graph;



class DataSourceAnalyzer
{

public:

	typedef double				Period;
	typedef unsigned int		SampleCounter;

	enum SeriesIndex
	{
		SeriesPeriod		= 0,
		SeriesLatency		= 1,
		SeriesDataSize		= 2,
		SeriesBandwidth		= 3,
		SeriesBudget		= 4,
		SeriesNumMultiReads	= 5,
		SeriesNumReads		= 6,

		SeriesCount			= 5
	};


protected:

	typedef std::vector<DataSourceAnalyzerSeries *>	SeriesSet;

protected:

	SeriesSet					series;

	pt::SimpleTimer				time;
	static pt::SimpleTimer		timeGlobal;
	pt::SimpleTimer				timeLastLatencySample;
	pt::SimpleTimer				timePeriod;

	unsigned int				minLatencyEntries;

	static bool					enableGraphs;
	SampleCounter				sampleCounter;

protected:

	void						setSeries						(SeriesIndex index, DataSourceAnalyzerSeries *series);
	DataSourceAnalyzerSeries *	getSeries						(SeriesIndex index);
	unsigned int				getNumSeries					(void);

	unsigned int				getNumLatencyEntries			(void);
	unsigned int				getNumBandwidthEntries				(void);

	DataSize					getLatencySampleDataSize		(void);

	DataSize					sampleLatency					(Period targetPeriod, Period &usePeriod);
	DataSize					sampleBandwidth					(Period targetPeriod, Period &usePeriod);

	Period						getTimeSinceLastLatencySample	(void);

	void						setSampleCounter				(SampleCounter value);
	SampleCounter				getSampleCounter				(void);

	Period						calculateEfficientPeriod		(Period targetPeriod);

public:
								DataSourceAnalyzer				(void);
							   ~DataSourceAnalyzer				(void);

	bool						initialize						(void);

	static void					initializeGraphs				(int positionOffsetX = 0, int positionOffsetY = 0);
	void						deleteGraphEntities				(void);

	DataSourceAnalyzerSeries *	createNewSeries					(Graph *graph, SeriesIndex seriesIndex, const wchar_t *seriesName, unsigned int maxPoints, GraphEntityStyle &style);

	static void					setEnableGraphs					(bool enabled);
	static bool					getEnableGraphs					(void);

	void						clear							(void);
	void						deleteAll						(void);

	void						beginPeriod						(void);
	void						endPeriod						(void);

	bool						beginRead						(unsigned int numMultiReads, unsigned int numReads, ptds::DataSize numBytes);
	bool						endRead							(unsigned int numMultiReads, unsigned int numReads, ptds::DataSize numBytes);

	void						setMinLatencyEntries			(unsigned int numEntries);
	unsigned int				getMinLatencyEntries			(void);

	DataSize					getReadSizePerPeriod			(Period targetPeriod, Period &usePeriod);

	static float				getSizeKB						(DataSize sizeBytes);
	static float				getSizeKb						(DataSize sizeBytes);
	static float				getSizeMB						(DataSize sizeBytes);
	static float				getSizeMb						(DataSize sizeBytes);
};


inline void DataSourceAnalyzer::setEnableGraphs(bool enabled)
{
	enableGraphs = enabled;
}


inline bool DataSourceAnalyzer::getEnableGraphs(void)
{
	return enableGraphs;
}


inline void DataSourceAnalyzer::setSampleCounter(SampleCounter v)
{
	sampleCounter = v;
}


inline DataSourceAnalyzer::SampleCounter DataSourceAnalyzer::getSampleCounter(void)
{
	return sampleCounter;
}


inline void DataSourceAnalyzer::setSeries(SeriesIndex index, DataSourceAnalyzerSeries *initSeries)
{
	if(index >= (int)getNumSeries())
	{
		series.resize(index + 1);
	}

	series[index] = initSeries;
}


inline unsigned int DataSourceAnalyzer::getNumSeries(void)
{
	return (unsigned int)series.size();
}


inline DataSourceAnalyzerSeries *DataSourceAnalyzer::getSeries(SeriesIndex index)
{
	if(index < (int)getNumSeries())
	{
		return series[index];
	}

	return NULL;
}


inline void DataSourceAnalyzer::setMinLatencyEntries(unsigned int numEntries)
{
	minLatencyEntries = numEntries;
}


inline unsigned int DataSourceAnalyzer::getMinLatencyEntries(void)
{
	return minLatencyEntries;
}


} // End ptds namespace