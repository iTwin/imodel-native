/*--------------------------------------------------------------------------------------+
|
|     $Source: DataCaptureSchema/Tests/BackDoor/PublicAPI/BackDoor/DataCapture/DataCaptureTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DataCapturev/DataCaptureSchemaApi.h>
#include <Bentley/BeTest.h>

#define BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE  BEGIN_BENTLEY_DATACAPTURE_NAMESPACE namespace Tests {
#define END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE    } END_BENTLEY_DATACAPTURE_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DATACAPTURE_UNITTESTS  using namespace BENTLEY_NAMESPACE_NAME::DataCapture::Tests;

BEGIN_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

struct DataCaptureProjectHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct DataCaptureProjectHost
{
friend struct DataCaptureTestsFixture;

private:
    DataCaptureProjectHostImpl* m_pimpl;

    void CleanOutputDirectory();

    Dgn::DgnDbPtr CreateProject(WCharCP);
    Dgn::DgnDbPtr OpenProject(WCharCP);

public:
    DataCaptureProjectHost();
    ~DataCaptureProjectHost();

    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    BeFileName BuildProjectFileName(WCharCP);
};

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

public:
    static DataCaptureProjectHost& GetHost() { return *m_host; }

    static Dgn::DgnModelId QueryFirstPhysicalModelId(Dgn::DgnDbR db);

    //! Creates and caches a fresh "created" file to make the whole process faster
    static Dgn::DgnDbPtr CreateProject(WCharCP, bool needsSetBriefcase = false);
    //! Uses private static variable to hold the last opened project in memory.
    static Dgn::DgnDbPtr OpenProject(WCharCP, bool needsSetBriefcase = false);
};

typedef DataCaptureTestsFixture DataCaptureTests;


END_BENTLEY_DATACAPTURE_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_DATACAPTURE
USING_NAMESPACE_BENTLEY_DATACAPTURE_UNITTESTS
