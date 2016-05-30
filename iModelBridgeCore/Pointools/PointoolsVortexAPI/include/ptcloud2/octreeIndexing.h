#pragma once

#include <pt/geomtypes.h>
#include <pt/boundingbox.h>
#include <pt/bounds.h>


#define MAX_BASE_DEPTH		4
#define MAX_OCTREE_DEPTH	5	// should not be deeper for quantisation alone, ok for volume

#define NODE_LARGE_SUBD_FLAG	4

namespace pcloud
{
	struct OctreeIndexing
	{
		/* compute the minimum depth for 16bit Quantisation given the data bounds */ 
		static int minDepthFor16bitQuant(const pt::BoundingBoxD &bb, float accuracy)
		{
			accuracy *= 65335;
			if (accuracy <= 0) return -1;

			pt::vector3d size(bb.dx(), bb.dy(), bb.dz());
			int a = size.major_axis();
			float val = static_cast<float>(bb.upper(a) - bb.lower(a));
			int subd = static_cast<int>(val / accuracy);

			/* find lowest power of 2 that is above subd*/ 
			int depth = 1;
			while ((1 << depth++) < subd && depth < 16);

			if (depth > 11) return -1;

			return depth;
		}

		static bool reqDepthFor16bitCompress(const pt::BoundingBoxD &bounds, float tol, int &base_depth, int &req_depth)
		{
			/* octree depth to maintain accuracy computation										*/ 
			req_depth = tol ? minDepthFor16bitQuant(bounds, tol) : -1;
			if (req_depth != -1 && req_depth < 2) req_depth = 2;

			/* base depth is the minimum depth of subdivision*/ 
			/* cap this otherwise subdivision takes a long time + memory*/ 
			base_depth = req_depth;
			if (base_depth > MAX_BASE_DEPTH) base_depth = MAX_BASE_DEPTH;

			/* dont compress if this would require excessive subdivision */ 
			if (req_depth > MAX_OCTREE_DEPTH) { req_depth = -1; base_depth = -1; }
			
			/* if mindepth <-1 don't vectorize */ 
			bool compress = base_depth >= 0 ? true : false;
			return compress;
		}
	};
	struct UniformFilter
	{
		UniformFilter()
		{
			_phm = 0;
			_phs = 0;
		}
		~UniformFilter()
		{
			clearData();
		}
		struct AvData 
		{ 
			AvData(const AvData &a) { *this = a; };
			AvData(const pt::vector3d &p) : num(0), pnt(p) {};

			pt::vector3d pnt; 
			int num; 
			void add(const pt::vector3d &p) { 
				pnt *= num;
				pnt += p;
				pnt /= ++num;
			}
			void operator = (const AvData &a) { memcpy(this, &a, sizeof(a)); }
		};

		static bool hashPoint(const pt::vector3d &vec, float multiple, uint64_t &hash)
		{
			pt::vector3i quantize_multiples(static_cast<int>(vec.x / multiple), static_cast<int>(vec.y / multiple), static_cast<int>(vec.z / multiple));

			/* trucate anything too big */ 
			pt::vector3i large_component(quantize_multiples);
			large_component /= 1000000;
			large_component *= 1000000;
			quantize_multiples -= large_component;

			/* get rid of any negatives */ 
			quantize_multiples.x += 1000010;
			quantize_multiples.y += 1000010;
			quantize_multiples.z += 1000010;

			if (quantize_multiples.x < 0 ||
				quantize_multiples.y < 0 ||
				quantize_multiples.z < 0) return false;

			hash = (((unsigned int)quantize_multiples.x) & 0x1fffff);
			hash |= (((unsigned int)quantize_multiples.y) << 21) & 0x3FFFFE00000;
			hash |= (((uint64_t)quantize_multiples.z) << 42) & 0x7FFFFC0000000000;
			return true;
		}

		void setVoxel(const Voxel *vox)
		{
			/* get map for this voxel */ 
			VoxelPHS::iterator it = _voxelPHS.find(vox);

			if (it == _voxelPHS.end())
			{
				_phs = new PointsHashSet;
				_voxelPHS.insert(VoxelPHS::value_type(vox, _phs));
			}
			else _phs = it->second;
		}
		
