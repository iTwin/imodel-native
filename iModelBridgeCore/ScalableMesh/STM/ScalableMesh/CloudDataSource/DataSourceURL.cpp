#include "stdafx.h"
#include "DataSourceURL.h"

DataSourceURL::DataSourceURL(void)
{

}

DataSourceURL::DataSourceURL(wchar_t * str) : std::wstring(str)
{

}


DataSourceURL::DataSourceURL(const std::wstring &str) : std::wstring(str)
{

}


DataSourceStatus DataSourceURL::getDirectory(unsigned int directoryIndex, DataSourceURL & dest) const
{
	size_t	position;
	size_t	start = 0;

	for (unsigned int t = 0; t < directoryIndex; t++)
	{
		if ((position = find(DATA_SOURCE_URL_SEPARATOR, start)) == npos)
		{
			dest = L"";
			return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
		}

		if (t == directoryIndex - 1)
		{
			dest = substr(start, position - start);
			return DataSourceStatus();
		}

	}

	return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
}

DataSourceStatus DataSourceURL::getPathAfterDirectory(unsigned int directoryIndex, DataSourceURL & dest) const
{
	size_t	position;
	size_t	start = 0;

	for (unsigned int t = 0; t < directoryIndex; t++)
	{
		if ((position = find(DATA_SOURCE_URL_SEPARATOR, start)) == npos)
		{
			dest = L"";
			return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
		}

		if (t == directoryIndex - 1)
		{
			size_t p = position + 1;
			dest = substr(p, length() - p);
			return DataSourceStatus();
		}

	}

	return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
}

DataSourceStatus DataSourceURL::getContainerAndBlob(DataSourceURL & containerName, DataSourceURL & blobPathName) const
{
	DataSourceStatus	status;
															// Get first container name
	if ((status = getDirectory(1, containerName)).isFailed())
		return status;
															// Get remaining path after root container
	if ((status = getPathAfterDirectory(1, blobPathName)).isFailed())
		return status;

	return status;
}

unsigned int DataSourceURL::findAndReplace(const std::wstring &findStr, const std::wstring &replaceStr)
{
    size_t			pos					= 0;
	unsigned int	count				= 0;

	size_t			findStrLength		= findStr.length();
	size_t			replaceStrLength	= replaceStr.length();


    while ((pos = find(findStr, pos)) != std::string::npos)
	{
         replace(pos, findStrLength, replaceStr);

         pos += replaceStrLength;

		 count++;
    }

    return count;
}

DataSourceStatus DataSourceURL::collapseDirectories(DataSourceURL & result) const
{
	result = *this;

	result.findAndReplace(DataSourceURL(DATA_SOURCE_URL_SEPARATOR_STR), DataSourceURL(DATA_SOURCE_VIRTUAL_URL_SEPARATOR_STR));

	return DataSourceStatus();
}

DataSourceStatus DataSourceURL::appendDirectory(const DataSourceURL & directory)
{
	*this += DATA_SOURCE_URL_SEPARATOR;

	*this += directory;

	return DataSourceStatus();
}
