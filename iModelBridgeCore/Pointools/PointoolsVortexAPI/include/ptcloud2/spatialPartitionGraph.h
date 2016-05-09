// ***************************************************************************************
// Module		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	08 December 2009
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************

#ifndef _SPATIAL_PARTITION_GRAPH
#define _SPATIAL_PARTITION_GRAPH



// ***************************************************************************************
// Defines
// ***************************************************************************************



// ***************************************************************************************
// Includes
// ***************************************************************************************

// C
#include <vector>


// ***************************************************************************************
// Class		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	08 December 2009
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************



/*
- Node

	- Number of children
	- Child nodes

- SpatialFilter


- SpatialPartitioner
	- Is a type of SpatialFilter
	- Specializes in a particular partitioning
	- Stores any data necessary to parameterize and represent geometry

- Filter Types
	- Only work when points may be modified
	- Project points to plane

- Partitioner Types

	- Must partition 2D and 3D points

	- KD
		- (Axis specified, partition centered)				(No storage required)
		- (Axis specified, partition position specified)	(Partition position required)

	- Quadtree

	- Octree

	- BSP Tree

	- Grid 2D
		- Specify number of regular partitions on X and Y

	- Grid 3D
		- Specify number of regular partitions on X, Y and Z


- Extents
	- Extents of volume being filtered down and used for partitioning

	- Each node type must be able to take parent's extents and clip them for each child

	- Switch between partition volume extents and data extents as a general solution

	- Quadtree and Octree Nodes
		- Know their own node extents
		- Subdivide their node extents and pass to their child nodes

	- BSP tree nodes
		- Know their own node extents
		- Form two complex half spaces after subdivision
		- Pass data's extents in each half space down to child nodes

	- Overlap problem
		- BSP etc. forms partitions with overlapping volumes. This must be known by algorithms using system.


- Child number problem
	- Grid has specifiable number of children at runtime and can not be present in templates
*/





/*

	- SpatialPartitionNode has Partitioner
	- Partitioner has one, two or three Partitions

*/

// C
#include <deque>

// Local
#include <math/matrix_math.h>
#include <pt/BoundingBox.h>

const double ZERO_OVERLAP_THRESHOLD = 0.000001;


namespace SpatialPartitioner
{

// Pip Test Begin
void testSpatialPartitioner(void);
// Pip Test End

class Container;


enum Axis
{
	AxisX		= 0,
	AxisY		= 1,
	AxisZ		= 2,

	AxisEnd		= 3,

	AxisNULL	= ~0,
};

typedef unsigned int NodeType;

enum SpatialPartitionNodeType
{
															// NULL node type
	NodeTypeNULL = 0,
															// KD Nodes (Central Partition)
	NodeTypeKDX,
	NodeTypeKDY,
	NodeTypeKDZ,
															// KD Nodes (Positioned Partition)
	NodeTypeKDPosX,
	NodeTypeKDPosY,
	NodeTypeKDPosZ,
															// Quadtree (Central Partition)
	NodeTypeQuadtree,
															// Quadtree (Positioned Partition)
	NodeTypeQuadtreePos,
															// Octree (Central Partition)
	NodeTypeOctree,
															// Octree (Positioned Partition)
	NodeTypeOctreePos,
															// BSP Tree (2D planes)
	NodeTypeBSP2D,
															// BSP Tree (3D planes)
	NodeTypeBSP3D,
															// Grid (1D, 2D or 3D)
	NodeTypeGrid,
															// End Marker
	NodeTypeEND
};


typedef unsigned long ChildIndex;
typedef unsigned long NodeIndex;

typedef std::vector<ChildIndex>	ChildIndexSet;


const ChildIndex CHILD_INDEX_NULL = ~0;

class NodeDataNULL		{};


template<class Real> class Types
{

public:

	typedef matrix<2, 1, Real>		Vector2;
	typedef matrix<3, 1, Real>		Vector3;
	typedef matrix<4, 1, Real>		Vector4;

	typedef pt::BoundingBox			Extents;

	typedef std::deque<Vector2>		Vector2Set;
	typedef std::deque<Vector3>		Vector3Set;
	typedef std::deque<Vector4>		Vector4Set;
};

class SpatialPartitionNode
{
public:

