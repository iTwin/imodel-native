//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

class GCSGeneralDatumSDKTests : public ::testing::TestWithParam< Utf8String >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        GCSGeneralDatumSDKTests() {};
        ~GCSGeneralDatumSDKTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* test all transformation paths are obtainable from datum to WGS84
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralDatumSDKTests, GeodeticTransformForAllDatum)
    {
    const bvector<Utf8String>& listOfDatums = GeoCoordTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfNotSupported = GeoCoordTestCommon::GetListOfUnsupportedDatums();

    for (int index = 0 ; index < listOfDatums.size() ; index++)
        {
        Utf8String theKeyname(listOfDatums[index]);
        static GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");

        // Check if not in exclusion list (datum that require grid files not installed by default
        bool found = false;
        for (int index2 = 0 ; !found && (index2 < listOfNotSupported.size()) ; index2++)
            {
            if (theKeyname.CompareTo(listOfNotSupported[index2]) == 0)
                found = true;
            }

        if (!found)
            {
            GeoCoordinates::DatumCP theDatum = GeoCoordinates::Datum::CreateDatum(theKeyname.c_str());
    
            // Check transformation properties
            ASSERT_TRUE(theDatum != NULL && theDatum->IsValid()) << "Datum: " << theKeyname.c_str();
    
            // Create a Datum converter between the two datums
            GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*theDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

            // Check converter properties
            ASSERT_TRUE(theConverter != NULL) << "Datum: " << theKeyname.c_str();

            // Expect any transformation not to be fully empty (except from self)
            // TODO This text causes intermittent issues on PRG machines ... possibly a file concurrent access related failure.
            if(!(theConverter->GetGeodeticTransformCount() > 0) && (theKeyname.CompareTo("WGS84") != 0))
                EXPECT_TRUE(false); 
        
            theConverter->Destroy();
            theDatum->Destroy();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* test all transformation to json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumTransformtoJsonThenBack)
{
    const bvector<Utf8String>& listOfDatums = GeoCoordTestCommon::GetListOfDatums();

    for (int index = 0; index < listOfDatums.size(); index++)
    {
        Utf8String theKeyname(listOfDatums[index]);
        GeoCoordinates::DatumCP currentDatum = GeoCoordinates::Datum::CreateDatum(theKeyname.c_str());

        printf("Processing Datum: %s\n", theKeyname.c_str());
        if (currentDatum != nullptr && currentDatum->IsValid())
        {
            Json::Value result;

            if (SUCCESS == currentDatum->ToJson(result, true))
            {
                // Sabotage GCS name to make sure everything is parsed
                result["id"] = "XYZ";

                // Transform to string (for debug purposes)
                Utf8String resultString = result.toStyledString();

                GeoCoordinates::DatumP resultDatum = GeoCoordinates::Datum::CreateDatum();

                Utf8String errMessage;
                EXPECT_EQ(SUCCESS, resultDatum->FromJson(result, errMessage)) << errMessage.c_str();

                if (currentDatum->GetConvertToWGS84MethodCode() != GeoCoordinates::ConvertType_MREG) // Cannot compare multiple regressions
                    EXPECT_TRUE(currentDatum->IsEquivalent(*resultDatum));

                resultDatum->Destroy();
            }
        }
        
        if (currentDatum != nullptr)
            currentDatum->Destroy();
    }
}

class datumPair {
public:
bool m_instancable; // Inidcates that the path exists but a DatumConverter cannot be created due to grid files missing
Utf8String m_source;
Utf8String m_target;
};

bvector<datumPair> listOfDatumPairs = {
{true, "AGD66" , "ASTRLA66-Grid"},
{true, "AGD66" , "EPSG:6202"},
{true, "AGD66" , "GDA94"},
{true, "AGD84" , "ASTRLA84-Grid"},
{true, "AGD84" , "EPSG:6203"},
{true, "AGD84" , "GDA94"},
{true, "ASTRLA66-Grid" , "EPSG:6283"},
{true, "ASTRLA66-Grid" , "GDA94"},
{true, "ASTRLA84-Grid" , "EPSG:6283"},
{true, "ASTRLA84-Grid" , "GDA94"},
{false, "ATS77" , "CSRS"},
{false, "ATS77" , "NAD27"},
{true, "CAPE-1" , "Hartebeesthoek1994"},
{false, "CAPE/GSB" , "Hartebeesthoek1994"},
{true, "CH1903/GSB" , "CH1903Plus_1"},
{true, "CH1903/GSB" , "CH1903Plus_2"},
{true, "CH1903Plus_1" , "CHTRF95"},
{true, "CH1903Plus_2" , "CHTRF95"},
{true, "CHTRF95" , "ETRF89"},
{true, "CORREGO-1961-A" , "SIRGAS2000"},
{true, "CORREGO-70-72-BZ/GSB" , "SIRGAS2000"},
{true, "CORREGO-7072-A" , "SIRGAS2000"},
{true, "CORREGO1961-BZ/GSB" , "SIRGAS2000"},
{false, "CSRS" , "EPSG:6140"},
{true, "Datum73-Mod/b" , "ETRF89"},
{true, "Datum73-Mod/b" , "WGS84"},
{true, "DGN95" , "WGS84"},
{true, "DHDN/BeTA2010" , "ETRF89"},
{true, "DHDN/BeTA" , "ETRF89"},
{true, "DHDN/HeTA2010" , "ETRF89"},
{true, "ECML14_NB-IRF" , "ETRF89"},
{true, "ECML14_NB-IRF" , "ETRS89"},
{true, "ED50-IGN.ES" , "ETRF89"},
{true, "EOS21-IRF" , "ETRF89"},
{true, "EOS21-IRF" , "ETRS89"},
{true, "EPSG:6171" , "EPSG:6275"},
{false, "EPSG:6122" , "CSRS"},
{true, "EPSG:6135" , "NAD83"},
{true, "EPSG:6136" , "NAD83"},
{true, "EPSG:6137" , "NAD83"},
{true, "EPSG:6138" , "NAD83"},
{true, "EPSG:6139" , "NAD27"},
{true, "EPSG:6139" , "NAD83"},
{true, "EPSG:6152" , "HPGN"},
{true, "EPSG:6171" , "NTF"},
{true, "EPSG:6171" , "NTF-G-Grid"},
{true, "EPSG:6171" , "NTF-G-Grid-ClrkIGN"},
{true, "EPSG:6171" , "WGS84"},
{true, "EPSG:6202" , "GDA94"},
{true, "EPSG:6203" , "GDA94"},
{true, "EPSG:6267" , "NAD83"},
{true, "EPSG:6269" , "HPGN"},
{true, "EPSG:6272" , "NZGD2000"},
{false, "EPSG:6608" , "NAD83"},
{false, "EPSG:6609" , "CSRS"},
{false, "EPSG:6609" , "NAD83"},
{true, "ERP50-Grid" , "ETRF89"},
{true, "FCS_DEN" , "ETRF89"},
{true, "GBK19-IRF" , "ETRF89"},
{true, "GBK19-IRF" , "ETRS89"},
{true, "GDA94/GSB" , "GDA2020"},
{false, "GENGRID-WGS84" , "GENGRID-WGS84"},
{true, "HS2SD_2002" , "ETRF89"},
{true, "HS2SD_2002" , "ETRS89"},
{true, "HS2SD_2015" , "ETRF89"},
{true, "HS2SD_2015" , "ETRS89"},
{true, "JGD2000-7P" , "JGD2011"},
{true, "JGD2000-MOD" , "JGD2000"},
{true, "JGD2000" , "JGD2011"},
{true, "JPNGSI-Grid" , "EPSG:6612"},
{true, "JPNGSI-Grid" , "JGD2000"},
{true, "Lisbon37/b" , "ETRF89"},
{true, "MICHIGAN" , "NAD83"},
{true, "MML07-IRF" , "ETRF89"},
{true, "MML07-IRF" , "ETRS89"},
{true, "MOLDOR11-IRF" , "ETRF89"},
{true, "MOLDOR11-IRF" , "ETRS89"},
{true, "MRH21-IRF" , "ETRF89"},
{true, "MRH21-IRF" , "ETRS89"},
{false, "NAD27/1976" , "NAD83"},
{false, "NAD27/CGQ77-83" , "NAD83"},
{false, "NAD27/CGQ77-98" , "CSRS"},
{false, "NAD27" , "ATS77"},
{true, "NAD27" , "EPSG:6267"},
{true, "NAD27" , "NAD83"},
{true, "NAD83/HARN-A" , "NSRS07"},
{true, "NAD83/HARN-A" , "WGS84"},
{true, "NAD83/HARN" , "NSRS07"},
{false, "NAD83" , "CSRS"},
{false,"NAD83" , "EPSG:6140"},
{true, "NAD83" , "EPSG:6152"},
{true, "NAD83" , "HARN/02"},
{true, "NAD83" , "HARN/10"},
{true, "NAD83" , "HARN/11"},
{true, "NAD83" , "HARN/12"},
{true, "NAD83" , "HARN/13"},
{true, "NAD83" , "HARN/14"},
{true, "NAD83" , "HARN/15"},
{true, "NAD83" , "HARN/16"},
{true, "NAD83" , "HARN/17"},
{true, "NAD83" , "HARN/18"},
{true, "NAD83" , "HARN/19"},
{true, "NAD83" , "HARN/AL"},
{true, "NAD83" , "HARN/AR"},
{true, "NAD83" , "HARN/AZ"},
{true, "NAD83" , "HARN/CA"},
{true, "NAD83" , "HARN/CO"},
{true, "NAD83" , "HARN/FL"},
{true, "NAD83" , "HARN/GA"},
{true, "NAD83" , "HARN/GU"},
{true, "NAD83" , "HARN/HI"},
{true, "NAD83" , "HARN/IA"},
{true, "NAD83" , "HARN/IL"},
{true, "NAD83" , "HARN/IN"},
{true, "NAD83" , "HARN/KS"},
{true, "NAD83" , "HARN/KY"},
{true, "NAD83" , "HARN/LA"},
{true, "NAD83" , "HARN/MD"},
{true, "NAD83" , "HARN/ME"},
{true, "NAD83" , "HARN/MI"},
{true, "NAD83" , "HARN/MN"},
{true, "NAD83" , "HARN/MO"},
{true, "NAD83" , "HARN/MS"},
{true, "NAD83" , "HARN/MT"},
{true, "NAD83" , "HARN/NB"},
{true, "NAD83" , "HARN/NC"},
{true, "NAD83" , "HARN/ND"},
{true, "NAD83" , "HARN/NewEngl"},
{true, "NAD83" , "HARN/NJ"},
{true, "NAD83" , "HARN/NM"},
{true, "NAD83" , "HARN/NV"},
{true, "NAD83" , "HARN/NY"},
{true, "NAD83" , "HARN/OH"},
{true, "NAD83" , "HARN/OK"},
{true, "NAD83" , "HARN/PA"},
{true, "NAD83" , "HARN/PV"},
{true, "NAD83" , "HARN/SA"},
{true, "NAD83" , "HARN/SC"},
{true, "NAD83" , "HARN/SD"},
{true, "NAD83" , "HARN/TN"},
{true, "NAD83" , "HARN/TX"},
{true, "NAD83" , "HARN/UT"},
{true, "NAD83" , "HARN/VA"},
{true, "NAD83" , "HARN/WI"},
{true, "NAD83" , "HARN/WO"},
{true, "NAD83" , "HARN/WV"},
{true, "NAD83" , "HARN/WY"},
{true, "NAD83" , "HPGN"},
{true, "NAD83" , "NAD83/HARN"},
{true, "NAD83" , "NAD83/HARN-A"},
{true, "NSRS07" , "NAD83/2011"},
{true, "NSRS07" , "NSRS11"},
{true, "NZGD49" , "NZGD2000"},
{true, "Observatorio65" , "ITRF2005-Macau"},
{true, "OldHawaiian" , "NAD83"},
{true, "OSGB/OSTN02" , "ETRF89"},
{true, "OSGB/OSTN15" , "ETRF89"},
{true, "PuertoRico" , "NAD83"},
{true, "RGF93" , "EPSG:6275"},
{true, "RGF93" , "NTF"},
{true, "RGF93" , "NTF-G-Grid"},
{true, "RGF93" , "NTF-G-Grid-ClrkIGN"},
{true, "SA1969-BZ-A" , "SIRGAS2000"},
{true, "SAD69-BZ/GSB" , "SIRGAS2000"},
{true, "SAD69/96-BZ/GSB" , "SIRGAS2000"},
{true, "Slov/JTSK-A" , "ETRF89"},
{true, "Slov/JTSK-A" , "ETRS89"},
{true, "Slov/JTSK03" , "ETRF89"},
{true, "Slov/JTSK03" , "Slov/JTSK"},
{true, "Slov/JTSK03" , "Slov/JTSK-NULL"},
{true, "SVY21" , "WGS84"},
{true, "Tokyo-Grid" , "EPSG:6612"},
{true, "Tokyo-Grid" , "JGD2000"},
{true, "TPEN11-IRF" , "ETRF89"},
{true, "TPEN11-IRF" , "ETRS89"},
{true, "XR09SD_2002" , "ETRF89"},
{true, "XR09SD_2002" , "ETRS89"},
{true, "XR09SD_2015" , "ETRF89"},
{true, "XR09SD_2015" , "ETRS89"},
//--------------------
{true, "AGD66" , "EPSG:6202"},
{true, "AGD84" , "EPSG:6203"},
{true, "ECML14_NB-IRF" , "OSGB/OSTN15"},
{true, "EOS21-IRF" , "OSGB/OSTN15"},
{false, "EPSG:6267" , "CSRS"},
{true, "EPSG:6326" , "HPGN"},
{true, "GBK19-IRF" , "OSGB/OSTN15"},
{true, "HS2SD_2015" , "HS2SD_2002"},
{true, "HS2SD_2002" , "OSGB/OSTN02"},
{true, "HS2SD_2015" , "OSGB/OSTN15"},
{true, "MML07-IRF" , "OSGB/OSTN15"},
{true, "MOLDOR11-IRF" , "OSGB/OSTN15"},
{true, "MRH21-IRF" , "OSGB/OSTN15"},
{false, "NAD27" , "CSRS"},
{true, "NAD27" , "HPGN"},
{true, "NAD83/2011" , "NAD83"},
{true, "NAD83/2011" , "NAD83/HARN"},
{true, "NAD83/2011" , "NAD83/HARN-A"},
{true, "NSRS07" , "NAD83"},
{true, "NSRS11" , "NAD83"},
{true, "NSRS11" , "NAD83/HARN"},
{true, "NSRS11" , "NAD83/HARN-A"},
{true, "Slov/JTSK" , "ETRF89"},
{true, "Slov/JTSK" , "ETRS89"},
{true, "Tokyo-Grid" , "JGD2011"},
{true, "Tokyo" , "JGD2011"},
{true, "TPEN11-IRF" , "OSGB/OSTN15"},
{true, "XR09SD_2002" , "OSGB/OSTN02"},
{true, "XR09SD_2015" , "OSGB/OSTN15"},
{true, "XR09SD_2015" , "XR09SD_2002"}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumAdditionalTransformAllTests)
{
    for (auto thePair : listOfDatumPairs)
        {
        GeoCoordinates::DatumCP sourceDatum = GeoCoordinates::Datum::CreateDatum(thePair.m_source.c_str());
        GeoCoordinates::DatumCP targetDatum = GeoCoordinates::Datum::CreateDatum(thePair.m_target.c_str());

        if (sourceDatum != nullptr && sourceDatum->IsValid() && targetDatum != nullptr && targetDatum->IsValid())
            {
            if (thePair.m_instancable)
                {
                // Create a Datum converter between the two datums
                GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*sourceDatum, *targetDatum, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

                // Check converter properties
                EXPECT_TRUE(theConverter != NULL);
                
                if (theConverter != nullptr)
                    theConverter->Destroy();
                }

            GeoCoordinates::GeodeticTransformPathCP newPath = GeoCoordinates::GeodeticTransformPath::Create(*sourceDatum, *targetDatum);
            EXPECT_TRUE(nullptr != newPath);
            if (nullptr != newPath)
                newPath->Destroy();

            sourceDatum->Destroy();
            targetDatum->Destroy();
            }
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumAdditionalTransformAllTestsToWgs84)
{
    GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");

    for (auto thePair : listOfDatumPairs)
    {
        GeoCoordinates::DatumCP sourceDatum = GeoCoordinates::Datum::CreateDatum(thePair.m_source.c_str());
        GeoCoordinates::DatumCP targetDatum = GeoCoordinates::Datum::CreateDatum(thePair.m_target.c_str());

        if (sourceDatum != nullptr && sourceDatum->IsValid() && targetDatum != nullptr && targetDatum->IsValid())
        {
            if (thePair.m_instancable)
            {
                // Create a Datum converter between the two datums
                GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*sourceDatum, *wgs84Datum, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

                // Check converter properties
                EXPECT_TRUE(theConverter != NULL);

                if (theConverter != nullptr)
                    theConverter->Destroy();
                
                GeoCoordinates::DatumConverterP theConverter2 = GeoCoordinates::DatumConverter::Create(*targetDatum, *wgs84Datum, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

                // Check converter properties
                EXPECT_TRUE(theConverter2 != NULL);
                
                if (theConverter2 != nullptr)
                    theConverter2->Destroy();
            }

            GeoCoordinates::GeodeticTransformPathCP newPath = GeoCoordinates::GeodeticTransformPath::Create(*sourceDatum, *wgs84Datum);
            EXPECT_TRUE(nullptr != newPath);
            if (nullptr != newPath)
                newPath->Destroy();

            newPath = GeoCoordinates::GeodeticTransformPath::Create(*targetDatum, *wgs84Datum);
            EXPECT_TRUE(nullptr != newPath);
            if (nullptr != newPath)
                newPath->Destroy();

            sourceDatum->Destroy();
            targetDatum->Destroy();
        }
    }
    wgs84Datum->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* test all transformation to json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumAdditionalTransform)
{
    const bvector<Utf8String>& listOfDatums = GeoCoordTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfAdditionalPathsDatum = GeoCoordTestCommon::GetListOfDatumsWithAdditionalPaths();

    for (int index = 0; index < listOfDatums.size(); index++)
    {
        Utf8String theKeyname(listOfDatums[index]);
        GeoCoordinates::DatumCP currentDatum = GeoCoordinates::Datum::CreateDatum(theKeyname.c_str());

        if (currentDatum != nullptr && currentDatum->IsValid())
        {
        // Check if not in known additional path datum list 
        bool shouldHaveAdditionalPaths = false;
        for (auto curName: listOfAdditionalPathsDatum)
            {
            if (theKeyname.CompareTo(curName) == 0)
                {
                shouldHaveAdditionalPaths = true;
                break;
                }
            }

            // If they have additional paths then they should have an reverse
            if (shouldHaveAdditionalPaths)
                EXPECT_TRUE(currentDatum->GetAdditionalGeodeticTransformPaths().size() > 0);

        }
        if (currentDatum != nullptr)
            currentDatum->Destroy();

    }
}

/*---------------------------------------------------------------------------------**//**
* This tests verifies that datum transformations between hS2 and Network Rail work
* correctly to OSTN/2015 - OSTN/2002 without a path definition.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumAdditionalTransformBiStageWithoutPath)
    {
    bvector<Utf8String> listOfNR_HS2_Datums = {
        "MRH21-IRF",
        "MOLDOR11-IRF",
        "MML07-IRF",
        "EOS21-IRF",
        "ECML14_NB-IRF",
        "GBK19-IRF",
        "TPEN11-IRF",
        "XR09SD_2015",
        "XR09SD_2002",
        "HS2SD_2015",
        "HS2SD_2002",
    };

    GeoCoordinates::DatumCP OSTN15Datum = GeoCoordinates::Datum::CreateDatum("OSGB/OSTN15");
    GeoCoordinates::DatumCP OSTN02Datum = GeoCoordinates::Datum::CreateDatum("OSGB/OSTN02");
    GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");

    for (auto datumName : listOfNR_HS2_Datums)
        {
        GeoCoordinates::DatumCP currentDatum = GeoCoordinates::Datum::CreateDatum(datumName.c_str());

        if (currentDatum != nullptr && currentDatum->IsValid())
            {
            EXPECT_TRUE(currentDatum->GetAdditionalGeodeticTransformPaths().size() == 0);

            GeoCoordinates::GeodeticTransformPathCP pathToOSTN15 = GeoCoordinates::GeodeticTransformPath::Create(*currentDatum, *OSTN15Datum);
            GeoCoordinates::GeodeticTransformPathCP pathToOSTN02 = GeoCoordinates::GeodeticTransformPath::Create(*currentDatum, *OSTN02Datum);
            GeoCoordinates::GeodeticTransformPathCP pathToWGS84 = GeoCoordinates::GeodeticTransformPath::Create(*currentDatum, *wgs84Datum);

            GeoCoordinates::GeodeticTransformPathCP pathFromOSTN15 = GeoCoordinates::GeodeticTransformPath::Create(*OSTN15Datum, *currentDatum);
            GeoCoordinates::GeodeticTransformPathCP pathFromOSTN02 = GeoCoordinates::GeodeticTransformPath::Create(*OSTN02Datum, *currentDatum);
            GeoCoordinates::GeodeticTransformPathCP pathFromWGS84 = GeoCoordinates::GeodeticTransformPath::Create(*wgs84Datum, *currentDatum);

            ASSERT_TRUE(nullptr != pathToOSTN15);
            ASSERT_TRUE(nullptr != pathToOSTN02);
            ASSERT_TRUE(nullptr != pathToWGS84);

            ASSERT_TRUE(nullptr != pathFromOSTN15);
            ASSERT_TRUE(nullptr != pathFromOSTN02);
            ASSERT_TRUE(nullptr != pathFromWGS84);

            ASSERT_TRUE(pathToOSTN15->GetGeodeticTransformCount() == 4);
            ASSERT_TRUE(pathToOSTN02->GetGeodeticTransformCount() == 4);
            ASSERT_TRUE(pathToWGS84->GetGeodeticTransformCount() == 2);

            ASSERT_TRUE(pathFromOSTN15->GetGeodeticTransformCount() == 4);
            ASSERT_TRUE(pathFromOSTN02->GetGeodeticTransformCount() == 4);
            ASSERT_TRUE(pathFromWGS84->GetGeodeticTransformCount() == 2);

            GeoCoordinates::GeodeticTransformPathP pathToOSTN15Reversed = pathToOSTN15->Clone();
            EXPECT_TRUE(SUCCESS == pathToOSTN15Reversed->Reverse());
            GeoCoordinates::GeodeticTransformPathP pathToOSTN02Reversed = pathToOSTN02->Clone();
            EXPECT_TRUE(SUCCESS == pathToOSTN02Reversed->Reverse());
            GeoCoordinates::GeodeticTransformPathP pathToWGS84Reversed = pathToWGS84->Clone();
            EXPECT_TRUE(SUCCESS == pathToWGS84Reversed->Reverse());

            EXPECT_TRUE(pathFromOSTN15->IsEquivalent(*pathToOSTN15Reversed));
            EXPECT_TRUE(pathFromOSTN02->IsEquivalent(*pathToOSTN02Reversed));
            EXPECT_TRUE(pathFromWGS84->IsEquivalent(*pathToWGS84Reversed));

            EXPECT_TRUE(Utf8String(pathToOSTN15->GetSourceDatumName()) == datumName);
            EXPECT_TRUE(Utf8String(pathToOSTN15->GetTargetDatumName()) == "OSGB/OSTN15");
            EXPECT_TRUE(Utf8String(pathToOSTN02->GetSourceDatumName()) == datumName);
            EXPECT_TRUE(Utf8String(pathToOSTN02->GetTargetDatumName()) == "OSGB/OSTN02");
            EXPECT_TRUE(Utf8String(pathToWGS84->GetSourceDatumName()) == datumName);
            EXPECT_TRUE(Utf8String(pathToWGS84->GetTargetDatumName()) == "WGS84");
            
            pathToOSTN15->Destroy();
            pathToOSTN02->Destroy();
            pathToWGS84->Destroy();
            pathFromOSTN15->Destroy();
            pathFromOSTN02->Destroy();
            pathFromWGS84->Destroy();
            pathToOSTN15Reversed->Destroy();
            pathToOSTN02Reversed->Destroy();
            pathToWGS84Reversed->Destroy();
            }
            
        if (currentDatum != nullptr)
            currentDatum->Destroy();

        }

        OSTN15Datum->Destroy();
        OSTN02Datum->Destroy();
        wgs84Datum->Destroy();
    }

    /*---------------------------------------------------------------------------------**//**
    * This tests verifies that datum transformations between XR09 version is possible
    * without paths.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(GCSGeneralDatumSDKTests, DatumXR09WithoutPath)
    {
        GeoCoordinates::DatumCP XR09_2015Datum = GeoCoordinates::Datum::CreateDatum("XR09SD_2015");
        GeoCoordinates::DatumCP XR09_2002Datum = GeoCoordinates::Datum::CreateDatum("XR09SD_2002");
        GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");

        GeoCoordinates::GeodeticTransformPathCP pathCrossXR09 = GeoCoordinates::GeodeticTransformPath::Create(*XR09_2015Datum, *XR09_2002Datum);
        GeoCoordinates::GeodeticTransformPathCP pathCrossXR09Inv = GeoCoordinates::GeodeticTransformPath::Create(*XR09_2002Datum, *XR09_2015Datum);

        ASSERT_TRUE(nullptr != pathCrossXR09);
        ASSERT_TRUE(nullptr != pathCrossXR09Inv);

        GeoCoordinates::GeodeticTransformPathCP path15ToWGS84 = GeoCoordinates::GeodeticTransformPath::Create(*XR09_2015Datum, *wgs84Datum);
        GeoCoordinates::GeodeticTransformPathCP pathWGS84To02 = GeoCoordinates::GeodeticTransformPath::Create(*wgs84Datum, *XR09_2002Datum);

        ASSERT_TRUE(nullptr != path15ToWGS84);
        ASSERT_TRUE(nullptr != pathWGS84To02);

        GeoCoordinates::GeodeticTransformPathCP mergedPath = GeoCoordinates::GeodeticTransformPath::CreateMerged(*path15ToWGS84, *pathWGS84To02);

        ASSERT_TRUE(nullptr != mergedPath);

        ASSERT_TRUE(pathCrossXR09->GetGeodeticTransformCount() == 4);
        ASSERT_TRUE(pathCrossXR09Inv->GetGeodeticTransformCount() == 4);
        ASSERT_TRUE(path15ToWGS84->GetGeodeticTransformCount() == 2);
        ASSERT_TRUE(pathWGS84To02->GetGeodeticTransformCount() == 2);
        ASSERT_TRUE(mergedPath->GetGeodeticTransformCount() == 4);

        EXPECT_TRUE(pathCrossXR09->IsEquivalent(*mergedPath));

        GeoCoordinates::GeodeticTransformPathP pathCrossXR09Reversed = pathCrossXR09->Clone();
        EXPECT_TRUE(SUCCESS == pathCrossXR09Reversed->Reverse());

        EXPECT_TRUE(pathCrossXR09Reversed->IsEquivalent(*pathCrossXR09Inv));

        XR09_2015Datum->Destroy();
        XR09_2002Datum->Destroy();
        wgs84Datum->Destroy();
        pathCrossXR09->Destroy();
        pathCrossXR09Inv->Destroy();
        path15ToWGS84->Destroy();
        pathWGS84To02->Destroy();
        mergedPath->Destroy();
        pathCrossXR09Reversed->Destroy();
    }

    /*---------------------------------------------------------------------------------**//**
    * This tests verifies that datum transformations between HS2 version is possible
    * without paths.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST_F(GCSGeneralDatumSDKTests, DatumHS2WithoutPath)
    {
        GeoCoordinates::DatumCP HS2_2015Datum = GeoCoordinates::Datum::CreateDatum("HS2SD_2015");
        GeoCoordinates::DatumCP HS2_2002Datum = GeoCoordinates::Datum::CreateDatum("HS2SD_2002");
        GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");

        GeoCoordinates::GeodeticTransformPathCP pathCrossHS2 = GeoCoordinates::GeodeticTransformPath::Create(*HS2_2015Datum, *HS2_2002Datum);
        GeoCoordinates::GeodeticTransformPathCP pathCrossHS2Inv = GeoCoordinates::GeodeticTransformPath::Create(*HS2_2002Datum, *HS2_2015Datum);

        ASSERT_TRUE(nullptr != pathCrossHS2);
        ASSERT_TRUE(nullptr != pathCrossHS2Inv);

        GeoCoordinates::GeodeticTransformPathCP path15ToWGS84 = GeoCoordinates::GeodeticTransformPath::Create(*HS2_2015Datum, *wgs84Datum);
        GeoCoordinates::GeodeticTransformPathCP pathWGS84To02 = GeoCoordinates::GeodeticTransformPath::Create(*wgs84Datum, *HS2_2002Datum);

        ASSERT_TRUE(nullptr != path15ToWGS84);
        ASSERT_TRUE(nullptr != pathWGS84To02);

        GeoCoordinates::GeodeticTransformPathCP mergedPath = GeoCoordinates::GeodeticTransformPath::CreateMerged(*path15ToWGS84, *pathWGS84To02);

        ASSERT_TRUE(nullptr != mergedPath);

        ASSERT_TRUE(pathCrossHS2->GetGeodeticTransformCount() == 4);
        ASSERT_TRUE(pathCrossHS2Inv->GetGeodeticTransformCount() == 4);
        ASSERT_TRUE(path15ToWGS84->GetGeodeticTransformCount() == 2);
        ASSERT_TRUE(pathWGS84To02->GetGeodeticTransformCount() == 2);
        ASSERT_TRUE(mergedPath->GetGeodeticTransformCount() == 4);

        EXPECT_TRUE(pathCrossHS2->IsEquivalent(*mergedPath));

        GeoCoordinates::GeodeticTransformPathP pathCrossHS2Reversed = pathCrossHS2->Clone();
        EXPECT_TRUE(SUCCESS == pathCrossHS2Reversed->Reverse());

        EXPECT_TRUE(pathCrossHS2Reversed->IsEquivalent(*pathCrossHS2Inv));

        HS2_2015Datum->Destroy();
        HS2_2002Datum->Destroy();
        wgs84Datum->Destroy();
        pathCrossHS2->Destroy();
        pathCrossHS2Inv->Destroy();
        path15ToWGS84->Destroy();
        pathWGS84To02->Destroy();
        mergedPath->Destroy();
        pathCrossHS2Reversed->Destroy();
    }

