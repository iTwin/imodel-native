#pragma once
#include <Bentley\BeTest.h>
#include <DgnView\DgnViewLib.h>

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Arturas.Mizaras                 11/17
+---------------+---------------+---------------+---------------+---------------+------*/
class ProfilesTest : public testing::Test
    {
    private:
        Dgn::DgnDbPtr m_dbPtr;
        Dgn::DgnModelPtr m_modelPtr;

    public:
        ProfilesTest();
        virtual ~ProfilesTest();

        Dgn::DgnDb& GetDb();
        Dgn::DgnModel& GetModel();
    };
