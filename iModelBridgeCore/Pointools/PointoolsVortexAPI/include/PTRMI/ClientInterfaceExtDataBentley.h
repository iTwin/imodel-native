
#pragma once

#include <PTRMI/ClientInterfaceExtData.h>
#include <PTRMI/Status.h>
#include <PTRMI/URL.h>

const unsigned int CLIENT_INTERFACE_EXT_DATA_BENTLEY_MAX_SIZE = 256 * 1024;


namespace PTRMI
{
	class DataBuffer;
	class URL;


	const unsigned short CLIENT_INTERFACE_EXT_DATA_CURRENT_VERSION = 1;

	class ClientInterfaceExtDataBentley : public ClientInterfaceExtData
	{
	public:

		typedef unsigned short		FileVersion;

		typedef uint64_t	FileHeader;

	protected:

		static const FileHeader	CLIENT_INTERFACE_EXT_DATA_BENTLEY_HEADER;

	protected:

		URL						originalServerFile;
		PTRMI::GUID				fileGUID;

		DataBuffer				bentleyData;

	protected:

		Status					readHeader						(const wchar_t *filepath, DataBuffer &buffer);
		Status					writeHeader						(DataBuffer &buffer);

		Status					readBentleyData					(DataBuffer &buffer);
		Status					writeBentleyData				(DataBuffer &buffer);

		void					generateFileGUID				(void);
		void					setFileGUID						(PTRMI::GUID &guid);

	public:

								ClientInterfaceExtDataBentley	(void);

		Status					set								(const wchar_t *originalServerFile, void *bentleyData, unsigned int bentleyDataSize);

		Status					readFile						(const wchar_t *filepath);
		Status					writeFile						(const wchar_t *filepath);

		Status					setOriginalServerFile			(URL &filepath);
		Status					getOriginalServerFile			(URL &filepath);

		Status					setBentleyData					(DataBuffer::Data *data, DataBuffer::DataSize dataSize);
		DataBuffer::Data	*	getBentleyData					(void);

		DataBuffer::DataSize	getBentleyDataSize				(void);

		DataBuffer::Data	*	getExtData						(DataBuffer::DataSize *size);
		DataBuffer::DataSize	getExtDataSize					(void);
		PTRMI::GUID			*	getFileGUID						(void);
	};

} // End PTRMI namespace

