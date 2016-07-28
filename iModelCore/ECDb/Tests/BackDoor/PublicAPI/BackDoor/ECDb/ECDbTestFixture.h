/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

        explicit SchemaItem(Utf8CP schemaXml) : SchemaItem("", schemaXml) {}
        SchemaItem(Utf8CP name, Utf8CP schemaXml) : m_name(name), m_expectedToSucceed(true) { m_schemaXmlList.push_back(Utf8String(schemaXml)); }
        SchemaItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) { m_schemaXmlList.push_back(Utf8String(schemaXml)); }
        SchemaItem(Utf8CP schemaXml, bool expectedToSucceeed) : m_expectedToSucceed(expectedToSucceeed) { m_schemaXmlList.push_back(Utf8String(schemaXml)); }
        SchemaItem(std::vector<Utf8String> const& schemaXmlList, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXmlList(schemaXmlList), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
        };

private:
    static bmap<bpair<WString, int>, Utf8String> s_seedECDbs;

    static bool s_isInitialized;

protected:
    mutable ECDb m_ecdb;

private:
    static BentleyStatus CreateECDb(BeFileNameR filePath, Utf8CP fileName, BeFileNameCR schemaECXmlFileName, int perClassRowCount = 0);
    static BentleyStatus CreateECDb(BeFileNameR filePath, Utf8CP fileName, SchemaItem const&, int perClassRowCount = 0);
    static DbResult CreateECDb(ECDbR, Utf8CP ecdbFileName);

    static BentleyStatus Populate(ECDbCR, ECN::ECSchemaCR, int instanceCountPerClass);
    static BentleyStatus Populate(ECDbCR, int instanceCountPerClass);

protected:
    ECDb& SetupECDb(Utf8CP ecdbFileName);
    ECDb& SetupECDb(Utf8CP ecdbFileName, BeFileNameCR schemaECXmlFileName, int perClassRowCount = 0, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    ECDb& SetupECDb(Utf8CP ecdbFileName, SchemaItem const& schema, int perClassRowCount = 0, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) const;

    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));

    ECDb& GetECDb() const { return m_ecdb; }
    BentleyStatus GetInstances (bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture () {};
    virtual void SetUp() override;
    virtual void TearDown () override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling SetupECDb, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();
    };

END_ECDBUNITTESTS_NAMESPACE
