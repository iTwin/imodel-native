/*----------------------------------------------------------*/ 
/* DataChannel.cpp				 							*/ 
/* Data Channel Implementation file							*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 

#include "PointoolsVortexAPIInternal.h"
#include <memory.h>

//#define DEBUG_OUTPUT

#include <math.h>
#include <ptcloud2/datachannel.h>
#include <pt/geomtypes.h>


#include <pt/ptstring.h>
#include <pt/boundingbox.h>

#include <pt/trace.h>
#include <random>

using namespace pcloud;
using namespace pt;

DataChannel::DataChannel(DataType native, DataType storeas, int multiple, double *offs, double *scal, const void *samples, uint num_samples)
{
	_storeas = storeas;
	_native = native;
	_typesize = dataTypeSize(storeas);
	_multiple = multiple;

	_data = 0;
	_count = 0;
	_offset = new double[_multiple];
	_scaler = new double[_multiple];
	_samples = 0;
	_numSamples = num_samples;
	_range = 0;

	if (_numSamples)
	{
		_samples = new ubyte[_multiple * _typesize * _numSamples];
		memcpy(_samples, samples, _multiple * _typesize * _numSamples);
	}

	if (offs && scal)
	{
		memcpy(_offset, offs, sizeof(double)*_multiple);
		memcpy(_scaler, scal, sizeof(double)*_multiple);
	}
	else 
		for (int i=0; i<_multiple; i++)
		{
			_offset[i] = 0;
			_scaler[i] = 1.0f;
		}
}
//-----------------------------------------------------------------------------
DataChannel::~DataChannel()
{
	dump(); 
	try
	{
	if (_offset) delete [] _offset; 
	if (_scaler) delete [] _scaler; 
	if (_samples) delete [] _samples;
	} catch(...) {}
}
//-----------------------------------------------------------------------------
uint pcloud::dataTypeSize(DataType dt)
{
	switch(dt)
	{
	case Byte8: 
	case UByte8: return 1;
	case Short16:
	case UShort16: return 2;
	case Long32:
	case ULong32:
	case Float32: return 4;
	case Long64: 
	case Float64: return 8;
	}
	return 0;
};
//-----------------------------------------------------------------------------
Channel pcloud::channel(int i)
{
	switch(i)
	{
	case 0: return PCloud_None;
	case 1: return PCloud_Geometry;
	case 2: return PCloud_RGB;
	case 3: return PCloud_Intensity;
	case 4: return PCloud_Normal;
	case 5: return PCloud_Classification;
	case 6: return PCloud_Grid;
	case 7: return PCloud_Filter;
	}
	return PCloud_None;
};
//-----------------------------------------------------------------------------
DataType pcloud::datatype(int i)
{
	switch(i)
	{
	case 0: return NullType;
	case 1: return Float32;
	case 2: return Float64;
	case 3: return Bit1;
	case 4: return Byte8;
	case 5: return UByte8;
	case 6: return Short16;
	case 7: return UShort16;
	case 8: return Long32;
	case 9: return ULong32;
	case 10: return Long64;
	}
	return NullType;
}
//-----------------------------------------------------------------------------
void DataChannel::readNative(void *d, double tolerance)
{
	assert(d);
	if (!d) return;

	if (_native == _storeas)		
		memcpy(_data, d, _typesize*_multiple*_count);
	else
	{
		tolerance *= 0.1;

		/*handle this case for now*/ 
		if (_native == Float32 && _storeas == Short16 && _multiple == 3)
		{
			pt::Bounds<3, double> bb;
			bb.makeEmpty();

			pt::vector3 *geom = reinterpret_cast<pt::vector3 *>(d);
			
			uint i; vector3d p;
			/*calc bounds of data*/ 
			for (i=0; i<_count; i++)
			{
				p.set(geom[i]);
				bb.expand(p);
			}

			pt::vector3s comppoint;
			pt::vector3d lower(&bb.lower(0));
			pt::vector3d range(&bb.upper(0));
			range -= lower;

			/*calc step size*/ 
			range /= 65335;
			
			/*set minimum step size*/ 
			if (fabs(range.x) < tolerance) range.x = tolerance; 
			if (fabs(range.y) < tolerance) range.y = tolerance; 
			if (fabs(range.z) < tolerance) range.z = tolerance; 

			/*calculate offset and scaler values*/ 
			pt::vector3d offs = (lower + (range / 2.0f));
			_offset[0] = offs.x;
			_offset[1] = offs.y;
			_offset[2] = offs.z;

			memcpy(_scaler, &range, sizeof(double)*3);
			
			for (i=0; i<_count; i++)
			{
				p.set(geom[i]);
				p -= offs;
				p /= range;

				comppoint.x = (short)p.x;	
				comppoint.y = (short)p.y;	
				comppoint.z = (short)p.z;	
				memcpy(&_data[i*sizeof(pt::vector3s)], &comppoint, sizeof(pt::vector3s));
			}
		}
		else
		{
			assert(0);
		}
	}
};
//-----------------------------------------------------------------------------
void DataChannel::readStore(void *d)
{
	memcpy(_data, d, _typesize*_multiple*_count);
}
//-----------------------------------------------------------------------------
void DataChannel::readEnd(void *d, uint size)
{
	assert(d);
	if (!d) return;

	if (_native == _storeas)		
	{
		assert(size <= _count);
		memcpy(&_data[_count-size], d, _typesize*_multiple*size);
	}
	else
	{
		/*requires decompression or compression*/ 
	}
};
//-----------------------------------------------------------------------------
void DataChannel::dump()
{
	if (_data)
	{
		delete [] _data;
		_data = 0;
		_count = 0;
	}
}
//-----------------------------------------------------------------------------
bool DataChannel::allocate(int sz)
{
	if (_data) delete [] _data;

	_count = 0;
	try
	{
		if (sz) 
		{
			_data = new ubyte[_typesize*_multiple*sz];
			_count = sz;
			return true;
		}
	}
	catch (...) { return false;	}
	return false;
}
//-----------------------------------------------------------------------------
void DataChannel::replace(ubyte *d, int size)
{ 
	if (_data) delete [] _data;
	_data = d; 
	_count = size; 
}
//-----------------------------------------------------------------------------
void DataChannel::replace(ubyte *d)
{ 
	if (_data) delete [] _data;
	_data = d; 
}
//-----------------------------------------------------------------------------
void DataChannel::copy(ubyte *d)
{
	memcpy(_data, d, bytesize());
}
//-----------------------------------------------------------------------------
bool DataChannel::resize(uint sz)
{
	// sanity check
	if (sz > 10e6) return false;

	// no change
	if (sz == _count) return true;

	// byte size
	size_t newsize = _typesize * _multiple * sz;

	if ( sz <= 0 )
	{
		delete [] _data;
		_data = 0;
		_count = 0;
		return true;
	}
	ubyte *d=0;
	
#ifdef DEBUG_OUTPUT
	pt::String debug_out;
	if (_count > sz) debug_out.format( "Channel downsize %i\n", (int)newsize );
	else  debug_out.format( "Channel upsize %i\n", (int)newsize );
	OutputDebugStringA( debug_out.c_str() );
#endif
	try
	{
		d = new ubyte[newsize];
	}
	catch (std::bad_alloc b)
	{
		return false;
	}

	if (!d)
	{
		return false;
	}
	if (_data)
	{
		uint transfer = _typesize * _multiple * (_count > sz ? sz : _count);
		memset(&d[transfer], 0, newsize - transfer);
		memcpy(d, _data, transfer);
		delete [] _data;
	}
	_data = d;
	_count = sz;
	
	return true;
}
//-----------------------------------------------------------------------------
void DataChannel::generateRandomIndex(std::vector<int> &index) const
{
	index.clear();
	index.reserve(_count);

	for (uint i=0; i<_count; i++) index.push_back(i);
	
	/*randomize order of points*/ 
    std::random_device rd;
    std::mt19937 rng(rd());
	std::shuffle(index.begin(), index.end(), rng);
}
//-----------------------------------------------------------------------------
bool DataChannel::repositionByIndex(const std::vector<int> &index)
{
	assert(_data);
	if (!_data) return false;

	assert(index.size() == _count);
	int objsize = _typesize * _multiple;

	ubyte *temp = 0;

	try 
	{
		temp = new ubyte[objsize * _count];
	}
	catch(...) { return false; }

	memcpy(temp,_data, objsize*_count);

	for (uint i=0;i<_count;i++)
		memcpy(&_data[i*objsize], &temp[index[i]*objsize], objsize);

	delete [] temp;
	return true;
}
//-----------------------------------------------------------------------------
template <class T>
struct CheckMinMax
{
	static void compute(int multiple, ubyte *data_, int count, T *min, T *max)
	{
		T *data = reinterpret_cast<T*>(data_);

		// initial values
		memcpy(min, data, multiple * sizeof(T));
		memcpy(max, data, multiple * sizeof(T));

		for (int i=0;i<count;i++)
		{
			for (int j=0; j<multiple; j++)
			{
				T val = data[i * multiple + j];
				if (val < min[j]) min[j] = val;
				if (val > max[j]) max[j] = val;
			}
		}
	}
};
//-----------------------------------------------------------------------------
void DataChannel::computeRange()
{
	if (!_data) return;

	switch (_storeas)
	{
	case Float32:
		{
		float *range = new float[ _multiple * 2];	// min and then max
		CheckMinMax<float>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	case Float64:
		{
		double *range = new double[ _multiple * 2];	// min and then max
		CheckMinMax<double>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	case Short16:
		{
		short *range = new short[ _multiple * 2];	// min and then max
		CheckMinMax<short>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	case UShort16:
		{
		unsigned short *range = new unsigned short[ _multiple * 2];	// min and then max
		CheckMinMax<unsigned short>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	case Byte8:
		{
		char *range = new char[ _multiple * 2];	// min and then max
		CheckMinMax<char>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	case UByte8:
		{
		unsigned char *range = new unsigned char[ _multiple * 2];	// min and then max
		CheckMinMax<unsigned char>::compute( _multiple, _data, _count, range, &range[_multiple]);
		_range = reinterpret_cast<ubyte*>(range);
		}
		break;
	}
}


