/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Cache/Persistence/StubFileManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
