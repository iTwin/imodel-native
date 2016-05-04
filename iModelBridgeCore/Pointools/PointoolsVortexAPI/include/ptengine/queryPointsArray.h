#pragma once

#include <pt/geomtypes.h>
#include <ptcloud2/voxel.h>
#include <ptengine/queryFilter.h>

namespace pointsengine
{
	template <typename T>
	class QueryPointsArray
	{
	public:

		typedef pt::vec3<T> PointType ;

		explicit QueryPointsArray( int initial_size=100000, int chunk_size=100000 )
			: m_chunkSize(chunk_size), m_position(0), m_data(0), m_capacity(0)
		{
			if (initial_size)
			{
				allocate( initial_size );
			}
		}

		static int64_t totalPointsStoredCount()
		{
			return totalPointsStored();
		}
		static int64_t totalPointsAllocatedCount()
		{
			return totalPointsAllocated();
		}
		QueryPointsArray::~QueryPointsArray()
		{
			destroy();
		}

//		inline void point( const pt::vec3<float> &pt, int index, ubyte layers )
//		{
//		}
		inline void point( const pt::vec3<double> &pt, int index, ubyte layers )
		{
		}

		inline PointType &operator[](int index) 
		{
			return point(index);
		}

		inline const PointType &operator[](int index) const 
		{
			return point(index);
		}

		inline void push_back( const PointType &p )
		{
			addPoint( p );
		}

		bool addPoint( const PointType &p )
		{
			if (m_position < m_capacity || expand())
			{				
				m_data[m_position] = p;
				++m_position;
				++totalPointsStored();
				return true;
			}
			return false;
		}
		


		int size() const 
		{
			return m_position;
		}

		const PointType *data() const 
		{
			return m_data;
		}
		//void addPoints( const PointType &pts, int num );

		void clear()
		{
			totalPointsStored() -= m_position;
			m_position = 0;
		}

		void destroy()
		{
			if (m_data)
			{
				totalPointsStored() -= m_position;
				totalPointsAllocated() -= m_capacity;

				delete [] m_data;
				m_data = 0;
				m_position = 0;
				m_capacity = 0;
			}
		}

		bool resizeToFit() 
		{
			if (!m_data || !m_position) return false;

			PointType *new_data=0;

			try 
			{ 
				 new_data = new PointType[m_position];
			}
			catch (std::bad_alloc)
			{
				return false;
			}
			memcpy( new_data, m_data, m_position * sizeof(PointType) );
			delete [] m_data;
			
			totalPointsAllocated() -= m_capacity - m_position;

			m_data = new_data;
			m_capacity = m_position;
			
			return true;
		}

		bool empty() const 
		{
			return m_position ? false : true;
		}
		int capacity() const
		{
			return m_capacity;
		}

		bool reserveCapacity( int num )
		{
			return allocate( num );
		}

		inline PointType &at( int index )	
		{
			return m_data[index];
		}
		const PointType &at( int index ) const
		{
			return m_data[index];
		}
		
		IMPL_FILTER_IN
	
	protected:

		static inline int64_t &totalPointsStored()
		{
			static int64_t total=0;
			return total;
		}
		static inline int64_t &totalPointsAllocated()
		{
			static int64_t total=0;
			return total;
		}
		bool expand()
		{
			if (allocate( m_capacity + m_chunkSize ))
			{
				m_chunkSize *= 1.25;
				return true;
			}
			return false;
		}

		bool allocate( int new_capacity )
		{
			if (new_capacity > m_capacity)
			{
				PointType *new_data=0;

				try 
				{ 
					new_data = new PointType[new_capacity];
				}
				catch (std::bad_alloc)
				{
					return false;
				}
				
				totalPointsAllocated() += new_capacity - m_capacity;

				if (m_data)
				{
					memcpy( new_data, m_data, m_position * sizeof(PointType) );
					delete [] m_data;
				}
				m_data = new_data;
				m_capacity = new_capacity;
				return true;
			}
			return true;
		}

	private:
		int m_chunkSize;
		int m_position;
		int m_capacity;

		PointType *m_data;
	};

	template <> 
	void QueryPointsArray<float>::point( const pt::vec3<double> &pt, int index, ubyte layers )
	{
		addPoint(pt::vec3<float>(pt.x, pt.y, pt.z));
	}

//	template <> 
//	void QueryPointsArray<float>::point( const pt::vec3<float> &pt, int index, ubyte layers )
//	{
//		addPoint(pt);
//	}

	template <> 
	void QueryPointsArray<double>::point( const pt::vec3<double> &pt, int index, ubyte layers )
	{
		addPoint(pt);
	}

//	template <> 
// 	void QueryPointsArray<double>::point( const pt::vec3<float> &pt, int index, ubyte layers )
// 	{
// 		addPoint(pt::vec3<double>(pt.x, pt.y, pt.z));
// 	}

//	typedef QueryPointsArray<float> QueryPointsArrayf; 
	typedef QueryPointsArray<double> QueryPointsArrayd; 
}