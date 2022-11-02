//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>

#include <GeoCoord/BaseGeoCoord.h>
#include <GeoCoord/BaseGeoTiffKeysList.h>

#include "GeoCoordTestCommon.h"

using namespace ::testing;

using namespace GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class SpecificXMLParseTests : public ::testing::Test
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        SpecificXMLParseTests() {};
        ~SpecificXMLParseTests() {};
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseXML1)
{
    // < ? xml version = "1.0" encoding = "UTF-8 standalone="no" ?>

Utf8String theXML = R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS-LCC">
    <Name>ETRS-LCC</Name>
    <Description>Europe ETRS89 Lambert Conformal Conic CRS, ETRS89 datum</Description>
    <Authority>EuroGeoGraphics</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-10.67</WestBoundLongitude>
            <EastBoundLongitude>31.55</EastBoundLongitude>
            <SouthBoundLatitude>34.5</SouthBoundLatitude>
            <NorthBoundLatitude>71.05</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRS89</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">35</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">65</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">10</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">2800000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">4000000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="ETRS89">
    <Name>ETRS89</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRS89_to_WGS84">
    <Name>ETRS89_to_WGS84</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Molodensky</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_TRUE(theGCS.IsValid());

    EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(theXML.c_str()));

    ASSERT_TRUE(theGCS->IsValid());

    EXPECT_STREQ("ETRS89", theGCS->GetDatumName());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseXML2)
{
    // < ? xml version = "1.0" encoding = "UTF-8 standalone="no" ?>

    Utf8String theXML = R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS-LCC">
    <Name>ETRS-LCC</Name>
    <Description>Europe ETRS89 Lambert Conformal Conic CRS, ETRS89 datum</Description>
    <Authority>EuroGeoGraphics</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-10.67</WestBoundLongitude>
            <EastBoundLongitude>31.55</EastBoundLongitude>
            <SouthBoundLatitude>34.5</SouthBoundLatitude>
            <NorthBoundLatitude>71.05</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRS89AA</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">35</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">65</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">10</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">2800000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">4000000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="ETRS89AA">
    <Name>ETRS89AA</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84A</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="WGS84A">
    <Name>WGS84A</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRS89AA_to_WGS84">
    <Name>ETRS89AA_to_WGS84</Name>
    <Description>ETRS89AA (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89AA</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Molodensky</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_TRUE(theGCS.IsValid());

    EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(theXML.c_str()));

    EXPECT_TRUE(theGCS->IsValid());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseXML3)
{
    // < ? xml version = "1.0" encoding = "UTF-8 standalone="no" ?>

    Utf8String theXML = R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="CAN83-10">
    <Name>CAN83-10</Name>
    <Description>Ontario MTM Zone 10, 79.5-78 over 47N, 80.25-78W to S; NAD 83</Description>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-81.3833333333333</WestBoundLongitude>
            <EastBoundLongitude>-77.6333333333333</EastBoundLongitude>
            <SouthBoundLatitude>40.25</SouthBoundLatitude>
            <NorthBoundLatitude>64.45</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83XYZ</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">-79.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">304800</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="32190" type="CoordinateSystem">
    <ObjectId>CAN83-10</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83XYZ">
    <Name>NAD83XYZ</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6269" type="Datum">
    <ObjectId>NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83XYZ_to_WGS84">
    <Name>NAD83XYZ_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83XYZ</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_TRUE(theGCS.IsValid());

    EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(theXML.c_str()));

    EXPECT_TRUE(theGCS->IsValid());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseXML5)
{
    // < ? xml version = "1.0" encoding = "UTF-8 standalone="no" ?>

    Utf8String theXML = R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="DHDN/3.Berlin/Cassini">
    <Name>DHDN/3.Berlin/Cassini</Name>
    <Description>DHDN / Soldner Berlin</Description>
    <Authority>EPSG, V6.11.2, 3068 [Berlin state statistical office. Also a]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>12.8666666666667</WestBoundLongitude>
            <EastBoundLongitude>13.9666666666667</EastBoundLongitude>
            <SouthBoundLatitude>52.0666666666667</SouthBoundLatitude>
            <NorthBoundLatitude>52.9</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>DHDN/3A</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Cassini-Soldner</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">13.6272036666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52.4186482777778</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">40000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">10000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3068" type="CoordinateSystem">
    <ObjectId>DHDN/3.Berlin/Cassini</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="DHDN/3A">
    <Name>DHDN/3A</Name>
    <Description>Deutsches Hauptdreiecksnetz</Description>
    <Authority>EPSG, V6.3, 6314 [EPSG]</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>BESSEL</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6314" type="Datum">
    <ObjectId>DHDN/3</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="BESSEL">
    <Name>BESSEL</Name>
    <Description>Bessel, 1841</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377397.155</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356078.96281819</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7004" type="Ellipsoid">
    <ObjectId>BESSEL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="DHDN/3_to_WGS84">
    <Name>DHDN/3A_to_WGS84</Name>
    <Description>Deutsches Hauptdreiecksnetz</Description>
    <Authority>EPSG, V6.3, 6314 [EPSG]</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>4.875</WestBoundLongitude>
            <EastBoundLongitude>14.825</EastBoundLongitude>
            <SouthBoundLatitude>46.29875</SouthBoundLatitude>
            <NorthBoundLatitude>56.01125</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>2</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">5</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>DHDN/3A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">598.1</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">73.7</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">418.2</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-5.61111111111111e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-1.25e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.000681944444444444</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">6.7e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1777" type="Transformation">
    <ObjectId>DHDN/3A_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>


)X";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_TRUE(theGCS.IsValid());

    EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(theXML.c_str()));

    EXPECT_TRUE(theGCS->IsValid());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseXML6)
{
    // < ? xml version = "1.0" encoding = "UTF-8 standalone="no" ?>

    Utf8String theXML = R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS89.TM32">
    <Name>ETRS89.TM32</Name>
    <Description>ETRS89 / ETRS-TM32</Description>
    <Authority>EPSG, V6.11.2, 3044 [identical to ETRF89.TM32]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>6</WestBoundLongitude>
            <EastBoundLongitude>12</EastBoundLongitude>
            <SouthBoundLatitude>36.7</SouthBoundLatitude>
            <NorthBoundLatitude>70</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRS89/01</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">9</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9996</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3044" type="CoordinateSystem">
    <ObjectId>ETRS89.TM32</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="ETRS89/01">
    <Name>ETRS89/01</Name>
    <Description>European Terrestrial Reference System, 1989 (== WGS84)</Description>
    <Authority>EPSG 6.12 6258, Identical to (alias for) ETRF89</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRS89/01_to_WGS84">
    <Name>ETRS89/01_to_WGS84</Name>
    <Description>European Terrestrial Reference System, 1989 (== WGS84)</Description>
    <Authority>EPSG 6.12 6258, Identical to (alias for) ETRF89</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89/01</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Geocentric translations (geog2D domain)</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>



)X";


    GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

    ASSERT_TRUE(theGCS.IsValid());

    EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(theXML.c_str()));

    EXPECT_TRUE(theGCS->IsValid());
}

