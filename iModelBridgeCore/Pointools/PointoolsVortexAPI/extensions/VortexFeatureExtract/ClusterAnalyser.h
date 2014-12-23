#include "Includes.h"
#include "Sites.h"
#include "QueryBuffer.h"

#pragma once

// this class is NOT intended for export

namespace vortex
{
	class ClusterAnalyser
	{
	public:
		ClusterAnalyser( QueryBufferf *points, float maxDistBetweenPts );
		~ClusterAnalyser();

		int				extractClusters(bool clusterPoints, bool clusterIndices);
		int				extractFastClusters(bool clusterPoints, bool clusterIndices);
		PThandle		resultsChannel();
		
		int				numClusters() const;

		Vector3f*		getPointsCluster(int clusterIndex, int &clusterSize);
		PTuint*			getIndicesCluster(int clusterIndex, int &clusterSize);

	private:

		bool			loadGrid();
		int				computeClusters();
		int				computeFastClusters();

		QueryBufferf			*m_queryBuffer;

		PThandle				m_resChannel;
		PTfloat					m_dist;

		GridSitesf				m_grid;
		Vector3f				*m_points;	
		int						m_numPoints;
		Vector3f				m_offset;

		std::vector<int>		m_clusterSizes;
		std::vector<Vector3f*>	m_clusters;
		std::vector<PTuint*>	m_clusterIndices;
	};
}