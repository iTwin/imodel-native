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
        static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
        static Utf8String SetLocale(Utf8CP name);
        static Utf8String JsonComparisonString(Json::Value const& created, Json::Value const& test);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FormattingTestFixture : ::testing::Test
{
private:
    static bmap<Utf8String, Format> s_stdFormats;
    static void CreateStdFormats();
    // A UnitRegistry instance to use for all testing that does not require adding any additional Units.
    // If you need to add additional Units get a new copy of the UnitRegistry.
public:
    static BEU::UnitRegistry* s_unitsContext;
    bmap<Utf8String, Format> GetStdFormats() {return s_stdFormats;}
    virtual void SetUp() override;
    virtual void TearDown() override;
};

END_BENTLEY_FORMATTEST_NAMESPACE
