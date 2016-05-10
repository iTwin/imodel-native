#pragma once

#include <ptcloud2/voxel.h>
#include <ptengine/queryFilter.h>
#include <pt/OBBox.h>

namespace pointsengine
{

template <class T, class Receiver>
struct OBBFilter
{
	// constructor
	OBBFilter( pt::OBBox<T> box ) : m_box(box), m_rec(Receiver()) 
	{}
	
	OBBFilter(Receiver &r, pt::OBBox<T> box) 
		: m_rec(r), m_box(box) 
	{}

	template <class PointType>
	inline void point( const PointType &pt, int &index, ubyte &layers)
	{
		if (m_box.contains(pt))
			m_rec.point(pt, index, layers);
	}	

	inline FilterResult node( const pcloud::Node *n )
	{
		FilterResult res;
		pt::BoundingBoxD box = n->extents();	

		if (m_box.contains(box)) res = FilterIn;
		else if (m_box.intersects( box )) res = FilterPartial; 
		else if (!m_box.containedByBBox(box)) res = FilterOut;
		else res = FilterPartial; // should never happen?

		return combinedResult( res, m_rec.node(n) );
	}
	
	inline FilterResult cloud( const pcloud::PointCloud *pc )
	{		
		return combinedResult( node(pc->root()), m_rec.cloud(pc) );
	}

	inline FilterResult scene( const pcloud::Scene *sc )
	{
		return combinedResult( FilterIn, m_rec.scene(sc) );
	}

private:
	Receiver			&m_rec;
	pt::OBBox<T>		m_box;
	
};

template <class T, class Receiver1, class Receiver2=Receiver1>
struct OBBFilter2
{
	// constructor	
	OBBFilter2(	Receiver1 &r1, 
				pt::OBBox<T> box1, 
				Receiver1 &r2, 
				pt::OBBox<T> box2, 
				bool intersectingPtsOnly=false, // inserts only points in intersecting voxels
				bool includeCorners=false	// inserts voxel corners as points
				) 

		:	m_box1(box1), m_box2(box2), 
			m_rec1(r1), m_rec2(r2),
			m_intersectingPtsOnly(intersectingPtsOnly),
			m_includeCorners(includeCorners)
	{}

	template <class PointType>
	inline void point( const PointType &pt, int &index, ubyte &layers)
	{
		if (m_box1.contains(pt))
			m_rec1.point(pt, index, layers);
		else if (m_box2.contains(pt))
			m_rec2.point(pt, index, layers);
	}	

	inline FilterResult node( const pcloud::Node *n )
	{
		FilterResult res = FilterOut;
		pt::BoundingBoxD box = n->extents();		
		
		if ( m_box1.contains(box) || m_box2.contains(box) ) 
		{
			if ( n->isLeaf())
			{
				res = m_intersectingPtsOnly ?  FilterOut : FilterIn;
		
				if (m_includeCorners)
				{
					pt::vec3<T> vt;
									
					//extract corners and add to pts
					for (int i=0; i<8; i++) 
					{
						box.getExtrema(i, vt);
						if (m_box1.contains(box))		
						{
							m_rec1.point(vt,0,0);
						}
						else
						{
							m_rec2.point(vt,0,0);
						}
					}
				}				
			}
			else res = FilterIn;
		}
		else if ( m_box1.intersects( box ) || m_box2.intersects( box )
			|| 
			(!m_intersectingPtsOnly && (m_box1.containedBy(box) || m_box2.containedBy(box)))
			)
		{
			res = FilterPartial; 
		}
		else 
		{
			res = FilterOut;
		}

		return combinedResult( res, m_rec1.node(n) );
	}
	
	inline FilterResult cloud( const pcloud::PointCloud *pc )
	{		
		return combinedResult( node(pc->root()), m_rec1.cloud(pc) );
	}

	inline FilterResult scene( const pcloud::Scene *sc )
	{
		return combinedResult( FilterIn, m_rec1.scene(sc) );
	}

	bool				m_intersectingPtsOnly;
	bool				m_includeCorners;

private:
	Receiver1			&m_rec1;
	Receiver2			&m_rec2;

	pt::OBBox<T>		m_box1;
	pt::OBBox<T>		m_box2;
	
};

// this version generates points for boundary conditions
// used to build a points bounds tree

template <class T, class Receiver1, class Receiver2=Receiver1>
struct OBBFilter2NodeExtents
{
	// constructor	
	OBBFilter2NodeExtents(	Receiver1 &r1, 
				pt::OBBox<T> box1, 
				Receiver1 &r2, 
				pt::OBBox<T> box2,
				T pointSpacing
				) 

		:	m_box1(box1), m_box2(box2), 
			m_rec1(r1), m_rec2(r2), m_spacing(pointSpacing)
	{}

	template <class PointType>
	inline void point( const PointType &pt, int &index, ubyte &layers)
	{
		// nothing to do
	}	

