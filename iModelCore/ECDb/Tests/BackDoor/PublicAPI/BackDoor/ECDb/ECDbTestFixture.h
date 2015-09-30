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

        static bool s_isInitialized;

        virtual ECDbTestProject& _GetTestProject() const { return *m_testProject; }

    protected:
        ECDbTestProject& GetTestProject () const;
 
        ECDbTestProject& SetupTestProject(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);

        static std::unique_ptr<ECDbTestProject> CreateTestProject(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);

    public:
        ECDbTestFixture ();
        virtual ~ECDbTestFixture () {};
        virtual void SetUp () override {}
        virtual void TearDown () override {}

        //! Initializes the test environment by setting up the schema read context and search dirs etc.
        //! Gets implicitly called when calling CreateTestProject, too. Tests that don't use
        //! that method can call this method statically.
        static void Initialize();
     };

END_ECDBUNITTESTS_NAMESPACE
