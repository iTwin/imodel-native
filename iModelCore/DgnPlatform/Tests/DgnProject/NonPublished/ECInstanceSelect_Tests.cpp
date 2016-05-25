/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECInstanceSelect_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <Logging/bentleylogging.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

USING_NAMESPACE_BENTLEY_DPTEST

//---------------------------------------------------------------------------------------
// @bsiClass                                      Muhammad Hassan                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
struct ECInstanceSelectTests : public DgnDbTestFixture
{
protected:
    void VerifyInstanceCounts(WCharCP fileName, bmap<Utf8String, int>& benchMark, bvector<Utf8String>& schemasToCheck);
};

void ECInstanceSelectTests::VerifyInstanceCounts(WCharCP fileName, bmap<Utf8String, int>& benchMark, bvector<Utf8String>& schemasToCheck)
    {
    WCharCP testProjFile = L"InstanceCountVerification.ibim";
    BeSQLite::Db::OpenMode mode = BeSQLite::Db::OpenMode::Readonly;

    BeFileName outFileName;
    ASSERT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, fileName, testProjFile, __FILE__));

    OpenDb(m_db, outFileName, mode);
    bvector<ECN::ECSchemaCP> schemaList;
    ECSqlStatement stmt;

    bmap<Utf8String, int> classList;
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().GetECSchemas(schemaList));

    for (auto i = schemaList.begin(); i != schemaList.end(); ++i)
        {
        ECN::ECSchemaCP schema = *i;
        Utf8StringCR schemaName = schema->GetName();
        bvector<Utf8String>::const_iterator iter = std::find(schemasToCheck.begin(), schemasToCheck.end(), schemaName);

        if (schemasToCheck.end() != iter)
            {
            Utf8String query;
            for (ECN::ECClassCP const& ecClass : schema->GetClasses())
                {
                if (ecClass->IsEntityClass() == false || ECN::ECClassModifier::Abstract == ecClass->GetClassModifier())
                    continue;
                if (!Utf8String::IsNullOrEmpty(query.c_str()))
                    query.append(" UNION ALL ");

                query.append("SELECT COUNT(*), '").append(ecClass->GetName()).append("' FROM ONLY ").append(ecClass->GetECSqlName());
                }

            if (query.empty())
                continue;

            ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, query.c_str())) << "Statement prepare failed for " << query.c_str();
            while (DbResult::BE_SQLITE_ROW == stmt.Step())
                {
                int count = stmt.GetValueInt(0);
                if (0 != count)
                    classList[stmt.GetValueText(1)] = stmt.GetValueInt(0);
                }
            stmt.Finalize();
            query.clear();
            }
        }
    if (classList.size() != benchMark.size())
        {
        for (bmap<Utf8String, int>::const_iterator iter = classList.begin(); iter != classList.end(); iter++)
            LOG.errorv("%s:%d", iter->first.c_str(), iter->second);
        }
    ASSERT_TRUE(classList.size() == benchMark.size()) << "Size of the maps doesn't match.  Expected: " << (int)benchMark.size() << " Actual: " << (int)classList.size();

    bmap<Utf8String, int>::iterator i, j;
    i = classList.begin();
    j = benchMark.begin();

    while (i != classList.end() && j != benchMark.end())
        {
        Utf8String actualClassName = i->first;
        Utf8String expectedClassName = j->first;
        int actualInstanceCount = i->second;
        int expectedInstanceCount = j->second;

        ASSERT_STREQ(expectedClassName.c_str(), actualClassName.c_str()) << "Class names mismatch at index " << i.position;
        ASSERT_EQ(expectedInstanceCount, actualInstanceCount) << "Instance count mismatch for class '" << i->first << "'";
        ++i;
        ++j;
        }
    CloseDb();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Maha Nasir                         10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECInstanceSelectTests, SelectQueriesOnDbGeneratedDuringBuild_04Plant)
    {
    SetupProject(L"04_Plant.i.ibim", L"SelectQueriesOnDbGeneratedDuringBuild_04Plant.ibim", BeSQLite::Db::OpenMode::ReadWrite);

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

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT EQUIP_NO FROM appdw.Nozzle WHERE COMP_LEN='4.5' OR COMPONENT_ID='AT_EGXUTRMA_1A'"));
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

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "SELECT COUNT(INSUL_THK) FROM appdw.Equipment WHERE INSUL_THK='2'"));
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
    WCharCP baseProjFile = L"79_Main.i.ibim";
    WCharCP testProjFile = L"SelectQueriesOnDbGeneratedDuringBuild_79Main.ibim";
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

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (*m_db, "Select EQP_NO From ams.EQUIP_MEQP where ELEMENT_ID>'5000' ORDER BY EQP_NO ASC"));
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
TEST_F (ECInstanceSelectTests, VerifyInstanceCountFor04Plant)
    {

    bmap<Utf8String, int> benchMark;

    benchMark["ArcPlates"] = 9;
    benchMark["ArcShapes"] = 40;
    benchMark["Equipment"] = 14;
    benchMark["Equipment_Part"] = 15;
    benchMark["NonComponent"] = 21;
    benchMark["Nozzle"] = 55;
    benchMark["Pipeline"] = 61;
    benchMark["PipingComponent"] = 703;
    benchMark["Plates"] = 1;
    benchMark["Shapes"] = 433;
    benchMark["VolBodies"] = 20;

    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("AutoPlantPDWPersistenceStrategySchema");
    schemasToCheck.push_back("AutoPlantPDW_CustomAttributes");
    schemasToCheck.push_back("ProStructures");
    VerifyInstanceCounts(L"04_Plant.i.ibim", benchMark, schemasToCheck);
    }

