/*--------------------------------------------------------------------------------------+
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


//#include "UnitsPCH.h"
//#include "StandardNames.h"
#include <Units/UnitRegistry.h>
#include <Units/UnitTypes.h>
#include <Units/Quantity.h>
#include <Units/Units.h>
//#define FORMAT_DEBUG_PRINT

#undef LOG
//#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Format"))
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Format"))
using namespace BentleyApi::Units;
BEGIN_BENTLEY_FORMATTING_NAMESPACE

TEST(FormattingTest, FormattingUOM)
    {
    UnitCP yrdUOM = UnitRegistry::Instance().LookupUnit("YRD");
    UnitRegistry::Instance().AddSynonym("YRD", "YARD");
    UnitCP yardUOM = UnitRegistry::Instance().LookupUnit("YARD");
    UnitRegistry::Instance().AddSynonym("YARD", "YRDS");
    UnitCP yrdsdUOM = UnitRegistry::Instance().LookupUnit("YRDS");
  
    UnitCP ftUOM = UnitRegistry::Instance().LookupUnit("FT");
    UnitCP inUOM = UnitRegistry::Instance().LookupUnit("IN");
    UnitCP degUOM = UnitRegistry::Instance().LookupUnit("ARC_DEG");
    UnitCP minUOM = UnitRegistry::Instance().LookupUnit("ARC_MINUTE");
    UnitCP secUOM = UnitRegistry::Instance().LookupUnit("ARC_SECOND");

    QuantityPtr len = Quantity::Create(22.7, "M");
    QuantityTriadSpec qtr = QuantityTriadSpec(*len, yrdUOM, ftUOM, inUOM);

    EXPECT_STREQ ("24 YRD 2 FT 5.7 IN", qtr.FormatQuantTriad(" ", 2).c_str());
    EXPECT_STREQ ("24_YRD 2_FT 5.7_IN", qtr.FormatQuantTriad("_", 2).c_str());
    EXPECT_STREQ ("74 15/32", NumericFormatSpec::StdFormatQuantity(*len, ftUOM, "fract").c_str());
    EXPECT_STREQ ("74 1/2", NumericFormatSpec::StdFormatQuantity(*len, ftUOM, "fract16").c_str());
    EXPECT_STREQ ("74 15/32", NumericFormatSpec::StdFormatQuantity(*len, ftUOM, "fract32").c_str());
    EXPECT_STREQ ("24 7/8", NumericFormatSpec::StdFormatQuantity(*len, yardUOM, "fract8").c_str());
    EXPECT_STREQ ("24 7/8", NumericFormatSpec::StdFormatQuantity(*len, yrdsdUOM, "fract8").c_str());

    QuantityPtr ang = Quantity::Create(135.0 + 23.0/120.0, "ARC_DEG");

    QuantityTriadSpec atr = QuantityTriadSpec(*ang, degUOM, minUOM, secUOM);
    atr.SetTopUnitLabel("\xC2\xB0");
    atr.SetMidUnitLabel(u8"'");
    atr.SetLowUnitLabel(u8"\"");
    EXPECT_STREQ (u8"135° 11' 30\"", atr.FormatQuantTriad("", 4).c_str());


    //LOG.infov(u8"135.45° = %s  (fract32)", NumericFormatSpec::StdFormatQuantity(*ang, degUOM, "fract").c_str());
    //LOG.infov(u8"135.45° = %s", atr.FormatQuantTriad("_", 4).c_str());
    atr.SetTopUnitLabel(u8"°");
    atr.SetMidUnitLabel(u8"'");
    atr.SetLowUnitLabel(u8"\"");
    //LOG.infov(u8"135.45° = %s", atr.FormatQuantTriad("", 4).c_str());
    EXPECT_STREQ (u8"135° 11' 30\"", atr.FormatQuantTriad("", 4).c_str());

    //LOG.infov(u8"135.45° = %s", NumericFormatSpec::StdFormatQuantityTriad(&atr, "fract32", "").c_str());

    QuantityPtr tmp = Quantity::Create(36.6, "CELSIUS");
    UnitCP farhUOM = UnitRegistry::Instance().LookupUnit("FAHRENHEIT");
    //LOG.infov(u8"36.6°C = %s (real2)", NumericFormatSpec::StdFormatQuantity(*tmp, farhUOM, "real2").c_str());
    
   /* double cels = -40.0;
    while (cels < 250.0)
        {
        LOG.infov(u8"%.2f °C = %s (real4)", cels, NumericFormatSpec::StdFormatPhysValue(cels, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
        cels += 4.0;
        }*/
    /*LOG.infov(u8"100.0°C = %s (real4)", NumericFormatSpec::StdFormatPhysValue(100.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    LOG.infov(u8"0.0°C = %s (real4)", NumericFormatSpec::StdFormatPhysValue(0.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
*/
    EXPECT_STREQ (u8"97.88°F", NumericFormatSpec::StdFormatPhysValue(36.6, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"212.0°F", NumericFormatSpec::StdFormatPhysValue(100, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());

    EXPECT_STREQ (u8"-40.0°C", NumericFormatSpec::StdFormatPhysValue(-40.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-35.0°C", NumericFormatSpec::StdFormatPhysValue(-31.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-30.0°C", NumericFormatSpec::StdFormatPhysValue(-22.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-25.0°C", NumericFormatSpec::StdFormatPhysValue(-13.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-20.0°C", NumericFormatSpec::StdFormatPhysValue(-4.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-15.0°C", NumericFormatSpec::StdFormatPhysValue(5.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-10.0°C", NumericFormatSpec::StdFormatPhysValue(14.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());
    EXPECT_STREQ (u8"-5.0°C", NumericFormatSpec::StdFormatPhysValue(23.0, "FAHRENHEIT", "CELSIUS", u8"°C", "real4").c_str());

    EXPECT_STREQ (u8"-40.0°F", NumericFormatSpec::StdFormatPhysValue(-40.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"-31.0°F", NumericFormatSpec::StdFormatPhysValue(-35.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"-22.0°F", NumericFormatSpec::StdFormatPhysValue(-30.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"-13.0°F", NumericFormatSpec::StdFormatPhysValue(-25.0, "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"-4.0°F",  NumericFormatSpec::StdFormatPhysValue(-20.0,  "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"5.0°F",   NumericFormatSpec::StdFormatPhysValue(-15.0,   "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"14.0°F",  NumericFormatSpec::StdFormatPhysValue(-10.0,  "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());
    EXPECT_STREQ (u8"23.0°F",  NumericFormatSpec::StdFormatPhysValue(-5.0,   "CELSIUS", "FAHRENHEIT", u8"°F", "real4").c_str());



    /*  reg.AddUnit(ANGLE, SI, "ARC_MINUTE", "ARC_DEG", 1.0 / 60.0);
    reg.AddUnit(ANGLE, SI, "ARC_SECOND", "ARC_DEG", 1.0 / 3600.0);*/

#if defined FORMAT_DEBUG_PRINT

    LOG.infov("22.7 M = %s FT", NumericFormat::StdFormatQuantity(*len, ftUOM, "fract32").c_str());
    LOG.infov("22.7 M = %s YRD", NumericFormat::StdFormatQuantity(*len, yardUOM, "fract8").c_str());
    LOG.infov("22.7 M = %s FT(16)", NumericFormat::StdFormatQuantity(*len, ftUOM, "fract16").c_str());
    LOG.infov("22.7 M = %s FT(32)", NumericFormat::StdFormatQuantity(*len, ftUOM, "fract32").c_str());

    UnitCP mileUOM = UnitRegistry::Instance().LookupUnit("MILE");
    if (nullptr != mileUOM && nullptr != yrdUOM)
        {
        double fact = mileUOM->Convert(1.0, yrdUOM);
        LOG.infov("There are %.4f %s in %s", fact, yrdUOM->GetName(), mileUOM->GetName());
        size_t rat = QuantityTriad::UnitRatio(mileUOM, yrdUOM);
        LOG.infov("ratio of  %s/%s is %d", mileUOM->GetName(), yrdUOM->GetName(), rat);
        rat = QuantityTriad::UnitRatio(yrdUOM, mileUOM);
        LOG.infov("ratio of  %s/%s is %d", yrdUOM->GetName(), mileUOM->GetName(), rat);

        rat = QuantityTriad::UnitRatio(yrdUOM, ftUOM);
        LOG.infov("ratio of  %s/%s is %d", yrdUOM->GetName(), ftUOM->GetName(), rat);
        rat = QuantityTriad::UnitRatio(yrdUOM, mileUOM);
        LOG.infov("ratio of  %s/%s is %d", ftUOM->GetName(), yrdUOM->GetName(), rat);

        rat = QuantityTriad::UnitRatio(ftUOM, inUOM);
        LOG.infov("ratio of  %s/%s is %d", ftUOM->GetName(), inUOM->GetName(), rat);
        rat = QuantityTriad::UnitRatio(yrdUOM, mileUOM);
        LOG.infov("ratio of  %s/%s is %d", inUOM->GetName(), ftUOM->GetName(), rat);
        }
    else 
        LOG.info("Units not found");
    if (qtr.IsProblem())
        LOG.info("22.7 M conversion failed");
    else
        LOG.infov("22.7 M = %s", qtr.FormatQuantTriad("_", false).c_str());
#endif
    }

TEST(FormattingTest, Simple)
    {
    double testV = 1000.0 * sqrt(2.0);
    double fval = sqrt(2.0);
    ////  demo print
#if defined FORMAT_DEBUG_PRINT
    LOG.infov("There are %d prime factors", FactorizedNumber::GetPrimeCount());
    int ival = 128;
    LOG.infov("size of int %d ", sizeof(int));
    LOG.infov("size of size_t %d ", sizeof(size_t));
    FactorizedNumber fan = FactorizedNumber(ival);
    LOG.infov("Value %d  (factors) %s ", ival, fan.ToText().c_str());
    LOG.infov("%s", FactorizedNumber(75).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(11925).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(117).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(119).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(113).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(219).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(218).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(211).DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(32*25*9*121).DebugText().c_str());

    FactorizedNumber fz1 = FactorizedNumber(32 * 25 * 9 * 121);
    FactorizedNumber fz2 = FactorizedNumber(8 * 125 * 81 * 11);
    size_t gcf = fz1.GetGreatestCommonFactor(fz2);
    LOG.infov("GreatestCommonFactor of %d and %d = %d", fz1.GetValue(), fz2.GetValue(), gcf);

    LOG.infov("%s", fz1.DebugText().c_str());
    LOG.infov("%s", fz2.DebugText().c_str());
    LOG.infov("%s", FactorizedNumber(gcf).DebugText().c_str());



    FractionalNumeric fn = FractionalNumeric(fval, 4);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());
    fval *= 3.5;

    fn = FractionalNumeric(fval, 16);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 32);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 8);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 3);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 7);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 16);
    LOG.infov("Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fval = 15.0 + (32.0*25.0*3.0) / (64.0*125.0*3.0);
    fn = FractionalNumeric(fval, 256);
    LOG.infov("Fractional Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fval = 15.0 + 14.0 / 16.0;
    fn = FractionalNumeric(fval, 256);
    LOG.infov("Fractional Value %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 2, 0.005);
    LOG.infov("Fractional Value(2) %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 7, 0.005);
    LOG.infov("Fractional Value(7) %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());

    fn = FractionalNumeric(fval, 7, 0.1);
    LOG.infov("Fractional Value(7) %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());
#endif

#if defined FORMAT_DEBUG_PRINT
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real").c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 7).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 6).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 5).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 4).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 3).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 2).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 1).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 0).c_str());

    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8, 5.0).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 7, 5.0).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 6, 5.0).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 5, 5.0).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 4, 5.0).c_str());
    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 3, 5.0).c_str());
    LOG.info("Scientific");
    LOG.infov("Value %.6f  (real) %s ", 2.0*testV, NumericFormat::StdFormatDouble(testV, "sci", 5).c_str());
    LOG.infov("Value %.6f  (real) %s ", 2.0*testV, NumericFormat::StdFormatDouble(testV, "sciN", 5).c_str());

    LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8, 0.05).c_str());
    LOG.infov("Value %.6f  (real) %s ", 5.0*testV, NumericFormat::StdFormatDouble(5.0*testV, "real", 7, 0.05).c_str());
    LOG.infov("Value %.6f  (real) %s ", 3.0*testV, NumericFormat::StdFormatDouble(3.0*testV, "real", 6, 0.05).c_str());
    LOG.infov("Value %.6f  (real) %s ", 7.0*testV, NumericFormat::StdFormatDouble(7.0*testV, "real", 5, 0.05).c_str());
    LOG.infov("Value %.6f  (real) %s ", 9.0*testV, NumericFormat::StdFormatDouble(9.0*testV, "real", 4, 0.05).c_str());
    LOG.infov("Value %.6f  (real) %s ", 2.0*testV, NumericFormat::StdFormatDouble(2.0*testV, "real", 3, 0.05).c_str());
    LOG.info("Scientific");
    LOG.infov("Value %.6f  (real) %s ", 9.0*testV, NumericFormat::StdFormatDouble(9.0*testV, "sci", 5).c_str());
    LOG.infov("Value %.6f  (real) %s ", 9.0*testV, NumericFormat::StdFormatDouble(9.0*testV, "sciN", 5).c_str());
#endif
    /////  end of demo print
        /*Value 1.414214  (denom 4) 1 1 / 2
        Value 4.949747  (denom 16) 4 15 / 16
        Value 4.949747  (denom 32) 4 15 / 16
        Value 4.949747  (denom 8) 5 (0 / 8)
        Value 4.949747  (denom 3) 5 (0 / 3)
        Value 4.949747  (denom 7) 5 (0 / 7)
        Value 4.949747  (denom 16) 4 15 / 16
        Fractional Value 15.100000  (denom 256) 15 13 / 128
        Fractional Value 15.875000  (denom 256) 15 7 / 8
        Fractional Value(2) 15.875000  (denom 256) 15 7 / 8
        Fractional Value(7) 15.875000  (denom 343) 15 300 / 343
        Fractional Value(7) 15.875000  (denom 49) 15 43 / 49*/

    fval = sqrt(2.0);

#if defined FORMAT_DEBUG_PRINT
    fn = FractionalNumeric(fval, 128);
    LOG.infov("FractDirect %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());
    LOG.infov("FractStd %.6f  %s ", fval, NumericFormat::StdFormatDouble(fval, "fract").c_str());
    LOG.infov("FractStd %.6f  %s ", fval, NumericFormat::StdFormatDouble(fval, "fractSign").c_str());
#endif

    EXPECT_STREQ ("1 27/64", NumericFormatSpec::StdFormatDouble(fval, "fract").c_str());
    EXPECT_STREQ ("+1 27/64", NumericFormatSpec::StdFormatDouble(fval, "fractSign").c_str());
    EXPECT_STREQ ("-1 27/64", NumericFormatSpec::StdFormatDouble(-fval, "fract").c_str());
    EXPECT_STREQ ("-1 27/64", NumericFormatSpec::StdFormatDouble(-fval, "fractSign").c_str());
    fval  *= 3.5;

#if defined FORMAT_DEBUG_PRINT
    fn = FractionalNumeric(fval, 128);
    LOG.infov("FractDirect %.6f  (denom %d) %s ", fval, fn.GetDenominator(), fn.ToTextDefault(true).c_str());
    LOG.infov("FractStd %.6f  %s ", fval, NumericFormat::StdFormatDouble(fval, "fract").c_str());
    LOG.infov("FractStd %.6f  %s ", fval, NumericFormat::StdFormatDouble(fval, "fractSign").c_str());
#endif

    EXPECT_STREQ ("4 61/64", NumericFormatSpec::StdFormatDouble(fval, "fract").c_str());
    EXPECT_STREQ ("+4 61/64", NumericFormatSpec::StdFormatDouble(fval, "fractSign").c_str());
    EXPECT_STREQ ("-4 61/64", NumericFormatSpec::StdFormatDouble(-fval, "fract").c_str());
    EXPECT_STREQ ("-4 61/64", NumericFormatSpec::StdFormatDouble(-fval, "fractSign").c_str());
    EXPECT_STREQ ("15 7/8", FractionalNumeric(15.0 + 14.0 / 16.0, 256).ToTextDefault(true).c_str());

    EXPECT_STREQ ("1414.213562", NumericFormatSpec::StdFormatDouble(testV, "real").c_str());
    EXPECT_STREQ ("1414.21356237", NumericFormatSpec::StdFormatDouble(testV, "real", 8).c_str());
    EXPECT_STREQ ("1414.2135624", NumericFormatSpec::StdFormatDouble(testV, "real", 7).c_str());
    EXPECT_STREQ ("1414.213562", NumericFormatSpec::StdFormatDouble(testV, "real", 6).c_str());
    EXPECT_STREQ ("1414.21356", NumericFormatSpec::StdFormatDouble(testV, "real", 5).c_str());
    EXPECT_STREQ ("1414.2136", NumericFormatSpec::StdFormatDouble(testV, "real", 4).c_str());
    EXPECT_STREQ ("1414.214", NumericFormatSpec::StdFormatDouble(testV, "real", 3).c_str());
    EXPECT_STREQ ("1414.21", NumericFormatSpec::StdFormatDouble(testV, "real", 2).c_str());
    EXPECT_STREQ ("1414.2", NumericFormatSpec::StdFormatDouble(testV, "real", 1).c_str());
    EXPECT_STREQ ("1414.0", NumericFormatSpec::StdFormatDouble(testV, "real", 0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 8, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 7, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 6, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 5, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 4, 5.0).c_str());
    EXPECT_STREQ ("1415.0", NumericFormatSpec::StdFormatDouble(testV, "real", 3, 5.0).c_str());
    EXPECT_STREQ ("1414.2", NumericFormatSpec::StdFormatDouble(testV, "real", 8, 0.05).c_str());
    EXPECT_STREQ ("7071.05", NumericFormatSpec::StdFormatDouble(5.0*testV, "real", 7, 0.05).c_str());
    EXPECT_STREQ ("4242.65", NumericFormatSpec::StdFormatDouble(3.0*testV, "real", 6, 0.05).c_str());
    EXPECT_STREQ ("9899.5", NumericFormatSpec::StdFormatDouble(7.0*testV, "real", 5, 0.05).c_str());
    EXPECT_STREQ ("12727.9", NumericFormatSpec::StdFormatDouble(9.0*testV, "real", 4, 0.05).c_str());
    EXPECT_STREQ ("2828.45", NumericFormatSpec::StdFormatDouble(2.0*testV, "real", 3, 0.05).c_str());
    EXPECT_STREQ ("2.82843e+3", NumericFormatSpec::StdFormatDouble(2.0*testV, "sci", 5).c_str());
    EXPECT_STREQ ("0.28284e+4", NumericFormatSpec::StdFormatDouble(2.0*testV, "sciN", 5).c_str());

    NumericFormatSpecP fmtP = StdFormatSet::FindFormat("real");
    fmtP->SetKeepTrailingZeroes(true);
    fmtP->SetUse1000Separator(true);

    EXPECT_STREQ ("1,414.20000000", fmtP->FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("7,071.0500000", fmtP->FormatDouble(5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("4,242.650000", fmtP->FormatDouble(3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("9,899.50000", fmtP->FormatDouble(7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("12,727.9000", fmtP->FormatDouble(9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("2,828.450", fmtP->FormatDouble(2.0*testV, 3, 0.05).c_str());

    ///////////////////////////////////////
#if defined FORMAT_DEBUG_PRINT
    LOG.info("With Separator and trailing zeroes");
    LOG.infov("Value1 %.6f  (real) %s ", testV, fmtP->FormatDouble(testV, 8, 0.05).c_str());
    LOG.infov("Value1 %.6f  (real) %s ", 5.0*testV, fmtP->FormatDouble(5.0*testV, 7, 0.05).c_str());
    LOG.infov("Value1 %.6f  (real) %s ", 3.0*testV, fmtP->FormatDouble(3.0*testV, 6, 0.05).c_str());
    LOG.infov("Value1 %.6f  (real) %s ", 7.0*testV, fmtP->FormatDouble(7.0*testV, 5, 0.05).c_str());
    LOG.infov("Value1 %.6f  (real) %s ", 9.0*testV, fmtP->FormatDouble(9.0*testV, 4, 0.05).c_str());
    LOG.infov("Value1 %.6f  (real) %s ", 2.0*testV, fmtP->FormatDouble(2.0*testV, 3, 0.05).c_str());
#endif
    ///////////////////////////////////////

    fmtP->SetKeepTrailingZeroes(false);
    fmtP->SetUse1000Separator(false);

    EXPECT_STREQ ("1414.2", fmtP->FormatDouble(testV, 8, 0.05).c_str());
    EXPECT_STREQ ("-7071.05", fmtP->FormatDouble(-5.0*testV, 7, 0.05).c_str());
    EXPECT_STREQ ("-4242.65", fmtP->FormatDouble(-3.0*testV, 6, 0.05).c_str());
    EXPECT_STREQ ("-9899.5", fmtP->FormatDouble(-7.0*testV, 5, 0.05).c_str());
    EXPECT_STREQ ("-12727.9", fmtP->FormatDouble(-9.0*testV, 4, 0.05).c_str());
    EXPECT_STREQ ("-2828.45", fmtP->FormatDouble(-2.0*testV, 3, 0.05).c_str());

    ///////////////////////////
#if defined FORMAT_DEBUG_PRINT
    LOG.info("With Separator and trailing zeroes turnes off again");
    LOG.infov("Value2 %.6f  (real) %s ", -testV, fmtP->FormatDouble(testV, 8, 0.05).c_str());
    LOG.infov("Value2 %.6f  (real) %s ", -5.0*testV, fmtP->FormatDouble(-5.0*testV, 7, 0.05).c_str());
    LOG.infov("Value2 %.6f  (real) %s ", -3.0*testV, fmtP->FormatDouble(-3.0*testV, 6, 0.05).c_str());
    LOG.infov("Value2 %.6f  (real) %s ", -7.0*testV, fmtP->FormatDouble(-7.0*testV, 5, 0.05).c_str());
    LOG.infov("Value2 %.6f  (real) %s ", -9.0*testV, fmtP->FormatDouble(-9.0*testV, 4, 0.05).c_str());
    LOG.infov("Value2 %.6f  (real) %s ", -2.0*testV, fmtP->FormatDouble(-2.0*testV, 3, 0.05).c_str());

    ///////////////////////////

    int repet = 1000000;
    double rval = -9.0*testV;
    FormatStopWatch* sw = new FormatStopWatch();
    Utf8String repStr;
    for (int i = 0; i < repet; i++)
        {
        repStr = fmtP->FormatDouble(rval, 4, 0.05);
        }
    LOG.info("Tested fmtP->FormatDouble");
    LOG.infov("Metrics for %s    %s", repStr.c_str(), sw->LastIntervalMetrics(repet).c_str());
    LOG.infov("Elapsed time %s", sw->LastInterval(1.0).c_str());

    for (int i = 0; i < repet; i++)
        {
        repStr = NumericFormat::StdFormatDouble(testV, "real", 8, 0.05).c_str();
        }
    LOG.info("Tested StdFormatDouble");
    LOG.infov("Metrics for %s    %s", repStr.c_str(), sw->LastIntervalMetrics(repet).c_str());
    LOG.infov("Elapsed time %s", sw->LastInterval(1.0).c_str());

    //NumericFormat fmtD = NumericFormat("TestD", PresentationType::Decimal, ShowSignOption::SignAlways, FormatTraits::TrailingZeroes, 8);
    //fmtD.SetKeepTrailingZeroes(true);
#endif

    FormatDictionary fd = FormatDictionary();
    NumericFormatSpec numFmt = NumericFormatSpec("Default");
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

#if defined FORMAT_DEBUG_PRINT
    LOG.infov("fract = %f  ingr = %f  num = %e", fract, ingr, dnum);

    //LOG.infov("Sizeof wchar_T is %d", sizeof(wchar_t));
    LOG.infov("FormatDescr1 = %s", fd.SerializeFormatDefinition(numFmt)->c_str());
    LOG.infov("UseSeparator = %s", numFmt.IsUse1000Separator() ? "true" : "false");
    LOG.infov("Expect: 3456.0000000000   actual %s", numFmt.FormatDouble(dval1).c_str());
#endif
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
    //LOG.infov("Expected -0.2718281828e-02  actual %s", numFmt.FormatDouble(-0.0027182818284590).c_str());


    EXPECT_STREQ ("-0.2718281828", numFmt.FormatDouble(-0.2718281828459045).c_str());
    numFmt.SetPresentationType(PresentationType::ScientificNorm);
    EXPECT_STREQ ("-2.7182818285e-03", numFmt.FormatDouble(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-2.7182818285e-01", numFmt.FormatDouble(-0.2718281828459045).c_str());
    EXPECT_STREQ ("-0.2718281828e+04", numFmt.FormatDouble(-2718.2818284590).c_str());
    EXPECT_STREQ ("0.2718281828e+04", numFmt.FormatDouble(2718.2818284590).c_str());
#if defined FORMAT_DEBUG_PRINT
    LOG.infov("Expected -0.2718281828e+04  actual %s", numFmt.FormatDouble(-2718.2818284590).c_str());
    LOG.infov("Expected  0.2718281828e+04  actual %s", numFmt.FormatDouble(2718.2818284590).c_str());
#endif
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

    //LOG.infov("FormatDescr2 = %s", fd.SerializeFormatDefinition(numFmt)->c_str());

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

#if defined FORMAT_DEBUG_PRINT
    size_t ucode = curs.GetNextSymbol();
    size_t scanned = curs.GetLastScanned();
    while (ucode != 0)
        {
        LOG.infov("Scanned %d chars  unicode %0x   %s", scanned, ucode, curs.IsASCII() ? "ASCII" : "Unicode");
        ucode = curs.GetNextSymbol();
        scanned = curs.GetLastScanned();
        }

    LOG.info("testing Triads");


    LOG.infov("1000.0 INCH = %s", tr.FormatTriad((Utf8CP)"YD", (Utf8CP)"FT", (Utf8CP)"IN", "_", false).c_str());
    LOG.infov("1000.0 INCH = %s", tr.FormatTriad((Utf8CP)"YD", (Utf8CP)"FT", (Utf8CP)"IN", " ", false).c_str());
    LOG.infov("1000.0 INCH = %s", tr.FormatTriad((Utf8CP)"Yard", (Utf8CP)"Feet", (Utf8CP)"Inch", "-", false).c_str());
#endif
    }


TEST(FormattingTest, DictionaryValidation)
    {
    FormatDictionary dict = FormatDictionary();

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

#if defined FORMAT_DEBUG_PRINT
    std::time_t result = std::time(nullptr);
    LOG.infov("Formatting test End %s", std::ctime(&result));
#endif
    }


END_BENTLEY_FORMATTING_NAMESPACE

#ifdef DFR_DEBUG

/*
LOG.infov("source size %d rnd size %d", sizeof(valR) / sizeof(double), sizeof(rnd) / sizeof(double));
for (int n = 0; n < 4; n++)
{
for (int m = 0; m < 9; m++)
{
LOG.infov("Value %.6f rounding %.4f rounded %.6f", valR[n], rnd[m], NumericFormat::RoundDouble(valR[n], rnd[m]));
}
}
*/

LOG.infov("Formatting test Start");
LOG.infov("Testing Double Formats ===================");
double tnum = 123.0004567;
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
tnum = 0.000012345;
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
numFmt.SetKeepTrailingZeroes(true);
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
tnum = 3456.0;
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
numFmt.SetKeepTrailingZeroes(false);
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
numFmt.SetKeepSingleZero(false);
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
numFmt.SetKeepDecimalPoint(false);
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
numFmt.SetSignOption(ShowSignOption::NegativeParentheses);
LOG.infov("Tnum %.10f = %s", tnum, numFmt.FormatDouble(tnum).c_str());
LOG.infov("Tnum %.10f = %s", -tnum, numFmt.FormatDouble(-tnum).c_str());
double dval2 = -0.0027182818284590;
int repet = 1000000;
FormatStopWatch* sw = new FormatStopWatch();
Utf8String repStr;
for (int i = 0; i < repet; i++)
{
    repStr = numFmt.FormatDouble(-tnum);
}
LOG.infov("Metrics for %s    %s", repStr, sw->LastIntervalMetrics(repet));
LOG.infov("Elapsed time %s", sw->LastInterval(1.0));
LOG.infov("End of Testing Double Formats =================");

sw = new FormatStopWatch();
char dbuf[128];
for (int i = 0; i < repet; i++)
{
    //repStr = numFmt.FormatDouble(dval2);
    numFmt.FormatDouble(dval2, dbuf, sizeof(dbuf));
}
LOG.infov("Metrics for %s    %s", dbuf, sw->LastIntervalMetrics(repet));


sw = new FormatStopWatch();
for (int i = 0; i < repet; i++)
{
    sprintf(dbuf, "sprintf %.10e", dval2);
    //repStr = dbuf;
}

LOG.infov("Metrics for %s    %s", dbuf, sw->LastIntervalMetrics(repet));

repet = 100000000; //100 000 000;
                   /*FormatStopWatch* w1 = new FormatStopWatch();
                   double x = 2.0;
                   for (int i = 0; i < repet; i++) { x = sqrt(x);  x = (x + 0.0001)*x;  }*/
                   //Utf8String strEl = w1->LastIntervalMicro();
                   //Utf8String strEl = w1->LastIntervalMetrics(repet);
                   /*const char* t = strEl.c_str();
                   char tt[256];
                   strcpy(tt, t);
                   LOG.infov("Time elapsed %s for %d reps", tt, repet);*/
                   //LOG.infov(strEl.c_str());
                   //LOG.infov("x=%.6f",  x, repet);

for (int i = 0; i < 10; i++)
{
    LOG.infov("[%0d] %0x    BIN:%s", i, uni[i], numFmt.ByteToBinaryText(uni[i]).c_str());
}

size_t ucode = curs.GetNextSymbol();
size_t scanned = curs.GetLastScanned();

while (ucode != 0)
{
    LOG.infov("Scanned %d chars  unicode %0x   %s", scanned, ucode, curs.IsASCII() ? "ASCII" : "Unicode");
    ucode = curs.GetNextSymbol();
    scanned = curs.GetLastScanned();
}

repet = 1000000;
//01234567890123456789012345678901234567890123456789
uni = u8"ЯABГCDE型号sautéςερτcañónЯABГCDE型号sautéςερτcañón";
curs = FormattingScannerCursor(uni, -1);
sw = new FormatStopWatch();
for (int i = 0; i < repet; i++)
{
    curs.Rewind();
    ucode = curs.GetNextSymbol();
    while (ucode != 0)
    {
        ucode = curs.GetNextSymbol();
    }
}
LOG.infov("Processed string %s", sw->LastIntervalMetrics(repet));

//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real"));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 7));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 6));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 5));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 4));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 3));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 2));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 1));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 0));

//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8, 5.0));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 7, 5.0));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 6, 5.0));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 5, 5.0));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 4, 5.0));
//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 3, 5.0));

//LOG.infov("Value %.6f  (real) %s ", testV, NumericFormat::StdFormatDouble(testV, "real", 8, 0.05));
//LOG.infov("Value %.6f  (real) %s ", 5.0*testV, NumericFormat::StdFormatDouble(5.0*testV, "real", 7, 0.05));
//LOG.infov("Value %.6f  (real) %s ", 3.0*testV, NumericFormat::StdFormatDouble(3.0*testV, "real", 6, 0.05));
//LOG.infov("Value %.6f  (real) %s ", 7.0*testV, NumericFormat::StdFormatDouble(7.0*testV, "real", 5, 0.05));
//LOG.infov("Value %.6f  (real) %s ", 9.0*testV, NumericFormat::StdFormatDouble(9.0*testV, "real", 4, 0.05));
//LOG.infov("Value %.6f  (real) %s ", 2.0*testV, NumericFormat::StdFormatDouble(2.0*testV, "real", 3, 0.05));
//LOG.info("With Separator and trailing zeroes");
//LOG.infov("Value1 %.6f  (real) %s ", testV, fmtP->FormatDouble(testV, 8, 0.05));
//LOG.infov("Value1 %.6f  (real) %s ", 5.0*testV, fmtP->FormatDouble(5.0*testV, 7, 0.05));
//LOG.infov("Value1 %.6f  (real) %s ", 3.0*testV, fmtP->FormatDouble(3.0*testV, 6, 0.05));
//LOG.infov("Value1 %.6f  (real) %s ", 7.0*testV, fmtP->FormatDouble(7.0*testV, 5, 0.05));
//LOG.infov("Value1 %.6f  (real) %s ", 9.0*testV, fmtP->FormatDouble(9.0*testV, 4, 0.05));
//LOG.infov("Value1 %.6f  (real) %s ", 2.0*testV, fmtP->FormatDouble(2.0*testV, 3, 0.05));
//LOG.info("With Separator and trailing zeroes turnes off again");
//LOG.infov("Value2 %.6f  (real) %s ", -testV, fmtP->FormatDouble(testV, 8, 0.05));
//LOG.infov("Value2 %.6f  (real) %s ", -5.0*testV, fmtP->FormatDouble(-5.0*testV, 7, 0.05));
//LOG.infov("Value2 %.6f  (real) %s ", -3.0*testV, fmtP->FormatDouble(-3.0*testV, 6, 0.05));
//LOG.infov("Value2 %.6f  (real) %s ", -7.0*testV, fmtP->FormatDouble(-7.0*testV, 5, 0.05));
//LOG.infov("Value2 %.6f  (real) %s ", -9.0*testV, fmtP->FormatDouble(-9.0*testV, 4, 0.05));
//LOG.infov("Value2 %.6f  (real) %s ", -2.0*testV, fmtP->FormatDouble(-2.0*testV, 3, 0.05));
#endif
