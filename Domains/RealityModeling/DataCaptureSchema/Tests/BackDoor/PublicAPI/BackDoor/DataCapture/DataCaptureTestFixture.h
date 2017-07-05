/*--------------------------------------------------------------------------------------+
|
|  $Source: DataCaptureSchema/Tests/BackDoor/PublicAPI/BackDoor/DataCapture/DataCaptureTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "DataCaptureTests.h"

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE


//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct DataCaptureTestsFixture : ::testing::Test
    {
    private:
        static DataCaptureProjectHost* m_host;
        //! This variable is used internally. Do not expose.
        static Dgn::DgnDbPtr s_currentProject;

    protected:
        //! Called before running all tests
        static void SetUpTestCase();
        //! Called after running all tests
        static void TearDownTestCase();

        //! Called before each test
        void SetUp() {}
        //! Called after each test
        void TearDown() {}

        //Create Sample cameraDevice-photos project
        void CreateSampleShotProjectWithCameraDevice(Dgn::DgnDbR dgndb, Utf8CP cameraDeviceLable);

        //Create Sample cameraDevice-drone project
        void CreateSampleDroneProjectWithCameraDevice(Dgn::DgnDbR dgndb, Utf8CP cameraDeviceLable)

    public:
        static DataCaptureProjectHost& GetHost() { return *m_host; }

        static Dgn::DgnModelId QueryFirstSpatialModelId(Dgn::DgnDbR db);
        static Dgn::DgnModelId QueryFirstDefinitionModelId(DgnDbR db);


        //! Creates and caches a fresh "created" file to make the whole process faster
        static Dgn::DgnDbPtr CreateProject(WCharCP, bool needsSetBriefcase = false);
        //! Uses private static variable to hold the last opened project in memory.
        static Dgn::DgnDbPtr OpenProject(WCharCP, bool needsSetBriefcase = false);
        //! Close the last opened project kept by static variable.
        static void          CloseProject();
    };

typedef DataCaptureTestsFixture DataCaptureTests;


END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE
