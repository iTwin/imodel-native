/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/FormattingTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif
#include "UnitsTests.h"
#include <Formatting/FormattingApi.h>
#include <BeSQLite/L10N.h>
using namespace BentleyApi::Formatting;
BEGIN_BENTLEY_FORMATTEST_NAMESPACE


#define LOG (*NativeLogging::LoggingManager::GetLogger (L"Format"))

/*=================================================================================**//**
* @bsiclass                                     		David Fox-Rabinovitz 06/2017
+===============+===============+===============+===============+===============+======*/

struct TraitJsonKeyMap
    {
private:
    FormatTraits m_bit;
    Utf8CP m_key;
public:
    TraitJsonKeyMap(FormatTraits bit, Utf8CP key):m_bit(bit), m_key(key){}
    FormatTraits GetTrait() { return m_bit; }
    Utf8CP GetKey() { return m_key; }
    static bvector<TraitJsonKeyMap> TraitJsonKeySet();
    };

struct FormattingTestFixture
    {
    private:
        Utf8CP m_text;
    public:
        static void SetUpL10N();
        static void TearDownL10N();

		static size_t AddSignatureSymbol(Utf8CP outBuf, size_t bufLen, size_t* bufIndex);
		static void CloseTestData();
        static size_t CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src);
		static void CrossValidateFUS(Utf8CP descr1, Utf8CP descr2);
		static void CustomFormatAnalyzer(double dval, Utf8CP uom, Utf8CP jsonCustomFormat);
        static void DecomposeString(Utf8CP str, bool revers);
		static size_t ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div);
		static Utf8String ExtractTokenValue(WCharCP line, WCharCP token, WCharCP delim);
		static size_t FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div);
		static int FindLastDividerPos(Utf8CP txt, Utf8Char div);
        static void FormatDoubleTest(double dval, Utf8CP formatName, int prec=-1.0, double round=-1.0, Utf8CP expect=nullptr);
		static void FormattingSpecTraitsTest(Utf8CP testName, NumericFormatSpecCR spec, bool verbose);
		static void FormattingTraitsTest();
		static size_t GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div);
		static bool GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen);
		static bool GetNextLine(Utf8P buf, int bufLen);
		static Utf8String GetStringSignature(Utf8CP str);
		static bool IsDataAvailalbe();
		static void LogMessage(Utf8CP format, va_list argptr);
		static void NamedFormatJsonTest(int testNum, Utf8CP formatName, bool verbose, Utf8CP expected);
		static void NamedSpecToJson(Utf8CP stdName);
		static NumericAccumulator* NumericAccState(NumericAccumulator* nacc, Utf8CP txt);
		static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
		static bool OpenTestData();
		static void ParseToQuantity(Utf8CP input, size_t start, Utf8CP unitName = nullptr, Utf8CP formatName = nullptr);
		static void RegisterFUS(Utf8CP descr, Utf8CP name);
		static void RegistryLookupUnitCITest(Utf8CP unitName);
		static Utf8String SetLocale(Utf8CP name);
		static void ShowFUS(Utf8CP koq);
		static void ShowFUG(Utf8CP name, Utf8CP fugText);
		static void ShowHexDump(Utf8String str, int len);
		static void ShowHexDump(Utf8CP str, int len, Utf8CP message = nullptr);
		static void ShowKnownPhenomena();
        static void ShowPhenomenon(BEU::PhenomenonCP phenP, bvector<BEU::PhenomenonCP>& undefPhenomena);		
		static void ShowSignature(Utf8CP txt, int tstN);
		static void ShowQuantifiedValue(Utf8CP input, Utf8CP formatName, Utf8CP fusUnit, Utf8CP spacer = nullptr);
		static void ShowSplitByDividers(Utf8CP txt, Utf8CP divDef);
        static void ShowSynonyms();
		static void ShowQuantity(double dval, Utf8CP uom, Utf8CP fusUnit, Utf8CP fusFormat, Utf8CP space);
		static void ShowQuantityS(Utf8CP descr);
		static void SignaturePattrenCollapsing(Utf8CP txt, int tstN, bool hexDump);
		static void StandaloneFUSTest(double dval, Utf8CP unitName, Utf8CP fusUnitName, Utf8CP formatName, Utf8CP result);
		static void StandaloneNamedFormatTest(Utf8CP jsonFormat, bool doPrint = false);
		static void StdFormattingTest(Utf8CP formatName, double dval, Utf8CP expectedValue);
		static void TestFUG(Utf8CP name, Utf8CP fusText, Utf8CP norm, Utf8CP aliased);
		static void TestFUGFormat(double dval, Utf8CP uom, Utf8CP name, Utf8CP fusText);
		static void TestFUS(Utf8CP fusText, Utf8CP norm, Utf8CP aliased);
		static void TestFusLabel(Utf8CP unitName, Utf8CP formatName, Utf8CP fusName);
		static void TestFUSQuantity(double dval, Utf8CP uom, Utf8CP fusDesc, Utf8CP space);
		static Utf8CP TestGrabber(Utf8CP input, size_t start = 0);
		static void TestScanPointVector(Utf8CP str);
		static void TestScanTriplets(Utf8CP str);
		static void TestSegments(Utf8CP input, size_t start, Utf8CP unitName, Utf8CP expectReduced = nullptr);
		static void TestTime(Utf8CP localeName, Utf8CP label, Utf8CP pattern = nullptr);
		static void TestTimeFormat(int year, int month, int day, int hour, int min, int sec);
		static void UnitProxyJsonTest(Utf8CP unitName, Utf8CP labelName);
		static void UnitSynonymMapTest(Utf8CP unitName, Utf8CP synonym = nullptr);
		static bool ValidateSchemaUnitNames(char* schemaPath, Utf8CP token, char* reportPath = nullptr);
		static void VerifyQuantity(Utf8CP input, Utf8CP unitName, Utf8CP formatName, double magnitude, Utf8CP qtyUnitName);

        //static void LoadUnitSynonymsTest();
    };

END_BENTLEY_FORMATTEST_NAMESPACE

//struct FormattingTestData
//    {
//public:
//    static Utf8String ReadFile(Utf8String filePath)
//        {
//        bvector<Byte> fileContents;
//
//        BeFile file;
//        BeFileStatus status;
//
//        status = file.Open(filePath, BeFileAccess::Read);
//        BeAssert(status == BeFileStatus::Success);
//
//        status = file.ReadEntireFile(fileContents);
//        BeAssert(status == BeFileStatus::Success);
//
//        status = file.Close();
//        BeAssert(status == BeFileStatus::Success);
//
//        Utf8String stringContents;
//        stringContents.append(fileContents.begin(), fileContents.end());
//        return stringContents;
//        }
//
//    static void FileHexDump(Utf8String filePath)
//        {
//        Utf8String str = ReadFile(filePath);
//        Utf8String hd = Utils::HexDump(str.c_str(), 30);
//        LOG.infov(u8"FileDump: %s", hd.c_str());
//        }
//
//
//    };

