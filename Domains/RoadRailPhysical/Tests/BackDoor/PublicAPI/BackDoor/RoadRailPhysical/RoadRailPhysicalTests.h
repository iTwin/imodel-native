/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <RoadRailPhysical/RoadRailPhysicalApi.h>
#include <Bentley/BeTest.h>

#define BEGIN_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE  BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE namespace Tests {
#define END_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE    } END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL_UNITTESTS  using namespace BENTLEY_NAMESPACE_NAME::RoadRailPhysical::Tests;

BEGIN_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

struct RoadRailPhysicalProjectHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct RoadRailPhysicalProjectHost
{
friend struct RoadRailPhysicalTestsFixture;

private:
    RoadRailPhysicalProjectHostImpl* m_pimpl;

    void CleanOutputDirectory();

    Dgn::DgnDbPtr CreateProject(WCharCP);
    Dgn::DgnDbPtr OpenProject(WCharCP);

public:
    RoadRailPhysicalProjectHost();
    ~RoadRailPhysicalProjectHost();

    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    BeFileName BuildProjectFileName(WCharCP);
};

//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct RoadRailPhysicalTestsFixture : ::testing::Test
{
private:
    static RoadRailPhysicalProjectHost* m_host;
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
    static RoadRailPhysicalProjectHost& GetHost() { return *m_host; }

    //! Creates and caches a fresh "created" file to make the whole process faster
    static Dgn::DgnDbPtr CreateProject(WCharCP, bool needsSetBriefcase = false);
    //! Uses private static variable to hold the last opened project in memory.
    static Dgn::DgnDbPtr OpenProject(WCharCP, bool needsSetBriefcase = false);
};

typedef RoadRailPhysicalTestsFixture RoadRailPhysicalTests;


END_BENTLEY_ROADRAILPHYSICAL_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL
USING_NAMESPACE_BENTLEY_RAILPHYSICAL
USING_NAMESPACE_BENTLEY_ROADPHYSICAL
USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL_UNITTESTS
