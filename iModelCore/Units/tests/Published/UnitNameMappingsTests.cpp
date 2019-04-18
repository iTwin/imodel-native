/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/UnitsTestFixture.h"
#include <unordered_set>
#include <unordered_map>

// Hash implementation for Utf8String type
namespace std
    {
    template<> struct hash<Utf8String>
        {
        typedef Utf8String argument_type;
        typedef size_t result_type;
        result_type operator()(argument_type const& string) const { return hash<std::string>{}(string.c_str()); }
        };
    }

USING_NAMESPACE_BENTLEY_UNITS
BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitNameMappingsTests : UnitsTestFixture {};

std::unordered_map<Utf8String, Utf8String> newNamesToDifferentOldNamesOnRoundtrip =
    {
        { "ONE", "UNITLESS_UNIT" },
        { "KG/LITRE", "KILOGRAM_PER_LITRE" },
        { "PERSON", "PERSON" },
        { "M/M", "METRE_VERTICAL_PER_METRE_HORIZONTAL" },
        { "FT/FT", "FOOT_VERTICAL_PER_FOOT_HORIZONTAL" },
        { "MEGAPASCAL", "MEGAPASCAL" },
        { "PA", "NEWTON_PER_METRE_SQUARED" },
        { "RPM", "REVOLUTION_PER_MINUTE" }
    };

std::unordered_map<Utf8String, Utf8String> oldNamesToDifferentNewNamesOnRoundtrip =
    {
        { "CAPITA", "PERSON" },
        { "CUSTOMER", "PERSON" },
        { "EMPLOYEE", "PERSON" },
        { "GUEST", "PERSON" },
        { "PASSENGER", "PERSON" },
        { "RESIDENT", "PERSON" },
        { "STUDENT", "PERSON" },
        { "HUNDRED_CAPITA", "HUNDRED_PERSON" },
        { "THOUSAND_CAPITA", "THOUSAND_PERSON" }
    };

std::unordered_map<Utf8String, Utf8String> ecNamesToDifferentNewNamesOnRoundtrip =
    {
        { "UNITS:KPF", "KPF" },
        { "UNITS:DECA", "DECA" },
        { "UNITS:MEGAPASCAL", "MEGAPASCAL" },
        { "UNITS:PERSON", "PERSON" },
        { "UNITS:HUNDRED_PERSON", "HUNDRED_PERSON" },
        { "UNITS:THOUSAND_PERSON", "THOUSAND_PERSON" }
    };

std::unordered_map<Utf8String, Utf8String> ecNamesToDifferentOldNamesOnRoundtrip =
    {
        { "UNITS:ONE", "UNITLESS_UNIT" },
        { "UNITS:KG_PER_LITRE", "KILOGRAM_PER_LITRE" },
        { "UNITS:PERSON", "PERSON" },
        { "UNITS:M_PER_M", "METRE_VERTICAL_PER_METRE_HORIZONTAL" },
        { "UNITS:FT_PER_FT", "FOOT_VERTICAL_PER_FOOT_HORIZONTAL" },
        { "UNITS:MEGAPASCAL", "MEGAPASCAL" },
        { "UNITS:PA", "NEWTON_PER_METRE_SQUARED" },
        { "UNITS:RPM", "REVOLUTION_PER_MINUTE" }
    };

std::unordered_set<Utf8String> newNamesToNoECNames =
    {
    "CM/REVOLUTION",
    "MM/REVOLUTION",
    "FT/REVOLUTION",
    "IN/REVOLUTION",
    "MM/RAD",
    "IN/RAD",
    "M/DEGREE"
    };

