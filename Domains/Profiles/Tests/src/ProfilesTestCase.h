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

    protected:
        ProfilesTestCase();

    public:
        virtual ~ProfilesTestCase();

        Dgn::DgnDb& GetDb();
        Dgn::DgnModel& GetModel();
    };
