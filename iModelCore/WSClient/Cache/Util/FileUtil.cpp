/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/FileUtil.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/FileUtil.h>

#include <Bentley/BeFile.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define IS_ESCAPE_BYTE(b) ((b & 0xc0) == 0xc0)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t FileUtil::GetFileSize(BeFileNameCR filePath)
    {
    if (filePath.empty())
        {
        return 0;
        }
    uint64_t fileSize = 0;
    if (BeFileNameStatus::Success != BeFileName::GetFileSize(fileSize, filePath))
        {
        return 0;
        }
    return fileSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FileUtil::TruncateFileName(Utf8StringCR fileName)
    {
#if defined(ANDROID)
    // On Android, trim file name to NAME_MAX *bytes*
    return TruncateFileNameUtf8(fileName, NAME_MAX);
#else

#if defined(__APPLE__)
    // On iOS, trim file name to NAME_MAX *characters*
    size_t maximumFileNameLength = NAME_MAX;
#elif defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
    // On Windows, trim file name to 255 *characters*
    size_t maximumFileNameLength = 255;
#else
#error Unknown platform
#endif

    return TruncateFileNameUtf16(fileName, maximumFileNameLength);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FileUtil::TruncateFileNameUtf8(Utf8StringCR fileName, size_t maxLength)
    {
    if (fileName.length() <= maxLength)
        {
        return fileName;
        }

    Utf8String ext(strrchr(fileName.c_str(), '.'));

    size_t limit = 0;
    if (ext.length() < maxLength)
        {
        limit = maxLength - ext.length();
        }
    else
        {
        // Extension cannot be preserved
        limit = maxLength;
        ext.clear();
        }

    size_t end = 0;
    for (size_t i = 0; i < fileName.length() && i <= limit; ++i)
        {
        unsigned char c = fileName[i];
        if (c < 128 || IS_ESCAPE_BYTE(c))
            end = i;
        }

    return fileName.substr(0, end) + Utf8String(ext);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FileUtil::TruncateFileNameUtf16(Utf8StringCR fileName, size_t maxLength)
    {
    WString wFileName(fileName.c_str(), true);
    size_t length = wFileName.length();

    if (length <= maxLength)
        {
        return fileName;
        }

    // Try to preserve extension
    WString ext(wcsrchr(wFileName.c_str(), L'.'));
    size_t end = 0;

    if (ext.length() < maxLength)
        {
        end = maxLength - ext.length();
        }
    else
        {
        // Extension cannot be preserved
        end = maxLength;
        ext.clear();
        }

    return Utf8String(wFileName.substr(0, end) + ext);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileUtil::TruncateFilePath(BeFileNameR filePath)
    {
    return TruncateFilePath(filePath, MAX_PATH - 1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
// TODO: test android behavour with UTF characters as TruncateFileName
// TODO: merge functionality with TruncateFileName
BentleyStatus FileUtil::TruncateFilePath(BeFileNameR filePath, size_t maxPath)
    {
    size_t length = filePath.length();

    if (length < maxPath)
        {
        return SUCCESS;
        }

    BeFileName directory(BeFileName::GetDirectoryName(filePath));
    WString baseName = BeFileName::GetFileNameWithoutExtension(filePath);
    WString ext = BeFileName::GetExtension(filePath);

    if (directory.length() + ext.length() + 2 > maxPath)
        {
        return ERROR;
        }

    WString shortName(baseName.substr(0, maxPath - directory.length() - ext.length() - (ext.empty() ? 0 : 1)));

    directory.AppendToPath(shortName.c_str());
    directory.AppendExtension(ext.c_str());

    filePath = directory;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileUtil::CopyFileContent(BeFileNameCR source, BeFileNameCR target)
    {
    BeFile sourceFile;
    BeFile targetFile;

    auto status = CopyFileContent(source, sourceFile, target, targetFile);

    sourceFile.Close();
    targetFile.Close();

    return status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus FileUtil::CopyFileContent(BeFileNameCR sourcePath, BeFile& source, BeFileNameCR targetPath, BeFile& target)
    {
    if (BeFileStatus::Success != source.Open(sourcePath, BeFileAccess::Read))
        {
        return ERROR;
        }

    if (targetPath.DoesPathExist())
        {
        if (BeFileStatus::Success != target.Open(targetPath, BeFileAccess::Write))
            {
            return ERROR;
            }
        }
    else
        {
        if (BeFileStatus::Success != target.Create(targetPath))
            {
            return ERROR;
            }
        }

    if (BeFileStatus::Success != target.SetSize(0))
        {
        return ERROR;
        }

    uint32_t bufferSize = 1024 * 8;
    std::unique_ptr<char[]> buffer(new char[bufferSize]);

    uint32_t bytesRead = 1;
    uint32_t bytesWritten = 0;

    while (bytesRead > 0)
        {
        if (BeFileStatus::Success != source.Read(buffer.get(), &bytesRead, bufferSize) ||
            BeFileStatus::Success != target.Write(&bytesWritten, buffer.get(), bytesRead) ||
            bytesRead != bytesWritten)
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }