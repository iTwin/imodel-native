#pragma once
#include "DataSourceDefs.h"
#include <string>
#include "DataSourceStatus.h"

const wchar_t	DATA_SOURCE_URL_SEPARATOR						= L'/';
const wchar_t	DATA_SOURCE_URL_SEPARATOR_STR[2]				= L"/";

const wchar_t	DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR		= L':';
const wchar_t	DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR_STR[2] = L":";

const wchar_t	DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR			= L'\\';
const wchar_t	DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR_STR[2]	= L"\\";

const wchar_t	DATA_SOURCE_URL_DIR_UP_STR[3]					= L"..";

const wchar_t	DATA_SOURCE_VIRTUAL_URL_SEPARATOR				= L'.';
const wchar_t	DATA_SOURCE_VIRTUAL_URL_SEPARATOR_STR[2]		= L".";



class DataSourceURL : public std::wstring
{

protected:

	DataSourceStatus		normalizeDirUp			(DataSourceURL &str);

public:

CLOUD_EXPORT							DataSourceURL			(void);
CLOUD_EXPORT							DataSourceURL			(wchar_t *str);
CLOUD_EXPORT							DataSourceURL			(const std::wstring &str);

CLOUD_EXPORT		bool				isWindowsFilePath		(void) const;
CLOUD_EXPORT		DataSourceStatus	getFilePath				(std::wstring &filePath) const;

CLOUD_EXPORT		DataSourceStatus	normalize				(void);
CLOUD_EXPORT		DataSourceStatus	normalizeDirUp			(void);

CLOUD_EXPORT		DataSourceStatus	getDirectory			(unsigned int directoryIndex, DataSourceURL &dest) const;
CLOUD_EXPORT		DataSourceStatus	getPathAfterDirectory	(unsigned int directoryIndex, DataSourceURL &dest) const;

CLOUD_EXPORT		DataSourceStatus	getContainerAndBlob		(DataSourceURL &containerName, DataSourceURL &blobPathName) const;

CLOUD_EXPORT		unsigned int		findAndReplace			(const std::wstring & findStr, const std::wstring & replaceStr);

CLOUD_EXPORT		DataSourceStatus	collapseDirectories		(DataSourceURL &result) const;

CLOUD_EXPORT		DataSourceStatus	append					(const DataSourceURL &directory);

};