		bool filter(const pt::vector3d &pnt,  double multiple)
		{	
			uint64_t hash;
			if (!hashPoint(pnt, static_cast<float>(multiple), hash)) return false;
			PointsHashSet::iterator it = _phs->find(hash);
			
			if (it != _phs->end()) return false;
			
			else _phs->insert(hash);
			return true;

		}
		
		void clearData()
		{
			VoxelPHS::iterator it = _voxelPHS.begin();
			while (it != _voxelPHS.end())
			{
				delete it->second;
				++it;
			}
			_voxelPHS.clear();
		}

		typedef std::set<uint64_t> PointsHashSet;
		typedef std::map<uint64_t, AvData > PointsHashMap;
		typedef std::map<const Voxel *, PointsHashMap*> VoxelPHM;
		typedef std::map<const Voxel *, PointsHashSet*> VoxelPHS;

		VoxelPHM _voxelPHM;
		VoxelPHS _voxelPHS;

		PointsHashMap *_phm;
		PointsHashSet *_phs;
	};
	/*--------------------------*/ 
	/* Incremental Node			*/ 
	/*--------------------------*/ 
	class IncNode : public Node
	{
	public:
		IncNode(const pt::BoundingBoxD &bb) : Node(bb) 
		{ 
			_flags[0] = 0;
		}
		IncNode(){};
		Node* newNode() const { return new IncNode; }
		unsigned int merge(int maxPointCount, int minDepth, unsigned int mergedCount = 0)
		{
			if (!hasChildren())
			{
#ifdef PTTRACEOUT
				if ( _pointCount > maxPointCount )
				{
					PTTRACEOUT << "WARNING: Large node, unable to subdivide " << _pointCount << " points";
				}
#endif
				return mergedCount;
			}

			/*first kill off empties*/ 
			int i;
			pt::BoundingBoxD bb;

			for (i=0; i<8; i++)
			{
				if(_child[i])
				{
					if (_child[i]->lodPointCount() == 0 && _child[i]->flag(NODE_LARGE_SUBD_FLAG) == false)
					{
						delete _child[i];
						_child[i] =0;
						mergedCount++;
					}
				}
			}
			/*now merge (ie remove) children*/ 
			if (_parent && _pointCount < maxPointCount && depth() >= minDepth && flag(NODE_LARGE_SUBD_FLAG) == false)
			{
				for (i=0; i<8; i++)
				{
					if (_child[i])
					{
						delete _child[i];
						_child[i] =0;
						mergedCount++;
					}

				}
			}
			else
			{
				for (i=0; i<8; i++)
				{
					if (_child[i])
					{
						mergedCount = incChild(i)->merge(maxPointCount, minDepth, mergedCount);
					}
				}
			}

			return mergedCount;
		}

		void collectLarge(std::vector<Node*> &nodes, int max_size)
		{
			if (!hasChildren())
			{
				if (_pointCount > max_size)
				{
					nodes.push_back(this);
				}
			}
			else for (int i=0; i<8; i++)
					if (_child[i]) incChild(i)->collectLarge(nodes, max_size);
		}


		int subdivideLarge(int max_size, int subd)
		{
			int res = 0;

			if (!hasChildren())
			{
				if(_pointCount > max_size && _localextents.maxDimensionSize() > 0.000001)
				{
					subdivide( subd );
#ifdef PTTRACEOUT
					PTTRACEOUT << "Subdivided large node : " << _pointCount << "pts";
#endif
					flag(NODE_LARGE_SUBD_FLAG, true, true);
					res += 1;
					_pointCount = 0;
				}
				else
				{
															// Clear NODE_LARGE_SUBD_FLAG flag for valid, non subdividing leaf node
					flag(NODE_LARGE_SUBD_FLAG, false, false);
				}
			}
			else 
			{
															// Clear NODE_LARGE_SUBD_FLAG flag for all internal nodes
				flag(NODE_LARGE_SUBD_FLAG, false, false);

				for (int i=0; i<8; i++)
				{
					if (_child[i])
					{
						res += incChild(i)->subdivideLarge( max_size, subd );
					}
				}
			}

			return res;
		}


