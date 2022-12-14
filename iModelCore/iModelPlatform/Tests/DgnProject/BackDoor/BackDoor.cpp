/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <Bentley/Logging.h>

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

namespace BackDoor
{
    namespace DgnModel
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod
        +-------+---------------+---------------+---------------+---------------+------*/
        Dgn::DgnModelId GetModelId (DgnModelR model)
            {
            return model.GetModelId();
            }

    };

    namespace PolyfaceQueryP
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod
        +-------+---------------+---------------+---------------+---------------+------*/
        double SumTetrahedralVolumes (BentleyApi::PolyfaceQueryP p, DPoint3dCR origin)
            {
            return p->SumTetrahedralVolumes (origin);
            }
    };

}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile)
    {
    return TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile)) != SUCCESS)
        return ERROR;
    //Copy this to out path and then set that path
    BeTest::GetHost().GetOutputRoot (outFullFileName);
    outFullFileName.AppendToPath (outName);
    BeFileName::CreateNewDirectory (BeFileName::GetDirectoryName(outFullFileName).c_str());
    if (BeFileName::BeCopyFile (fullFileName, outFullFileName) != BeFileNameStatus::Success)
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetUtDatPath (CharCP callerSourceFile)
    {
    if (NULL == callerSourceFile || !*callerSourceFile)
        return BeFileName (L"DgnDb");   // just look in the docs root

    // start looking in the sub-directory that is specific to this UT. (Then look in each parent directory in turn.)
    BeFileName path (L"DgnDb");
    path.AppendToPath (L"Published");           // *** NB: This must agree with the directory structure of the DgnPlatformTest/DgnDbUnitTests repository
    path.AppendToPath (L"DgnHandlers");
    path.AppendToPath (BeFileName (BeFileName::Basename, BeFileName(callerSourceFile)).GetName());
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy)
    {
    BeFileName fullFileName;
    StatusInt status;
    if (forceMakeCopy)
        status = GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile);
    else
        status = TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));

    if (SUCCESS != status)
        {
        NativeLogging::CategoryLogger("BeTest").errorv (L"failed to find project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetWritableTestData (WCharCP fileName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile) != SUCCESS)
        {
        NativeLogging::CategoryLogger("BeTest").errorv (L"failed to find or copy project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find or copy input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbTestDgnManager::DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile, BeSQLite::Db::OpenMode omode, bool needBriefcase, DgnInitializeMode imode, bool forceMakeCopy)
    : TestDgnManager(
        (BeSQLite::Db::OpenMode::Readonly==omode) ? GetReadOnlyTestData(dgnfilename,callerSourceFile,forceMakeCopy) : GetWritableTestData(dgnfilename,callerSourceFile),
        omode,
        needBriefcase,
        imode
        )
    {}

END_DGNDB_UNIT_TESTS_NAMESPACE
