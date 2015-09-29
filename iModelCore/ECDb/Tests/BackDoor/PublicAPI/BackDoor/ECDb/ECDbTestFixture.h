/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbTestProject.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
    private:
        std::unique_ptr<ECDbTestProject> m_testProject;
        static bmap<std::pair<WCharCP, int>, Utf8String> s_seedDbs;

        virtual ECDbTestProject& _GetTestProject () const;

    protected:
        static std::unique_ptr<ECDbTestProject> CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName);
        static std::unique_ptr<ECDbTestProject> CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams, int perClassRowCount);
        void SetTestProject (std::unique_ptr<ECDbTestProject> testProject);    
        ECDbTestProject& GetTestProject () const;

    public:
        ECDbTestFixture ();
        virtual ~ECDbTestFixture () {};
        virtual void SetUp () override {}
        virtual void TearDown () override {}

    };

END_ECDBUNITTESTS_NAMESPACE
