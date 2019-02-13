/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/UnitsTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"

#include <fstream>
#include <sstream>
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_UNITS_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct UnitsTests : UnitsTestFixture
    {
    static void GetMapping(WCharCP file, bmap<Utf8String, Utf8String>& unitNameMap, bset<Utf8String>& notMapped)
        {
        auto lineProcessor = [&unitNameMap, &notMapped] (bvector<Utf8String>& tokens)
            {
            Utf8String newName1 = ParseUOM(tokens[1].begin(), notMapped);
            Utf8String newName2 = ParseUOM(tokens[3].begin(), notMapped);
            unitNameMap[tokens[1]] = newName1;
            unitNameMap[tokens[3]] = newName2;
            };

        ReadCSVFile(file, lineProcessor);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                03/16
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

    EXPECT_EQ (106, notMapped.size() ) << guess; // Increased from 97 to 106 because THREAD_PITCH Phen removed ... units just don't fit Phen

    //Test that all mappings do not use synonmyms
    for(const auto& i : unitNameMap)
        {
        if (i.second.Equals("NULL"))
            continue;
        UnitCP unit = s_unitsContext->LookupUnit(i.second.c_str());
        ASSERT_NE(nullptr, unit) << "Couldn't find unit with name " << i.second.c_str();
        EXPECT_STREQ(i.second.c_str(), unit->GetName().c_str()) << "Mapping for old unit '" << i.first.c_str() << "' uses a synonmym";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, CheckSignatureForEveryPhenomenon)
    {
    bvector<PhenomenonCP> allPhenomena;
    s_unitsContext->AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        PERFORMANCELOG.errorv("Dimension string for %s: %s", phenomenon->GetName().c_str(), phenomenon->GetPhenomenonSignature().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, PhenomenonAndUnitSignaturesMatch)
    {
    bvector<PhenomenonCP> allPhenomena;
    s_unitsContext->AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        for (auto const& unit : phenomenon->GetUnits())
            {
            EXPECT_TRUE(phenomenon->IsCompatible(*unit)) << "The unit " << unit->GetName() << " is not dimensionally compatible with the phenomenon it belongs to: " << phenomenon->GetName();
            }
        }
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                            Colin.Kerr                                 02/18
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsTests, EveryPhenomenonHasAtleastOneUnit)
    {
    bvector<PhenomenonCP> allPhenomena;
    s_unitsContext->AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        EXPECT_NE(0, phenomenon->GetUnits().size()) << "The Phenomenon '" << phenomenon->GetName().c_str() << "' has no units.";
        }
    }

struct TestUnit : Unit
    {
    public:
        TestUnit(Utf8CP name) : Unit(name) {};
    };

//---------------------------------------------------------------------------------------//
// @bsimethod                            Colin.Kerr                                 02/18
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitsTests, IsSIReturnsFalseWhenNoUnitSystemSet)
    {
    TestUnit unit("Banana");

    ASSERT_FALSE(unit.IsSI()) << "No UnitSystem is set so false should be returned";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Robert.Schili                              03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, PrintOutAllUnitsGroupedByPhenonmenon)
    {
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"AllUnitsByPhenomenon.csv");
    BeFile file;
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);

    Utf8String fileName2 = UnitsTestFixture::GetOutputDataPath(L"AllUnitNamesFlat.csv");
    BeFile file2;
    EXPECT_EQ(file2.Create(fileName2, true), BeFileStatus::Success);

    bvector<PhenomenonCP> phenomena;
    s_unitsContext->AllPhenomena(phenomena);

    WriteLine(file, "Name,Unit System,Sub Components,Signature,ParsedDefinition");

    for (auto const& phenomenon : phenomena)
        {
        if (phenomenon->GetUnits().size() == 0)
            continue;

        WriteLine(file, "-Phenomenon-");
        Utf8PrintfString line("%s,,%s,%s", phenomenon->GetName().c_str(), phenomenon->GetDefinition().c_str(), phenomenon->GetPhenomenonSignature().c_str());
        WriteLine(file, line.c_str());

        WriteLine(file);
        
        for (auto const& unit : phenomenon->GetUnits())
            {
            if (unit->IsConstant())
                continue;

            Utf8String parsedExpression = unit->GetParsedUnitExpression();
            line.Sprintf("%s,%s,%s,%s,%s", unit->GetName().c_str(), unit->GetUnitSystem()->GetName().c_str(), unit->GetDefinition().c_str(), unit->GetUnitSignature().c_str(), parsedExpression.c_str());

            WriteLine(file, line.c_str());

            WriteLine(file2, unit->GetName().c_str());
            }

        WriteLine(file);
        }

    file.Close();
    file2.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, UnitNamesByReferencedComponents)
    {
    bmap<Utf8String, bvector<Utf8String>> unitNamesByReferencedComponent;
    bvector<UnitCP> allUnits;
    s_unitsContext->AllUnits(allUnits);
    for (auto const& unit : allUnits)
        {
        bvector<Utf8String> symbols;
        BeStringUtilities::Split(unit->GetDefinition().c_str(), "*", symbols);
        for (auto const& symbol : symbols)
            {
            auto index = symbol.find('(');
            Utf8String unitName;
            if (index < symbol.length())
                unitName = symbol.substr(0, index).c_str();
            else
                unitName = symbol.c_str();

            unitName.ReplaceAll("[", "");
            unitName.ReplaceAll("]", "");

            UnitCP subUnit = s_unitsContext->LookupUnit(unitName.c_str());
            ASSERT_NE(nullptr, subUnit) << "Could not find subunit: " << unitName;

            auto it = unitNamesByReferencedComponent.find(unitName);
            if (it == unitNamesByReferencedComponent.end())
                {
                bvector<Utf8String> unitsWhichReferenceSubUnit;
                unitsWhichReferenceSubUnit.push_back(unit->GetName());
                unitNamesByReferencedComponent.Insert(unitName, unitsWhichReferenceSubUnit);
                }
            else
                {
                it->second.push_back(unit->GetName());
                }
            }
        }

    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"UnitNamesByReferencedComponents.csv");
    BeFile file;
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
    for (auto const& unitsWhichRefUnit : unitNamesByReferencedComponent)
        {
        WriteLine(file, unitsWhichRefUnit.first.c_str());
        for (auto const& unit : unitsWhichRefUnit.second)
            {
            Utf8PrintfString unitString("    %s", unit.c_str());
            WriteLine(file, unitString.c_str());
            }
        }
    WriteLine(file);
    file.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, TestUnitDefinitionsDoNotContainSynonyms)
    {
    bvector<UnitCP> allUnits;
    s_unitsContext->AllUnits(allUnits);
    for (auto const& unit : allUnits)
        {
        bvector<Utf8String> symbols;
        BeStringUtilities::Split(unit->GetDefinition().c_str(), "*", symbols);
        for (auto const& symbol : symbols)
            {
            auto index = symbol.find('(');
            Utf8String unitName;
            if (index < symbol.length())
                unitName = symbol.substr(0, index).c_str();
            else
                unitName = symbol.c_str();

            unitName.ReplaceAll("[", "");
            unitName.ReplaceAll("]", "");

            UnitCP subUnit = s_unitsContext->LookupUnit(unitName.c_str());
            ASSERT_NE(nullptr, subUnit) << "Could not find subunit: " << unitName;
            EXPECT_STREQ(unitName.c_str(), subUnit->GetName().c_str()) << "The Unit " << unit->GetName() << " has sub unit " << unitName << " in it's definition which is a Synonym for " << subUnit->GetName();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, ExportDisplayLabelsFromOldSystem)
    {
    bmap<Utf8String, Utf8String> labels;
    bvector<Utf8String> missingUnits;
    auto lineProcessor = [&labels, &missingUnits] (bvector<Utf8String>& tokens)
        {
        UnitCP unit = LocateUOM(tokens[0].c_str(), true);
        if (unit == nullptr)
            {
            unit = LocateUOM(tokens[0].c_str(), false);
            if (nullptr == unit)
                {
                missingUnits.push_back(tokens[0]);
                return;
                }
            }
        // Skip labels which are for a unit that already has a label
        if (labels.end() != labels.find(unit->GetName()))
            return;

        labels[unit->GetName()] = Utf8PrintfString("%s,%s", tokens[1].c_str(), tokens[2].c_str());
        };

    ReadCSVFile(L"oldunitlabels.csv", lineProcessor);

    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"NewUnitLabelsByName.csv");
    BeFile file;
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
    for (auto const& s : labels)
        {
        Utf8PrintfString line("%s,%s", s.first.c_str(), s.second.c_str());
        WriteLine(file, line.c_str());
        }

    file.Close();

    if (missingUnits.size() > 0)
        {
        fileName = UnitsTestFixture::GetOutputDataPath(L"OldUnitLabelsNotUsedInNewSystem.txt");
        EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
        for (auto const& s : missingUnits)
            {
            WriteLine(file, s.c_str());
            }

        file.Close();
        }

    bvector<Utf8String> allUnitNames;
    s_unitsContext->AllUnitNames(allUnitNames, false);
    bvector<Utf8String> missingLabels;
    for (auto const& name : allUnitNames)
        {
        if (labels.end() == labels.find(name))
            missingLabels.push_back(name);
        }

    fileName = UnitsTestFixture::GetOutputDataPath(L"UnitsWithoutDisplayLabel.csv");
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
    for (auto const& s : missingLabels)
        {
        WriteLine(file, s.c_str());
        }

    file.Close();
    }

END_UNITS_UNITTESTS_NAMESPACE
