/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UnitsTestFixture.h"
#include <fstream>

BEGIN_UNITS_UNITTESTS_NAMESPACE

UnitRegistry* UnitsTestFixture::s_unitsContext = nullptr;

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void UnitsTestFixture::SetUp()
    {
    if (nullptr == s_unitsContext)
        s_unitsContext = new UnitRegistry();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void UnitsTestFixture::TearDown()
    {
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::GetConversionDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testData);
    testData.AppendToPath(L"ConversionData");
    testData.AppendToPath(dataFile);
    return Utf8String(testData);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::GetOutputDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot(testData);
    testData.AppendToPath(dataFile);
    return Utf8String(testData);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void UnitsTestFixture::WriteLine(BeFile& file, Utf8CP line)
    {
    Utf8String finalLine;
    if (Utf8String::IsNullOrEmpty(line))
        finalLine.assign("\r\n");
    else
        finalLine.Sprintf("%s\r\n", line);

    uint32_t bytesToWrite = static_cast<uint32_t>(finalLine.SizeInBytes() - 1); //not safe, but our line will not exceed 32bits.
    uint32_t bytesWritten;
    EXPECT_EQ(file.Write(&bytesWritten, finalLine.c_str(), bytesToWrite), BeFileStatus::Success);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void UnitsTestFixture::WriteToFile(Utf8CP fileName, bvector<bpair<Utf8String, Utf8String>> lines)
    {
    BeFile file;
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
    for (auto line : lines)
        {
        Utf8PrintfString formatted("%s,%s", line.first.c_str(), line.second.c_str());
        WriteLine(file, formatted.c_str());
        }

    file.Close();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::ParseUOM(Utf8CP unitName, bset<Utf8String>& notMapped)
    {
    UnitCP uom = LocateUOM(unitName, true);
    if (nullptr != uom)
        return uom->GetName();

    notMapped.insert(unitName);
    return "NULL";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
UnitCP UnitsTestFixture::LocateUOM(Utf8CP unitName, bool useLegacyNames)
    {
    if (useLegacyNames)
        return s_unitsContext->LookupUnitUsingOldName(unitName);
        
    return s_unitsContext->LookupUnit(unitName);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
void UnitsTestFixture::ReadCSVFile(WCharCP fileName, MultipleTokensProcessor const& lineProcessor)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.begin(), std::ifstream::in);
    std::string line;

    ASSERT_TRUE(ifs.good()) << "File '" + path + "' does not exist.";

    while (std::getline(ifs, line))
        {
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(line.c_str(), ",", tokens);
        lineProcessor(tokens);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
void UnitsTestFixture::ReadCSVFile(WCharCP fileName, SingleTokenProcessor const& lineProcessor)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.begin(), std::ifstream::in);
    std::string line;
    
    ASSERT_TRUE(ifs.good()) << "File '" + path + "' does not exist.";

    while (std::getline(ifs, line))
        {
        Utf8String token(line.c_str());
        token.Trim();
        lineProcessor(token);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Gintaras.Volkvicius 10/18
//---------------------------------------------------------------------------------------
int UnitsTestFixture::GetCSVFileLineCount(WCharCP fileName)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.begin(), std::ifstream::in);
    std::string line;

    ASSERT_TRUE(ifs.good()) << "File '" + path + "' does not exist.", 0;

    int count = 0;
    while (std::getline(ifs, line))
        ++count;

    return count;
    }

END_UNITS_UNITTESTS_NAMESPACE
