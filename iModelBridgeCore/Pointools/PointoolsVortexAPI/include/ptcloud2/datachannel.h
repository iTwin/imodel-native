/*----------------------------------------------------------*/ 
/* DataChannel.h				 							*/ 
/* Data Channel Interface file								*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2008								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTCLOUD_DATACHANNEL
#define POINTCLOUD_DATACHANNEL 1

#define MAX_CHANNELS	8
#define MAX_LOAD_CHANNELS 6

#include <ptcloud2/defs.h>
#include <ptcloud2/pcloud.h>
#include <vector>

namespace pcloud
{
	enum Channel
	{
		PCloud_None				= 0,
		PCloud_Geometry			= 1,
		PCloud_RGB				= 2,
		PCloud_Intensity		= 3,
		PCloud_Normal			= 4,
		PCloud_Classification	= 5,
		PCloud_Grid				= 6,
		PCloud_Filter			= 7,
		PCloud_NumChannelTypes	= 8
	};
	enum DataType
	{
		NullType	= 0,
		Float32		= 1,
		Float64		= 2,
		Bit1		= 3,
		Byte8		= 4,
		UByte8		= 5,
		Short16		= 6,
		UShort16	= 7,
		Long32		= 8,
		ULong32		= 9, 
		Long64		= 10
	};
	PCLOUD_API Channel channel(int i);
	PCLOUD_API DataType datatype(int i);

	PCLOUD_API uint dataTypeSize(DataType dt);

	/* Data Channel									*/ 
	/* Requires External Locking in MT environment	*/ 
	class PCLOUD_API DataChannel
	{
	public:	
		DataChannel(DataType native, DataType storeas, int multiple, double *offs=0, double *scal=0, const void *samples=0, uint num_samples=0);
		~DataChannel();

		void generateRandomIndex(std::vector<int> &index) const;
		bool repositionByIndex(const std::vector<int> &index);

		uint size()			const { return _count; }
		uint bytesize()		const { return size() * getPointSize(); }

		uint getPointSize() const	{return typesize() * multiple();}

		void replace(ubyte *data, int size);
		void replace(ubyte *data);
		void copy(ubyte*data);

		void readNative(void *data, double tolerance);
		void readStore(void *data);
		void readEnd(void *data, uint size);

		inline ubyte* data()				{ return _data; }
		inline const ubyte* data() const	{ return _data; }

		bool allocate(int count);
		bool resize(uint count);
		void dump();
		
		/*access*/ 
		void *element(uint index) const  { return &_data[_multiple * _typesize * index]; }
		inline void set(uint index, const void*value) { memcpy(&_data[_multiple * _typesize * index], value, _typesize*_multiple); }
		inline bool validIndex(uint index) { return (_multiple * _typesize * index) < bytesize(); }

		template<class T>	bool getSample(uint index, T &v) const	
		{ 
			if (!_samples || index >= _numSamples) return false; 
			 v = *((T*)(&_samples[_multiple * _typesize * index])); 
			 return true;
		}
		const void *begin() const { return data(); }
		const void *end() const { return data() + bytesize(); }
		template<class T>	inline void begin(const T** ptr) const { (*ptr) = reinterpret_cast<const T*>(_data); }
		template<class T>	inline void end(const T** ptr) const { begin(ptr); (*ptr) += _count; }

		template<class T>	inline void begin(T** ptr) { (*ptr) = reinterpret_cast<T*>(_data); }
		template<class T>	inline void end(T** ptr) { begin(ptr); (*ptr) += _count; }

		template<class T> inline void getConstPtr(const T **ptr, int index)  const { (*ptr) = reinterpret_cast<const T*>(&_data[_multiple * _typesize * index]); }
		template<class T> inline void getptr(T **ptr, int index)  { (*ptr) = reinterpret_cast<T*>(&_data[_multiple * _typesize * index]); }
		template<class T> inline void getval(T &v, int index) const { v = *((T*)(&_data[_multiple * _typesize * index])); }

		inline bool valid() const { return _data ? true : false; }
		inline uint typesize() const { return _typesize; }
		inline uint multiple() const { return _multiple; }

		inline bool hasRange() const { return _range ? true : false; }
		template<class T> bool getRangeMin(T &v) const { if (!_range) return false; v = (reinterpret_cast<T*>(_range)[0]); return true; }
		template<class T> bool getRangeMax(T &v) const { if (!_range) return false; v = (reinterpret_cast<T*>(_range)[1]); return true; }
		void computeRange();

#ifndef __INTEL_COMPILER
		DataType nativeType() const { return (DataType)_native;}
		DataType storeType() const { return (DataType)_storeas; }
#else
		const DataType &nativeType() const { return _native;}
		const DataType &storeType() const { return _storeas; }
#endif
		const double* offset() const { return _offset; }
		const double* scaler() const { return _scaler; }
		
	protected:
		ushort	_typesize;
		ushort	_multiple;
		ushort	_storeas;
		ushort	_native;

		double *_scaler;
		double *_offset;

		uint	_count;
		ubyte*	_data;

		ubyte*	_samples;
		uint	_numSamples;

		ubyte*	_range;
	};
}
#endif