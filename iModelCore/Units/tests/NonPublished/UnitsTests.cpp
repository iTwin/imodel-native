/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"

#include <fstream>
#include <sstream>
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_UNITS_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct UnitsTests : UnitsTestFixture
    {
    static void GetMapping(WCharCP file, bmap<Utf8String, Utf8String>& unitNameMap, bset<Utf8String>& notMapped)
        {
        auto lineProcessor = [&unitNameMap, &notMapped] (bvector<Utf8String>& tokens)
            {
            Utf8String newName1 = GetECNameFromOldName(tokens[1].c_str(), notMapped);
            Utf8String newName2 = GetECNameFromOldName(tokens[3].c_str(), notMapped);
            unitNameMap[tokens[1]] = newName1;
            unitNameMap[tokens[3]] = newName2;
            };

        ReadCSVFile(file, lineProcessor);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, UnitsMapping)
    {
    bmap<Utf8String, Utf8String> unitNameMap;
    bset<Utf8String> notMapped;
    GetMapping(L"unitcomparisondata.csv", unitNameMap, notMapped);
    GetMapping(L"complex.csv", unitNameMap, notMapped);

    //output of oldUnit, newUnit mapping
    Utf8String mapOldtoNew = UnitsTestFixture::GetOutputDataPath(L"mapOldtoNew.csv");

    Utf8String guess= "";
    for (auto i : notMapped)
        {
        guess += i + ", ";
        }

    // Increased from 97 to 106 because THREAD_PITCH Phen removed ... units just don't fit Phen.  
    // Decreased from 106 to 103 because three thread pitch units were added back for some reason: M_PER_REVOLUTION, M_PER_RAD and IN_PER_DEGREE
    EXPECT_EQ (103, notMapped.size() ) << guess;
    }
struct TestUnit : Unit
    {
    public:
        TestUnit(Utf8CP name) : Unit(name) {};
    };

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsTests, IsSIReturnsFalseWhenNoUnitSystemSet)
    {
    TestUnit unit("Banana");

    ASSERT_FALSE(unit.IsSI()) << "No UnitSystem is set so false should be returned";
    }

END_UNITS_UNITTESTS_NAMESPACE
