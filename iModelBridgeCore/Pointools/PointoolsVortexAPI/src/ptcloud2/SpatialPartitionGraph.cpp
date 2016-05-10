#include "PointoolsVortexAPIInternal.h"

#include <vector>
#include <map>

#include <ptcloud2/SpatialPartitionGraph.h>

using namespace SpatialPartitioner;


namespace SpatialPartitioner
{

template<class ContainerType> void testSpatialPartitioner(void)
{
	typedef SpatialPartitioner::Types<float>::Vector3				Point;
	typedef std::vector<SpatialPartitioner::Types<float>::Vector3 >	PointSet;

	SpatialPartitionGraph<Point, PointSet>	manager;

	ContainerType			spatialPartitionContainer;
	typename ContainerType::Leaf		spatialPartitionContainerLeaf;

	SpatialPartitioner::Types<double>::Vector3	p(0, 0, 0);
	pt::BoundingBox								e;


	spatialPartitionContainer.getNode(0)->classify(p, e);

	spatialPartitionContainerLeaf.getNode(0)->classify(p, e);


}


void testSpatialPartitioner(void)
{
																// Force test code compilation for all node types
	testSpatialPartitioner<ContainerH1KDX<float, NodeDataNULL> >();
	testSpatialPartitioner<ContainerH1KDY<float, NodeDataNULL> >();
	testSpatialPartitioner<ContainerH1KDZ<float, NodeDataNULL> >();
																// KD Nodes (Positioned Partition)
	testSpatialPartitioner<ContainerH1KDPosX<float, NodeDataNULL> >();
	testSpatialPartitioner<ContainerH1KDPosY<float, NodeDataNULL> >();
	testSpatialPartitioner<ContainerH1KDPosZ<float, NodeDataNULL> >();
																// Quadtree (Central Partition)
	testSpatialPartitioner<ContainerH1Quadtree<float, NodeDataNULL> >();
																// Quadtree (Positioned Partition)
	testSpatialPartitioner<ContainerH1QuadtreePos<float, NodeDataNULL> >();
																// Octree (Central Partition)
	testSpatialPartitioner<ContainerH1Octree<float, NodeDataNULL> >();
																// Octree (Positioned Partition)
	testSpatialPartitioner<ContainerH1OctreePos<float, NodeDataNULL> >();
																// BSP Tree (2D planes)
	testSpatialPartitioner<ContainerH1BSP2D<float, NodeDataNULL> >();
																// BSP Tree (3D planes)
	testSpatialPartitioner<ContainerH1BSP3D<float, NodeDataNULL> >();

	testSpatialPartitioner<ContainerH1Grid<float, NodeDataNULL> >();

	SpatialPartitioner::SpatialPartitionGraph<Types<float>::Vector3, Types<float>::Vector3Set>	manager;

	manager.setRoot(manager.createContainer<SpatialPartitioner::ContainerH1Grid<float, NodeDataNULL> >());
}



template<class Point, class PointSet> SpatialPartitionGraph<Point, PointSet>::SpatialPartitionGraph(void)
{
	setRoot(NULL);
}

template<class Point, class PointSet> SpatialPartitionGraph<Point, PointSet>::~SpatialPartitionGraph(void)
{
	deleteAll();
}

#if defined (NEEDS_WORK_VORTEX_DGNDB)
template<class Point, class PointSet> Container *SpatialPartitionGraph<Point, PointSet>::createNode(NodeType type)
{
	typedef float	Real;

	switch(type)
	{
															// KD Nodes (Central Partition)
	case NodeTypeKDX:
		return createNode<SpatialPartitionNodeKDX<Real> >();

	case NodeTypeKDY:
		return createNode<SpatialPartitionNodeKDY<Real> >();

	case NodeTypeKDZ:
		return createNode<SpatialPartitionNodeKDZ<Real> >();
															// KD Nodes (Positioned Partition)
	case NodeTypeKDPosX:
		return createNode<SpatialPartitionNodeKDPosX<Real> >();

	case NodeTypeKDPosY:
		return createNode<SpatialPartitionNodeKDPosY<Real> >();

	case NodeTypeKDPosZ:
		return createNode<SpatialPartitionNodeKDPosZ<Real> >();
															// Quadtree (Central Partition)
	case NodeTypeQuadtree:
		return createNode<NodeQuadtree<Real> >();
															// Quadtree (Positioned Partition)
	case NodeTypeQuadtreePos:
		return createNode<SpatialPartitionNodeQuadtreePos<Real> >();
															// Octree (Central Partition)
	case NodeTypeOctree:
		return createNode<NodeOctree<Real> >();
															// Octree (Positioned Partition)
	case NodeTypeOctreePos:
		return createNode<NodeOctreePos<Real> >();
															// BSP Tree (2D planes)
	case NodeTypeBSP2D:
		return createNode<NodeBSP2D<Real> >();
															// BSP Tree (3D planes)
	case NodeTypeBSP3D:
		return createNode<NodeBSP3D<Real> >();

	default:
		return NULL;
	}
}
#endif





}
