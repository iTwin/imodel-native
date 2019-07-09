/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <Bentley/BeTest.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <Formatting/FormattingApi.h>
#include <CivilBaseGeometry/CivilBaseGeometryApi.h>

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

    Dgn::DgnDbPtr CreateProject(WCharCP);
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

    Dgn::CategorySelectorPtr CreateSpatialCategorySelector(Dgn::DefinitionModelR model);
    Dgn::CategorySelectorPtr CreateDrawingCategorySelector(Dgn::DefinitionModelR model);
    Dgn::ModelSelectorPtr CreateModelSelector(Dgn::DefinitionModelR definitionModel, Utf8StringCR name);
    Dgn::DisplayStyle2dPtr CreateDisplayStyle2d(Dgn::DefinitionModelR model);
    Dgn::DisplayStyle3dPtr CreateDisplayStyle3d(Dgn::DefinitionModelR model);

    BentleyStatus Create2dView(Dgn::DefinitionModelR model, Utf8CP viewName,
        Dgn::CategorySelectorR categorySelector, Dgn::DgnModelId modelToDisplay, Dgn::DisplayStyle2dR displayStyle);
    BentleyStatus Create3dView(Dgn::DefinitionModelR model, Utf8CP viewName,
        Dgn::CategorySelectorR categorySelector, Dgn::ModelSelectorR modelSelector, Dgn::DisplayStyle3dR displayStyle);

public:
    static RoadRailAlignmentProjectHost& GetHost() { return *m_host; }

    static Dgn::DgnModelId QueryFirstModelIdOfType(Dgn::DgnDbR db, Dgn::DgnClassId classId);

    //! Creates and caches a fresh "created" file to make the whole process faster
    static Dgn::DgnDbPtr CreateProject(WCharCP, bool needsSetBriefcase = false);
    //! Uses private static variable to hold the last opened project in memory.
    static Dgn::DgnDbPtr OpenProject(WCharCP, bool needsSetBriefcase = false);
    static Dgn::PhysicalModelPtr SetUpPhysicalPartition(Dgn::SubjectCR subject);
};

typedef RoadRailAlignmentTestsFixture RoadRailAlignmentTests;

//---------------------------------------------------------------------------------------
//! Toleranced asserts
//---------------------------------------------------------------------------------------
#define EXPECT_EQ_DPOINT3D(expected, actual) EXPECT_TRUE(expected.AlmostEqual(actual));
#define EXPECT_VALID_CURVE(curvePtr) EXPECT_TRUE(curvePtr.IsValid() && !curvePtr->empty());
#define EXPECT_EQ_DOUBLE(expected, actual) EXPECT_TRUE(DoubleOps::AlmostEqual(expected, actual));

END_BENTLEY_ROADRAILALIGNMENT_UNITTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_BENTLEY_FORMATTING
USING_NAMESPACE_BENTLEY_UNITS
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT_UNITTESTS
USING_NAMESPACE_BENTLEY_CIVILGEOMETRY
