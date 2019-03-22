/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealitySerialization.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeXml/BeXml.h>
#include <Geom/GeomApi.h>

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)
#define WIDEN(quote) _WIDEN(quote)
#define _WIDEN(quote) L##quote

// **************************************************
// Naming-Convention:
// Elements are TitleCase. Ex: WmsSource
// Attributes are camelCase. Ex: someAttribute.
#define W3SCHEMA_PREFIX     "xsi"
#define W3SCHEMA_URI        "http://www.w3.org/2001/XMLSchema-instance"

// *** Version management ***
// Minor version upgrade must remain foreword and backward compatible.
// Major version upgrade should occurs when the change would be incompatible with the current implementation. 
// Namespace change occurs only when the major version changes.  
#define PACKAGE_PREFIX              "rdp"
#define PACKAGE_CURRENT_MAJOR_VERSION 2
#define PACKAGE_CURRENT_MINOR_VERSION 1
#define PACKAGE_CURRENT_VERSION       (WIDEN(STRINGIFY(PACKAGE_CURRENT_MAJOR_VERSION)) L"." WIDEN(STRINGIFY(PACKAGE_CURRENT_MINOR_VERSION)))
#define PACKAGE_CURRENT_NAMESPACE     "http://www.bentley.com/RealityDataServer/v" STRINGIFY(PACKAGE_CURRENT_MAJOR_VERSION)
#define PACKAGE_V1_NAMESPACE          "http://www.bentley.com/RealityDataServer/v1"

#define PACKAGE_ELEMENT_Root                "RealityDataPackage"
#define PACKAGE_ATTRIBUTE_Version           "version"

// Package 
#define PACKAGE_ELEMENT_Name                "Name"
#define PACKAGE_ELEMENT_Description         "Description"
#define PACKAGE_ELEMENT_CreationDate        "CreationDate"
#define PACKAGE_ELEMENT_Copyright           "Copyright"
#define PACKAGE_ELEMENT_PackageId           "PackageId"
#define PACKAGE_ELEMENT_BoundingPolygon     "BoundingPolygon"
#define PACKAGE_ELEMENT_Context             "Context"
#define PACKAGE_ELEMENT_PackageOrigin       "PackageOrigin"
#define PACKAGE_ELEMENT_RequestingApplication       "RequestingApplication"
#define PACKAGE_ELEMENT_Origin              "Origin"

// RealityData
#define PACKAGE_ELEMENT_ImageryGroup        "ImageryGroup"
#define PACKAGE_ELEMENT_ImageryData         "ImageryData"
#define PACKAGE_ELEMENT_Corners             "Corners"
#define PACKAGE_ELEMENT_LowerLeft           "LowerLeft"
#define PACKAGE_ELEMENT_LowerRight          "LowerRight"
#define PACKAGE_ELEMENT_UpperLeft           "UpperLeft"
#define PACKAGE_ELEMENT_UpperRight          "UpperRight"

#define PACKAGE_ELEMENT_ModelGroup          "ModelGroup"
#define PACKAGE_ELEMENT_ModelData           "ModelData"

#define PACKAGE_ELEMENT_PinnedGroup         "PinnedGroup"
#define PACKAGE_ELEMENT_PinnedData          "PinnedData"
#define PACKAGE_ELEMENT_Position            "Position"
#define PACKAGE_ELEMENT_Area                "Area"

#define PACKAGE_ELEMENT_TerrainGroup        "TerrainGroup"
#define PACKAGE_ELEMENT_TerrainData         "TerrainData"

#define PACKAGE_ELEMENT_UndefinedGroup      "UndefinedGroup"
#define PACKAGE_ELEMENT_UndefinedData       "UndefinedData"

#define PACKAGE_ELEMENT_Sources             "Sources"
#define PACKAGE_ELEMENT_Dataset             "Dataset"
#define PACKAGE_ELEMENT_Resolution          "Resolution"

