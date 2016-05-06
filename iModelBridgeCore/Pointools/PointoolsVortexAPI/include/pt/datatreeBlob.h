#pragma once

#define		BLOB_TYPE 129

#include <pt/ptmath.h>

namespace pt
{
namespace datatree
{

//! Blob
//! Maintains binary blob data for writing only
class Blob
{
public:
	enum Compression
	{
		None = 0,
		CompressSCZ = 1,
		CompressZIP = 2
	};

	Blob(uint8_t *data, uint32 size, bool del = false );
	~Blob();

	bool	compress( Compression type );
	bool	decompress();
	bool	isCompressed() const { return _compression ? true : false; }

	bool	_del;
	int		_compression;
    uint8_t	*_data;
	uint32	_size;
};
}
} // namespace