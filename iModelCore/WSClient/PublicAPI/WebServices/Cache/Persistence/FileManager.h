/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef std::shared_ptr<struct IFileManager> IFileManagerPtr;

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct IFileManager
    {
    virtual ~IFileManager() {}

    virtual BeFileNameStatus CopyFile(BeFileNameCR source, BeFileNameCR target) = 0;
    virtual BeFileNameStatus MoveFile(BeFileNameCR source, BeFileNameCR target) = 0;
    virtual BeFileNameStatus DeleteFile(BeFileNameCR path) = 0;
    virtual BeFileNameStatus CreateDirectory(BeFileNameCR path) = 0;
    virtual BeFileNameStatus DeleteDirectory(BeFileNameCR path) = 0;
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileManager : IFileManager
    {
    virtual BeFileNameStatus CopyFile(BeFileNameCR source, BeFileNameCR target) override { return BeFileName::BeCopyFile(source, target); }
    virtual BeFileNameStatus MoveFile(BeFileNameCR source, BeFileNameCR target) override { return BeFileName::BeMoveFile(source, target); }
    virtual BeFileNameStatus DeleteFile(BeFileNameCR path) override { return BeFileName::BeDeleteFile(path); }
    virtual BeFileNameStatus CreateDirectory(BeFileNameCR path) override { return BeFileName::CreateNewDirectory(path); }
    virtual BeFileNameStatus DeleteDirectory(BeFileNameCR path) override { return BeFileName::EmptyAndRemoveDirectory(path); }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
