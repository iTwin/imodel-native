#pragma once
#include <pt/os.h>
#include <ptcloud2/bitvector.h>
#include <pt/boundingbox.h>

#define GUARD_BITS 128	//floating pt inprecision can cause overrun

namespace pt
{
class BitVoxelGrid
{
public:
	BitVoxelGrid( double x, double y, double z, double xd, double yd, double zd, double spacing )
	{
		_xd = xd;	// delta x
		_yd = yd;	// delta y
		_zd = zd;	// delta z

		_xd_s = xd / spacing;	// dist in voxels
		_yd_s = yd / spacing;	
		_zd_s = zd / spacing;

		_divx = 1+(xd / spacing) / 64;	// number of divisions in X
		_divy = 1+(yd / spacing) / 64;  // in Y
		_divz = 1+(zd / spacing) / 64;  // in Z

		_xdv = xd / _divx;			// size of division in X
		_ydv = yd / _divy;			// in Y
		_zdv = zd / _divz;			// in Z

		_xdv_s = _xdv / spacing;	// voxel dim of div in X
		_ydv_s = _ydv / spacing;	// in Y
		_zdv_s = _zdv / spacing;	// in Z

		if (_xdv_s <= 0) _xdv_s = 1;	// check for zero thickness
		if (_ydv_s <= 0) _ydv_s = 1;
		if (_zdv_s <= 0) _zdv_s = 1;

		_spacing = spacing;

		_x = x;
		_y = y;
		_z = z;
	
		// create empty pointer array
		try
		{
			_grid = new bitvector*[_divx*_divy*_divz+1];	
		}
		catch (...)
		{
			_grid = 0;
			return;
		}
		memset(_grid, 0, sizeof(void*) * _divx*_divy*_divz+1);
	}
	~BitVoxelGrid()
	{
		clear();
		delete [] _grid;
	}
	__int64 countSet() const
	{
		if (!_grid) return 0;

		__int64 count = 0;
		for (unsigned int i=0;i<(_divx*_divy*_divz);i++)
		{
			if (_grid[i]) count += _grid[i]->count();
		}
		return count;
	}
	void clear()
	{
		if (!_grid) return;

		for (unsigned int i=0;i<(_divx*_divy*_divz);i++)
		{
			if (_grid[i])
			{
				delete _grid[i];
				_grid[i] = 0;
			}
		}
	}
	void getBoundingBox(BoundingBox &bb)
	{
		bb.setBox(_x + _xd * _spacing, _x, _y + _yd * _spacing, _y, _z + _zd * _spacing, _z );
	}
	bool set( const double *pnt, bool val=true )
	{
		int p;
		bitvector *bv = getVoxel( pnt[0], pnt[1], pnt[2], p, true );
		
		if (bv && p >= 0)
		{
			bv->assign(p, val);
			return true;
		}
		return false;

	}
	bool set( double x, double y, double z, bool val=true )
	{
		int pnt;
		bitvector *bv = getVoxel( x, y, z, pnt, true );
		
		if (bv && pnt >= 0)
		{
			bv->assign(pnt, val);
			return true;
		}
		return false;
	}
	bool get( const double *pnt, bool &val )
	{
		int p;
		bitvector *bv = getVoxel( pnt[0], pnt[1], pnt[2], p, true );
		
		if (bv && p >= 0)
		{
			val = bv->value(p);
			return true;
		}
		return false;
	}
	bool get( double x, double y, double z, bool &val )
	{
		int pnt;
		bitvector *bv = getVoxel( x, y, z, pnt, true );
		
		if (bv && pnt >= 0)
		{
			val = bv->value(pnt);
			return true;
		}
		return false;
	}
	unsigned __int64 getPntIndex( double x, double y, double z ) const
	{
		struct I { unsigned int a; unsigned int b; };
		union I64 { unsigned __int64 i64; I i; };
		I64 p;

		int a = 0;
		int b = getIndex(x,y,z,a);
		
		p.i.a = a;
		p.i.b = b;

		return p.i64;

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

		int vindex = xi + yi * _divx + zi * (_divx * _divy);

		return vindex;
	}
	bitvector *getVoxel( double x, double y, double z , int &pnt, bool create = false )
	{
		if (!_grid) return 0;

		pnt = -1;
		if ( x < _x ) return 0;
		if ( y < _y ) return 0;
		if ( z < _z ) return 0;

		if ( x > _x + _xd) return 0;
		if ( y > _y + _yd) return 0;
		if ( z > _z + _zd) return 0;

		unsigned int index = getIndex( x,y,z, pnt );

		if (index >= (_divx*_divy*_divz)) return 0;
		if (index < 0) return 0;
		
		bitvector *bv = _grid[index];

		if (create && !bv)
		{
			try
			{
				bv = new bitvector( _xdv_s * _ydv_s * _zdv_s + GUARD_BITS);
				bv->assign_all(false);
			}
			catch(...)
			{
				return 0;
			}
			_grid[index] = bv;
		}
		return bv;
	}
private:
	bitvector **_grid;
	
	/* corner */ 
	double _x;
	double _y;
	double _z;

	/* dimensions */ 
	double _xd;
	double _yd;
	double _zd;

	/* dimensions in spacings*/ 
	unsigned int _xd_s;
	unsigned int _yd_s;
	unsigned int _zd_s;

	/* divisions in x, y, z of space*/
	unsigned int _divx;
	unsigned int _divy;
	unsigned int _divz;

	/* dimension of voxel */ 
	double _xdv;
	double _ydv;
	double _zdv;

	/* dimension of voxel in spacing */ 
	unsigned int _xdv_s;
	unsigned int _ydv_s;
	unsigned int _zdv_s;

	/* the spacing value */ 
	double _spacing;
};
}
