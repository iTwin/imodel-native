#include "stdafx.h"
#include "DataSourceURL.h"
#include "include\DataSourceURL.h"

DataSourceURL::DataSourceURL(void)
{

}

DataSourceURL::DataSourceURL(wchar_t * str) : std::wstring(str)
{

}


DataSourceURL::DataSourceURL(const std::wstring &str) : std::wstring(str)
{

}

void DataSourceURL::operator=(const DataSourceURL & url)
    {
    m_separator = url.m_separator;
    this->std::wstring::operator=((std::wstring)url);
    }

void DataSourceURL::setSeparator(const std::wstring& separator)
    {
    m_separator = separator;
    }

std::wstring DataSourceURL::getSeparator() const
    {
    return m_separator;
    }

bool DataSourceURL::isWindowsFilePath(void) const
{
                                                            // Assume windows filepath if second character is ':'
    return ((*this)[1] == DATA_SOURCE_URL_WINDOWS_DEVICE_SEPARATOR);
}


DataSourceStatus DataSourceURL::getFilePath(std::wstring &filePath) const
{
    if (isWindowsFilePath() == false)
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_File_Path);

    DataSourceURL    result;

    result = *this;
                                                            // Revert forward slashes to back slashes
    result.findAndReplace(std::wstring(DATA_SOURCE_URL_SEPARATOR_STR), std::wstring(DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR_STR));

    filePath = result;

    return DataSourceStatus();
}

DataSourceStatus DataSourceURL::normalize(void)
{
                                                            // Normalize windows file separators to forward slash
    findAndReplace(std::wstring(DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR_STR), std::wstring(DATA_SOURCE_URL_SEPARATOR_STR));

    normalizeDirUp();

    return DataSourceStatus();
}

// Folder1/Folder2/Folder3/../../Folder4/../File.blob

// a/b/../file.bin

DataSourceStatus DataSourceURL::normalizeDirUp(void)
{
    return normalizeDirUp(*this);
}


DataSourceStatus DataSourceURL::normalizeDirUp(DataSourceURL &s)
{
    size_t            f;
    size_t            g;
    DataSourceURL    n;

    f = s.find(DATA_SOURCE_URL_DIR_UP_STR, 0);

    if (f != npos && f != 0)
    {
        DataSourceURL    pre;

        pre = s.substr(0, f - 1);

        g = pre.find_last_of(DATA_SOURCE_URL_SEPARATOR);

        if (g != npos)
        {
            n = s.substr(0, g);
            n += DATA_SOURCE_URL_SEPARATOR;

            n += s.substr(f + 3, length() - (f + 3));

            normalizeDirUp(n);
        }
    }

    *this = s;

    return DataSourceStatus();
}


DataSourceStatus DataSourceURL::getDirectory(unsigned int directoryIndex, DataSourceURL & dest) const
{
    size_t    position;
    size_t    start = 0;

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
    size_t    position;
    size_t    start = 0;

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
    DataSourceStatus    status;
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
    size_t            pos                    = 0;
    unsigned int    count                = 0;

    size_t            findStrLength        = findStr.length();
    size_t            replaceStrLength    = replaceStr.length();


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

bool DataSourceURL::endsWithSeparator()
    {
    bool endsWithSeparator = true;
    if (length() < m_separator.length()) return !endsWithSeparator;
    for (auto c1 = rbegin(), c2 = m_separator.rbegin(); c2 != m_separator.rend() && endsWithSeparator; c1++, c2++)
        endsWithSeparator = *c1 == *c2;
    return endsWithSeparator;
    }

DataSourceStatus DataSourceURL::append(const DataSourceURL & path)
{
    if (length() > 0 && path.length() > 0 && !endsWithSeparator() && (*this)[length() - 1] != DATA_SOURCE_URL_WINDOWS_FILE_SEPARATOR)
    {
        *this += m_separator;
    }

    *this += path;

    return DataSourceStatus();
}
