//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------
#include <Bentley/BeTest.h>
#include <Bentley/BeTextFile.h>
#include <GeoCoord/BaseGeoCoord.h>
#include "GeoCoordTestCommon.h"

using namespace ::testing;
using namespace GeoCoordinates;

/*---------------------------------------------------------------------------------**//**
* @bsi
+---------------+---------------+---------------+---------------+---------------+------*/
class ParseErrorStruct {
    public:
        bool m_isWKT;
        GeoCoordParseStatus m_expected;
        Utf8String m_definition;
};

class SpecificParseErrorTests : public ::testing::TestWithParam< ParseErrorStruct >
    {   
    public:
        virtual void SetUp() { GeoCoordTestCommon::Initialize(); };
        virtual void TearDown() {GeoCoordTestCommon::Shutdown();};

        SpecificParseErrorTests() {};
        ~SpecificParseErrorTests() {};
    };


static bvector<ParseErrorStruct> s_ListOfDefs =
    {
    {true, GeoCoordParse_Success,   R"X(PROJCS["OSTN 2015 - British National Grid (update for EPSG:27700)", GEOGCS["OSGB/OSTN15", DATUM["OSGB/OSTN15_to_ETRF89", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]], TOWGS84[446.448,-125.157,542.06,0.15,0.247,0.842,-20.489], EXTENSION["PROJ4_GRIDS","OSTN15_NTv2_OSGBtoETRS.gsb"]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433], AUTHORITY["EPSG","5338"]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1, AUTHORITY["EPSG","9001"]], AXIS["Easting",EAST], AXIS["Northing",NORTH], AUTHORITY["EPSG","27700"]])X"},
    {true, GeoCoordParse_Success,   R"X(PROJCS["OSTN 2015", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1], AXIS["Easting",EAST], AXIS["Northing",NORTH]])X"},

    {true, GeoCoordParse_NoGCS,   R"X(PRO["OSTN 2002 - British National Grid (update for EPSG:27700)", GEOGCS["OSGB/OSTN02", DATUM["OSGB/OSTN02_to_ETRF89", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]], TOWGS84[446.448,-125.157,542.06,0.15,0.247,0.842,-20.489], EXTENSION["PROJ4_GRIDS","ostn02.gsb"]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433], AUTHORITY["EPSG","7709"]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1, AUTHORITY["EPSG","9001"]], AXIS["Easting",EAST], AXIS["Northing",NORTH], AUTHORITY["EPSG","27700"]])X"},
    {true, GeoCoordParse_UnknownDatum, R"X(PROJCS["Pulkovo",GEOGCS["Pulkovo",DATUM["UnknownName",SPHEROID["Krassowsky 1940",6378245,298.2999999999985]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",45],PARAMETER["scale_factor",1],PARAMETER["false_easting",15500000],PARAMETER["false_northing",0],UNIT["metre",1]])X"},
    {true, GeoCoordParse_NoEllipsoid, R"X(PROJCS["Pulkovo",GEOGCS["Pulkovo",DATUM["UnknownName"],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",45],PARAMETER["scale_factor",1],PARAMETER["false_easting",15500000],PARAMETER["false_northing",0],UNIT["metre",1]])X"},

    {true, GeoCoordParse_UnknownProjectionMethod, R"X(PROJCS["Amersfoort",GEOGCS["Amersfoort",DATUM["Amersfoort",SPHEROID["Bessel 1841",6377397.155,299.1528128,AUTHORITY["EPSG","7004"]],TOWGS84[565.2369,50.0087,465.658,-0.406857,0.350733,-1.87035,4.0812],AUTHORITY["EPSG","6289"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4289"]],PROJECTION["StrangeUnknownProjection"],PARAMETER["latitude_of_origin",52.15616055555555],PARAMETER["central_meridian",5.38763888888889],PARAMETER["scale_factor",0.9999079],PARAMETER["false_easting",155000],PARAMETER["false_northing",463000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X",EAST],AXIS["Y",NORTH],AUTHORITY["EPSG","28992"]])X"},
    {true, GeoCoordParse_UnknownProjectionParam,   R"X(PROJCS["OSTN 201", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]], TOWGS84[446.448,-125.157,542.06,0.15,0.247,0.842,-20.489], EXTENSION["PROJ4_GRIDS","OSTN15_NTv2_OSGBtoETRS.gsb"]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433], AUTHORITY["EPSG","5338"]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin_sabotaged",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1, AUTHORITY["EPSG","9001"]], AXIS["Easting",EAST], AXIS["Northing",NORTH], AUTHORITY["EPSG","27700"]])X"},
    {true, GeoCoordParse_InvalidParamForMethod,   R"X(PROJCS["OSTN 2015", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433]], PROJECTION["Transverse_Mercator"], PARAMETER["Azimuth",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1, AUTHORITY["EPSG","9001"]], AXIS["Easting",EAST], AXIS["Northing",NORTH], AUTHORITY["EPSG","27700"]])X"},

    {true, GeoCoordParse_BadAxis,   R"X(PROJCS["OSTN 2015", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1], AXIS["Easting",WEST], AXIS["Northing",NONSENSE]])X"},

    {true, GeoCoordParse_UnknownUnit,   R"X(PROJCS["OSTN 2015", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]]], PRIMEM["Greenwich",0], UNIT["degree",0.0174532925199433]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["Notmetre",1.0342], AXIS["Easting",EAST], AXIS["Northing",NORTH]])X"},
    {true, GeoCoordParse_UnsupportedMeridian,   R"X(PROJCS["OSTN 2015", GEOGCS["OSGB/OSTN15_to_ETRF89", DATUM["OSGB/OSTN15", SPHEROID["Airy 1830",6377563.396,299.3249646,AUTHORITY["EPSG","7001"]]], PRIMEM["NotGreenwich",12.3], UNIT["degree",0.0174532925199433]], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin",49], PARAMETER["central_meridian",-2], PARAMETER["scale_factor",0.9996012717], PARAMETER["false_easting",400000], PARAMETER["false_northing",-100000], UNIT["metre",1], AXIS["Easting",EAST], AXIS["Northing",NORTH]])X"},

    {true, GeoCoordParse_InvalidDefinition,       R"X(PROJCS["RGF93 / CC43",GEOGCS["RGF93",DATUM["Reseau_Geodesique_Francais_1993",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY["EPSG","6171"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4171"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",90],PARAMETER["standard_parallel_2",90],PARAMETER["latitude_of_origin",43],PARAMETER["central_meridian",3],PARAMETER["false_easting",1700000],PARAMETER["false_northing",2200000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["X",EAST],AXIS["Y",NORTH],AUTHORITY["EPSG","3943"]])X"},
    {true, GeoCoordParse_BadGCS,   R"X(LOCAL_CS["", LOCAL_DATUM["AnywhereXYZ", 11000, AUTHORITY["BENTLEY_SYSTEMS", "11000"]], UNIT["meter", 1], AXIS["X", OTHER], AXIS["Y", OTHER], AXIS["Z", OTHER], AUTHORITY["BENTLEY_SYSTEMS", "0"]])X"},
    {false, GeoCoordParse_Success,  
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

)X"     
     },
     {false, GeoCoordParse_NoGCS,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">
</Dictionary>
)X"     
     },

    {false, GeoCoordParse_NoGCS,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <GeodeticDatum id="NAD83A">
    <Name>NAD83A</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980A">
    <Name>GRS1980A</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD83A_to_WGS84">
    <Name>NAD83A_to_WGS84</Name>
    <SourceDatumId>NAD83A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <IsReversible>true</IsReversible>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>
</Dictionary>

)X"     
     },     

    {false, GeoCoordParse_NoDatum,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83FB">
    <Name>TN83FB</Name>
    <DatumId>NAD83A</DatumId>
    <Axis uom="FOOT">
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

  <Ellipsoid id="GRS1980A">
    <Name>GRS1980A</Name>
    <Description>Geodetic Reference System of 1980</Description>
    <Authority>Stem, L.E., Jan 1989, State Plane Coordinate System of 1983</Authority>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

</Dictionary>

)X"     
     },     

    {false, GeoCoordParse_Success,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83FA">
    <Name>TN83FA</Name>
    <DatumId>NAD83A</DatumId>
    <Axis uom="FOOT">
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

  <GeodeticDatum id="NAD83A">
    <Name>NAD83A</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980A</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980A">
    <Name>GRS1980A</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD83A_to_WGS84">
    <Name>NAD83A_to_WGS84</Name>
    <SourceDatumId>NAD83A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

</Dictionary>

)X"     
     },     

    {false, GeoCoordParse_NoEllipsoid,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83FA">
    <Name>TN83FA</Name>
    <DatumId>NAD83A</DatumId>
    <Axis uom="FOOT">
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

  <GeodeticDatum id="NAD83A">
    <Name>NAD83A</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980A</EllipsoidId>
  </GeodeticDatum>

  <Transformation id="NAD83A_to_WGS84">
    <Name>NAD83A_to_WGS84</Name>
    <SourceDatumId>NAD83A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

</Dictionary>

)X"     
     },      

    {false, GeoCoordParse_UnknownProjectionMethod,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83FA">
    <Name>TN83FA</Name>
    <DatumId>NAD83A</DatumId>
    <Axis uom="FOOT">
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>INVALID</OperationMethodId>
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

  <GeodeticDatum id="NAD83A">
    <Name>NAD83A</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980A</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980A">
    <Name>GRS1980A</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD83A_to_WGS84">
    <Name>NAD83A_to_WGS84</Name>
    <SourceDatumId>NAD83A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

</Dictionary>

)X"     
     },      

         {false, GeoCoordParse_UnknownProjectionParam,  
    R"X(
<Dictionary version="1.0" xmlns="http://www.osgeo.org/mapguide/coordinatesystem">

  <ProjectedCoordinateSystem id="TN83FA">
    <Name>TN83FA</Name>
    <DatumId>NAD83A</DatumId>
    <Axis uom="FOOT">
    </Axis>
    <Conversion>
      <Projection>
        <OperationMethodId>Lambert Conic Conformal (2SP)</OperationMethodId>
        <ParameterValue>
          <OperationParameterId>INVALID</OperationParameterId>
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

  <GeodeticDatum id="NAD83A">
    <Name>NAD83A</Name>
    <PrimeMeridianId>Greenwich</PrimeMeridianId>
    <EllipsoidId>GRS1980A</EllipsoidId>
  </GeodeticDatum>

  <Ellipsoid id="GRS1980A">
    <Name>GRS1980A</Name>
    <SemiMajorAxis uom="meter">6378137</SemiMajorAxis>
    <SecondDefiningParameter>
      <SemiMinorAxis uom="meter">6356752.31414035</SemiMinorAxis>
    </SecondDefiningParameter>
  </Ellipsoid>

  <Transformation id="NAD83A_to_WGS84">
    <Name>NAD83A_to_WGS84</Name>
    <SourceDatumId>NAD83A</SourceDatumId>
    <TargetDatumId>WGS84</TargetDatumId>
    <OperationMethod>
      <OperationMethodId>Null transformation (no coordinate change)</OperationMethodId>
    </OperationMethod>
  </Transformation>

</Dictionary>

)X"     
     },    
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P (SpecificParseErrorTests, ParseDefTest)
    {
    ParseErrorStruct theParseTestParam = GetParam(); 

    BaseGCSPtr currentGCS;

    currentGCS = BaseGCS::CreateGCS();

    if (theParseTestParam.m_isWKT)
        {
        Utf8String wellKnownText = theParseTestParam.m_definition;
        GeoCoordParseStatus status;
        status = currentGCS->InitFromWellKnownText(NULL, NULL, wellKnownText.c_str());

        EXPECT_TRUE(theParseTestParam.m_expected == status) << "Expected: " << (int)theParseTestParam.m_expected << " for: " << wellKnownText.c_str() << std::endl;
        }
    else
        {
        Utf8String theXML = theParseTestParam.m_definition;
        GeoCoordParseStatus status;
        status = currentGCS->InitFromOSGEOXML(theXML.c_str());

        EXPECT_TRUE(theParseTestParam.m_expected == status) << "Expected: " << (int)theParseTestParam.m_expected << " for: " << theXML.c_str() << std::endl;
        }
    }

    
INSTANTIATE_TEST_SUITE_P(SpecificParseErrorTests_Combined,
                        SpecificParseErrorTests,
                        ValuesIn(s_ListOfDefs));
