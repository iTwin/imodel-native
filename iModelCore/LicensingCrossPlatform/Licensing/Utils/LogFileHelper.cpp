/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Licensing/Utils/LogFileHelper.h>
#if defined (BENTLEY_WIN32)
#include <filesystem>
#else
#include <Bentley/BeDirectoryIterator.h>
#endif

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<WString> LogFileHelper::GetLogFiles(Utf8StringCR logFilesDir)
    {
    bvector<WString> matchFileList;
#if defined (BENTLEY_WIN32)
    for (std::experimental::filesystem::directory_iterator dir_iter(logFilesDir.c_str()), end; dir_iter != end; ++dir_iter)
        {
        WString pathStr = dir_iter->path().c_str();
        if (std::experimental::filesystem::is_regular_file(dir_iter->path()))
            {
            WString curFile = dir_iter->path().filename().c_str();
            WString curFileFullPath = dir_iter->path().c_str();
            if (curFileFullPath.Contains(L".csv"))
                {
                matchFileList.push_back(curFileFullPath);
                }
            }
        }
#else
    bvector<BeFileName> matchBeFileNameList;
    BeDirectoryIterator::WalkDirsAndMatch (matchBeFileNameList, BeFileName(logFilesDir), L"*.csv", /*recursive*/false);
    for (BeFileNameCR matchBeFileName: matchBeFileNameList)
        {
        WString matchFileName = matchBeFileName.c_str();
        matchFileList.push_back(matchFileName);
        }
#endif
    return matchFileList;
    }
