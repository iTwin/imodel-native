/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif

#include "../FormattingTestsPch.h"
#include "UnitsTestFixture.h"


USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

/*=================================================================================**//**
* @bsiclass
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FormattingTestUtils
    {
    public:
        static void NumericFormatSpecJsonTest(NumericFormatSpecCR nfs);
        static Utf8String JsonComparisonString(Json::Value const& created, Json::Value const& test);
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FormattingTestFixture : Units::Tests::UnitsTestFixture
{
private:
    static bmap<Utf8String, Format> s_stdFormats;
    static void CreateStdFormats();
    // The inherited UnitRegistry is for all testing that does not require adding any additional Units.
    // If you need to add additional Units get a new copy of the UnitRegistry.
public:
    bmap<Utf8String, Format> GetStdFormats() {return s_stdFormats;}
    virtual void SetUp() override;
    virtual void TearDown() override;
};

END_BENTLEY_FORMATTEST_NAMESPACE
