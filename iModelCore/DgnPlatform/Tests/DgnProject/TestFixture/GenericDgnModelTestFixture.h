/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/TestFixture/GenericDgnModelTestFixture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>


#define TEST_MODEL2D_NAME     "TestModel2D"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                             Umar.Hayat          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericBaseFixture : public testing::Test
{
private:
    Dgn::ScopedDgnHost m_testHost;

protected:
    DgnDbPtr m_dgnDb;

public:
    GenericBaseFixture() {  }

    virtual ~GenericBaseFixture() {}

    virtual void TearDown() override;

    DgnDbPtr GetDgnDb();
};
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                             Umar.Hayat          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModelTestFixture : public GenericBaseFixture
{
    BETEST_DECLARE_TC_SETUP
    BETEST_DECLARE_TC_TEARDOWN
public:
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    GenericDgnModelTestFixture() { }

    virtual ~GenericDgnModelTestFixture() {}

    bool            Is3d() const { return true; }

    DgnDbPtr GetDgnDb();
};
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                             Umar.Hayat          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModel2dTestFixture : public GenericBaseFixture
{
    BETEST_DECLARE_TC_SETUP
    BETEST_DECLARE_TC_TEARDOWN
public:
    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    GenericDgnModel2dTestFixture() { }

    virtual ~GenericDgnModel2dTestFixture() {}

    bool            Is3d() const { return false; }

    DgnDbPtr GetDgnDb();
};
END_DGNDB_UNIT_TESTS_NAMESPACE
