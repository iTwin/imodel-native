
#pragma once

#include <ptds/DataSource.h>

#include <string.h>
#include <map>

namespace PTRMI
{
	class Name;
}


namespace ptds
{

class DataSourceStructuredStorage : public DataSource
{

protected:

	typedef DataSource							Super;

	typedef std::map<std::wstring, IStream *>	PathStreamMap;

protected:

	IStream				  *	stream;
	bool					clone;

	static PathStreamMap	pathStreamMap;

protected:

	bool					getStreamFilename				(const FilePath &path, std::wstring &filename);
	bool					getStreamMode					(unsigned int &result);


public:

							DataSourceStructuredStorage		(void);
							DataSourceStructuredStorage		(const FilePath *filepath, IStream *initStream, bool clone = false);
						   ~DataSourceStructuredStorage		(void);

	static DataSourcePtr	createNew						(const FilePath *path);

	DataSourceForm			getDataSourceForm				(void);

	void					destroy							(void);

	static DataSourceType	getDataSourceType				(void);

	void					clear							(void);

	void					setStream						(IStream *initStream)			{stream = initStream;}
	IStream			*		getStream						(void)							{return stream;}

	void					setClone						(bool isClone);
	bool					getClone						(void);

	bool					openForRead						(const FilePath *filepath, bool create = false);
	bool					openForWrite					(const FilePath *filepath, bool create = false);
	bool					openForReadWrite				(const FilePath *filepath, bool create = false);

	bool					validHandle						(void);
	static bool				isValidPath						(const FilePath *filepath);

	void					close							(void);
	bool					closeAndDelete					(void);

	Size					readBytes						(Data *buffer, Size number_bytes);
	Size					writeBytes						(const Data *buffer, Size number_bytes);

	DataSize				getFileSize						(void);

	bool					movePointerBy					(DataPointer number_bytes);
	bool					movePointerTo					(DataPointer number_bytes);

	bool					addExistingPathStream			(const FilePath &path, IStream *initStream);
	static IStream *		getExistingPathStream			(const FilePath &path);
	bool					removeExistingPathStream		(const FilePath &path);

	static HRESULT			openStructuredStorage			(const FilePath &filepath, IStorage *parent, unsigned int mode, IStorage **result, bool create);
	static HRESULT			openStructuredStorageStream		(const FilePath &filepath, IStorage *parent, unsigned int mode, IStream **result, bool create);
};

} // End ptds namespace
