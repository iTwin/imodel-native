/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPackage/RealitySerialization.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityPackage.h>
#include <BeXml/BeXml.h>
#include <geom/GeomApi.h>

// **************************************************
// Naming-Convention:
// Elements are TitleCase. Ex: WmsSource
// Attributes are camelCase. Ex: someAttribute.
#define W3SCHEMA_PREFIX     "xsi"
#define W3SCHEMA_URI        "http://www.w3.org/2001/XMLSchema-instance"

#define PACKAGE_PREFIX              "rdp"
#define PACKAGE_NAMESPACE_1_0       "http://www.bentley.com/RealityDataServer/1.0"
#define PACKAGE_VERSION_1_0         L"1.0"

#define PACKAGE_ROOT_ELEMENT                "RealityDataPackage"
#define PACKAGE_ROOT_ATTRIBUTE_Version      "Version"

// Package 
#define PACKAGE_ELEMENT_Name                "Name"
#define PACKAGE_ELEMENT_Description         "Description"
#define PACKAGE_ELEMENT_CreationDate        "CreationDate"
#define PACKAGE_ELEMENT_Copyright           "Copyright"
#define PACKAGE_ELEMENT_PackageId           "PackageId"
#define PACKAGE_ELEMENT_BoundingPolygon     "BoundingPolygon"

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
#define PACKAGE_ELEMENT_SpatialLocation     "SpatialLocation"

#define PACKAGE_ELEMENT_TerrainGroup        "TerrainGroup"
#define PACKAGE_ELEMENT_TerrainData         "TerrainData"

// RealityDataSources
#define PACKAGE_ELEMENT_Source              "Source"
#define PACKAGE_SOURCE_ATTRIBUTE_Uri        "uri"
#define PACKAGE_SOURCE_ATTRIBUTE_Type       "type"

#define PACKAGE_ELEMENT_WmsSource           "WmsSource"
#define WMS_SOURCE_TYPE                     L"wms"

#define SPACE_DELIMITER    " "
#define SPACE_DELIMITER_U L" "


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
struct RealityDataSourceSerializer
{
    //----------------------------------------------------------------------------------------
    // @bsiclass
    //----------------------------------------------------------------------------------------
    struct IDataSourceCreate
        {
        virtual RealityDataSourcePtr Create() const = 0;
        };

    static RealityDataSourceSerializer& Get() {static RealityDataSourceSerializer s_instance; return s_instance;}

    RealityDataSourceSerializer();

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    RealityDataSourcePtr Load(BeXmlNodeR node);

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    RealityPackageStatus Store(RealityDataSourceCR source, BeXmlNodeR parentNode);

    bmap<Utf8String, IDataSourceCreate*> m_creators;
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
struct RealityDataSerializer
    {

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    static RealityPackageStatus ReadDPoint2d(DPoint2dR point, BeXmlNodeR parent, Utf8CP childName)
        {
        // Use UTF8 since it is the native format.
        Utf8String pointStr;
        if(BEXML_Success != parent.GetContent(pointStr, childName))
            return RealityPackageStatus::UnknownError;  

        Utf8StringTokenizer tokenizer(pointStr, SPACE_DELIMITER);

        if(!tokenizer.Get(point.x) || !tokenizer.Get(point.y))
            return RealityPackageStatus::UnknownError;  

        return RealityPackageStatus::Success;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    static RealityPackageStatus WriteDPoint2d(BeXmlNodeR parent, Utf8CP childName, DPoint2dCR point)
        {
        WString pointString;
        pointString.Sprintf (L"%.10g %.10g", point.x, point.y);  // Alain Robert: Lat/long precision of 1/100 millimeter.
        if(NULL == parent.AddElementStringValue(childName, pointString.c_str()))
            return RealityPackageStatus::UnknownError;

        return RealityPackageStatus::Success;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    template<class RefCountedData_T>
    static RefCountedData_T TryLoad(RealityPackageStatus& status, BeXmlNodeR node)
        {
        status = RealityPackageStatus::Success; // We return NULL and SUCCESS if node is not of RealityData type.
        if(0 != BeStringUtilities::Stricmp(node.GetName(), RefCountedData_T::element_type::ElementName))
            return NULL;

        RefCountedData_T pData = new RefCountedData_T::element_type();

        if(RealityPackageStatus::Success != (status = pData->_Read(node)))
            return NULL;

        return pData;
        }

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  3/2015
    //----------------------------------------------------------------------------------------
    template<class Data_T>
    static RealityPackageStatus Store(Data_T const& data, BeXmlNodeR parentNode)
        {
        BeXmlNodeP pDataNode = parentNode.AddEmptyElement(Data_T::ElementName);
        if(NULL == pDataNode)
            return RealityPackageStatus::UnknownError;

        return data._Write(*pDataNode);
        }
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE