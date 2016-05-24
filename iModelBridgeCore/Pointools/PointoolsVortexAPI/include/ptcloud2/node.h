/*----------------------------------------------------------*/ 
/* Node.h													*/ 
/* Node Interface file										*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2009								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 

#ifndef POINTCLOUD_NODE
#define POINTCLOUD_NODE	1

#include <pt/boundingbox.h>

#include <ptcloud2/defs.h>
#include <ptcloud2/pcloud.h>

#include <vector>
#include <set>

namespace pcloud
{

class PointCloud;

enum Flag
{
	Visible			= 1,	// byte 1	// voxel is within the viewing fustrum and should always be drawn
	WholeClipped	= 2,				// the whole voxel has been clipped by the clip box
	PartClipped		= 3,				// part of the voxel has been clipped by the clip box
	Rendered		= 4,				// the voxel was drawn in the previous rendering pass so does not need to be redrawn
	Flagged			= 5,				// temporarily used by various algorithms e.g. Edit refresh
	WholeSelected	= 8,	// byte 2	// the whole voxel is selected
	PartSelected	= 9,			    // part of the voxel is selected
	WholeHidden		= 10,				// the voxel is not visible given the current layer settings
	PartHidden		= 11,				// the voxel is partially visible given the current layer settings
	OutOfCore		= 12,				// voxel is out of core i.e. it will be loaded, rendered then dumped out of memory again
	Painted			= 13,				// the rgb colours in the voxel have been edited
	ExtentsDirty	= 14,				// extents of the voxel have changed
	RenderDismiss	= 16,	// byte 3	// the voxel should not be rendered
	DebugShowRed	= 20,
	DebugShowBlue	= 21,
	DebugShowGreen	= 22,
	DebugShowPurple	= 23	
};

class PCLOUD_API Node
{
public:
	Node() 
	{ 
		memset(_child, 0, sizeof(void*)*8); 
		memset(_lower, 0, sizeof(void*)*3); 
		memset(_upper, 0, sizeof(void*)*3); 
		_pointCount = 0;
		_parent = 0; 
		_depth = 0;
		_flags[0] = 0;
		_flags[1] = 0;
		_flags[2] = 0;
		_layers[0] = 1;
		_layers[1] = 0;
		_quadtree = 0;
	}
	/*Root constructor*/ 
	Node(const pt::BoundingBoxD &exts)
	{
		_lower[0] = static_cast<float>(exts.lx());
		_lower[1] = static_cast<float>(exts.ly());
		_lower[2] = static_cast<float>(exts.lz());
		_upper[0] = static_cast<float>(exts.ux());
		_upper[1] = static_cast<float>(exts.uy());
		_upper[2] = static_cast<float>(exts.uz());

		memset(_child, 0, sizeof(void*)*8); 

		_depth = 0;
		_pointCount = 0;
		_parent = 0; 
		_depth = 0;
		_flags[0] = 0;
		_flags[1] = 0;
		_flags[2] = 0;
		_layers[0] = 1;
		_layers[1] = 0;
		_quadtree = 0;
	}
	Node(float *lower, float *upper, int deep, const pt::BoundingBoxD &exts, ubyte quadtree=0)
	{
		memset(_child, 0, sizeof(void*)*8); 
		memcpy(_lower, lower, sizeof(float)*3); 
		memcpy(_upper, upper, sizeof(float)*3); 
		_parent = 0; 
		_pointCount = 0;
		_depth = (ubyte) deep;
		_worldExtents = exts;
		_flags[0] = 0;
		_flags[1] = 0;
		_flags[2] = 0;
		_layers[0] = 1;
		_layers[1] = 0;
		_quadtree = quadtree;
	}
	virtual ~Node();

	Node &operator = (const Node& b)
	{
		memcpy(_lower, b._lower, sizeof(void*)*3);
		memcpy(_upper, b._upper, sizeof(void*)*3);
		memcpy(_child, b._child, sizeof(void*)*8);
		_parent = b._parent;
		_depth = b._depth;
		_pointCount = b._pointCount;
		_layers[0] = b._layers[0];
		_layers[1] = b._layers[1];
		_quadtree = b._quadtree;

		return *this;
	};

	virtual bool isLeaf() const { return false; }

	const float &lx() const { return _lower[0]; }
	const float &ly() const { return _lower[1]; }
	const float &lz() const { return _lower[2]; }
	const float &ux() const { return _upper[0]; }
	const float &uy() const { return _upper[1]; }
	const float &uz() const { return _upper[2]; }

	inline const float &partition(int i) const 
	{ 
		return i < 3 ? _lower[i] : _upper[i-3]; 
	} 

	inline pt::vector3 mid() const
	{ 
		return pt::vector3(
			_lower[0] + ((_upper[0] - _lower[0]) / 2.0f),
			_lower[1] + ((_upper[1] - _lower[1]) / 2.0f),
			_lower[2] + ((_upper[2] - _lower[2]) / 2.0f));
	}

	/*bounds as bounding box*/ 
	inline void getBounds(pt::BoundingBox &bb) const
	{
		bb.setBox(_upper[0], _lower[0], _upper[1], _lower[1], _upper[2], _lower[2]);
	}
	inline void getBoundsD(pt::BoundingBoxD &bb) const
	{
		bb.setBox(_upper[0], _lower[0], _upper[1], _lower[1], _upper[2], _lower[2]);
	}
	template<class T>
	inline bool inBounds(const pt::vec3<T> &pnt) const
	{
		return (pnt.x >= _lower[0] && pnt.x <= _upper[0] && 
				pnt.y >= _lower[1] && pnt.y <= _upper[1] && 
				pnt.z >= _lower[2] && pnt.z <= _upper[2]) ? true : false;
	}
	/*extents as bounding box*/ 
	inline const pt::BoundingBoxD &extents() const		{ return _worldExtents; }	
	void setExtents(pt::BoundingBoxD& ext) { _worldExtents = ext; }
	virtual void computeExtents();
	
	/*children*/ 
	void setChildren(Node** c) { memcpy(_child, c, sizeof(void*)*8); }
	inline Node*child(int i) { return _child[i]; }
	inline const Node* child(int i) const { return _child[i]; }
	bool hasChildren() const { return (_child[0] || _child[1] || _child[2] || _child[3] ||
										_child[4] || _child[5] || _child[6] || _child[7]); }
	int  numChildren() const { int c=0; for (int i=0; i<8; i++) if (_child[i]) ++c; return c; }
	void removeChildren();

	/*parent*/ 
	void reparent(Node*n) { _parent = n; _depth = (ubyte) _parent->depth()+1; }
	inline Node* parent() { return _parent; }
	inline const Node* parent() const { return _parent; }

	bool replace(Node *oldnode, Node* newnode);

	struct Visitor	
	{	
		virtual bool visitNode(const Node *n) = 0;	
		virtual bool cloud(pcloud::PointCloud *cloud) { return true; }
	};
	/*traversal*/ 
	void traverseTopDown(Visitor *i);
	void traverseTopDown(Visitor *i) const;
	void traverseAlongRay(Visitor *i, const pt::vector3 &origin, const pt::vector3 &dir);

