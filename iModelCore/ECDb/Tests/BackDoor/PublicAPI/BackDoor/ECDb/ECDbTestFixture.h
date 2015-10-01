/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
private:
    mutable ECDb m_ecdb;
    static bmap<std::pair<WCharCP, int>, Utf8String> s_seedECDbs;

    static bool s_isInitialized;

    virtual ECDb& _GetECDb() const { return m_ecdb; }

protected:
    ECDb& GetECDb() const { return _GetECDb(); }
 
    void SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);

    static DbResult CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);

public:
    ECDbTestFixture () {}
    virtual ~ECDbTestFixture () {};
    virtual void SetUp () override {}
    virtual void TearDown () override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling CreateTestProject, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();
     };

END_ECDBUNITTESTS_NAMESPACE