// RealityDataSource
#define PACKAGE_ELEMENT_Source              "Source"
#define PACKAGE_SOURCE_ATTRIBUTE_Uri        "uri"
#define PACKAGE_SOURCE_ATTRIBUTE_Type       "type"
#define PACKAGE_SOURCE_ATTRIBUTE_Streamed   "streamed"
// Copyright element already defined for package
#define PACKAGE_ELEMENT_TermOfUse           "TermOfUse"
#define PACKAGE_ELEMENT_Id                  "Id"
#define PACKAGE_ELEMENT_Provider            "Provider"
#define PACKAGE_ELEMENT_ServerLoginKey      "ServerLoginKey"
#define PACKAGE_ELEMENT_ServerLoginMethod   "ServerLoginMethod"
#define PACKAGE_ELEMENT_ServerRegPage       "ServerRegistrationPage"
#define PACKAGE_ELEMENT_ServerOrgPage       "ServerOrganisationPage"
#define PACKAGE_ELEMENT_Visibility          "Visibility"
#define PACKAGE_ELEMENT_Size                "Size"
#define PACKAGE_ELEMENT_Filesize            "Filesize"
#define PACKAGE_ELEMENT_Metadata            "Metadata"
#define PACKAGE_ELEMENT_GeoCS               "GeoCS"
#define PACKAGE_ELEMENT_NoDataValue         "NoDataValue"
#define PACKAGE_ELEMENT_SisterFiles         "SisterFiles"
#define PACKAGE_ELEMENT_File                "File"

// WmsSource
#define PACKAGE_ELEMENT_WmsSource           "WmsSource"
#define WMS_SOURCE_TYPE                     "wms"

// OsmSource
#define PACKAGE_ELEMENT_OsmSource           "OsmSource"
#define OSM_SOURCE_TYPE                     "osm"

// MultiBandSource
#define PACKAGE_ELEMENT_MultiBandSource     "MultiBandSource"
#define PACKAGE_ELEMENT_RedBand             "Red"
#define PACKAGE_ELEMENT_GreenBand           "Green"
#define PACKAGE_ELEMENT_BlueBand            "Blue"
#define PACKAGE_ELEMENT_PanchromaticBand    "Panchromatic"

#define SPACE_DELIMITER    " "
#define SPACE_DELIMITER_U L" "

