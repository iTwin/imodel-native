#pragma once

#include <Bentley/BeTest.h>
#include <CS06Bridge/CS06BridgeApi.h>

struct CS06BridgeTestsHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct CS06BridgeTestsHost
{
	friend struct CiviliModelBridgesCS06BridgeTestsFixture;

private:
	CS06BridgeTestsHostImpl* m_pimpl;

	void CleanOutputDirectory();

public:
	CS06BridgeTestsHost();
	~CS06BridgeTestsHost();

	BeFileName GetTestAppProductDirectory();
	BeFileName GetOutputDirectory();
	BeFileName GetDgnPlatformAssetsDirectory();
	BeFileName BuildProjectFileName(WCharCP);
};

//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct CiviliModelBridgesCS06BridgeTestsFixture : ::testing::Test
{
private:
	static CS06BridgeTestsHost* m_host;

protected:
	//! Called before running all tests
	static void SetUpTestCase();
	//! Called after running all tests
	static void TearDownTestCase();

	//! Called before each test
	void SetUp() {}
	//! Called after each test
	void TearDown() {}

	static bool RunTestApp(Utf8CP input, Utf8CP bimFileName);
	static Dgn::DgnDbPtr VerifyConvertedElements(Utf8CP bimFileName, size_t alignmentCount, size_t roadwayCount);

public:
};

typedef CiviliModelBridgesCS06BridgeTestsFixture CiviliModelBridgesCS06BridgeTests;

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
