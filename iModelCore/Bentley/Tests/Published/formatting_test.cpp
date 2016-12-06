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

//#undef LOG
//#define LOG (LoggingManager::GetLogger(L"Format"))

TEST(FormattingTest, Simple)
{
    LOG.infov("Formatting test Start");
    //ShowSignOption sigOpt = ShowSignOption::OnlyNegative;
    NumericFormat numFmt = NumericFormat();
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);
    //EXPECT_TRUE(false) << "Integer Formatting:  ShowSignOption::OnlyNegative";
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::SignAlways);
    //EXPECT_TRUE(false) << "Integer Formatting:  ShowSignOption::SignAlways";
    EXPECT_STREQ ("+135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("+135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("-846356", numFmt.FormatInteger(-846356).c_str());
    numFmt.SetSignOption(ShowSignOption::NoSign);
    //EXPECT_TRUE(false) << "Integer Formatting:  ShowSignOption::NoSign";
    EXPECT_STREQ ("135", numFmt.FormatInteger(135).c_str());
    EXPECT_STREQ ("135689", numFmt.FormatInteger(135689).c_str());
    EXPECT_STREQ ("846356", numFmt.FormatInteger(-846356).c_str());

    //EXPECT_TRUE(false) << "Double Formatting:  ShowSignOption::OnlyNegative Precision10 Scientific";
    numFmt.SetDecimalPrecision(DecimalPrecision::Precision10);
    numFmt.SetSignOption(ShowSignOption::OnlyNegative);

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

    double dval2 = -0.0027182818284590;
    char dbuf[128];
    sw = new FormatStopWatch();

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


    EXPECT_STREQ ("01000001", numFmt.ByteToBinaryText('A').c_str());
    EXPECT_STREQ ("01100110", numFmt.ByteToBinaryText('f').c_str());
    numFmt.SetThousandSeparator(' ');
    numFmt.SetUseSeparator(true);
    EXPECT_STREQ ("00000001 00000011", numFmt.ShortToBinaryText(259, true).c_str());
    numFmt.SetThousandSeparator('.');
    EXPECT_STREQ ("00000001.00000011", numFmt.ShortToBinaryText(259, true).c_str());
    EXPECT_STREQ ("11111111.11111111.11111111.11111111", numFmt.IntToBinaryText(-1, true).c_str());
    EXPECT_STREQ ("00000000.00000001.00000001.00000011", numFmt.IntToBinaryText(65536+259, true).c_str());
    EXPECT_STREQ ("00000001.00000001.00000001.00000011", numFmt.IntToBinaryText(65536*257 + 259, true).c_str());
    EXPECT_STREQ ("00000010.00000001.00000001.00000011", numFmt.IntToBinaryText(65536 * 513 + 259, true).c_str());

    EXPECT_STREQ ("11111111111111111111111111111111", numFmt.IntToBinaryText(-1, false).c_str());
    EXPECT_STREQ ("00000000000000010000000100000011", numFmt.IntToBinaryText(65536 + 259, false).c_str());
    EXPECT_STREQ ("00000001000000010000000100000011", numFmt.IntToBinaryText(65536 * 257 + 259, false).c_str());
    EXPECT_STREQ ("00000010000000010000000100000011", numFmt.IntToBinaryText(65536 * 513 + 259, false).c_str());

    EXPECT_STREQ ("00111111.11110000.00000000.00000000.00000000.00000000.00000000.00000000", numFmt.DoubleToBinaryText(1.0, true).c_str());
    EXPECT_STREQ ("10111111.11100000.00000000.00000000.00000000.00000000.00000000.00000000", numFmt.DoubleToBinaryText(-0.5, true).c_str());


    FormatDictionary fd = FormatDictionary();
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

    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants()->Get2ByteMask()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants()->Get3ByteMask()).c_str());
    EXPECT_STREQ ("11111000", numFmt.ByteToBinaryText(curs.GetConstants()->Get4ByteMask()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants()->Get2ByteMark()).c_str());
    EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants()->Get3ByteMark()).c_str());
    EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants()->Get4ByteMark()).c_str());

    EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingByteMask()).c_str());
    EXPECT_STREQ ("10000000", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingByteMark()).c_str());
    EXPECT_STREQ ("00111111", numFmt.ByteToBinaryText(curs.GetConstants()->GetTrailingBitsMask()).c_str());
 
    repet = 100000000; //100 000 000;
    FormatStopWatch* w1 = new FormatStopWatch();
    double x = 2.0;
    for (int i = 0; i < repet; i++) { x = sqrt(x);  x = (x + 0.0001)*x;  }
    //Utf8String strEl = w1->LastIntervalMicro();
    Utf8String strEl = w1->LastIntervalMetrics(repet);
    /*const char* t = strEl.c_str();
    char tt[256];
    strcpy(tt, t);
    LOG.infov("Time elapsed %s for %d reps", tt, repet);*/
    LOG.infov(strEl.c_str());
    //LOG.infov("x=%.6f",  x, repet);

    for (char c = 'A'; c < 'z'; c++)
    {
        EXPECT_EQ(1, curs.GetConstants()->GetSequenceLength(c));
    }



    for (int i = 0; i < 10; i++)
    {
        LOG.infov("[%0d] %0x    BIN:%s", i, uni[i], numFmt.ByteToBinaryText(uni[i]).c_str());
    }

    EXPECT_EQ(2, curs.GetConstants()->GetSequenceLength(uni[0]));
    EXPECT_TRUE(curs.GetConstants()->IsTrailingByteValid(uni[1]));

    size_t ucode = curs.GetNextSymbol();
    size_t scanned = curs.GetLastScanned();

    while (ucode != 0)
        {
        LOG.infov("Scanned %d chars  unicode %0x   %s", scanned, ucode, curs.IsASCII()?"ASCII":"Unicode");
        ucode = curs.GetNextSymbol();
        scanned = curs.GetLastScanned();
        }

    LOG.infov("Formatting test End");
}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(FormattingTest, TrueIndex)
    {
    FormattingScannerCursor fmScanner(nullptr, 0);
    UnicodeConstantP unicodeN = fmScanner.GetConstants();
    UnicodeConstant unicodeP;
    char inChar = 'c';
    size_t length = unicodeP.GetSequenceLength(inChar);
    ASSERT_EQ(1, length);
    EXPECT_TRUE(unicodeP.IsLittleEndian());
    EXPECT_FALSE(unicodeP.ForceBigEndian());
    EXPECT_FALSE(unicodeP.IsLittleEndian());
    unicodeP.IsTrailingByteValid('s');
    EXPECT_EQ(9, fmScanner.TrueIndex(9, 7));
    unicodeN->ForceBigEndian();
    size_t sd = fmScanner.TrueIndex(23, 17);
    EXPECT_EQ(-7, (int)sd);
    }