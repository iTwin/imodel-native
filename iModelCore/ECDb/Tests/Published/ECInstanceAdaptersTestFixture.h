/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECInstanceAdaptersTestFixture.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
struct ECInstanceAdaptersTestFixture : public ::testing::Test
    {
    private:
        std::unique_ptr<ECDbTestProject> m_testProject;
        virtual ECDbTestProject& _GetTestProject () const;

    protected:
        static std::unique_ptr<ECDbTestProject> CreateTestProject (Utf8CP ecdbFileName, WCharCP schemaECXmlFileName);
        void SetTestProject (std::unique_ptr<ECDbTestProject> testProject);    
        ECDbTestProject& GetTestProject () const;

    public:
        ECInstanceAdaptersTestFixture ();
        virtual ~ECInstanceAdaptersTestFixture () {};
        virtual void SetUp () override {}
        virtual void TearDown () override {}

    };

END_ECDBUNITTESTS_NAMESPACE