bvector<Utf8String> ListOfOSGEO_XML = {

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="CAN83-10">
    <Name>CAN83-10</Name>
    <Description>Ontario MTM Zone 10, 79.5-78 over 47N, 80.25-78W to S; NAD 83</Description>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-81.3833333333333</WestBoundLongitude>
            <EastBoundLongitude>-77.6333333333333</EastBoundLongitude>
            <SouthBoundLatitude>40.25</SouthBoundLatitude>
            <NorthBoundLatitude>64.45</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">-79.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">304800</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="32190" type="CoordinateSystem">
    <ObjectId>CAN83-10</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83">
    <Name>NAD83</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6269" type="Datum">
    <ObjectId>NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="DHDN/3.Berlin/Cassini">
    <Name>DHDN/3.Berlin/Cassini</Name>
    <Description>DHDN / Soldner Berlin</Description>
    <Authority>EPSG, V6.11.2, 3068 [Berlin state statistical office. Also a]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>12.8666666666667</WestBoundLongitude>
            <EastBoundLongitude>13.9666666666667</EastBoundLongitude>
            <SouthBoundLatitude>52.0666666666667</SouthBoundLatitude>
            <NorthBoundLatitude>52.9</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>DHDN/3</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Cassini-Soldner</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">13.6272036666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52.4186482777778</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">40000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">10000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3068" type="CoordinateSystem">
    <ObjectId>DHDN/3.Berlin/Cassini</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="DHDN/3">
    <Name>DHDN/3</Name>
    <Description>Deutsches Hauptdreiecksnetz</Description>
    <Authority>EPSG, V6.3, 6314 [EPSG]</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>BESSEL</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6314" type="Datum">
    <ObjectId>DHDN/3</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="BESSEL">
    <Name>BESSEL</Name>
    <Description>Bessel, 1841</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377397.155</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356078.96281819</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7004" type="Ellipsoid">
    <ObjectId>BESSEL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="DHDN/3_to_WGS84">
    <Name>DHDN/3_to_WGS84</Name>
    <Description>Deutsches Hauptdreiecksnetz</Description>
    <Authority>EPSG, V6.3, 6314 [EPSG]</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>4.875</WestBoundLongitude>
            <EastBoundLongitude>14.825</EastBoundLongitude>
            <SouthBoundLatitude>46.29875</SouthBoundLatitude>
            <NorthBoundLatitude>56.01125</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>2</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">5</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>DHDN/3</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">598.1</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">73.7</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">418.2</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-5.61111111111111e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-1.25e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.000681944444444444</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">6.7e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1777" type="Transformation">
    <ObjectId>DHDN/3_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS89.TM32">
    <Name>ETRS89.TM32</Name>
    <Description>ETRS89 / ETRS-TM32</Description>
    <Authority>EPSG, V6.11.2, 3044 [identical to ETRF89.TM32]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>6</WestBoundLongitude>
            <EastBoundLongitude>12</EastBoundLongitude>
            <SouthBoundLatitude>36.7</SouthBoundLatitude>
            <NorthBoundLatitude>70</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRS89/01</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">9</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9996</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3044" type="CoordinateSystem">
    <ObjectId>ETRS89.TM32</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="ETRS89/01">
    <Name>ETRS89/01</Name>
    <Description>European Terrestrial Reference System, 1989 (== WGS84)</Description>
    <Authority>EPSG 6.12 6258, Identical to (alias for) ETRF89</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRS89/01_to_WGS84">
    <Name>ETRS89/01_to_WGS84</Name>
    <Description>European Terrestrial Reference System, 1989 (== WGS84)</Description>
    <Authority>EPSG 6.12 6258, Identical to (alias for) ETRF89</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89/01</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Geocentric translations (geog2D domain)</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS89.UTM-33N">
    <Name>ETRS89.UTM-33N</Name>
    <Description>ETRS89 / UTM zone 33N</Description>
    <Authority>EPSG, V6.3, 25833 [Large and medium scale topographic mappi]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>11.5</WestBoundLongitude>
            <EastBoundLongitude>18.5166666666667</EastBoundLongitude>
            <SouthBoundLatitude>45.65</SouthBoundLatitude>
            <NorthBoundLatitude>84.7666666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRF89</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator Zoned Grid System</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>UTM Zone Number</OperationParameterId>
          <IntegerValue>33</IntegerValue>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Hemisphere, North or South</OperationParameterId>
          <IntegerValue>1</IntegerValue>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="25833" type="CoordinateSystem">
    <ObjectId>ETRS89.UTM-33N</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="ETRF89">
    <Name>ETRF89</Name>
    <Description>European Terrestrial Reference Frame, 1989 (== WGS84)</Description>
    <Authority>Various source, all say its equivalent to WGS84</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6258" type="Datum">
    <ObjectId>ETRF89</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRF89_to_WGS84">
    <Name>ETRF89_to_WGS84</Name>
    <Description>European Terrestrial Reference Frame, 1989 (== WGS84)</Description>
    <Authority>Various source, all say its equivalent to WGS84</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-15.9475</WestBoundLongitude>
            <EastBoundLongitude>36.8275</EastBoundLongitude>
            <SouthBoundLatitude>29.93125</SouthBoundLatitude>
            <NorthBoundLatitude>75.61875</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRF89</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Geocentric translations (geog2D domain)</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1149" type="Transformation">
    <ObjectId>ETRF89_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="ETRS-LCC">
    <Name>ETRS-LCC</Name>
    <Description>Europe ETRS89 Lambert Conformal Conic CRS, ETRS89 datum</Description>
    <Authority>EuroGeoGraphics</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-10.67</WestBoundLongitude>
            <EastBoundLongitude>31.55</EastBoundLongitude>
            <SouthBoundLatitude>34.5</SouthBoundLatitude>
            <NorthBoundLatitude>71.05</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>ETRS89</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">35</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">65</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">10</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">2800000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">4000000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="ETRS89">
    <Name>ETRS89</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="ETRS89_to_WGS84">
    <Name>ETRS89_to_WGS84</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Molodensky</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="HARN/IL.IL-EF">
    <Name>HARN/IL.IL-EF</Name>
    <Description>HARN/IL Illinois State Planes, East Zone, US Foot</Description>
    <Authority>Calculated from IL83-E by Mentor Software</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-89.55</WestBoundLongitude>
            <EastBoundLongitude>-86.7333333333333</EastBoundLongitude>
            <SouthBoundLatitude>36.5166666666667</SouthBoundLatitude>
            <NorthBoundLatitude>43.05</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>HARN/IL</DatumId>
    <Axis uom="FOOT">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">-88.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">36.6666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.999975</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="FOOT">984250</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="FOOT">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3443" type="CoordinateSystem">
    <ObjectId>HARN/IL.IL-EF</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="HARN/IL">
    <Name>HARN/IL</Name>
    <Description>High Accuracy Regional (IL) Network (aka HPGN, NAD83/91)</Description>
    <Authority>Derived by Mentor Software from US NGS NADCON 2.10</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6152" type="Datum">
    <ObjectId>HARN/IL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_HARN/IL">
    <Name>NAD83_to_HARN/IL</Name>
    <Description>High Accuracy Regional Network (aka HPGN) for region IL</Description>
    <Authority>Derived by Mentor Software from US NGS NADCON 2.10</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>HARN/IL</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Harn\ilhpgn.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Harn\ilhpgn.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Alias id="1553" type="Transformation">
    <ObjectId>NAD83_to_HARN/IL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="HARN/IL_to_WGS84">
    <Name>HARN/IL_to_WGS84</Name>
    <Description>High Accuracy Regional Network (aka HPGN) for region IL</Description>
    <Authority>Derived by Mentor Software from US NGS NADCON 2.10</Authority>
    <SourceDatumId>HARN/IL</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>NAD83_to_HARN/IL</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="JGD2011-08-ITRF08">
    <Name>JGD2011-08-ITRF08</Name>
    <Description>Japan Geodetic Datum 2011 Plane No. 08 (Except Shizuoka)</Description>
    <Authority>Geospatial Information Authority of Japan</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>137.166666666667</WestBoundLongitude>
            <EastBoundLongitude>140.233333333333</EastBoundLongitude>
            <SouthBoundLatitude>33.6333333333333</SouthBoundLatitude>
            <NorthBoundLatitude>39.1166666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>JGD2011</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">138.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">36</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="6676" type="CoordinateSystem">
    <ObjectId>JGD2011-08-ITRF08</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="JGD2011">
    <Name>JGD2011</Name>
    <Description>Japan Geodetic Datum 2011</Description>
    <Authority>Geospatial Information Authority of Japan</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="1128" type="Datum">
    <ObjectId>JGD2011</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="JGD2011_to_WGS84">
    <Name>JGD2011_to_WGS84</Name>
    <Description>Japan Geodetic Datum 2011 (7 Parameter Transform)</Description>
    <Authority>Geospatial Information Authority of Japan</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">3</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>JGD2011</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0.0828</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-0.5024</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-0.2862</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-5.08333333333333e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">8.33333333333333e-08</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">-1.96111111111111e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">-7.09e-09</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <GeographicCoordinateSystem id="LL84">
    <Name>LL84</Name>
    <Description>WGS84 datum, Latitude-Longitude; Degrees</Description>
    <Authority>Mentor Software</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-182</WestBoundLongitude>
            <EastBoundLongitude>182</EastBoundLongitude>
            <SouthBoundLatitude>-90</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>WGS84</DatumId>
    <Axis uom="DEGREE">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Geodetic latitude</AxisName>
        <AxisAbbreviation>Lat</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Geodetic longitude</AxisName>
        <AxisAbbreviation>Long</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
  </GeographicCoordinateSystem>

  <Alias id="4326" type="CoordinateSystem">
    <ObjectId>LL84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6326" type="Datum">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Luxembourg30.Gauss">
    <Name>Luxembourg30.Gauss</Name>
    <Description>Deprecated by EPSG synch. Replaced by Luxembourg30a.Gauss.</Description>
    <Authority>EPSG, V6.3, 2169 [Large and medium scale topographic mappi]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DatumId>Luxembourg30</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">6.16666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">49.8333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">1</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">80000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">100000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2169" type="CoordinateSystem">
    <ObjectId>Luxembourg30.Gauss</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="Luxembourg30">
    <Name>Luxembourg30</Name>
    <Description>Deprecated by EPSG synch. Replaced by Luxembourg30a.</Description>
    <Authority>EPSG, V6.3, 6181 [EPSG]</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>INTNL</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6181" type="Datum">
    <ObjectId>Luxembourg30</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="INTNL">
    <Name>INTNL</Name>
    <Description>International - 1924</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378388</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356911.94612795</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7022" type="Ellipsoid">
    <ObjectId>INTNL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="Luxembourg30_to_WGS84">
    <Name>Luxembourg30_to_WGS84</Name>
    <Description>Deprecated by EPSG synchronization. Replaced by Luxembourg30a_to_WGS84.</Description>
    <Authority>EPSG, V6.3, 6181 [EPSG]</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>5.63</WestBoundLongitude>
            <EastBoundLongitude>6.63</EastBoundLongitude>
            <SouthBoundLatitude>49.38875</SouthBoundLatitude>
            <NorthBoundLatitude>50.30125</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>2</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">0.5</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>Luxembourg30</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">-193</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">13.7</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-39.3</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">0.000113888888888889</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">0.000814722222222222</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">-0.000746666666666667</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">4.3e-07</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1079" type="Transformation">
    <ObjectId>Luxembourg30_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Luxembourg30a.Gauss">
    <Name>Luxembourg30a.Gauss</Name>
    <Description>Deprecated (datum); replaced by Luxembourg30b.Gauss.</Description>
    <Authority>EPSG, V6.3, 2169 [Large and medium scale topographic mappi]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>5.63333333333333</WestBoundLongitude>
            <EastBoundLongitude>6.63333333333333</EastBoundLongitude>
            <SouthBoundLatitude>49.4</SouthBoundLatitude>
            <NorthBoundLatitude>50.2833333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>Luxembourg30a</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">6.16666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">49.8333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">1</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">80000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">100000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2169" type="CoordinateSystem">
    <ObjectId>Luxembourg30a.Gauss</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="Luxembourg30a">
    <Name>Luxembourg30a</Name>
    <Description>Deprecated (rotation sign), replaced by Luxembourg30b.</Description>
    <Authority>EPSG, V6.3, 6181 [EPSG]</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>INTNL</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6181" type="Datum">
    <ObjectId>Luxembourg30a</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="INTNL">
    <Name>INTNL</Name>
    <Description>International - 1924</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378388</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356911.94612795</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7022" type="Ellipsoid">
    <ObjectId>INTNL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="Luxembourg30a_to_WGS84">
    <Name>Luxembourg30a_to_WGS84</Name>
    <Description>Deprecated (rotations); replaced by Luxembourg30b_to_WGS84.</Description>
    <Authority>EPSG, V6.3, 6181 [EPSG]</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>5.63</WestBoundLongitude>
            <EastBoundLongitude>6.63</EastBoundLongitude>
            <SouthBoundLatitude>49.38875</SouthBoundLatitude>
            <NorthBoundLatitude>50.30125</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">0.5</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>Luxembourg30a</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">-193</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">13.7</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-39.3</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-0.000113888888888889</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-0.000814722222222222</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.000746666666666667</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">4.3e-07</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1079" type="Transformation">
    <ObjectId>Luxembourg30a_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="MGA-56">
    <Name>MGA-56</Name>
    <Description>Map Grid of Australia Zone 56, using GDA94 datum</Description>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>149.683333333333</WestBoundLongitude>
            <EastBoundLongitude>156.3</EastBoundLongitude>
            <SouthBoundLatitude>-59.9666666666667</SouthBoundLatitude>
            <NorthBoundLatitude>-12.85</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>GDA94</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">153</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9996</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">10000000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="28356" type="CoordinateSystem">
    <ObjectId>MGA-56</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="GDA94">
    <Name>GDA94</Name>
    <Description>Geocentric Datum of Australia, 1994 (GDA 94)</Description>
    <Authority>AUSLIG</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6283" type="Datum">
    <ObjectId>GDA94</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="GDA94_to_WGS84">
    <Name>GDA94_to_WGS84</Name>
    <Description>Geocentric Datum of Australia, 1994 (GDA 94)</Description>
    <Authority>AUSLIG</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>102.125</WestBoundLongitude>
            <EastBoundLongitude>160.875</EastBoundLongitude>
            <SouthBoundLatitude>-49.375</SouthBoundLatitude>
            <NorthBoundLatitude>-5.625</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>GDA94</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1150" type="Transformation">
    <ObjectId>GDA94_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Netherlands-RD">
    <Name>Netherlands-RD</Name>
    <Description>Netherlands, Rijksdriehoeksmeting datum, Oblique Stereographic</Description>
    <Authority>EuroGeoGraphics</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>2.7</WestBoundLongitude>
            <EastBoundLongitude>7.75</EastBoundLongitude>
            <SouthBoundLatitude>50.45</SouthBoundLatitude>
            <NorthBoundLatitude>54.05</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>RD</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Oblique Stereographic</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">5.38763888888889</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">52.1561605555556</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999079</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">155000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">463000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="RD">
    <Name>RD</Name>
    <Description>Rijksdriehoeksmeting, Netherlands (7 Parameter Transform)</Description>
    <Authority>EuroGeographics</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>BESSEL</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="BESSEL">
    <Name>BESSEL</Name>
    <Description>Bessel, 1841</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377397.155</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356078.963</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7004" type="Ellipsoid">
    <ObjectId>BESSEL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="RD_to_WGS84">
    <Name>RD_to_WGS84</Name>
    <Description>Rijksdriehoeksmeting, Netherlands (7 Parameter Transform)</Description>
    <Authority>EuroGeographics</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">3</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>RD</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">565.04</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">49.91</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">465.84</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">0.000113722222222222</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-9.99166666666667e-005</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.000519027777777778</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">4.0772e-006</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <TransformationPath id="RD_to_WGS84_INVERSED">
    <Name>RD_to_WGS84_INVERSED</Name>
    <Description>RD to WGS84 SHIFT INVERSED (EARTHMINE)</Description>
    <SourceDatumId>WGS84</SourceDatumId>
    <TargetDatumId>RD</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>RD_to_WGS84</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="NordSahara1959.UnifieN">
    <Name>NordSahara1959.UnifieN</Name>
    <Description>Nord Sahara 1959 / Voirol Unifie Nord</Description>
    <Authority>EPSG, V6.3, 30791 [Large and medium scale topographic mappi]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-3.58333333333333</WestBoundLongitude>
            <EastBoundLongitude>10.0166666666667</EastBoundLongitude>
            <SouthBoundLatitude>34.4</SouthBoundLatitude>
            <NorthBoundLatitude>37.3333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NordSahara1959</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (1SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">2.7</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">36</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.99962554</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">500135</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">300090</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="30791" type="CoordinateSystem">
    <ObjectId>NordSahara1959.UnifieN</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NordSahara1959">
    <Name>NordSahara1959</Name>
    <Description>Nord Sahara 1959</Description>
    <Authority>EPSG, V6.3, 6307 [EPSG]</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>CLRK80</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6307" type="Datum">
    <ObjectId>NordSahara1959</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="CLRK80">
    <Name>CLRK80</Name>
    <Description>Clarke 1880, RGS</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378249.145</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356514.87</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7012" type="Ellipsoid">
    <ObjectId>CLRK80</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NordSahara1959_to_WGS84">
    <Name>NordSahara1959_to_WGS84</Name>
    <Description>Nord Sahara 1959</Description>
    <Authority>EPSG, V6.3, 6307 [EPSG]</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NordSahara1959</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Geocentric translations (geog2D domain)</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">-186</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-93</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">310</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="OSGB1936.NationalGrid">
    <Name>OSGB1936.NationalGrid</Name>
    <Description>OSGB 1936 British National Grid</Description>
    <Authority>Ordnance Survey: OSTN15 NTv2 Transformation</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-9.8</WestBoundLongitude>
            <EastBoundLongitude>3.48333333333333</EastBoundLongitude>
            <SouthBoundLatitude>49.1666666666667</SouthBoundLatitude>
            <NorthBoundLatitude>61.7166666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>OSGB1936</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">-2</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">49</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9996012717</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">400000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">-100000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="27700" type="CoordinateSystem">
    <ObjectId>OSGB1936.NationalGrid</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="OSGB1936">
    <Name>OSGB1936</Name>
    <Description>Ordnance Survey of Great Britain 1936</Description>
    <Authority>Ordnance Survey: OSTN15 NTv2 Transofrmation</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>AIRY30</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6277" type="Datum">
    <ObjectId>OSGB1936</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="AIRY30">
    <Name>AIRY30</Name>
    <Description>Airy, 1830</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377563.396</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356256.909</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7001" type="Ellipsoid">
    <ObjectId>AIRY30</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="OSGB1936_to_ETRS89">
    <Name>OSGB1936_to_ETRS89</Name>
    <Description>OSGB 1936 to ETRS89, via OSTN15 NTv2</Description>
    <Authority>Ordnance Survey: OSTN15 NTv2 Transformation</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>TransformationGridFileFallback</Key>
        <StringValue>OSGB-7P_to_WGS84</StringValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>9</WestBoundLongitude>
            <EastBoundLongitude>2</EastBoundLongitude>
            <SouthBoundLatitude>49</SouthBoundLatitude>
            <NorthBoundLatitude>61</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>OSGB1936</SourceDatumId>
    <TargetDatumId>ETRS89</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>NTv2</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\UK\OSTN15_NTv2_OSGBtoETRS.gsb</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="ETRS89_to_WGS84">
    <Name>ETRS89_to_WGS84</Name>
    <Description>ETRS89 (ETRF89), used for EUREF89 System</Description>
    <Authority>Norwegian Geodetic Institute geodetic publication 1990:1</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">8</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>ETRS89</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Molodensky</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">0</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <TransformationPath id="OSGB1936_to_WGS84">
    <Name>OSGB1936_to_WGS84</Name>
    <Description>Ordnance Survey of Great Britain 1936</Description>
    <Authority>Autodesk</Authority>
    <SourceDatumId>OSGB1936</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>OSGB1936_to_ETRS89</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>ETRS89_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>

)X",
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="OSGB-GPS-2002">
    <Name>OSGB-GPS-2002</Name>
    <Description>ETRF89 Referenced OSGB (ETRF89&lt;--&gt;OSGB via OSTN02)</Description>
    <Authority>Ordnance Survey, Transformations and OSGM02 User Guide</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-7.5</WestBoundLongitude>
            <EastBoundLongitude>3.75</EastBoundLongitude>
            <SouthBoundLatitude>49.75</SouthBoundLatitude>
            <NorthBoundLatitude>62.3333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>WGS84</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Ordnance Survey National Grid Transformation of 2002</OperationMethodId>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6326" type="Datum">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83F">
    <Name>TN83F</Name>
    <Description>NAD83 Tennessee State Plane Zone, US Foot</Description>
    <Authority>Calculated from TN83 by Mentor Software</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-91.4</WestBoundLongitude>
            <EastBoundLongitude>-80.5833333333333</EastBoundLongitude>
            <SouthBoundLatitude>34.8333333333333</SouthBoundLatitude>
            <NorthBoundLatitude>36.85</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83</DatumId>
    <Axis uom="FOOT">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">36.4166666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">35.25</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-86</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">34.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="FOOT">1968500</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="FOOT">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2274" type="CoordinateSystem">
    <ObjectId>TN83F</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83">
    <Name>NAD83</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6269" type="Datum">
    <ObjectId>NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="UTM84-14N">
    <Name>UTM84-14N</Name>
    <Description>UTM-WGS 1984 datum, Zone 14 North, Meter; Cent. Meridian 99d W</Description>
    <Authority>Snyder, J.P, 1987, Map Projections - A Working Manual</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-102.75</WestBoundLongitude>
            <EastBoundLongitude>-95.25</EastBoundLongitude>
            <SouthBoundLatitude>-8.4</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>WGS84</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator Zoned Grid System</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>UTM Zone Number</OperationParameterId>
          <IntegerValue>14</IntegerValue>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Hemisphere, North or South</OperationParameterId>
          <IntegerValue>1</IntegerValue>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="32614" type="CoordinateSystem">
    <ObjectId>UTM84-14N</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6326" type="Datum">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.3142</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>
)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="WA83-SF">
    <Name>WA83-SF</Name>
    <Description>NAD83 Washington State Planes, South Zone, US Foot</Description>
    <Authority>Calculated from WA83-S by Mentor Software</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-125.333333333333</WestBoundLongitude>
            <EastBoundLongitude>-115.983333333333</EastBoundLongitude>
            <SouthBoundLatitude>45.3333333333333</SouthBoundLatitude>
            <NorthBoundLatitude>47.8166666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83</DatumId>
    <Axis uom="FOOT">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">47.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">45.8333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-120.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">45.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="FOOT">1640416.66667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="FOOT">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2286" type="CoordinateSystem">
    <ObjectId>WA83-SF</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83">
    <Name>NAD83</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6269" type="Datum">
    <ObjectId>NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="NAD83-WGS84">
    <Name>NAD83-WGS84</Name>
    <Description>TRIAL</Description>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>NAD83_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>
)X",
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Xian80.GK3d-37">
    <Name>Xian80.GK3d-37</Name>
    <Description>Xian 1980 / 3-degree Gauss-Kruger zone 37</Description>
    <Authority>EPSG, V6.6, 2361 [EPSG]</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>109.133333333333</WestBoundLongitude>
            <EastBoundLongitude>112.883333333333</EastBoundLongitude>
            <SouthBoundLatitude>15.4833333333333</SouthBoundLatitude>
            <NorthBoundLatitude>47.8</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>Xian80</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">111</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">1</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">37500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2361" type="CoordinateSystem">
    <ObjectId>Xian80.GK3d-37</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="Xian80">
    <Name>Xian80</Name>
    <Description>Xian 1980,</Description>
    <Authority>Mentor Software Client</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>Xian80</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6610" type="Datum">
    <ObjectId>Xian80</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="Xian80">
    <Name>Xian80</Name>
    <Description>Xian 1980</Description>
    <Authority>EPSG, V6.3, 7049 [EPSG]</Authority>
    <SemiMajorAxis uom="meter">6378140</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356755.288158</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7049" type="Ellipsoid">
    <ObjectId>Xian80</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="Xian80_to_WGS84">
    <Name>Xian80_to_WGS84</Name>
    <Description>Xian 1980,</Description>
    <Authority>Mentor Software Client</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">3</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>Xian80</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">24</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-123</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-94</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-5.55555555555556e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-6.94444444444444e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">3.61111111111111e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">1.1e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="GA/20-LCC">
    <Name>GA/20-LCC</Name>
    <Description>Geoscience Australia Lambert, GDA2020 (7Parameter)</Description>
    <Authority>EPSG 9.0</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>111.833333333333</WestBoundLongitude>
            <EastBoundLongitude>154.683333333333</EastBoundLongitude>
            <SouthBoundLatitude>-44.7</SouthBoundLatitude>
            <NorthBoundLatitude>-8.85</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>GDA2020-7P</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">-18</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">-36</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">134</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="7845" type="CoordinateSystem">
    <ObjectId>GA/20-LCC</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="GDA2020-7P">
    <Name>GDA2020-7P</Name>
    <Description>Geocentric Datum of Australia 2020</Description>
    <Authority>Intergovernmental Committee on Surveying &amp; Mapping</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="1168" type="Datum">
    <ObjectId>GDA2020-7P</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="GDA94_to_GDA2020-7P">
    <Name>GDA94_to_GDA2020-7P</Name>
    <Description>Geocentric Datum of Australia 2020 (7 Parameter)</Description>
    <Authority>Geocentric Datum of Australia, Interim Release Note</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">0.005</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>GDA94</SourceDatumId>
    <TargetDatumId>GDA2020-7P</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">0.06155</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-0.01087</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-0.04019</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-1.09701111111111e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-9.08947222222222e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">-9.13830555555556e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">-9.994e-09</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Transformation id="GDA94_to_WGS84">
    <Name>GDA94_to_WGS84</Name>
    <Description>Geocentric Datum of Australia, 1994 (GDA 94)</Description>
    <Authority>AUSLIG</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>102.125</WestBoundLongitude>
            <EastBoundLongitude>160.875</EastBoundLongitude>
            <SouthBoundLatitude>-49.375</SouthBoundLatitude>
            <NorthBoundLatitude>-5.625</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>GDA94</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1150" type="Transformation">
    <ObjectId>GDA94_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="GDA2020-7P_to_WGS84">
    <Name>GDA2020-7P_to_WGS84</Name>
    <Description>GDA2020 to WGS8a, thru GDA94 via seven parameter then to WGS84</Description>
    <Authority>Geocentric Datum of Australia, Interim Release Note</Authority>
    <SourceDatumId>GDA2020-7P</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>GDA94_to_GDA2020-7P</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>GDA94_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="CA-VII">
    <Name>CA-VII</Name>
    <Description>NAD27 California State Planes, Zone VII(407), US Foot</Description>
    <Authority>Snyder, J.P., 1987, Map Projections - A Working Manual</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-119.116666666667</WestBoundLongitude>
            <EastBoundLongitude>-117.483333333333</EastBoundLongitude>
            <SouthBoundLatitude>33.55</SouthBoundLatitude>
            <NorthBoundLatitude>34.9333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD27</DatumId>
    <Axis uom="FOOT">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">34.4166666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">33.8666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-118.333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">34.1333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="FOOT">4186692.58</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="FOOT">4160926.74</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="26799" type="CoordinateSystem">
    <ObjectId>CA-VII</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD27">
    <Name>NAD27</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>CLRK66</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6267" type="Datum">
    <ObjectId>NAD27</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="CLRK66">
    <Name>CLRK66</Name>
    <Description>Clarke 1866, Benoit Ratio</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378206.4</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356583.8</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>


  <Transformation id="NAD27_to_NAD83">
    <Name>NAD27_to_NAD83</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>TransformationGridFileFallback</Key>
        <StringValue>NAD27-48_to_WGS84</StringValue>
      </ParameterItem>
    </AdditionalInformation>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">0.15000001</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD27</SourceDatumId>
    <TargetDatumId>NAD83</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\conus.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\conus.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\alaska.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\alaska.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\prvi.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\prvi.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Alias id="1241" type="Transformation">
    <ObjectId>NAD27_to_NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <TransformationPath id="NAD27_to_WGS84">
    <Name>NAD27_to_WGS84</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SourceDatumId>NAD27</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>NAD27_to_NAD83</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>

)X",

