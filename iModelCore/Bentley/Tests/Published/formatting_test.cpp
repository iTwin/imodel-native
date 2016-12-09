/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/formatting_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Formatting.h>
#include <Logging/bentleylogging.h>
#include <Bentley/BeTimeUtilities.h>

#undef LOG
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"Format"))

TEST(FormattingTest, Simple)
    {


    FormatDictionary fd = FormatDictionary();
    NumericFormat numFmt = NumericFormat("Default");
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
    EXPECT_STREQ ("-0.000012345", numFmt.FormatDouble(-dval1).c_str());
    numFmt.SetKeepTrailingZeroes(true);
    EXPECT_STREQ ("0.0000123450", numFmt.FormatDouble(dval1).c_str());
    EXPECT_STREQ ("-0.0000123450", numFmt.FormatDouble(-dval1).c_str());

    dval1 = 3456.0;

    numFmt.SetKeepTrailingZeroes(true);
    numFmt.SetKeepSingleZero(true);
    numFmt.SetKeepDecimalPoint(true);

    LOG.infov("FormatDescr1 = %s", fd.SerializeFormatDefinition(numFmt)->c_str());

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
    EXPECT_STREQ ("-0.2718281828", numFmt.FormatDouble(-0.2718281828459045).c_str());
    numFmt.SetPresentationType(PresentationType::ScientificNorm);
    EXPECT_STREQ ("-2.7182818285e-3", numFmt.FormatDouble(-0.0027182818284590).c_str());
    EXPECT_STREQ ("-2.7182818285e-1", numFmt.FormatDouble(-0.2718281828459045).c_str());

    EXPECT_STREQ ("01000001", numFmt.ByteToBinaryText('A').c_str());
    EXPECT_STREQ ("01100110", numFmt.ByteToBinaryText('f').c_str());
    numFmt.SetThousandSeparator(' ');
    numFmt.SetUseSeparator(true);
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

    LOG.infov("FormatDescr2 = %s", fd.SerializeFormatDefinition(numFmt)->c_str());

    const char *uni = u8"ЯABГCDE型号sautéςερτcañón";  // (char*)mem;
    FormattingScannerCursor curs = FormattingScannerCursor(uni, -1);   // just a core scanner

    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants()->Get2ByteMask()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants()->Get3ByteMask()).c_str());
    EXPECT_STREQ ("11111000", numFmt.ByteToBinaryText(curs.GetConstants()->Get4ByteMask()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants()->Get2ByteMark()).c_str());
    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants()->Get3ByteMark()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants()->Get4ByteMark()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingByteMask()).c_str());
    EXPECT_STREQ ("10000000", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingByteMark()).c_str());
    EXPECT_STREQ ("00111111", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingBitsMask()).c_str());


    for (char c = 'A'; c < 'z'; c++)
        {
        EXPECT_EQ(1, curs.GetConstants()->GetSequenceLength(c));
        }


    EXPECT_EQ(2, curs.GetConstants()->GetSequenceLength(uni[0]));
    EXPECT_TRUE(curs.GetConstants()->IsTrailingByteValid(uni[1]));
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



    LOG.infov("Formatting test End");
    }


#ifdef DFR_DEBUG
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
#endif
