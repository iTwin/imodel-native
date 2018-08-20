/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/formatting_test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeTimeUtilities.h>
#include <Formatting/FormattingApi.h>
#include <Formatting/AliasMappings.h>
#include <Units/UnitRegistry.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/Units.h>
#include "FormattingTestFixture.h"
#include <BeSQLite/L10N.h>




//#define FORMAT_DEBUG_PRINT

using namespace BentleyApi::Formatting;
BEGIN_BENTLEY_FORMATTEST_NAMESPACE
static UnitProxySetCP upx = nullptr;
static int repc = 0;
BE_JSON_NAME(degrees)
BE_JSON_NAME(low)
BE_JSON_NAME(high)
BE_JSON_NAME(yaw)
BE_JSON_NAME(pitch)
BE_JSON_NAME(roll)

//static void SetUpL10N() 
//    {
//    BeFileName sqlangFile;
//    BeTest::GetHost().GetDgnPlatformAssetsDirectory(sqlangFile);
//    sqlangFile.AppendToPath(L"sqlang");
//    sqlangFile.AppendToPath(L"Units_en.sqlang.db3");
//
//    BeFileName temporaryDirectory;
//    BeTest::GetHost().GetTempDir(temporaryDirectory);
//
//    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory, BeSQLite::BeSQLiteLib::LogErrors::Yes);
//    BeSQLite::L10N::Shutdown();
//    BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(sqlangFile));
//    }
//
//static void TearDownL10N()
//    {
//    BeSQLite::L10N::Shutdown();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
// Arbitrary locale support is inconsistent / missing on other OSes.
#if defined(BENTLEYCONFIG_OS_WINDOWS)
TEST(FormattingTest, Preliminary)
    {
    FormattingTestFixture::SetUpL10N();
    LOG.infov("================  Formatting Log ===========================");

    FormattingTestFixture::TestTimeFormat(2018, 3, 21, 7, 23, 35);

    // Empirical evidence suggests that Windows wants hyphens, and Unix'ish system want underscores. It's not clear there's an industry standard...
    FormattingTestFixture::SetLocale("en-US");
    FormattingTestFixture::SetLocale("de-DE");

    //FormattingDividers fdiv = FormattingDividers("()[]{}");
    //const char *uni = u8"         ЯABГCDE   型号   sautéςερ   τcañón    ";

    //BeFileName bfn = BeFileName("E:\\Bim0200Dev\\out\\Winx64\\Product\\Units - Gtest\\UnitTry.txt", false);
    //Utf8String fnam = Utf8String("E:\\Bim0200Dev\\out\\Winx64\\Product\\Units - Gtest\\UnitTry.txt");
    //FormattingTestData::FileHexDump(fnam);

    if (FormattingTestFixture::OpenTestData())
        {
        LOG.infov("================  Reading Data File ===========================");
        int len = 256;
        //int n = 0;
        Utf8P buf = (Utf8P)alloca(len+2);
        //Utf8P com = (Utf8P)alloca(len + 2);
        bvector<Utf8CP> parts;
        size_t narg = FormattingTestFixture::GetNextArguments(buf, len, &parts, '@');
        int linN = 1000;
        bool keepGoing = true;
        while (FormattingTestFixture::IsDataAvailalbe() && keepGoing)
            {
            switch (narg)
                {
                case 0:
                    LOG.info("Empty");
                    break;
                case 1:
                    LOG.infov("Single: |%s|", parts[0]);
                    if (BeStringUtilities::Stricmp(parts[0], "stop") == 0)
                        {
                        keepGoing = false;
                        LOG.info("!!!!!!!!!!!Processing Stopped by STOP command!!!!!!!!");
                        }
                    break;
                case 2:
                    LOG.infov("Command: |%s| arg: |%s| ", parts[0], parts[1]);
                    if(BeStringUtilities::Stricmp(parts[0], "Pattern") == 0)
                      FormattingTestFixture::SignaturePattrenCollapsing(parts[1], linN++, false);
                    else if (BeStringUtilities::Stricmp(parts[0], "ShowQ") == 0)
                      FormattingTestFixture::ShowQuantityS(parts[1]);
                    break;
                case 3:
                    LOG.infov("Command: |%s| arg: |%s| expect: |%s|", parts[0], parts[1], parts[2]);
                    break;
                default:
                    for (int k = 0; k < narg; k++)
                        {
                        LOG.infov("Arg[%d]: |%s|", k, parts[k]);
                        }
                    break;
                }
            narg = FormattingTestFixture::GetNextArguments(buf, len, &parts, '@');
            }
        LOG.info("================  Data File Processing Complete  ========================");
        }
    else
        LOG.info("Test Data File is not available");

    LOG.info("\n=============== Testing default format ==================");
    LOG.infov("(real) 215.9 = %s", NumericFormatSpec::StdFormatDouble("real", 215.90000000000001).c_str());
    LOG.infov("(DefaultReal) 215.9 = %s", NumericFormatSpec::StdFormatDouble("DefaultReal", 215.90000000000001).c_str());
    LOG.infov("(stop100-2u)98765.4321 = %s", NumericFormatSpec::StdFormatDouble("stop100-2u", 98765.4321).c_str());
    LOG.infov("(stop100-2u)98765 = %s", NumericFormatSpec::StdFormatDouble("stop100-2u", 98765.0).c_str());
    LOG.infov("(stop100-2uz)98765 = %s", NumericFormatSpec::StdFormatDouble("stop100-2uz", 98765.0).c_str());
    LOG.info("=============== Testing default format (end) ==================\n");
    //{\"NumericFormat\":{\"decPrec\":2, \"minWidth\" : 3, \"presentType\" : \"Stop1000\", \"statSeparator\":\"+\", \"TrailZeroes\":\"true\"}, \"SpecAlias\" : \"StationM\", \"SpecName\" : \"StationM\", \"SpecType\" : \"numeric\"} Call with Diego Diaz (diego.diaz@bentley.com) has ended. 7 minutes  

    NumericAccumulator nacc = NumericAccumulator();
    //LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'-')).c_str());
    //LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.SetComplete()).c_str());
    //if(nacc.HasProblem())
    //    LOG.infov("NumAcc problem (%s)", nacc.GetProblemDescription().c_str());
    //else
    //    LOG.infov("NumAcc %d %s  (%s)", nacc.GetByteCount(), nacc.ToText().c_str());


    FormattingTestFixture::ShowQuantity(135.191736, "ARC_DEG", "ARC_DEG", "dms8", "");
    FormattingTestFixture::ShowQuantity(-135.191736, "ARC_DEG", "ARC_DEG", "dms8", "");
    FormattingTestFixture::ShowQuantity(0.256, "ARC_DEG", "ARC_DEG", "dms8", "");
    FormattingTestFixture::ShowQuantity(-0.256, "ARC_DEG", "ARC_DEG", "dms8", "");

    FormattingTestFixture::ShowQuantity(10.0, "M", "FT", "fi8", " ");
    FormattingTestFixture::ShowQuantity(-10.0, "M", "FT", "fi8", " ");
    FormattingTestFixture::ShowQuantity(10.0, "M", "FT", "fi16", "");
    FormattingTestFixture::ShowQuantity(-10.0, "M", "FT", "fi16", "");
    FormattingTestFixture::ShowQuantity(20.0, "M", "FT", "fi8", nullptr);

    LOG.info("Gauges:");
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract16", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract32", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract64", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract128", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract16u", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract32u", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract64u", nullptr);
    FormattingTestFixture::ShowQuantity(1.7859375, "MM", "IN", "fract128u", nullptr);

    FormattingTestFixture::ShowQuantity(419.1, "MM", "FT", "fract8", nullptr);
    FormattingTestFixture::ShowQuantity(419.1, "MM", "FT", "fract16", nullptr);
    FormattingTestFixture::ShowQuantity(3.042, "FT", "FT", "fract16", nullptr);
    FormattingTestFixture::ShowQuantity(3.042, "FT", "FT", "fract32", nullptr);
    LOG.info("End Of Gauges:");

    FormattingTestFixture::TestFUSQuantity(20.0, "M", "FT(real4)", nullptr);
    FormattingTestFixture::TestFUSQuantity(20.0, "M", "FT(real4u)", nullptr);
    FormattingTestFixture::TestFUSQuantity(20.0, "M", "FT(real4)", "_");
    FormattingTestFixture::TestFUSQuantity(20.0, "M", "FT(real4u)", "_");

    FormattingTestFixture::ShowQuantity(98765.4321, "FT", "FT", "stop100-2u", nullptr);
    FormattingTestFixture::ShowQuantity(98765.4321, "FT", "FT", "stop100-2-4u", nullptr);
    FormattingTestFixture::ShowQuantity(98765.4321, "FT", "M", "stop1000-2-4u", nullptr);

    LOG.info("\n========Using Dynamic Formats");

    Utf8CP custJ = "{\"NumericFormat\":{\"decPrec\":2, \"minWidth\" : 4, \"presentType\" : \"Stop100\"}, \
                    \"SpecAlias\" : \"StationFt\", \"SpecName\" : \"StationFt\", \"SpecType\" : \"numeric\"}";

    FormattingTestFixture::CustomFormatAnalyzer(98765.4321, "FT", custJ);

    custJ = "{\"NumericFormat\":{\"decPrec\":2, \"minWidth\" : 4, \"presentType\" : \"Stop100\", \
              \"statSeparator\":\"#\"}, \"SpecAlias\" : \"Station#Ft\", \"SpecName\" : \"Station#Ft\", \"SpecType\" : \"numeric\"}";
    FormattingTestFixture::CustomFormatAnalyzer(98765.4321, "FT", custJ);

    custJ = "{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"'\", \"unitName\" : \"FT\"}}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"CustomFT\", \"SpecName\" : \"CustomFT\", \"SpecType\" : \"composite\"}";
    FormattingTestFixture::CustomFormatAnalyzer(23.5, "FT", custJ);
    FormattingTestFixture::CustomFormatAnalyzer(23.5, "M", custJ);

    custJ = "{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"'\", \"unitName\" : \"FT\"}}, \"NumericFormat\" : {\"fractPrec\":32, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"CustomFT\", \"SpecName\" : \"CustomFT\", \"SpecType\" : \"composite\"}";
    FormattingTestFixture::CustomFormatAnalyzer(23.5, "FT", custJ);
    FormattingTestFixture::CustomFormatAnalyzer(23.5, "M", custJ);

    custJ = "{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"deg\", \"unitName\" : \"ARC_DEG\"}}, \"NumericFormat\" : {\"fractPrec\":16, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"CustomDeg\", \"SpecName\" : \"CustomDeg\", \"SpecType\" : \"composite\"}";
    FormattingTestFixture::CustomFormatAnalyzer(23.5, "ARC_DEG", custJ);

    custJ = "{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}}, \"NumericFormat\" : {\"fractPrec\":16, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"CustomDeg\", \"SpecName\" : \"CustomDeg\", \"SpecType\" : \"composite\"}";
    FormattingTestFixture::CustomFormatAnalyzer(123.25, "ARC_DEG", custJ);

    LOG.info("\n========Using Dynamic Formats================ (end) \n");

    FormattingTestFixture::NumericAccState (&nacc, "-23.45E-03_MM");
    if (nacc.HasProblem())
        LOG.infov("NumAcc problem (%s)", nacc.GetProblemDescription().c_str());
    else
        LOG.infov("NumAcc %d %s", nacc.GetByteCount(), nacc.ToText().c_str(), nacc.GetProblemDescription().c_str());

    EXPECT_TRUE( Utils::IsJsonCandidate("   {bbb}  "));
    EXPECT_FALSE(Utils::IsJsonCandidate("   bbb   "));
    EXPECT_FALSE(Utils::IsJsonCandidate("  {bbb  "));
    EXPECT_FALSE(Utils::IsJsonCandidate(" bbb}  "));
    FormattingTestFixture::TearDownL10N();
    }
