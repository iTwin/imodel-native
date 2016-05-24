#pragma once

#include <string>
#include "DataSourceStatus.h"

const wchar_t	DATA_SOURCE_URL_SEPARATOR						= L'/';
const wchar_t	DATA_SOURCE_URL_SEPARATOR_STR[2]				= L"/";

const wchar_t	DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR		= L':';
const wchar_t	DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR_STR[2] = L":";

const wchar_t	DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR			= L'\\';
const wchar_t	DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR_STR[2]	= L"\\";

const wchar_t	DATA_SOURCE_VIRTUAL_URL_SEPARATOR				= L'.';
const wchar_t	DATA_SOURCE_VIRTUAL_URL_SEPARATOR_STR[2]		= L".";


class DataSourceURL : public std::wstring
{


public:

							DataSourceURL			(void);
							DataSourceURL			(wchar_t *str);
							DataSourceURL			(const std::wstring &str);

		bool				isWindowsFilePath		(void) const;
		DataSourceStatus	getFilePath				(std::wstring &filePath) const;

		DataSourceStatus	normalize				(void);

		DataSourceStatus	getDirectory			(unsigned int directoryIndex, DataSourceURL &dest) const;
		DataSourceStatus	getPathAfterDirectory	(unsigned int directoryIndex, DataSourceURL &dest) const;

		DataSourceStatus	getContainerAndBlob		(DataSourceURL &containerName, DataSourceURL &blobPathName) const;

		unsigned int		findAndReplace			(const std::wstring & findStr, const std::wstring & replaceStr);

		DataSourceStatus	collapseDirectories		(DataSourceURL &result) const;

		DataSourceStatus	appendDirectory			(const DataSourceURL &directory);
};