// CGM - Unfortunately, this test takes over 11 seconds to run
//TEST_F(ECInstanceSelectTests, VerifyInstanceCountFor79Main)
//    {
//    bmap<Utf8String, int> benchMark;
//
//    benchMark["TriformaCommon"] = 91;
//    benchMark["EQUIP_MEQP"] = 16;
//    benchMark["EQUIP_PNOZ"] = 58;
//    benchMark["ILPIP_ILPP"] = 30;
//    benchMark["PIPE_OPLT"] = 5;
//    benchMark["PIPE_PBRN"] = 28;
//    benchMark["PIPE_PCAP"] = 3;
//    benchMark["PIPE_PCRD"] = 25;
//    benchMark["PIPE_PELB"] = 148;
//    benchMark["PIPE_PERD"] = 11;
//    benchMark["PIPE_PFLG"] = 128;
//    benchMark["PIPE_PFLR"] = 5;
//    benchMark["PIPE_PGKT"] = 131;
//    benchMark["PIPE_PIPE"] = 177;
//    benchMark["PIPE_PNOT"] = 2;
//    benchMark["PIPE_PNPL"] = 4;
//    benchMark["PIPE_PVLV"] = 45;
//
//    bvector<Utf8String> schemasToCheck;
//    schemasToCheck.push_back("BuildingDataGroup");
//    schemasToCheck.push_back("ams");
//    VerifyInstanceCounts(L"79_Main.i.ibim", benchMark, schemasToCheck);
//    }

TEST_F(ECInstanceSelectTests, VerifyInstanceCountForBGRSubset)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["GroupingComponent"] = 405;
    benchMark["Organizer"] = 11;
    benchMark["PartComponent"] = 1410;
    benchMark["Root"] = 1;
    benchMark["SystemComponent"] = 1424;
    benchMark["VisualizationRuleSet"] = 5;

    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("CSimProductData");
    schemasToCheck.push_back("ReviewVisualization");
    VerifyInstanceCounts(L"BGRSubset.i.ibim", benchMark, schemasToCheck);
    }