#endif
    
//#ifdef _WIN32
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST(FormattingTest, SchemaValidation)
//    {
//    LOG.infov("================  Schema Validation ===========================");
//    FormattingTestFixture::ValidateSchemaUnitNames("C:\\Test\\TestSchema.txt", "UnitName", "C:\\Test\\TestSchemaUnits.txt");
//
//    LOG.infov("================ Schema Validation (end) ===========================");
// 
//    }
//#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, StdFormatting)
    {
    FormattingTestFixture::SetUpL10N();

    Utf8String tes1 = "{AddSynonym:{UnitName:\"FT\", Synonym:\"'\"";
    Utf8String tes2 = tes1;
    LOG.infov("tes1 %s", tes1.c_str());
    LOG.infov("tes2 %s", tes2.c_str());
    tes1.clear();
    LOG.infov("t1-cleared tes2 %s", tes2.c_str());
    NamedFormatSpec nfs1 = NamedFormatSpec();

    Utf8CP scnT = "{\"NumericFormat\":{\"presentType\":\"ScientificNorm\"},\"SpecAlias\":\"sciN\",\"SpecName\":\"NormalizedExp\",\"SpecType\":\"numeric\"}";
    FormattingTestFixture::StandaloneNamedFormatTest(scnT, true);
    scnT = "{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"mile(s)\",\"unitName\":\"MILE\"},\"MiddleUnit\":{\"unitLabel\":\"yrd(s)\",\"unitName\":\"YRD\"},\"MinorUnit\":{\"unitLabel\":\"'\",\"unitName\":\"FT\"},\"SubUnit\":{\"unitLabel\":\"\\\"\",\"unitName\":\"IN\"},\"includeZero\":true},\"NumericFormat\":{\"fractPrec\":4,\"presentType\":\"Fractional\"},\"SpecAlias\":\"myfi4\",\"SpecName\":\"AmerMYFI4\",\"SpecType\":\"composite\"}";
    FormattingTestFixture::StandaloneNamedFormatTest(scnT, true);


    FormattingTestFixture::StdFormattingTest("stop100-2",   1517.23, "15+17.23");
    FormattingTestFixture::StdFormattingTest("stop1000-2",   1517.23, "1+517.23");
    FormattingTestFixture::StdFormattingTest("stop100-2-4", 1517.23, "15+0017.23");
    FormattingTestFixture::StdFormattingTest("stop1000-2-4",1517.23,"1+0517.23");
    FormattingTestFixture::StdFormattingTest("stop100-2-4", 12.23, "0+0012.23");
    FormattingTestFixture::StdFormattingTest("stop100-2-4", 3.17, "0+0003.17");
        
    NumericFormatSpec numFmt = NumericFormatSpec();
    EXPECT_STREQ ("152", numFmt.FormatIntegerToString(152, 0, false).c_str());
    EXPECT_STREQ ("00152", numFmt.FormatIntegerToString(152, 5, false).c_str());
    EXPECT_STREQ ("-0152",  numFmt.FormatIntegerToString(-152, 4, false).c_str());
    EXPECT_STREQ ("0000", numFmt.FormatIntegerToString(0, 4, false).c_str());
    numFmt.SetSignOption(Formatting::ShowSignOption::SignAlways);
    EXPECT_STREQ ("+00152", numFmt.FormatIntegerToString(152, 5, false).c_str());
    FormattingTestFixture::TearDownL10N();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, TestFUS)
    {
    LOG.infov("\n================  Testing FUS (start) ===========================");
    FormattingTestFixture::SetUpL10N();

    FormattingTestFixture::ShowFUS("MM");
    FormattingTestFixture::ShowFUS("IN");
    FormattingTestFixture::ShowFUS("CM");

    FormattingTestFixture::ShowFUS("{\"unitName\":\"MM\"}");
    FormattingTestFixture::ShowFUS("{\"unitName\":\"IN\"}");
    FormattingTestFixture::ShowFUS("{\"unitName\":\"CM\"}");

    //FormattingTestFixture::ShowFUS("MM|fract8");

    FormattingTestFixture::ShowFUS("MM|fract8|");
    FormattingTestFixture::ShowFUS("W/(M*C)|DefaultReal");

    FormattingTestFixture::CrossValidateFUS("MM", "{\"unitName\":\"MM\"}");
    FormattingTestFixture::CrossValidateFUS("IN", "{\"unitName\":\"IN\"}");
    FormattingTestFixture::CrossValidateFUS("CM", "{\"unitName\":\"CM\"}");

    FormattingTestFixture::CrossValidateFUS("MM", "{\"unitName\":\"MM\", \"formatName\":\"real\"}");
    FormattingTestFixture::CrossValidateFUS("IN", "{\"unitName\":\"IN\", \"formatName\":\"real\"}");
    FormattingTestFixture::CrossValidateFUS("CM", "{\"unitName\":\"CM\", \"formatName\":\"real\"}");

    FormattingTestFixture::CrossValidateFUS("{\"unitName\":\"MM\"}", "{\"unitName\":\"MM\", \"formatName\":\"real\"}");
    FormattingTestFixture::CrossValidateFUS("{\"unitName\":\"IN\"}", "{\"unitName\":\"IN\", \"formatName\":\"real\"}");
    FormattingTestFixture::CrossValidateFUS("{\"unitName\":\"CM\"}", "{\"unitName\":\"CM\", \"formatName\":\"real\"}");

    FormattingTestFixture::ShowQuantifiedValue("3' 4\"", "real", "IN");
    FormattingTestFixture::ShowQuantifiedValue("3' 4\"", "realu", "IN", "_");
    FormattingTestFixture::ShowQuantifiedValue("3'4\"", "realu", "IN", "_");
    FormattingTestFixture::ShowQuantifiedValue("3'4 1/8\"", "realu", "IN", "_");
    FormattingTestFixture::ShowQuantifiedValue("3'4 1/8\"", "fract16u", "IN", "_");
    FormattingTestFixture::ShowQuantifiedValue("3'4 1/8\"", "fract16", "IN");
    FormattingTestFixture::ShowQuantifiedValue("3.5 FT", "fract16", "IN");
    FormattingTestFixture::ShowQuantifiedValue("3.6 FT", "real", "IN");
    FormattingTestFixture::ShowQuantifiedValue("3.75 FT", "realu", "IN", "-");
    FormattingTestFixture::ShowQuantifiedValue("3 1/3'", "realu", "IN");
    FormattingTestFixture::ShowQuantifiedValue("3 1/3'", "realu", "M");
    FormattingTestFixture::ShowQuantifiedValue("3 1/3'", "realu", "MM");

    FormattingTestFixture::RegisterFUS("MM", "DefaultMM");

    NamedQuantity nmq = NamedQuantity("PipeLength", 12.45, "FT");
    LOG.infov("Named Qty: %s", nmq.ToText(3).c_str());

   LOG.infov("Named Qty: %s", NumericFormatSpec::StdFormatQuantity("fi32", *nmq.GetQuantity()).c_str());
   
   //BEU::UnitCP useUnit = nullptr, Utf8CP space = "", Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);



    FormattingTestFixture::TearDownL10N();

LOG.infov("================  Testing FUS (end) ===========================\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, FullySpecifiedFUS)
    {
    LOG.infov("\n================  FullySpecifiedFUS (start) ===========================");
    FormattingTestFixture::SetUpL10N();
    
    FormattingTestFixture::StandaloneFUSTest(22.7, "M", "FT", "fract32u", "74 15/32ft");

    FormattingTestFixture::StandaloneFUSTest(22.7, "M", "FT", "real4u", "74.4751ft");
    FormattingTestFixture::StandaloneFUSTest(22.7, "M", "FT", "fi8", "74' 5 3/4\"");
    FormattingTestFixture::StandaloneFUSTest(22.7, "M", "IN", "real", "893.700787");
    FormattingTestFixture::StandaloneFUSTest(22.7, "M", "IN", "", "893.700787");

    FormattingTestFixture::TearDownL10N();
    LOG.infov("================  FullySpecifiedFUS (end) ===========================\n");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, Json)
    {
   // StdFormatSetP stdSet = StdFormatSet::GetStdSet();
    bool stdIdent = StdFormatSet::AreSetsIdentical();
    LOG.infov("Sets are identical %s ", FormatConstant::BoolText(stdIdent));
    UnitProxy prox = UnitProxy("FT", "Feet");
    Json::Value proxV = prox.ToJson();
    LOG.infov("UnitProxy  %s ", proxV.ToString().c_str());
    prox = UnitProxy("FT");
    proxV = prox.ToJson();
    LOG.infov("UnitProxy (no label) %s ", proxV.ToString().c_str());

    FormattingTestFixture::NamedSpecToJson("dms8");
    FormattingTestFixture::NamedSpecToJson(nullptr);
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    NumericFormatSpec nfst100 = NumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex());

    Json::Value nfcJson = nfst100.ToJson(false);
    LOG.infov("JsonNFC %s", nfcJson.ToString().c_str());


    FormattingTestFixture::FormattingTraitsTest();

    NumericFormatSpec jsonTestSpec = NumericFormatSpec();
    FormattingTestFixture::FormattingSpecTraitsTest("default", jsonTestSpec, false);
    FormattingTestFixture::FormattingSpecTraitsTest("default(verbose)", jsonTestSpec, true);

    Json::Value jDeg;
    jDeg[json_degrees()] = 47.5;
    LOG.infov("JsonDeg %s", jDeg.ToString().c_str());

    Json::Value jY;
    jY[json_degrees()] = 30.1234;
    Json::Value jP;
    jP[json_degrees()] = 45.9876;
    Json::Value jR;
    jR[json_degrees()] = 65.786;

  /*  Json::Value val;
    val[json_yaw()] = jY;
    val[json_pitch()] = jP;
    val[json_roll()] = jR;
    LOG.infov("JsonAstro %s", val.ToString().c_str());

    Json::Value jPnt;
    jPnt[0] = 12.0;
    jPnt[1] = 23.0;
    jPnt[2] = 34.0;
    LOG.infov("Dpnt3D %s", jPnt.ToString().c_str());

    double x = jPnt[0].asDouble();
    double y = jPnt[1].asDouble();
    double z = jPnt[2].asDouble();
    LOG.infov("Dpnt3D restored %.2f %.2f %.2f", x, y, z);*/

    LOG.info("================  All Std formats to Json");
    bvector<Utf8CP> stdNames = StdFormatSet::StdFormatNames(true);
    for (int i = 0; i < stdNames.size(); i++)
        {
        FormattingTestFixture::NamedFormatJsonTest(i, stdNames[i], false, "");
        }
    for (int i = 0; i < stdNames.size(); i++)
        {
        FormattingTestFixture::NamedFormatJsonTest(i, stdNames[i], true, "");
        }
    LOG.info("================  All Std formats to Json (end)");

    NumericFormatSpecCP nfsP;
    NamedFormatSpecCP nSpec;
    for (int i = 0; i < stdNames.size(); i++)
        {
        nSpec = StdFormatSet::FindFormatSpec(stdNames[i]);
        nfsP = nSpec->GetNumericSpec();
        FormattingTestFixture::NumericFormatSpecJsonTest(*nfsP);
        }

    FormattingTestFixture::UnitProxyJsonTest("FT", "FEET");
    FormattingTestFixture::UnitProxyJsonTest("FT", "feet");
    FormattingTestFixture::UnitProxyJsonTest("FT", "'");
    FormattingTestFixture::UnitProxyJsonTest("ARC_DEG", "°");
    FormattingTestFixture::UnitProxyJsonTest("ARC_MINUTE", "'");
    FormattingTestFixture::UnitProxyJsonTest("ARC_SECOND", "\"");

    LOG.info("\n============ UnitSynonymMapTest ==================");

    FormattingTestFixture::UnitSynonymMapTest("FT", "feet");
    FormattingTestFixture::UnitSynonymMapTest("ARC_DEG", u8"°");
    FormattingTestFixture::UnitSynonymMapTest("ARC_SECOND", "\"");
    FormattingTestFixture::UnitSynonymMapTest("FT", u8"фут");
    FormattingTestFixture::UnitSynonymMapTest(u8"FT,фут");
    FormattingTestFixture::UnitSynonymMapTest(u8"{\"synonym\":\"фут\", \"unitName\" : \"FT\"}");
    LOG.info("============ UnitSynonymMapTest(end) ==================\n");

    /*bvector<BEU::UnitSynonymMap> mapV;
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "FT", "feet");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "FT", "foot");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "FT", u8"фут");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "FT", "'");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "ARC_DEG", "^");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "ARC_DEG", u8"°");
    BEU::UnitSynonymMap::AugmentUnitSynonymVector(mapV, "ARC_MINUTE", "'");
    Json::Value mapJ = BEU::Phenomenon::SynonymMapVectorToJson(mapV);
    Utf8String mapS = mapJ.ToString();
    LOG.infov("mapVector %s", mapS.c_str());
    bvector<BEU::UnitSynonymMap> mapV2 = BEU::UnitSynonymMap::UnitSynonymMap::MakeUnitSynonymVector(mapJ);
    bool id = BEU::UnitSynonymMap::AreVectorsIdentical(mapV, mapV2);
    LOG.infov("Vectors identical %s", FormatConstant::BoolText(id));
    BEU::UnitRegistry::Instance().LoadSynonyms(mapJ);
    Json::Value allSyn = BEU::UnitRegistry::Instance().SynonymsToJson();
    Utf8String synS = allSyn.ToString();
    LOG.infov("AllSynonyms %s", synS.c_str());*/
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, Pasring)
    {
    LOG.infov("NUNFU = %d", FormatConstant::ParsingPatternCode("NUNFU"));
    LOG.infov("NUNU = %d", FormatConstant::ParsingPatternCode("NUNU"));
    LOG.infov("null = %d", FormatConstant::ParsingPatternCode(nullptr));

    FormattingTestFixture::ParseToQuantity("-23.45E-03_M", 0, "MM");
    FormattingTestFixture::ParseToQuantity("30 1/2 IN", 0, "FT");
    FormattingTestFixture::ParseToQuantity("30 1/2", 0, "IN");
    FormattingTestFixture::ParseToQuantity("3 FT 6 IN", 0, "FT");
    FormattingTestFixture::ParseToQuantity("3 FT 6 IN", 0, "IN");
    FormattingTestFixture::ParseToQuantity("3 FT 6 IN", 0, "CM");
    FormattingTestFixture::ParseToQuantity("3 1/2 FT", 0, "CM");
    FormattingTestFixture::ParseToQuantity("3 1/2FT", 0, "CM");
    FormattingTestFixture::ParseToQuantity("3 1/2_FT", 0, "CM");
    FormattingTestFixture::ParseToQuantity("2/3_FT", 0, "IN");
    FormattingTestFixture::ParseToQuantity("3_FT 1/2IN", 0, "IN");
    FormattingTestFixture::ParseToQuantity(u8"135°11'30 1/4\" ", 0, "ARC_DEG");
    FormattingTestFixture::ParseToQuantity(u8"-135°11'30 1/4\" ", 0, "ARC_DEG");
    FormattingTestFixture::ParseToQuantity(u8"135°11'30 1/4\" ", 0, "RAD");
    FormattingTestFixture::ParseToQuantity("5' 0\"", 0, "FT");
    FormattingTestFixture::ParseToQuantity("0' 3\"", 0, "FT");
    FormattingTestFixture::ParseToQuantity("0' 0\"", 0, "FT");
    FormattingTestFixture::ParseToQuantity("0' 0\"", 0, "FT");
    FormattingTestFixture::ParseToQuantity("3 HR 13 MIN 7 S", 0, "MIN");
    FormattingTestFixture::ParseToQuantity("3 HR 13 MIN 7 S", 0, "S");

    LOG.info("\n============= Equivalence of quantity test =================");

    FormattingTestFixture::ParseToQuantity("5:6", 0, "IN", "fi8");
    FormattingTestFixture::ParseToQuantity("5:", 0, "IN", "fi8");
    FormattingTestFixture::ParseToQuantity(":6", 0, "IN", "fi8");
    FormattingTestFixture::ParseToQuantity("135:23:11", 0, "ARC_DEG", "dms8");
    FormattingTestFixture::ParseToQuantity("3  1/5 FT", 0, "MM", "real");
    FormattingTestFixture::ParseToQuantity("3  1/5 FT", 0, "IN", "real");


    FormattingTestFixture::VerifyQuantity("-23.45E-03_M", "MM", "real", -23.45, "MM");
    FormattingTestFixture::VerifyQuantity("30 1/2 IN", "FT", "real", 2.541667, "FT");
    FormattingTestFixture::VerifyQuantity("30 1/2",    "IN", "real", 30.5, "IN");
    FormattingTestFixture::VerifyQuantity("3 FT 6 IN", "FT", "real", 3.5, "FT");
    FormattingTestFixture::VerifyQuantity("3 FT 6 IN", "IN", "real", 42.0, "IN");
    FormattingTestFixture::VerifyQuantity("3 FT 6 IN", "CM", "real", 106.68, "CM");
    FormattingTestFixture::VerifyQuantity("3 1/2 FT", "CM", "real", 106.68, "CM");
    FormattingTestFixture::VerifyQuantity("3 1/2FT", "CM", "real",  1.0668, "M");
    FormattingTestFixture::VerifyQuantity("3 1/2_FT", "M", "real",  1.0668, "M");
    FormattingTestFixture::VerifyQuantity("2/3_FT", "IN", "real", 8.0, "IN");
    FormattingTestFixture::VerifyQuantity("3_FT 1/2IN", "IN", "real", 36.5, "IN");
    FormattingTestFixture::VerifyQuantity(u8"135°11'30 1/4\" ", "ARC_DEG", "real", 135.191736, "ARC_DEG");
    FormattingTestFixture::VerifyQuantity(u8"135°11'30 1/4\" ", "ARC_DEG", "real", 2.359541, "RAD");
    FormattingTestFixture::VerifyQuantity("5' 0\"", "FT", "real", 5.0, "FT");
    FormattingTestFixture::VerifyQuantity("0' 3\"", "FT", "real", 0.25, "FT");
    FormattingTestFixture::VerifyQuantity("3 HR 13 MIN 7 S", "MIN", "real", 193.116667, "MIN");
    FormattingTestFixture::VerifyQuantity("3 HR 13 MIN 7 S", "MIN", "real", 11587.0, "SEC");

    
    FormattingTestFixture::VerifyQuantity("135:23:11", "ARC_DEG", "dms8", 135.386389, "ARC_DEG");
    FormattingTestFixture::VerifyQuantity("135::", "ARC_DEG", "dms8", 135.0, "ARC_DEG");
    FormattingTestFixture::VerifyQuantity("135:30:", "ARC_DEG", "dms8", 135.5, "ARC_DEG");
    FormattingTestFixture::VerifyQuantity("5:6", "IN", "fi8", 1.6764, "M");
    FormattingTestFixture::VerifyQuantity("5:6", "IN", "fi8", 5.5, "FT");
    FormattingTestFixture::VerifyQuantity("5:", "IN", "fi8", 152.4, "CM");
    FormattingTestFixture::VerifyQuantity(":6", "IN", "fi8", 15.24, "CM");
    FormattingTestFixture::VerifyQuantity("3:13:7", "S", "hms", 11587.0, "S");
    FormattingTestFixture::VerifyQuantity("3:13:7", "S", "hms", 193.116667, "MIN");
    FormattingTestFixture::VerifyQuantity("3 1/5 FT", "IN", "real", 975.36, "MM");
    FormattingTestFixture::VerifyQuantity("3 1/5 FT", "IN", "real", 38.4, "IN");
    FormattingTestFixture::VerifyQuantity("975.36", "MM", "real", 38.4, "IN");
    FormattingTestFixture::VerifyQuantity("1 3/13 IN", "IN", "real", 1.2307692, "IN");
    FormattingTestFixture::VerifyQuantity("1 3/13 IN", "IN", "real", 31.26154, "MM");
    FormattingTestFixture::VerifyQuantity("3/23", "M", "real", 130.4348, "MM");
    FormattingTestFixture::VerifyQuantity("13/113", "M", "real", 115.044, "MM");
    FormattingTestFixture::VerifyQuantity("13/113", "M", "real", 0.37744, "FT");

    FormattingTestFixture::VerifyQuantity(u8"1 3/13 дюйма", "IN", "real", 1.2307692, "IN");
    FormattingTestFixture::VerifyQuantity(u8"3 фута 4 дюйма", "IN", "real", 40.0, "IN");
    FormattingTestFixture::VerifyQuantity(u8"1 фут 1 дюйм", "IN", "real", 13.0, "IN");

    LOG.info("============= Equivalence of quantity test (end) \n=================");

    //FormattingScannerCursor tc = FormattingScannerCursor(u8"ЯA型号   sautéςερ", -1);
    FormattingTestFixture::TestScanPointVector(u8"ЯA型号   sautéςερ135°11'30-1/4\"");
    FormattingTestFixture::TestScanPointVector("-23.45E-03_MM");
    FormattingTestFixture::TestScanPointVector("1+512.15m");
    FormattingTestFixture::TestScanTriplets("1+512.15m");
    FormattingTestFixture::TestScanTriplets("-23.45E-03_MM");

    FormattingTestFixture::TestGrabber("23.451e03_MM");
    Utf8CP tail = FormattingTestFixture::TestGrabber("1+512.15m");
    FormattingTestFixture::TestGrabber(tail, 1);
    FormattingTestFixture::TestGrabber("-23.45E-03_MM");
    FormattingTestFixture::TestGrabber("--A23.45E-03_MM");

    FormattingTestFixture::TestSegments("--A23.45E-03_MM", 0, "MM", "WNU");
    FormattingTestFixture::TestSegments(u8"135°11'30-1/4\" ", 0, "ARC_DEG", "NUNUNFU");
    //01234567890123
    FormattingTestFixture::TestSegments("  -22 FT 3 1/2 IN", 0, "FT","NUNFU");
    FormattingTestFixture::TestSegments("  -22FT 3 1/2IN", 0, "FT","NUNFU");
    FormattingTestFixture::TestSegments("-22' 3 1/2\"", 0, "FT", "NUNFU");

    FormattingTestFixture::TestSegments("12:6:5", 0, "FT", "NCNCN");
    FormattingTestFixture::TestSegments("12::5", 0, "FT","NCCN");
    FormattingTestFixture::TestSegments("12::", 0, "FT", "NCC");
    FormattingTestFixture::TestSegments("12 : 6 : 5", 0, "FT", "NCNCN");
    FormattingTestFixture::TestSegments("12 ::", 0, "FT", "NCC");
    FormattingTestFixture::TestSegments("12 : :", 0, "FT", "NCC");
    FormattingTestFixture::TestSegments(": 12 :", 0, "FT", "CNC");
    FormattingTestFixture::TestSegments(":12:5", 0, "FT", "CNCN");
    FormattingTestFixture::TestSegments(":12 : 5", 0, "FT", "CNCN");
    FormattingTestFixture::TestSegments("::12", 0, "FT", "CCN");
    FormattingTestFixture::TestSegments("-::12", 0, "FT", "-CCN");

    FormattingTestFixture::TestSegments("3:5", 0, "FT", "NCN");
    FormattingTestFixture::TestSegments("3:", 0, "FT", "NC");
    FormattingTestFixture::TestSegments(":5", 0, "FT", "CN");
    FormattingTestFixture::TestSegments("-3:5", 0, "FT", "NCN");
    FormattingTestFixture::TestSegments("-3:", 0, "FT", "NC");
    FormattingTestFixture::TestSegments("-:5", 0, "FT", "-CN");
    FormattingTestFixture::TestSegments("5'-8\"", 0, "FT", "NUNU");

    FormattingTestFixture::ParseToQuantity("5'-8\"", 0, "FT");
    FormattingTestFixture::ParseToQuantity("5'-8\"", 0, "M");
    FormattingTestFixture::ParseToQuantity("5'-8\"", 0, "IN");

    /* LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'2')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'3')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'.')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'4')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'5')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'E')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'-')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'0')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'3')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.AddSymbol((size_t)'_')).c_str());
    LOG.infov("Acc %d state %s", nacc.GetByteCount(), Utils::AccumulatorStateName(nacc.SetComplete()).c_str());*/
    //LOG.infov("NumAcc %d %s  (%s)", nacc.GetByteCount(), nacc.ToText(), nacc.GetProblemDescription());

    FormattingTestFixture::ShowSignature(u8"135°", 200);
    FormattingTestFixture::ShowSignature(u8"135°11'30-1/4\" S", 201);

    FormattingTestFixture::ShowSignature("12:6:5", 210);
    FormattingTestFixture::ShowSignature("12:6:5", 211);
    FormattingTestFixture::ShowSignature("12::5", 212);
    FormattingTestFixture::ShowSignature("12::", 213);

    LOG.info("============= Hex Unicodes =================");
    FormattingTestFixture::ShowHexDump(u8"°K", 10, u8"°K");
    FormattingTestFixture::ShowHexDump(u8"°C", 10, u8"°C");
    FormattingTestFixture::ShowHexDump(u8"°F", 10, u8"°F");
    FormattingTestFixture::ShowHexDump(u8"°R", 10, u8"°R");
    FormattingTestFixture::ShowHexDump(u8"135°11'30-1/4\" S", 30);
    FormattingTestFixture::ShowHexDump(u8"135°11'30-1/4\" S", 30, u8"135°11'30-1/4\" S");
    FormattingTestFixture::ShowHexDump(u8"дюйм",   20, u8"дюйм");  
    FormattingTestFixture::ShowHexDump(u8"дюйма",  20, u8"дюйма");
    FormattingTestFixture::ShowHexDump(u8"дюймов", 20, u8"дюймов");
    FormattingTestFixture::ShowHexDump(u8"фут",   20, u8"фут");
    FormattingTestFixture::ShowHexDump(u8"фута",  20, u8"фута");
    FormattingTestFixture::ShowHexDump(u8"футов", 20, u8"футов");
    FormattingTestFixture::ShowHexDump(u8"135°11'30-1/4\" S", 30);

    FormattingTestFixture::SignaturePattrenCollapsing(u8"         ЯABГCDE   型号   sautéςερ   τcañón    ", 1, true);
    //FormattingTestFixture::SignaturePattrenCollapsing(u8"135°11'30-1/4\" S", 10, true);
    //   012345678912345678901234567901234
    /*FormattingTestFixture::SignaturePattrenCollapsing(u8"135° 11' 30¼\" S", 11, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  135     °     11     ' 30¼\" S ", 12, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  135     °     11     ' 30 ¼\" S ", 13, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  135     °     11     ' 30-¼\" S ", 14, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  135     °     11     ' 30 3/4\" S ", 15, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  135     °     11     ' 30-3/4\" S ", 16, false);
    FormattingTestFixture::SignaturePattrenCollapsing(u8"  -135     °     11     ' 30 3/4\" S ", 17, false);
    FormattingTestFixture::SignaturePattrenCollapsing("   22' 3 1/2\"", 18, false);
    FormattingTestFixture::SignaturePattrenCollapsing("  -22 FT 3 1/2 IN", 19, false);
    FormattingTestFixture::SignaturePattrenCollapsing("  -22 FT 3-1/2 IN", 20, false);
    FormattingTestFixture::SignaturePattrenCollapsing("  -22 FT 3.5IN", 21, false);
    FormattingTestFixture::SignaturePattrenCollapsing("  15_mm", 22, false);
    FormattingTestFixture::SignaturePattrenCollapsing("125.43 ARC_DEG", 23, false);
    FormattingTestFixture::SignaturePattrenCollapsing("125.43ARC_DEG", 24, false);
    FormattingTestFixture::SignaturePattrenCollapsing("1.3RAD", 24, false);*/
   
    //BEU::UnitCP thUOM = BEU::UnitRegistry::Instance().LookupUnit("TONNE/HR");
    //Utf8CP sysN = (nullptr == thUOM) ? "Unknown System" : thUOM->GetUnitSystem();
    //LOG.infov("TONNE_PER_HR-System  %s", sysN);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, PhysValues)
    {
    FormattingTestFixture::SetUpL10N();

    FormattingDividers fdiv = FormattingDividers("()[]{} ");
    EXPECT_TRUE(fdiv.IsDivider('('));
    EXPECT_TRUE(fdiv.IsDivider(')'));
    EXPECT_TRUE(fdiv.IsDivider('{'));
    EXPECT_TRUE(fdiv.IsDivider(' '));
    EXPECT_FALSE(fdiv.IsDivider('A'));

    FormattingTestFixture::ShowSplitByDividers("sample of [text] separated {by stoppers}", "()[]{}");
    FormattingTestFixture::ShowSplitByDividers(u8"135°11'30-1/4\"", "()[]{} ");

    FormattingTestFixture::TestFUS("MM", "MM(DefaultReal)","MM(real)");
    FormattingTestFixture::TestFUS("MM|fract8", "MM(Fractional8)", "MM(fract8)");
    FormattingTestFixture::TestFUS("MM|fract8|", "MM(Fractional8)", "MM(fract8)");
    FormattingTestFixture::TestFUS("W/(M*C)|DefaultReal", "W/(M*C)(DefaultReal)", "W/(M*C)(real)");
    FormattingTestFixture::TestFUS("W/(M*C)|DefaultReal|", "W/(M*C)(DefaultReal)", "W/(M*C)(real)");
    FormattingTestFixture::TestFUS("W/(M*C)(DefaultReal)", "W/(M*C)(DefaultReal)", "W/(M*C)(real)");
    FormattingTestFixture::TestFUS("W/(M*C)", "W/(M*C)(DefaultReal)", "W/(M*C)(real)");

    FormattingTestFixture::TestFUS("CUB.M(real)", "CUB.M(DefaultReal)", "CUB.M(real)");
    FormattingTestFixture::TestFUS("CUB.FT(real)", "CUB.FT(DefaultReal)", "CUB.FT(real)");
    FormattingTestFixture::TestFUS("W/(M*C)(DefaultReal)", "W/(M*C)(DefaultReal)", "W/(M*C)(real)");

    FormattingTestFixture::TestFUG("KOQgroup", "FT(fract8)  IN(fract8), M(real4), MM(Real2), M(Stop1000-2-4)", 
                                    "KOQgroup FT(Fractional8),IN(Fractional8),M(Real4),MM(Real2),M(Stop1000-2-4)", 
                                    "KOQgroup FT(fract8),IN(fract8),M(real4),MM(real2),M(stop1000-2-4)");

    FormattingTestFixture::ShowFUG("FUG1", "MM, IN, MM, CM");

    // preparing pointers to various Unit definitions used in the following tests
    //  adding practically convenient aliases/synonyms to selected Units

    BEU::UnitCP yrdUOM = BEU::UnitRegistry::Instance().LookupUnit("YRD");
    BEU::UnitRegistry::Instance().AddSynonym("YRD", "YARD");
    BEU::UnitCP yardUOM = BEU::UnitRegistry::Instance().LookupUnit("YARD");
    BEU::UnitRegistry::Instance().AddSynonym("YARD", "YRDS");
    BEU::UnitCP yrdsdUOM = BEU::UnitRegistry::Instance().LookupUnit("YRDS");
  
    BEU::UnitCP ftUOM = BEU::UnitRegistry::Instance().LookupUnit("FT");
    BEU::UnitRegistry::Instance().AddSynonym("FT", "FOOT");
    BEU::UnitRegistry::Instance().AddSynonym("IN", "INCH");
    BEU::UnitCP inUOM = BEU::UnitRegistry::Instance().LookupUnit("IN");
    BEU::UnitCP degUOM = BEU::UnitRegistry::Instance().LookupUnit("ARC_DEG");
    BEU::UnitCP minUOM = BEU::UnitRegistry::Instance().LookupUnit("ARC_MINUTE");
    BEU::UnitCP secUOM = BEU::UnitRegistry::Instance().LookupUnit("ARC_SECOND");
    BEU::UnitCP metrUOM = BEU::UnitRegistry::Instance().LookupUnit("M");
    // creating several quantites of various kinds using two different constructors:
    //  one with the Uint Name and another with the pointer to a Unit definition
    BEU::UnitCP defUom = BEU::UnitRegistry::Instance().GetPlatformLengthUnit();

    LOG.infov("Default Platform Units is %s", defUom->GetName());

    BEU::Quantity const len = BEU::Quantity(22.7, *metrUOM);

    BEU::Quantity ang = BEU::Quantity(135.0 + 23.0 / 120.0, *degUOM);

    QuantityTriadSpec qtr = QuantityTriadSpec(len, yrdUOM, ftUOM, inUOM);

    EXPECT_STREQ ("24 YRD 2 FT 5.7 IN", qtr.FormatQuantTriad(" ", 2).c_str());
    EXPECT_STREQ ("24_YRD 2_FT 5.7_IN", qtr.FormatQuantTriad("_", 2).c_str());
    //StdFormatQuantity(Utf8P stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit, int prec = -1, double round = -1.0);
    EXPECT_STREQ ("74 15/32 ft", NumericFormatSpec::StdFormatQuantity("fractu", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("74 1/2 ft", NumericFormatSpec::StdFormatQuantity("fract16u", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("74 15/32 ft", NumericFormatSpec::StdFormatQuantity("fract32u", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("24 7/8 yd", NumericFormatSpec::StdFormatQuantity("fract8u", len, yardUOM, " ").c_str());
    EXPECT_STREQ ("24 7/8-yd", NumericFormatSpec::StdFormatQuantity("fract8u", len, yrdsdUOM, "-").c_str());

    FormatUnitSet fusYF = FormatUnitSet("FT(fract32u)");
    //LOG.infov("FUS->Q  %s", fusYF.FormatQuantity(len).c_str());
    EXPECT_STREQ ("74 15/32ft", fusYF.FormatQuantity(len, "").c_str());

    QuantityTriadSpec atr = QuantityTriadSpec(ang, degUOM, minUOM, secUOM);
    QuantityTriadSpec atrU = QuantityTriadSpec(ang, degUOM, minUOM, secUOM);
    atr.SetTopUnitLabel("\xC2\xB0");
    atr.SetMidUnitLabel(u8"'");
    atr.SetLowUnitLabel(u8"\"");

    EXPECT_STREQ (u8"135° 11' 30\"", atr.FormatQuantTriad("", 4).c_str());
    EXPECT_STREQ ("135 ARC_DEG 11 ARC_MINUTE 30 ARC_SECOND", atrU.FormatQuantTriad(" ", 4).c_str());
    atrU.SetTopUnitLabel("\xC2\xB0");
    atrU.SetMidUnitLabel(u8"'");
    atrU.SetLowUnitLabel(u8"\"");

    EXPECT_STREQ (u8"135° 11' 30\"", atrU.FormatQuantTriad("", 4).c_str());

    atr.SetTopUnitLabel(u8"°");
    atr.SetMidUnitLabel(u8"'");
    atr.SetLowUnitLabel(u8"\"");

    // Arc Angles
    EXPECT_STREQ (u8"135° 11' 30\"", atr.FormatQuantTriad("", 4).c_str());

    EXPECT_STREQ (u8"135° 11' 30\"", NumericFormatSpec::StdFormatQuantity("AngleDMS", ang).c_str());
    EXPECT_STREQ (u8"135° 11' 30\"", NumericFormatSpec::StdFormatQuantity("dms8", ang).c_str());
    EXPECT_STREQ (u8"135° 11' 30\"", NumericFormatSpec::StdFormatQuantity("cdms8", ang).c_str());

    EXPECT_STREQ (u8"135° 11 1/2'", NumericFormatSpec::StdFormatQuantity("dm8", ang).c_str());
    BEU::Quantity distM = BEU::Quantity(3560.5, *metrUOM);
    EXPECT_STREQ ("2mile(s)373yrd(s) 2' 5 1/4\"", NumericFormatSpec::StdFormatQuantity("myfi4", distM).c_str());

    distM = BEU::Quantity(500.0, *metrUOM);
    EXPECT_STREQ ("546yrd(s) 2' 5\"", NumericFormatSpec::StdFormatQuantity("yfi8", distM).c_str());
    EXPECT_STREQ ("1640' 5\"", NumericFormatSpec::StdFormatQuantity("fi8", distM).c_str());

  
    BEU::Quantity ang90 = BEU::Quantity(89.9999999986, *degUOM);
    LOG.infov("DMS-90 %s", NumericFormatSpec::StdFormatQuantity("AngleDMS", ang90).c_str());

    // Temperature
    Utf8CP ems = FormatConstant::EmptyString();
    EXPECT_STREQ (u8"97.88°F", NumericFormatSpec::StdFormatPhysValue("real4u", 36.6, "CELSIUS", "FAHRENHEIT",  u8"°F", ems).c_str());
    EXPECT_STREQ (u8"212.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", 100, "CELSIUS", "FAHRENHEIT",   u8"°F", ems).c_str());

    EXPECT_STREQ (u8"-40.0°C", NumericFormatSpec::StdFormatPhysValue("real4u", -40.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-35.0°C", NumericFormatSpec::StdFormatPhysValue("real4u", -31.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-30.0°C", NumericFormatSpec::StdFormatPhysValue("real4u", -22.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-25.0°C", NumericFormatSpec::StdFormatPhysValue("real4u", -13.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-20.0°C", NumericFormatSpec::StdFormatPhysValue("real4u",  -4.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-15.0°C", NumericFormatSpec::StdFormatPhysValue("real4u",   5.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-10.0°C", NumericFormatSpec::StdFormatPhysValue("real4u",  14.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());
    EXPECT_STREQ (u8"-5.0°C",  NumericFormatSpec::StdFormatPhysValue("real4u",  23.0, "FAHRENHEIT", "CELSIUS", u8"°C", ems).c_str());

    // using default UOM spacer

    EXPECT_STREQ (u8"97.88 °F", NumericFormatSpec::StdFormatPhysValue("real4u", 36.6, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"212.0 °F", NumericFormatSpec::StdFormatPhysValue("real4u", 100, "CELSIUS", "FAHRENHEIT", u8"°F",  nullptr).c_str());

    EXPECT_STREQ (u8"-40.0 °C", NumericFormatSpec::StdFormatPhysValue("real4u", -40.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-35.0 °C", NumericFormatSpec::StdFormatPhysValue("real4u", -31.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());

    EXPECT_STREQ (u8"-40.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -40.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"-31.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -35.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"-22.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -30.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"-13.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -25.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"-4.0°F",  NumericFormatSpec::StdFormatPhysValue("real4u", -20.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"5.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -15.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"14.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -10.0, "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    EXPECT_STREQ (u8"23.0°F", NumericFormatSpec::StdFormatPhysValue("real4u", -5.0,  "CELSIUS", "FAHRENHEIT", u8"°F", ems).c_str());
    
    EXPECT_STREQ (u8"415.53°R", NumericFormatSpec::StdFormatPhysValue("real4u", -42.3, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"450.27°R", NumericFormatSpec::StdFormatPhysValue("real4u", -23.0, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"468.27°R", NumericFormatSpec::StdFormatPhysValue("real4u", -13.0, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"481.77°R", NumericFormatSpec::StdFormatPhysValue("real4u", -5.5,  "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"491.67°R", NumericFormatSpec::StdFormatPhysValue("real4u", 0.0, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"498.996°R", NumericFormatSpec::StdFormatPhysValue("real4u", 4.07, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"524.07°R", NumericFormatSpec::StdFormatPhysValue("real4u", 18.0, "CELSIUS", "RANKINE", u8"°R", ems).c_str());
    EXPECT_STREQ (u8"557.604°R", NumericFormatSpec::StdFormatPhysValue("real4u", 36.63, "CELSIUS", "RANKINE", u8"°R", ems).c_str());

    // Velocity
    EXPECT_STREQ ("10.0 m/s", NumericFormatSpec::StdFormatPhysValue("real4u", 36.0, "KM/HR", "M/SEC", " m/s", ems).c_str());
    EXPECT_STREQ ("2905.76 cm/s", NumericFormatSpec::StdFormatPhysValue("real4u", 65.0, "MPH", "CM/SEC", " cm/s", ems).c_str());
    EXPECT_STREQ ("40.3891 mph", NumericFormatSpec::StdFormatPhysValue("real4u", 65.0, "KM/HR", "MPH", " mph", ems).c_str()); 

    // Volumes
    EXPECT_STREQ ("405.0 CFT", NumericFormatSpec::StdFormatPhysValue("real4u", 15.0, "CUB.YRD", "CUB.FT", "CFT", " ").c_str());

    EXPECT_STREQ (u8"11.4683 m³", NumericFormatSpec::StdFormatPhysValue("real4u", 15.0, "CUB.YRD", "CUB.M", nullptr, " ").c_str());
    EXPECT_STREQ ("2058.0148 L", NumericFormatSpec::StdFormatPhysValue("real4u", 543.6700, "GALLON", "LITRE", "L", " ").c_str());

    // Areas
    EXPECT_STREQ ("327.7853 acres", NumericFormatSpec::StdFormatPhysValue("real4u", 132.65, "HECTARE", "ACRE", nullptr, " ").c_str());

    // Pressure
    EXPECT_STREQ ("99.974 KP", NumericFormatSpec::StdFormatPhysValue("real4u", 14.5, "PSI", "KILOPASCAL", "KP", " ").c_str());
    EXPECT_STREQ (u8"6701.3526 lbf/in²", NumericFormatSpec::StdFormatPhysValue("real4u", 456.0, "ATM", "PSI", nullptr, " ").c_str());

    // mass
    EXPECT_STREQ ("115.3485 kg", NumericFormatSpec::StdFormatPhysValue("real4u", 254.3, "LBM", "KG", nullptr, " ").c_str());
    EXPECT_STREQ ("35.274 oz", NumericFormatSpec::StdFormatPhysValue("real4u", 1.0, "KG", "OZM", nullptr, " ").c_str());
    EXPECT_STREQ ("35.273962_oz", NumericFormatSpec::StdFormatPhysValue("realu", 1.0, "KG", "OZM", nullptr, "_").c_str());
    EXPECT_STREQ ("1.0 kg", NumericFormatSpec::StdFormatPhysValue("realu", 35.27396194958, "OZM", "KG", nullptr, " ").c_str());
    EXPECT_STREQ ("4068.7986 oz", NumericFormatSpec::StdFormatPhysValue("real4u", 115.3485, "KG", "OZM", nullptr, " ").c_str());

    //CompositeValueSpec cvs = CompositeValueSpec("MILE", "YRD", "FOOT", "INCH");
    FormattingTestFixture::TearDownL10N();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, Simple)
    {
    double testV = 1000.0 * sqrt(2.0);
    double fval = sqrt(2.0);

    EXPECT_STREQ ("1 27/64", NumericFormatSpec::StdFormatDouble("fract", fval).c_str());
    EXPECT_STREQ ("+1 27/64", NumericFormatSpec::StdFormatDouble("fractSign", fval).c_str());
    EXPECT_STREQ ("-1 27/64", NumericFormatSpec::StdFormatDouble("fract", -fval).c_str());
    EXPECT_STREQ ("-1 27/64", NumericFormatSpec::StdFormatDouble("fractSign", -fval).c_str());
    fval  *= 3.5;

    EXPECT_STREQ ("4 61/64", NumericFormatSpec::StdFormatDouble("fract", fval).c_str());
    EXPECT_STREQ ("+4 61/64", NumericFormatSpec::StdFormatDouble("fractSign", fval).c_str());
    EXPECT_STREQ ("-4 61/64", NumericFormatSpec::StdFormatDouble("fract", -fval).c_str());
    EXPECT_STREQ ("-4 61/64", NumericFormatSpec::StdFormatDouble("fractSign", -fval).c_str());
    EXPECT_STREQ ("15 7/8", FractionalNumeric(15.0 + 14.0 / 16.0, 256).ToTextDefault(true).c_str());

    EXPECT_STREQ ("1414.213562", NumericFormatSpec::StdFormatDouble("real", testV).c_str());
    EXPECT_STREQ ("1414.21356237", NumericFormatSpec::StdFormatDouble("real", testV, 8).c_str());
    EXPECT_STREQ ("1414.2135624", NumericFormatSpec::StdFormatDouble("real", testV, 7).c_str());
    EXPECT_STREQ ("1414.213562", NumericFormatSpec::StdFormatDouble("real", testV, 6).c_str());
    EXPECT_STREQ ("1414.21356", NumericFormatSpec::StdFormatDouble("real", testV, 5).c_str());
    EXPECT_STREQ ("1414.2136", NumericFormatSpec::StdFormatDouble("real", testV, 4).c_str());
    EXPECT_STREQ ("1414.214", NumericFormatSpec::StdFormatDouble("real", testV, 3).c_str());
    EXPECT_STREQ ("1414.21", NumericFormatSpec::StdFormatDouble("real", testV, 2).c_str());
    EXPECT_STREQ ("1414.2", NumericFormatSpec::StdFormatDouble("real", testV, 1).c_str());
    EXPECT_STREQ ("1414.0", NumericFormatSpec::StdFormatDouble("real", testV, 0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 8, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 7, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 6, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 5, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 4, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble("real", testV, 3, 5.0).c_str());
    EXPECT_STREQ ("1414.2", NumericFormatSpec::StdFormatDouble("real", testV, 8, 0.05).c_str());
    EXPECT_STREQ ("7071.05", NumericFormatSpec::StdFormatDouble("real", 5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("4242.65", NumericFormatSpec::StdFormatDouble("real", 3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("9899.5", NumericFormatSpec::StdFormatDouble("real", 7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("12727.9", NumericFormatSpec::StdFormatDouble("real", 9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("2828.45", NumericFormatSpec::StdFormatDouble("real", 2.0*testV, 3, 0.05).c_str());
    EXPECT_STREQ ("2.82843e+3", NumericFormatSpec::StdFormatDouble("sci", 2.0*testV, 5).c_str());
    EXPECT_STREQ ("0.28284e+4", NumericFormatSpec::StdFormatDouble("sciN", 2.0*testV, 5).c_str());

    //NumericFormatSpec fmtP = (NumericFormatSpecP)StdFormatSet::GetNumericFormat(FormatConstant::DefaultFormatAlias());
    NumericFormatSpec fmtP = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
    fmtP.SetKeepTrailingZeroes(true);
    fmtP.SetUse1000Separator(true);

    EXPECT_STREQ ("1,414.20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("7,071.0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("4,242.650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("9,899.50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("12,727.9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("2,828.450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Testing European ==================");
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean());

    EXPECT_STREQ("1.414,20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ("7.071,0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ("4.242,650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ("9.899,50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ("12.727,9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ("2.828,450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Testing German ==================");
    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultEuropean(true));

    EXPECT_STREQ("1 414,20000000", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ("7 071,0500000", fmtP.FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ("4 242,650000", fmtP.FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ("9 899,50000", fmtP.FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ("12 727,9000", fmtP.FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ("2 828,450", fmtP.FormatDouble(2.0*testV, 3, 0.05).c_str());

    LOG.info("\n=============== Switching back to American ==================\n");

    fmtP.ImbueLocaleProperties(LocaleProperties::DefaultAmerican());


    fmtP.SetKeepTrailingZeroes(false);
    fmtP.SetUse1000Separator(false);

    EXPECT_STREQ ("1414.2", fmtP.FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("-7071.05", fmtP.FormatDouble(-5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("-4242.65", fmtP.FormatDouble(-3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("-9899.5", fmtP.FormatDouble(-7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("-12727.9", fmtP.FormatDouble(-9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("-2828.45", fmtP.FormatDouble(-2.0*testV, 3, 0.05).c_str());

    FormatDictionary fd = FormatDictionary();
    NumericFormatSpec numFmt = NumericFormatSpec();
    numFmt.ImbueLocale("en-US");
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::SignAlways);
    EXPECT_STREQ ("+135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("+135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::NoSign);
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("846356", numFmt.FormatInteger(-846356).c_str());

    numFmt.SetDecimalPrecision(DecimalPrecision::Precision10);
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);

    double dval1 = 123.0004567;
    EXPECT_STREQ ("123.0004567", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-123.0004567", numFmt.FormatDouble(-dval1).c_str());
    dval1 = 0.000012345;
    EXPECT_STREQ ("0.000012345", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("0.00001235", numFmt.FormatDouble(dval1, 8).c_str());
    EXPECT_STREQ ("-0.000012345", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetKeepTrailingZeroes(true);
    EXPECT_STREQ ("0.0000123450", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-0.0000123450", numFmt.FormatDouble(-dval1).c_str());

    dval1 = 3456.0;

    numFmt.SetKeepTrailingZeroes(true);
    numFmt.SetKeepSingleZero(true);
    numFmt.SetKeepDecimalPoint(true);

    double fract, ingr, dnum;
    dnum = -0.003415;
    fract = modf(dnum, &ingr);

    double valR[4] = { 31415.9265359, 314.159265359, 3.14159265359, 314159.265359 };
    double rnd[9] = { 0.0, 1.0, 0.5, 0.01, 0.001, 10.0, 100.0, 1.0 / 3.0, 0.25 };
    double resultRnd[36] = { 31415.926536, 31416.0, 31416.0, 31415.930, 31415.927, 31420.0, 31400.0, 31416.0, 31416.0, 
                             314.159265, 314.0, 314.00, 314.160, 314.1590, 310.0, 300.0, 314.0, 314.250, 3.141593, 3.0, 3.0, 
                             3.140, 3.142, 0.0, 0.0, 3.0, 3.250, 314159.265359, 314159.0, 314159.50, 314159.270,
                             314159.265, 314160.0, 314200.0, 314159.333333, 314159.25 };
    for (int n = 0; n < 4; n++)
        {
        for (int m = 0; m < 9; m++)
            {
            EXPECT_NEAR(resultRnd[n * 9 + m], NumericFormatSpec::RoundDouble(valR[n], rnd[m]), 0.00001);
            }
        }
   
    EXPECT_STREQ ("3456.0000000000", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-3456.0000000000", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetDecimalPrecision(DecimalPrecision::Precision4);
    EXPECT_STREQ ("3456.0000", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-3456.0000", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetKeepTrailingZeroes(false);
    EXPECT_STREQ ("3456.0", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-3456.0", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetKeepSingleZero(false);
    EXPECT_STREQ ("3456.", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-3456.", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetKeepDecimalPoint(false);
    EXPECT_STREQ ("3456", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-3456", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetSignOption(ShowSignOption::NegativeParentheses);
    EXPECT_STREQ ("3456", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("(3456)", numFmt.FormatDouble(-dval1).c_str());

    numFmt.SetSignOption(ShowSignOption::OnlyNegative);
    numFmt.SetDecimalPrecision(DecimalPrecision::Precision10);
    numFmt.SetPresentationType(PresentationType::Scientific);
    EXPECT_STREQ ("-0.2718281828e-2", numFmt.FormatDouble(-0.0027182818284590).c_str());
    numFmt.SetExponentZero(true);
    EXPECT_STREQ ("-0.2718281828e-02", numFmt.FormatDouble(-0.0027182818284590).c_str());

    EXPECT_STREQ ("-0.2718281828", numFmt.FormatDouble(-0.2718281828459045).c_str());
    numFmt.SetPresentationType(PresentationType::ScientificNorm);
    EXPECT_STREQ ("-2.7182818285e-03", numFmt.FormatDouble(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-2.7182818285e-01", numFmt.FormatDouble(-0.2718281828459045).c_str());
    EXPECT_STREQ ("-0.2718281828e+04", numFmt.FormatDouble(-2718.2818284590).c_str());
    EXPECT_STREQ ("0.2718281828e+04", numFmt.FormatDouble(2718.2818284590).c_str());

    EXPECT_STREQ ("01000001", numFmt.ByteToBinaryText('A').c_str());
    EXPECT_STREQ ("01100110", numFmt.ByteToBinaryText('f').c_str());
    numFmt.SetThousandSeparator(' ');
    numFmt.SetUse1000Separator(true);
    EXPECT_STREQ ("00000001 00000011", numFmt.ShortToBinaryText(259, true).c_str());
    numFmt.SetThousandSeparator('.');
    EXPECT_STREQ ("00000001.00000011", numFmt.ShortToBinaryText(259, true).c_str());
    EXPECT_STREQ ("11111111.11111111.11111111.11111111", numFmt.IntToBinaryText(-1, true).c_str());
    EXPECT_STREQ ("00000000.00000001.00000001.00000011", numFmt.IntToBinaryText(65536 + 259, true).c_str());
    EXPECT_STREQ ("00000001.00000001.00000001.00000011", numFmt.IntToBinaryText(65536 * 257 + 259, true).c_str());
    EXPECT_STREQ ("00000010.00000001.00000001.00000011", numFmt.IntToBinaryText(65536 * 513 + 259, true).c_str());

    EXPECT_STREQ ("11111111111111111111111111111111", numFmt.IntToBinaryText(-1, false).c_str());
    EXPECT_STREQ ("00000000000000010000000100000011", numFmt.IntToBinaryText(65536 + 259, false).c_str());
    EXPECT_STREQ ("00000001000000010000000100000011", numFmt.IntToBinaryText(65536 * 257 + 259, false).c_str());
    EXPECT_STREQ ("00000010000000010000000100000011", numFmt.IntToBinaryText(65536 * 513 + 259, false).c_str());

    EXPECT_STREQ ("00111111.11110000.00000000.00000000.00000000.00000000.00000000.00000000", numFmt.DoubleToBinaryText(1.0, true).c_str());
    EXPECT_STREQ ("10111111.11100000.00000000.00000000.00000000.00000000.00000000.00000000", numFmt.DoubleToBinaryText(-0.5, true).c_str());

    FormatParameterP fp, fp1, fp2;
    int count = fd.GetCount();

    for (int i = 0; i < count; i++)
        {
        fp = fd.GetParameterByIndex(i);
        fp1 = fd.FindParameterByCode(fp->GetParameterCode());
        fp2 = fd.FindParameterByName(fp->GetName());
        EXPECT_EQ(fp, fp1);
        EXPECT_EQ(fp, fp2);
        }

    const char *uni = u8"ЯABГCDE型号sautéςερτcañón";  // (char*)mem;
    FormattingScannerCursor curs = FormattingScannerCursor(uni, -1);   // just a core scanner

    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(FormatConstant::UTF_2ByteMask()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(FormatConstant::UTF_3ByteMask()).c_str());
    EXPECT_STREQ ("11111000", numFmt.ByteToBinaryText(FormatConstant::UTF_4ByteMask()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(FormatConstant::UTF_2ByteMark()).c_str());
    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(FormatConstant::UTF_3ByteMark()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(FormatConstant::UTF_4ByteMark()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(FormatConstant::UTF_TrailingByteMask()).c_str());
    EXPECT_STREQ ("10000000", numFmt.ByteToBinaryText(FormatConstant::UTF_TrailingByteMark()).c_str());
    EXPECT_STREQ ("00111111", numFmt.ByteToBinaryText(FormatConstant::UTF_TrailingBitsMask()).c_str());


    for (char c = 'A'; c < 'z'; c++)
        {
        EXPECT_EQ(1, FormatConstant::GetSequenceLength(c));
        }


    EXPECT_EQ(2, FormatConstant::GetSequenceLength(uni[0]));
    EXPECT_TRUE(FormatConstant::IsTrailingByteValid(uni[1]));

    curs.Rewind();

    NumericTriad tr = NumericTriad(1000.0, (size_t)3, (size_t)12);

    EXPECT_STREQ ("27_YD 2_FT 4_IN", tr.FormatTriad((Utf8CP)"YD", (Utf8CP)"FT", (Utf8CP)"IN", "_", 2).c_str());
    EXPECT_STREQ ("27 YD 2 FT 4 IN", tr.FormatTriad((Utf8CP)"YD", (Utf8CP)"FT", (Utf8CP)"IN", " ", 2).c_str());
    EXPECT_STREQ ("27-Yard 2-Feet 4-Inch", tr.FormatTriad((Utf8CP)"Yard", (Utf8CP)"Feet", (Utf8CP)"Inch", "-", 2).c_str());
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, LargeNumbers)
    {
    LOG.info("\n=============== Testing Large Numbers ==================");
    FormattingTestFixture::FormatDoubleTest(1.0, "real4");
    FormattingTestFixture::FormatDoubleTest(1000.0, "real4");
    FormattingTestFixture::FormatDoubleTest(1234567.0, "real4");
    FormattingTestFixture::FormatDoubleTest(1234567891.0, "real4");
    FormattingTestFixture::FormatDoubleTest(1234567891.0e+3, "real4");
    FormattingTestFixture::FormatDoubleTest(1234567891.0e+6, "real4");
    FormattingTestFixture::FormatDoubleTest(1234567891.0e+9, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+15, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+18, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+21, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+22, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+23, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+24, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+25, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+26, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+29, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+31, "real4");
    FormattingTestFixture::FormatDoubleTest(-3.0479999999999998e+35, "real4");
    LOG.info("\n=============== Testing Large Numbers (end) ==================\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, DictionaryValidation)
    {
    FormatDictionary dict = FormatDictionary();
    LOG.infov("%s  %s", FormatConstant::FPN_Precision4().c_str(), dict.CodeToName(ParameterCode::DecPrec4).c_str());

    EXPECT_STREQ(FormatConstant::FPN_NoSign().c_str(), dict.CodeToName(ParameterCode::NoSign).c_str());
    EXPECT_STREQ(FormatConstant::FPN_OnlyNegative().c_str(), dict.CodeToName(ParameterCode::OnlyNegative).c_str());
    EXPECT_STREQ(FormatConstant::FPN_SignAlways().c_str(), dict.CodeToName(ParameterCode::SignAlways).c_str());
    EXPECT_STREQ(FormatConstant::FPN_NegativeParenths().c_str(), dict.CodeToName(ParameterCode::NegativeParenths).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Decimal().c_str(), dict.CodeToName(ParameterCode::Decimal).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Fractional().c_str(), dict.CodeToName(ParameterCode::Fractional).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Scientific().c_str(), dict.CodeToName(ParameterCode::Scientific).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ScientificNorm().c_str(), dict.CodeToName(ParameterCode::ScientificNorm).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Binary().c_str(), dict.CodeToName(ParameterCode::Binary).c_str());
    EXPECT_STREQ(FormatConstant::FPN_DefaultTraits().c_str(), dict.CodeToName(ParameterCode::DefaultZeroes).c_str());
    EXPECT_STREQ(FormatConstant::FPN_LeadingZeroes().c_str(), dict.CodeToName(ParameterCode::LeadingZeroes).c_str());
    EXPECT_STREQ(FormatConstant::FPN_TrailingZeroes().c_str(), dict.CodeToName(ParameterCode::TrailingZeroes).c_str());
    EXPECT_STREQ(FormatConstant::FPN_KeepDecimalPoint().c_str(), dict.CodeToName(ParameterCode::KeepDecimalPoint).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ZeroEmpty().c_str(), dict.CodeToName(ParameterCode::ZeroEmpty).c_str());
    EXPECT_STREQ(FormatConstant::FPN_KeepSingleZero().c_str(), dict.CodeToName(ParameterCode::KeepSingleZero).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ExponentZero().c_str(), dict.CodeToName(ParameterCode::ExponentZero).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision0().c_str(), dict.CodeToName(ParameterCode::DecPrec0).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision1().c_str(), dict.CodeToName(ParameterCode::DecPrec1).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision2().c_str(), dict.CodeToName(ParameterCode::DecPrec2).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision3().c_str(), dict.CodeToName(ParameterCode::DecPrec3).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision4().c_str(), dict.CodeToName(ParameterCode::DecPrec4).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision5().c_str(), dict.CodeToName(ParameterCode::DecPrec5).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision6().c_str(), dict.CodeToName(ParameterCode::DecPrec6).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision7().c_str(), dict.CodeToName(ParameterCode::DecPrec7).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision8().c_str(), dict.CodeToName(ParameterCode::DecPrec8).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision9().c_str(), dict.CodeToName(ParameterCode::DecPrec9).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision10().c_str(), dict.CodeToName(ParameterCode::DecPrec10).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision11().c_str(), dict.CodeToName(ParameterCode::DecPrec11).c_str());
    EXPECT_STREQ(FormatConstant::FPN_Precision12().c_str(), dict.CodeToName(ParameterCode::DecPrec12).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec1().c_str(), dict.CodeToName(ParameterCode::FractPrec1).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec2().c_str(), dict.CodeToName(ParameterCode::FractPrec2).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec4().c_str(), dict.CodeToName(ParameterCode::FractPrec4).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec8().c_str(), dict.CodeToName(ParameterCode::FractPrec8).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec16().c_str(), dict.CodeToName(ParameterCode::FractPrec16).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec32().c_str(), dict.CodeToName(ParameterCode::FractPrec32).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec64().c_str(), dict.CodeToName(ParameterCode::FractPrec64).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec128().c_str(), dict.CodeToName(ParameterCode::FractPrec128).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractPrec256().c_str(), dict.CodeToName(ParameterCode::FractPrec256).c_str());
    EXPECT_STREQ(FormatConstant::FPN_DecimalComma().c_str(), dict.CodeToName(ParameterCode::DecimalComma).c_str());
    EXPECT_STREQ(FormatConstant::FPN_DecimalPoint().c_str(), dict.CodeToName(ParameterCode::DecimalPoint).c_str());
    EXPECT_STREQ(FormatConstant::FPN_DecimalSepar().c_str(), dict.CodeToName(ParameterCode::DecimalSepar).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ThousandSepComma().c_str(), dict.CodeToName(ParameterCode::ThousandSepComma).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ThousandSepPoint().c_str(), dict.CodeToName(ParameterCode::ThousandSepPoint).c_str());
    EXPECT_STREQ(FormatConstant::FPN_ThousandsSepar().c_str(), dict.CodeToName(ParameterCode::ThousandsSepar).c_str());
    EXPECT_STREQ(FormatConstant::FPN_RoundUp().c_str(), dict.CodeToName(ParameterCode::RoundUp).c_str());
    EXPECT_STREQ(FormatConstant::FPN_RoundDown().c_str(), dict.CodeToName(ParameterCode::RoundDown).c_str());
    EXPECT_STREQ(FormatConstant::FPN_RoundToward0().c_str(), dict.CodeToName(ParameterCode::RoundToward0).c_str());
    EXPECT_STREQ(FormatConstant::FPN_RoundAwayFrom0().c_str(), dict.CodeToName(ParameterCode::RoundAwayFrom0).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractBarHoriz().c_str(), dict.CodeToName(ParameterCode::FractBarHoriz).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractBarOblique().c_str(), dict.CodeToName(ParameterCode::FractBarOblique).c_str());
    EXPECT_STREQ(FormatConstant::FPN_FractBarDiagonal().c_str(), dict.CodeToName(ParameterCode::FractBarDiagonal).c_str());
    EXPECT_STREQ(FormatConstant::FPN_AngleRegular().c_str(), dict.CodeToName(ParameterCode::AngleRegular).c_str());
    EXPECT_STREQ(FormatConstant::FPN_AngleDegMin().c_str(), dict.CodeToName(ParameterCode::AngleDegMin).c_str());
    EXPECT_STREQ(FormatConstant::FPN_AngleDegMinSec().c_str(), dict.CodeToName(ParameterCode::AngleDegMinSec).c_str());
    EXPECT_STREQ(FormatConstant::FPN_PaddingSymbol().c_str(), dict.CodeToName(ParameterCode::PaddingSymbol).c_str());
    EXPECT_STREQ(FormatConstant::FPN_CenterAlign().c_str(), dict.CodeToName(ParameterCode::CenterAlign).c_str());
    EXPECT_STREQ(FormatConstant::FPN_LeftAlign().c_str(), dict.CodeToName(ParameterCode::LeftAlign).c_str());
    EXPECT_STREQ(FormatConstant::FPN_RightAlign().c_str(), dict.CodeToName(ParameterCode::RightAlign).c_str());
    EXPECT_STREQ(FormatConstant::FPN_MapName().c_str(), dict.CodeToName(ParameterCode::MapName).c_str());

    bvector<Utf8CP> vec = StdFormatSet::StdFormatNames(true);

    Utf8CP name = *vec.begin();
    Utf8String nameL = StdFormatSet::StdFormatNameList(true);
    NamedFormatSpecCP fmtP;
    Utf8String serT;
    LOG.infov("Aliases:  %s", nameL.c_str());
    nameL = StdFormatSet::CustomNameList(false);
    LOG.infov("Custom:  %s", nameL.c_str());


    nameL = StdFormatSet::StdFormatNameList(false);
    LOG.infov("Names:  %s", nameL.c_str());

    for (auto itr = vec.begin(); itr != vec.end(); ++itr)
        {
        name = *itr;
        fmtP = StdFormatSet::FindFormatSpec(name);
        serT = dict.SerializeFormatDefinition(fmtP);
        LOG.infov("%s", serT.c_str());
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, LocaleTest)
{

    LOG.infov("\n================  Locale Test ===========================");
    
    LocaleProperties lop = LocaleProperties::DefaultAmerican();
    Json::Value jval = lop.ToJson();
    LOG.infov("American Default %s", jval.ToString().c_str());
    LocaleProperties lop1 = LocaleProperties(jval);
    LOG.infov("American Default origin %s", lop.ToText().c_str());
    LOG.infov("American Default restor %s", lop1.ToText().c_str());


    lop = LocaleProperties::DefaultEuropean();
    jval = lop.ToJson();
    LOG.infov("European Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European Default origin %s", lop.ToText().c_str());
    LOG.infov("European Default restor %s", lop1.ToText().c_str());

    lop = LocaleProperties::DefaultEuropean(true);
    jval = lop.ToJson();
    LOG.infov("European1 Default %s", jval.ToString().c_str());
    lop1 = LocaleProperties(jval);
    LOG.infov("European1 Default origin %s", lop.ToText().c_str());
    LOG.infov("European1 Default restor %s", lop1.ToText().c_str());

    LOG.infov("================  Locale Test (end) ===========================\n");


    LOG.infov("\n================  String Decomposition ===========================");


    FormattingTestFixture::DecomposeString("-3.1415926 FT", false);
    FormattingTestFixture::DecomposeString("-3,141,592.6 FT", false);
    FormattingTestFixture::DecomposeString("-3.141.592,6 FT", false);
    FormattingTestFixture::DecomposeString("-3.141.592 2/3 FT", false);

    LOG.infov("================  End of String Decomposition ===========================\n");
    //FormattingTestFixture::TestTime("", "", "%c %a (%A) ");
    //FormattingTestFixture::TestTime("en-US", "US time", "%c %a (%A) day %d of %b (%B)");
    //FormattingTestFixture::TestTime("de-DE", "German time", "%c %a (%A) day %d of %b (%B)");
    //FormattingTestFixture::TestTime("fr", "French time", "%c %a (%A) day %d of %b (%B)");
    //FormattingTestFixture::TestTime("ru", "Russian time", u8"%c %a (%A) день %d месяца %b (%B)");
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                            David.Fox-Rabinovitz                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FormattingTest, PhenomenaTest)
    {

    LOG.infov("\n================  Phenomena Test ===========================");
    FormattingTestFixture::ShowKnownPhenomena();
    LOG.infov("================  Phenomena Test (end) ===========================\n");
    
    LOG.infov("\n================  Synonyms Test ===========================");
    FormattingTestFixture::ShowSynonyms();
    LOG.infov("================  Synonyms Test (end) ===========================\n");

    LOG.infov("================  Formatting Log (completed) ===========================\n\n\n");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                            Kyle.Abramowitz                         04/18
//--------------------------------------------------------------------------------------
TEST(FormattingTest, TestParseUnitFormatDescriptor) 
    {
    FormattingTestFixture::SetUpL10N();
    Utf8String unitName;
    Utf8String formatString;

    Utf8String input = "(N*M)/DEG";
    Utf8String input2 = "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)";
    Utf8String input3 = "(N*M)/DEG(real)";
    Utf8String input4 = "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)(real)";
    Utf8String input5 = "W/(SQ.M*K)";
    Utf8String input6 = "W/(SQ.M*K)(real)";

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input.c_str());
    auto fus = FormatUnitSet(input.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "(N*M)/DEG");
    EXPECT_STREQ("(N*M)/DEG", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input2.c_str());
    fus = FormatUnitSet(input2.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)");
    EXPECT_STREQ("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input3.c_str());
    fus = FormatUnitSet(input3.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "(N*M)/DEG");
    EXPECT_STREQ("(N*M)/DEG", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input4.c_str());
    fus = FormatUnitSet(input4.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)");
    EXPECT_STREQ("(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input5.c_str());
    fus = FormatUnitSet(input5.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "W/(SQ.M*K)");
    EXPECT_STREQ("W/(SQ.M*K)", unitName.c_str());
    EXPECT_TRUE(formatString.empty());

    FormatUnitSet::ParseUnitFormatDescriptor(unitName, formatString, input6.c_str());
    fus = FormatUnitSet(input6.c_str());
    EXPECT_STREQ(fus.GetUnitName().c_str(), "W/(SQ.M*K)");
    EXPECT_STREQ("W/(SQ.M*K)", unitName.c_str());
    EXPECT_STREQ("real", formatString.c_str());
    }

bvector<Utf8String> names = 
    { 
    "DefaultReal",
    "DefaultRealU",
    "Real2",
    "Real3",
    "Real4",
    "Real2U",
    "Real3U",
    "Real4U",
    "Real6U",
    "Real2UNS",
    "Real3UNS",
    "Real4UNS",
    "Real6UNS",
    "Stop100-2u",
    "Stop100-2-4u",
    "Stop1000-2-4u",
    "Stop1000-2u",
    "Stop100-2",
    "Stop100-2-4",
    "Stop1000-2-4",
    "Stop1000-2",
    "SignedReal",
    "ParenthsReal",
    "DefaultFractional",
    "DefaultFractionalU",
    "SignedFractional",
    "DefaultExp",
    "SignedExp",
    "NormalizedExp",
    "DefaultInt",
    "Fractional4",
    "Fractional8",
    "Fractional16",
    "Fractional32",
    "Fractional128",
    "Fractional4U",
    "Fractional8U",
    "Fractional16U",
    "Fractional32U",
    "Fractional128U",
    "AngleDMS",
    "AngleDMS8",
    "AngleDM8",
    "AmerMYFI4",
    "AmerFI8",
    "AmerFI16",
    "AmerFI32",
    "AmerYFI8",
    "Meters4u",
    "Feet4u",
    "Inches4u",
    "Inches18u",
    "DecimalDeg4",
    "StationFt2",
    "StationM4",
    "DefaultReal",
    "DefaultRealU",
    "Real2",
    "Real3",
    "Real4",
    "Real2U",
    "Real3U",
    "Real4U",
    "Real6U",
    "Stop100-2u",
    "Stop100-2uz",
    "Stop100-2-2z",
    "Stop1000-2-3z",
    "Stop100-2-4u",
    "Stop1000-2-4u",
    "Stop1000-2u",
    "Stop100-2",
    "Stop100-2-4",
    "Stop1000-2-4",
    "Stop1000-2",
    "SignedReal",
    "ParenthsReal",
    "DefaultFractional",
    "DefaultFractionalU",
    "SignedFractional",
    "DefaultExp",
    "SignedExp",
    "NormalizedExp",
    "DefaultInt",
    "Fractional4",
    "Fractional8",
    "Fractional16",
    "Fractional32",
    "Fractional64",
    "Fractional128",
    "Fractional4U",
    "Fractional8U",
    "Fractional16U",
    "Fractional64U",
    "Fractional32U",
    "Fractional128U",
    "CAngleDMS",
    "CAngleDMS8",
    "CAngleDM8",
    "AmerMYFI4",
    "HMS"
    };

bvector<Utf8String> aliases = 
    {
    "real",
    "realu",
    "real2",
    "real3",
    "real4",
    "real2u",
    "real3u",
    "real4u",
    "real6u",
    "real2uns",
    "real3uns",
    "real4uns",
    "real6uns",
    "stop100-2u",
    "stop100-2-4u",
    "stop1000-2-4u",
    "stop1000-2u",
    "stop100-2",
    "stop100-2-4",
    "stop1000-2-4",
    "stop1000-2",
    "realSign",
    "realPth",
    "fract",
    "fractu",
    "fractSign",
    "sci",
    "sciSign",
    "sciN",
    "int",
    "fract4",
    "fract8",
    "fract16",
    "fract32",
    "fract128",
    "fract4u",
    "fract8u",
    "fract16u",
    "fract32u",
    "fract128u",
    "dms",
    "dms8",
    "dm8",
    "myfi4",
    "fi8",
    "fi16",
    "fi32",
    "yfi8",
    "meters4u",
    "feet4u",
    "inches4u",
    "Inches18u",
    "decimalDeg4",
    "stationFt2",
    "stationM4",
    "real",
    "realu",
    "real2",
    "real3",
    "real4",
    "real2u",
    "real3u",
    "real4u",
    "real6u",
    "stop100-2u",
    "stop100-2uz",
    "stop100-2-2z",
    "stop1000-2-3z",
    "stop100-2-4u",
    "stop1000-2-4u",
    "stop1000-2u",
    "stop100-2",
    "stop100-2-4",
    "stop1000-2-4",
    "stop1000-2",
    "realSign",
    "realPth",
    "fract",
    "fractu",
    "fractSign",
    "sci",
    "sciSign",
    "sciN",
    "int",
    "fract4",
    "fract8",
    "fract16",
    "fract32",
    "fract64",
    "fract128",
    "fract4u",
    "fract8u",
    "fract16u",
    "fract32u",
    "fract64u",
    "fract128u",
    "cdms",
    "cdms8",
    "cdm8",
    "myfi4",
    "hms",
    };

// Legacy names that are not the only one mapping to a single FormatString.
bvector<Utf8String> onlyNames = 
    {
    "Real",
    "DefaultInt",
    "RealU",
    "Real6U",
    "Fractional64",
    "Fractional64U",
    "CAngleDMS",
    "CAngleDMS8",
    "CAngleDM8"
    };

bool IsNameThatDoesNotMap(Utf8CP tmpName)
    {
    for (auto const& name : onlyNames)
        {
        if (0 == name.CompareToI(tmpName))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST(FormattingTest, AliasFormatStringTest)
    {
    FormattingTestFixture::SetUpL10N();
    for (auto const& name : names)
        {
        auto formatString = LegacyNameMappings::TryGetFormatStringFromLegacyName(name.c_str());
        EXPECT_FALSE(Utf8String::IsNullOrEmpty(formatString)) << "Did not find a FormatString mapped to the LegacyName '" << name.c_str() << "'.";

        // Skip the reverse for name that don't map back from FormatString -> LegacyName
        if (IsNameThatDoesNotMap(name.c_str()))
            continue;

        EXPECT_STREQ(name.c_str(), LegacyNameMappings::TryGetLegacyNameFromFormatString(formatString)) << "The mapped FormatString '" << formatString << "' does not map back to the original name '" << name.c_str() << "'.";
        }
    
    for (auto const& alias : aliases)
        {
        auto name = AliasMappings::TryGetNameFromAlias(alias.c_str());
        EXPECT_FALSE(Utf8String::IsNullOrEmpty(name)) << "Did not find a Name mapped to the Alias '" << alias.c_str() << "'.";
        EXPECT_STREQ(alias.c_str(), AliasMappings::TryGetAliasFromName(name)) << "The mapped name '" << name << "' does not map back to the original alias '" << alias.c_str() << "'.";
        }
    }

END_BENTLEY_FORMATTEST_NAMESPACE
