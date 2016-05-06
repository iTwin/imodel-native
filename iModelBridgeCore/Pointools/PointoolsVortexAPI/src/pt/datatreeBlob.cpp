#include "PointoolsVortexAPIInternal.h"
#include <pt/datatreeBlob.h>
#include <scz_compress/scz.h>

namespace pt
{
namespace datatree
{
Blob::~Blob()
{
	if (_del) delete _data;
}
Blob::Blob(uint8_t *data, uint32 size, bool del /*= false*/ )
	: _data(data), _size(size), _del(del)
{
	_compression = None;
}

bool Blob::compress( Compression type )
{
	if (_compression != None || !_del) return false;

	if (type == CompressSCZ && _size > 256 && _size < 16777215)
	{
		int new_size=0;
		char *compressed=0;

		int compressed_size = Scz_Compress_Buffer2Buffer((char*)_data, _size, &compressed, &new_size, 1 );

		if (compressed_size)
		{
			// copy compressed data in
			delete [] _data;
			_data = new uint8[new_size];
			memcpy(_data, compressed, new_size);

			_size = new_size;
			_compression = type;

			// free scz
			scz_cleanup();
		}
		return true;
	}
	return false;
}

bool Blob::decompress()
{
	if (_compression==None) return false;

	if (_compression == CompressSCZ)
	{
		int new_size=0;
		char *compressed=0;

		int compressed_size = Scz_Decompress_Buffer2Buffer((char*)_data, _size, &compressed, &new_size );

		if (compressed_size)
		{
			// copy compressed data in
			delete [] _data;
			_data = new uint8[new_size];
			memcpy(_data, compressed, new_size);

			_size = new_size;
			_compression = None;

			// free scz
			scz_cleanup();
		}
		return true;
	}
	return false;
}
} // namespace datatree
} // namespace pt
