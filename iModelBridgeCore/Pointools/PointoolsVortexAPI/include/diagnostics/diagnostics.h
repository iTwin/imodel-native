#pragma once

#include <pt/timer.h>
#include <pt/typedefs.h>
#include <queue>

#ifdef HAVE_BOOST
    namespace boost { 	namespace interprocess { class message_queue; } }

    #include <boost/interprocess/ipc/message_queue.hpp>
#endif

namespace ptdg
{
#define PT_DIAG_PACKET_SIZE		1024

	enum  MetricTypeID
	{
		eQueryMetric1	= 1,
		eLoadMetric1	= 2,
		ePagerMetric1	= 3,
		eRenderMetric1	= 4,
		eVisibilityMetric1 = 5
	};

	// interface for Metrics
	struct Time
	{
		template <class T>
		static void stamp( T &t )			{ t.m_timeStamp =  pt::Timer::instance()->tick(); }

		template <class T>
		static void endStamp( T &t )		{ t.m_endStamp =  pt::Timer::instance()->tick(); }

		template <class T>
		static void timeTakenMS( T &t )		{ return pt::Timer::instance()->delta_ms( t.m_timeStamp, t.m_endStamp ); }

	};
	// all metric structs MUST BE BINARY SERIALIZABLE, no strings etc..

	// Query Stats - must be fixed size, no strings etc. 
	// Version number appended to name
	struct QueryData1
	{
		static		MetricTypeID		metricID()	{ return eQueryMetric1; }

		char		m_queryType[16];

		uint		m_queryID;
		pt::Timer_t	m_timeStamp;
		pt::Timer_t	m_endStamp;

		int64_t		m_numPoints;
		int			m_bufferSize;
		uint		m_channels;

		uint		m_density;
		float		m_densityVal;
	};

	struct LoadData1
	{
		static		MetricTypeID		metricID()	{ return eLoadMetric1; }

		pt::Timer_t	m_timeStamp;
		
		int64_t		m_ptsShortfall;
		int64_t		m_ptsLoadedSinceDraw;
	};

	struct PagerData1
	{
		static		MetricTypeID		metricID()	{ return ePagerMetric1; }

		pt::Timer_t	m_timeStamp;

		uint		m_actualPtsLoaded;
		int64_t		m_memAval;
		uint		m_mode;
	};

	struct RenderData1
	{
		static		MetricTypeID		metricID()	{ return eRenderMetric1; }

		pt::Timer_t	m_timeStamp;

		int			m_voxelsRendered;
		int			m_ptsRendered;
		int			m_frameTime;
		int			m_ptsOOC;
	};
	
	struct VisibilityData1
	{
		static		MetricTypeID		metricID()	{ return eVisibilityMetric1; }

		pt::Timer_t	m_timeStamp;

		int			m_numVoxelsInView;
		int64_t		m_numFullPtsInView;		//full number, not LOD
		
		int64_t		m_numLODPtsInView1;		// to test different lod algos
		int64_t		m_numLODPtsInView2;
		int64_t		m_numLODPtsInView3;
		int64_t		m_numLODPtsInView4;

		float		m_averageLOD;
		float		m_maxLOD;
		float		m_minLOD;
		float		m_totalPxArea;
		float		m_avPxArea;
	};

	class Diagnostics
	{
	public:
		~Diagnostics();

		bool initialise();

		static Diagnostics *instance();

		template< class Metric > 
		void addMetric( Metric &m )
		{
			addMetric( m.metricID(), (ubyte*)&m, sizeof(m) );
		}

	private:
		Diagnostics();

		void addMetric( MetricTypeID id, ubyte* data, int size );
        
#ifdef HAVE_BOOST
		boost::interprocess::message_queue	*m_ptmq;
#endif

	};

};