	virtual ChildIndex					classify(SpatialPartitioner::Types<double>::Vector3 &point, SpatialPartitioner::Types<float>::Extents &extents) = 0;
	virtual unsigned int				classify(SpatialPartitioner::Types<double>::Vector3 &point, SpatialPartitioner::Types<float>::Extents &extents, ChildIndexSet &result) = 0;

	virtual NodeType					getType(void) = 0;

	virtual ChildIndex					getMaxChildren(void) = 0;
};

template<typename Real, typename Data> class SpatialPartitionNodeTyped : public SpatialPartitionNode, public Data
{
public:


};

class NodeH;

class Visitor
{
public:

		virtual bool	visitNode	(NodeH &node) = 0;
};


class Container
{


public:

	virtual void							setChild				(ChildIndex index, Container *container) = 0;
	virtual Container					*	getChild				(ChildIndex index)	= 0;

	virtual NodeH							getChildH				(NodeIndex node, ChildIndex child) = 0;

	virtual ChildIndex						getMaxChildren			(void) = 0;

	virtual	bool							isLeaf					(void) = 0;
	virtual	bool							isLeaf					(NodeIndex node) = 0;
	virtual bool							isStrictPartitioner		(void)				{return true;}

	virtual SpatialPartitionNode		  *	getNode					(NodeIndex index) = 0;
};


class NodeH // Note: Derive this later to implement interface : public SpatialPartitioner::SpatialPartitionNode
{
protected:

	Container *		container;
	NodeIndex		node;

public:
								NodeH			(void)						{setContainer(NULL); setNode(0);}
								NodeH			(Container *initContainer, NodeIndex initNode) {setContainer(initContainer); setNode(initNode);}

	void						setNode			(NodeIndex initNode)		{node = initNode;}
	SpatialPartitionNode	*	getNode			(void)						{assert(container); return container->getNode(node);}

	void						setContainer	(Container *initContainer)	{container = initContainer;}
	Container				*	getContainer	(void)						{return container;}

	NodeH						getChild		(ChildIndex child)			{assert(container); return container->getChildH(node, child);}
	bool						isLeaf			(void)						{assert(container); return container->isLeaf(node);}

	void						setInvalid		(void)						{setContainer(NULL);}
	bool						isValid			(void)						{return (container != NULL);}

