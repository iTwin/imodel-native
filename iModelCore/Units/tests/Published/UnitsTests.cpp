/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/UnitsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../UnitsTestsPch.h"
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

        ReadConversionCsvFile(file, lineProcessor);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitsTests, UnitsMapping)
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

    EXPECT_EQ (97, notMapped.size() ) << guess;

    //Test that all mappings do not use synonmyms
    for(const auto& i : unitNameMap)
        {
        if (i.second.Equals("NULL"))
            continue;
        UnitCP unit = UnitRegistry::Instance().LookupUnit(i.second.c_str());
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
    UnitRegistry::Instance().AllPhenomena(allPhenomena);
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
    UnitRegistry::Instance().AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        for (auto const& unit : phenomenon->GetUnits())
            {
            EXPECT_TRUE(phenomenon->IsCompatible(*unit)) << "The unit " << unit->GetName() << " is not dimensionally compatible with the phenomenon it belongs to: " << phenomenon->GetName();
            }
        }
    }

//void ReadFile(Utf8CP path, std::function<void(Utf8CP)> lineProcessor)
//    {
//    std::ifstream ifs(path, std::ifstream::in);
//    std::string line;
//
//    while (std::getline(ifs, line))
//        {
//        lineProcessor(line.c_str());
//        }
//    }
//
//TEST_F(UnitsTests, MergeListsOfUnits)
//    {
//    bvector<Utf8String> unitsList;
//    auto merger = [&unitsList] (Utf8CP token)
//        {
//        auto it = find(unitsList.begin(), unitsList.end(), token);
//        if (it == unitsList.end())
//            unitsList.push_back(token);
//        };
//
//    ReadFile("C:\\Source\\GraphiteTestData\\Second pass units lists\\units.txt.bak", merger);
//    ReadFile("C:\\Source\\GraphiteTestData\\Second pass units lists\\allowableUnits.txt", merger);
//    ReadFile("C:\\Source\\DgnDb0601Dev_1\\src\\Units\\test\\ConversionData\\NeededUnits.csv", merger);
//
//    sort(unitsList.begin(), unitsList.end());
//
//    ofstream fileStream("C:\\NeededUnits.csv", ofstream::out);
//    for (auto const& unit : unitsList)
//        fileStream << unit.c_str() << endl;
//    fileStream.close();
//    }

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
    UnitRegistry::Instance().AllPhenomena(phenomena);

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
    UnitRegistry::Instance().AllUnits(allUnits);
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

            UnitCP subUnit = UnitRegistry::Instance().LookupUnit(unitName.c_str());
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
    UnitRegistry::Instance().AllUnits(allUnits);
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

            UnitCP subUnit = UnitRegistry::Instance().LookupUnit(unitName.c_str());
            ASSERT_NE(nullptr, subUnit) << "Could not find subunit: " << unitName;
            EXPECT_STREQ(unitName.c_str(), subUnit->GetName().c_str()) << "The Unit " << unit->GetName() << " has sub unit " << unitName << " in it's definition which is a Synonym for " << subUnit->GetName();
            }
        }
    }

//#define CREATE_XLIFF
/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsTests, ExportDisplayLabelsFromOldSystem)
    {
    bmap<Utf8String, Utf8String> labels;
    bvector<Utf8String> missingUnits;

#ifdef CREATE_XLIFF
    Utf8String l10nfileName = "c:\\UnitLabels_xliff.xml";
    BeFile l10nfile;
    EXPECT_EQ(l10nfile.Create(l10nfileName, true), BeFileStatus::Success);

    WriteLine(l10nfile, R"*(
<?xml version='1.0' encoding='UTF-8'?>
<xliff version="1.2" xmlns="urn:oasis:names:tc:xliff:document:1.2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="urn:oasis:names:tc:xliff:document:1.2 xliff-core-1.2-strict.xsd">
  <!--XLIFF document generated by Bentley Systems, Incorporated.-->
  <file datatype="plaintext" original="Units.xliff.h" source-language="en">
    <body>
      <group resname="units_labels">
)*");

    auto lineProcessor = [&labels, &missingUnits, &l10nfile] (bvector<Utf8String>& tokens)
#else
    auto lineProcessor = [&labels, &missingUnits] (bvector<Utf8String>& tokens)
#endif
        {
        UnitCP unit = LocateUOM(tokens[0].c_str(), true);
        if (unit == nullptr)
            {
            missingUnits.push_back(tokens[0]);
            return;
            }

#ifdef CREATE_XLIFF
        MD5 md5;
        Utf8PrintfString resname("%s:%s", unit->GetPhenomenon()->GetName(), unit->GetName());
        Utf8PrintfString line1("<trans-unit id=\"LABEL_%s\" resname=\"LABEL_%s\">", md5(resname).c_str(), md5(resname).c_str());
        WriteLine(l10nfile, line1.c_str());
        Utf8PrintfString line2("<source xml:space=\"preserve\">%s</source>", tokens[1].c_str());
        WriteLine(l10nfile, line2.c_str());
        Utf8PrintfString line3("<note>%s</note>", resname.c_str());
        WriteLine(l10nfile, line3.c_str());
        WriteLine(l10nfile, "</trans-unit>");
        Utf8PrintfString line4("<trans-unit id=\"DESCRIPTION_%s\" resname=\"DESCRIPTION_%s\">", md5(resname).c_str(), md5(resname).c_str());
        WriteLine(l10nfile, line4.c_str());
        Utf8PrintfString line5("<source xml:space=\"preserve\">%s</source>", tokens[1].c_str());
        WriteLine(l10nfile, line5.c_str());
        Utf8PrintfString line6("<note>%s</note>", resname.c_str());
        WriteLine(l10nfile, line6.c_str());
        WriteLine(l10nfile, "</trans-unit>");
#endif

        labels[unit->GetName()] = Utf8PrintfString("%s,%s", tokens[1].c_str(), tokens[2].c_str());
        };

    ReadConversionCsvFile(L"oldunitlabels.csv", lineProcessor);

#ifdef CREATE_XLIFF
    WriteLine(l10nfile, R"*(
      </group>
    </body>
  </file>
</xliff>
)*");

    l10nfile.Close();
#endif

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
    UnitRegistry::Instance().AllUnitNames(allUnitNames, false);
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
