#pragma once

#include <ptcloud2/voxel.h>
#include <ptengine/queryFilter.h>
#include <ptcloud2/bitVoxelGrid.h>

namespace pointsengine
{
// grid implemented using a set
struct HashGrid
{
	typedef uint64_t HashVal;

	HashGrid( const pt::vector3d &spacing ) 
		:  m_spacing(spacing)
	{
		m_lower.set(0,0,0);
	}

	bool setSpacing(const pt::vector3d &pt)
	{
		if (m_set.size() == 0)
		{
			m_spacing = pt;
			return true;
		}
		return false; // cannot change the spacing after the HashGrid has been used!
	}

	bool insertPoint( const pt::vector3d &pt)
	{	
		HashVal hash;

		pt::vector3d offset(pt-m_lower);

		if (hashPoint(offset, hash) && m_set.find( hash )==m_set.end())
		{
			m_set.insert( hash );
			return true;
		}
		return false;
	}

	bool insertPoint( const pt::vector3 &pt)
	{
		HashVal hash;

		pt::vector3d ptd(pt.x, pt.y, pt.z);
		
		pt::vector3d offset(ptd-m_lower);

		if (hashPoint(offset, hash) && m_set.find( hash )==m_set.end())
		{
			m_set.insert( hash );
			return true;
		}
		return false;
	}

	bool hasPoint( const pt::vector3d &pt ) const
	{
		HashVal hash;
		
		pt::vector3d offset(pt-m_lower);

		if (hashPoint(offset, hash) && m_set.find( hash )==m_set.end())
		{
			return true;
		}
		return false;

	}
	void clear()
	{
		m_set.clear();
	}
	void bounds( const pt::vector3d &lower, const pt::vector3d &upper) 
	{
		m_lower=lower;
	}

private:

	// compute a hash value for the point position
	bool hashPoint(const pt::vector3d &pnt, HashVal &hash) const
	{
		pt::vector3i quantize_multiples( 
			static_cast<int>(floor(pnt.x / m_spacing.x)), 
			static_cast<int>(floor(pnt.y / m_spacing.y)), 
			static_cast<int>(floor(pnt.z / m_spacing.z)));
                                                       
		// truncate anything too big 
		pt::vector3i large_component(quantize_multiples);
		large_component /= 1000000;

		for (int i=0; i<3; i++)
		{
			if (large_component[i] > 0)
			{
				large_component[i] *= 1000000;
				quantize_multiples[i] -= large_component[i];

				// get rid of any negatives 
				quantize_multiples[i] += 1000000;
			}
		}

		if (quantize_multiples.x < 0 ||
			quantize_multiples.y < 0 ||
			quantize_multiples.z < 0) 
			return false;

		HashVal qx = (quantize_multiples.x) & 0x1fffff;
		HashVal qy = quantize_multiples.y;
		qy <<= 21;
		qy &= 0x3FFFFE00000;

		HashVal qz = quantize_multiples.z;
		qz <<= 42;
		qz &= 0x7FFFFC0000000000;
		
		hash = qx | qy | qz;
		return true;
	}

	typedef std::set<HashVal>	PointsHashSet;
	PointsHashSet	m_set;
	pt::vector3d	m_spacing;
	pt::vector3d	m_lower;
};

// grid implemented using binary voxel grid
struct VoxelGrid
{
	VoxelGrid( const pt::vector3d &spacing) 
		:  m_spacing(spacing), m_grid(0)
	{
	}
	void bounds( const pt::vector3d &lower, const pt::vector3d &upper)
	{
		if (m_grid) delete m_grid;
		m_grid = new pt::BitVoxelGrid(lower.x,lower.y,lower.z,
			upper.x-lower.x,upper.y-lower.y,upper.z-lower.z, m_spacing.x);
	}
	bool insertPoint( const pt::vector3 &pt)
	{	
		return insertPoint( pt::vector3d(pt.x,pt.y,pt.z));
	}	

	bool insertPoint( const pt::vector3d &pt)
	{
		bool val;
		if (m_grid->get( pt.x, pt.y, pt.z, val ))
		{
			if (!val)
			{
				m_grid->set( pt.x, pt.y, pt.z, true );
				return true;
			}
			return false;
		}
		return false;
	}
	void clear()
	{
		if (m_grid) delete m_grid;
		m_grid = 0;
	}
	pt::vector3d		m_spacing;
	pt::BitVoxelGrid	*m_grid;
};

// grid filter
template <class Receiver, class Grid=HashGrid>
struct GridFilter
{
	enum Separation
	{
		PerLeaf,
		PerCloud,
		PerScene
	};

	// constructor
	GridFilter(pt::vector3d spacing, Separation sep=PerLeaf) 
		: m_rec(Receiver()), m_grid(spacing), m_sep(sep),
		m_insertCount(0), m_filteredCount(0)
	{}
	
	GridFilter(Receiver &r, pt::vector3d spacing, Separation sep=PerLeaf) 
		: m_rec(r), m_sep(sep), m_grid(spacing),
		m_insertCount(0), m_filteredCount(0)
	{}

	inline FilterResult node( const pcloud::Node *n )
	{
		if (m_sep==PerLeaf && n->isLeaf())	
		{
			m_grid.clear();

			const pcloud::Voxel *v =static_cast<const pcloud::Voxel*>(n);
			pt::BoundingBoxD b = v->extents();		

			m_grid.bounds( &b.lower(0), &b.upper(0) );
		}

		return FilterIn;
	}
	
	inline FilterResult cloud( const pcloud::PointCloud *pc )
	{
		if (m_sep==PerCloud)
		{
			m_grid.clear();
			const pt::BoundingBoxD &b = pc->projectBounds().bounds();
			m_grid.bounds( &b.lower(0), &b.upper(0) );		
		}

		return FilterIn;
	}

	inline FilterResult scene( const pcloud::Scene *sc )
	{
		if (m_sep==PerScene)
		{
			m_grid.clear();
			const pt::BoundingBoxD &b = sc->projectBounds().bounds();
			m_grid.bounds( &b.lower(0), &b.upper(0) );
		}

		return FilterIn;
	}

	template <class PointType>
	inline void point( const PointType &pt, int &index, ubyte &layers)
	{
		if (m_grid.insertPoint(pt)) 
		{
			m_rec.point(pt,index,layers);
			++m_filteredCount;
		}
		++m_insertCount;
	}

	uint	insertCount(void) const			{ return m_insertCount; }
	
	uint	filteredCount(void) const		{ return m_filteredCount; }

private:

	Grid			m_grid;
	Separation		m_sep;
	Receiver		&m_rec;
	uint			m_insertCount;
	uint			m_filteredCount;
};

}