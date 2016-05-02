#pragma once

#include <pt/bitarray.h>
#include <ptcloud2/bitvector.h>
#include <pt/boundingbox.h>

#define VOXEL_DIVISIONS 8
#define PLATE_DIVISIONS (VOXEL_DIVISIONS*VOXEL_DIVISIONS)
#define TOTAL_DIVISIONS (VOXEL_DIVISIONS*VOXEL_DIVISIONS*VOXEL_DIVISIONS)

namespace pt
{
class BitVoxelGrid
{
public:
	BitVoxelGrid( double x, double y, double z, double xd, double yd, double zd, double spacing )
	{
		_grid = new bitvector*[TOTAL_DIVISIONS];
		memset(_grid, 0, sizeof(void*) * TOTAL_DIVISIONS);

		_xd = xd / spacing;
		_yd = yd / spacing;
		_zd = zd / spacing;

		_xdv = xd / VOXEL_DIVISIONS;
		_ydv = yd / VOXEL_DIVISIONS;
		_zdv = zd / VOXEL_DIVISIONS;

		_xdv_s = _xdv / spacing;
		_ydv_s = _ydv / spacing;
		_zdv_s = _zdv / spacing;

		_spacing = spacing;

		_x = x;
		_y = y;
		_z = z;
	}
	~BitVoxelGrid()
	{
		for (int i=0;i<TOTAL_DIVISIONS;i++)
			if (_grid[i]) delete _grid[i];
	}
	void getBoundingBox(BoundingBox &bb)
	{
		bb.setBox(_x + _xd * _spacing, _x, _y + _yd * _spacing, _y, _z + _zd * _spacing, _z );
	}
	void set( double x, double y, double z, bool val )
	{
		int pnt;
		bitvector *bv = getVoxel( x, y, z, pnt, true );
		
		if (bv && pnt >= 0)
			bv->assign(pnt, val);
	}
	bool get( double x, double y, double z )
	{
		int pnt;
		bitvector *bv = getVoxel( x, y, z, pnt, true );
		
		if (bv && pnt >= 0)
			return bv->value(pnt);

		return false;
	}
	int getIndex( double x, double y, double z, int &pnt ) const 
	{
		x -= _x;
		y -= _y;
		z -= _z;

		int xi = x / _xdv;
		int yi = y / _ydv;
		int zi = z / _zdv;

		x -= xi * _xdv;
		y -= yi * _ydv;
		z -= zi * _zdv;

		x /= _spacing;
		y /= _spacing;
		z /= _spacing;

		pnt = (int)x + (int)y * _xdv_s + (int)z * _ydv_s;

		int vindex = xi + yi * VOXEL_DIVISIONS + zi * PLATE_DIVISIONS;

		return vindex;
	}
	bitvector *getVoxel( double x, double y, double z , int &pnt, bool create = false )
	{
		pnt = -1;
		if ( x < _x ) return 0;
		if ( y < _y ) return 0;
		if ( z < _z ) return 0;

		if ( x > _x + _xd) return 0;
		if ( y > _y + _yd) return 0;
		if ( z > _z + _zd) return 0;

		int index = getIndex( x,y,z, pnt );

		if (index > TOTAL_DIVISIONS) return 0;

		bitvector *bv = _grid[index];

		if (create && !bv)
		{
			bv = new bitvector( _xdv_s * _ydv_s * _zdv_s );
			bv->assign_all(false);
		}
		return bv;
	}
private:
	bitvector *_grid;
	
	/* corner */ 
	double _x;
	double _y;
	double _z;

	/* dimensions */ 
	unsigned int _xd;
	unsigned int _yd;
	unsigned int _zd;

	/* dimension of voxel */ 
	double _xdv;
	double _ydv;
	double _zdv;

	/* dimension of voxel in spacing */ 
	unsigned int _xdv_s;
	unsigned int _ydv_s;
	unsigned int _zdv_s;
};
}
