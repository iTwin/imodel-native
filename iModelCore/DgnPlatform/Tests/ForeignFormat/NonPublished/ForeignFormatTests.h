/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ForeignFormat/NonPublished/ForeignFormatTests.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <DgnPlatform/ForeignFormat/DgnV8ProjectImporter.h>
//#include "GenericDgnModelTestFixture.h"
//#include <UnitTests/BackDoor/DgnProject/ElementCreateHelpers.h>
//#include <UnitTests/BackDoor/DgnProject/DgnElementHelpers.h>

#define BEGIN_DGNDB_UNIT_TESTS_NAMESPACE BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace DgnDbUnitTests {
#define END_DGNDB_UNIT_TESTS_NAMESPACE                                                                               } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_DGNDB_UNIT_TESTS_NAMESPACE using namespace BentleyApi::DgnPlatform::DgnDbUnitTests;

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

struct DgnDbTestDgnManager : TestDgnManager
{
    DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile="", FileOpenMode omode=OPENMODE_READWRITE, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel, bool forceMakeCopy=false);
    static BeFileName GetUtDatPath (CharCP callerSourceFile);
    static StatusInt FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile);
    static StatusInt GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile);
    static BeFileName GetWritableTestData (WCharCP fileName, CharCP callerSourceFile);
    static BeFileName GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy);

    static void CreateProjectFromDgn (DgnProjectPtr& project, BeFileName const& projectFileName, BeFileName const& dgnFileName, BeFileName const& refFileName = BeFileName(), bool deleteIfExists = true)
        {
        if (deleteIfExists)
            BeFileName::BeDeleteFile (projectFileName);

        CreateProjectParams params;

        DgnFileStatus result;
        project = DgnProject::CreateProject (&result, projectFileName, params);

        ASSERT_TRUE( project.IsValid());
        ASSERT_TRUE( result == DGNFILE_STATUS_Success);

        Utf8String projectFileNameUtf8 (projectFileName.GetName());
        ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName()));

        ASSERT_TRUE( BeFileName::DoesPathExist(dgnFileName) ) << L" seed file not found: " << dgnFileName.GetName();

        ForeignFormat::DgnV8::DgnV8ProjectImporter publisher (*project, ForeignFormat::DgnImportParams());
        publisher.SetSeedFile(dgnFileName);
        if (*refFileName.GetName())
            publisher.AddForeignFile (refFileName.GetName(), false);
        publisher.PerformImport();
        }

    static BeFileName GetOutputFilePath (WCharCP fn)
        {
        BeFileName projectFileName;
        BeTest::GetHost().GetOutputRoot (projectFileName);
        projectFileName.AppendToPath (fn);
        return projectFileName;
        }

    static BeFileName GetSeedFilePath (WCharCP fn)
        {
        BeFileName fullFileName;
        FindTestData (fullFileName, fn, __FILE__);
        return fullFileName;
        }
};

END_DGNDB_UNIT_TESTS_NAMESPACE

#define LOCALIZED_STR(str) str

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_DGNDB_UNIT_TESTS_NAMESPACE

