/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ECDbTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================    
struct SchemaItem final
    {
    public:
        enum class Type
            {
            String,
            File
            };

    private:
        Type m_type;
        Utf8String m_xmlStringOrFileName;

        SchemaItem(Type type, Utf8StringCR xmlStringOrFileName) : m_type(type), m_xmlStringOrFileName(xmlStringOrFileName) {}

    public:
        explicit SchemaItem(Utf8StringCR xmlString) : SchemaItem(Type::String, xmlString) {}
        static SchemaItem CreateForFile(Utf8StringCR schemaFileName) { return SchemaItem(Type::File, schemaFileName); }

        Type GetType() const { return m_type; }
        Utf8StringCR GetXmlString() const { BeAssert(m_type == Type::String);  return m_xmlStringOrFileName; }
        BeFileName GetFileName() const { BeAssert(m_type == Type::File); return BeFileName(m_xmlStringOrFileName.c_str(), true); }
        Utf8StringCR ToString() const { return m_xmlStringOrFileName; }
    };


//=======================================================================================
//! All non-static methods operate on ECDb held by the test fixture. The test fixture's ECDb
//! is created by using SetupECDb.
// @bsiclass                                                 Carole.MacDonald     09/2015
//=======================================================================================    
struct ECDbTestFixture : public ::testing::Test
    {
private:
    struct SeedECDbManager final : NonCopyableClass
        {
    private:
        bmap<BeFileName, BeFileName> m_seedFilePathsBySchemaFileName;

    public:
        bool TryGet(BeFileName& seedPath, BeFileNameCR schemaFileName) const
            {
            auto it = m_seedFilePathsBySchemaFileName.find(schemaFileName);
            if (it == m_seedFilePathsBySchemaFileName.end())
                return false;

            seedPath = it->second;
            return true;
            }

        BeFileNameCR Add(BeFileNameCR schemaFileName, BeFileNameCR seedPath) 
            { 
            BeAssert(m_seedFilePathsBySchemaFileName.find(schemaFileName) == m_seedFilePathsBySchemaFileName.end());
            auto ret = m_seedFilePathsBySchemaFileName.insert(bpair<BeFileName, BeFileName>(schemaFileName, seedPath));
            //return the inserted seed path
            return ret.first->second;
            }

        };

    static bool s_isInitialized;
    static SeedECDbManager* s_seedECDbManager;

    static SeedECDbManager& SeedECDbs();

protected:
    ECDb m_ecdb;

    DbResult SetupECDb(Utf8CP ecdbFileName);
    BentleyStatus SetupECDb(Utf8CP ecdbFileName, SchemaItem const&, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    DbResult ReopenECDb();
    DbResult CloneECDb(Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite)) { return CloneECDb(m_ecdb, cloneFileName, seedFilePath, openParams); }
    static DbResult CloneECDb(ECDbR clone, Utf8CP cloneFileName, BeFileNameCR seedFilePath, ECDb::OpenParams openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
    BentleyStatus PopulateECDb(ECN::ECSchemaCR, int instanceCountPerClass);
    BentleyStatus PopulateECDb(int instanceCountPerClass);

    BentleyStatus ImportSchema(SchemaItem const&);
    BentleyStatus ImportSchemas(std::vector<SchemaItem> const&);

    BentleyStatus GetInstances(bvector<ECN::IECInstancePtr>& instances, Utf8CP schemaName, Utf8CP className);

public:
    ECDbTestFixture() : ::testing::Test() {}
    virtual ~ECDbTestFixture() {}
    void SetUp() override { Initialize(); }
    void TearDown() override {}

    //! Initializes the test environment by setting up the schema read context and search dirs etc.
    //! Gets implicitly called when calling SetupECDb, too. Tests that don't use
    //! that method can call this method statically.
    static void Initialize();

    static BeFileName BuildECDbPath(Utf8CP ecdbFileName);
    static DbResult CreateECDb(ECDbR, Utf8CP ecdbFileName = nullptr);
    static BentleyStatus ReadECSchema(ECN::ECSchemaReadContextPtr&, ECDbCR, SchemaItem const&);
    };

END_ECDBUNITTESTS_NAMESPACE
