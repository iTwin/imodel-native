#pragma once
#include <ptengine/queryTraversal.h>
#include <ptengine/pointsScene.h>
#include <ptengine/engine.h>

namespace pointsengine
{
	// traverse each scene, then point cloud
	// then top down octree traversal
	class OctreeTraversal : public QueryTraversal
	{
	public:
		OctreeTraversal(pt::CoordinateSpace cs=pt::ProjectSpace)
			: QueryTraversal(cs)
		{}
		
		OctreeTraversal( pcloud::PointCloud *cloud, pt::CoordinateSpace cs=pt::ProjectSpace ) 
			: QueryTraversal(cloud,cs)
		{}

		OctreeTraversal( pcloud::Scene *scene, pt::CoordinateSpace cs=pt::ProjectSpace )
			: QueryTraversal(scene,cs)
		{}

		template <class Receiver, class DensityPolicy>
		void traverse( Receiver &R, DensityPolicy &D=DensityPolicy() )
		{
			if (m_sceneScope)
			{
				traverseScene( m_sceneScope, R, D );
			}
			else if (m_cloudScope)
			{
				traverseCloud( m_cloudScope, R, D );
			}
			else
			{
				int numScenes = thePointsScene().size();
				
				for (int sc=0; sc<numScenes; sc++)
				{
					pcloud::Scene *scene = thePointsScene()[sc];
					traverseScene( scene, R, D );
				}
			}
		}

		template <class Receiver >
		void traverse( Receiver &R )
		{
			traverse( R, CurrentDensity() );
		}

		template <class Receiver, class DensityPolicy>
		void traverseScene( pcloud::Scene *scene, Receiver &R, DensityPolicy &D )
		{
			if (R.scene( scene ))
			{
				for (uint pc=0;pc<scene->size(); pc++)
				{		
					pcloud::PointCloud *cloud = scene->cloud(pc);					
					traverseCloud(cloud, R, D );
				}	
			}
		}

		template <class Receiver, class DensityPolicy>
		void traverseCloud( pcloud::PointCloud *cloud, Receiver &R, DensityPolicy &D )
		{
			if (R.cloud( cloud ))
			{
				pcloud::Node *node = cloud->root();
				traverseNode( node, R, D );
			}
		}

		template <class Receiver, class DensityPolicy>
		void traverseNode( pcloud::Node *node, Receiver &R, DensityPolicy &D )
		{
			if (R.node( node ))
			{
				if (node->isLeaf())
				{
					traverseVoxel( static_cast<pcloud::Voxel*>(node), R, D );
				}
				else
				{
					for (int i=0; i<8; i++)
					{
						if (node->child(i)) 
						{
							traverseNode( node->child(i), R, D );
						}
					}
				}
			}
		}
	};

// traverse each scene, then point cloud
	// then top down octree traversal
	class ConstOctreeTraversal : public ConstQueryTraversal
	{
	public:
		ConstOctreeTraversal(pt::CoordinateSpace cs=pt::ProjectSpace)
			: ConstQueryTraversal(cs)
		{}
		
		ConstOctreeTraversal( const pcloud::PointCloud *cloud, pt::CoordinateSpace cs=pt::ProjectSpace ) 
			: ConstQueryTraversal(cloud,cs)
		{}

		ConstOctreeTraversal( const pcloud::Scene *scene, pt::CoordinateSpace cs=pt::ProjectSpace )
			: ConstQueryTraversal(scene,cs)
		{}

		template <class Receiver, class DensityPolicy>
		void traverse( Receiver &R, DensityPolicy D=DensityPolicy() )
		{
			if (m_sceneScope)
			{
				traverseScene( m_sceneScope, R, D );
			}
			else if (m_cloudScope)
			{
				traverseCloud( m_cloudScope, R, D );
			}
			else
			{
				int numScenes = thePointsScene().size();
				
				for (int sc=0; sc<numScenes; sc++)
				{
					const pcloud::Scene *scene = thePointsScene()[sc];
					traverseScene( scene, R, D );
				}
			}
		}

		template <class Receiver>
		void traverse( Receiver &R )
		{
			traverse( R, CurrentDensity() );
		}

		template <class Receiver, class DensityPolicy>
		void traverseScene( const pcloud::Scene *scene, Receiver &R, DensityPolicy &D )
		{
			if (R.scene( scene ))
			{
				for (int pc=0;pc<scene->size(); pc++)
				{		
					const pcloud::PointCloud *cloud = scene->cloud(pc);					
					traverseCloud(cloud, R, D );
				}	
			}
		}

		template <class Receiver, class DensityPolicy>
		void traverseCloud( const pcloud::PointCloud *cloud, Receiver &R, DensityPolicy &D )
		{
			if (R.cloud( cloud ))
			{
				const pcloud::Node *node = cloud->root();
				traverseNode( node, R, D );
			}
		}

		template <class Receiver, class DensityPolicy>
		void traverseNode( const pcloud::Node *node, Receiver &R, DensityPolicy &D )
		{
			if (R.node( node ))
			{
				if (node->isLeaf())
				{
					traverseVoxel( static_cast<const pcloud::Voxel*>(node), R, D );
				}
				else
				{
					for (int i=0; i<8; i++)
					{
						if (node->child(i)) 
						{
							traverseNode( node->child(i), R, D );
						}
					}
				}
			}
		}
	};
}