	template<class N> N		*	getNode			(void)						{return dynamic_cast<N *>(getNode());}
};

typedef std::deque<NodeH>	NodeHSet;


template<class Point, class PointSet> class SpatialPartitionGraph
{
protected:

	Container *												root;

public:

															SpatialPartitionGraph		(void);
														   ~SpatialPartitionGraph		(void);

	template<class PartitionNode> unsigned int				partition					(PointSet &source);

	void													insert						(PointSet &source);


															// Node Creation and Deletion

	Container											*	createNode					(NodeType nodeType);

	template<class N> N *createContainer(void)
	{
		return new N();
	}

	template<class N> N *createNode(void)
	{
		return new N();
	}


	void deleteContainer(Container *node)
	{
		if(node)
			delete node;
	}

	void deleteGraph(Container *c)
	{
															// Note: ADD HANDLING FOR MULTIPLE PARENT DANGLING PTRS
		if(c)
		{
			ChildIndex		child;
			Container	*	childContainer;
															// Delete all child containers first
			for(child = 0; child < c->getMaxChildren(); child++)
			{
				if((childContainer = c->getChild(child)) != NULL)
				{
					deleteGraph(childContainer);
				}
			}
															// Delete given container
			deleteContainer(c);
		}
	}

	void deleteAll(void)
	{
															// Delete graph from root downward
		deleteGraph(getRootContainer());
															// No container present
		setRoot(NULL);
	}

	void setRoot(Container *node)
	{
		root = node;
	}
															
	Container *getRootContainer(void)
	{
															// Return root container
		return root;
	}

	NodeH getRootNode(void)
	{
															// Return handle to root node
		return NodeH(getRootContainer(), 0);
	}

	void visitDepthFirst(Visitor &visitor, NodeH &node, NodeType type = NodeTypeNULL)
	{
															// If node is valid
		if(node.isValid())
		{
															// If type not specified OR type matches requested
			if(type == NodeTypeNULL || type == node.getNode()->getType())
			{
				NodeH	childH;
																// Call client supplied callback
				if(visitor.visitNode(node))
				{
																// If visitor returns true, continue to children
					ChildIndex	child;
					ChildIndex	maxChildren = node.getNode()->getMaxChildren();
																	// Visit each child node
					for(child = 0; child < maxChildren; child++)
					{
						childH = node.getChild(child);
	
						visitDepthFirst(visitor, childH, type);
					}
				}
			}
		}
	}

	void listDepthFirst(NodeHSet &result, NodeH &node, NodeType type = NodeTypeNULL)
	{
															// If node is valid
		if(node.isValid())
		{															// If type not specified OR type matches requested
			if(type == NodeTypeNULL || type == node.getNode()->getType())
				result.push_back(node);

			NodeH	childH;
															// If visitor returns true, continue to children
			ChildIndex	child;
			ChildIndex	maxChildren = node.getNode()->getMaxChildren();
															// Visit each child node
			for(child = 0; child < maxChildren; child++)
			{
				childH = node.getChild(child);

				listDepthFirst(result, childH, type);
			}
		}
	}

	bool isEmpty(void)
	{
		return (getRootContainer() == NULL);
	}

	NodeH classify(Types<double>::Vector3 &point, Types<double>::Extents &boundingBox)
	{
		NodeH		node;
		NodeH		result;
		ChildIndex	child;

		node = getRootNode();

		while(node.isValid())
		{
			if(node.isLeaf())
				return node;

			if((child = node.getNode()->classify(point, boundingBox)) != CHILD_INDEX_NULL)
			{
				node = node.getChild(child);
			}
			else
			{
				node.setInvalid();
			}

			return node;
		}
															// Return NULL node
		return node;
	}


	unsigned int classify(Types<double>::Vector3 &point, Types<double>::Extents &boundingBox, NodeHSet &result)
	{
		NodeH				node;
		//ChildIndex			numChildren;
		ChildIndexSet		children;
		NodeHSet			nodes[2];
		unsigned int		t;
		unsigned int		c;
		unsigned int		numNodesRefined;

		NodeHSet		*	source	= &(nodes[0]);
		NodeHSet		*	dest	= &(nodes[1]);

		source->push_back(getRootNode());

		do
		{
			numNodesRefined = 0;

			if(source->empty() == false)
			{
				dest->clear();

				for(t = 0; t < source->size(); t++)
				{
					node = (*source)[t];
															// If node is valid
					if(node.isValid())
					{
						if(node.isLeaf() == false)
						{
							ChildIndexSet	children;
															// Add child classifications to dest
							node.getNode()->classify(point, boundingBox, children);
															// Add Node handles to destination
							for(c = 0; c < children.size(); c++)
							{
								NodeH child = node.getChild(children[c]);

								dest->push_back(child);
							}

							numNodesRefined++;
						}
						else
						{
							result.push_back(node);
						}
					}
				}
			}
															// Dest becomes source
			swap(source, dest);

		} while(source->empty() == false && numNodesRefined > 0);

															// Return num results
		return static_cast<uint>(result.size());
	}

};


template<typename Real, class PartitionNode, unsigned int numNodes> class ContainerHLeaf : public Container
{

public:

protected:
			PartitionNode				nodes[numNodes];

public:

			bool						isLeaf				(void)					{return true;}

			void						setChild			(ChildIndex index, Container *container) {}
			Container				  *	getChild			(ChildIndex index)		{return NULL;}

			ChildIndex					getMaxChildren		(void)					{return 0;}

			NodeType					getPartitionerType	(void)					{return PartitionNode::getType();}

			NodeH getChildH(NodeIndex node, ChildIndex child)
			{
				// Implement this properly for non H1 containers

				return NodeH(getChild(child), 0);
			}

			bool isLeaf(NodeIndex node)
			{
				// Implement this properly for non H1 containers

				return true;
			}

			SpatialPartitionNode *getNode(NodeIndex index)
			{
				if(index < numNodes)
					return &(nodes[index]);

				return NULL;
			}
};

template<typename Real, class PartitionNode, unsigned int numNodes = 1, unsigned int maxChildren = PartitionNode::maxChildren> class ContainerH : public ContainerHLeaf<Real, PartitionNode, numNodes>
{

public:

	typedef ContainerHLeaf<Real, PartitionNode, numNodes>	Leaf;

protected:

			Container		*	children[maxChildren];

protected:

			bool								isValidIndex		(ChildIndex index)		{return index < maxChildren;}

public:

			Container						*	getChild			(ChildIndex index)		{assert(isValidIndex(index)); return children[index];}

			NodeType							getPartitionerType	(void)					{return PartitionNode::getType();}

			bool isLeaf(void)
			{
				ChildIndex	child;

				for(child = 0; child < maxChildren; child++)
				{
					if(getChild(child) != NULL)
						return false;
				}

				return true;
			}

			void setChild(ChildIndex index, Container *container)
			{
				if(index < getMaxChildren())
					children[index] = container;
			}

			SpatialPartitionNode *getNode(NodeIndex index)
			{
				if(index < numNodes)
					return &(nodes[index]);

				return NULL;
			}
};


template<typename Real, class PartitionNode> class ContainerHvLeaf : public Container
{
protected:

	typedef std::vector<PartitionNode>					NodeSet;

protected:

	NodeSet							nodes;

public:

	void setMaxNodes(NodeIndex maxNodes)
	{
		nodes.resize(maxNodes);
	}

	NodeIndex getMaxNodes(void)
	{
        return static_cast<NodeIndex>(nodes.size());
	}

	void							setMaxChildren	(ChildIndex maxChildren) {}
	ChildIndex						getMaxChildren	(void) {return 0;}
	void							setChild		(ChildIndex index, Container *child) {}
	Container					*	getChild		(ChildIndex index) {return NULL;}

	bool							isLeaf(void)
	{
		 // Note: Implement properly

		return false;
	}

	bool isLeaf(NodeIndex node)
	{
		// Note: Implement properly

		return true;
	}

	SpatialPartitionNode *getNode(NodeIndex index)
	{
		assert(index < getMaxNodes());

		return &(nodes[index]);
	}

	NodeH getChildH(NodeIndex node, ChildIndex child)
	{
		// NOTE: Implement this properly for non H1 hierarchies
		return NodeH(NULL, 0);
	}
};


template<typename Real, class PartitionNode> class ContainerHv : public ContainerHvLeaf<Real, PartitionNode>
{
public:
	typedef ContainerHvLeaf<Real, PartitionNode>	Leaf; 

protected:

	typedef std::vector<Container *>				ChildSet;

protected:

			ChildSet						children;

public:

			bool isLeaf(void)
			{
				// Note: Implement

				return false;
			}

			bool isLeaf(NodeIndex node)
			{
				// Note: Implement

				return false;
			}

			NodeH getChildH(NodeIndex node, ChildIndex child)
			{
				// Note: Implement this properly for non H1 hierarchies
				return NodeH(getChild(child), 0);
			}

			void setMaxChildren(ChildIndex maxChildren)
			{
				children.resize(maxChildren);
			}

			ChildIndex getMaxChildren(void)
			{
				return static_cast<ChildIndex>(children.size());
			}


			void setChild(ChildIndex index, Container *child)
			{
				if(index < getMaxChildren())
					children[index] = child;
			}

			Container *getChild(ChildIndex index)
			{
				if(index < getMaxChildren())
					return children[index];

				return NULL;
			}

};

// ************************************************************************************************
// Spatial Partition Geometry
// ************************************************************************************************

template<class Real> class PartitionGeometryBase
{
public:

typedef matrix<2, 1, Real>						Vector2;
typedef matrix<3, 1, Real>						Vector3;
typedef matrix<4, 1, Real>						Vector4;

protected:

public:


};


template<class Real, unsigned int D> class PartitionGeometryHyperPlane : public PartitionGeometryBase<Real>
{

public:

	static const unsigned int partitionCoef = 2;

public:

typedef matrix<D, 1, double>	Vector;

protected:

		Vector					normal;
		Real					constant;

public:

	PartitionGeometryHyperPlane(void)
	{

	}

	PartitionGeometryHyperPlane(Vector &n, Real c)
	{
		normal		= n;
		constant	= c;
	}

	PartitionGeometryHyperPlane(Vector &n, Vector &point)
	{
		normal = n;
		normal.normalize();

		setConstant(-normal.dot(point));
	}

	template<unsigned int VD> ChildIndex classify(matrix<VD, 1, double> &point)
	{
		return static_cast<ChildIndex>(normal.dot(point) + constant);
	}

	template<unsigned int VD> ChildIndex classify(matrix<VD, 1, double> &point, typename Types<Real>::Extents &extents)
	{
		return classify(point);
	}
};



// ************************************************************************************************

/*

- P Dimensions

	- 1 at a time	KD
	- 2 at a time	Quadtree
	- 3 at a time	Octree

*/


class PartitionNull
{
public:
	const static unsigned int	partitionCoef = 1;
};

class Partition2Null : public PartitionNull {};
class Partition3Null : public PartitionNull {};

template<class Real, unsigned int axis> class PartitionAligned
{
public:
	const static unsigned int	partitionCoef = 2;

public:

	ChildIndex classify(typename Types<double>::Vector3 &point, typename Types<Real>::Extents &extents)
	{
															// Return partition child index (without logic branches)
		return static_cast<ChildIndex>(point(axis, 0) <= ((extents.upper(axis) + extents.lower(axis)) * 0.5));
	}
};

template<class Real, unsigned int axis, class Positioner> class PartitionAlignedPositioned : public PartitionAligned<Real, axis>
{

protected:

	Positioner	position;

public:

	ChildIndex classify(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
		return static_cast<ChildIndex>(point(axis, 0) <= position);
	}

};

// ************************************************************************************************
// Spatial Partitioner
// ************************************************************************************************


template<class Real, class Data, NodeType type, class P1, class P2 = Partition2Null, class P3 = Partition3Null> class NodeD : public SpatialPartitionNodeTyped<Real, Data>, public P1, public P2, public P3
{

public:

	const static unsigned int maxChildren = P1::partitionCoef * P2::partitionCoef * P3::partitionCoef;

public:


	ChildIndex classify(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
		return classify<maxChildren>(point, extents);
	}

	unsigned int classify(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents, ChildIndexSet &result)
	{
		// Implement this
		return 0;		
	}

	template<unsigned int> ChildIndex classify(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
		// Implement this
		return 0;
	}

	template<> ChildIndex classify<2>(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
															// Create mapping onto two children (binary)
		return P1::classify(point, extents);
	}

	template<> ChildIndex classify<4>(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
															// Create unique mapping onto four children (quad)
		return (P2::classify(point, extents) * 2) + P1::classify(point, extents);
	}

	template<> ChildIndex classify<8>(typename Types<double>::Vector3 &point, typename Types<double>::Extents &extents)
	{
															// Create unique mapping onto eight children (oct)
		return (P3::classify(point, extents) * 4) + (P2::classify(point, extents) * 2) + P1::classify(point, extents);
	}

	NodeType getType(void)
	{
		return type;
	}

	ChildIndex getMaxChildren(void)
	{
		return maxChildren;
	}
};


// ************************************************************************************************
// Specific Partition Types
// ************************************************************************************************

template<class Real> class PartitionAlignedX	: public PartitionAligned<Real, AxisX> {};
template<class Real> class PartitionAlignedY	: public PartitionAligned<Real, AxisY> {};
template<class Real> class PartitionAlignedZ	: public PartitionAligned<Real, AxisZ> {};

template<class Real> class PartitionAlignedPosX : public PartitionAlignedPositioned<Real, AxisX, Real> {};
template<class Real> class PartitionAlignedPosY : public PartitionAlignedPositioned<Real, AxisY, Real> {};
template<class Real> class PartitionAlignedPosZ : public PartitionAlignedPositioned<Real, AxisZ, Real> {};

template<class Real> class PartitionLine		: public PartitionGeometryHyperPlane<Real, 2> {};
template<class Real> class PartitionPlane		: public PartitionGeometryHyperPlane<Real, 3> {};


// ************************************************************************************************
// Specific Spatial Partitioners
// ************************************************************************************************


template<class Real, class Data, NodeType type, unsigned int axis> class NodeKD : public NodeD<Real, Data, type, PartitionAligned<Real, axis> >
{

};

template<class Real, class Data, NodeType type, unsigned int axis> class NodeKDPos : public NodeD<Real, Data, type, PartitionAlignedPositioned<Real, axis, Real> >
{

};


template<class Real, class Data> class NodeQuadtree : public NodeD<Real, Data, NodeTypeQuadtree, PartitionAlignedX<Real>, PartitionAlignedY<Real>, Partition3Null>
{

};

template<class Real, class Data> class NodeQuadtreePos : public NodeD<Real, Data, NodeTypeQuadtreePos, PartitionAlignedPosX<Real>, PartitionAlignedPosY<Real>, Partition3Null>
{

};


template<class Real, class Data> class NodeOctree : public NodeD<Real, Data, NodeTypeOctree, PartitionAlignedX<Real>, PartitionAlignedY<Real>, PartitionAlignedZ<Real> >
{

};

template<class Real, class Data> class NodeOctreePos : public NodeD<Real, Data, NodeTypeOctreePos, PartitionAlignedPosX<Real>, PartitionAlignedPosY<Real>, PartitionAlignedPosZ<Real> >
{

};


template<class Real, class Data> class NodeBSP2D : public NodeD<Real, Data, NodeTypeBSP2D, PartitionLine<Real> >
{

};

template<class Real, class Data> class NodeBSP3D : public NodeD<Real, Data, NodeTypeBSP3D, PartitionPlane<Real> >
{

};

template<class Real, class Data> class NodeGrid : public SpatialPartitionNodeTyped<Real, Data>
{

public:

	typedef NodeGrid<Real, Data>			this_type;

	typedef ContainerHv<Real, this_type>	ContainerH1;

protected:

		ChildIndex				numCells[3];
		matrix<3,1, double>		overlapDistance;
		bool					overlapDefined;

public:

		NodeGrid(void)
		{
			setNumCells(NULL, 0, 0, 0);
			setOverlapDistance(0, 0, 0);
		}

		void setNumCells(ContainerH1 *container, ChildIndex numX, ChildIndex numY, ChildIndex numZ)
		{
															// Resize the number of children in the grid's container
			if(container)
				container->setMaxChildren(numX * numY * numZ);
															// Set the number of cells
			numCells[0] = numX;
			numCells[1] = numY;
			numCells[2] = numZ;
		}

		void getNumCells(ChildIndex &numX, ChildIndex &numY, ChildIndex &numZ)
		{
			numX = numCells[0];
			numY = numCells[1];
			numZ = numCells[2];
		}

		void setOverlapDistance(float x, float y, float z)
		{
			overlapDistance(0, 0) = x;
			overlapDistance(1, 0) = y;
			overlapDistance(2, 0) = z;

			setOverlapDefined(x > ZERO_OVERLAP_THRESHOLD || y > ZERO_OVERLAP_THRESHOLD || z > ZERO_OVERLAP_THRESHOLD);
		}

		void getOverlapDistance(float &x, float &y, float &z)
		{
			x = overlapDistance(0, 0);
			y = overlapDistance(1, 0);
			z = overlapDistance(2, 0);
		}

		void setOverlapDefined(bool defined)
		{
			overlapDefined = defined;
		}

		bool getOverlapDefined(void)
		{
			return overlapDefined;
		}


		ChildIndex getMaxChildren(void)
		{
			return numCells[0] * numCells[1] * numCells[2];
		}

		ChildIndex getChildIndex(ChildIndex cellX, ChildIndex cellY, ChildIndex cellZ)
		{
			ChildIndex	result;

			result = cellX;										// x
			result += cellY * numCells[0];						// y*sizeX
			result += cellZ * numCells[0] * numCells[1];		// z*sizeX*sizeY

			return result;
		}

		template<class Point, class PointSet> void insert(PointSet &source)
		{

		}

		ChildIndex classify(Types<double>::Vector3 &point)
		{
			return 0;
		}

		ChildIndex classify(Types<double>::Vector3 &point, Types<float>::Extents &extents)
		{
			double			minOffset[3];
			double			cellSize[3];
			uint				cellPos[3];
															// Transform relative to centralized extents
			minOffset[0] = point(0, 0) - extents.lower(0);
			minOffset[1] = point(1, 0) - extents.lower(1);
			minOffset[2] = point(2, 0) - extents.lower(2);
															// Cull negative values
			if(minOffset[0] < 0 || minOffset[1] < 0 || minOffset[2] < 0)
				return CHILD_INDEX_NULL;

			cellSize[0] = extents.size(0) / static_cast<double>(numCells[0]);
			cellSize[1] = extents.size(1) / static_cast<double>(numCells[1]);
			cellSize[2] = extents.size(2) / static_cast<double>(numCells[2]);

			cellPos[0] = static_cast<int>(minOffset[0] / cellSize[0]);
			cellPos[1] = static_cast<int>(minOffset[1] / cellSize[1]);
			cellPos[2] = static_cast<int>(minOffset[2] / cellSize[2]);

															// If point lies outside grid (+ve), return NULL index
			if(cellPos[0] >= numCells[0] || cellPos[1] >= numCells[1] || cellPos[2] >= numCells[2])
				return CHILD_INDEX_NULL;

			return getChildIndex(cellPos[0], cellPos[1], cellPos[2]);
		}

		bool inCell(Types<double>::Vector3 &cellPos, matrix<3, 1, uint> &cell, Types<double>::Vector3 &overlapCellSpace)
		{
			Types<double>::Vector3 boxMin;
			Types<double>::Vector3 boxMax;

			boxMin(0, 0) = cell(0, 0);
			boxMin(1, 0) = cell(1, 0);
			boxMin(2, 0) = cell(2, 0);

			boxMax = boxMin;
			boxMax += Types<double>::Vector3(1, 1, 1);

			boxMin -= overlapCellSpace;
			boxMax += overlapCellSpace;

			return (cellPos >= boxMin) && (cellPos <= boxMax);
		}

		bool inGrid(matrix<3, 1, uint> &cell)
		{
			return (cell(0, 0) >= 0 && cell(1, 0) >= 0 && cell(2, 0) >= 0) &&
				   (cell(0, 0) < numCells[0] && cell(1, 0) < numCells[1] && cell(2, 0) < numCells[2]);
		}

		unsigned int classify(Types<double>::Vector3 &point, Types<float>::Extents &extents, ChildIndexSet &result)
		{
															// If cell overlap is zero, use simpler classify
			if(getOverlapDefined() == false)
			{
				ChildIndex child = classify(point, extents);

				if(child != CHILD_INDEX_NULL)
					result.push_back(child);

				return 1;
			}

			Types<double>::Vector3	minOffset;
			Types<double>::Vector3	cellSize;
			Types<double>::Vector3	cellPos;
			Types<double>::Vector3	overlapCellSpace;
			matrix<3, 1, uint>		cellPosInt;
			matrix<3, 1, uint>		cell;

															// Get cell size
			cellSize(0, 0) = extents.size(0) / static_cast<double>(numCells[0]);
			cellSize(1, 0) = extents.size(1) / static_cast<double>(numCells[1]);
			cellSize(2, 0) = extents.size(2) / static_cast<double>(numCells[2]);
															// Get overlap distance in grid space
			overlapCellSpace = overlapDistance / cellSize;

															// Transform relative to centralized extents
			minOffset(0, 0) = point(0, 0) - extents.lower(0);
			minOffset(1, 0) = point(1, 0) - extents.lower(1);
			minOffset(2, 0) = point(2, 0) - extents.lower(2);

													// Get point position in grid space
			cellPos = minOffset / cellSize;

			cellPosInt(0, 0) = static_cast<unsigned int>(cellPos(0, 0));
			cellPosInt(1, 0) = static_cast<unsigned int>(cellPos(1, 0));
			cellPosInt(2, 0) = static_cast<unsigned int>(cellPos(2, 0));

													// Test 27 local cells
			for(cell(0, 0) = cellPosInt(0, 0) - 1; cell(0, 0) <= cellPosInt(0, 0) + 1; cell(0, 0)++)
			{
				for(cell(1, 0) = cellPosInt(1, 0) - 1; cell(1, 0) <= cellPosInt(1, 0) + 1; cell(1, 0)++)
				{
					for(cell(2, 0) = cellPosInt(2, 0) - 1; cell(2, 0) <= cellPosInt(2, 0) + 1; cell(2, 0)++)
					{
						if(inGrid(cell) && inCell(cellPos, cell, overlapCellSpace))
						{
							result.push_back(getChildIndex(cell(0, 0), cell(1, 0), cell(2, 0)));
						}
					}
				}
			}

			return static_cast<uint>(result.size());
		}

		NodeType getType(void)
		{
			return NodeTypeGrid;
		}
};

// ************************************************************************************************
// Spatial Partition Node Types
// ************************************************************************************************