R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="UTM83-10">
    <Name>UTM83-10</Name>
    <Description>UTM with NAD83 datum, Zone 10, Meter; Central Meridian 123d W</Description>
    <Authority>Snyder, J.P, 1987, Map Projections - A Working Manual</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-126.75</WestBoundLongitude>
            <EastBoundLongitude>-119.25</EastBoundLongitude>
            <SouthBoundLatitude>25.4166666666667</SouthBoundLatitude>
            <NorthBoundLatitude>86.9333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator Zoned Grid System</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>UTM Zone Number</OperationParameterId>
          <IntegerValue>10</IntegerValue>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Hemisphere, North or South</OperationParameterId>
          <IntegerValue>1</IntegerValue>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="26910" type="CoordinateSystem">
    <ObjectId>UTM83-10</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83">
    <Name>NAD83</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6269" type="Datum">
    <ObjectId>NAD83</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7019" type="Ellipsoid">
    <ObjectId>GRS1980</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-189.53875</WestBoundLongitude>
            <EastBoundLongitude>-180</EastBoundLongitude>
            <SouthBoundLatitude>16.335</SouthBoundLatitude>
            <NorthBoundLatitude>90</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>

)X",

// To test coordinate system alias use
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="UTM83-10-OBFUSCATED">
    <Name>UTM83-10-OBFUSCATED</Name>
    <Description>UTM with NAD83 datum, Zone 10, Meter; Central Meridian 123d W</Description>
    <Authority>Snyder, J.P, 1987, Map Projections - A Working Manual</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-126.75</WestBoundLongitude>
            <EastBoundLongitude>-119.25</EastBoundLongitude>
            <SouthBoundLatitude>25.4166666666667</SouthBoundLatitude>
            <NorthBoundLatitude>86.9333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD83-OBFUSCATED</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator Zoned Grid System</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>UTM Zone Number</OperationParameterId>
          <IntegerValue>10</IntegerValue>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Hemisphere, North or South</OperationParameterId>
          <IntegerValue>1</IntegerValue>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="26910" type="CoordinateSystem">
    <ObjectId>UTM83-10-OBFUSCATED</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD83-OBFUSCATED">
    <Name>NAD83-OBFUSCATED</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980-OBFUSCATED</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980-OBFUSCATED">
    <Name>GRS1980-OBFUSCATED</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>
