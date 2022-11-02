/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UnitsTestFixture.h"
#include <fstream>

BEGIN_UNITS_UNITTESTS_NAMESPACE

UnitRegistry* UnitsTestFixture::s_unitsContext = nullptr;
static Utf8CP const SI = "SI";
static Utf8CP const METRIC = "METRIC";
static Utf8CP const INTERNATIONAL = "INTERNATIONAL";
static Utf8CP const USCUSTOM = "USCUSTOM";
static Utf8CP const CONSTANT = "CONSTANT";
static Utf8CP const DUMMY = "DUMMY";

static Utf8CP const LENGTH = "LENGTH";
static Utf8CP const MASS = "MASS";
static Utf8CP const TIME = "TIME";
static Utf8CP const NUMBER = "NUMBER";
static Utf8CP const ANGLE = "ANGLE";
static Utf8CP const VELOCITY = "VELOCITY";
static Utf8CP const ACCELERATION = "ACCELERATION";
static Utf8CP const FORCE = "FORCE";
static Utf8CP const WORK = "WORK";
static Utf8CP const LENGTH_RATIO = "LENGTH_RATIO";
static Utf8CP const METRIC_PREFIX = "METRIC_PREFIX";

static double const PI = 3.1415926535897932384626433832795;


void UnitsTestFixture::FillRegistry(UnitRegistry* registry)
    {
    registry->AddSystem(SI);
    registry->AddSystem(METRIC);
    registry->AddSystem(INTERNATIONAL);
    registry->AddSystem(USCUSTOM);
    registry->AddSystem(CONSTANT);
    registry->AddSystem(DUMMY);

    registry->AddPhenomenon(LENGTH, LENGTH);
    registry->AddPhenomenon(MASS, MASS);
    registry->AddPhenomenon(TIME, TIME);
    registry->AddPhenomenon(NUMBER, NUMBER);
    registry->AddPhenomenon(ANGLE, ANGLE);

    registry->AddPhenomenon(LENGTH_RATIO, NUMBER);
    registry->AddPhenomenon(METRIC_PREFIX, NUMBER);
    registry->AddPhenomenon(VELOCITY, "LENGTH*TIME(-1)");
    registry->AddPhenomenon(ACCELERATION, "LENGTH*TIME(-2)");
    registry->AddPhenomenon(FORCE, "MASS*ACCELERATION");
    registry->AddPhenomenon(WORK, "FORCE*LENGTH");

    registry->AddUnit(LENGTH, SI, "M", "M", 1, 1, 0);
    registry->AddUnit(MASS, SI, "KG", "KG", 1, 1, 0);
    registry->AddUnit(TIME, SI, "S", "S", 1, 1, 0);
    registry->AddUnit(ANGLE, SI, "RAD", "RAD", 1, 1, 0);
    registry->AddUnit(NUMBER, SI, "ONE", "ONE", 1, 1, 0);

    registry->AddConstant(METRIC_PREFIX, CONSTANT, "DECI", "ONE", 1.0e-1); //, "DECI-prefix");
    registry->AddConstant(METRIC_PREFIX, CONSTANT, "CENTI", "ONE", 1.0e-2); //, "CENTI-prefix");
    registry->AddConstant(METRIC_PREFIX, CONSTANT, "MILLI", "ONE", 1.0e-3); //, "MILLI-prefix");
    registry->AddConstant(METRIC_PREFIX, CONSTANT, "MEGA", "ONE", 1.0e6); //, "MEGA-prefix");

    registry->AddConstant(LENGTH_RATIO, CONSTANT, "PI", "ONE", PI); //, "Ratio of Circumference to its Diameter");

    registry->AddUnit(LENGTH, METRIC, "MM", "[MILLI]*M");
    registry->AddUnit(LENGTH, METRIC, "CM", "[CENTI]*M");
    registry->AddUnit(LENGTH, METRIC, "DM", "[DECI]*M");
    registry->AddUnit(LENGTH, USCUSTOM, "IN", "MM", 25.4); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix B. Section 3.1, Page B-10
    registry->AddUnit(LENGTH, USCUSTOM, "FT", "IN", 12.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    registry->AddUnit(LENGTH, USCUSTOM, "YRD", "FT", 3.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 2, Page C-4
    registry->AddUnit(LENGTH, USCUSTOM, "MILE", "YRD", 1760.0); // Exact, http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8

    registry->AddUnit(ANGLE, METRIC, "ARC_DEG", "[PI]*RAD", 1.0, 180.0 ); // 1/180
    registry->AddUnit(ANGLE, METRIC, "ARC_MINUTE", "ARC_DEG", 1.0, 60.0); // 1/60
    registry->AddUnit(ANGLE, METRIC, "ARC_SECOND", "ARC_DEG", 1.0, 3600.0); // 1/3600 

    registry->AddUnit(TIME, INTERNATIONAL, "MIN", "S", 60.0);
    registry->AddUnit(TIME, INTERNATIONAL, "HR", "MIN", 60.0);

    registry->AddUnit(VELOCITY, USCUSTOM, "MPH", "MILE*HR(-1)");
    registry->AddUnit(VELOCITY, SI, "M/SEC", "M*S(-1)");

    registry->AddUnit(FORCE, SI, "N", "KG*M*S(-2)");

    registry->AddUnit(WORK, SI, "J", "N*M");
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void UnitsTestFixture::SetUp()
    {
    if (nullptr == s_unitsContext)
        {
        s_unitsContext = new UnitRegistry();
        FillRegistry(s_unitsContext);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void UnitsTestFixture::TearDown()
    {
    }

//---------------------------------------------------------------------------------------
// @bsiclass
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
// @bsiclass
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
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UnitsTestFixture::GetECNameFromOldName(Utf8CP unitName, bset<Utf8String>& notMapped)
    {
    Utf8CP newName = UnitNameMappings::TryGetECNameFromOldName(unitName);
    if (nullptr != newName)
        return newName;

    notMapped.insert(unitName);
    return "NULL";
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
void UnitsTestFixture::ReadCSVFile(WCharCP fileName, MultipleTokensProcessor const& lineProcessor)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.c_str(), std::ifstream::in);
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
// @bsimethod
//---------------------------------------------------------------------------------------
void UnitsTestFixture::ReadCSVFile(WCharCP fileName, SingleTokenProcessor const& lineProcessor)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.c_str(), std::ifstream::in);
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
// @bsimethod
//---------------------------------------------------------------------------------------
int UnitsTestFixture::GetCSVFileLineCount(WCharCP fileName)
    {
    Utf8String path = UnitsTestFixture::GetConversionDataPath(fileName);
    std::ifstream ifs(path.c_str(), std::ifstream::in);
    std::string line;

    ASSERT_TRUE(ifs.good()) << "File '" + path + "' does not exist.", 0;

    int count = 0;
    while (std::getline(ifs, line))
        ++count;

    return count;
    }

END_UNITS_UNITTESTS_NAMESPACE