	inline FilterResult node( const pcloud::Node *n )
	{
		FilterResult res = FilterOut;
		pt::BoundingBoxD box = n->extents();				
		pt::vec3<T> vt;
		
		// fully contained, return corners
		if ( m_box1.contains(box) || m_box2.contains(box) ) 
		{									
			//extract corners and add to pts
			for (int i=0; i<8; i++) 
			{
				box.getExtrema(i, vt);
				if (m_box1.contains(box))		
				{
					m_rec1.point(vt,0,0);
				}
				else
				{
					m_rec2.point(vt,0,0);
				}							 
			}
			res = FilterOut;
		}
		// intersection
		else if ( m_box1.intersects( box ) || m_box2.intersects( box ) )
		{
			if ( !n->isLeaf())
			{
				res = FilterPartial;
			}
			else
			{
#ifdef OLD_METHOD
			// generate box points
				for (double x=box.lx(); x< box.ux(); x += m_spacing)
				{
					for (double y=box.ly(); y< box.uy(); y += m_spacing)
					{						
						for (double z=box.lz(); z< box.uz(); z += m_spacing)
						{
							vec3<T> pnt(x,y,z);							

						if (m_box1.contains(pnt))
							m_rec1.point(pnt, 0, 0);
						else if (m_box2.contains(pnt))
							m_rec2.point(pnt, 0, 0);												
						}
					}
				}
#endif
				// generate box points
				for (double x=box.lx(); x< box.ux(); x += m_spacing)
				{
					for (double y=box.ly(); y< box.uy(); y += m_spacing)
					{						
						pt::vec3<T> pnt(x,y,box.lz());
						pt::vec3<T> pnt2(x,y,box.uz());

						if (m_box1.contains(pnt))
							m_rec1.point(pnt, 0, 0);
						else if (m_box2.contains(pnt))
							m_rec2.point(pnt, 0, 0);						

						if (m_box1.contains(pnt2))
							m_rec1.point(pnt2, 0, 0);
						else if (m_box2.contains(pnt2))
							m_rec2.point(pnt2, 0, 0);						
					}
				}	
				// generate box points
				for (double x=box.lx(); x< box.ux(); x += m_spacing)
				{
					for (double z=box.lz(); z< box.uz(); z += m_spacing)
					{						
						pt::vec3<T> pnt(x,box.ly(),z);
						pt::vec3<T> pnt2(x,box.uy(),z);

						if (m_box1.contains(pnt))
							m_rec1.point(pnt, 0, 0);
						else if (m_box2.contains(pnt))
							m_rec2.point(pnt, 0, 0);						

						if (m_box1.contains(pnt2))
							m_rec1.point(pnt2, 0, 0);
						else if (m_box2.contains(pnt2))
							m_rec2.point(pnt2, 0, 0);						
					}
				}
				// generate box points
				for (double y=box.ly(); y< box.uy(); y += m_spacing)
				{
					for (double z=box.lz(); z< box.uz(); z += m_spacing)
					{						
						pt::vec3<T> pnt(box.lx(),y,z);
                        pt::vec3<T> pnt2(box.ux(),y,z);

						if (m_box1.contains(pnt))
							m_rec1.point(pnt, 0, 0);
						else if (m_box2.contains(pnt))
							m_rec2.point(pnt, 0, 0);						

						if (m_box1.contains(pnt2))
							m_rec1.point(pnt2, 0, 0);
						else if (m_box2.contains(pnt2))
							m_rec2.point(pnt2, 0, 0);						
					}
				}
				// add the far corners as well as the these corners will be missed by the above box point generation
				for (int i=0; i<8; i++) 
				{
					box.getExtrema(i, vt);
					if (m_box1.contains(box))		
					{
						m_rec1.point(vt,0,0);
					}
					if (m_box2.contains(box))	
					{
						m_rec2.point(vt,0,0);
					}							 
				}
			}
		}
		// search box contained in this node
		else if ( m_box1.containedBy(box) || m_box2.containedBy(box) )
		{
			pt::vec3<T> vertices[8];

			// return corners of the search box
			if (m_box1.containedBy(box))							
			{
				m_box1.computeVertices( vertices );
				for (int i=0; i<8; i++)
					m_rec1.point(vertices[i], 0, 0);
			}
			if (m_box2.containedBy(box))							
			{
				m_box2.computeVertices(vertices);
				for (int i=0; i<8; i++)
					m_rec2.point(vertices[i], 0, 0);
			}
			res = FilterOut;
		}
		else res = FilterIn;

		if (n->isLeaf()) 
			res = FilterOut;

		return combinedResult( res, m_rec1.node(n) );
	}
	
	inline FilterResult cloud( const pcloud::PointCloud *pc )
	{		
		return combinedResult( node(pc->root()), m_rec1.cloud(pc) );
	}

	inline FilterResult scene( const pcloud::Scene *sc )
	{
		return combinedResult( FilterIn, m_rec1.scene(sc) );
	}

private:
	Receiver1			&m_rec1;
	Receiver2			&m_rec2;

	pt::OBBox<T>		m_box1;
	pt::OBBox<T>		m_box2;

	T					m_spacing;	
};
}