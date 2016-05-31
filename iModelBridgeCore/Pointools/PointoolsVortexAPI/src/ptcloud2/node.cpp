#include "PointoolsVortexAPIInternal.h"
#include <ptcloud2/node.h>
#include <set>
#include <assert.h>

using namespace pcloud;

Node::~Node()	
{ 
	/*delete children*/ 
	try {
	for (int i=0; i<8; i++) if (_child[i]) delete _child[i];
	}
	catch (...){}
	_parent = 0;
}
/* iteration */ 
void Node::traverseTopDown(Visitor *v) const
{
	if (v->visitNode(this))
	{
		for (int i=0;i<8;i++) if (_child[i]) _child[i]->traverseTopDown(v);
	}
	else return;
}
void Node::traverseTopDown( Visitor *v )
{
	if (v->visitNode(this))
	{
		for (int i=0;i<8;i++)
			if (_child[i]) _child[i]->traverseTopDown(v);
	}
	else return;
}
/*****************************************************************************/
/**
* @brief
* @return void
*/
/*****************************************************************************/
void Node::recursiveLayerConsolidate()
{
	if (!isLeaf())
	{	
		_layers[0] = 0;
		_layers[1] = 0;

		for (int i=0; i<8; i++)
		{
			if (_child[i])
			{
				_child[i]->recursiveLayerConsolidate();
	
				// If a child is partially in a layer, then this node is also partially in that layer
				_layers[1] |= _child[i]->layers(1);
			}
		}
		
		// Full occupancy check. For each layer, see if there is a child fully in that layer.
		// If all the children are fully that layer mark this node as fully in that layer, 
		// otherwise mark it as partially in that layer.
		for (int lyr = 0; lyr < 8; lyr++)
		{
			if (hasChildInLayer(0, (lyr+1)))
			{
				bool fullyInLayer = true;

				for (int i = 0; i < 8; i++)
				{
					if (_child[i] && !(_child[i]->layers(0) & (1<<lyr)))
					{
						// Child node is not in this layer, mark as partial occupancy only
						_layers[1] |= (1<<lyr);
						fullyInLayer = false;
					}
				}

				if (fullyInLayer)
					_layers[0] |= (1<<lyr);
			}
		}

		_layers[0] &= ~ _layers[1];	// if partial, can't be full also
	}
}

