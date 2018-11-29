/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfilesTestCase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley\BeTest.h>
#include <DgnView\DgnViewLib.h>

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
class ProfilesTestCase : public testing::Test
    {
private:
    Dgn::DgnDbPtr m_dbPtr;
    Dgn::DefinitionModelPtr m_definitionModelPtr;
    Dgn::PhysicalModelPtr m_physicalModelPtr;
    Dgn::DgnCategoryId m_categoryId;

protected:
    ProfilesTestCase();
    virtual ~ProfilesTestCase();

    Dgn::DgnDb& GetDb();
    Dgn::DgnModel& GetModel();
    Dgn::PhysicalModel& GetPhysicalModel();
    Dgn::DgnCategoryId GetCategoryId();
    };
