/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

// BackDoor is built with the non-published the DgnPlatform APIs. BackDoor then presents its API to the published unit test code, which is build with the published API. 
// The purpose of BackDoor is to provide the published unit tests with access to selected non-published functions so that they can do necessary set-up or result-checking 
// as part of the testing mechanism, not as part of the API or functionality that is tested.
// NB: Even though it uses the Bentley-internal API, BackDoor must be portable to other platforms.

#include <Bentley/Bentley.h>
//#include <Bentley/CatchNonPortable.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <Bentley/BeTest.h>

#define BEGIN_DGNDB_UNIT_TESTS_NAMESPACE BEGIN_BENTLEY_DGN_NAMESPACE namespace DgnDbUnitTests {
#define END_DGNDB_UNIT_TESTS_NAMESPACE   } END_BENTLEY_DGN_NAMESPACE
#define USING_DGNDB_UNIT_TESTS_NAMESPACE using namespace BentleyApi::Dgn::DgnDbUnitTests;

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

struct DgnDbTestDgnManager : TestDgnManager
{
    DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile="", BeSQLite::Db::OpenMode mode=BeSQLite::Db::OpenMode::ReadWrite, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel, bool forceMakeCopy=false);
    static BeFileName GetUtDatPath (CharCP callerSourceFile);
    static StatusInt FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile);
    static StatusInt GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile);
    static BeFileName GetWritableTestData (WCharCP fileName, CharCP callerSourceFile);
    static BeFileName GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy);

#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
    static void CreateProjectFromDgn (DgnDbPtr& project, BeFileName const& projectFileName, BeFileName const& dgnFileName, BeFileName const& refFileName = BeFileName(), bool deleteIfExists = true);
#endif

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

namespace BackDoor
{
/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DirectionParser 
    {
        void SetTrueNorth (Dgn::DirectionParser& parser, double const& trueNorth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      11/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DgnModel
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnModelType GetModelType (DgnModelCR model);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      08/10
        +-------+---------------+---------------+---------------+---------------+------*/
        void SetReadOnly (Dgn::DgnModel& model, bool isReadOnly);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnElementP FindByElementId (DgnModelR model, Dgn::DgnElementId id);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        Dgn::DgnModelId GetModelId (DgnModelR model);

    };

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 JoshSchifter    11/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace NonVisibleViewport
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnViewportP Create (ViewControllerR viewInfo);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void Delete (DgnViewportP viewport);

    }; // NonVisibleViewport
    
    /*-------------------------------------------------------------------------**//**
    * @bsinamespace                                                 KevinNyman      07/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace PolyfaceQueryP
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double SumTetrahedralVolumes (BentleyApi::PolyfaceQueryP p, DPoint3dCR origin);
    };

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace RealityData
    {
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                07/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void RunOnAnotherThread(std::function<void()> const&);
    }; // RealityData
}

END_DGNDB_UNIT_TESTS_NAMESPACE