/*****************************************************************************/
/**
* @brief		Consolidate flags in Node structure beneath this.
* @detail		Node that not all flags are consolidated
*/
/*****************************************************************************/
void Node::recursiveFlagsConsolidate()
{
	// single child note sets to true
	static int partFlags []	= 
	{	
		PartClipped, PartSelected, PartHidden, Visible, Flagged, Painted, OutOfCore
	};
	// all child nodes must be true to set to true
	static int wholeFlags [] = 
	{	
		WholeClipped, WholeSelected, WholeHidden
	};

	if (!isLeaf())
	{	
		int i=0;
		int f =0;
		for (f=0;f<7;f++)	flag( partFlags[f], false );
		for (f=0;f<3;f++)	flag( wholeFlags[f], true );

		for (i=0; i<8; i++)
		{
			if (_child[i])
			{
				_child[i]->recursiveFlagsConsolidate();

				// single child note sets to true
				for (f=0; f<7; f++)
				{
					if (_child[i]->flag( partFlags[f] )) 
						flag( partFlags[f], true );
				}
				// all child nodes must be true
				for (f=0; f<3; f++)
				{
					if (!_child[i]->flag( wholeFlags[f] )) 
					{
						flag( wholeFlags[f], false );
					}
				}
			}
		}

		// some child flags but not all have whole true so this should have part true
		for (f=0; f<3; f++)
		{
			for (i=0; i<8; i++)
			{
				if (!_child[i]) continue;

				if ( !flag( wholeFlags[f]) && !flag( partFlags[f] ) )
				{	
					if ( _child[i]->flag( wholeFlags[f] ))
					{
						flag( partFlags[f], true );
						break;
					}
				}
			}
		}
	}
}
/*****************************************************************************/
/**
* @brief		Creates a new node, 
* @detail		Used at octree creation time only
*/
/*****************************************************************************/
Node * Node::newNode() const 
{ 
	return new Node; 
}
/* extents computation - can be expensive*/ 
void Node::computeExtents()
{ 
	_worldExtents.clear();

	for (int i=0;i<8;i++) 
	{
		if (_child[i])
		{
			_child[i]->computeExtents();
			_worldExtents.expandBy(_child[i]->extents());
		}
	}
}
/*****************************************************************************/
/**
* @brief			Subdivides this node ie creates another level of heirachy
* @param iterations	The number of times to subdivide
* @param quadtree	Subdivde as a quadtree, ie into 4 child nodes
* @param __deep		internal use for recursion
*/
/*****************************************************************************/
void Node::subdivide( int iterations, int quadtree, int __deep )
{
	if (__deep++ <= iterations)
	{
		uint i;
		for (i=0; i<8; i++)
			if (!_child[i])
			{
				_child[i] = newNode();
				_child[i]->reparent( this );
			}

		if (parent()) _depth = (ubyte) parent()->depth() + 1;
		
		pt::vector3 m = mid();
		
		// child index is a bitmask of dimensions in which the node is in the upper half
		// so index = (upper x) 1 | (upper y) 2 | (upper z) 4

		/*assign partitions*/ 
		/*lower planes*/ 
		_child[0]->_lower[0] = _child[2]->_lower[0] = _child[4]->_lower[0] = _child[6]->_lower[0] = _lower[0];
		_child[0]->_lower[1] = _child[1]->_lower[1] = _child[4]->_lower[1] = _child[5]->_lower[1] = _lower[1];
		_child[0]->_lower[2] = _child[1]->_lower[2] = _child[2]->_lower[2] = _child[3]->_lower[2] = _lower[2];
		/*upper planes*/ 
		_child[1]->_upper[0] = _child[3]->_upper[0] = _child[5]->_upper[0] = _child[7]->_upper[0] = _upper[0];
		_child[2]->_upper[1] = _child[3]->_upper[1] = _child[6]->_upper[1] = _child[7]->_upper[1] = _upper[1];
		_child[4]->_upper[2] = _child[5]->_upper[2] = _child[6]->_upper[2] = _child[7]->_upper[2] = _upper[2];
		
		/*mid x plane*/ 
		_child[0]->_upper[0] = _child[2]->_upper[0] = _child[4]->_upper[0] = _child[6]->_upper[0] = m.x;
		_child[1]->_lower[0] = _child[3]->_lower[0] = _child[5]->_lower[0] = _child[7]->_lower[0] = m.x;

		/*mid y plane*/ 
		_child[0]->_upper[1] = _child[1]->_upper[1] = _child[4]->_upper[1] = _child[5]->_upper[1] = m.y;
		_child[2]->_lower[1] = _child[3]->_lower[1] = _child[6]->_lower[1] = _child[7]->_lower[1] = m.y;

		/*mid z plane*/ 
		_child[0]->_upper[2] = _child[1]->_upper[2] = _child[2]->_upper[2] = _child[3]->_upper[2] = m.z;
		_child[4]->_lower[2] = _child[5]->_lower[2] = _child[6]->_lower[2] = _child[7]->_lower[2] = m.z;


		if (quadtree > -1)		// want to make this a quad tree
		{
			uint qt_dim = 1 << quadtree;
			
			// store which dimension the quadtree is in
			_quadtree |= qt_dim;

			// cull unneeded child nodes
			for (i=0; i<8; i++)
			{
				if (!(qt_dim & (uint)i)) 
				{
					delete _child[i];
					_child[i] = 0;
				}
				else _child[i]->_quadtree = _quadtree;
			}

			// Extend the remaining children to fill the space left by deleting the unneeded child nodes
			switch(quadtree)
			{
			case 0:	// Quadtree in X
				_child[1]->_lower[0] = _child[3]->_lower[0] = _child[5]->_lower[0] = _child[7]->_lower[0] = _lower[0];
				_child[5]->_lower[1] = _child[1]->_lower[1] = _lower[1]; 
				_child[3]->_lower[2] = _child[1]->_lower[2] = _lower[2];
				break;

			case 1: // Quadtree in Y
				_child[6]->_lower[0] = _child[2]->_lower[0] = _lower[0];
				_child[2]->_lower[1] = _child[3]->_lower[1] = _child[6]->_lower[1] = _child[7]->_lower[1] = _lower[1];
				_child[3]->_lower[2] = _child[2]->_lower[2] = _lower[2];				
				break;

			case 2:	// Quadtree in Z
				_child[6]->_lower[0] = _child[4]->_lower[0] = _lower[0];
				_child[5]->_lower[1] = _child[4]->_lower[1] = _lower[1];
				_child[4]->_lower[2] = _child[5]->_lower[2] = _child[6]->_lower[2] = _child[7]->_lower[2] = _lower[2];
				break;
			}	
		}
		
		/*subdivide children*/ 
		for (i=0; i<8; i++)
		{
			if (_child[i]) 
				_child[i]->subdivide( iterations, quadtree, __deep );	
		}
	}
}
/*****************************************************************************/
/**
* @brief		creates a child node and setups it up
* @detail		Used during octree creation only
* @param c		The child node index 0-7
* @return void
*/
/*****************************************************************************/
void Node::buildChild( int c )
{
	// quadtree in x,y or z - check it is not trying to create a node where
	// it should not be for a quad tree
	assert( !_quadtree || ((uint)c & _quadtree));
	if (_quadtree && !((uint)c & _quadtree)) return;

	pt::vector3 m = mid();

	if (hasChildren())
	{
		for (int i=0; i<8; i++)
		{
			if (_child[i])
			{
				switch(i)
				{
				case 4:	case 6:	case 2:	case 0: m.x = _child[i]->_upper[0]; break;
				case 1:	case 3:	case 5:	case 7: m.x = _child[i]->_lower[0]; break;
				}
				switch(i)
				{
				case 0:	case 1:	case 4:	case 5: m.y = _child[i]->_upper[1]; break;
				case 2:	case 3:	case 6:	case 7: m.y = _child[i]->_lower[1]; break;
				}
				switch(i)
				{
				case 0:	case 1:	case 2:	case 3: m.z = _child[i]->_upper[2]; break;
				case 4:	case 5:	case 6:	case 7: m.z = _child[i]->_lower[2]; break;
				}
				break;
			}
		}
	}
	_child[c] = newNode();
	_child[c]->_quadtree = _quadtree;	//quadtree info
	_child[c]->reparent(this);

	/*lower[0]*/ 
	switch(c)
	{
	case 4:	case 6:	case 2:	case 0: 
		_child[c]->_lower[0] = _lower[0]; 
		_child[c]->_upper[0] = m.x; 
		break;

	case 1:	case 3:	case 5:	case 7: 
		_child[c]->_lower[0] = m.x; 
		_child[c]->_upper[0] = _upper[0]; 
		break;
	}
	/*lower[1]*/ 
	switch(c)
	{
	case 0:	case 1:	case 4:	case 5: 
		_child[c]->_lower[1] = _lower[1]; 
		_child[c]->_upper[1] = m.y; 
		break;

	case 2:	case 3:	case 6:	case 7: 
		_child[c]->_lower[1] = m.y; 
		_child[c]->_upper[1] = _upper[1]; 
		break;
	}
	/*lower[2]*/ 
	switch(c)
	{
	case 0:	case 1:	case 2:	case 3: 
		_child[c]->_lower[2] = _lower[2]; 
		_child[c]->_upper[2] = m.z; 
		break;

	case 4:	case 5:	case 6:	case 7: 
		_child[c]->_lower[2] = m.z; 
		_child[c]->_upper[2] = _upper[2]; 
		break;
	}

	// quadtree in x,y or z
	if (_quadtree & 1)			_child[c]->_lower[0] = _lower[0];
	else if (_quadtree & 2)		_child[c]->_lower[1] = _lower[1];
	else if (_quadtree & 4)		_child[c]->_lower[2] = _lower[2];

}
/*****************************************************************************/
/**
* @brief			swap a node for another
* @param oldnode	Old node to swap, must be child of this node
* @param newnode	New node to replace old node
* @return bool		False if old node cannot be found, otherwise true
*/
/*****************************************************************************/
bool Node::replace(Node *oldnode, Node* newnode)
{
	for (int i=0; i<8; i++)
	{
		if (_child[i])
		{
			if (_child[i] == oldnode) 
			{
				delete _child[i];
				_child[i] = newnode;
				return true;
			}
			else if (_child[i]->replace(oldnode, newnode))
				return true;
		}
	}
	return false;
}
/*****************************************************************************/
/**
* @brief		recursively calc currently held point count beneath this node
* @return uint64
*/
/*****************************************************************************/
uint64 Node::calcLodPointCount()
{
	if (!isLeaf())
	{
		_pointCount = 0;
		
		for (int i=0; i<8; i++)
			if (_child[i]) _pointCount += _child[i]->calcLodPointCount();

		return _pointCount;
	}
	return lodPointCount();
}
/*****************************************************************************/
/**
* @brief		recursively calc full point count including OOC
* @return uint64
*/
/*****************************************************************************/
uint64 Node::fullPointCount() const
{
	uint64 point_count = 0;

	if (!isLeaf())
	{
		
		for (int i=0; i<8; i++)
			if (_child[i]) 
				point_count += _child[i]->fullPointCount();
	}
	return point_count;
}
/*****************************************************************************/
/**
* @brief			trim octree up to depth level
* @param minDepth	The minimum depth to trim to, everything deeper is removed
* @param pbl		point limit (check meaning??)
*/
/*****************************************************************************/
void Node::trim(uint minDepth, uint pbl)
{
	if (_pointCount > pbl && _depth > minDepth)
	{
		for (int i=0; i<8; i++)
			if (_child[i]) _child[i]->trim(minDepth, pbl);	
	}
	else if (_depth >= minDepth)
	{
		/*trim branches*/ 
		for (int i=0; i<8; i++)
			if (_child[i]) delete _child[i];

		memset(_child, 0, sizeof(void*)*8);
	}
}

