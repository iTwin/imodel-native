﻿/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/formatting_test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeTimeUtilities.h>
#include <Formatting/Formatting.h>
#include <Units/UnitRegistry.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/Units.h>

//#define FORMAT_DEBUG_PRINT

//using namespace BentleyApi::Units;
//USING_BENTLEY_FORMATTING;
using namespace BentleyApi::Formatting;
#define BEGIN_BENTLEY_FORMATTEST_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace FormatTests {
#define END_BENTLEY_FORMATTEST_NAMESPACE   } }
#define USING_BENTLEY_FORMATTEST using namespace BENTLEY_NAMESPACE_NAME::FormatTests;
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

#undef LOG
//#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Format"))
#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Format"))

TEST(FormattingTest, Preliminary)
    {
    FormattingDividers fdiv = FormattingDividers("()[]{}");
    const char *uni = u8"         ЯABГCDE型号sautéςερτcañón";
    NumericFormatSpec numFmt = NumericFormatSpec();
    LOG.infov("================  Formatting Log ===========================");
    FormattingScannerCursor curs = FormattingScannerCursor(uni, -1);
      
    EXPECT_TRUE(fdiv.IsDivider('('));
    EXPECT_TRUE(fdiv.IsDivider(')'));
    EXPECT_TRUE(fdiv.IsDivider('{'));
    EXPECT_FALSE(fdiv.IsDivider('A'));

    FormatUnitSet fus1 = FormatUnitSet("CUB.M(real)");
    EXPECT_STREQ ("CUB.M(DefaultReal)", fus1.ToText(false).c_str());
    EXPECT_STREQ ("CUB.M(real)", fus1.ToText(true).c_str());
   
    FormatUnitSet fus = FormatUnitSet("FT(fract8)");
    FormatUnitGroup fusG = FormatUnitGroup("FT(fract8)  IN(fract8), M(real4), MM(Real2)");
   
    EXPECT_STREQ ("FT(fract8),IN(fract8),M(real4),MM(real2)", fusG.ToText(true).c_str());
    EXPECT_STREQ ("FT(Fractional8),IN(Fractional8),M(Real4),MM(Real2)", fusG.ToText(false).c_str());
    }