std::unordered_set<Utf8String> ecNamesToNoNewNames =
    {
    "UNITS:MONETARY_UNIT"
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, OldNameToNewNameToOldNameRoundtrip)
    {
    int actualNotMappedCount = 0;
    auto const processUnit = [&actualNotMappedCount](Utf8StringCR actualOldName)
        {
        auto newName = UnitNameMappings::TryGetNewNameFromOldName(actualOldName.c_str());
        
        if (nullptr == newName)
            {
            ++actualNotMappedCount;
            return;
            }
        
        auto expectedOldName = UnitNameMappings::TryGetOldNameFromNewName(newName);
        auto const it = newNamesToDifferentOldNamesOnRoundtrip.find(newName);

        if (std::end(newNamesToDifferentOldNamesOnRoundtrip) != it)
            EXPECT_STREQ(expectedOldName, it->second.c_str());
        else
            EXPECT_STREQ(expectedOldName, actualOldName.c_str());
        };

    ReadCSVFile(L"AllOldUnits.csv", processUnit);
    EXPECT_EQ(GetCSVFileLineCount(L"OldNamesWithoutNewNames.csv"), actualNotMappedCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NewNameToOldNameToNewNameRoundtrip)
    {
    int actualNotMappedCount = 0;
    auto const processUnit = [&actualNotMappedCount](Utf8StringCR actualNewName)
        {
        auto oldName = UnitNameMappings::TryGetOldNameFromNewName(actualNewName.c_str());
        
        if (nullptr == oldName)
            {
            ++actualNotMappedCount;
            return;
            }

        auto expectedNewName = UnitNameMappings::TryGetNewNameFromOldName(oldName);
        auto const it = oldNamesToDifferentNewNamesOnRoundtrip.find(oldName);
        
        if (std::end(oldNamesToDifferentNewNamesOnRoundtrip) != it)
            EXPECT_STREQ(expectedNewName, it->second.c_str());
        else
            EXPECT_STREQ(expectedNewName, actualNewName.c_str());
        };

    ReadCSVFile(L"All3_1Names.csv", processUnit);
    EXPECT_EQ(GetCSVFileLineCount(L"NewNamesWithoutOldNames.csv"), actualNotMappedCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NewNameToECNameToNewNameRoundtrip)
    {
    auto const processUnit = [](Utf8StringCR actualNewName)
        {
        auto ecName = UnitNameMappings::TryGetECNameFromNewName(actualNewName.c_str());

        if (std::end(newNamesToNoECNames) != newNamesToNoECNames.find(actualNewName))
            {
            ASSERT_TRUE(nullptr == ecName) << "New name " + actualNewName + " should not map to any EC name, but it was mapped to " + ecName;
            return;
            }
        
        ASSERT_TRUE(nullptr != ecName) << "Failed to map new name " + actualNewName + " to EC name.";

        auto expectedNewName = UnitNameMappings::TryGetNewNameFromECName(ecName);
        auto const it = ecNamesToDifferentNewNamesOnRoundtrip.find(ecName);

        if (std::end(ecNamesToDifferentNewNamesOnRoundtrip) != it)
            EXPECT_STREQ(expectedNewName, it->second.c_str());
        else
            EXPECT_STREQ(expectedNewName, actualNewName.c_str());
        };

    ReadCSVFile(L"All3_1Names.csv", processUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, ECNameToNewNameToECNameRoundtrip)
    {
    auto const processUnit = [](Utf8StringCR actualECName)
        {
        auto newName = UnitNameMappings::TryGetNewNameFromECName(actualECName.c_str());

        if (std::end(ecNamesToNoNewNames) != ecNamesToNoNewNames.find(actualECName))
            {
            ASSERT_TRUE(nullptr == newName) << "EC name " + actualECName + " should not map to any new name, but it was mapped to " + newName;
            return;
            }

        ASSERT_TRUE(nullptr != newName) << "Failed to map EC name " + actualECName + " to new name.";

        auto expectedECName = UnitNameMappings::TryGetECNameFromNewName(newName);
        EXPECT_STREQ(expectedECName, actualECName.c_str());
        };

    ReadCSVFile(L"AllECNames.csv", processUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, OldNameToECNameToOldNameRoundtrip)
    {
    int actualNotMappedCount = 0;
    auto const processUnit = [&actualNotMappedCount](Utf8StringCR actualOldName)
    {
        auto ecName = UnitNameMappings::TryGetECNameFromOldName(actualOldName.c_str());
        
        if (nullptr == ecName)
            {
            ++actualNotMappedCount;
            return;
            }

        auto expectedOldName = UnitNameMappings::TryGetOldNameFromECName(ecName);
        auto const it = ecNamesToDifferentOldNamesOnRoundtrip.find(ecName);

        if (std::end(ecNamesToDifferentOldNamesOnRoundtrip) != it)
            EXPECT_STREQ(expectedOldName, it->second.c_str());
        else
            EXPECT_STREQ(expectedOldName, actualOldName.c_str());
        };
    
    ReadCSVFile(L"AllOldUnits.csv", processUnit);
    EXPECT_EQ(GetCSVFileLineCount(L"OldNamesWithoutECNames.csv"), actualNotMappedCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, ECNameToOldNameToECNameRoundtrip)
    {
    int actualNotMappedCount = 0;
    auto const processUnit = [&actualNotMappedCount](Utf8StringCR actualECName)
        {
        auto oldName = UnitNameMappings::TryGetOldNameFromECName(actualECName.c_str());

        if (nullptr == oldName)
            {
            ++actualNotMappedCount;
            return;
            }

        auto expectedECName = UnitNameMappings::TryGetECNameFromOldName(oldName);
        EXPECT_STREQ(expectedECName, actualECName.c_str());
        };

    ReadCSVFile(L"AllECNames.csv", processUnit);
    EXPECT_EQ(GetCSVFileLineCount(L"ECNamesWithoutOldNames.csv"), actualNotMappedCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, SpecificMappings)
    {
    EXPECT_STREQ("UNITS:KPF", UnitNameMappings::TryGetECNameFromNewName("KPF"));
    EXPECT_STREQ("UNITS:KPF", UnitNameMappings::TryGetECNameFromNewName("KIPF"));
    EXPECT_STREQ("KPF", UnitNameMappings::TryGetNewNameFromECName("UNITS:KPF"));

    EXPECT_STREQ("UNITS:MEGAPASCAL", UnitNameMappings::TryGetECNameFromNewName("MEGAPASCAL"));
    EXPECT_STREQ("UNITS:MEGAPASCAL", UnitNameMappings::TryGetECNameFromNewName("N/SQ.MM"));
    EXPECT_STREQ("MEGAPASCAL", UnitNameMappings::TryGetNewNameFromECName("UNITS:MEGAPASCAL"));

    EXPECT_STREQ("UNITS:DECA", UnitNameMappings::TryGetECNameFromNewName("DECA"));
    EXPECT_STREQ("UNITS:DECA", UnitNameMappings::TryGetECNameFromNewName("DEKA"));
    EXPECT_STREQ("DECA", UnitNameMappings::TryGetNewNameFromECName("UNITS:DECA"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NoMappingFromOldNameToNewName)
    {
    auto const processUnit = [](Utf8StringCR oldName)
        {
        EXPECT_EQ(nullptr, UnitNameMappings::TryGetNewNameFromOldName(oldName.c_str()));
        };

    ReadCSVFile(L"OldNamesWithoutNewNames.csv", processUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NoMappingFromNewNameToOldName)
    {
    auto const processUnit = [](Utf8StringCR newName)
        {
        EXPECT_EQ(nullptr, UnitNameMappings::TryGetOldNameFromNewName(newName.c_str()));
        };

    ReadCSVFile(L"NewNamesWithoutOldNames.csv", processUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NoMappingFromOldNameToECName)
    {
    auto const processUnit = [](Utf8StringCR oldName)
        {
        EXPECT_EQ(nullptr, UnitNameMappings::TryGetECNameFromOldName(oldName.c_str()));
        };

    ReadCSVFile(L"OldNamesWithoutECNames.csv", processUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
TEST_F(UnitNameMappingsTests, NoMappingFromECNameToOldName)
    {
    auto const processUnit = [](Utf8StringCR ecName)
        {
        EXPECT_EQ(nullptr, UnitNameMappings::TryGetOldNameFromECName(ecName.c_str()));
        };

    ReadCSVFile(L"ECNamesWithoutOldNames.csv", processUnit);
    }

END_UNITS_UNITTESTS_NAMESPACE