/*****************************************************************************/
/**
* @brief
* @param ray
* @return Node *
*/
/*****************************************************************************/
Node *Node::findIntersectingLeaf(const pt::Ray<float> &ray) const
{
	// convert to float
	pt::Rayd rayd( pt::vector3d(ray.origin), pt::vector3d(ray.direction) );

	if (_worldExtents.intersectsRay(rayd)) 
	{
		if (isLeaf()) return const_cast<Node*>(this); //messy, fix it!
		else
		{
			for (int i=0; i<8; i++)
			{
				if (_child[i])
				{
					const Node* leaf = _child[i]->findIntersectingLeaf(ray);
					if (leaf) return const_cast<Node*>(leaf);
				}
			}
		}
	}
	return 0;
}
/*****************************************************************************/
/**
* @brief
* @param segment
* @return Node *
*/
/*****************************************************************************/
Node *Node::findIntersectingLeaf(const pt::Segment<float> &segment) const
{
	pt::Segment<double> segd( pt::vector3d(segment.a), pt::vector3d(segment.b) );

	if (_worldExtents.intersectsSegment(segd))
	{
		if (isLeaf()) return const_cast<Node*>(this);
		else
		{
			for (int i=0; i<8; i++)
			{
				if (_child[i])
				{
					const Node* leaf = _child[i]->findIntersectingLeaf(segment);
					if (leaf) return const_cast<Node*>(leaf);;
				}
			}
		}
	}
	return 0;
}
          
