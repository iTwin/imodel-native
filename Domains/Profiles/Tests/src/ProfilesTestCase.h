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
        Dgn::DgnModelPtr m_modelPtr;
        Dgn::PhysicalModelPtr m_physicalModelPtr;
    protected:
        ProfilesTestCase();

    public:
        virtual ~ProfilesTestCase();

        Dgn::DgnDb& GetDb();
        Dgn::DgnModel& GetModel();
        Dgn::PhysicalModelR GetPhysicalModel() {return *m_physicalModelPtr;}
    };
