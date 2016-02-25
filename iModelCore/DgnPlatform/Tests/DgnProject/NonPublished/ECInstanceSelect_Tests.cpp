/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECInstanceSelect_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
USING_NAMESPACE_BENTLEY_DPTEST

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ECInstanceSelectTests : public DgnDbTestFixture
{

};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECInstanceSelectTests, SelectQueriesOnDbGeneratedDuringBuild_04Plant)
    {
    SetupProject(L"04_Plant.i.idgndb", L"SelectQueriesOnDbGeneratedDuringBuild_04Plant.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT TAG FROM appdw.Equipment WHERE EQUIP_NO='50P-104B'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("50P-104B", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COUNT(EQUIP_NO) FROM appdw.Equipment WHERE INSUL_THK='2' AND DES_TEMP='0'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT DISTINCT INSULATION FROM appdw.PipingComponent"));
    Utf8String ExpectedInsulationValue = "AA-N-";
    Utf8String ActualInsulationValue = "";
    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualInsulationValue.append (stmt.GetValueText (0));
        ActualInsulationValue.append ("-");
        }
    ASSERT_EQ (ExpectedInsulationValue, ActualInsulationValue);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT AREA FROM appdw.PipingComponent WHERE MAIN_SIZE='10'"));
    int ExpectedSumOfArea = 500;
    int ActualSumOfArea = 0;
    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualSumOfArea += stmt.GetValueInt (0);
        }
    ASSERT_EQ (ExpectedSumOfArea, ActualSumOfArea);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT EQUIP_NO FROM appdw.Nozzle WHERE COMP_LEN=4.5 OR COMPONENT_ID='AT_EGXUTRMA_1A'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("50TW-102", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT NAME FROM appdw.Pipeline WHERE NAME LIKE 'TRIM'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("TRIM", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COMPONENT_ID FROM appdw.Equipment_Part WHERE EQUIP_NO IN(SELECT EQUIP_NO FROM appdw.Equipment_Part WHERE POSX=15798.99902)"));
    Utf8String ExpectedCompIdValue = "AT_EGXUTRMA_3-AT_EGXUTRMA_5-AT_EGXUTRMA_8-AT_EGXUTRMA_9-";
    Utf8String ActualCompIdValue = "";
    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualCompIdValue.append (stmt.GetValueText (0));
        ActualCompIdValue.append ("-");
        }
    ASSERT_EQ (ExpectedCompIdValue, ActualCompIdValue);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COMPONENT_ID FROM appdw.Nozzle WHERE COMPONENT_ID LIKE '%_3G'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("AT_EGXUTRMA_3G", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COUNT(EQUIP_NO) FROM appdw.Equipment WHERE POSX BETWEEN 16300 AND 16400"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (2, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COUNT(COMPONENT_ID) FROM appdw.Equipment_Part WHERE TAG IS NULL"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT DISTINCT MODULE_NAME FROM appdw.PipingComponent WHERE COMPONENT_ID IN(SELECT COMPONENT_ID FROM appdw.PipingComponent WHERE MAIN_SIZE IS NOT NULL AND MAIN_SIZE='10')"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("Base", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT SUM(POSX) FROM appdw.Equipment WHERE INSUL_THK='2'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (95429, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT MAX(POSY) FROM appdw.Equipment"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (5403.87402, stmt.GetValueDouble (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT AVG(POSZ) FROM appdw.Equipment"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (239, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COUNT(INSUL_THK) FROM appdw.Equipment WHERE INSUL_THK=2"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT EQUIP_NO FROM ONLY appdw.Equipment WHERE EQUIP_NO='50E-101A'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("50E-101A", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select COMPONENT_ID From appdw.Nozzle where COMPONENT_ID ='AT_EGXUTRMA_1P' "));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("AT_EGXUTRMA_1P", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select COUNT(SPEC_TABLE) From appdw.PipingComponent where SPEC_TABLE Like 'ELBOW'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (147, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select COMPONENT_ID From appdw.Nozzle where COMPONENT_ID Like '%A_3%'"));

    Utf8String ExpectedStringValue = "AT_EGXUTRMA_35-AT_EGXUTRMA_36-AT_EGXUTRMA_37-AT_EGXUTRMA_38-AT_EGXUTRMA_3D-AT_EGXUTRMA_3E-AT_EGXUTRMA_3F-AT_EGXUTRMA_3G-";
    Utf8String ActualStringValue = "";

    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualStringValue.append (stmt.GetValueText (0));
        ActualStringValue.append ("-");
        }
    ASSERT_EQ (ExpectedStringValue, ActualStringValue);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::InvalidECSql, stmt.Prepare (*m_db, "Select * FROM appdw.blabla")) << "No such table exists.";
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select POSX,POSY,POSZ From appdw.Equipment WHERE EQUIP_NO='50TW-102'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (16026.99902, stmt.GetValueDouble (0));
    ASSERT_EQ (5235.62402, stmt.GetValueDouble (1));
    ASSERT_EQ (353.99803, stmt.GetValueDouble (2));
    stmt.Finalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECInstanceSelectTests, SelectQueriesOnDbGeneratedDuringBuild_79Main)
    {
    WCharCP baseProjFile = L"79_Main.i.idgndb";
    WCharCP testProjFile = L"SelectQueriesOnDbGeneratedDuringBuild_79Main.idgndb";
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;

    BeFileName outFileName;
    ASSERT_EQ (SUCCESS, DgnDbTestDgnManager::GetTestDataOut (outFileName, baseProjFile, testProjFile, __FILE__));

    OpenDb (m_db, outFileName, mode);

    ECSqlStatement stmt;

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select DISTINCT EQP_DESC From ams.EQUIP_MEQP Where EQP_DESC LIKE 'HEA%%'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("HEAT EXCHANGER", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select COUNT(SRC_CODE) From ams.EQUIP_MEQP Where SRC_CODE='FENCE'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (8, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select BOM_FLAG From ams.EQUIP_MEQP where CLIP_LENGTH=6.0572915077209473"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (1, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select EQP_NO From ams.EQUIP_MEQP where ELEMENT_ID>5000 ORDER BY EQP_NO ASC"));
    Utf8String ExpectedStringValue = "102-104-104-104-";
    Utf8String ActualStringValue = "";

    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualStringValue.append (stmt.GetValueText (0));
        ActualStringValue.append ("-");
        }
    ASSERT_EQ (ExpectedStringValue, ActualStringValue);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select COUNT(BOM_FLAG) AS MyQuery From ams.EQUIP_MEQP where BOM_FLAG>=1"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (16, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select DISTINCT COMPTYPE From ams.EQUIP_MEQP Where COMPTYPE LIKE '%EQ%'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("MEQP", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select EQP_SEQ From ams.EQUIP_MEQP Where EQP_DESC='FEED PUMP' AND D_STATUS='IN_STUDY'"));
    Utf8String ExpectedEqpSeq = "P-P-P-P-";
    Utf8String ActualEqpSeq = "";

    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualEqpSeq.append (stmt.GetValueText (0));
        ActualEqpSeq.append ("-");
        }
    ASSERT_EQ (ExpectedEqpSeq, ActualEqpSeq);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select SRC_CODE From ams.EQUIP_MEQP Where CLIP_LENGTH BETWEEN 1 AND 10"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("CLSLIB", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select ORIG_DES From ams.EQUIP_MEQP Where EQP_SEQ='TW' OR ELEMENT_ID='700'"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("Alan.Leonard", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT SPEC From ams.PIPE_PFLR Where FL_TYPE IN(Select FL_TYPE From ams.PIPE_PFLR Where EPREP1='BW')"));
    Utf8String ExpectedSpec = "1C-1C-1C-1C-";
    Utf8String ActualSpec = "";

    while (stmt.Step () != DbResult::BE_SQLITE_DONE)
        {
        ActualSpec.append (stmt.GetValueText (0));
        ActualSpec.append ("-");
        }
    ASSERT_EQ (ExpectedSpec, ActualSpec);
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select SUM(RATING) From ams.EQUIP_PNOZ Where SIZE_1=10"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ (750, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select DISTINCT DATABASE From ams.EQUIP_PNOZ Where SIZE_1 NOT BETWEEN 1 AND 19"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("EQUIP", stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT NOZ_NO From ams.EQUIP_PNOZ Where LINENO =(Select LINENO From ams.EQUIP_PNOZ Where EQP_TRN='0' AND EQP_NO='101')"));
    ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
    ASSERT_STREQ ("N3", stmt.GetValueText (0));
    stmt.Finalize ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         1/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECInstanceSelectTests, ImportSchema)
    {
    WCharCP baseProjFile = L"79_Main.i.idgndb";
    CharCP testProjFile = "TestDb.idgndb";
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::ReadWrite;

    DgnDbTestDgnManager tdm (baseProjFile, testProjFile, mode, false);
    m_db = tdm.GetDgnProjectP ();

    auto status = DgnPlatformTestDomain::GetDomain ().ImportSchema (*m_db);
    ASSERT_EQ (DgnDbStatus::Success, status);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         1/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECInstanceSelectTests, VerifyInstanceCountFor04Plant)
    {
    SetupProject (L"04_Plant.i.idgndb", L"InstanceCountVerification04Plant.idgndb", BeSQLite::Db::OpenMode::ReadWrite);

    ECSchemaList schemaList;
    ECSqlStatement stmt;

    bmap<Utf8String, int> ClassList;
    bmap<Utf8String, int> BenchMark;

    BenchMark["ArcPlates"] = 9;
    BenchMark["ArcShapes"] = 40;
    BenchMark["Equipment"] = 29;
    BenchMark["Equipment_Part"] = 15;
    BenchMark["NonComponent"] = 21;
    BenchMark["Nozzle"] = 55;
    BenchMark["Pipeline"] = 61;
    BenchMark["PipingComponent"] = 703;
    BenchMark["Plates"] = 1;
    BenchMark["Shapes"] = 433;
    BenchMark["VolBodies"] = 20;

    ASSERT_EQ (BentleyStatus::SUCCESS, m_db->Schemas ().GetECSchemas (schemaList));

    for (auto i = schemaList.begin (); i != schemaList.end (); ++i)
        {
        ECN::ECSchemaCP schema = *i;
        Utf8StringCR schemaName = schema->GetName ();

        if (schemaName == "AutoPlantPDWPersistenceStrategySchema" || schemaName == "AutoPlantPDW_CustomAttributes" || schemaName == "ProStructures")
            {
            Utf8StringCR schemaPrefix = schema->GetNamespacePrefix ();
            for (ECN::ECClassCP const& ecClass : schema->GetClasses ())
                {
                if (ecClass->IsEntityClass () == false)
                    continue;
                Utf8String ClassName = ecClass->GetName ().c_str ();
                Utf8String query = "SELECT COUNT(*) FROM ";
                query.append (schemaPrefix);
                query.append (".[");
                query.append (ClassName);
                query.append ("]");

                ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, query.c_str ())) << "Statement prepare failed for " << query.c_str ();
                ASSERT_EQ (stmt.Step (), DbResult::BE_SQLITE_ROW);
                ClassList[ClassName] = stmt.GetValueInt (0);
                stmt.Finalize ();
                query.clear ();
                }
            }
        }

    ASSERT_TRUE(ClassList.size() == BenchMark.size()) << "Size of the maps doesn't match.  Expected: " << (int)BenchMark.size() << " Actual: " << (int)ClassList.size();

    bmap<Utf8String, int>::iterator i, j;
    i = ClassList.begin ();
    j = BenchMark.begin ();

    while (i != ClassList.end () && j != BenchMark.end ())
        {
        Utf8String actualClassName = i->first;
        Utf8String expectedClassName = j->first;
        int actualInstanceCount = i->second;
        int expectedInstanceCount = j->second;

        ASSERT_STREQ (expectedClassName.c_str (), actualClassName.c_str ()) << "Class names mismatch at index " << i.position;
        ASSERT_EQ (expectedInstanceCount, actualInstanceCount) << "Instance count mismatch for class '" << i->first << "'";
        ++i;
        ++j;
        }
    }