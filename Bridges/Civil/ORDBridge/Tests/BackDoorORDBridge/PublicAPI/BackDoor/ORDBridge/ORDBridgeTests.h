#pragma once

#include <Bentley/BeTest.h>
#include <ORDBridge/ORDBridgeApi.h>

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

    static bool RunTestApp(Utf8CP input, Utf8CP modelName, Utf8CP bimFileName);
    static Dgn::DgnDbPtr VerifyConvertedElements(Utf8CP bimFileName, size_t alignmentCount, size_t roadwayCount);

public:
};

typedef CiviliModelBridgesORDBridgeTestsFixture CiviliModelBridgesORDBridgeTests;

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