</Dictionary>

)X",

// To test ellipsoid property matching
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="UTM83-10-OBFUSCATED">
    <Name>UTM83-10-OBFUSCATED</Name>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DatumId>NAD83-OBFUSCATED</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator Zoned Grid System</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>UTM Zone Number</OperationParameterId>
          <IntegerValue>10</IntegerValue>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Hemisphere, North or South</OperationParameterId>
          <IntegerValue>1</IntegerValue>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="NAD83-OBFUSCATED">
    <Name>NAD83-OBFUSCATED</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980-OBFUSCATED</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980-OBFUSCATED">
    <Name>GRS1980-OBFUSCATED</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD83-OBFUSCATED_to_WGS84">
    <Name>NAD83-OBFUSCATED_to_WGS84</Name>
    <SourceDatumId>NAD83-OBFUSCATED</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",

// To test 7 param transform matching for datum
R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Xian80-OBF.GK3d-37">
    <Name>Xian80-OBF.GK3d-37</Name>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DatumId>Xian80-OBF</DatumId>
    <Axis uom="Meter">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">111</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">1</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="Meter">37500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="Meter">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="Xian80-OBF">
    <Name>Xian80-OBF</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>Xian80</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="Xian80">
    <Name>Xian80</Name>
    <SemiMajorAxis uom="meter">6378140</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356755.288158</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="Xian80-OBF_to_WGS84">
    <Name>Xian80-OBF_to_WGS84</Name>
    <SourceDatumId>Xian80-OBF</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">24</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-123</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-94</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-5.55555555555556e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">-6.94444444444444e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">3.61111111111111e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">1.1e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>
)X",
R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="JPNGSI07-Grid">
    <Name>JPNGSI07-Grid</Name>
    <Description>Japan GSI Plane No. 07, GSI datum; use with JGD2000 Grid Files</Description>
    <Authority>Autodesk</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>136.05</WestBoundLongitude>
            <EastBoundLongitude>138.05</EastBoundLongitude>
            <SouthBoundLatitude>34.25</SouthBoundLatitude>
            <NorthBoundLatitude>37.8333333333333</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>JPNGSI-Grid</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Transverse Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">137.166666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">36</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="JPNGSI-Grid">
    <Name>JPNGSI-Grid</Name>
    <Description>Japan, GSI Datum for use with JGD 2000 Grid Files</Description>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>BESL-JPN</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="BESL-JPN">
    <Name>BESL-JPN</Name>
    <Description>Bessel - 1841, Japan GSI</Description>
    <Authority>Autodesk</Authority>
    <SemiMajorAxis uom="meter">6377397.15</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356078.95784915</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="JPNGSI-Grid_to_JGD2000">
    <Name>JPNGSI-Grid_to_JGD2000</Name>
    <Description>Japan, GSI Datum for use with JGD 2000 Grid Files</Description>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>TransformationGridFileFallback</Key>
        <StringValue>TOKYO_to_WGS84</StringValue>
      </ParameterItem>
    </AdditionalInformation>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>JPNGSI-Grid</SourceDatumId>
    <TargetDatumId>JGD2000</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>Japanese Grid Mesh Interpolation</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Japan\TKY2JGD.par</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="JGD2000_to_WGS84">
    <Name>JGD2000_to_WGS84</Name>
    <Description>Japan Geodetic Datum 2000</Description>
    <Authority>Japanese Geographic Institute</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>117.79</WestBoundLongitude>
            <EastBoundLongitude>164.79</EastBoundLongitude>
            <SouthBoundLatitude>15.64625</SouthBoundLatitude>
            <NorthBoundLatitude>49.28375</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>JGD2000</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1826" type="Transformation">
    <ObjectId>JGD2000_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="JPNGSI-Grid_to_WGS84">
    <Name>JPNGSI-Grid_to_WGS84</Name>
    <Description>Japan, GSI Datum for use with JGD 2000 Grid Files</Description>
    <SourceDatumId>JPNGSI-Grid</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>JPNGSI-Grid_to_JGD2000</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>JGD2000_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>


)X",
R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="AK-1">
    <Name>AK-1</Name>
    <Description>NAD27 datum, Alaska State Planes; Zone 1(5001), US Foot</Description>
    <Authority>Snyder, J.P., 1987, Map Projections - A Working Manual</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-150</WestBoundLongitude>
            <EastBoundLongitude>-124</EastBoundLongitude>
            <SouthBoundLatitude>50</SouthBoundLatitude>
            <NorthBoundLatitude>64</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NAD27</DatumId>
    <Axis uom="FOOT">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Hotine Oblique Mercator (variant A)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of projection center</OperationParameterId>
          <Value uom="degree">-133.666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of projection center</OperationParameterId>
          <Value uom="degree">57</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Azimuth of initial line</OperationParameterId>
          <Value uom="degree">-36.8698976458333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="FOOT">16404166.6667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="FOOT">-16404166.6667</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="26731" type="CoordinateSystem">
    <ObjectId>AK-1</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NAD27">
    <Name>NAD27</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>CLRK66</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="CLRK66">
    <Name>CLRK66</Name>
    <Description>Clarke 1866, Benoit Ratio</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378206.4</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356583.8</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD27_to_NAD83">
    <Name>NAD27_to_NAD83</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>TransformationGridFileFallback</Key>
        <StringValue>NAD27-48_to_WGS84</StringValue>
      </ParameterItem>
    </AdditionalInformation>
    <SourceDatumId>NAD27</SourceDatumId>
    <TargetDatumId>NAD83</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\conus.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Nadcon\conus.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <Description>NAD 1983, Alaska, Canada,Continental US,Mexico,Central America</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, August 1993</Authority>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">4</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1188" type="Transformation">
    <ObjectId>NAD83_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="NAD27_to_WGS84">
    <Name>NAD27_to_WGS84</Name>
    <Description>NAD 1927, mean Values, Continental United States</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SourceDatumId>NAD27</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>NAD27_to_NAD83</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>


)X",
R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="CzechJTSK/5.Krovak">
    <Name>CzechJTSK/5.Krovak</Name>
    <Description>Deprecated (datum); replaced by CzechJTSK/5b.Krovak.</Description>
    <Authority>Prof. ing. Bohuslav Veverka, DrSc, GEOSOFT</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>0</WestBoundLongitude>
            <EastBoundLongitude>29.3334068333333</EastBoundLongitude>
            <SouthBoundLatitude>42</SouthBoundLatitude>
            <NorthBoundLatitude>55</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>CzechJTSK/5</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Southing</AxisName>
        <AxisAbbreviation>S</AxisAbbreviation>
        <AxisDirection>south</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Westing</AxisName>
        <AxisAbbreviation>W</AxisAbbreviation>
        <AxisDirection>west</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Krovak</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Oblique Pole Longitude</OperationParameterId>
          <Value uom="degree">42.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Oblique Pole Latitude</OperationParameterId>
          <Value uom="degree">59.7575985555556</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of pseudo standard parallel</OperationParameterId>
          <Value uom="degree">78.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-17.6666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">49.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.9999</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="2065" type="CoordinateSystem">
    <ObjectId>CzechJTSK/5.Krovak</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="CzechJTSK/5">
    <Name>CzechJTSK/5</Name>
    <Description>Deprecated (rotation sign), replaced by CzechJTSK/5b.</Description>
    <Authority>Transformace souradnic ze systemu WGS84 do S-JTSK</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>BESSEL</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6156" type="Datum">
    <ObjectId>CzechJTSK/5</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="BESSEL">
    <Name>BESSEL</Name>
    <Description>Bessel, 1841</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377397.155</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356078.96281819</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7004" type="Ellipsoid">
    <ObjectId>BESSEL</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="CzechJTSK/5_to_WGS84">
    <Name>CzechJTSK/5_to_WGS84</Name>
    <Description>Deprecated (rotations); replaced by CzechJTSK/5b_to_WGS84.</Description>
    <Authority>Transformace souradnic ze systemu WGS84 do S-JTSK</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>11.25125</WestBoundLongitude>
            <EastBoundLongitude>19.83875</EastBoundLongitude>
            <SouthBoundLatitude>48.24375</SouthBoundLatitude>
            <NorthBoundLatitude>51.40625</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>5</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>CzechJTSK/5</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">572.213</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">85.334</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">461.94</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">0.00138144444444444</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">0.000424722222222222</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.00145788888888889</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">3.5378e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1623" type="Transformation">
    <ObjectId>CzechJTSK/5_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>


)X",
R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="NSRS11.SD-S">
    <Name>NSRS11.SD-S</Name>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-105.016666666667</WestBoundLongitude>
            <EastBoundLongitude>-95.4833333333333</EastBoundLongitude>
            <SouthBoundLatitude>42.2666666666667</SouthBoundLatitude>
            <NorthBoundLatitude>45.0166666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>NSRS11</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">44.4</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">42.8333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-100.333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">42.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">600000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="6573" type="CoordinateSystem">
    <ObjectId>NSRS11.SD-S</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="NSRS11">
    <Name>NSRS11</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980">
    <Name>GRS1980</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NSRS2007_to_NSRS2011">
    <Name>NSRS2007_to_NSRS2011</Name>
    <SourceDatumId>NSRS07</SourceDatumId>
    <TargetDatumId>NSRS11</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2011\dsl?11.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2011\dsl?a11.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2011\dsl?p11.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="NAD83/HARN_to_NSRS2007">
    <Name>NAD83/HARN_to_NSRS2007</Name>
    <SourceDatumId>NAD83/HARN</SourceDatumId>
    <TargetDatumId>NSRS07</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2007\dsl?.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2007\dsl?a.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
      <OperationMethod>
        <OperationMethodId>GEOCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude and longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\NSRS2007\dsl?p.b</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="NAD83_to_NAD83/HARN">
    <Name>NAD83_to_NAD83/HARN</Name>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>NAD83/HARN</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>NADCON</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Harn\48hpgn.las</ValueGridFile>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude difference file</OperationParameterId>
          <ValueGridFile direction="forward">.\Usa\Harn\48hpgn.los</ValueGridFile>
        </ParameterValue>
      </OperationMethod>

    </OperationMethodGroup>
  </Transformation>

  <Transformation id="NAD83_to_WGS84">
    <Name>NAD83_to_WGS84</Name>
    <SourceDatumId>NAD83</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <TransformationPath id="NSRS11_to_WGS84">
    <Name>NSRS11_to_WGS84</Name>
    <Description>National Spatial Reference System of 2011</Description>
    <Authority>National Geodetic Survey (ngs.noaa.gov/NationalReadjustment)</Authority>
    <SourceDatumId>NSRS11</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>NSRS2007_to_NSRS2011</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83/HARN_to_NSRS2007</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83_to_NAD83/HARN</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>NAD83_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>


)X",
R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="Ocotepeque35b.CR-N">
    <Name>Ocotepeque35b.CR-N</Name>
    <Description>Costa Rica, Norte; Meter;  Ocotepeque 1935 (B)</Description>
    <Authority>ESTUDIOS BÝSICOS DE INGENIERIA, 31 Oct 2007</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DatumId>Ocotepeque35b</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Latitude of 1st standard parallel</OperationParameterId>
          <Value uom="degree">9.93333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of 2nd standard parallel</OperationParameterId>
          <Value uom="degree">11</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">-84.3333333333333</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">10.4666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">500000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">271820.522</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="Ocotepeque35b">
    <Name>Ocotepeque35b</Name>
    <Description>Ocotepeque 1935 Datum (Costa Rica) Version B (Molodensky Badeka</Description>
    <Authority>Instituto Geografico Nacional</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>CLRK66</EllipsoidId>
  </GeodeticDatum>

  <Alias id="1070" type="Datum">
    <ObjectId>Ocotepeque35b</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="CLRK66">
    <Name>CLRK66</Name>
    <Description>Clarke 1866, Benoit Ratio</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378206.4</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356583.8</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7008" type="Ellipsoid">
    <ObjectId>CLRK66</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="Ocotepeque35b_to_WGS84">
    <Name>Ocotepeque35b_to_WGS84</Name>
    <Description>Ocotepeque 1935 Datum (Costa Rica) Version B (Molodensky Badeka</Description>
    <Authority>Instituto Geografico Nacional</Authority>
    <OperationVersion>2</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">5</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>Ocotepeque35b</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Molodensky-Badekas (geog2D domain)</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">213.116</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">9.358</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-74.946</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-0.000653171886388889</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">1.70741422222222e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">-0.001776169165</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">-5.22e-06</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Ordinate 1 of evaluation point</OperationParameterId>
        <Value uom="meter">617749.7118</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Ordinate 2 of evaluation point</OperationParameterId>
        <Value uom="meter">-6250547.7336</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Ordinate 3 of evaluation point</OperationParameterId>
        <Value uom="meter">1102063.6099</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="6889" type="Transformation">
    <ObjectId>Ocotepeque35b_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>


)X",

R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="HD72/7Pa.EOV">
    <Name>HD72/7Pa.EOV</Name>
    <Description>Hungary, Uniform National Projection System of 1972</Description>
    <Authority>http://lazarus.elte.hu/gb/geodez/geod2.htm</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>15</WestBoundLongitude>
            <EastBoundLongitude>25</EastBoundLongitude>
            <SouthBoundLatitude>45</SouthBoundLatitude>
            <NorthBoundLatitude>50</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>HD72/7Pa</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Oblique Cylindrical</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Normal Parallel</OperationParameterId>
          <Value uom="degree">47.1666666666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">19.0485717777778</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">47.1443937222222</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.99993</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">650000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">200000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="23700" type="CoordinateSystem">
    <ObjectId>HD72/7Pa.EOV</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="HD72/7Pa">
    <Name>HD72/7Pa</Name>
    <Description>Hungarian Datum 1972, Hungary (7 Parameter Transform)</Description>
    <Authority>http://www.lazarus.elte.hu/gb/geodez/geod5.htm</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1967</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6237" type="Datum">
    <ObjectId>HD72/7Pa</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="GRS1967">
    <Name>GRS1967</Name>
    <Description>Geodetic Reference System, 1967</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378160</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356774.516</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7036" type="Ellipsoid">
    <ObjectId>GRS1967</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="HD72/7Pa_to_WGS84">
    <Name>HD72/7Pa_to_WGS84</Name>
    <Description>Hungarian Datum 1972, Hungary (7 Parameter Transform)</Description>
    <Authority>http://www.lazarus.elte.hu/gb/geodez/geod5.htm</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">3</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>HD72/7Pa</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">52.684</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">-71.194</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-13.975</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">8.66666666666667e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">2.95277777777778e-05</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">0.000103583333333333</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">1.0191e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

  <Alias id="1448" type="Transformation">
    <ObjectId>HD72/7Pa_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>


)X",

