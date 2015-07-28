/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/TempFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/TempFile.h>

#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>
#include <WebServices/Cache/Util/FileUtil.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TempFile::TempFile(BeFileNameCR tempDir, Utf8StringCR fileName)
    {
    BeFileName directory;
    directory
        .AppendToPath(tempDir)
        .AppendToPath(BeFileName(BeGuid().ToString()))
        .AppendSeparator();

    m_filePath
        .AppendToPath(directory)
        .AppendToPath(BeFileName(FileUtil::TruncateFileName(fileName)));

    if (SUCCESS != FileUtil::TruncateFilePath(m_filePath))
        {
        m_filePath.clear();
        }

    auto status = BeFileName::CreateNewDirectory(directory);
    BeAssert(BeFileNameStatus::Success == status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TempFile::~TempFile()
    {
    Cleanup();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void TempFile::Cleanup()
    {
    WString directory = BeFileName::GetDirectoryName(BeFileName(m_filePath));

    BeFileNameStatus status = BeFileName::EmptyAndRemoveDirectory(directory.c_str());

    if (BeFileNameStatus::Success != status &&
        BeFileNameStatus::FileNotFound != status)
        {
        BeAssert(false);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR TempFile::GetPath() const
    {
    return m_filePath;
    }
