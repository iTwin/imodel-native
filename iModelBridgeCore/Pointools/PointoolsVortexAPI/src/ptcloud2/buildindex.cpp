/*----------------------------------------------------------------------*/ 
/* BuildIndex.h															*/ 
/* Point Cloud Index builder Implementation file						*/ 
/*----------------------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004											*/   
/*----------------------------------------------------------------------*/ 
/* Written by Faraz Ravi												*/ 
/*----------------------------------------------------------------------*/ 

#include "PointoolsVortexAPIInternal.h"

#include <ptcloud2/defs.h>
#include <ptcloud2/buildindex.h>

#define DEFAULT_MAX_POINTS_PER_LEAF 100000
#define DEFAULT_MIN_SUBDIVISION_DEPTH 1
#define DEFAULT_GEOM_STORE_TYPE Float32

using namespace pcloud;
using namespace pt;

namespace pcindex
{
	struct _graph_idx
	{
		ushort i;
		ushort j;
		ushort k;
		ushort w;
	};
	union _bits64
	{
		double m64;
		_graph_idx g;
	};

	class IndexNode : public Voxel
	{
	public:
		IndexNode() 
		{ 
			setCurrentLOD(LOD_MAX);
			points = 0;
			rgb = 0;
			intensity = 0;
			normals = 0;
		}