R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="IGN-I-Grid">
    <Name>IGN-I-Grid</Name>
    <Description>Deprecated as duplicate of NTF.Lambert-1</Description>
    <Authority>IGN France</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DatumId>NTF-G-Grid</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Tangential Conformal Conic</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of false origin</OperationParameterId>
          <Value uom="degree">2.33722916666667</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of false origin</OperationParameterId>
          <Value uom="degree">49.5</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.99987734</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">600000</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">200000</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="NTF-G-Grid">
    <Name>NTF-G-Grid</Name>
    <Description>Nouvelle Triangulation Francaise, for use with RGF93 Grid files</Description>
    <Authority>IGN Paris</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>CLRK80</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6275" type="Datum">
    <ObjectId>NTF-G-Grid</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="CLRK80">
    <Name>CLRK80</Name>
    <Description>Clarke 1880, RGS</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378249.145</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356514.87</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7012" type="Ellipsoid">
    <ObjectId>CLRK80</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Transformation id="RGF93_to_NTF-G-Grid">
    <Name>RGF93_to_NTF-G-Grid</Name>
    <Description>Nouvelle Triangulation Francaise, for use with RGF93 Grid files</Description>
    <Authority>IGN Paris</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>TransformationGridFileFallback</Key>
        <StringValue>WGS84_to_NTF-G</StringValue>
      </ParameterItem>
    </AdditionalInformation>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>RGF93</SourceDatumId>
    <TargetDatumId>NTF-G-Grid</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethodGroup>
      <OperationMethod>
        <OperationMethodId>France geocentric interpolation</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Geocentric translation file</OperationParameterId>
          <ValueGridFile direction="forward">.\France\gr3df97a.txt</ValueGridFile>
        </ParameterValue>
      </OperationMethod>
    </OperationMethodGroup>
  </Transformation>

  <Transformation id="RGF93_to_WGS84">
    <Name>RGF93_to_WGS84</Name>
    <Description>Reseau Geodesique Francais, RGF93</Description>
    <Authority>IGN Paris</Authority>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-12.11</WestBoundLongitude>
            <EastBoundLongitude>12.79</EastBoundLongitude>
            <SouthBoundLatitude>39.885</SouthBoundLatitude>
            <NorthBoundLatitude>52.835</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <OperationVersion>1</OperationVersion>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">1</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>RGF93</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

  <Alias id="1671" type="Transformation">
    <ObjectId>RGF93_to_WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <TransformationPath id="NTF-G-Grid_to_WGS84">
    <Name>NTF-G-Grid_to_WGS84</Name>
    <Description>Nouvelle Triangulation Francaise, for use with RGF93 Grid files</Description>
    <Authority>IGN Paris</Authority>
    <SourceDatumId>NTF-G-Grid</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <TransformationOperationGroup>
      <TransformationOperation>
        <TransformationId>RGF93_to_NTF-G-Grid</TransformationId>
        <Direction>inverse</Direction>
      </TransformationOperation>
      <TransformationOperation>
        <TransformationId>RGF93_to_WGS84</TransformationId>
        <Direction>forward</Direction>
      </TransformationOperation>
    </TransformationOperationGroup>
  </TransformationPath>

