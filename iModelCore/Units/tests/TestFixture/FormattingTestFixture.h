/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/TestFixture/FormattingTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif

#include "../FormattingTestsPch.h"

#include <BeSQLite/L10N.h>

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                             David Fox-Rabinovitz 06/2017
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

/*=================================================================================**//**
* @bsiclass                                             David Fox-Rabinovitz 06/2017
+===============+===============+===============+===============+===============+======*/
struct FormattingTestUtils
    {
    private:
        Utf8CP m_text;
    public:
        static void SetUpL10N();
        static void TearDownL10N();

        static void SignaturePattrenCollapsing(Utf8CP txt, int tstN);
        static void ShowSignature(Utf8CP txt, int tstN);
        static void CrossValidateFUS(Utf8CP descr1, Utf8CP descr2);
        static void ShowQuantity(double dval, Utf8CP uom, Utf8CP fusUnit, Utf8CP fusFormat, Utf8CP space);
        static void ShowQuantityS(Utf8CP descr);
        static void TestFUSQuantity(double dval, Utf8CP uom, Utf8CP fusDesc, Utf8CP space);
        static int FindLastDividerPos(Utf8CP txt, Utf8Char div);
        static void ShowSplitByDividers(Utf8CP txt, Utf8CP divDef);
        static size_t FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div);
        static bool OpenTestData();
        static void CloseTestData();
        static bool IsDataAvailalbe();
        static bool GetNextLine(Utf8P buf, int bufLen);
        static Utf8String ExtractTokenValue(wchar_t* line, wchar_t* token, wchar_t* delim);
        static bool ValidateSchemaUnitNames(char* schemaPath, Utf8CP token, char* reportPath=nullptr);
        static bool GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen);
        static size_t CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src);
        static size_t ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div);
        static size_t GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div);
        static void DecomposeString(Utf8CP str, bool revers);
        static void TestScanPointVector(Utf8CP str);
        static void TestScanTriplets(Utf8CP str);
        static void TestSegments(Utf8CP input, Utf8CP unitName, Utf8CP expectReduced=nullptr);
        static void StdFormattingTest(Utf8CP formatName, double dval, Utf8CP expectedValue);
        static void NamedFormatJsonTest(int testNum, Utf8CP formatName, bool verbose, Utf8CP expected);
        static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
        static void UnitProxyJsonTest(Utf8CP unitName, Utf8CP labelName);
        static void UnitSynonymMapTest(Utf8CP unitName, Utf8CP synonym=nullptr);
        static void StandaloneNamedFormatTest(Utf8CP jsonFormat, bool doPrint = false);
        static void StandaloneFUSTest(double dval, Utf8CP unitName, Utf8CP fusUnitName, Utf8CP formatName, Utf8CP result);
        static void FormatDoubleTest(double dval, Utf8CP formatName, int prec=-1.0, double round=-1.0, Utf8CP expect=nullptr);
        static void ShowPhenomenon(BEU::PhenomenonCP phenP, bvector<BEU::PhenomenonCP>& undefPhenomena);
        static void ShowKnownPhenomena();
        static void ShowSynonyms();
        static Utf8String SetLocale(Utf8CP name);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FormattingTestFixture : ::testing::Test
{
    FormattingTestFixture() {}
    virtual void SetUp() override {FormattingTestUtils::SetUpL10N();}
    virtual void TearDown() override {FormattingTestUtils::TearDownL10N();}
};

END_BENTLEY_FORMATTEST_NAMESPACE
