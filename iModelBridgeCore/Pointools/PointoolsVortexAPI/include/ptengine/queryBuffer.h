#pragma once

#include <FastDelegate/fastdelegate.h>
#include <ptengine/queryFilter.h>
#include <memory>

namespace pointsengine
{
	
	template<class PointType>
	struct QueryBuffer
	{
		typedef pt::FastDelegate1<QueryBuffer*> FlushDelegate;	

		QueryBuffer( int bufferSize, FlushDelegate flushDelegate=0 )
		{
			m_size=bufferSize; 
			m_pointsBuffer=0; 
			m_rgbBuffer=0;
			m_intensityBuffer=0;
			m_classificationBuffer=0;
			m_layerBuffer=0;
			m_position=0;
		}
		virtual ~QueryBuffer()
		{
			clear();
		}
		bool empty() const
		{
			return m_position ? false : true;
		}
		void flush()
		{
			if (m_flushDelegate) m_flushDelegate(this);
			m_position = 0;
		}
		int numValidPoints() const
		{
			return m_position;
		}
		bool initialize	( bool needRGB=false, bool needIntensity=false, 
			bool needClassification=false, bool needLayer=false )
		{
			try
			{
				if (!m_pointsBuffer) 
					m_pointsBuffer = new PointType[m_size];

				if (needRGB && !m_rgbBuffer) 
					m_rgbBuffer = new ubyte[m_size*3];

				if (needIntensity && !m_intensityBuffer) 
					m_intensityBuffer = new short[m_size*3];

				if (needClassification && !m_classificationBuffer)
					m_classificationBuffer = new ubyte[m_size];

				if (needLayer && !m_layerBuffer)
					m_layerBuffer = new ubyte[m_size];

				return true;
			}
			catch(std::bad_alloc)
			{
				clear();
				return false;
			}
		}

		void clear()
		{
			if (m_pointsBuffer)	delete [] m_pointsBuffer;
			m_pointsBuffer=0;

			if (m_rgbBuffer)	delete [] m_rgbBuffer;
			m_rgbBuffer=0;

			if (m_intensityBuffer)	delete [] m_intensityBuffer;
			m_intensityBuffer=0;

			if (m_classificationBuffer)	delete [] m_classificationBuffer;
			m_classificationBuffer=0;

			if (m_layerBuffer)	delete [] m_layerBuffer;
			m_layerBuffer=0;
		}

		inline const PointType	&getPoint( int index ) const				{ return m_pointsBuffer[index]; }
		inline ubyte			*getRGB( int index )						{ return &m_rgbBuffer[index*3]; }

		inline const PointType	*getPointsBuffer()	const					{ return m_pointsBuffer; }
		inline const ubyte		*getRGBBuffer()	const						{ return m_rgbBuffer; }
		inline const ubyte		*getClassificationBuffer() const			{ return m_classificationBuffer; }
		inline const ubyte		*getLayerBuffer() const						{ return m_layerBuffer; }

		template<class QueryPointType>
		void point( const QueryPointType &pt, int index, ubyte layer )
		{
			m_pointsBuffer[m_position].x = pt.x;
			m_pointsBuffer[m_position].y = pt.y;
			m_pointsBuffer[m_position].z = pt.z;

			if (m_layerBuffer)
				m_layerBuffer[m_position] = layer;

			++m_position;

			if (m_position >=m_size) flush();
		}

		inline FilterResult node( const pcloud::Node *n )			{	return FilterIn;	}
		inline FilterResult cloud( const pcloud::PointCloud *pc )	{	return FilterIn;	}
		inline FilterResult scene( const pcloud::Scene *sc )		{	return FilterIn;	}

	private:

		PointType		*m_pointsBuffer;
		ubyte			*m_rgbBuffer;
		short			*m_intensityBuffer;
		ubyte			*m_classificationBuffer;
		ubyte			*m_layerBuffer;

		int				m_position;
		int				m_size;
		FlushDelegate	m_flushDelegate;
	};

	typedef QueryBuffer< pt::vec3<float> >	QueryBufferf;
	typedef QueryBuffer< pt::vec3<double> >  QueryBufferd;
};