															// KD Nodes (Central Partition)

template<class Real, class Data> class ContainerH1KDX : public ContainerH<Real, NodeKD<Real, Data, NodeTypeKDX, AxisX> >
{
};

template<class Real, class Data> class ContainerH1KDY : public ContainerH<Real, NodeKD<Real, Data, NodeTypeKDY, AxisY> >
{
};

template<class Real, class Data> class ContainerH1KDZ : public ContainerH<Real, NodeKD<Real, Data, NodeTypeKDZ, AxisZ> >
{
};

															// KD Nodes (Positioned Partition)

template<class Real, class Data> class ContainerH1KDPosX : public ContainerH<Real, NodeKDPos<Real, Data, NodeTypeKDPosX, AxisX> >
{
};

template<class Real, class Data> class ContainerH1KDPosY : public ContainerH<Real, NodeKDPos<Real, Data, NodeTypeKDPosY, AxisY> >
{
};

template<class Real, class Data> class ContainerH1KDPosZ : public ContainerH<Real, NodeKDPos<Real, Data, NodeTypeKDPosZ, AxisZ> >
{
};

															// Quadtree (Central Partition)
template<class Real, class Data> class ContainerH1Quadtree : public ContainerH<Real, NodeQuadtree<Real, Data> >
{
};

															// Quadtree (Positioned Partition)
template<class Real, class Data> class ContainerH1QuadtreePos : public ContainerH<Real, NodeQuadtreePos<Real, Data> >
{
};