TEST(FormattingTest, PhysValues)
    {
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


    BEU::Quantity const len = BEU::Quantity(22.7, *metrUOM);

    BEU::Quantity ang = BEU::Quantity(135.0 + 23.0 / 120.0, *degUOM);

    QuantityTriadSpec qtr = QuantityTriadSpec(len, yrdUOM, ftUOM, inUOM);

    EXPECT_STREQ ("24 YRD 2 FT 5.7 IN", qtr.FormatQuantTriad(" ", 2).c_str());
    EXPECT_STREQ ("24_YRD 2_FT 5.7_IN", qtr.FormatQuantTriad("_", 2).c_str());
    //StdFormatQuantity(Utf8P stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit, int prec = -1, double round = -1.0);
    EXPECT_STREQ ("74 15/32 FT", NumericFormatSpec::StdFormatQuantity("fract", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("74 1/2 FT", NumericFormatSpec::StdFormatQuantity("fract16", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("74 15/32 FT", NumericFormatSpec::StdFormatQuantity("fract32", len, ftUOM, " ").c_str());
    EXPECT_STREQ ("24 7/8 YRD", NumericFormatSpec::StdFormatQuantity("fract8", len, yardUOM, " ").c_str());
    EXPECT_STREQ ("24 7/8-YRD", NumericFormatSpec::StdFormatQuantity("fract8", len, yrdsdUOM, "-").c_str());

    FormatUnitSet fusYF = FormatUnitSet("FT(fract32)");
    LOG.infov("FUS->Q  %s", fusYF.FormatQuantity(len).c_str());


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
    EXPECT_STREQ (u8"135° 11 1/2'", NumericFormatSpec::StdFormatQuantity("dm8", ang).c_str());
    BEU::Quantity distM = BEU::Quantity(3560.5, *metrUOM);
    EXPECT_STREQ ("2mile(s)373yrd(s) 2' 5 1/4\"", NumericFormatSpec::StdFormatQuantity("myfi4", distM).c_str());

    distM = BEU::Quantity(500.0, *metrUOM);
    EXPECT_STREQ ("546yrd(s) 2' 5\"", NumericFormatSpec::StdFormatQuantity("yfi8", distM).c_str());
    EXPECT_STREQ ("1640' 5\"", NumericFormatSpec::StdFormatQuantity("fi8", distM).c_str());

    BEU::Quantity ang90 = BEU::Quantity(89.9999999986, *degUOM);
    LOG.infov("DMS-90 %s", NumericFormatSpec::StdFormatQuantity("AngleDMS", ang90).c_str());

    // Temperature
    EXPECT_STREQ (u8"97.88°F", NumericFormatSpec::StdFormatPhysValue("real4", 36.6, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"212.0°F", NumericFormatSpec::StdFormatPhysValue("real4", 100, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());

    EXPECT_STREQ (u8"-40.0°C", NumericFormatSpec::StdFormatPhysValue("real4", -40.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-35.0°C", NumericFormatSpec::StdFormatPhysValue("real4", -31.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-30.0°C", NumericFormatSpec::StdFormatPhysValue("real4", -22.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-25.0°C", NumericFormatSpec::StdFormatPhysValue("real4", -13.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-20.0°C", NumericFormatSpec::StdFormatPhysValue("real4",  -4.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-15.0°C", NumericFormatSpec::StdFormatPhysValue("real4",   5.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-10.0°C", NumericFormatSpec::StdFormatPhysValue("real4",  14.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());
    EXPECT_STREQ (u8"-5.0°C",  NumericFormatSpec::StdFormatPhysValue("real4",  23.0, "FAHRENHEIT", "CELSIUS", u8"°C", nullptr).c_str());

    EXPECT_STREQ (u8"-40.0°F", NumericFormatSpec::StdFormatPhysValue("real4", -40.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"-31.0°F", NumericFormatSpec::StdFormatPhysValue("real4", -35.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"-22.0°F", NumericFormatSpec::StdFormatPhysValue("real4", -30.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"-13.0°F", NumericFormatSpec::StdFormatPhysValue("real4", -25.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"-4.0°F",  NumericFormatSpec::StdFormatPhysValue("real4", -20.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"5.0°F",   NumericFormatSpec::StdFormatPhysValue("real4", -15.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"14.0°F",  NumericFormatSpec::StdFormatPhysValue("real4", -10.0, "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    EXPECT_STREQ (u8"23.0°F",  NumericFormatSpec::StdFormatPhysValue("real4", -5.0,  "CELSIUS", "FAHRENHEIT", u8"°F", nullptr).c_str());
    
    EXPECT_STREQ (u8"415.53°R", NumericFormatSpec::StdFormatPhysValue("real4", -42.3, "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"450.27°R", NumericFormatSpec::StdFormatPhysValue("real4", -23.0, "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"468.27°R", NumericFormatSpec::StdFormatPhysValue("real4", -13.0, "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"481.77°R", NumericFormatSpec::StdFormatPhysValue("real4", -5.5,  "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"491.67°R", NumericFormatSpec::StdFormatPhysValue("real4", 0.0,   "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"498.996°R",NumericFormatSpec::StdFormatPhysValue("real4", 4.07,  "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"524.07°R", NumericFormatSpec::StdFormatPhysValue("real4", 18.0,  "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());
    EXPECT_STREQ (u8"557.604°R",NumericFormatSpec::StdFormatPhysValue("real4", 36.63, "CELSIUS", "RANKINE", u8"°R", nullptr).c_str());

    // Velocity
    EXPECT_STREQ ("10.0 m/s", NumericFormatSpec::StdFormatPhysValue("real4", 36.0, "KM/HR", "M/SEC", " m/s", nullptr).c_str());
    EXPECT_STREQ ("2905.76 cm/s", NumericFormatSpec::StdFormatPhysValue("real4", 65.0, "MPH", "CM/SEC", " cm/s", nullptr).c_str());
    EXPECT_STREQ ("40.3891 mph", NumericFormatSpec::StdFormatPhysValue("real4", 65.0, "KM/HR", "MPH", " mph", nullptr).c_str()); 

    // Volumes
    EXPECT_STREQ ("405.0 CFT", NumericFormatSpec::StdFormatPhysValue("real4", 15.0, "CUB.YRD", "CUB.FT", "CFT", " ").c_str());
    EXPECT_STREQ ("11.4683 CUB.M", NumericFormatSpec::StdFormatPhysValue("real4", 15.0, "CUB.YRD", "CUB.M", nullptr, " ").c_str());
    EXPECT_STREQ ("2058.0148 L", NumericFormatSpec::StdFormatPhysValue("real4", 543.6700, "GALLON", "LITRE", "L", " ").c_str());

    // Areas
    EXPECT_STREQ ("327.7853 ACRE", NumericFormatSpec::StdFormatPhysValue("real4", 132.65, "HECTARE", "ACRE", nullptr, " ").c_str());

    // Pressure
    EXPECT_STREQ ("99.974 KP", NumericFormatSpec::StdFormatPhysValue("real4", 14.5, "PSI", "KILOPASCAL", "KP", " ").c_str());
    EXPECT_STREQ ("6701.3526 PSI", NumericFormatSpec::StdFormatPhysValue("real4", 456.0, "ATM", "PSI", nullptr, " ").c_str());

    // mass
    EXPECT_STREQ ("115.3485 KG", NumericFormatSpec::StdFormatPhysValue("real4", 254.3, "LBM", "KG", nullptr, " ").c_str());
    EXPECT_STREQ ("35.274 OZM", NumericFormatSpec::StdFormatPhysValue("real4", 1.0, "KG", "OZM", nullptr, " ").c_str());
    EXPECT_STREQ ("35.273962_OZM", NumericFormatSpec::StdFormatPhysValue("real", 1.0, "KG", "OZM", nullptr, "_").c_str());
    EXPECT_STREQ ("1.0 KG", NumericFormatSpec::StdFormatPhysValue("real", 35.27396194958, "OZM", "KG", nullptr, " ").c_str());
    EXPECT_STREQ ("4068.7986 OZM", NumericFormatSpec::StdFormatPhysValue("real4", 115.3485, "KG", "OZM", nullptr, " ").c_str());

    //CompositeValueSpec cvs = CompositeValueSpec("MILE", "YRD", "FOOT", "INCH");

  

    }


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

    NumericFormatSpecP fmtP = (NumericFormatSpecP)StdFormatSet::GetNumericFormat("real");
    fmtP->SetKeepTrailingZeroes(true);
    fmtP->SetUse1000Separator(true);

    EXPECT_STREQ ("1,414.20000000", fmtP->FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("7,071.0500000", fmtP->FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("4,242.650000", fmtP->FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("9,899.50000", fmtP->FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("12,727.9000", fmtP->FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("2,828.450", fmtP->FormatDouble(2.0*testV, 3, 0.05).c_str());

    fmtP->SetKeepTrailingZeroes(false);
    fmtP->SetUse1000Separator(false);

    EXPECT_STREQ ("1414.2", fmtP->FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("-7071.05", fmtP->FormatDouble(-5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("-4242.65", fmtP->FormatDouble(-3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("-9899.5", fmtP->FormatDouble(-7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("-12727.9", fmtP->FormatDouble(-9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("-2828.45", fmtP->FormatDouble(-2.0*testV, 3, 0.05).c_str());

    FormatDictionary fd = FormatDictionary();
    NumericFormatSpec numFmt = NumericFormatSpec();
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
    EXPECT_STREQ(FormatConstant::FPN_DefaultZeroes().c_str(), dict.CodeToName(ParameterCode::DefaultZeroes).c_str());
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
    nameL = StdFormatSet::StdFormatNameList(false);
    LOG.infov("Names:  %s", nameL.c_str());

    for (auto itr = vec.begin(); itr != vec.end(); ++itr)
        {
        name = *itr;
        fmtP = StdFormatSet::FindFormatSpec(name);
        serT = dict.SerializeFormatDefinition(fmtP);
        LOG.infov("%s", serT.c_str());
        }

   
   /* NumericFormatSpecP fmtP = StdFormatSet::FindFormat("fract16");

    LOG.infov("%s", serT.c_str());
    fmtP = StdFormatSet::FindFormat("fract32");
    serT = dict.SerializeFormatDefinition(*fmtP);
    LOG.infov("%s", serT.c_str());*/

    LOG.infov("================  Formatting Log (completed) ===========================\n\n\n");
    }


END_BENTLEY_FORMATTEST_NAMESPACE

