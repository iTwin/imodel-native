#pragma once

#include <Bentley/BeTest.h>
#include <ORDBridge/ORDBridgeApi.h>
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>
#include <ORDBridge/PublishORDToBimDLL.h>

struct ORDBridgeTestsHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct ORDBridgeTestsHost
{
friend struct CiviliModelBridgesORDBridgeTestsFixture;

private:
    ORDBridgeTestsHostImpl* m_pimpl;

    void CleanOutputDirectory();

public:
    ORDBridgeTestsHost();
    ~ORDBridgeTestsHost();

    BeFileName GetTestAppProductDirectory();
    BeFileName GetOutputDirectory();
    BeFileName GetDgnPlatformAssetsDirectory();
    WString* GetInputFileArgument(BeFileName inputPath, WCharCP input);
    WString* GetOutputFileArgument(BeFileName outputPath, WCharCP bimFileName);
    BeFileName BuildProjectFileName(WCharCP);
};

//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct CiviliModelBridgesORDBridgeTestsFixture : ::testing::Test
{
private:
    static ORDBridgeTestsHost* m_host;

protected:
    //! Called before running all tests
    static void SetUpTestCase();
    //! Called after running all tests
    static void TearDownTestCase();

    //! Called before each test
    void SetUp() {}
    //! Called after each test
    void TearDown() {}

    static bool CopyTestFile(Utf8CP source, Utf8CP target);
    static bool RunTestApp(WCharCP input, WCharCP bimFileName, bool updateMode);
    static Dgn::DgnDbPtr VerifyConvertedElements(Utf8CP bimFileName, size_t alignmentCount, size_t corridorCount);

public:
};

typedef CiviliModelBridgesORDBridgeTestsFixture CiviliModelBridgesORDBridgeTests;

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
