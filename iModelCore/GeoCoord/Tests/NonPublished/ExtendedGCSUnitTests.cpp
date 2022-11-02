//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <Bentley/Desktop/FileSystem.h>

#include <GeoCoord/BaseGeoCoord.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class ExtendedGCSUnitTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        ExtendedGCSUnitTests() {};
        ~ExtendedGCSUnitTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* Basic tests for Library Manager
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, WKParseTestNotEqual)
    {
    GeoCoordinates::BaseGCSPtr currentGCS;

    currentGCS = GeoCoordinates::BaseGCS::CreateGCS("UTM84-18N");

    GeoCoordinates::BaseGCSPtr currentGCS2 = GeoCoordinates::BaseGCS::CreateGCS();

    currentGCS2->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, "COMPD_CS[\"UTM84-18N\",PROJCS[\"UTM84-18N\",GEOGCS[\"LL84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137.000,298.25722293]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Universal Transverse Mercator System\"],PARAMETER[\"UTM Zone Number (1 - 60)\",18.0],PARAMETER[\"Hemisphere, North or South\",1.0],UNIT[\"Meter\",1.00000000000000]],VERT_CS[\"Ellipsoid Height\",VERT_DATUM[\"Ellipsoid\",2002],UNIT[\"METER\",1.000000]]]");

    EXPECT_TRUE(currentGCS2->IsEquivalent(*currentGCS));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, WKParseTest1)
    {
    GeoCoordinates::BaseGCSPtr currentGCS2 = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(SUCCESS == currentGCS2->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC,
        R"(COMPD_CS["IBR Project Datum",PROJCS["IBR Project Datum",GEOGCS["HPGN",DATUM["HPGN",SPHEROID["GRS1980",6378137.000,298.25722210]],PRIMEM["Greenwich",0],UNIT["Degree",0.017453292519943295]],PROJECTION["Lambert Conformal Conic (2SP) with Affine Post Process"],PARAMETER["False Easting",1640416.670],PARAMETER["False Northing",0.000],PARAMETER["Central Meridian",-120.50000000000000],PARAMETER["Origin Latitude",45.33333333333334],PARAMETER["Northern Standard Parallel",47.33333333333334],PARAMETER["Southern Standard Parallel",45.83333333333334],PARAMETER["Affine Transformation A0 Coefficient",0.000000000000],PARAMETER["Affine Transformation B0 Coefficient",0.000000000000],PARAMETER["Affine Transformation A1 Coefficient",0.999942400000],PARAMETER["Affine Transformation A2 Coefficient",0.000000000000],PARAMETER["Affine Transformation B1 Coefficient",0.000000000000],PARAMETER["Affine Transformation B2 Coefficient",0.999942400000],UNIT["Foot",0.30480060960122]],VERT_CS["Generic Geoid",VERT_DATUM["Generic Vertical Datum",2005],UNIT["Foot",3.280833]]])"));

    EXPECT_TRUE(currentGCS2->IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, WisconsinToJson)
    {
    GeoCoordinates::BaseGCSPtr currentGCS1 = GeoCoordinates::BaseGCS::CreateGCS("AdamsWI-F");
    GeoCoordinates::BaseGCSPtr currentGCS2 = GeoCoordinates::BaseGCS::CreateGCS("BayfieldWI-F");

    Json::Value result1;
    EXPECT_TRUE(SUCCESS == currentGCS1->ToJson(result1, true));

    Utf8String resultString1 = result1.toStyledString();

    EXPECT_NEAR(result1["horizontalCRS"]["projection"]["geoidSeparation"].asDouble(), -35.05 / 0.3048006096, 0.001);

    Json::Value result2;
    EXPECT_TRUE(SUCCESS == currentGCS2->ToJson(result2, true));

    Utf8String resultString2 = result2.toStyledString();

    EXPECT_NEAR(result2["horizontalCRS"]["projection"]["geoidSeparation"].asDouble(), -30.45 / 0.3048006096, 0.001);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedGCSUnitTests, SevenParamReversalTest)
{
    GeoCoordinates::GeodeticTransform* theTransform = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();

    theTransform->SetConvertMethodCode(GeoCoordinates::GenConvertCode::GenConvertType_7PARM);

    DPoint3d delta;
    delta.x = 123.4;
    delta.y = -234.1;
    delta.z = 245.654;
    DPoint3d rotation;
    rotation.x = -27.12;
    rotation.y = -32.23;
    rotation.z = 12.34;

    theTransform->SetDelta(delta);
    theTransform->SetRotation(rotation);
    theTransform->SetScale(-6.22);

    GeoCoordinates::EllipsoidP theEllipsoid1A = const_cast<GeoCoordinates::Ellipsoid*>(GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84"));
    GeoCoordinates::EllipsoidP theEllipsoid1B = const_cast<GeoCoordinates::Ellipsoid*>(GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84"));

    theTransform->SetSourceEllipsoid(theEllipsoid1A->Clone());
    theTransform->SetTargetEllipsoid(theEllipsoid1B->Clone());

    GeoCoordinates::GeodeticTransform* theTransform2 = theTransform->Clone();

    theTransform2->Reverse();

    EXPECT_TRUE(theTransform2->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_7PARM);

    GeoCoordinates::GeodeticTransform* theTransformReversed = theTransform2->Clone();

    theTransform2->Reverse();

    DPoint3d resultDelta;
    theTransform2->GetDelta(resultDelta);
    EXPECT_NEAR(resultDelta.x, delta.x, 0.000001);
    EXPECT_NEAR(resultDelta.y, delta.y, 0.000001);
    EXPECT_NEAR(resultDelta.z, delta.z, 0.000001);
    DPoint3d resultRotation;
    theTransform2->GetRotation(resultRotation);
    EXPECT_NEAR(resultRotation.x, rotation.x, 0.000001);
    EXPECT_NEAR(resultRotation.y, rotation.y, 0.000001);
    EXPECT_NEAR(resultRotation.z, rotation.z, 0.000001);
    EXPECT_NEAR(theTransform2->GetScale(), theTransform->GetScale(), 0.0001);

    // Coordinate conversion
    theTransform->SetSourceDatumName("TOTO1");
    theTransform->SetTargetDatumName("TOTO2");
    theTransform->SetSourceEllipsoid(theEllipsoid1B->Clone());
    theTransform->SetTargetEllipsoid(theEllipsoid1A->Clone());

    theTransformReversed->SetSourceDatumName("TOTO2");
    theTransformReversed->SetTargetDatumName("WGS84");
    theTransformReversed->SetSourceEllipsoid(theEllipsoid1A->Clone());
    theTransformReversed->SetTargetEllipsoid(theEllipsoid1B->Clone());

    GeoCoordinates::Datum* theDatum1 = const_cast<GeoCoordinates::Datum*>(GeoCoordinates::Datum::CreateDatum("WGS84"));
    theDatum1->SetName("TOTO1");
    theDatum1->SetDescription("");
    theDatum1->SetSource("");
    theDatum1->SetEllipsoid(theEllipsoid1B->Clone());
    GeoCoordinates::GeodeticTransformPath* thePath1 = GeoCoordinates::GeodeticTransformPath::Create();
    thePath1->AddGeodeticTransform(theTransform);
    thePath1->AddGeodeticTransform(theTransformReversed);
    theDatum1->SetStoredGeodeticTransformPath(thePath1);

    GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");

    GeoCoordinates::DatumConverter* datumConverter1 = GeoCoordinates::DatumConverter::Create(*theDatum1, *wgs84Datum);

    ASSERT_TRUE(datumConverter1 != nullptr);

    GeoPoint point1;
    point1.latitude = 0.0;
    point1.longitude = 0.0;
    point1.elevation = 0.0;

    GeoPoint point2 = {0, 0, 0};

    EXPECT_TRUE(REPROJECT_Success == datumConverter1->ConvertLatLong3D(point2, point1));

    EXPECT_NEAR(point1.latitude, point2.latitude, 0.00000003);
    EXPECT_NEAR(point1.longitude, point2.longitude, 0.00000003);
    EXPECT_NEAR(point1.elevation, point2.elevation, 0.0000003);

    theEllipsoid1A->Destroy();
    theEllipsoid1B->Destroy();
    wgs84Datum->Destroy();
    theDatum1->Destroy();
    datumConverter1->Destroy();
    theTransform2->Destroy();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, BadekasReversalTest)
    {
    GeoCoordinates::GeodeticTransform* theTransform = GeoCoordinates::GeodeticTransform::CreateGeodeticTransform();
    ASSERT_TRUE(theTransform != nullptr);

    theTransform->SetConvertMethodCode(GeoCoordinates::GenConvertCode::GenConvertType_BDKAS);

    DPoint3d delta;
    delta.x = 123.4;
    delta.y = -234.1;
    delta.z = 12.654;
    DPoint3d rotation;
    rotation.x = -27.12;
    rotation.y = -32.23;
    rotation.z = 12.34;
    DPoint3d translation;
    translation.x = 617749.71180000005;
    translation.y = -6250547.7335999999;
    translation.z = 1102063.6099;

    theTransform->SetDelta(delta);
    theTransform->SetRotation(rotation);
    theTransform->SetTranslation(translation);
    theTransform->SetScale(-6.22);

    GeoCoordinates::EllipsoidP theEllipsoid1A = const_cast<GeoCoordinates::Ellipsoid*>(GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84"));
    GeoCoordinates::EllipsoidP theEllipsoid1B = const_cast<GeoCoordinates::Ellipsoid*>(GeoCoordinates::Ellipsoid::CreateEllipsoid("WGS84"));

    ASSERT_TRUE(theEllipsoid1A != nullptr);
    ASSERT_TRUE(theEllipsoid1B != nullptr);

    theTransform->SetSourceEllipsoid(theEllipsoid1A->Clone());
    theTransform->SetTargetEllipsoid(theEllipsoid1B->Clone());

    GeoCoordinates::GeodeticTransform* theTransform2 = theTransform->Clone();

    theTransform2->Reverse();

    EXPECT_TRUE(theTransform2->GetConvertMethodCode() == GeoCoordinates::GenConvertCode::GenConvertType_BDKAS);

    GeoCoordinates::GeodeticTransform* theTransformReversed = theTransform2->Clone();

    theTransform2->Reverse();

    DPoint3d resultDelta;
    theTransform2->GetDelta(resultDelta);
    EXPECT_NEAR(resultDelta.x, delta.x, 0.000001);
    EXPECT_NEAR(resultDelta.y, delta.y, 0.000001);
    EXPECT_NEAR(resultDelta.z, delta.z, 0.000001);
    DPoint3d resultRotation;
    theTransform2->GetRotation(resultRotation);
    EXPECT_NEAR(resultRotation.x, rotation.x, 0.000001);
    EXPECT_NEAR(resultRotation.y, rotation.y, 0.000001);
    EXPECT_NEAR(resultRotation.z, rotation.z, 0.000001);
    EXPECT_NEAR(theTransform2->GetScale(), theTransform->GetScale(), 0.0001);

    DPoint3d resultTranslation;
    theTransform2->GetTranslation(resultTranslation);
    EXPECT_NEAR(resultTranslation.x, translation.x, 0.000001);
    EXPECT_NEAR(resultTranslation.y, translation.y, 0.000001);
    EXPECT_NEAR(resultTranslation.z, translation.z, 0.000001);

    // Coordinate conversion
    theTransform->SetSourceDatumName("TOTO1");
    theTransform->SetTargetDatumName("TOTO2");
    theTransform->SetSourceEllipsoid(theEllipsoid1B->Clone());
    theTransform->SetTargetEllipsoid(theEllipsoid1A->Clone());

    theTransformReversed->SetSourceDatumName("TOTO2");
    theTransformReversed->SetTargetDatumName("WGS84");
    theTransformReversed->SetSourceEllipsoid(theEllipsoid1A->Clone());
    theTransformReversed->SetTargetEllipsoid(theEllipsoid1B->Clone());

    GeoCoordinates::Datum* theDatum1 = const_cast<GeoCoordinates::Datum*>(GeoCoordinates::Datum::CreateDatum("WGS84"));
    ASSERT_TRUE(theDatum1 != nullptr);

    theDatum1->SetName("TOTO1");
    theDatum1->SetDescription("");
    theDatum1->SetSource("");
    theDatum1->SetEllipsoid(theEllipsoid1B->Clone());
    GeoCoordinates::GeodeticTransformPath* thePath1 = GeoCoordinates::GeodeticTransformPath::Create();
    thePath1->AddGeodeticTransform(theTransform);
    thePath1->AddGeodeticTransform(theTransformReversed);
    theDatum1->SetStoredGeodeticTransformPath(thePath1);

    GeoCoordinates::DatumCP wgs84Datum = GeoCoordinates::Datum::CreateDatum("WGS84");
    ASSERT_TRUE(wgs84Datum != nullptr);

    GeoCoordinates::DatumConverter* datumConverter1 = GeoCoordinates::DatumConverter::Create(*theDatum1, *wgs84Datum);
    ASSERT_TRUE(datumConverter1 != nullptr);

    GeoPoint point1;
    point1.latitude = 0.0;
    point1.longitude = 0.0;
    point1.elevation = 0.0;

    GeoPoint point2 = {0, 0, 0};

    EXPECT_TRUE(REPROJECT_Success == datumConverter1->ConvertLatLong3D(point2, point1));

    // Note that the tolerance is on the large side (centimeters) but the selected parameters were large.
    // Note also that CSMAP does not even bother inverting the rotation matrix and simply applies the Bursa-Wolfe assumption
    // which sets son(A) = A and cos(A) = 1 when A is small so our reversal process is likely much better.
    EXPECT_NEAR(point1.latitude, point2.latitude, 0.000001);
    EXPECT_NEAR(point1.longitude, point2.longitude, 0.000001);
    EXPECT_NEAR(point1.elevation, point2.elevation, 0.000001);

    theEllipsoid1A->Destroy();
    theEllipsoid1B->Destroy();
    wgs84Datum->Destroy();
    theDatum1->Destroy();
    datumConverter1->Destroy();
    theTransform2->Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, WKGenerateToWGS84Config)
    {
    GeoCoordinates::BaseGCSPtr currentGCS2 = GeoCoordinates::BaseGCS::CreateGCS("DB_REF.GK3d-2/EN");

    Utf8String wellKnownText;
    EXPECT_TRUE(SUCCESS == currentGCS2->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false, false, false));
    
    EXPECT_TRUE(wellKnownText.find("TOWGS84") != std::string::npos);
    
    size_t pos = wellKnownText.find("TOWGS84");
    Utf8String toWGS84 = wellKnownText.substr(pos);
    toWGS84 = toWGS84.substr(7);
    toWGS84.Trim();
    toWGS84 = toWGS84.substr(1);

    double  deltaX, deltaY, deltaZ, rotationX, rotationY, rotationZ, scalePPM;
    
    EXPECT_TRUE(7 == Utf8String::Sscanf_safe(toWGS84.c_str(), "%lf,%lf,%lf,%lf,%lf,%lf,%lf", &deltaX, &deltaY, &deltaZ, &rotationX, &rotationY, &rotationZ, &scalePPM));
    
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaX, 584.9636));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaY, 107.7175));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaZ, 413.8067));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationX, 1.1155));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationY, 0.2824));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationZ, -3.1384));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(scalePPM, 7.9922));

    // NO TOWGS84 clause
    EXPECT_TRUE(SUCCESS == currentGCS2->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false, true, false));
    
    EXPECT_TRUE(wellKnownText.find("TOWGS84") == std::string::npos);

    // TOWGS84 clause with Positional Vector convention.
    EXPECT_TRUE(SUCCESS == currentGCS2->GetWellKnownText(wellKnownText, GeoCoordinates::BaseGCS::wktFlavorOGC, false, false, true));

    pos = wellKnownText.find("TOWGS84");
    toWGS84 = wellKnownText.substr(pos);
    toWGS84 = toWGS84.substr(7);
    toWGS84.Trim();
    toWGS84 = toWGS84.substr(1);

    EXPECT_TRUE(7 == Utf8String::Sscanf_safe(toWGS84.c_str(), "%lf,%lf,%lf,%lf,%lf,%lf,%lf", &deltaX, &deltaY, &deltaZ, &rotationX, &rotationY, &rotationZ, &scalePPM));
    
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaX, 584.9636));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaY, 107.7175));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(deltaZ, 413.8067));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationX, -1.1155));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationY, -0.2824));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(rotationZ, 3.1384));
    EXPECT_TRUE(GeoCoordTestCommon::doubleSame(scalePPM, 7.9922));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ExtendedGCSUnitTests, MetadataInWKT)
    {
    GeoCoordinates::BaseGCSPtr currentGCS = GeoCoordinates::BaseGCS::CreateGCS();

    EXPECT_TRUE(GeoCoordinates::GeoCoordParse_Success == currentGCS->InitFromWellKnownText(NULL, NULL, R"(PROJCS["la_cm19_lo5",GEOGCS["GCS_WGS_1984",DATUM["D_WGS_1984",SPHEROID["WGS_1984",6378137.0,298.257223563]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.017453292519943295],METADATA["World",-180.0,-90.0,180.0,90.0,0.0,0.0174532925199433,0.0,1262]],PROJECTION["Lambert_Azimuthal_Equal_Area"],PARAMETER["False_Easting",0.0],PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",19.0],PARAMETER["Latitude_Of_Origin",5.0],UNIT["Meter",1.0]])"));
    EXPECT_TRUE(currentGCS->IsValid());
    }