/** Helper function for checking if any of the children of a node are in
 a particular layer.
 @param layerType	0 for checking the full layer, 1 for checking the partial layer
 @param layerID		The number of the layer to check (1 - 8)
 */
bool Node::hasChildInLayer(int layerType, int layerID) const
{
#ifdef _DEBUG
	assert((layerType == 0) || (layerType == 1));
	assert((layerID > 0) && (layerID < 9));
#endif // _DEBUG

	int numInLayer = 0;

	for (int i = 0; i < 8; i++)
	{
		if (_child[i] && (_child[i]->layers(layerType) & (1<<(layerID-1))))
			numInLayer++;
	}

	return (numInLayer > 0);
}
          
void Node::traverseRayIntersecting(Visitor *v, const pt::Rayf &ray, bool leavesOnly) const
{
	pt::Rayd rayd( pt::vector3d(ray.origin), pt::vector3d(ray.direction) );

	if (_worldExtents.intersectsRay(rayd))
	{
		if (isLeaf()) v->visitNode(this);
		else
		{
			if (!leavesOnly) v->visitNode(this);

			for (int i=0; i<8; i++)
			{
				if (_child[i])
				{
					_child[i]->traverseRayIntersecting(v, ray, leavesOnly);
				}
			}
		}
	}
}
/*find point - must check that the point is in this node first*/ 
Node* Node::findContainingLeaf(const pt::vector3d &point) const
{
	if (hasChildren()) 
	{
		int c = inOctant(point);
		if (_child[c]) return _child[c]->findContainingLeaf(point);
		else return 0;
	}
	return const_cast<Node*>(this);
}