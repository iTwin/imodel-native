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
public:
    //---------------------------------------------------------------------------------------
    // @bsiclass                                   Krischan.Eberle                  07/15
    //+---------------+---------------+---------------+---------------+---------------+------
    struct SchemaItem
        {
        Utf8String m_name;
        std::vector<Utf8String> m_schemaXmlList;
        bool m_expectedToSucceed;
        Utf8String m_assertMessage;

        SchemaItem(Utf8CP name, Utf8CP schemaXml) : m_name(name), m_schemaXmlList({schemaXml}), m_expectedToSucceed(true) {}
        explicit SchemaItem(Utf8CP schemaXml) : SchemaItem("", schemaXml) {}
        SchemaItem(std::vector<Utf8String> const& schemaXmlList, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXmlList(schemaXmlList), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        SchemaItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXmlList({schemaXml}), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        SchemaItem(Utf8CP schemaXml, bool expectedToSucceeed) : m_schemaXmlList({Utf8String(schemaXml)}), m_expectedToSucceed(expectedToSucceeed) {}
        };

private:
    mutable ECDb m_ecdb;
    static bmap<bpair<WString, int>, Utf8String> s_seedECDbs;

    static bool s_isInitialized;

protected:
    ECDb& GetECDb() const { return m_ecdb; }
    BentleyStatus GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

    ECDb& SetupECDb(Utf8CP ecdbFileName);
    ECDb& SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);
    ECDb& SetupECDb(Utf8CP ecdbFileName, WCharCP schemaECXmlFileName, bool importArbitraryNumberECInstances, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    ECDb& SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) const;

    static DbResult CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite), int perClassRowCount = 0);
    static BentleyStatus CreateSeedECDb(BeFileNameR seedFilePath, Utf8CP seedFileName, BeFileNameCR schemaECXmlFileName, int perClassRowCount = 0);
    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));

    static DbResult CreateECDb(ECDbR ecdb, Utf8CP ecdbFileName);

public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture () {};
    virtual void SetUp() override;
    virtual void TearDown () override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling CreateTestProject, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();
    };

END_ECDBUNITTESTS_NAMESPACE
