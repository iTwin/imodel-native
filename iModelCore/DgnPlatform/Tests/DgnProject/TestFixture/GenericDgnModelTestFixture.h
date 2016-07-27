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
    
    DgnDbPtr GetDgnDb(WCharCP seedName);

    DgnDbPtr GetDgnDb(WCharCP seedName, WCharCP newName);

public:
    GenericBaseFixture() {  }

    virtual ~GenericBaseFixture() {}

    virtual void TearDown() override;
};
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                             Umar.Hayat          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModelTestFixture : public GenericBaseFixture
{
    public: static void SetUpTestCase();
    public: static void TearDownTestCase();
public:
    DEFINE_T_SUPER(GenericBaseFixture);

    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    GenericDgnModelTestFixture() { }

    virtual ~GenericDgnModelTestFixture() {}

    bool            Is3d() const { return true; }

    DgnDbPtr GetDgnDb(){ return T_Super::GetDgnDb(s_seedFileInfo.fileName); }

    DgnDbPtr GetDgnDb(WCharCP newName){ return T_Super::GetDgnDb(s_seedFileInfo.fileName, newName); }
};
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                             Umar.Hayat          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModel2dTestFixture : public GenericBaseFixture
{
    public: static void SetUpTestCase();
    public: static void TearDownTestCase();
public:
    DEFINE_T_SUPER(GenericBaseFixture);

    static DgnDbTestUtils::SeedDbInfo s_seedFileInfo;

    GenericDgnModel2dTestFixture() { }

    virtual ~GenericDgnModel2dTestFixture() {}

    bool            Is3d() const { return false; }

    DgnDbPtr GetDgnDb(){ return T_Super::GetDgnDb(s_seedFileInfo.fileName); }

    DgnDbPtr GetDgnDb(WCharCP newName){ return T_Super::GetDgnDb(s_seedFileInfo.fileName, newName); }
};
END_DGNDB_UNIT_TESTS_NAMESPACE
