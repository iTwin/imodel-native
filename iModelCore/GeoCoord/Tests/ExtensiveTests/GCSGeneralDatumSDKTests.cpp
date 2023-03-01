//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#include <Bentley/BeTest.h>

#include <GeoCoord/BaseGeoCoord.h>

#include "GeoCoordExtensiveTestCommon.h"

using namespace ::testing;

class GCSGeneralDatumSDKTests : public ::testing::TestWithParam< Utf8String >
    {   
    public:
        virtual void SetUp() { GeoCoordExtensiveTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordExtensiveTestCommon::Shutdown();};

        GCSGeneralDatumSDKTests() {};
        ~GCSGeneralDatumSDKTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* test all transformation paths are obtainable from datum to WGS84
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralDatumSDKTests, GeodeticTransformForAllDatum)
    {
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfNotSupported = GeoCoordExtensiveTestCommon::GetListOfUnsupportedDatums();

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
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();

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

/*---------------------------------------------------------------------------------**//**
* Test specific to grid file based transformations
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralDatumSDKTests, GeodeticTransformGetAllThroughPath)
    {
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfNotSupported = GeoCoordExtensiveTestCommon::GetListOfUnsupportedDatums();
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");

    ASSERT_TRUE(wgs84 != nullptr);

    for (int index = 0 ; index < listOfDatums.size() ; index++)
        {
        Utf8String theKeyname(listOfDatums[index]);

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
            ASSERT_TRUE(theDatum != NULL && theDatum->IsValid());

            // Create a Datum converter between the two datums
            GeoCoordinates::GeodeticTransformPathP theTransformPath = GeoCoordinates::GeodeticTransformPath::Create(*theDatum, *wgs84);

            // Check converter properties
            ASSERT_TRUE(theTransformPath != NULL);

            for (int indexTrf = 0 ; indexTrf < theTransformPath->GetGeodeticTransformCount() ; indexTrf++)
                {
                GeoCoordinates::GeodeticTransformCP theTransform = theTransformPath->GetGeodeticTransform(indexTrf);

                ASSERT_TRUE(theTransform != NULL);

                EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataAvailable) << "Data unavailable for " << theKeyname.c_str();
                }

            theTransformPath->Destroy();
            theDatum->Destroy();
            }
        }

        wgs84->Destroy();
    }
    
    
    /*---------------------------------------------------------------------------------**//**
* Test specific to grid file based transformations
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSGeneralDatumSDKTests, GeodeticTransformGetAllGridFileBasedTransforms)
    {
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfNotSupported = GeoCoordExtensiveTestCommon::GetListOfUnsupportedDatums();
    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");

    for (int index = 0 ; index < listOfDatums.size() ; index++)
        {
        Utf8String theKeyname(listOfDatums[index]);

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
            ASSERT_TRUE(theDatum != NULL && theDatum->IsValid());

            // Create a Datum converter between the two datums
            GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*theDatum, *wgs84);

            // Check converter properties
            ASSERT_TRUE(theConverter != NULL);

            for (int indexTrf = 0 ; indexTrf < theConverter->GetGeodeticTransformCount() ; indexTrf++)
                {
                GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(indexTrf);

                ASSERT_TRUE(theTransform != NULL);

                EXPECT_TRUE(theTransform->GetDataAvailability() == GeoCoordinates::GeodeticTransformDataAvailability::DataAvailable);

                if(theTransform->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_GFILE)
                    {
                    size_t numGrid = theTransform->GetGridFileDefinitionCount();

                    for (size_t indexGrid = 0 ; indexGrid < numGrid ; ++indexGrid)
                        {
                        GeoCoordinates::GridFileDefinition theGridFile = theTransform->GetGridFileDefinition(indexGrid);

                        GeoCoordinates::GridFileDirection theDirection = theGridFile.GetDirection();

                        int directionInt = (int)theDirection;

                        if (directionInt != 0x49 && directionInt != 0x46)
                            EXPECT_TRUE(false);
                        }
                    }

                theTransform->Destroy();
                }
            theConverter->Destroy();
            theDatum->Destroy();
            }
        }

        wgs84->Destroy();
    }


#if (0) // Deactivated to introduce new dataset (to be reactivated afterwards)
/*---------------------------------------------------------------------------------**//**
* Checks all transform paths are non multiple once Null Transformas are eliminated
* except the few know muli transforms.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (GCSUnitTests, GeodeticTransformGetAllNonMultiTransforms)
    {
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfNotSupported = GeoCoordExtensiveTestCommon::GetListOfUnsupportedDatums();

    GeoCoordinates::DatumCP wgs84 = GeoCoordinates::Datum::CreateDatum("WGS84");

    for (int index = 0 ; index < listOfDatums.size() ; index++)
        {
        Utf8String theKeyname(listOfDatums[index]);

        // Check if not in exclusion list (datum that require grid files not installed by default
        bool found = false;
        for (int index2 = 0 ; !found && (index2 < listOfNotSupported.size()) ; index2++)
            {
            if (theKeyname.CompareTo(listOfNotSupported[index2]) == 0)
                found = true;
            }

        if (!found)
            {

            // Check if not in exclusion list (datum that require grid files not installed by default
            bool foundKnownDualMultiTransform = false;
            for (int index3 = 0 ; !foundKnownDualMultiTransform && (index3 < s_listOfKnownMultiTransform.size()) ; index3++)
                {
                if (theKeyname.CompareTo(s_listOfKnownMultiTransform[index3]) == 0)
                    foundKnownDualMultiTransform = true;
                }

            if (!foundKnownDualMultiTransform)
                {
                GeoCoordinates::DatumCP theDatum = GeoCoordinates::Datum::CreateDatum(theKeyname.c_str());

                // Check transformation properties
                ASSERT_TRUE(theDatum != NULL && theDatum->IsValid());

                // Create a Datum converter between the two datums
                GeoCoordinates::DatumConverterP theConverter = GeoCoordinates::DatumConverter::Create(*theDatum, *wgs84, GeoCoordinates::vdcGeoid, GeoCoordinates::vdcGeoid);

                // Check converter properties
                ASSERT_TRUE(theConverter != NULL);

                int countNonNullTrf = 0;
                for (int indexTrf = 0 ; indexTrf < theConverter->GetGeodeticTransformCount() ; indexTrf++)
                    {
                    GeoCoordinates::GeodeticTransformP theTransform = theConverter->GetGeodeticTransform(indexTrf);

                    ASSERT_TRUE(theTransform != NULL);

                    if (!theTransform->IsNullTransform())
                        countNonNullTrf++;

                    theTransform->Destroy();
                    }

                EXPECT_TRUE(countNonNullTrf == 1 || countNonNullTrf == 0);
                theConverter->Destroy();
                theDatum->Destroy();
                }
            }
        }
        wgs84->Destroy();
    }
#endif


/*---------------------------------------------------------------------------------**//**
* test all transformation to json then back
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GCSGeneralDatumSDKTests, DatumAdditionalTransform)
{
    const bvector<Utf8String>& listOfDatums = GeoCoordExtensiveTestCommon::GetListOfDatums();
    const bvector<Utf8String>& listOfAdditionalPathsDatum = GeoCoordExtensiveTestCommon::GetListOfDatumsWithAdditionalPaths();

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

