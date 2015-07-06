/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/FileMoniker.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnCore/FileMoniker.h>

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
FileMoniker::FileMoniker (Utf8StringCR fullPath, Utf8StringCR basePath)
    {
    m_fullPath = fullPath;
    m_basePath = basePath;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
FileMonikerPtr FileMoniker::Create (Utf8StringCR fullPath, Utf8StringCR basePath)
    {
    return new FileMoniker(fullPath, basePath);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
BentleyStatus FileMoniker::ResolveFileName (Utf8StringR resolvedName, Utf8StringCR basePath) const
    {
    BeFileName fullPath(m_fullPath);
    BeFileName basePathWhenCreated(m_basePath);
    WString relativePath;

    // Find relative path according to the creation paths.
    // E.g. if fullPath == "d:\dir1\dir2\file.jpg" and basePathWhenCreated == "d:\dir1\"
    //      then relativePath will be equal to "dir2\file.jpg"
    BeFileName::FindRelativePath(relativePath, fullPath.c_str(), basePathWhenCreated.c_str());

    // Find full path relatively to current base path. Current base path is a directory and may contain a file name (probably the name of the dgndb).
    // E.g. if relativePath == "dir2\file.jpg" and currentBasePath == "d:\dir5\dir6\myDgnDb.dgndb"
    //      then relativePath will be equal to "d:\dir5\dir6\dir2\file.jpg"
    WString resolvedNameW;
    BeFileName currentBasePath(basePath);
    BentleyStatus status = BeFileName::ResolveRelativePath(resolvedNameW, relativePath.c_str(), currentBasePath.c_str());
    if (status == SUCCESS)
        {
        Utf8String resolvedNameUtf8(resolvedNameW);
        resolvedName = resolvedNameUtf8;
        }

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void FileMoniker::ToJson (JsonValueR outValue) const
    {
    outValue["fullPath"] = m_fullPath.c_str();
    outValue["basePath"] = m_basePath.c_str();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void FileMoniker::FromJson (JsonValueCR inValue)
    {
    m_fullPath = inValue["fullPath"].asString();
    m_basePath = inValue["basePath"].asString();
    }

