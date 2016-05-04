#ifndef POINTOOLS_PCLOUD2_OCTREE_COMPRESSOR
#define POINTOOLS_PCLOUD2_OCTREE_COMPRESSOR

#include <ptcloud2/pointcloud.h>

namespace pcloud
{
	class OctreeCompressor
	{
	public:
		OctreeCompressor(int maxdepth, std::vector <ubyte> &data)
			: _maxDepth(maxdepth), _byteData(data) {}

		OctreeCompressor(int maxdepth, std::vector <ubyte> &data, 
			pt::vector3s *pnts, int size)
			: _points(pnts), _numPoints(size), _maxDepth(maxdepth), _byteData(data) {}

		void compress(pt::vector3s *geom, int size, pt::vector3s p = pt::vector3s(0,0,0), int depth=0);
		
		int _maxDepth;
		std::vector <ubyte> &_byteData;
		int _numPoints;
		pt::vector3s *_points;
		int _pos;
	};
	class DeltaEncoder
	{
	public:
		DeltaEncoder(std::vector <ubyte> &data) : _byteData(data) {};
		void compress(const pt::vector3s *geom, int size);
		void compress(std::vector<int64_t> &diff);
		std::vector <ubyte> &_byteData;
	};
}
#endif