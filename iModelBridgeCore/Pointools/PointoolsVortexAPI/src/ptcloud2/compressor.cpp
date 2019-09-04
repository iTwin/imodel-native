#include "PointoolsVortexAPIInternal.h"

#include <ptcloud2/compressor.h>
#include <mathimf.h>

using namespace pcloud;
using namespace pt;

void OctreeCompressor::compress(pt::vector3s *geom, int size, pt::vector3s p, int depth)
{
	if (depth++ > _maxDepth)
	{
		delete [] geom;
		return;
	}

	pt::vector3s *_child[8];
	int _childsize[8];

	/*check occupancy*/ 
	ubyte oct;
	ubyte occ = 0;
	memset(_childsize, 0, sizeof(int)*8);
	memset(_child, 0, sizeof(void*)*8);

	int i;
	/*count points in each of the octants*/ 
	for (i=0; i<size; i++)
	{
		oct = 0;
		if (geom[i].x > p.x) oct |= 1;
		if (geom[i].y > p.y) oct |= 2;
		if (geom[i].z > p.z) oct |= 4;
		occ |= 1 << oct;
		
		++_childsize[oct];
	}

	/*allocate child geometry arrays*/ 
	for (i=0; i<8; i++)
		if (_childsize[i]) _child[i] = new pt::vector3s[_childsize[i]];

	memset(_childsize, 0, sizeof(int)*8);

	/*place points into arrays*/ 
	for (i=0; i<size; i++)
	{
		oct = 0;
		if (geom[i].x > p.x) oct |= 1;
		if (geom[i].y > p.y) oct |= 2;
		if (geom[i].z > p.z) oct |= 4;
		
		_child[oct][_childsize[oct]] = geom[i];
		++_childsize[oct];
	}
	delete [] geom;

	short shift = 1 << (15 - depth);

	pt::vector3s p1;

	for (i=0; i<8;i++) 
	{
		p1 = p;

		if (_childsize[i]) 
		{
			if (i & 1)	p1.x += shift;
			else		p1.x -= shift;

			if (i & 2)	p1.y += shift;
			else		p1.y -= shift;

			if (i & 4)	p1.z += shift;
			else		p1.z -= shift;

			compress(_child[i], _childsize[i], p1, depth);
		}
	}
	/*push occupany byte onto vector*/ 
	_byteData.push_back(occ);

}
void DeltaEncoder::compress(const pt::vector3s *geom, int size)
{
	std::vector <int64_t> pnts;
	int i;
	/*get data range*/ 
	vector3s _min, _max;
	for (i=0; i<size; i++)
	{
		if (!i)
		{
			_min = geom[0];
			_max = geom[0];
		}
		else 
		{
			if (_min.x > geom[i].x) _min.x = geom[i].x;
			if (_min.y > geom[i].y) _min.y = geom[i].y;
			if (_min.z > geom[i].z) _min.z = geom[i].z;

			if (_max.x < geom[i].x) _max.x = geom[i].x;
			if (_max.y < geom[i].y) _max.y = geom[i].y;
			if (_max.z < geom[i].z) _max.z = geom[i].z;
		}
	}
	vector3s range = _max - _min;
	int a, b, c;
	range.dominant_axes(a,b,c);

	for (i=0; i<size; i++)
	{
		int64_t p = 0;
		vector3s s(geom[i][c], geom[i][b], geom[i][a]);
		memcpy(&p, &s, sizeof(vector3));
		pnts.push_back(p);
	}
	std::stable_sort(pnts.begin(), pnts.end());

	/*find largest difference*/ 
	int64_t diff =0;
	std::vector<int64_t> diffv;

	for (i=0; i<size-1; i++)
	{
		int64_t d = pnts[i+1] - pnts[i];
		diffv.push_back(d);
		if ( d > diff) diff = d;
	}
	/*find number bits needed to encode deltas*/ 
	int bits = 0;
	while (diff > 1) { diff /= 2; bits++; }	

	compress(diffv);
}
void DeltaEncoder::compress(std::vector<int64_t> &diff)
{
	/*find largest difference*/ 
	int64_t df =0;
	int i;
	int enc8 =0;
	int enc256 = 0;
	int enc512 = 0;
	int enc65336 = 0;
	int enc32bit = 0;
	int size = diff.size();

	for (i=0; i<size; i++)
	{
		int64_t d = diff[i+1] - diff[i];
		if ( d > df) df = d;
		if ( d <= 256) ++enc256;
		if ( d <= 512) ++enc512;
		if ( d <= 8) ++enc8;
		if ( d <= 65336) ++enc65336;
		if ( d > 4294967296) ++enc32bit;
	}	
	/*find number bits needed to encode deltas*/ 
	int bits = 0;
	while (df > 1) { df /= 2; bits++; }	
}