		void countPoint(const pt::vector3d &pt, const pt::vector3d &xpt, int max_depth, bool flagged_only=false)
		{
			if (flagged_only == false || flagged_only && flag(NODE_LARGE_SUBD_FLAG))
			{
				_pointCount++;

				_worldExtents.expand(xpt);
				pt::vector3 p(pt);
				_localextents.expand(p);
			}

			int c = inOctant(pt);

			if(flagged_only == false && _child[c] == NULL && depth() < max_depth)
			{
				buildChild(c);
			}

			if(_child[c])
			{
				incChild(c)->countPoint(pt, xpt, max_depth, flagged_only);
			}
		}

		void resetPointCount() { _pointCount = 0; }
		void output()
		{
#ifdef _IOSTREAM_
			for (int s=0;s<depth();s++) std::cout << " ";
			if (!hasChildren())
			{
				std::cout << "V:" << _pointCount << std::endl;
			}	
			else
			{
				std::cout << "N:" << _pointCount << std::endl;
				for (int i=0; i<8; i++)	if (_child[i]) incChild(i)->output();
			}
#endif
		}
		inline IncNode *incChild(int i) { 	return static_cast<IncNode*>(_child[i]); }
		
#define INCNODE(a) static_cast<IncNode*>(a)

		void makeLeavesVoxels(std::vector<Voxel*> &voxels, bool rgb, bool intensity, bool normals, bool classification, bool gridded, DataType geomtype, float tolerance)
		{
			for (int i=0; i<8; i++)
			{
				if (_child[i])
				{
					if (!_child[i]->hasChildren())
					{
						Voxel *vox = new Voxel(_lower, _upper, depth()+1, _child[i]->extents(), static_cast<uint>(_child[i]->lodPointCount()));
						voxels.push_back(vox);

						vox->addChannel(PCloud_Geometry, Float32, geomtype, 3, 0, 0, 0);
						if (geomtype == Short16)
						{
							pt::BoundingBox b(INCNODE(_child[i])->_localextents);
						
							pt::vector3d sc(b.dx()/65336.0f, b.dy()/65336.0f, b.dz()/65336.0f);
							pt::vector3d of(b.mid(0), b.mid(1), b.mid(2));

							float t = tolerance * 0.1f;
							if (sc.x< t) sc.x = t;
							if (sc.y< t) sc.y = t;
							if (sc.z< t) sc.z = t;
						
							memcpy(const_cast<double*>(vox->channel(PCloud_Geometry)->scaler()), &sc, sizeof(pt::vector3d));
							memcpy(const_cast<double*>(vox->channel(PCloud_Geometry)->offset()), &of, sizeof(pt::vector3d));							
						}
						else
						{
							pt::BoundingBox b(INCNODE(_child[i])->_localextents);
							
							pt::vector3d sc(1,1,1);
							for (int d=0;d<3;d++)
							{
								if (b.size(d) < 1e2) sc[d] = 1.0; 
								else if (b.size(d) < 1e3) sc[d] = 10; 
								else if (b.size(d) < 1e4) sc[d] = 100; 
								else if (b.size(d) < 1e6) sc[d] = 1000;
								else sc[d] = 1e4;
							}		
							pt::vector3d of(b.lower(0), b.lower(1), b.lower(2));

							memcpy(const_cast<double*>(vox->channel(PCloud_Geometry)->offset()), &of, sizeof(pt::vector3d));																					
							memcpy(const_cast<double*>(vox->channel(PCloud_Geometry)->scaler()), &sc, sizeof(pt::vector3d));
						}
						if (rgb) vox->addChannel(PCloud_RGB, UByte8, UByte8, 3, 0, 0, 0);
						if (intensity) vox->addChannel(PCloud_Intensity, Short16, Short16, 1, 0, 0, 0);
						if (normals) vox->addChannel(PCloud_Normal, Short16, Short16, 3, 0, 0, 0);
						if (gridded) vox->addChannel(PCloud_Grid, ULong32, ULong32, 1, 0, 0, 0);
						if (classification) vox->addChannel(PCloud_Classification, UByte8, UByte8, 1, 0, 0, 0);

						delete _child[i];
						_child[i] = vox;
						vox->reparent(this);
					} 
					else INCNODE(_child[i])->makeLeavesVoxels(voxels, rgb, intensity, normals, classification, gridded, geomtype, tolerance);
				}
			}
		}
		pt::BoundingBox _localextents;
	};

};