#include "ClusterAnalyser.h"
#include <queue>
#include <algorithm>

using namespace vortex;

//-----------------------------------------------------------------------------
ClusterAnalyser::ClusterAnalyser( QueryBufferf *points, float maxDistBetweenPts )
: m_grid(maxDistBetweenPts*2)
//-----------------------------------------------------------------------------
{
	m_queryBuffer = points;
	m_dist = maxDistBetweenPts;
	m_points = 0;
	m_numPoints = 0;
	m_offset.set(100,100,100);
}
//-----------------------------------------------------------------------------
ClusterAnalyser::~ClusterAnalyser()
//-----------------------------------------------------------------------------
{
	if (m_points)
		delete [] m_points;
}
//-----------------------------------------------------------------------------
int ClusterAnalyser::extractClusters(bool clusterPoints, bool clusterIndices)
//-----------------------------------------------------------------------------
{
	if (loadGrid())
	{
		computeClusters();

		if (m_points) 
			delete [] m_points;

		m_points = 0;
		m_numPoints = 0;

		return m_clusters.size();
	}
	return 0;
}
//-----------------------------------------------------------------------------
int ClusterAnalyser::extractFastClusters(bool clusterPoints, bool clusterIndices)
//-----------------------------------------------------------------------------
{
	if (loadGrid())
	{
		computeFastClusters();

		if (m_points) 
			delete [] m_points;

		m_points = 0;
		m_numPoints = 0;

		return m_clusters.size();
	}
	return 0;
}
//-----------------------------------------------------------------------------
int		ClusterAnalyser::computeClusters()
//-----------------------------------------------------------------------------
{
	GridSitesf				&sites = m_grid;

	// start with seed point
	Vector3f				seedPnt;
	std::vector<SIndexType>	pnts;
	double					D = m_dist;

	// identify neighborhoods
	int						num_sample_points = 0;
	SIndexType				seed_index;
	SKeyType				siteKey;

	int						pnts_processed = 0;
	std::queue<SIndexType>	front;	// moving front

	PTubyte					*clusters = new PTubyte[ m_numPoints ];
	std::vector<int>		clusterSizes;

	float					candidate_range = 2;
	int						clusterIndex = 1;

	memset(clusters, 0, m_numPoints);

	// get seed point
	sites.getNextValidPnt(seed_index, siteKey);
	front.push(seed_index);
	clusterSizes.push_back(0);

	double D2 = D*D;

	while (!front.empty() || sites.getNextValidPnt(seed_index, siteKey))
	{
		if (!front.empty())
		{
			seed_index = front.front();
			front.pop();
		}
		else 
		{
			//new cluster
			if (clusterSizes.back()>0)	// reuse old if zero points
			{
				clusterIndex++;
				clusterSizes.push_back(0);
			}
		}
			 
		if (seed_index > m_numPoints) break;	// sanity check

		seedPnt = m_points[seed_index];

		// remove this point
		Site *site = sites.getSite( siteKey );

		if (!site || !site->removePnt(seed_index)) 
			continue; // point has already been removed

		// get pnt pos
		++pnts_processed;

		pnts.clear();

		// get neighborhood and expand front
		sites.getPntNeighbourhood( seedPnt, D*candidate_range, pnts );

		std::vector< Vector3f > candidateFrontPts;	// track points in this front, so new front points can do quick test for proximity

		for (int i=0; i<pnts.size(); i++)
		{
			int index = pnts[i];
			Vector3f pnt( m_points[ index ] );

			float d = pnt.distanceSquared(seedPnt);

			if ( d < D2 )
			{
				// remove point
				Site *s = sites.getSite( sites.siteKey(pnt) );
				if (s)
				{
					s->removePnt(index);
				}

				++pnts_processed;

				clusters[index]=0;
			}
			else if(d < D2 *candidate_range && !clusters[index])
			{
				++pnts_processed;

				//bool tooCloseToFront=false;
				//// check if there is already a point in the front that is < min_dist from this
				//for (int j=0;j<candidateFrontPts.size();j++)
				//{
				//	if (candidateFrontPts[j].distanceSquared(pnt) < D2)
				//	{
				//		tooCloseToFront = true;
				//		break;
				//	}
				//}
				//if (tooCloseToFront)
				//{
				//	// discard point
				//	clusters[index]=0;
				//	Site *s = sites.getSite( sites.siteKey(pnt) );
				//	if (s) s->removePnt(index);
				//}
				//else 
				{
					//insert point into cluster
					seed_index = index;
					++num_sample_points;

					clusters[index]=clusterIndex;
					clusterSizes.back()++;

					front.push(index);
					candidateFrontPts.push_back(pnt);
				}
			}
			//// if more than 6 fronts already generated, its enough
			//if (candidateFrontPts.size() > 7) 
			//{
			//	// remove these points
			//	for (int j=i;j<pnts.size();j++)
			//	{
			//		int index = pnts[i];

			//		Site *s = sites.getSite( sites.siteKey(pnt) );
			//		if (s)
			//		{
			//			s->removePnt(index);
			//		}
			//		++pnts_processed;
			//		clusters[index]=0;
			//	}
			//	break;
			//}
		}
	}
	// extract clusters
	m_clusterSizes = clusterSizes;

	std::sort(m_clusterSizes.begin(), m_clusterSizes.end());

	for (int c=0; c<clusterSizes.size();c++)
	{
		int size = clusterSizes[c];
		Vector3f *clusterPts = new Vector3f[size];	
		clusterSizes[c]=0;	//use as a counter
		m_clusters.push_back( clusterPts );
	}
	for (int i=0; i<m_numPoints; i++)
	{
		int c = clusters[i];
		m_clusters[ c ][clusterSizes[c]] = m_points[i]-m_offset;
	}
	delete [] clusters;

	return num_sample_points;

}
//-----------------------------------------------------------------------------
PThandle ClusterAnalyser::resultsChannel()
//-----------------------------------------------------------------------------
{
	return 0;
}
//-----------------------------------------------------------------------------
bool ClusterAnalyser::loadGrid()
//-----------------------------------------------------------------------------
{
	PThandle				channel = ptGetChannelByName( L"diagnostic" );

	// feed grid the points
	m_grid.clear();

	m_numPoints = 0;

	if (m_queryBuffer)
	{
		// load Grid in first iteration
		while (m_queryBuffer->executeQuery())
		{
			const Vector3f *pnts = reinterpret_cast<const Vector3f*>(m_queryBuffer->getPointsBuffer());

			for (int i=0; i<m_queryBuffer->numPntsInQueryIteration(); i++)
			{
				Vector3f pnt(pnts[i]);
				pnt += m_offset;

				m_grid.addPoint(pnt, i+m_numPoints);
			}			
			m_numPoints += m_queryBuffer->numPntsInQueryIteration();
		}
		m_queryBuffer->resetQuery();

		if (m_points) delete [] m_points;
		
		try
		{
			m_points = new Vector3f[m_numPoints];
		}
		catch(std::bad_alloc)
		{
			m_points = 0;
			return false;
		}
		
		while ( m_queryBuffer->executeQueryChannel( channel) )
		{
			const Vector3f *pnts = reinterpret_cast<const Vector3f*>(m_queryBuffer->getPointsBuffer());

			for (int i=0; i<m_queryBuffer->numPntsInQueryIteration(); i++)
			{
				m_points[i] = pnts[i]+m_offset;
			}
		}
	}
	else return false;
	return true;
}
//-----------------------------------------------------------------------------
int ClusterAnalyser::numClusters() const
//-----------------------------------------------------------------------------
{
	return m_clusters.size();
}
//-----------------------------------------------------------------------------
Vector3f* ClusterAnalyser::getPointsCluster( int clusterIndex, int &clusterSize )
//-----------------------------------------------------------------------------
{
	if (clusterIndex >= m_clusters.size()) return 0;

	clusterSize = m_clusterSizes[clusterIndex];
	return m_clusters[clusterIndex];
}
//-----------------------------------------------------------------------------
// Compute clusters based on grid neighbors
//-----------------------------------------------------------------------------
int ClusterAnalyser::computeFastClusters()
//-----------------------------------------------------------------------------
{
	GridSitesf				&sites = m_grid;

	// start with seed point
	Vector3f				seedPnt;
	std::vector<SIndexType>	pnts;
	double					D = m_dist;

	// identify neighborhoods
	SIndexType				seed_index;
	SKeyType				siteKey;

	std::queue<Site*>		front;	// moving front

	PTubyte					*clusters = new PTubyte[ m_numPoints ];
	std::vector<int>		clusterSizes;

	float					candidate_range = 2;
	int						clusterIndex = 1;

	Site					*site = 0;

	PThandle				channel = ptGetChannelByName( L"diagnostic" );

	memset(clusters, 0, m_numPoints);

	// get seed point
	sites.getNextValidPnt(seed_index, siteKey);
	front.push(sites.getSite(siteKey));

	clusterSizes.push_back(0);

	while (!front.empty() || sites.getNextValidPnt(seed_index, siteKey))
	{
		if (!front.empty())
		{
			site = front.front();
			front.pop();
		}
		else
		{
			//new cluster
			if (clusterSizes.back()>0)	// reuse old if zero points
			{
				clusterIndex++;
				clusterSizes.push_back(0);
			}
			site = sites.getSite(siteKey);
		}
		
		//add all points from this site into the cluster
		
		if (site)
		{
			std::vector<SIndexType> pnt_indices;
			site->getPnts(pnt_indices);

			if (pnt_indices.size())
			{
				clusterSizes.back() += pnt_indices.size();

				for (int i=0; i<pnt_indices.size(); i++)
				{
					clusters[ pnt_indices[i] ] = clusterIndex;					
					m_queryBuffer->setChannelValue( pnt_indices[i], clusterIndex );	// diagnostics
				}

				site->clear();	

				for (int i=-1; i<2; i++)
				for (int j=-1; j<2; j++)
				for (int w=-1; w<2; w++)
				{
					if (i||j||w)
					{
						site = sites.getSiteNeighbour( siteKey, i,j,w );
						if (site)
							front.push(site);
					}
				}
			}
		}
	}
	// extract clusters
	m_clusterSizes = clusterSizes;

	// sort largest first
	//std::sort(m_clusterSizes.end(), m_clusterSizes.begin());

	//for (int c=0; c<clusterSizes.size();c++)
	//{
	//	int size = clusterSizes[c];
	//	Vector3f *clusterPts = new Vector3f[size];	
	//	clusterSizes[c]=0;	//use as a counter
	//	m_clusters.push_back( clusterPts );
	//}
	//for (int i=0; i<m_numPoints; i++)
	//{
	//	int c = clusters[i]-1;
	//	m_clusters[ c ][clusterSizes[c]++] = m_points[i]-m_offset;
	//}
	delete [] clusters;

	ptSubmitPointChannelUpdate( m_queryBuffer->query(), channel );

	return m_numPoints;

}