															// Octree (Central Partition)
template<class Real, class Data> class ContainerH1Octree : public ContainerH<Real, NodeOctree<Real, Data> >
{
};

															// Octree (Positioned Partition)
template<class Real, class Data> class ContainerH1OctreePos : public ContainerH<Real, NodeOctreePos<Real, Data> >
{
};

															// BSP Tree (2D planes)
template<class Real, class Data> class ContainerH1BSP2D : public ContainerH<Real, NodeBSP2D<Real, Data> >
{
};

															// BSP Tree (3D planes)
template<class Real, class Data> class ContainerH1BSP3D : public ContainerH<Real, NodeBSP3D<Real, Data> >
{
};

															// Grid (2D/3D) resizable at runtime
template<class Real, class Data> class ContainerH1Grid : public ContainerHv<Real, NodeGrid<Real, Data> >
{
public:

	ContainerH1Grid(void)
	{
		setMaxNodes(1);
	}
};
															// Generic container with arbitrary node type
template<class Real, class N> class ContainerH1Node : public ContainerH<Real, N>
{
public:

	//ContainerH1Node(void)
	//{
	//	setMaxNodes(1);
	//}
};

														// Generic container with arbitrary node type (Leaf)
template<class Real, class N> class ContainerH1NodeLeaf : public ContainerHLeaf<Real, N, 1>
{
};


} // End SpatialPartitioner namespace


#endif
