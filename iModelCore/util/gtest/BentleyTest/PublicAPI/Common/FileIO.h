/*__BENTLEY_INTERNAL_ONLY__*/
/*--------------------------------------------------------------------------------------+
|
|  $Source: BentleyTest/PublicAPI/Common/FileIO.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#define NOMINMAX
#include <windows.h>

static const int REDEF_MAX_PATH = 1024;
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirExists (char const* dir)
    {
    return GetFileAttributesA(dir) != INVALID_FILE_ATTRIBUTES;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirExists (wchar_t const* dir)
    {
    return GetFileAttributesW(dir) != INVALID_FILE_ATTRIBUTES;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreDots(wchar_t const* filename)
    {
    int count = 0;
    for (int i = 0; i < REDEF_MAX_PATH; ++i)
        {
        if (0 == filename[i])
            {
            count = i;
            break;
            }
        }
    if (count > 1)
        {
        if (filename[count-1] == '.')
            {
            if (filename[count-2] == '.')
                return true;
            return true;
            }
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DeleteDirectory(bool& hasFilesLeft, bool& hasDirsLeft, wchar_t const* sPath, bool isRemovingFiles)
    {
    HANDLE hFind;    // file handle
    WIN32_FIND_DATAW findFileData;

    wchar_t dirPath[REDEF_MAX_PATH];
    wchar_t fileName[REDEF_MAX_PATH];

    wcscpy (dirPath,sPath);
    wcscat (dirPath,L"\\*");    // searching all files
    wcscpy(fileName,sPath);
    wcscat (fileName,L"\\");

    // find the first file
    hFind = FindFirstFileW(dirPath,&findFileData);
    if(hFind == INVALID_HANDLE_VALUE)
        return false;
    wcscpy (dirPath,fileName);

    bool bSearch = true;
    while(bSearch)
        {    // until we find an entry
        if(FindNextFileW(hFind,&findFileData))
            {
            if(AreDots(findFileData.cFileName))  // skip '.', and '..'
                continue;
            wcscat (fileName,findFileData.cFileName);
            if((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                // we have found a directory, recurse
                bool hhasFilesLeft = false;
                bool hhasDirsLeft = false;
                if(!DeleteDirectory(hhasFilesLeft, hhasDirsLeft, fileName, isRemovingFiles))
                    {
                    hasDirsLeft = true;
                    }
                // remove the empty directory
                if (!hhasFilesLeft)
                    {
                    RemoveDirectoryW(fileName);
                    }
                else
                    hasFilesLeft = true;
                wcscpy (fileName,dirPath);
                }
            else
                {
                if (isRemovingFiles)
                    {
                    if(findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                        ::SetFileAttributesW(fileName, FILE_ATTRIBUTE_NORMAL); // turn off read-only attribute
                    if(!DeleteFileW(fileName))
                        {    // delete the file
                        hasFilesLeft = true;
                        }
                    }
                else
                    {
                    hasFilesLeft = true;
                    }
                wcscpy (fileName,dirPath);
                }
            } // FindNextFile
        else
            {
            // no more files there
            if(GetLastError() == ERROR_NO_MORE_FILES)
                bSearch = false;
            else
                {
                // some error occurred; close the handle and return false
                FindClose(hFind);
                return false;
                }
            } // else
        } // while
    FindClose(hFind);                  // close the file handle

    if (!hasFilesLeft)
        return (TRUE == RemoveDirectoryW(sPath)) ? true : false;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DeleteDirectoryFiles (bool& hasFilesLeft, bool& hasDirsLeft, wchar_t const* sPath)
    {
    return DeleteDirectory (hasFilesLeft, hasDirsLeft, sPath, true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool DeleteEmptyDirectories (bool& hasFilesLeft, bool& hasDirsLeft, wchar_t const* sPath)
    {
    return DeleteDirectory (hasFilesLeft, hasDirsLeft, sPath, false);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      07/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateDirectoryRecursive (wchar_t const * path, bool failIfExists)
    {
    if (DirExists (path))
        return true;
    std::wstring tempPath = path;
    std::wstring incPath = L"";
    size_t i, sz = tempPath.size();
    for (i = 0; i < sz; ++i)
        {
        if (tempPath[i] == L'/')
            tempPath[i] = L'\\';
        }

    // Some shells like TCC won't allow you to mkdir with a full path so do it piecewise.
    while (tempPath.size() > 0)
        {
        std::size_t pos = tempPath.find (L"\\");
        std::wstring part = tempPath.substr (0, pos);
        tempPath = tempPath.substr (pos+1, tempPath.size());
        incPath += part;
        incPath += L"\\";
        if (!DirExists (incPath.c_str()))
            {
            CreateDirectoryW (incPath.c_str(), NULL);
            }
        else
            if (failIfExists)
                return false;
        }
    return (DirExists (path));
    }
