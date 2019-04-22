#pragma once

#include <ptds/DataSource.h>

namespace ptds
{
	class DataSourceNull : public DataSource
	{

	public:

		DataSourceForm			getDataSourceForm		(void);

		static DataSourcePtr	createNew				(const FilePath *path);

		void					destroy					(void) {}

		static DataSourceType	getDataSourceType		(void);

		bool					openForRead				(const FilePath * /*path*/, bool /*create = false*/)	{return false;}
		bool					openForWrite			(const FilePath * /*path*/, bool /*create = false*/)	{return false;}
		bool					openForReadWrite		(const FilePath * /*path*/, bool /*create = false*/) {return false;}

		bool					validHandle				(void) {return false;}
		static bool				isValidPath				(const FilePath * /*path*/) {return false;}

		void					close					(void) {}
		bool					closeAndDelete			(void) {return false;}

		Size					readBytes				(Data * /*buffer*/, Size /*number_bytes*/) {return 0;}
		Size					writeBytes				(const Data * /*buffer*/, Size /*number_bytes*/) {return 0;}

		DataSize				getFileSize				(void) {return 0;}

		bool					movePointerBy			(DataPointer /*number_bytes*/) {return false;}
		bool					movePointerTo			(DataPointer /*number_bytes*/) {return false;}

	};

} // End ptds namespace
