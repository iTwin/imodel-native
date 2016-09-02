#pragma once

#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <Bentley/BeTest.h>

#define BEGIN_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE  BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE namespace Tests {
#define END_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE    } END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT_UNITTESTS  using namespace BENTLEY_NAMESPACE_NAME::RoadRailAlignment::Tests;

BEGIN_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

struct RoadRailAlignmentProjectHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct RoadRailAlignmentProjectHost
{
friend struct RoadRailAlignmentTestsFixture;

private:
    RoadRailAlignmentProjectHostImpl* m_pimpl;

    void CleanOutputDirectory();

    Dgn::DgnDbPtr CreateProject(WCharCP, Dgn::DgnModelId&);
    Dgn::DgnDbPtr OpenProject(WCharCP);

public:
    RoadRailAlignmentProjectHost();
    ~RoadRailAlignmentProjectHost();

    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    BeFileName BuildProjectFileName(WCharCP);
};

//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct RoadRailAlignmentTestsFixture : ::testing::Test
{
private:
    static RoadRailAlignmentProjectHost* m_host;
    //! This variable is used internally. Do not expose.
    static Dgn::DgnDbPtr m_currentProject;

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
    static RoadRailAlignmentProjectHost& GetHost() { return *m_host; }

    //! Creates and caches a fresh "created" file to make the whole process faster
    static Dgn::DgnDbPtr CreateProject(WCharCP, Dgn::DgnModelId&, bool needsSetBriefcase = false);
    //! Uses private static variable to hold the last opened project in memory.
    static Dgn::DgnDbPtr OpenProject(WCharCP, bool needsSetBriefcase = false);
};

typedef RoadRailAlignmentTestsFixture RoadRailAlignmentTests;


END_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT_UNITTESTS
