/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Utils/LogFileHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/LogFileHelper.h>
#if defined (BENTLEY_WIN32)
#include <filesystem>
#endif

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<WString> LogFileHelper::GetLogFiles(Utf8StringCR logFilesDir)
    {
    bvector<WString> matchFileList;
#if defined (BENTLEY_WIN32)
    for (std::tr2::sys::directory_iterator dir_iter(Utf8String(logFilesDir).c_str()), end; dir_iter != end; ++dir_iter)
        {
        WString pathStr = dir_iter->path().c_str();
        if (std::tr2::sys::is_regular_file(dir_iter->path()))
            {
            WString curFile = dir_iter->path().filename().c_str();
            WString curFileFullPath = dir_iter->path().c_str();
            if (curFileFullPath.Contains(L".csv"))
                {
                matchFileList.push_back(curFileFullPath);
                }
            }
        }
#endif
    return matchFileList;
    }
