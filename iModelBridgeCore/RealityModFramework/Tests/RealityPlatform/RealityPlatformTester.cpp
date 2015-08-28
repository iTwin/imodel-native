//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityPlatformTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <Bentley/BeTest.h>
#include <RealityPlatform/WMSCapabilities.h>

USING_NAMESPACE_BENTLEY_WMSPARSER

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)
#define _WIDEN(quote) L##quote
#define WIDEN(quote) _WIDEN(quote)


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
class PlatformTestFixture : public testing::Test 
{
public:
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    BeFileName BuildOutputFilename(WCharCP filename)
        {
        BeFileName outputFilePath;
        BeTest::GetHost().GetOutputRoot (outputFilePath);
        outputFilePath.AppendToPath(filename);

        return outputFilePath;
        }
};

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 3/2015
//-------------------------------------------------------------------------------------
TEST_F (PlatformTestFixture, ReadVersion_1_3)
    {
    ASSERT_EQ(1/*expected*/, 1/*actual*/);    

    #define WMSCAPABILITIES_VERSION "1.3.0"

    #define WMSSERVICE_NAME           "WMS"
    #define WMSSERVICE_TITLE "Acme Corp.Map Server"
    #define WMSSERVICE_ABSTRACT "Map Server maintained by Acme Corporation.Contact: webmaster@wmt.acme.com.High - quality maps showing roadrunner nests and possible ambush locations."
    #define WMSSERVICE_KEYWORD_0 "bird"
    #define WMSSERVICE_KEYWORD_1 "roadrunner"
    #define WMSSERVICE_KEYWORD_2 "ambush"
    #define WMSSERVICE_ONLINERES_HREF "http://hostname/"
    #define WMSSERVICE_ONLINERES_TYPE "simple"
    #define WMSSERVICE_CONTACT_PERS_NAME "Jeff Smith"
    #define WMSSERVICE_CONTACT_PERS_ORG "NASA"
    #define WMSSERVICE_CONTACT_POS "Computer Scientist"
    #define WMSSERVICE_CONTACT_ADDR_TYPE "postal"
    #define WMSSERVICE_CONTACT_ADDR "NASA Goddard Space Flight Center"
    #define WMSSERVICE_CONTACT_ADDR_CITY "Greenbelt"
    #define WMSSERVICE_CONTACT_ADDR_STATE "MD"
    #define WMSSERVICE_CONTACT_ADDR_POSTCODE "20771"
    #define WMSSERVICE_CONTACT_ADDR_COUNTRY "USA"
    #define WMSSERVICE_CONTACT_VOICE_TEL "+1 301 555 - 1212"
    #define WMSSERVICE_CONTACT_EMAIL "user@host.com"
    #define WMSSERVICE_FEES "none"
    #define WMSSERVICE_ACCESS_CONST "none"
    #define WMSSERVICE_LAYER_LIMIT 16
    #define WMSSERVICE_MAX_WIDTH 2048
    #define WMSSERVICE_MAX_HEIGHT 2048

    #define WMSCAPABILITY_FORMAT_XML "text/xml"
    #define WMSCAPABILITY_FORMAT_PLAIN "text/plain"
    #define WMSCAPABILITY_FORMAT_HTML "text/html"
    #define WMSCAPABILITY_FORMAT_XSL "text/xsl"
    #define WMSCAPABILITY_FORMAT_GIF "image/gif"
    #define WMSCAPABILITY_FORMAT_PNG "image/png"
    #define WMSCAPABILITY_FORMAT_JPEG "image/jpeg"
    #define WMSCAPABILITY_ONLINERES_HREF "http://hostname/path?"
    #define WMSCAPABILITY_ONLINERES_TYPE "simple"
    #define WMSCAPABILITY_EXCEPTION_XML "XML"
    #define WMSCAPABILITY_EXCEPTION_INIMAGE "INIMAGE"
    #define WMSCAPABILITY_EXCEPTION_BLANK "BLANK"

    #define WMSLAYER0_TITLE "Acme Corp.Map Server"
    #define WMSLAYER0_CRS "CRS:84"
    #define WMSLAYER0_AUTH_NAME "DIF_ID"
    #define WMSLAYER0_AUTH_ONLINERES_HREF "http://gcmd.gsfc.nasa.gov/difguide/whatisadif.html"

    #define WMSLAYER00_NAME "ROADS_RIVERS"
    #define WMSLAYER00_TITLE "Roads and Rivers"
    #define WMSLAYER00_CRS "EPSG:26986"
    #define WMSLAYER00_GEOBBOX_W -71.63
    #define WMSLAYER00_GEOBBOX_E -70.78
    #define WMSLAYER00_GEOBBOX_S 41.75
    #define WMSLAYER00_GEOBBOX_N 42.90
    #define WMSLAYER00_BBOX0_RES_X 0.01
    #define WMSLAYER00_BBOX0_RES_Y 0.01
    #define WMSLAYER00_BBOX0_MIN_X -71.63
    #define WMSLAYER00_BBOX0_MIN_Y 41.75
    #define WMSLAYER00_BBOX0_MAX_X -70.78
    #define WMSLAYER00_BBOX0_MAX_Y 42.90
    #define WMSLAYER00_BBOX0_CRS "CRS:84"
    #define WMSLAYER00_BBOX1_RES_X 1
    #define WMSLAYER00_BBOX1_RES_Y 1
    #define WMSLAYER00_BBOX1_MIN_X 189000
    #define WMSLAYER00_BBOX1_MIN_Y 834000
    #define WMSLAYER00_BBOX1_MAX_X 285000
    #define WMSLAYER00_BBOX1_MAX_Y 962000
    #define WMSLAYER00_BBOX1_CRS "EPSG:26986"
    #define WMSLAYER00_ATTR_TITLE "State College University"
    #define WMSLAYER00_ATTR_ONLINERES_HREF "http://www.university.edu/"
    #define WMSLAYER00_LOGO_HEIGHT 100
    #define WMSLAYER00_LOGO_WIDTH 100
    #define WMSLAYER00_LOGO_ONLINERES_HREF "http://www.university.edu/icons/logo.gif"
    #define WMSLAYER00_ID_AUTH "DIF_ID"
    #define WMSLAYER00_ID "123456"
    #define WMSLAYER00_FTLIST_FORMAT "XML"
    #define WMSLAYER00_FTLIST_ONLINERES_HREF "http://www.university.edu/data/roads_rivers.gml"
    #define WMSLAYER00_STYLE_NAME "USGS"
    #define WMSLAYER00_STYLE_TITLE "USGS Topo Map Style"
    #define WMSLAYER00_STYLE_ABSTRACT "Features are shown in a style like that used in USGS topographic maps."   
    #define WMSLAYER00_STYLE_LEG_HEIGHT 72
    #define WMSLAYER00_STYLE_LEG_WIDTH 72
    #define WMSLAYER00_STYLE_LEG_ONLINERES_HREF "http://www.university.edu/legends/usgs.gif"
    #define WMSLAYER00_STYLE_SHEET_ONLINERES_HREF "http://www.university.edu/stylesheets/usgs.xsl"

    #define WMSLAYER000_QUERY 1
    #define WMSLAYER000_NAME "ROADS_1M"
    #define WMSLAYER000_TITLE "Roads at 1:1M scale"
    #define WMSLAYER000_ABSTRACT "Roads at a scale of 1 to 1 million."
    #define WMSLAYER000_KEYWORD0 "road"
    #define WMSLAYER000_KEYWORD1 "transportation"
    #define WMSLAYER000_KEYWORD2 "atlas"
    #define WMSLAYER000_ID_AUTH "DIF_ID"
    #define WMSLAYER000_ID "123456"
    #define WMSLAYER000_METADATA0_TYPE "FGDC:1998"
    #define WMSLAYER000_METADATA0_ONLINERES_HREF "http://www.university.edu/metadata/roads.txt"
    #define WMSLAYER000_METADATA1_TYPE "ISO19115:2003"
    #define WMSLAYER000_METADATA1_ONLINERES_HREF "http://www.university.edu/metadata/roads.xml"
    #define WMSLAYER000_STYLE_NAME "ATLAS"
    #define WMSLAYER000_STYLE_TITLE "Road atlas style"
    #define WMSLAYER000_STYLE_ABSTRACT "Roads are shown in a style like that used in a commercial road atlas."   
    #define WMSLAYER000_STYLE_LEG_HEIGHT 72
    #define WMSLAYER000_STYLE_LEG_WIDTH 72
    #define WMSLAYER000_STYLE_LEG_ONLINERES_HREF "http://www.university.edu/legends/atlas.gif"

    #define WMSLAYER001_QUERY 1
    #define WMSLAYER001_NAME "RIVERS_1M"
    #define WMSLAYER001_TITLE "Rivers at 1:1M scale"
    #define WMSLAYER001_ABSTRACT "Rivers at a scale of 1 to 1 million."
    #define WMSLAYER001_KEYWORD0 "river"
    #define WMSLAYER001_KEYWORD1 "canal"
    #define WMSLAYER001_KEYWORD2 "waterway"

    #define WMSLAYER01_QUERY 1
    #define WMSLAYER01_TITLE "Weather Forecast Data"
    #define WMSLAYER01_CRS "CRS:84"
    #define WMSLAYER01_GEOBBOX_W -180
    #define WMSLAYER01_GEOBBOX_E 180
    #define WMSLAYER01_GEOBBOX_S -90
    #define WMSLAYER01_GEOBBOX_N 90
    #define WMSLAYER01_DIM_NAME "time"
    #define WMSLAYER01_DIM_DEFAULT "2000-08-22"
    #define WMSLAYER01_DIM_UNITS "ISO8601"
    #define WMSLAYER01_DIM "1999-01-01/2000-08-22/P1D"

    #define WMSLAYER010_NAME "Clouds"
    #define WMSLAYER010_TITLE "Forecast cloud cover"

    #define WMSLAYER011_NAME "Temperature"
    #define WMSLAYER011_TITLE "Forecast temperature"

    #define WMSLAYER012_NAME "Pressure"
    #define WMSLAYER012_TITLE "Forecast barometric pressure"
    #define WMSLAYER012_DIM0_NAME "elevation"
    #define WMSLAYER012_DIM0_UNITS "EPSG:5030"
    #define WMSLAYER012_DIM1_NAME "time"
    #define WMSLAYER012_DIM1_DEFAULT "2000-08-22"
    #define WMSLAYER012_DIM1_UNITS "ISO8601"
    #define WMSLAYER012_DIM1 "1999-01-01/2000-08-22/P1D"
    #define WMSLAYER012_DIM2_NAME "elevation"
    #define WMSLAYER012_DIM2_DEFAULT "0"
    #define WMSLAYER012_DIM2_UNITS "CRS:88"
    #define WMSLAYER012_DIM2_NEAREST_VALUE "1"
    #define WMSLAYER012_DIM2 "0,1000,3000,5000,10000"

    #define WMSLAYER02_FIXEDHEIGHT 256
    #define WMSLAYER02_FIXEDWIDTH 512
    #define WMSLAYER02_NOSUBSETS 1
    #define WMSLAYER02_OPAQUE 1
    #define WMSLAYER02_NAME "ozone_image"
    #define WMSLAYER02_TITLE "Global ozone distribution (1992)"
    #define WMSLAYER02_GEOBBOX_W -180
    #define WMSLAYER02_GEOBBOX_E 180
    #define WMSLAYER02_GEOBBOX_S -90
    #define WMSLAYER02_GEOBBOX_N 90
    #define WMSLAYER02_DIM_NAME "time"
    #define WMSLAYER02_DIM_DEFAULT "1992"
    #define WMSLAYER02_DIM_UNITS "ISO8601"
    #define WMSLAYER02_DIM "1992"

    #define WMSLAYER03_CASCADED 1
    #define WMSLAYER03_NAME "population"
    #define WMSLAYER03_TITLE "World population, annual"
    #define WMSLAYER03_GEOBBOX_W -180
    #define WMSLAYER03_GEOBBOX_E 180
    #define WMSLAYER03_GEOBBOX_S -90
    #define WMSLAYER03_GEOBBOX_N 90
    #define WMSLAYER03_DIM_NAME "time"
    #define WMSLAYER03_DIM_DEFAULT "2000"
    #define WMSLAYER03_DIM_UNITS "ISO8601"
    #define WMSLAYER03_DIM "1990/2000/P1Y"

    Utf8String capabilitiesString =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<WMS_Capabilities xsi:schemaLocation='http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.opengis.net/wms' version='1.3.0'>"
            "<Service>"
                "<Name>" WMSSERVICE_NAME "</Name>"
                "<Title>" WMSSERVICE_TITLE "</Title>"
                "<Abstract>" WMSSERVICE_ABSTRACT "</Abstract>"
                "<KeywordList>"
                    "<Keyword>" WMSSERVICE_KEYWORD_0 "</Keyword>"
                    "<Keyword>" WMSSERVICE_KEYWORD_1 "</Keyword>"
                    "<Keyword>" WMSSERVICE_KEYWORD_2 "</Keyword>"
                "</KeywordList>"
                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSSERVICE_ONLINERES_HREF "' xlink:type='" WMSSERVICE_ONLINERES_TYPE "'/>"
                "<ContactInformation>"
                    "<ContactPersonPrimary>"
                        "<ContactPerson>" WMSSERVICE_CONTACT_PERS_NAME "</ContactPerson>"
                        "<ContactOrganization>" WMSSERVICE_CONTACT_PERS_ORG "</ContactOrganization>"
                    "</ContactPersonPrimary>"
                    "<ContactPosition>" WMSSERVICE_CONTACT_POS "</ContactPosition>"
                    "<ContactAddress>"
                        "<AddressType>" WMSSERVICE_CONTACT_ADDR_TYPE "</AddressType>"
                        "<Address>" WMSSERVICE_CONTACT_ADDR "</Address>"
                        "<City>" WMSSERVICE_CONTACT_ADDR_CITY "</City>"
                        "<StateOrProvince>" WMSSERVICE_CONTACT_ADDR_STATE "</StateOrProvince>"
                        "<PostCode>" WMSSERVICE_CONTACT_ADDR_POSTCODE "</PostCode>"
                        "<Country>" WMSSERVICE_CONTACT_ADDR_COUNTRY "</Country>"
                    "</ContactAddress>"
                    "<ContactVoiceTelephone>" WMSSERVICE_CONTACT_VOICE_TEL "</ContactVoiceTelephone>"
                    "<ContactElectronicMailAddress>" WMSSERVICE_CONTACT_EMAIL "</ContactElectronicMailAddress>"
                "</ContactInformation>"
                "<Fees>" WMSSERVICE_FEES "</Fees>"
                "<AccessConstraints>" WMSSERVICE_ACCESS_CONST "</AccessConstraints>"
                "<LayerLimit>" STRINGIFY(WMSSERVICE_LAYER_LIMIT) "</LayerLimit>"
                "<MaxWidth>" STRINGIFY(WMSSERVICE_MAX_WIDTH) "</MaxWidth>"
                "<MaxHeight>" STRINGIFY(WMSSERVICE_MAX_HEIGHT) "</MaxHeight>"
            "</Service>"
            "<Capability>"
                "<Request>"
                    "<GetCapabilities>"
                        "<Format>" WMSCAPABILITY_FORMAT_XML "</Format>"
                        "<DCPType>"
                            "<HTTP>"
                                "<Get>"
                                    "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSCAPABILITY_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                                "</Get>"
                                "<Post>"
                                    "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSCAPABILITY_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                                "</Post>"
                            "</HTTP>"
                        "</DCPType>"
                    "</GetCapabilities>"
                    "<GetMap>"
                        "<Format>" WMSCAPABILITY_FORMAT_GIF "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_PNG "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_JPEG "</Format>"
                        "<DCPType>"
                            "<HTTP>"
                                "<Get>"
                                    "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSCAPABILITY_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                                "</Get>"
                            "</HTTP>"
                        "</DCPType>"
                    "</GetMap>"
                    "<GetFeatureInfo>"
                        "<Format>" WMSCAPABILITY_FORMAT_XML "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_PLAIN "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_HTML "</Format>"
                        "<DCPType>"
                            "<HTTP>"
                                "<Get>"
                                    "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSCAPABILITY_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                                "</Get>"
                            "</HTTP>"
                        "</DCPType>"
                    "</GetFeatureInfo>"
                "</Request>"
                "<Exception>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_XML "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_INIMAGE "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_BLANK "</Format>"
                "</Exception>"
                "<Layer>"
                    "<Title>" WMSLAYER0_TITLE "</Title>"
                    "<CRS>" WMSLAYER0_CRS "</CRS>"
                    "<AuthorityURL name='" WMSLAYER0_AUTH_NAME "'>"
                        "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER0_AUTH_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                    "</AuthorityURL>"
                    "<Layer>"
                        "<Name>" WMSLAYER00_NAME "</Name>"
                        "<Title>" WMSLAYER00_TITLE "</Title>"
                        "<CRS>" WMSLAYER00_CRS "</CRS>"
                        "<EX_GeographicBoundingBox>"
                            "<westBoundLongitude>" STRINGIFY(WMSLAYER00_GEOBBOX_W) "</westBoundLongitude>"
                            "<eastBoundLongitude>" STRINGIFY(WMSLAYER00_GEOBBOX_E) "</eastBoundLongitude>"
                            "<southBoundLatitude>" STRINGIFY(WMSLAYER00_GEOBBOX_S) "</southBoundLatitude>"
                            "<northBoundLatitude>" STRINGIFY(WMSLAYER00_GEOBBOX_N) "</northBoundLatitude>"
                        "</EX_GeographicBoundingBox>"
                        "<BoundingBox resy='" STRINGIFY(WMSLAYER00_BBOX0_RES_Y) "' resx='" STRINGIFY(WMSLAYER00_BBOX0_RES_X) "' maxy='" STRINGIFY(WMSLAYER00_BBOX0_MAX_Y) "' maxx='" STRINGIFY(WMSLAYER00_BBOX0_MAX_X) "' miny='" STRINGIFY(WMSLAYER00_BBOX0_MIN_Y) "' minx='" STRINGIFY(WMSLAYER00_BBOX0_MIN_X) "' CRS='" WMSLAYER00_BBOX0_CRS "'/>"
                        "<BoundingBox resy='" STRINGIFY(WMSLAYER00_BBOX1_RES_Y) "' resx='" STRINGIFY(WMSLAYER00_BBOX1_RES_X) "' maxy='" STRINGIFY(WMSLAYER00_BBOX1_MAX_Y) "' maxx='" STRINGIFY(WMSLAYER00_BBOX1_MAX_X) "' miny='" STRINGIFY(WMSLAYER00_BBOX1_MIN_Y) "' minx='" STRINGIFY(WMSLAYER00_BBOX1_MIN_X) "' CRS='" WMSLAYER00_BBOX1_CRS "'/>"
                        "<Attribution>"
                            "<Title>" WMSLAYER00_ATTR_TITLE "</Title>"
                            "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER00_ATTR_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                            "<LogoURL height='" STRINGIFY(WMSLAYER00_LOGO_HEIGHT) "' width='" STRINGIFY(WMSLAYER00_LOGO_WIDTH) "'>"
                                "<Format>"WMSCAPABILITY_FORMAT_GIF"</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER00_LOGO_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                            "</LogoURL>"
                        "</Attribution>"
                        "<Identifier authority='" WMSLAYER00_ID_AUTH "'>" WMSLAYER00_ID "</Identifier>"
                        "<FeatureListURL>"
                            "<Format>" WMSLAYER00_FTLIST_FORMAT "</Format>"
                            "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER00_FTLIST_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                        "</FeatureListURL>"
                        "<Style>"
                            "<Name>" WMSLAYER00_STYLE_NAME "</Name>"
                            "<Title>" WMSLAYER00_STYLE_TITLE "</Title>"
                            "<Abstract>" WMSLAYER00_STYLE_ABSTRACT "</Abstract>"
                            "<LegendURL height='" STRINGIFY(WMSLAYER00_STYLE_LEG_HEIGHT) "' width='" STRINGIFY(WMSLAYER00_STYLE_LEG_WIDTH) "'>"
                                "<Format>" WMSCAPABILITY_FORMAT_GIF "</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER00_STYLE_LEG_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                            "</LegendURL>"
                            "<StyleSheetURL>"
                                "<Format>" WMSCAPABILITY_FORMAT_XSL "</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER00_STYLE_SHEET_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "'/>"
                            "</StyleSheetURL>"
                        "</Style>"
                        "<Layer queryable='" STRINGIFY(WMSLAYER000_QUERY) "'>"
                            "<Name>" WMSLAYER000_NAME "</Name>"
                            "<Title>" WMSLAYER000_TITLE "</Title>"
                            "<Abstract>" WMSLAYER000_ABSTRACT "</Abstract>"
                            "<KeywordList>"
                                "<Keyword>"WMSLAYER000_KEYWORD0"</Keyword>"
                                "<Keyword>"WMSLAYER000_KEYWORD1"</Keyword>"
                                "<Keyword>"WMSLAYER000_KEYWORD2"</Keyword>"
                            "</KeywordList>"
                            "<Identifier authority='" WMSLAYER000_ID_AUTH "'>" WMSLAYER000_ID "</Identifier>"
                            "<MetadataURL type='" WMSLAYER000_METADATA0_TYPE "'>"
                                "<Format>"WMSCAPABILITY_FORMAT_PLAIN"</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER000_METADATA0_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "' />"
                            "</MetadataURL>"
                            "<MetadataURL type='" WMSLAYER000_METADATA1_TYPE "'>"
                                "<Format>"WMSCAPABILITY_FORMAT_XML"</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER000_METADATA1_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "' />"
                            "</MetadataURL>"
                            "<Style>"
                                "<Name>"WMSLAYER000_STYLE_NAME"</Name>"
                                "<Title>"WMSLAYER000_STYLE_TITLE"</Title>"
                                "<Abstract>"WMSLAYER000_STYLE_ABSTRACT"</Abstract>"
                                "<LegendURL height='" STRINGIFY(WMSLAYER000_STYLE_LEG_HEIGHT) "' width='" STRINGIFY(WMSLAYER000_STYLE_LEG_WIDTH) "'>"
                                "<Format>"WMSCAPABILITY_FORMAT_GIF"</Format>"
                                "<OnlineResource xmlns:xlink='http://www.w3.org/1999/xlink' xlink:href='" WMSLAYER000_STYLE_LEG_ONLINERES_HREF "' xlink:type='" WMSCAPABILITY_ONLINERES_TYPE "' />"
                                "</LegendURL>"
                            "</Style>"
                        "</Layer>"
                        "<Layer queryable='" STRINGIFY(WMSLAYER001_QUERY) "'>"
                            "<Name>" WMSLAYER001_NAME "</Name>"
                            "<Title>" WMSLAYER001_TITLE "</Title>"
                            "<Abstract>" WMSLAYER001_ABSTRACT "</Abstract>"
                            "<KeywordList>"
                                "<Keyword>" WMSLAYER001_KEYWORD0 "</Keyword>"
                                "<Keyword>" WMSLAYER001_KEYWORD1 "</Keyword>"
                                "<Keyword>" WMSLAYER001_KEYWORD2 "</Keyword>"
                            "</KeywordList>"
                        "</Layer>"
                    "</Layer>"
                    "<Layer queryable='" STRINGIFY(WMSLAYER01_QUERY) "'>"
                        "<Title>"WMSLAYER01_TITLE"</Title>"
                        "<CRS>"WMSLAYER01_CRS"</CRS>"
                        "<EX_GeographicBoundingBox>"
                            "<westBoundLongitude>" STRINGIFY(WMSLAYER01_GEOBBOX_W) "</westBoundLongitude>"
                            "<eastBoundLongitude>" STRINGIFY(WMSLAYER01_GEOBBOX_E) "</eastBoundLongitude>"
                            "<southBoundLatitude>" STRINGIFY(WMSLAYER01_GEOBBOX_S) "</southBoundLatitude>"
                            "<northBoundLatitude>" STRINGIFY(WMSLAYER01_GEOBBOX_N) "</northBoundLatitude>"
                        "</EX_GeographicBoundingBox>"
                        "<Dimension name='" WMSLAYER01_DIM_NAME "' default='" WMSLAYER01_DIM_DEFAULT "' units='" WMSLAYER01_DIM_UNITS "'>"WMSLAYER01_DIM"</Dimension>"
                        "<Layer>"
                            "<Name>"WMSLAYER010_NAME"</Name>"
                            "<Title>"WMSLAYER010_TITLE"</Title>"
                        "</Layer>"
                        "<Layer>"
                            "<Name>"WMSLAYER011_NAME"</Name>"
                            "<Title>"WMSLAYER011_TITLE"</Title>"
                        "</Layer>"
                        "<Layer>"
                            "<Name>"WMSLAYER012_NAME"</Name>"
                            "<Title>"WMSLAYER012_TITLE"</Title>"
                            "<Dimension name='"WMSLAYER012_DIM0_NAME"' units='"WMSLAYER012_DIM0_UNITS"' />"
                            "<Dimension name='"WMSLAYER012_DIM1_NAME"' default='"WMSLAYER012_DIM1_DEFAULT"' units='"WMSLAYER012_DIM1_UNITS"'>"WMSLAYER012_DIM1"</Dimension>"
                            "<Dimension name='"WMSLAYER012_DIM2_NAME"' default='"WMSLAYER012_DIM2_DEFAULT"' units='"WMSLAYER012_DIM2_UNITS"' nearestValue='"WMSLAYER012_DIM2_NEAREST_VALUE"'>"WMSLAYER012_DIM2"</Dimension>"
                        "</Layer>"
                    "</Layer>"
                    "<Layer fixedHeight='" STRINGIFY(WMSLAYER02_FIXEDHEIGHT) "' fixedWidth='" STRINGIFY(WMSLAYER02_FIXEDWIDTH) "' noSubsets='" STRINGIFY(WMSLAYER02_NOSUBSETS) "' opaque='" STRINGIFY(WMSLAYER02_OPAQUE) "'>"
                        "<Name>"WMSLAYER02_NAME"</Name>"
                        "<Title>"WMSLAYER02_TITLE"</Title>"
                        "<EX_GeographicBoundingBox>"
                            "<westBoundLongitude>" STRINGIFY(WMSLAYER02_GEOBBOX_W) "</westBoundLongitude>"
                            "<eastBoundLongitude>" STRINGIFY(WMSLAYER02_GEOBBOX_E) "</eastBoundLongitude>"
                            "<southBoundLatitude>" STRINGIFY(WMSLAYER02_GEOBBOX_S) "</southBoundLatitude>"
                            "<northBoundLatitude>" STRINGIFY(WMSLAYER02_GEOBBOX_N) "</northBoundLatitude>"
                        "</EX_GeographicBoundingBox>"
                        "<Dimension name='"WMSLAYER02_DIM_NAME"' default='"WMSLAYER02_DIM_DEFAULT"' units='"WMSLAYER02_DIM_UNITS"'>"WMSLAYER02_DIM"</Dimension>"
                    "</Layer>"
                    "<Layer cascaded='" STRINGIFY(WMSLAYER03_CASCADED) "'>"
                        "<Name>"WMSLAYER03_NAME"</Name>"
                        "<Title>"WMSLAYER03_TITLE"</Title>"
                        "<EX_GeographicBoundingBox>"
                            "<westBoundLongitude>" STRINGIFY(WMSLAYER03_GEOBBOX_W) "</westBoundLongitude>"
                            "<eastBoundLongitude>" STRINGIFY(WMSLAYER03_GEOBBOX_E) "</eastBoundLongitude>"
                            "<southBoundLatitude>" STRINGIFY(WMSLAYER03_GEOBBOX_S) "</southBoundLatitude>"
                            "<northBoundLatitude>" STRINGIFY(WMSLAYER03_GEOBBOX_N) "</northBoundLatitude>"
                        "</EX_GeographicBoundingBox>"
                        "<Dimension name='"WMSLAYER03_DIM_NAME"' default='"WMSLAYER03_DIM_DEFAULT"' units='"WMSLAYER03_DIM_UNITS"'>"WMSLAYER03_DIM"</Dimension>"
                    "</Layer>"
                "</Layer>"
            "</Capability>"
        "</WMS_Capabilities>";

    WString parseError;
    WMSParserStatus status = WMSParserStatus::Success;
    WMSCapabilitiesPtr pCapabilities = WMSCapabilities::CreateAndReadFromMemory(status, capabilitiesString.c_str(), capabilitiesString.length(), &parseError);

    ASSERT_STREQ(L"", parseError.c_str());
    //ASSERT_EQ(WMSParserStatus::Success, status);

    // Capabilities.
    ASSERT_TRUE(pCapabilities.IsValid());
    ASSERT_STREQ(WMSCAPABILITIES_VERSION, pCapabilities->GetVersion().c_str());

    // Service Group.
    WMSServiceCP pService = pCapabilities->GetServiceGroup();
    ASSERT_STREQ(WMSSERVICE_NAME, pService->GetName().c_str());
    ASSERT_STREQ(WMSSERVICE_TITLE, pService->GetTitle().c_str());
    ASSERT_STREQ(WMSSERVICE_ABSTRACT, pService->GetAbstract().c_str());
    ASSERT_STREQ(WMSSERVICE_FEES, pService->GetFees().c_str());
    ASSERT_STREQ(WMSSERVICE_ACCESS_CONST, pService->GetAccessConstraints().c_str());
    ASSERT_STREQ(WMSSERVICE_KEYWORD_0, pService->GetKeywordList()[0].c_str());
    ASSERT_STREQ(WMSSERVICE_KEYWORD_1, pService->GetKeywordList()[1].c_str());
    ASSERT_STREQ(WMSSERVICE_KEYWORD_2, pService->GetKeywordList()[2].c_str());
    ASSERT_STREQ(WMSSERVICE_ONLINERES_HREF, pService->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSSERVICE_ONLINERES_TYPE, pService->GetOnlineResource()->GetType().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_PERS_NAME, pService->GetContactInformation()->GetPerson()->GetName().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_PERS_ORG, pService->GetContactInformation()->GetPerson()->GetOrganization().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_POS, pService->GetContactInformation()->GetPosition().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_ADDR, pService->GetContactInformation()->GetAddress()->GetAddress().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_ADDR_TYPE, pService->GetContactInformation()->GetAddress()->GetType().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_ADDR_CITY, pService->GetContactInformation()->GetAddress()->GetCity().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_ADDR_POSTCODE, pService->GetContactInformation()->GetAddress()->GetPostCode().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_ADDR_COUNTRY, pService->GetContactInformation()->GetAddress()->GetCountry().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_VOICE_TEL, pService->GetContactInformation()->GetVoiceTelephone().c_str());
    ASSERT_STREQ(WMSSERVICE_CONTACT_EMAIL, pService->GetContactInformation()->GetEmailAddress().c_str());
    ASSERT_EQ(WMSSERVICE_LAYER_LIMIT, pService->GetLayerLimit());
    ASSERT_EQ(WMSSERVICE_MAX_WIDTH, pService->GetMaxWidth());
    ASSERT_EQ(WMSSERVICE_MAX_HEIGHT, pService->GetMaxHeight());

    // Capability Group.
    WMSCapabilityCP pCapability = pCapabilities->GetCapabilityGroup();
    // Request.
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_XML, pCapability->GetRequest()->GetCapabilities()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_TYPE, pCapability->GetRequest()->GetCapabilities()->GetDcpType()->GetHttpGet()->GetType().c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_HREF, pCapability->GetRequest()->GetCapabilities()->GetDcpType()->GetHttpGet()->GetHref().c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_TYPE, pCapability->GetRequest()->GetCapabilities()->GetDcpType()->GetHttpPost()->GetType().c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_HREF, pCapability->GetRequest()->GetCapabilities()->GetDcpType()->GetHttpPost()->GetHref().c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_GIF, pCapability->GetRequest()->GetMap()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PNG, pCapability->GetRequest()->GetMap()->GetFormatList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_JPEG, pCapability->GetRequest()->GetMap()->GetFormatList()[2].c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_TYPE, pCapability->GetRequest()->GetMap()->GetDcpType()->GetHttpGet()->GetType().c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_HREF, pCapability->GetRequest()->GetMap()->GetDcpType()->GetHttpGet()->GetHref().c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_XML, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PLAIN, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_HTML, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[2].c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_TYPE, pCapability->GetRequest()->GetFeatureInfo()->GetDcpType()->GetHttpGet()->GetType().c_str());
    ASSERT_STREQ(WMSCAPABILITY_ONLINERES_HREF, pCapability->GetRequest()->GetFeatureInfo()->GetDcpType()->GetHttpGet()->GetHref().c_str());
    // Exception.
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_XML, pCapability->GetExceptionList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_INIMAGE, pCapability->GetExceptionList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_BLANK, pCapability->GetExceptionList()[2].c_str());
    // Layer.
    WMSLayerPtr pLayer = pCapability->GetLayerList()[0];
    ASSERT_STREQ(WMSLAYER0_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER0_CRS, pLayer->GetCRSList()[0].c_str());
    ASSERT_STREQ(WMSLAYER0_AUTH_NAME, pLayer->GetAuthorityUrl()->GetName().c_str());
    ASSERT_STREQ(WMSLAYER0_AUTH_ONLINERES_HREF, pLayer->GetAuthorityUrl()->GetOnlineResource()->GetHref().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[0];
    ASSERT_STREQ(WMSLAYER00_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER00_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER00_CRS, pLayer->GetCRSList()[0].c_str());
    ASSERT_EQ(WMSLAYER00_GEOBBOX_W, pLayer->GetGeoBBox()->GetWestBoundLong());
    ASSERT_EQ(WMSLAYER00_GEOBBOX_E, pLayer->GetGeoBBox()->GetEastBoundLong());
    ASSERT_EQ(WMSLAYER00_GEOBBOX_S, pLayer->GetGeoBBox()->GetSouthBoundLat());
    ASSERT_EQ(WMSLAYER00_GEOBBOX_N, pLayer->GetGeoBBox()->GetNorthBoundLat());
    ASSERT_EQ(WMSLAYER00_BBOX0_RES_X, pLayer->GetBBox()[0]->GetResX());
    ASSERT_EQ(WMSLAYER00_BBOX0_RES_Y, pLayer->GetBBox()[0]->GetResY());
    ASSERT_EQ(WMSLAYER00_BBOX0_MIN_X, pLayer->GetBBox()[0]->GetMinX());
    ASSERT_EQ(WMSLAYER00_BBOX0_MIN_Y, pLayer->GetBBox()[0]->GetMinY());
    ASSERT_EQ(WMSLAYER00_BBOX0_MAX_X, pLayer->GetBBox()[0]->GetMaxX());
    ASSERT_EQ(WMSLAYER00_BBOX0_MAX_Y, pLayer->GetBBox()[0]->GetMaxY());
    ASSERT_STREQ(WMSLAYER00_BBOX0_CRS, pLayer->GetBBox()[0]->GetCRS().c_str());
    ASSERT_EQ(WMSLAYER00_BBOX1_RES_X, pLayer->GetBBox()[1]->GetResX());
    ASSERT_EQ(WMSLAYER00_BBOX1_RES_Y, pLayer->GetBBox()[1]->GetResY());
    ASSERT_EQ(WMSLAYER00_BBOX1_MIN_X, pLayer->GetBBox()[1]->GetMinX());
    ASSERT_EQ(WMSLAYER00_BBOX1_MIN_Y, pLayer->GetBBox()[1]->GetMinY());
    ASSERT_EQ(WMSLAYER00_BBOX1_MAX_X, pLayer->GetBBox()[1]->GetMaxX());
    ASSERT_EQ(WMSLAYER00_BBOX1_MAX_Y, pLayer->GetBBox()[1]->GetMaxY());
    ASSERT_STREQ(WMSLAYER00_BBOX1_CRS, pLayer->GetBBox()[1]->GetCRS().c_str());
    ASSERT_STREQ(WMSLAYER00_ATTR_TITLE, pLayer->GetAttribution()->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER00_ATTR_ONLINERES_HREF, pLayer->GetAttribution()->GetOnlineResource()->GetHref().c_str());
    ASSERT_EQ(WMSLAYER00_LOGO_HEIGHT, pLayer->GetAttribution()->GetLogoUrl()->GetHeight());
    ASSERT_EQ(WMSLAYER00_LOGO_WIDTH, pLayer->GetAttribution()->GetLogoUrl()->GetWidth());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_GIF, pLayer->GetAttribution()->GetLogoUrl()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSLAYER00_LOGO_ONLINERES_HREF, pLayer->GetAttribution()->GetLogoUrl()->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSLAYER00_ID_AUTH, pLayer->GetIdentifier()->GetAuthority().c_str());
    //&&JFC:    It seems like BeXml is not able to retrieve all the information when a node has some attributes plus a node value. 
    //          Example: <Identifier authority="DIF_ID">123456</Identifier>
    //          The node value (in the example above: 123456) is always missing. To investigate ...
    //ASSERT_STREQ(WMSLAYER00_ID, pLayer->GetIdentifier()->GetID().c_str());
    ASSERT_STREQ(WMSLAYER00_FTLIST_FORMAT, pLayer->GetFeatureListUrl()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSLAYER00_FTLIST_ONLINERES_HREF, pLayer->GetFeatureListUrl()->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSLAYER00_STYLE_NAME, pLayer->GetStyle()->GetName().c_str());
    ASSERT_STREQ(WMSLAYER00_STYLE_TITLE, pLayer->GetStyle()->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER00_STYLE_ABSTRACT, pLayer->GetStyle()->GetAbstract().c_str());
    ASSERT_EQ(WMSLAYER00_STYLE_LEG_HEIGHT, pLayer->GetStyle()->GetLegendUrl()->GetHeight());
    ASSERT_EQ(WMSLAYER00_STYLE_LEG_WIDTH, pLayer->GetStyle()->GetLegendUrl()->GetWidth());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_GIF, pLayer->GetStyle()->GetLegendUrl()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSLAYER00_STYLE_LEG_ONLINERES_HREF, pLayer->GetStyle()->GetLegendUrl()->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_XSL, pLayer->GetStyle()->GetStyleSheetUrl()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSLAYER00_STYLE_SHEET_ONLINERES_HREF, pLayer->GetStyle()->GetStyleSheetUrl()->GetOnlineResource()->GetHref().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[0]->GetLayerList()[0];
    ASSERT_TRUE(pLayer->IsQueryable());
    ASSERT_STREQ(WMSLAYER000_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER000_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER000_ABSTRACT, pLayer->GetAbstract().c_str());
    ASSERT_STREQ(WMSLAYER000_KEYWORD0, pLayer->GetKeywordList()[0].c_str());
    ASSERT_STREQ(WMSLAYER000_KEYWORD1, pLayer->GetKeywordList()[1].c_str());
    ASSERT_STREQ(WMSLAYER000_KEYWORD2, pLayer->GetKeywordList()[2].c_str());
    ASSERT_STREQ(WMSLAYER000_ID_AUTH, pLayer->GetIdentifier()->GetAuthority().c_str());
    //ASSERT_STREQ(WMSLAYER000_ID, pLayer->GetIdentifier()->GetID().c_str());
    ASSERT_STREQ(WMSLAYER000_METADATA0_TYPE, pLayer->GetMetadataUrlList()[0]->GetType().c_str());
    ASSERT_STREQ(WMSLAYER000_METADATA0_ONLINERES_HREF, pLayer->GetMetadataUrlList()[0]->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSLAYER000_METADATA1_TYPE, pLayer->GetMetadataUrlList()[1]->GetType().c_str());
    ASSERT_STREQ(WMSLAYER000_METADATA1_ONLINERES_HREF, pLayer->GetMetadataUrlList()[1]->GetOnlineResource()->GetHref().c_str());
    ASSERT_STREQ(WMSLAYER000_STYLE_NAME, pLayer->GetStyle()->GetName().c_str());
    ASSERT_STREQ(WMSLAYER000_STYLE_TITLE, pLayer->GetStyle()->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER000_STYLE_ABSTRACT, pLayer->GetStyle()->GetAbstract().c_str());
    ASSERT_EQ(WMSLAYER000_STYLE_LEG_HEIGHT, pLayer->GetStyle()->GetLegendUrl()->GetHeight());
    ASSERT_EQ(WMSLAYER000_STYLE_LEG_WIDTH, pLayer->GetStyle()->GetLegendUrl()->GetWidth());
    ASSERT_STREQ(WMSLAYER000_STYLE_LEG_ONLINERES_HREF, pLayer->GetStyle()->GetLegendUrl()->GetOnlineResource()->GetHref().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[0]->GetLayerList()[1];
    ASSERT_TRUE(pLayer->IsQueryable());
    ASSERT_STREQ(WMSLAYER001_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER001_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER001_ABSTRACT, pLayer->GetAbstract().c_str());
    ASSERT_STREQ(WMSLAYER001_KEYWORD0, pLayer->GetKeywordList()[0].c_str());
    ASSERT_STREQ(WMSLAYER001_KEYWORD1, pLayer->GetKeywordList()[1].c_str());
    ASSERT_STREQ(WMSLAYER001_KEYWORD2, pLayer->GetKeywordList()[2].c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[1];
    ASSERT_TRUE(pLayer->IsQueryable());
    ASSERT_STREQ(WMSLAYER01_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER01_CRS, pLayer->GetCRSList()[0].c_str());
    ASSERT_EQ(WMSLAYER01_GEOBBOX_W, pLayer->GetGeoBBox()->GetWestBoundLong());
    ASSERT_EQ(WMSLAYER01_GEOBBOX_E, pLayer->GetGeoBBox()->GetEastBoundLong());
    ASSERT_EQ(WMSLAYER01_GEOBBOX_S, pLayer->GetGeoBBox()->GetSouthBoundLat());
    ASSERT_EQ(WMSLAYER01_GEOBBOX_N, pLayer->GetGeoBBox()->GetNorthBoundLat());
    ASSERT_STREQ(WMSLAYER01_DIM_NAME, pLayer->GetDimensionList()[0]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER01_DIM_DEFAULT, pLayer->GetDimensionList()[0]->GetDefault().c_str());
    ASSERT_STREQ(WMSLAYER01_DIM_UNITS, pLayer->GetDimensionList()[0]->GetUnits().c_str());
    //ASSERT_STREQ(WMSLAYER01_DIM, pLayer->GetDimensionList()[0]->GetDimension().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[1]->GetLayerList()[0];
    ASSERT_STREQ(WMSLAYER010_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER010_TITLE, pLayer->GetTitle().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[1]->GetLayerList()[1];
    ASSERT_STREQ(WMSLAYER011_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER011_TITLE, pLayer->GetTitle().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[1]->GetLayerList()[2];
    ASSERT_STREQ(WMSLAYER012_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER012_TITLE, pLayer->GetTitle().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM0_NAME, pLayer->GetDimensionList()[0]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM0_UNITS, pLayer->GetDimensionList()[0]->GetUnits().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM1_NAME, pLayer->GetDimensionList()[1]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM1_DEFAULT, pLayer->GetDimensionList()[1]->GetDefault().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM1_UNITS, pLayer->GetDimensionList()[1]->GetUnits().c_str());
    //ASSERT_STREQ(WMSLAYER012_DIM1, pLayer->GetDimensionList()[1]->GetDimension().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM2_NAME, pLayer->GetDimensionList()[2]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM2_DEFAULT, pLayer->GetDimensionList()[2]->GetDefault().c_str());
    ASSERT_STREQ(WMSLAYER012_DIM2_UNITS, pLayer->GetDimensionList()[2]->GetUnits().c_str());
    ASSERT_TRUE(pLayer->GetDimensionList()[2]->GetNearestValue());
    //ASSERT_STREQ(WMSLAYER012_DIM2, pLayer->GetDimensionList()[2]->GetDimension().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[2];
    ASSERT_EQ(WMSLAYER02_FIXEDHEIGHT, pLayer->GetFixedHeight());
    ASSERT_EQ(WMSLAYER02_FIXEDWIDTH, pLayer->GetFixedWidth());
    ASSERT_TRUE(pLayer->HasSubsets());
    ASSERT_TRUE(pLayer->IsOpaque());
    ASSERT_STREQ(WMSLAYER02_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER02_TITLE, pLayer->GetTitle().c_str());
    ASSERT_EQ(WMSLAYER02_GEOBBOX_W, pLayer->GetGeoBBox()->GetWestBoundLong());
    ASSERT_EQ(WMSLAYER02_GEOBBOX_E, pLayer->GetGeoBBox()->GetEastBoundLong());
    ASSERT_EQ(WMSLAYER02_GEOBBOX_S, pLayer->GetGeoBBox()->GetSouthBoundLat());
    ASSERT_EQ(WMSLAYER02_GEOBBOX_N, pLayer->GetGeoBBox()->GetNorthBoundLat());
    ASSERT_STREQ(WMSLAYER02_DIM_NAME, pLayer->GetDimensionList()[0]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER02_DIM_DEFAULT, pLayer->GetDimensionList()[0]->GetDefault().c_str());
    ASSERT_STREQ(WMSLAYER02_DIM_UNITS, pLayer->GetDimensionList()[0]->GetUnits().c_str());
    //ASSERT_STREQ(WMSLAYER02_DIM, pLayer->GetDimensionList()[0]->GetDimension().c_str());
    pLayer = pCapability->GetLayerList()[0]->GetLayerList()[3];
    ASSERT_EQ(WMSLAYER03_CASCADED, pLayer->GetCascaded());
    ASSERT_STREQ(WMSLAYER03_NAME, pLayer->GetName().c_str());
    ASSERT_STREQ(WMSLAYER03_TITLE, pLayer->GetTitle().c_str());
    ASSERT_EQ(WMSLAYER03_GEOBBOX_W, pLayer->GetGeoBBox()->GetWestBoundLong());
    ASSERT_EQ(WMSLAYER03_GEOBBOX_E, pLayer->GetGeoBBox()->GetEastBoundLong());
    ASSERT_EQ(WMSLAYER03_GEOBBOX_S, pLayer->GetGeoBBox()->GetSouthBoundLat());
    ASSERT_EQ(WMSLAYER03_GEOBBOX_N, pLayer->GetGeoBBox()->GetNorthBoundLat());
    ASSERT_STREQ(WMSLAYER03_DIM_NAME, pLayer->GetDimensionList()[0]->GetName().c_str());
    ASSERT_STREQ(WMSLAYER03_DIM_DEFAULT, pLayer->GetDimensionList()[0]->GetDefault().c_str());
    ASSERT_STREQ(WMSLAYER03_DIM_UNITS, pLayer->GetDimensionList()[0]->GetUnits().c_str());
    //ASSERT_STREQ(WMSLAYER03_DIM, pLayer->GetDimensionList()[0]->GetDimension().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
TEST_F(PlatformTestFixture, ReadMultiElementList)
    {
    #define WMSCAPABILITIES_VERSION "1.3.0"

    #define WMSSERVICE_KEYWORD_0 "bird"
    #define WMSSERVICE_KEYWORD_1 "roadrunner"
    #define WMSSERVICE_KEYWORD_2 "ambush"

    #define WMSCAPABILITY_FORMAT_XML "text/xml"
    #define WMSCAPABILITY_FORMAT_PLAIN "text/plain"
    #define WMSCAPABILITY_FORMAT_HTML "text/html"
    #define WMSCAPABILITY_FORMAT_GIF "image/gif"
    #define WMSCAPABILITY_FORMAT_PNG "image/png"
    #define WMSCAPABILITY_FORMAT_JPEG "image/jpeg"

    #define WMSCAPABILITY_EXCEPTION_XML "XML"
    #define WMSCAPABILITY_EXCEPTION_INIMAGE "INIMAGE"
    #define WMSCAPABILITY_EXCEPTION_BLANK "BLANK"

    #define WMSLAYER_CRS_0 "CRS:84"
    #define WMSLAYER_CRS_1 "EPSG:26986"
    #define WMSLAYER_CRS_2 "CRS:88"

    ASSERT_EQ(1/*expected*/, 1/*actual*/);

    Utf8String capabilitiesString =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<WMS_Capabilities xsi:schemaLocation='http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.opengis.net/wms' version='1.3.0'>"
            "<Service>"
                "<KeywordList>"
                    "<Keyword>" WMSSERVICE_KEYWORD_0 "</Keyword>"
                    "<Keyword>" WMSSERVICE_KEYWORD_1 "</Keyword>"
                    "<Keyword>" WMSSERVICE_KEYWORD_2 "</Keyword>"
                "</KeywordList>"
            "</Service>"
            "<Capability>"
                "<Request>"
                    "<GetCapabilities>"
                        "<Format>" WMSCAPABILITY_FORMAT_XML "</Format>"
                    "</GetCapabilities>"
                    "<GetMap>"
                        "<Format>" WMSCAPABILITY_FORMAT_GIF "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_PNG "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_JPEG "</Format>"
                    "</GetMap>"
                    "<GetFeatureInfo>"
                        "<Format>" WMSCAPABILITY_FORMAT_PLAIN "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_HTML "</Format>"
                    "</GetFeatureInfo>"
                "</Request>"
                "<Exception>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_XML "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_INIMAGE "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_BLANK "</Format>"
                "</Exception>"
                "<Layer>"
                    "<CRS>" WMSLAYER_CRS_0 "</CRS>"
                    "<CRS>" WMSLAYER_CRS_1 "</CRS>"
                    "<CRS>" WMSLAYER_CRS_2 "</CRS>"
                "</Layer>"
            "</Capability>"
        "</WMS_Capabilities>";

    WString parseError;
    WMSParserStatus status = WMSParserStatus::Success;
    WMSCapabilitiesPtr pCapabilities = WMSCapabilities::CreateAndReadFromMemory(status, capabilitiesString.c_str(), capabilitiesString.length(), &parseError);

    ASSERT_STREQ(L"", parseError.c_str());
    //ASSERT_EQ(WMSParserStatus::Success, status);

    // Capabilities.
    ASSERT_TRUE(pCapabilities.IsValid());
    ASSERT_STREQ(WMSCAPABILITIES_VERSION, pCapabilities->GetVersion().c_str());

    // Service Group.
    WMSServiceCP pService = pCapabilities->GetServiceGroup();
    ASSERT_STREQ(WMSSERVICE_KEYWORD_0, pService->GetKeywordList()[0].c_str());
    ASSERT_STREQ(WMSSERVICE_KEYWORD_1, pService->GetKeywordList()[1].c_str());
    ASSERT_STREQ(WMSSERVICE_KEYWORD_2, pService->GetKeywordList()[2].c_str());

    // Capability Group.
    WMSCapabilityCP pCapability = pCapabilities->GetCapabilityGroup();
    // Request.
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_XML, pCapability->GetRequest()->GetCapabilities()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_GIF, pCapability->GetRequest()->GetMap()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PNG, pCapability->GetRequest()->GetMap()->GetFormatList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_JPEG, pCapability->GetRequest()->GetMap()->GetFormatList()[2].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PLAIN, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_HTML, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[1].c_str());
    // Exception.
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_XML, pCapability->GetExceptionList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_INIMAGE, pCapability->GetExceptionList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_BLANK, pCapability->GetExceptionList()[2].c_str());
    // Layer.
    WMSLayerPtr pLayer = pCapability->GetLayerList()[0];
    ASSERT_STREQ(WMSLAYER_CRS_0, pLayer->GetCRSList()[0].c_str());
    ASSERT_STREQ(WMSLAYER_CRS_1, pLayer->GetCRSList()[1].c_str());
    ASSERT_STREQ(WMSLAYER_CRS_2, pLayer->GetCRSList()[2].c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
TEST_F(PlatformTestFixture, ReadEmptyObject)
    {
    #define WMSCAPABILITIES_VERSION "1.3.0"

    #define WMSCAPABILITY_FORMAT_XML "text/xml"
    #define WMSCAPABILITY_FORMAT_PLAIN "text/plain"
    #define WMSCAPABILITY_FORMAT_HTML "text/html"
    #define WMSCAPABILITY_FORMAT_GIF "image/gif"
    #define WMSCAPABILITY_FORMAT_PNG "image/png"
    #define WMSCAPABILITY_FORMAT_JPEG "image/jpeg"

    #define WMSCAPABILITY_EXCEPTION_XML "XML"
    #define WMSCAPABILITY_EXCEPTION_INIMAGE "INIMAGE"
    #define WMSCAPABILITY_EXCEPTION_BLANK "BLANK"

    ASSERT_EQ(1/*expected*/, 1/*actual*/);

    Utf8String capabilitiesString =
        "<?xml version='1.0' encoding='UTF-8'?>"
        "<WMS_Capabilities xsi:schemaLocation='http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.opengis.net/wms' version='1.3.0'>"
            "<Service>"
            "</Service>"
            "<Capability>"
                "<Request>"
                    "<GetCapabilities>"
                        "<Format>" WMSCAPABILITY_FORMAT_XML "</Format>"
                    "</GetCapabilities>"
                    "<GetMap>"
                        "<Format>" WMSCAPABILITY_FORMAT_GIF "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_PNG "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_JPEG "</Format>"
                    "</GetMap>"
                    "<GetFeatureInfo>"
                        "<Format>" WMSCAPABILITY_FORMAT_PLAIN "</Format>"
                        "<Format>" WMSCAPABILITY_FORMAT_HTML "</Format>"
                    "</GetFeatureInfo>"
                "</Request>"
                "<Exception>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_XML "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_INIMAGE "</Format>"
                    "<Format>" WMSCAPABILITY_EXCEPTION_BLANK "</Format>"
                "</Exception>"
            "</Capability>"
        "</WMS_Capabilities>";

    WString parseError;
    WMSParserStatus status = WMSParserStatus::Success;
    WMSCapabilitiesPtr pCapabilities = WMSCapabilities::CreateAndReadFromMemory(status, capabilitiesString.c_str(), capabilitiesString.length(), &parseError);

    ASSERT_STREQ(L"", parseError.c_str());
    //ASSERT_EQ(WMSParserStatus::Success, status);

    // Capabilities.
    ASSERT_TRUE(pCapabilities.IsValid());
    ASSERT_STREQ(WMSCAPABILITIES_VERSION, pCapabilities->GetVersion().c_str());

    // Service Group.
    WMSServiceCP pService = pCapabilities->GetServiceGroup();
    ASSERT_EQ(0, pService->GetKeywordList().size());

    // Capability Group.
    WMSCapabilityCP pCapability = pCapabilities->GetCapabilityGroup();
    // Request.
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_XML, pCapability->GetRequest()->GetCapabilities()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_GIF, pCapability->GetRequest()->GetMap()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PNG, pCapability->GetRequest()->GetMap()->GetFormatList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_JPEG, pCapability->GetRequest()->GetMap()->GetFormatList()[2].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_PLAIN, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_FORMAT_HTML, pCapability->GetRequest()->GetFeatureInfo()->GetFormatList()[1].c_str());
    // Exception.
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_XML, pCapability->GetExceptionList()[0].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_INIMAGE, pCapability->GetExceptionList()[1].c_str());
    ASSERT_STREQ(WMSCAPABILITY_EXCEPTION_BLANK, pCapability->GetExceptionList()[2].c_str());
    // Layer.
    ASSERT_EQ(0, pCapability->GetLayerList().size());
    }
