/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <WebServices/Cache/Persistence/FileManager.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct StubFileManager : FileManager
    {
    // TODO: this would be much nicer with BeTest using GMOCK on iOS/Android
    BeFileName          overrideDeleteFileArg;
    BeFileNameStatus    overrideDeleteFileRet = BeFileNameStatus::Success;
    virtual BeFileNameStatus DeleteFile(BeFileNameCR path) override
        {
        if (BeFileNameStatus::Success != overrideDeleteFileRet)
            if (overrideDeleteFileArg.empty() || overrideDeleteFileArg == path)
                return overrideDeleteFileRet;

        return FileManager::DeleteFile(path);
        }

    BeFileName          overrideMoveFileArg;
    BeFileNameStatus    overrideMoveFileRet = BeFileNameStatus::Success;
    virtual BeFileNameStatus MoveFile(BeFileNameCR src, BeFileNameCR trg) override
        {
        if (BeFileNameStatus::Success != overrideMoveFileRet)
            if (overrideMoveFileArg.empty() || overrideMoveFileArg == src || overrideMoveFileArg == trg)
                return overrideMoveFileRet;

        return FileManager::MoveFile(src, trg);
        }

    BeFileName          overrideDeleteDirectoryFileArg;
    BeFileNameStatus    overrideDeleteDirectoryFileRet = BeFileNameStatus::Success;
    virtual BeFileNameStatus DeleteDirectory(BeFileNameCR path) override
        {
        if (BeFileNameStatus::Success != overrideDeleteDirectoryFileRet)
            if (overrideDeleteDirectoryFileArg.empty() || overrideDeleteDirectoryFileArg == path)
                return overrideDeleteDirectoryFileRet;

        return FileManager::DeleteDirectory(path);
        }
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
