/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/formatting_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/Formatting.h>


TEST(FormattingTest, Simple)
{
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
	numFmt.SetPresentationType(PresentationType::Scientific);
	numFmt.SetSignOption(ShowSignOption::OnlyNegative);
	EXPECT_STREQ ("-0.2718281828e-2", numFmt.FormatDouble(-0.0027182818284590).c_str());
	EXPECT_STREQ ("-0.2718281828", numFmt.FormatDouble(-0.2718281828459045).c_str());
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


	FormatDictionary fd = FormatDictionary();
	FormatParameterP fp, fp1, fp2;
	size_t count = fd.GetCount();

	for (int i = 0; i < count; i++)
	{
		fp = fd.GetParameterByIndex(i);
		fp1 = fd.FindParameterByCode(fp->GetParameterCode());
		fp2 = fd.FindParameterByName(fp->GetName());
		EXPECT_EQ(fp, fp1);
		EXPECT_EQ(fp, fp2);
	}

	FormatingScannerCursor curs = FormatingScannerCursor(nullptr, 0);   // just a core scanner

	EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants().Get2ByteMask()).c_str());
	EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants().Get3ByteMask()).c_str());
	EXPECT_STREQ ("11111000", numFmt.ByteToBinaryText(curs.GetConstants().Get4ByteMask()).c_str());

	EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants().Get2ByteMark()).c_str());
	EXPECT_STREQ ("11100000", numFmt.ByteToBinaryText(curs.GetConstants().Get3ByteMark()).c_str());
	EXPECT_STREQ ("11110000", numFmt.ByteToBinaryText(curs.GetConstants().Get4ByteMark()).c_str());

	EXPECT_STREQ ("11000000", numFmt.ByteToBinaryText(curs.GetConstants().GetTrailingByteMask()).c_str());
	EXPECT_STREQ ("10000000", numFmt.ByteToBinaryText(curs.GetConstants().GetTrailingByteMark()).c_str());
	EXPECT_STREQ ("00111111", numFmt.ByteToBinaryText(curs.GetConstants().GetTrailingBitsMask()).c_str());

	for (char c = 'A'; c < 'z'; c++)
	{
		EXPECT_EQ(1, curs.GetConstants().GetSequenceLength(c));
	}

	char *uni = u8"ЯABГCDE型号sautéςερτcañón";  // (char*)mem;
	EXPECT_EQ(2, curs.GetConstants().GetSequenceLength(uni[0]));
	EXPECT_TRUE(curs.GetConstants().IsTrailingByteValid(uni[1]));

}
