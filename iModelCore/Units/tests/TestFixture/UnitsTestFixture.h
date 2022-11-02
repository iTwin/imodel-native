/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "UnitsTests.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct UnitsTestFixture : public ::testing::Test
    {
    public:
        // A UnitRegistry instance to use for all testing that does not require adding any additional Units.
        // If you need to add additional Units get a new copy of the UnitRegistry.
        static UnitRegistry* s_unitsContext;

        void SetUp() override;
        void TearDown() override;

        static void FillRegistry (UnitRegistry* registry);

        using MultipleTokensProcessor = std::function<void(bvector<Utf8String>&)>;
        using SingleTokenProcessor = std::function<void(Utf8StringCR)>;

        static Utf8String GetConversionDataPath(WCharCP dataFile);
        static Utf8String GetOutputDataPath(WCharCP dataFile);

        static void WriteLine(BeFile& file, Utf8CP line = nullptr);
        static void WriteToFile(Utf8CP fileName, bvector<bpair<Utf8String, Utf8String>> lines);

        static void ReadCSVFile(WCharCP fileName, MultipleTokensProcessor const& lineProcessor);
        static void ReadCSVFile(WCharCP fileName, SingleTokenProcessor const& lineProcessor);
        
        static int GetCSVFileLineCount(WCharCP fileName);


        static Utf8String GetECNameFromOldName(Utf8CP unitName, bset<Utf8String>& notMapped);
    };

END_UNITS_UNITTESTS_NAMESPACE