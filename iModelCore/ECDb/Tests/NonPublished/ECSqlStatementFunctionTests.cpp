/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/Base64Utilities.h>

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct Utf8StringVirtualSet final : VirtualSet
    {
    private:
        std::function<bool(Utf8CP)> m_predicate;

        bool _IsInSet(int nVals, DbValue const* values) const override
            {
            BeAssert(nVals == 1);
            return m_predicate(values[0].GetValueText());
            }

    public:
        explicit Utf8StringVirtualSet(std::function<bool(Utf8CP)> const& predicate) : VirtualSet(), m_predicate(predicate) {}

        ~Utf8StringVirtualSet() {}
    };

struct ECSqlStatementFunctionTestFixture : ECDbTestFixture
    {
    //---------------------------------------------------------------------------------------
    // @bsiclass
    //+---------------+---------------+---------------+---------------+---------------+------
    struct ExpectedResult final
        {
        bool m_isPrepareSupported = false;
        bool m_isStepSupported = false;
        ECN::PrimitiveType m_returnType;

        ExpectedResult() {}
        explicit ExpectedResult(ECN::PrimitiveType returnType, bool isStepSupported = true) : m_isPrepareSupported(true), m_isStepSupported(isStepSupported), m_returnType(returnType) {}
        };
    };

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, SearchCaseExp_Type)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("CaseExp.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    struct Test{
        Utf8CP sql;
        ECSqlStatus status;
        ECN::PrimitiveType type;
    };
    std::vector<Test> testDataset{
        {"SELECT CASE WHEN 1 THEN B      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Boolean},
        {"SELECT CASE WHEN 1 THEN Bi     END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Binary},
        {"SELECT CASE WHEN 1 THEN D      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Double},
        {"SELECT CASE WHEN 1 THEN Dt     END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_DateTime},
        {"SELECT CASE WHEN 1 THEN I      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Integer},
        {"SELECT CASE WHEN 1 THEN L      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN 1 THEN S      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_String},
        {"SELECT CASE WHEN 1 THEN NULL ELSE S END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_String},
        {"SELECT CASE WHEN 1 THEN NULL ELSE I END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Integer},
        {"SELECT CASE WHEN 1 THEN NULL WHEN 1 THEN I  ELSE S END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Integer},
        {"SELECT CASE WHEN 1 THEN NULL WHEN 1 THEN Dt ELSE I END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_DateTime},        
        {"SELECT CASE WHEN 1 THEN P2D    END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN 1 THEN P3D    END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN 1 THEN MyPSA  END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN B  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN Bi THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN D  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN Dt THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN I  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN L  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Long},
        {"SELECT CASE WHEN P2D  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN P3D  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN MyPSA  THEN 1  END FROM ecsql.P LIMIT 1", ECSqlStatus::InvalidECSql},
        {"SELECT CASE WHEN 1>2 THEN I+2      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Double},
        {"SELECT CASE WHEN I>1 THEN I/3      END FROM ecsql.P LIMIT 1", ECSqlStatus::Success, ECN::PRIMITIVETYPE_Double},


    };
    for (Test const& test : testDataset)
        {
        Utf8String ecsql = test.sql;
        ECSqlStatement stmt;
        EXPECT_EQ(test.status, stmt.Prepare(m_ecdb,test.sql)) << test.sql;
        if (test.status == ECSqlStatus::Success)
            {
            EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql.c_str();
            ECN::ECTypeDescriptor const& actualColumnType = stmt.GetColumnInfo(0).GetDataType();
            ASSERT_TRUE(actualColumnType.IsPrimitive()) << ecsql;
            EXPECT_EQ(test.type, actualColumnType.GetPrimitiveType()) << ecsql;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, IIF)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("CaseExp.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    std::vector<std::pair<Utf8CP, Utf8CP>> testDataset{
        {"SELECT IIF(ec_classname(ecclassid)='ECSqlTest:TH5', 'First','Second') FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT IIF(1, 'First','Second') FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT IIF(0, 'First','Second') FROM ecsql.TH5 LIMIT 1", "Second" },
        {"SELECT IIF(NULL, 'First','Second') FROM ecsql.TH5 LIMIT 1", "Second" },
        {"SELECT IIF(NULL, 'First',NULL) FROM ecsql.TH5 LIMIT 1", nullptr },
        {"SELECT IIF(1>2, 'First','Second') FROM ecsql.TH5 LIMIT 1", "Second" },
        {"SELECT IIF(?>1, 'First','Second') FROM ecsql.TH5 LIMIT 1", "Second" },
        {"SELECT IIF(1 BETWEEN 2 AND 3, 'First','Second') FROM ecsql.TH5 LIMIT 1", "Second" },
    };
    for (std::pair<Utf8CP, Utf8CP> const& kvPair : testDataset)
        {
        Utf8String ecsql = kvPair.first;
        Utf8CP expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql.c_str();
        EXPECT_STREQ(expectedResult, stmt.GetValueText(0)) << ecsql.c_str();
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, SearchCaseExp_Basic)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("CaseExp.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    std::vector<std::pair<Utf8CP, Utf8CP>> testDataset{
        {"SELECT CASE WHEN ec_classname(ecclassid)  =   'ECSqlTest:TH5' THEN 'First' ELSE 'Second' END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN ec_classname(ecclassid)  <>  'ECSqlTest:TH5' THEN 'First' ELSE 'Second' END FROM ecsql.TH5 LIMIT 1", "Second"},
        {"SELECT CASE WHEN ec_classname(ecclassid) LIKE 'ECSqlTest%TH5' THEN 'First' ELSE 'Second' END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN ec_classname(ecclassid) LIKE 'ECSqlTest%TH5' THEN 'First' ELSE 'Second' END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN ec_classname(ecclassid) LIKE 'ECSqlTest%TH5' THEN 'First'               END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN ec_classname(ecclassid) LIKE 'ECSqlTest%TH5' THEN 'First'               END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN 1 THEN 'First' WHEN 0 THEN 'Second' WHEN 0 THEN 'Third' ELSE 'Forth'  END FROM ecsql.TH5 LIMIT 1", "First" },
        {"SELECT CASE WHEN 0 THEN 'First' WHEN 1 THEN 'Second' WHEN 0 THEN 'Third' ELSE 'Forth'  END FROM ecsql.TH5 LIMIT 1", "Second" },
        {"SELECT CASE WHEN 0 THEN 'First' WHEN 0 THEN 'Second' WHEN 1 THEN 'Third' ELSE 'Forth'  END FROM ecsql.TH5 LIMIT 1", "Third" },
        {"SELECT CASE WHEN 0 THEN 'First' WHEN 0 THEN 'Second' WHEN 0 THEN 'Third' ELSE 'Forth'  END FROM ecsql.TH5 LIMIT 1", "Forth" },
        {"SELECT CASE WHEN 1 THEN  NULL END FROM ecsql.TH5 LIMIT 1", nullptr },
    };
    for (std::pair<Utf8CP, Utf8CP> const& kvPair : testDataset)
        {
        Utf8String ecsql = kvPair.first;
        Utf8CP expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql.c_str();
        EXPECT_STREQ(expectedResult, stmt.GetValueText(0)) << ecsql.c_str();
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, TypeFilter)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InstanceOfFunc.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    m_ecdb.SaveChanges();
    auto th1 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH1");
    auto th2 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH2");
    auto th3 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH3");
    auto th4 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH4");
    auto th5 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH5");
    std::vector<std::pair<Utf8CP, int>> testDataset{
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ecsql.TH5)", 5},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ecsql.TH4)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ecsql.TH3)", 15},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ecsql.TH2)", 20},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ecsql.TH1)", 25},

        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH5)", 5},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH4)", 5},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH3)", 5},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH2)", 5},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH1)", 5},

        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH5, ONLY ecsql.TH1)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH4, ONLY ecsql.TH1)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH3, ONLY ecsql.TH1)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH2, ONLY ecsql.TH1)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS (ONLY ecsql.TH1, ONLY ecsql.TH2)", 10},

        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS NOT (ecsql.TH5)", 25},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS NOT (ecsql.TH4)", 20},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS NOT (ecsql.TH3)", 15},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS NOT (ecsql.TH2)", 10},
        {"SELECT COUNT(*) FROM ecsql.THBase t WHERE t.ECClassId IS NOT (ecsql.TH1)", 5},
        {"WITH getBase(ecClassId) AS (SELECT ECClassId FROM ecsql.THBase) SELECT COUNT(*) FROM getBase WHERE ecClassId IS (ecsql.TH4)", 10},
        {"WITH getBase(ecClassId) AS (SELECT ECClassId FROM ecsql.THBase) SELECT COUNT(*) FROM getBase WHERE ecClassId IS (ONLY ecsql.TH4)", 5},
    };
    for (std::pair<Utf8CP, int> const& kvPair : testDataset)
        {
        Utf8String ecsql = kvPair.first;
        int expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql.c_str();
        EXPECT_EQ(expectedResult, stmt.GetValueInt(0)) << ecsql.c_str() << "sql" << stmt.GetNativeSql();
        }    
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, InstanceOfFunc)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("InstanceOfFunc.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    auto th1 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH1");
    auto th2 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH2");
    auto th3 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH3");
    auto th4 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH4");
    auto th5 = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH5");

    std::vector<std::pair<Utf8CP, int>> testDataset {
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH5')", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH4')", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH3')", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH2')", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH1')", 25 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql.TH5', 'ecsql.TH3')", 15 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH5')", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH4')", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH3')", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH2')", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH1')", 25 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest.TH5', 'ECSqlTest.TH3')", 15 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH5')", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH4')", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH3')", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH2')", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH1')", 25 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ecsql:TH5', 'ecsql:TH3')", 15 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH5')", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH4')", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH3')", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH2')", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH1')", 25 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, 'ECSqlTest:TH5', 'ECSqlTest:TH3')", 15 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th5)", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th4)", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th3)", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th2)", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th1)", 25 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(t.ECClassId, %th5, %th3)", 15 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(%th5, t.ECClassId)", 30 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof('ECSqlTest:TH5', t.ECClassId)", 30 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof('ECSqlTest.TH5', t.ECClassId)", 30 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof('ecsql:TH5', t.ECClassId)", 30 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof('ecsql.TH5', t.ECClassId)", 30 },

            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(ec_classname(t.ECClassId), ec_classid('ECSqlTest:TH5'))", 5 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(ec_classname(t.ECClassId), ec_classid('ECSqlTest:TH4'))", 10 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(ec_classname(t.ECClassId), ec_classid('ECSqlTest:TH3'))", 15 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(ec_classname(t.ECClassId), ec_classid('ECSqlTest:TH2'))", 20 },
            {"SELECT COUNT(*) FROM ecsql.THBase t WHERE ec_instanceof(ec_classname(t.ECClassId), ec_classid('ECSqlTest:TH1'))", 25 },
    };
    for (std::pair<Utf8CP, int> const& kvPair : testDataset)
        {
        Utf8String ecsql = kvPair.first;
        ecsql.ReplaceAll("%th1", th1.ToString().c_str());
        ecsql.ReplaceAll("%th2", th2.ToString().c_str());
        ecsql.ReplaceAll("%th3", th3.ToString().c_str());
        ecsql.ReplaceAll("%th4", th4.ToString().c_str());
        ecsql.ReplaceAll("%th5", th5.ToString().c_str());
        int expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql.c_str();
        EXPECT_EQ(expectedResult, stmt.GetValueInt(0)) << ecsql.c_str();
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, ClassNameFunc)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ec_classname.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    // ASSERT_EQ(SUCCESS, PopulateECDb(5));

    std::vector<std::pair<Utf8CP, Utf8CP>> testDataset {
            {"SELECT ec_classname(ECInstanceId)           FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest:TH5" },
            {"SELECT ec_classname(ECInstanceId, NULL    ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest:TH5" },
            {"SELECT ec_classname(ECInstanceId, 0       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest:TH5" },
            {"SELECT ec_classname(ECInstanceId, 1       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql:TH5"     },
            {"SELECT ec_classname(ECInstanceId, 2       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest"     },
            {"SELECT ec_classname(ECInstanceId, 3       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql"         },
            {"SELECT ec_classname(ECInstanceId, 4       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "TH5"           },
            {"SELECT ec_classname(ECInstanceId, 5       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest.TH5" },
            {"SELECT ec_classname(ECInstanceId, 6       ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql.TH5"     },
            {"SELECT ec_classname(ECInstanceId, 's:c'   ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest:TH5" },
            {"SELECT ec_classname(ECInstanceId, 'a:c'   ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql:TH5"     },
            {"SELECT ec_classname(ECInstanceId, 's'     ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest"     },
            {"SELECT ec_classname(ECInstanceId, 'a'     ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql"         },
            {"SELECT ec_classname(ECInstanceId, 'c'     ) FROM meta.ECClassDef WHERE Name = 'TH5'", "TH5"           },
            {"SELECT ec_classname(ECInstanceId, 's.c'   ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ECSqlTest.TH5" },
            {"SELECT ec_classname(ECInstanceId, 'a.c'   ) FROM meta.ECClassDef WHERE Name = 'TH5'", "ecsql.TH5"     },
            {"SELECT ec_classname(ECInstanceId, 'a-c'   ) FROM meta.ECClassDef WHERE Name = 'TH5'", nullptr         },
            {"SELECT ec_classname(ECInstanceId, -1      ) FROM meta.ECClassDef WHERE Name = 'TH5'", nullptr         },
            {"SELECT ec_classname(ECInstanceId, ''      ) FROM meta.ECClassDef WHERE Name = 'TH5'", nullptr         },
    };
    for (std::pair<Utf8CP, Utf8CP> const& kvPair : testDataset)
        {
        Utf8CP ecsql = kvPair.first;
        Utf8CP expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;
        EXPECT_STREQ(expectedResult, stmt.GetValueText(0)) << ecsql;
        }
    }
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, ClassIdFunc)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ec_classid.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ECN::ECClassId th5ClassId = m_ecdb.Schemas().GetClassId("ECSqlTest", "TH5");
    ECN::ECClassId noClassId((uint64_t)0);

    std::vector<std::pair<Utf8CP, ECN::ECClassId>> testDataset {
        {"VALUES(ec_classid('ECSqlTest.TH5'     ))", th5ClassId },
        {"VALUES(ec_classid('ecsql.TH5'         ))", th5ClassId },
        {"VALUES(ec_classid('ECSqlTest:TH5'     ))", th5ClassId },
        {"VALUES(ec_classid('ecsql:TH5'         ))", th5ClassId },        
        {"VALUES(ec_classid('ecsql'    , 'TH5'  ))", th5ClassId },
        {"VALUES(ec_classid('ECSqlTest', 'TH5'  ))", th5ClassId },
        {"VALUES(ec_classid('ECSqlTest.TH5-'    ))", noClassId  },
        {"VALUES(ec_classid('ecsqlw.TH5'        ))", noClassId  },
        {"VALUES(ec_classid('ecsql1'    , 'TH5' ))", noClassId  },
        {"VALUES(ec_classid('ECSqlTest2', 'TH5' ))", noClassId  },
        {"VALUES(ec_classid(NULL, 'TH5'         ))", noClassId  },            
        {"VALUES(ec_classid(NULL, NULL          ))", noClassId  },            
    };
    for (std::pair<Utf8CP, ECN::ECClassId> const& kvPair : testDataset)
        {
        Utf8CP ecsql = kvPair.first;
        ECN::ECClassId expectedResult = kvPair.second;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;
        EXPECT_EQ(expectedResult, stmt.GetValueId<ECN::ECClassId>(0)) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, BuiltinFunctions)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(SUCCESS, PopulateECDb(5));
    ASSERT_EQ(SUCCESS, m_ecdb.AttachChangeCache(ECDb::GetDefaultChangeCachePath(m_ecdb.GetDbFileName())));

    std::vector<std::pair<Utf8CP, ExpectedResult>> testDataset {
            {"SELECT ABS(I) FROM ecsql.P LIMIT 1", ExpectedResult (ECN::PRIMITIVETYPE_Integer)},
            {"SELECT ANY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT AVG(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT CHANGES() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT CHAR(123,124) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT COALESCE(I,L) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT COUNT(*) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(ECInstanceId) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT COUNT(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT DATE('now') FROM ecsql.P LIMIT 1", ExpectedResult()}, //Not supported as DATE is a reserved token, and the ECSQL grammar only has one rule yet for DATE which is the DATE literal exp DATE '2000-01-01'
            {"SELECT DATETIME('now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT ec_classname(ECInstanceId, 5) FROM meta.ECClassDef WHERE Name = 'TH5'", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT EVERY(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GLOB(S,'Sample') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT GROUP_CONCAT(S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(S,'|') FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT GROUP_CONCAT(DISTINCT S) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT HEX(Bi) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT INSTR(S,'str') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT JSON(12.3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_ARRAY(12.3, 1.3, 3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_ARRAY_LENGTH('[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT JSON_EXTRACT('{\"a\" : 12}','$.a') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_GROUP_ARRAY('[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_GROUP_OBJECT('a','[1,2,3]') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_INSERT('{\"a\" : 12}','$.e',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_OBJECT('a',1,'b',2) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_QUOTE(123) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_REMOVE('{\"a\" : 12}','$.a') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_REPLACE('{\"a\" : 12}','$.a',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_SET('{\"a\" : 12}','$.a',10) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_TYPE('{\"a\" : 12}') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT JSON_VALID('{\"a\" : 12}') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT JULIANDAY('now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT LAST_INSERT_ROWID() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT LENGTH(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT LOWER(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT LIKE(S,'Sample') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as LIKE is a reserved token, and the ECSQL grammar only has one rule yet for LIKe which is the standard X LIKE Y syntax
            {"SELECT LIKE(S,'Sample','/') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as LIKE is a reserved token, and the ECSQL grammar only has one rule yet for LIKe which is the standard X LIKE Y syntax
            {"SELECT LTRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT LTRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT MATCH(S, 'str') FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MATCH is a reserved token, and the ECSQL grammar only has one rule yet for MATCH which is the X MATCH function() syntax
            {"SELECT MAX(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT MAX(123, 125, 512) FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MAX(arg) is a dedicated ECSQL grammar rule
            {"SELECT MIN(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT MIN(123, 125, 512) FROM ecsql.P LIMIT 1", ExpectedResult()},//Not supported as MIN(arg) is a dedicated ECSQL grammar rule
            {"SELECT NULLIF(I,123) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT QUOTE(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT PRINTF('%d',I) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RANDOM() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT RANDOMBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT REGEXP(S, 'str') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT REPLACE(S,'Sample','Simple') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT RTRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SOME(B) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT SOUNDEX(S) FROM ecsql.P LIMIT 1", ExpectedResult()}, //Only available if SQLite is compiled with SQLITE_SOUNDEX compile option
            {"SELECT SQLITE_COMPILEOPTION_GET(0) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SQLITE_COMPILEOPTION_USED(0) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Boolean)},
            {"SELECT SQLITE_SOURCE_ID() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SQLITE_VERSION() FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT STRFTIME('%J','now') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SUBSTR(S,'a',3) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT SUM(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Integer)},
            {"SELECT TIME('now') FROM ecsql.P LIMIT 1", ExpectedResult()}, //Not supported as TIME is a reserved token, and the ECSQL grammar only has one rule yet for TIME which is the TIME literal exp TIME '23:44:14'
            {"SELECT TOTAL(I) FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Double)},
            {"SELECT TOTAL_CHANGES() FROM ecsql.P", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT TRIM(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT TRIM(S, '$') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT TYPEOF(I) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT UNICODE('K') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            {"SELECT UPPER(S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            {"SELECT ZEROBLOB(5) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)},
            //BeSQLite built-in functions
            {"SELECT 10 FROM ecsql.P WHERE NOT InVirtualSet(?,123)", ExpectedResult(ECN::PRIMITIVETYPE_Long)},
            //ECDb built-in functions 
            {"SELECT ChangedValue(1,'S','AfterInsert',S) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)}, //only for ECDb internal use
            {"SELECT ChangedValueStateToOpCode('BeforeUpdate') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)}, //only for ECDb internal use
            {"SELECT ChangedValueStateToOpCode(2) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)}, //only for ECDb internal use
            {"SELECT StrToGuid('51cb28a5-624a-416b-8b5b-d92368a33492') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Binary)}, //only for ECDb internal use
            {"SELECT GuidToStr(StrToGuid('51cb28a5-624a-416b-8b5b-d92368a33492')) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)}, //only for ECDb internal use
            {"SELECT IdToHex(2333) FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_String)}, //only for ECDb internal use
            {"SELECT HexToId('0x1') FROM ecsql.P LIMIT 1", ExpectedResult(ECN::PRIMITIVETYPE_Long)}, //only for ECDb internal use
        };


    for (std::pair<Utf8CP, ExpectedResult> const& kvPair : testDataset)
        {
        Utf8CP ecsql = kvPair.first;
        ExpectedResult const& expectedResult = kvPair.second;
        const ECSqlStatus expectedPrepareStat = expectedResult.m_isPrepareSupported ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;

        ECSqlStatement stmt;
        ASSERT_EQ(expectedPrepareStat, stmt.Prepare(m_ecdb, ecsql)) << ecsql;

        if (!expectedResult.m_isStepSupported)
            continue;

        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << ecsql;

        ECN::ECTypeDescriptor const& actualColumnType = stmt.GetColumnInfo(0).GetDataType();

        ASSERT_TRUE(actualColumnType.IsPrimitive()) << ecsql;
        EXPECT_EQ(expectedResult.m_returnType, actualColumnType.GetPrimitiveType()) << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, CoalesceAndNullIf)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("builtinfunctiontests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    {
    //insert two test rows
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,L) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 124));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());


    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,L,COALESCE(I,L),COALESCE(L,I) FROM ecsql.P"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (stmt.IsValueNull(0))
            {
            ASSERT_FALSE(stmt.IsValueNull(1)) << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(2)) << "First coalesce " << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(3)) << "Second coalesce " << stmt.GetECSql();
            }
        else
            {
            ASSERT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(2)) << "First coalesce " << stmt.GetECSql();
            ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(3)) << "Second coalesce " << stmt.GetECSql();
            }
        }

    stmt.Finalize();
    m_ecdb.AbandonChanges();
    }

    {
    //insert a test row
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,L) VALUES(123,124)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT I,L, NULLIF(I,123), NULLIF(I,124), NULLIF(L,123), NULLIF(L,124) FROM ecsql.P"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql();


    ASSERT_TRUE(stmt.IsValueNull(2)) << "first nullif " << stmt.GetECSql();
    ASSERT_FALSE(stmt.IsValueNull(3)) << "second nullif " << stmt.GetECSql();
    ASSERT_EQ(stmt.GetValueInt(0), stmt.GetValueInt(3)) << "second nullif " << stmt.GetECSql();

    ASSERT_FALSE(stmt.IsValueNull(4)) << "third nullif " << stmt.GetECSql();
    ASSERT_EQ(stmt.GetValueInt(1), stmt.GetValueInt(4)) << "third nullif " << stmt.GetECSql();
    ASSERT_TRUE(stmt.IsValueNull(5)) << "fourth nullif " << stmt.GetECSql();
    }

    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, FunctionCallWithDistinct)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlbuiltinfunctiontest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml"), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    auto getIntScalar = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
            return -1;

        if (BE_SQLITE_ROW != stmt.Step())
            return -1;

        return stmt.GetValueInt(0);
        };

    auto getStringScalar = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql))
            return Utf8String();

        if (BE_SQLITE_ROW != stmt.Step())
            return Utf8String();

        return Utf8String(stmt.GetValueText(0));
        };


    {
    //create the following data
    //I|S
    //---
    //1|'1'
    //2|'1'
    //1|'2'
    //2|'3'
    //2|'3'
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.P(I,S) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "1", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "1", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "2", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "3", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "3", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    ASSERT_EQ(5, getIntScalar(m_ecdb, "SELECT count(*) from ecsql.P"));
    ASSERT_EQ(5, getIntScalar(m_ecdb, "SELECT count(distinct ECInstanceId) from ecsql.P"));
    ASSERT_EQ(2, getIntScalar(m_ecdb, "SELECT count(distinct I) from ecsql.P"));
    ASSERT_EQ(3, getIntScalar(m_ecdb, "SELECT count(distinct S) from ecsql.P"));

    ASSERT_STREQ("1,1,2,3,3", getStringScalar(m_ecdb, "SELECT group_concat(S) from ecsql.P").c_str());
    ASSERT_STREQ("1&1&2&3&3", getStringScalar(m_ecdb, "SELECT group_concat(S,'&') from ecsql.P").c_str());
    ASSERT_STREQ("11233", getStringScalar(m_ecdb, "SELECT group_concat(S,'') from ecsql.P").c_str());
    ASSERT_STREQ("1,2,3", getStringScalar(m_ecdb, "SELECT group_concat(DISTINCT S) from ecsql.P").c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, InVirtualSetFunction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    const int perClassRowCount = 10;
    ASSERT_EQ(SUCCESS, PopulateECDb( perClassRowCount));

    bvector<ECInstanceId> allIds;
    {
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT ECInstanceId FROM ecsql.PSA"));
    while (BE_SQLITE_ROW == statement.Step())
        {
        allIds.push_back(statement.GetValueId<ECInstanceId>(0));
        }
    ASSERT_EQ(perClassRowCount, (int) allIds.size());
    }

    IdSet<BeInt64Id> idSet;
    ASSERT_TRUE(0 < perClassRowCount);
    idSet.insert(allIds[0]);
    ASSERT_TRUE(4 < perClassRowCount);
    idSet.insert(allIds[4]);
    ASSERT_TRUE(6 < perClassRowCount);
    idSet.insert(allIds[6]);

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE I<>? AND InVirtualSet(?, ECInstanceId)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    statement.Reset();
    statement.ClearBindings();

    idSet.clear();
    ASSERT_TRUE(1 < perClassRowCount);
    idSet.insert(allIds[1]);
    ASSERT_TRUE(3 < perClassRowCount);
    idSet.insert(allIds[3]);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    statement.Reset();
    statement.ClearBindings();
    
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0));

    statement.Reset();
    statement.ClearBindings();

    //now binding a NULL virtual set
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0)) << statement.GetECSql() << " with NULL bound for the virtual set";

    statement.Reset();
    statement.ClearBindings();

    //now binding an empty virtual set
    idSet.clear();
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0)) << statement.GetECSql() << " with empty virtual set";

    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(2, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0)) << statement.GetECSql() << " with empty id set";

    //now same statement but with InVirtualSet in parentheses
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT count(*) FROM ecsql.PSA WHERE (InVirtualSet(?, ECInstanceId))"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding a virtual set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding a virtual set in parentheses";

    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding an id set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding an id set in parentheses";

    statement.Reset();
    statement.ClearBindings();

    idSet.clear();
    ASSERT_TRUE(1 < perClassRowCount);
    idSet.insert(allIds[1]);
    ASSERT_TRUE(3 < perClassRowCount);
    idSet.insert(allIds[3]);

    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding a virtual set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding a virtual set in parentheses";

    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(1, idSet));

    ASSERT_EQ(BE_SQLITE_ROW, statement.Step()) << "Step with binding an id set in parentheses";
    ASSERT_EQ((int) idSet.size(), statement.GetValueInt(0)) << "Step with binding an id set in parentheses";
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementFunctionTestFixture, InVirtualSetValidation)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecsqlstatementtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT InVirtualSet(1023, 1);")) << "First parameter should not be Primitive type";
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT InVirtualSet('HELLO', 1);")) << "First parameter should not be Primitive type";
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT InVirtualSet(3.3, 1);")) << "First parameter should not be Primitive type";
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT InVirtualSet(2+?, 1);")) << "First parameter should not be Primitive type";
    statement.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, statement.Prepare(m_ecdb, "SELECT InVirtualSet(?+?, 1);")) << "First parameter should not be Primitive type";
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT InVirtualSet(?, 1)"));
    // Bind nothing
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();

    // Bind NULL value
    ASSERT_EQ(ECSqlStatus::Success, statement.BindNull(1));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind Boolean value;
    ASSERT_EQ(ECSqlStatus::Error, statement.BindBoolean(1, false));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind Double value
    ASSERT_EQ(ECSqlStatus::Error, statement.BindDouble(1, 1.2));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind Int value
    ASSERT_EQ(ECSqlStatus::Error, statement.BindInt(1, 15));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // // Bind Int64 value
    // ASSERT_EQ(ECSqlStatus::Error, statement.BindInt64(1, 15));
    // ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    // ASSERT_EQ(0, statement.GetValueInt(0));
    // statement.Reset();
    // statement.ClearBindings();

    // Bind Point2d value
    ASSERT_EQ(ECSqlStatus::Error, statement.BindPoint2d(1, DPoint2d::From(2, 3)));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind Point3d value
    ASSERT_EQ(ECSqlStatus::Error, statement.BindPoint3d(1, DPoint3d::From(2, 3, 4)));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind Text value
    ASSERT_EQ(ECSqlStatus::Error, statement.BindText(1, "str", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind empty IdSet
    IdSet<BeInt64Id> idSet;
    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(1, idSet));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind empty VirtualSet IdSet
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(0, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind non-empty IdSet
    idSet.insert(BeInt64Id(1));
    idSet.insert(BeInt64Id(5));
    idSet.insert(BeInt64Id(10));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindIdSet(1, idSet));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(1, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind non-empty VirtualSet IdSet
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, idSet));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(1, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();

    // Bind string VirtualSet
    Utf8StringVirtualSet stringSet([] (Utf8CP name) { return "tempstring"; });
    ASSERT_EQ(ECSqlStatus::Success, statement.BindVirtualSet(1, stringSet));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(1, statement.GetValueInt(0));
    statement.Reset();
    statement.ClearBindings();
    }


END_ECDBUNITTESTS_NAMESPACE