// lat/long precision of 0.1 millimeter
// g == The precision specifies the maximum number of significant digits printed. Any trailing zeros are truncated. 
// So to get get the precision we want 9 decimal AFTER the point we need 12 digits. xxx.123456789
#define LATLONG_PRINT_FORMAT               L"%.12g %.12g"    //  lat/long precision of 0.1 millimeter.
#define LATLONG_PRINT_FORMAT_COMMA          "%.12g,%.12g"    //  lat/long precision of 0.1 millimeter.

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<typename String_T>
inline double StringToDouble(String_T const* pString)
    {
    // *** If you get an error here, you must implement the specialization for type 'String_T'
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<>
inline double StringToDouble(wchar_t const* pString)
    {
    return BeStringUtilities::Wtof(pString);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<>
inline double StringToDouble(char const* pString)
    {
    return atof(pString);
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
template<typename String_T> 
struct StringTokenizer
    {
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    StringTokenizer(String_T const& source, typename String_T::value_type const* delimeter)
    :m_source(source), m_delimiter(delimeter)
        {
        m_startPos = m_source.find_first_not_of(m_delimiter, 0); // Skip delimiters at beginning.
        m_endPos = m_source.find_first_of(m_delimiter, m_startPos); // Find first non-delimiter.
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    bool Get(double& val)
        {
        if(!HasValue())
            return false;

        String_T valStr = m_source.substr(m_startPos, m_endPos - m_startPos);
        val = StringToDouble(valStr.c_str());

        // Goto next.
        m_startPos = m_source.find_first_not_of(m_delimiter, m_endPos); // Skip delimiters.
        m_endPos = m_source.find_first_of(m_delimiter, m_startPos);     // Find next non-delimiter.

        return true;
        }

    bool HasValue() const {return String_T::npos != m_startPos;}

    typename String_T::size_type m_startPos;
    typename String_T::size_type m_endPos;
    String_T const& m_source;
    typename String_T::value_type const*   m_delimiter;
    };

typedef StringTokenizer<WString>    WStringTokenizer;
typedef StringTokenizer<Utf8String> Utf8StringTokenizer;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              6/2016
//=====================================================================================
struct RealityDataSerializer : public RefCountedBase
    {
    public:
        // Serialize/Deserialize methods.
        RealityPackageStatus Read(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const;

        // Utility methods.
        static bool IsValidLongLat(double longitude, double latitude);
        static RealityPackageStatus ReadLongLat(double& longitude, double& latitude, BeXmlNodeR parent, Utf8CP childName);
        static RealityPackageStatus WriteLongLat(BeXmlNodeR parent, Utf8CP childName, double longitude, double latitude);
        static RealityPackageStatus ReadGeoPoint2d(GeoPoint2dR point, BeXmlNodeR parent, Utf8CP childName);
        static RealityPackageStatus WriteGeoPoint2d(BeXmlNodeR parent, Utf8CP childName, GeoPoint2dCR point);

    protected:
        virtual ~RealityDataSerializer() {};

        // Read methods.
        RealityPackageStatus ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        RealityPackageStatus ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode);
        SpatialEntityDataSourcePtr ReadSource(RealityPackageStatus& status, BeXmlNodeP pNode);
        MultiBandSourcePtr   ReadMultiBandSource(RealityPackageStatus& status, BeXmlNodeP pNode);

        // Write methods.
        RealityPackageStatus WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        RealityPackageStatus WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const;

    private:
        // Serialize/Deserialize methods to override.
        virtual RealityPackageStatus _Read(RealityDataPackageR package, BeXmlDomR xmlDom) = 0;
        virtual RealityPackageStatus _Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const = 0;

        // Read methods to override if necessary.
        virtual RealityPackageStatus _ReadPackageInfo(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadUnknownElements(RealityDataPackageR package, BeXmlNodeP pNode);
        virtual SpatialEntityDataSourcePtr _ReadSource(RealityPackageStatus& status, BeXmlNodeP pNode);
        virtual MultiBandSourcePtr   _ReadMultiBandSource(RealityPackageStatus& status, BeXmlNodeP pNode);

        // Write methods to override if necessary.
        virtual RealityPackageStatus _WritePackageInfo(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              6/2016
//=====================================================================================
struct RealityDataSerializerV1 : public RealityDataSerializer
    {
    public:
        //! Create serializer for a version 1 package.
        static RealityDataSerializerV1Ptr Create();

    protected:
        RealityDataSerializerV1();
        virtual ~RealityDataSerializerV1() {}

    private:
        // Serialize/Deserialize methods override.
        virtual RealityPackageStatus _Read(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const;

        // Read methods override.
        virtual RealityPackageStatus _ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom);

        // Write methods override.
        virtual RealityPackageStatus _WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              6/2016
//=====================================================================================
struct RealityDataSerializerV2 : public RealityDataSerializer
    {
    public:
        //! Create serializer for a version 2 package.
        static RealityDataSerializerV2Ptr Create();

    protected:
        RealityDataSerializerV2();
        virtual ~RealityDataSerializerV2() {}

    private:
        // Serialize/Deserialize methods override.
        virtual RealityPackageStatus _Read(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _Write(BeXmlDomR xmlDom, RealityDataPackageCR package) const;

        // Read methods override.
        virtual RealityPackageStatus _ReadImageryGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadModelGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadPinnedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);
        virtual RealityPackageStatus _ReadTerrainGroup(RealityDataPackageR package, BeXmlDomR xmlDom);        
        virtual RealityPackageStatus _ReadUndefinedGroup(RealityDataPackageR package, BeXmlDomR xmlDom);        

        // Write methods override.
        virtual RealityPackageStatus _WriteImageryGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteModelGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WritePinnedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteTerrainGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteUndefinedGroup(BeXmlNodeR node, RealityDataPackageCR package) const;
        virtual RealityPackageStatus _WriteSource(BeXmlNodeR node, SpatialEntityDataSourceCR source) const;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              6/2016
//=====================================================================================
struct RealityDataSerializerFactory : RefCountedBase
    {
    public:
        //! Create proper serializer according to package major version.
        static RealityDataSerializerPtr CreateSerializer(BeXmlDomR xmlDom);
        static RealityDataSerializerPtr CreateSerializer(uint32_t majorVersion);

    private:
        //! Read package version.
        static RealityPackageStatus ReadVersion(uint32_t& majorVersion, uint32_t& minorVersion, BeXmlDomR xmlDom);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE