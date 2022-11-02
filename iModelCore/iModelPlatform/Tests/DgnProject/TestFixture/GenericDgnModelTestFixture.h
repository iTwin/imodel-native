/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 

#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "DgnPlatformSeedManager.h"

#define TEST_MODEL2D_NAME     "TestModel2D"

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass
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

    void TearDown() override;
};
/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModelTestFixture : public GenericBaseFixture
{
    DEFINE_T_SUPER(GenericBaseFixture);

    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

    static void SetUpTestCase();
    static void TearDownTestCase();

    static bool Is3d() {return true;}

    GenericDgnModelTestFixture() { }
    virtual ~GenericDgnModelTestFixture() {}

    DgnDbPtr GetDgnDb(){ return T_Super::GetDgnDb(s_seedFileInfo.fileName); }
    DgnDbPtr GetDgnDb(WCharCP newName){ return T_Super::GetDgnDb(s_seedFileInfo.fileName, newName); }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericDgnModel2dTestFixture : public GenericBaseFixture
{
    DEFINE_T_SUPER(GenericBaseFixture);

    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;
    static DgnModelId s_drawingModelId;

    static void SetUpTestCase();
    static void TearDownTestCase();

    static bool Is3d() {return false;}
    static DgnModelId GetDrawingModelId() {return s_drawingModelId;} //!< Return the DgnModelId of the DrawingModel created during SetUpTestCase

    GenericDgnModel2dTestFixture() {}
    virtual ~GenericDgnModel2dTestFixture() {}

    DgnDbPtr GetDgnDb() {return T_Super::GetDgnDb(s_seedFileInfo.fileName);}
    DgnDbPtr GetDgnDb(WCharCP newName) {return T_Super::GetDgnDb(s_seedFileInfo.fileName, newName);}
};

END_DGNDB_UNIT_TESTS_NAMESPACE
