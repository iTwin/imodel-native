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
    public:
        static void SignaturePattrenCollapsing(Utf8CP txt, int tstN);
        static void ShowSignature(Utf8CP txt, int tstN);
        static int FindLastDividerPos(Utf8CP txt, Utf8Char div);
        static void ShowSplitByDividers(Utf8CP txt, Utf8CP divDef);
        static size_t FindDividerPos(Utf8CP txt, bvector<int>* pos, Utf8Char div);
        static bool OpenTestData();
        static void CloseTestData();
        static bool IsDataAvailalbe();
        static bool GetNextLine(Utf8P buf, int bufLen);
        static Utf8String ExtractTokenValue(wchar_t* line, wchar_t* token, wchar_t* delim);
        static bool GetNextInstruction(Utf8P buf, int bufLen, Utf8P com, int comLen);
        static size_t CopyTextSecure(Utf8P dest, size_t destSize, Utf8CP src);
        static size_t ExtractArgs(Utf8CP desc, Utf8P buf, size_t bufL, bvector<Utf8CP>* parts, Utf8Char div);
        static size_t GetNextArguments(Utf8P buf, int bufLen, bvector<Utf8CP>* parts, Utf8Char div);
        static void DecomposeString(Utf8CP str, bool revers);
        static void TestScanPointVector(Utf8CP str);
        static void TestScanTriplets(Utf8CP str);
        static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
        static void ShowPhenomenon(BEU::PhenomenonCP phenP, bvector<BEU::PhenomenonCP>& undefPhenomena);
        static Utf8String SetLocale(Utf8CP name);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FormattingTestFixture : ::testing::Test
{
    // A UnitRegistry instance to use for all testing that does not require adding any additional Units.
    // If you need to add additional Units get a new copy of the UnitRegistry.
    static BEU::UnitRegistry* s_unitsContext;

    virtual void SetUp() override;
    virtual void TearDown() override;
};

END_BENTLEY_FORMATTEST_NAMESPACE