		void calcDepth()
		{
			Node* p = this;
			_depth = 0;
			while (p->parent()) { p = p->parent(); _depth++; }
		}
		void unlinkChildren()
		{
			memset(_child, 0, sizeof(void*)*8);
		}
		void calcInitialBounds()
		{
			BoundingBox bb;
            BuildIndex::calcBoundingBox(points, static_cast<int>(_pointCount), bb);
			
			/*set up partitions*/ 
			_lower[0] = bb.lx();
			_lower[1] = bb.ly();
			_lower[2] = bb.lz();

			_upper[0] = bb.ux();
			_upper[1] = bb.uy();
			_upper[2] = bb.uz();
		}
		void indentOutput()
		{
			int D = _depth;
			for (int i=0; i<D; i++) std::cout << " ";
		}
		void subdivide( int maxPoints, int minDepth )
		{
			calcDepth();
			int D = _depth;
            UNUSED_VARIABLE(D);
			
			//indentOutput();
			//std::cout << "D" << D << " P:" << _pointCount << std::endl;

			if (_depth < minDepth || _pointCount > maxPoints)
			{
				int i;
				/*partition this node in the middle*/ 
				vector3 m = mid();

				//indentOutput();
				//std::cout << "M " << m.x << ", " << m.y << ", " << m.z << std::endl;

				/*redistribute points*/ 
				int size[] = {0,0,0,0,0,0,0,0};
				ubyte* node = new ubyte[_pointCount];

				BoundingBox bb;
				for (i=0;i<_pointCount; i++)
				{
					ubyte c=0;
					if (points[i].x > m.x) c |= 1;
					if (points[i].y > m.y) c |= 2;
					if (points[i].z > m.z) c |= 4;

					bb.expand(points[i]);
					++size[c];
					node[i] = c;
				}
				if (bb.dx() < 0.000001 || bb.dy() < 0.000001 || bb.dz() < 0.000001)
				{
					return;
				}

				//indentOutput();
				//for (i=0;i<8;i++) std::cout << "cn" << i << ": " << size[i];
				//std::cout << ".";

				/*create child nodes*/ 
				IndexNode *ins[8];

				for (i=0; i<8; i++)
				{
					_child[i] = ins[i] = new IndexNode;
					ins[i]->reparent(this);
				}

				/*assign partitions*/ 
				/*lower planes*/ 
				ins[0]->_lower[0] = ins[2]->_lower[0] = ins[4]->_lower[0] = ins[6]->_lower[0] = _lower[0];
				ins[0]->_lower[1] = ins[1]->_lower[1] = ins[4]->_lower[1] = ins[5]->_lower[1] = _lower[1];
				ins[0]->_lower[2] = ins[1]->_lower[2] = ins[2]->_lower[2] = ins[3]->_lower[2] = _lower[2];
				/*upper planes*/ 
				ins[1]->_upper[0] = ins[3]->_upper[0] = ins[5]->_upper[0] = ins[7]->_upper[0] = _upper[0];
				ins[2]->_upper[1] = ins[3]->_upper[1] = ins[6]->_upper[1] = ins[7]->_upper[1] = _upper[1];
				ins[4]->_upper[2] = ins[5]->_upper[2] = ins[6]->_upper[2] = ins[7]->_upper[2] = _upper[2];
				
				/*mid x plane*/ 
				ins[0]->_upper[0] = ins[2]->_upper[0] = ins[4]->_upper[0] = ins[6]->_upper[0] = m.x;
				ins[1]->_lower[0] = ins[3]->_lower[0] = ins[5]->_lower[0] = ins[7]->_lower[0] = m.x;

				/*mid y plane*/ 
				ins[0]->_upper[1] = ins[1]->_upper[1] = ins[4]->_upper[1] = ins[5]->_upper[1] = m.y;
				ins[2]->_lower[1] = ins[3]->_lower[1] = ins[6]->_lower[1] = ins[7]->_lower[1] = m.y;

				/*mid z plane*/ 
				ins[0]->_upper[2] = ins[1]->_upper[2] = ins[2]->_upper[2] = ins[3]->_upper[2] = m.z;
				ins[4]->_lower[2] = ins[5]->_lower[2] = ins[6]->_lower[2] = ins[7]->_lower[2] = m.z;

				/*now allocate and copy points and delete unused children*/ 
				for (i=0; i<8; i++)
				{
					if (size[i])
					{
						ins[i]->points = new vector3[size[i]];
						if (rgb) ins[i]->rgb = new ubyte[size[i]*3];
						if (intensity) ins[i]->intensity = new short[size[i]];
						if (normals) ins[i]->normals = new pt::vector3s[size[i]];
					}
					else 
					{
						delete ins[i];
						ins[i] = 0;
						_child[i] = 0;
					}
				}
				
				IndexNode *indexnode=0;

				for (i=0;i<_pointCount; i++)
				{
					indexnode = ins[node[i]];
					if (rgb) memcpy(&indexnode->rgb[indexnode->_pointCount*3], &rgb[i*3], 3);
					if (intensity) memcpy(&indexnode->intensity[indexnode->_pointCount], &intensity[i], sizeof(short));
					if (normals) memcpy(&indexnode->normals[indexnode->_pointCount], &normals[i], sizeof(pt::vector3s));

					indexnode->points[indexnode->_pointCount++] = points[i];
				}
				/* make this a branch node*/ 
				delete [] node;
				if (_parent)
				{
					delete [] points;
					if (rgb) delete [] rgb;
					if (intensity) delete [] intensity;
					if (normals) delete [] normals;
				}
				points = 0;
				rgb = 0;
				intensity = 0;
				normals = 0;

				std::cout << std::endl;

				/*recurse subdivision*/ 
				for (i=0; i<8; i++)
					if (ins[i]) ins[i]->subdivide(maxPoints, minDepth);
			}
		}
		/*initialize*/ 
		void initialize(int count, vector3 *pnts, ubyte* color=0, short *inten=0, pt::vector3s *norms=0)
		{
			_pointCount = count;
			setCurrentLOD(LOD_MAX);
			_parent = 0;
			
			points = pnts;
			rgb = color;
			intensity = inten;
			normals = norms;
		}
		void makeNodeVoxel(std::vector<Voxel*> &voxels)
		{
			int i;
			for (i=0;i<8;i++)
			{
				if (_child[i])
				{
					IndexNode* inode = static_cast<IndexNode*>(_child[i]);
					Node* node=0;
					inode->makeNodeVoxel(voxels);
					
					/*is this a leaf*/ 
					if (inode->points)
					{	
						/*copy into new voxel*/ 
						Voxel*vox = new Voxel(*inode);
						vox->addChannel(PCloud_Geometry, Float32, BuildIndex::geomStoreType(), 3);
						vox->readChannel(PCloud_Geometry, static_cast<int>(inode->fullPointCount()), inode->points);

						if (inode->rgb)
						{
							vox->addChannel(PCloud_RGB, UByte8, UByte8, 3);
							vox->readChannel(PCloud_RGB, static_cast<int>(inode->fullPointCount()), inode->rgb);
						}
						if (inode->intensity)
						{
							vox->addChannel(PCloud_Intensity, Short16, Short16, 1);
							vox->readChannel(PCloud_Intensity, static_cast<int>(inode->fullPointCount()), inode->intensity);
						}
						if (inode->normals)
						{
							vox->addChannel(PCloud_Normal, Short16, Short16, 3);
							vox->readChannel(PCloud_Normal, static_cast<int>(inode->fullPointCount()), inode->normals);
						}
						delete [] inode->points;
						if (inode->rgb) delete [] inode->rgb;
						if (inode->intensity) delete [] inode->intensity;
						if (inode->normals) delete [] inode->normals;
						
						node = vox;
						voxels.push_back(vox);
					}
					else
					{
						/*copy node and continue checking children*/ 
						node = new Node;
						(*node) = (*inode);
					}
					inode->unlinkChildren();
					delete inode;

					_child[i] = node;
					_child[i]->reparent(this);
				}
			}
		}		
		vector3 *points;
		pt::vector3s *normals;
		ubyte	*rgb;
		short  *intensity;
	};
}
using namespace pcindex;
/*----------------------------------------------------------------------*/ 
/* maximum points per tree leaf - used to stop subdivision				*/ 
/*----------------------------------------------------------------------*/ 
int &BuildIndex::maxPointsPerLeaf() 
{
	static int mbpl = DEFAULT_MAX_POINTS_PER_LEAF; return mbpl; 
}
/*----------------------------------------------------------------------*/ 
/* minimum subdivision depth											*/ 
/*----------------------------------------------------------------------*/ 
int &BuildIndex::minDepth() 
{
	static int mindepth = DEFAULT_MIN_SUBDIVISION_DEPTH; return mindepth; 
}
/*----------------------------------------------------------------------*/ 
/* minimum subdivision depth											*/ 
/*----------------------------------------------------------------------*/ 
DataType &BuildIndex::geomStoreType() 
{
	static DataType gst= DEFAULT_GEOM_STORE_TYPE; return gst; 
}
/*----------------------------------------------------------------------*/ 
/* buildUniformIndex - builds a uniform octtree index					*/ 
/*----------------------------------------------------------------------*/ 
Node* BuildIndex::buildUniformPointsTree(std::vector<ChannelData*> &cd,
									 std::vector<Voxel*> &voxels)
{
	std::cout << "Building uniform tree.." << std::endl;
	std::cout << "-----------------------------" << std::endl;

	IndexNode *root = new IndexNode();

	int count = cd[0]->count;

	vector3* points = 0;
	pt::vector3s* normals = 0;
	ubyte* rgb = 0;
	short* intensity = 0;

	for (int i=0; i<cd.size(); i++)
	{
		switch (cd[i]->channel)
		{
		case pcloud::PCloud_Geometry: 
			points = reinterpret_cast<vector3*>(cd[i]->data); 
			std::cout << "\t+ Geometry channel" << std::endl;
			break;
		case pcloud::PCloud_Normal: 
			normals = reinterpret_cast<pt::vector3s*>(cd[i]->data); 
			std::cout << "\t+ Normal channel" << std::endl;
			break;
		case pcloud::PCloud_RGB: 
			rgb = reinterpret_cast<ubyte*>(cd[i]->data); 
			std::cout << "\t+ RGB channel" << std::endl;
			break;
		case pcloud::PCloud_Intensity: 
			intensity = reinterpret_cast<short*>(cd[i]->data); 
			std::cout << "\t+ Intensity channel" << std::endl;
			break;
		}
	}
	std::cout << "\t-> Initialize root" << std::endl;
	root->initialize(count, points, rgb, intensity, normals);

	std::cout << "\t-> Calculate initial bounds" << std::endl;
	root->calcInitialBounds();

	std::cout << "\t-> Subdivide" << std::endl;
	root->subdivide(maxPointsPerLeaf(), minDepth());

	/*clean up results*/ 
	/*make branches -> Nodes*/ 
	/*make leafs -> Voxels*/ 
	IndexNode *node = root;

	std::cout << "\t-> Make into Node Voxels " << std::endl;
	root->makeNodeVoxel(voxels);

	if (node->points)
	{
		node->addChannel(PCloud_Geometry, Float32, geomStoreType(), 3);
        node->readChannel(PCloud_Geometry, static_cast<int>(node->lodPointCount()), node->points);

		if (node->rgb)
		{
			node->addChannel(PCloud_RGB, UByte8, UByte8, 3);
			node->readChannel(PCloud_RGB, static_cast<int>(node->lodPointCount()), node->rgb );
		}
		if (node->intensity)
		{
			node->addChannel(PCloud_Intensity, Short16, Short16, 1);
			node->readChannel(PCloud_Intensity, static_cast<int>(node->lodPointCount()), node->intensity );
		}
		if (node->normals)
		{
			node->addChannel(PCloud_Normal, Short16, Short16, 3);
			node->readChannel(PCloud_Normal, static_cast<int>(node->lodPointCount()), node->normals );
		}
		voxels.push_back(node);
		delete [] node->points;
		delete [] node->rgb;
		delete [] node->intensity;
		delete [] node->normals;

		return node;
	}

	Node *newroot = new Node;
	*newroot = *root;
		
	root->unlinkChildren();
	delete root;
	
	newroot->calcLodPointCount();

	std::cout << "Done" << std::endl;

	return newroot;
}
/*----------------------------------------------------------------------*/ 
/* maximum points per tree leaf - used to stop subdivision				*/ 
/*----------------------------------------------------------------------*/ 
void BuildIndex::calcBoundingBox(pt::vector3 *points, int count, pt::BoundingBox &bb)
{	
	assert(points);
	bb.clear();

	if (!points) return;

	int i=0;
	while (i++ < count)
	{
		bb.expand(*points);
		++points;
	}
}

