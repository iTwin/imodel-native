/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/NonPublished/FormattingTestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif
#include "UnitsTests.h"
#include <Formatting/FormattingApi.h>

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
        static void SignaturePattrenCollapsing(Utf8CP txt, int tstN, bool hexDump);
        static void ShowSignature(Utf8CP txt, int tstN);
        static void ShowHexDump(Utf8String str, int len);
        static void ShowHexDump(Utf8CP str, int len);
        static void ShowFUS(Utf8CP koq);
        static void TestFUS(Utf8CP fusText, Utf8CP norm, Utf8CP aliased);
        static void TestFUG(Utf8CP name, Utf8CP fusText, Utf8CP norm, Utf8CP aliased);
        static void ShowQuantity(double dval, Utf8CP uom, Utf8CP fusUnit, Utf8CP fusFormat, Utf8CP space);
        static void ShowQuantityS(Utf8CP descr);
        static NumericAccumulator* NumericAccState(NumericAccumulator* nacc, Utf8CP txt);
        static void TestFUSQuantity(double dval, Utf8CP uom, Utf8CP fusDesc, Utf8CP space);
        static int FindLastDividerPos(Utf8CP txt, Utf8Char div);
        static void ShowSplitByDividers(Utf8CP txt, Utf8CP divDef);
        static size_t FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div);
        static bool OpenTestData();
        static void CloseTestData();
        static bool IsDataAvailalbe();
        static bool GetNextLine(Utf8P buf, int bufLen);
        static bool GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen);
        static size_t CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src);
        static size_t ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div);
        static size_t GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div);
        static void DecomposeString(Utf8CP str, bool revers);
        static void TestScanPointVector(Utf8CP str);
        static void TestScanTriplets(Utf8CP str);
        static Utf8CP TestGrabber(Utf8CP input, size_t start= 0);
        static void TestSegments(Utf8CP input, size_t start, Utf8CP unitName);
        static void ParseToQuantity(Utf8CP input, size_t start, Utf8CP unitName = nullptr);
        static void NamedSpecToJson(Utf8CP stdName);
        static void FormattingTraitsTest();
        static void FormattingSpecTraitsTest(Utf8CP testName, NumericFormatSpecCR spec, bool verbose);
        static void StdFormattingTest(Utf8CP formatName, double dval, Utf8CP expectedValue);
        static void NamedFormatJsonTest(Utf8CP formatName, bool verbose, Utf8CP expected);
        static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
        static void UnitProxyJsonTest(Utf8CP unitName, Utf8CP labelName);
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