TEST_F(ECInstanceSelectTests, VerifyinstanceCountsForfacilities_secondary)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["Coms__x0020__Outlet"] = 52;
    benchMark["Deduction__x0020__Area"] = 5;
    benchMark["Electr__x0020__Outlet"] = 45;
    benchMark["Employee"] = 18;
    benchMark["Floor"] = 1;
    benchMark["Office__x0020__Furniture"] = 141;
    benchMark["PC"] = 20;
    benchMark["Room"] = 25;
    benchMark["Zone"] = 3;
    benchMark["Secondary__x0020__ECInstanceElementAspect"] = 1;

    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("Bentley_Facilities_ExpImp_Schema_becert__x003a__Marlow");
    schemasToCheck.push_back("DgnCustomItemTypes_Custom__x0020__Item__x0020__Types");
    VerifyInstanceCounts(L"facilities_secondaryinstances.ibim", benchMark, schemasToCheck);
    }

// CGM - Unfortunately, this test takes over 8 seconds to run
//TEST_F(ECInstanceSelectTests, VerifyInstanceCountsForMain)
//    {
//    bmap<Utf8String, int> benchMark;
//
//    benchMark["TriformaCommon"] = 91;
//    benchMark["EQUIP_PNOZ"] = 46;
//    benchMark["ILPIP_ILPP"] = 30;
//    benchMark["PIPE_OPLT"] = 5;
//    benchMark["PIPE_PBRN"] = 28;
//    benchMark["PIPE_PCAP"] = 3;
//    benchMark["PIPE_PCRD"] = 25;
//    benchMark["PIPE_PELB"] = 148;
//    benchMark["PIPE_PERD"] = 11;
//    benchMark["PIPE_PFLG"] = 128;
//    benchMark["PIPE_PFLR"] = 5;
//    benchMark["PIPE_PGKT"] = 131;
//    benchMark["PIPE_PIPE"] = 177;
//    benchMark["PIPE_PNOT"] = 2;
//    benchMark["PIPE_PNPL"] = 4;
//    benchMark["PIPE_PVLV"] = 45;
//
//    bvector<Utf8String> schemasToCheck;
//    schemasToCheck.push_back("BuildingDataGroup");
//    schemasToCheck.push_back("ams");
//    VerifyInstanceCounts(L"Main.ibim", benchMark, schemasToCheck);
//    }

TEST_F(ECInstanceSelectTests, VerifyInstanceCountsForMobileDgn_test)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["Class"] = 2;
    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("Test");
    VerifyInstanceCounts(L"MobileDgn_test1.i.ibim", benchMark, schemasToCheck);
    }

TEST_F(ECInstanceSelectTests, VerifyInstanceCountsForMobile_file)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["Area"] = 1;
    benchMark["DOCUMENT"] = 1;
    benchMark["Equipment"] = 2;
    benchMark["Pipeline"] = 1;
    benchMark["PipingComponent"] = 5;
    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("AutoPlantPDWPersistenceStrategySchema");
    VerifyInstanceCounts(L"Mobile_file.i.ibim", benchMark, schemasToCheck);
    }

TEST_F(ECInstanceSelectTests, VerifyInstanceCountsForrxmrlw1f)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["Coms__x0020__Outlet"] = 52;
    benchMark["Deduction__x0020__Area"] = 5;
    benchMark["EXP_IMP_SETTINGS_CLASS"] = 9;
    benchMark["Electr__x0020__Outlet"] = 46;
    benchMark["Employee"] = 18;
    benchMark["Floor"] = 1;
    benchMark["Office__x0020__Furniture"] = 141;
    benchMark["PC"] = 20;
    benchMark["Room"] = 10;
    benchMark["Zone"] = 3;

    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("Bentley_Facilities_ExpImp_Schema_becert__x003a__Marlow");
    VerifyInstanceCounts(L"rxmrlw1f.ibim", benchMark, schemasToCheck);
    }

TEST_F(ECInstanceSelectTests, VerifyInstanceCountsForsecondary)
    {
    bmap<Utf8String, int> benchMark;

    benchMark["PrimInstance"] = 3;
    benchMark["SecInstanceElementAspect"] = 1;
    benchMark["SecInstanceBaseElementAspect"] = 1;

    bvector<Utf8String> schemasToCheck;
    schemasToCheck.push_back("secinstances");
    VerifyInstanceCounts(L"secondaryinstances.ibim", benchMark, schemasToCheck);
    }