</Dictionary>


)X",

R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="WGS84.PseudoMercator">
    <Name>WGS84.PseudoMercator</Name>
    <Description>WGS84 based Mercator (spherical formulation)</Description>
    <Authority>Autodesk</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>-180.75</WestBoundLongitude>
            <EastBoundLongitude>180.75</EastBoundLongitude>
            <SouthBoundLatitude>-85.8166666666667</SouthBoundLatitude>
            <NorthBoundLatitude>85.8166666666667</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>WGS84</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Popular Visualisation Pseudo Mercator</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of natural origin</OperationParameterId>
          <Value uom="degree">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <Alias id="3857" type="CoordinateSystem">
    <ObjectId>WGS84.PseudoMercator</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <GeodeticDatum id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>WGS84</EllipsoidId>
  </GeodeticDatum>

  <Alias id="6326" type="Datum">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

  <Ellipsoid id="WGS84">
    <Name>WGS84</Name>
    <Description>World Geodetic System of 1984, GEM 10C</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31424518</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Alias id="7030" type="Ellipsoid">
    <ObjectId>WGS84</ObjectId>
    <Namespace>EPSG Code</Namespace>
  </Alias>

</Dictionary>


)X",

R"X(
<?xml version="1.0" encoding="UTF-16" standalone="no" ?>
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="MAL-RSO-SK">
    <Name>MAL-RSO-SK</Name>
    <Description>East Malaysia (Sarawak),RSO System,meter;Timbalai Sarawak datum</Description>
    <Authority>Department of Surveying and Mapping, Malaysia</Authority>
    <AdditionalInformation>
      <ParameterItem type="CsMap">
        <Key>CSQuadrantSimplified</Key>
        <IntegerValue>1</IntegerValue>
      </ParameterItem>
    </AdditionalInformation>
    <DomainOfValidity>
      <Extent>
        <GeographicElement>
          <GeographicBoundingBox>
            <WestBoundLongitude>109.5</WestBoundLongitude>
            <EastBoundLongitude>119.5</EastBoundLongitude>
            <SouthBoundLatitude>0.833333333333333</SouthBoundLatitude>
            <NorthBoundLatitude>8</NorthBoundLatitude>
          </GeographicBoundingBox>
        </GeographicElement>
      </Extent>
    </DomainOfValidity>
    <DatumId>TIMBALAI68-SK</DatumId>
    <Axis uom="METER">
      <CoordinateSystemAxis>
        <AxisOrder>1</AxisOrder>
        <AxisName>Easting</AxisName>
        <AxisAbbreviation>E</AxisAbbreviation>
        <AxisDirection>east</AxisDirection>
      </CoordinateSystemAxis>
      <CoordinateSystemAxis>
        <AxisOrder>2</AxisOrder>
        <AxisName>Northing</AxisName>
        <AxisAbbreviation>N</AxisAbbreviation>
        <AxisDirection>north</AxisDirection>
      </CoordinateSystemAxis>
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Rectified Skew Orthomorphic, Skew Azimuth at Rectified Origin</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>Longitude of projection center</OperationParameterId>
          <Value uom="degree">115</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Latitude of projection center</OperationParameterId>
          <Value uom="degree">4</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Angle from Rectified to Skew Grid</OperationParameterId>
          <Value uom="degree">53.1301023611111</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>Scaling factor for coord differences</OperationParameterId>
          <Value uom="unity">0.99984</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False easting</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
        <ParameterValue>
          <OperationParameterId>False northing</OperationParameterId>
          <Value uom="METER">0</Value>
        </ParameterValue>
      </Projection>
    </Conversion>
  </ProjectedCoordinateSystem>

  <GeodeticDatum id="TIMBALAI68-SK">
    <Name>TIMBALAI68-SK</Name>
    <Description>Timbalai 1968, Sarawak East Malaysia-DSMM (7 Param Transform)</Description>
    <Authority>Department of Surveying and Mapping, Malaysia</Authority>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>EVRST-TM</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="EVRST-TM">
    <Name>EVRST-TM</Name>
    <Description>Everest 1830, Timbalai (1967 Adjustment)</Description>
    <Authority>US Defense Mapping Agency, TR-8350.2-B, December 1987</Authority>
    <SemiMajorAxis uom="meter">6377298.561</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356097.555</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="TIMBALAI68-SK_to_WGS84">
    <Name>TIMBALAI68-SK_to_WGS84</Name>
    <Description>Timbalai 1968, Sarawak East Malaysia-DSMM (7 Param Transform)</Description>
    <Authority>Department of Surveying and Mapping, Malaysia</Authority>
    <CoordinateOperationAccuracy>
      <Accuracy uom="meter">3</Accuracy>
    </CoordinateOperationAccuracy>
    <SourceDatumId>TIMBALAI68-SK</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Seven Parameter Transformation</OperationMethodId>
      <ParameterValue>
        <OperationParameterId>X-axis translation</OperationParameterId>
        <Value uom="meter">-520.310254717</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis translation</OperationParameterId>
        <Value uom="meter">675.810338464</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis translation</OperationParameterId>
        <Value uom="meter">-83.1190990571</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>X-axis rotation</OperationParameterId>
        <Value uom="degree">-0.000353713585960278</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Y-axis rotation</OperationParameterId>
        <Value uom="degree">0.000133460942906389</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Z-axis rotation</OperationParameterId>
        <Value uom="degree">-0.00132261053721611</Value>
      </ParameterValue>
      <ParameterValue>
        <OperationParameterId>Scale difference</OperationParameterId>
        <Value uom="unity">9.375247894461e-06</Value>
      </ParameterValue>
    </OperationMethod>
  </Transformation>

</Dictionary>


)X",
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SpecificXMLParseTests, ParseAllXML)
{
    int i = 0;
    int allNum = (int)ListOfOSGEO_XML.size();

    for (auto xml : ListOfOSGEO_XML)
        {
        i++;
        printf("[TRACE] Processing %i of %i\n", i, allNum);
        
        GeoCoordinates::BaseGCSPtr theGCS = GeoCoordinates::BaseGCS::CreateGCS();

        ASSERT_TRUE(theGCS.IsValid());

        EXPECT_TRUE(GeoCoordParse_Success == theGCS->InitFromOSGEOXML(xml.c_str()));

        EXPECT_TRUE(theGCS->IsValid());
        }
}