// 	void traverseRayIntersecting(Visitor *i, const pt::Rayf &ray, bool leavesOnly=true) const;

	template<class Real> void traverseRayIntersecting(Visitor *v, const pt::Ray<Real> &ray, bool leavesOnly = true) const
	{
		if (_worldExtents.intersectsRay(ray))
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
	Node* findContainingLeaf(const pt::vector3d &point) const;
	Node *findIntersectingLeaf(const pt::Ray<float> &ray) const;
	Node *findIntersectingLeaf(const pt::Segment<float> &ray) const;
	void traverseRayIntersecting(Visitor *v, const pt::Rayf &ray, bool leavesOnly) const;

	int depth() const { return (int)_depth; }
	int countNodes( bool leaves_only=false ) const { int i=0; _countNodes(i, leaves_only); return i; }
	void subdivide( int iterations=1, int quadTreeDimension=-1, int __LEAVE_UNDEFINED=1 );

	/*point count*/ 
	uint64			calcLodPointCount();

	virtual uint64	fullPointCount() const;
	virtual uint64	lodPointCount() const	{ return _pointCount; }

	/*trim tree to max points per leaf    */ 
	/*will create Leaf objects as leaves, */ 
	/*not suitable for points transfer	  */ 
	void trim(uint minDepth, uint pointsPerLeaf);
	
	/*check containment in Octant - does not check bounds, just half spaces */ 
	template<class T> uint inOctant(const T &pt) const
	{
        pt::vector3 m = mid();
		uint c = 0;
		if (pt.x > m.x || _quadtree & 1) c |= 1;
		if (pt.y > m.y || _quadtree & 2) c |= 2;
		if (pt.z > m.z || _quadtree & 4) c |= 4;
		
		return c;
	}
	void buildChild(int child_idx);

	inline bool flag(int f) const { return (_flags[f/8] & 1 << (f % 8)) ? true : false; }

	inline void flag(int f, bool v, bool propagate = false) 
	{ 
		if (v)
			_flags[f/8] |= 1 << (f % 8);
		else
			_flags[f/8] &= ~(1 << (f % 8)); 

		if (propagate)
		{
			for (int i=0; i<8; i++)
			{
				if (_child[i]) _child[i]->flag(f,v, true);
			}
		}
	}

	inline ubyte &layers(int i=0)	{ return _layers[i]; }
	inline const ubyte &layers(int i=0)	const { return _layers[i]; }

	/* top down traversal updating tree with correct layer and flag status */ 
	void recursiveLayerConsolidate();

	/* bottom up updating tree with correct flags status */ 
	void recursiveFlagsConsolidate();
	
	/* if subdivided as quadtree, returns the dimension in which the tree is based
	ie the dimension which is not subdivided. returns -1 for octree */ 
	int quadTreeAxis() const 
	{ 
		int axis = _quadtree;
		if (axis == 4) axis = 3;
		return axis - 1;
	}

protected:
	friend class PointCloud;
	virtual Node* newNode() const; 

	/* count the nodes under and inc this one */ 
	void _countNodes(int &i, bool leaves_only) const 
	{ 
		if (!leaves_only || isLeaf()) i++; 
		for (int a=0; a<8; a++) if (_child[a]) _child[a]->_countNodes(i, leaves_only); 
	}

	bool hasChildInLayer(int layerType, int layerID) const;

	/* point count of everything below this node */ 
	uint64	_pointCount;

	/* child nodes */ 
	Node*	_child[8];

	/* parent node */ 
	Node*	_parent;

	/* extents of data contained in world coords, returned when using extents() */ 
	pt::BoundingBoxD _worldExtents;

	/* extents of octree structure in local cloud coords, returned when using getBounds() and getBoundsD() */ 
	float	_lower[3];
	float	_upper[3];
	
	/* depth of node in graph */ 
	ubyte	_depth;

	/* extra flags aligning to 4 byte boundary */ 
	ubyte	_flags[3];

	/* layers - used by pt edit. Used to store the occupancy state of each layer.
	Use bitwise operations to compare the occupancy of a layer against one of the
	current layer states e.g. (layers(0) & g_currentLayer).
	Index 0: Layers that have full occupancy
	Index 1: Layers that have partial occupancy
	Index 2: Unused (padding for retaining 4 byte alignment of Node objects)
	*/
	ubyte	_layers[3];

	/* QuadTree */ 
	ubyte	_quadtree;

};
}
#endif
