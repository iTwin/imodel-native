#pragma once
#include <Bentley\BeTest.h>
#include <DgnView\DgnViewLib.h>

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras                 11/17
+---------------+---------------+---------------+---------------+---------------+------*/
class ProfilesTestCase : public testing::Test
    {
    private:
        Dgn::DgnDbPtr m_dbPtr;
        Dgn::DgnModelPtr m_modelPtr;

    public:
        ProfilesTestCase();
        virtual ~ProfilesTestCase();

        Dgn::DgnDb& GetDb();
        Dgn::DgnModel& GetModel();
    };
