#ifndef POINTOOLS_BINARY_OCTREE
#define POINTOOLS_BINARY_OCTREE

//Memory efficient octree generation
#include <pt/boundingbox.h>
#include <mathimf.h>
#include <assert.h>

#include <ptcloud2/node.h>
namespace pcloud
{
class BinaryOctree
{
public:
	/*----------------------------------------------*/ 
	/* Constructor / Destructor						*/ 
	/*----------------------------------------------*/ 
	BinaryOctree(const pt::BoundingBox &bounds,uint depth = 8)
	{
		_depth = depth;
		_size = computeSize(max_depth);
		_bounds = bounds;

		_data = new ubyte[_size]; 
		_pointcount = new uint[_size]; 

		assert(_data && _pointcount);

		memset(_data,0,_size);
		memset(_pointcount,0,_size*sizeof(uint));
		memset(_branchsize, 0, sizeof(uint)*16);
	}
	~BinaryOctree()
	{
		if (_data) delete [] _data;
		if (_pointcount) delete [] _pointcount;
	}
	/*----------------------------------------------*/ 
	/* Size											*/ 
	/*----------------------------------------------*/ 
	static uint computeSize(int depth)
	{
		int d =0 ;
		uint size = 0;
		do 
		{
			size += powl(8,d);	
			_branchsize[depth-d-1] = size;
		}
		while (++d < depth);
		return size;
	}
	inline int &branchSize(int depth) { return powl(8, _maxdepth-depth); }

	/*----------------------------------------------*/ 
	/* Build										*/ 
	/*----------------------------------------------*/ 
	void insert(const pt::vector3 *point)
	{
		pt::vector3d pt(point);
		pt -= ptpt::vector3(&_bounds.lower(0));

		pt::vector3 m = _bounds.center() - pt::vector3(&_bounds.lower(0));
		uint c = 0;
		uint d = 0;
		double shift = m;

		ubyte *pos = _data;
		ubyte node;
		uint pcpos = 0;

		while (d < _depth)
		{
			shift /= 2.0;

			if (pt.x > m.x) { c |= 1; m.x += shift.x; }
			else { m.x -= shift.x; }

			if (pt.y > m.y) { c |= 2; m.y += shift.y; }
			else { m.y -= shift.x; }

			if (pt.z > m.z) { c |= 4; m.y += shift.z; }			
			else { m.z -= shift.z; }
			
			node = 1 << c;
			*pos |= node;
			
			pos += _branchsize[++d] * node;
			pcpos += node * powl(8, (_depth - d));

			assert(pod < &_data[_size]);
		}	
		_pointcount[pcpos]++;
	}
	/*----------------------------------------------*/ 
	/* Tree Build via traversal						*/ 
	/*----------------------------------------------*/ 

	/*----------------------------------------------*/ 
	/* Tree Build via traversal						*/ 
	/*----------------------------------------------*/ 
	void buildTree(int max_points)
	{
		Node *root = new Node(_bounds);
		ubyte *pos = _data;
		
		Node* node = root;

		for (int i=0; i<8; i++)
		{
			if ((*pos) & 1 << i)
			{
				node->buildChild(i);
				
			}
		}
	}

private:
	pt::BoundingBox _bounds;
	uint _depth;
	ubyte *_data;
	uint *_pointcount;
	uint _size;
	uint _branchsize[16];
};
}
#endif