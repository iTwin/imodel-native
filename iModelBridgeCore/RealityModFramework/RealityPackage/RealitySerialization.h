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

#define PACKAGE_ELEMENT_ModelGroup          "ModelGroup"
#define PACKAGE_ELEMENT_ModelData           "ModelData"

#define PACKAGE_ELEMENT_PinnedGroup         "PinnedGroup"
#define PACKAGE_ELEMENT_PinnedData          "PinnedData"

#define PACKAGE_ELEMENT_TerrainGroup        "TerrainGroup"
#define PACKAGE_ELEMENT_TerrainData         "TerrainData"

// RealityDataSources
#define PACKAGE_ELEMENT_Source              "Source"
#define PACKAGE_SOURCE_ATTRIBUTE_Uri        "uri"
#define PACKAGE_SOURCE_ATTRIBUTE_Type       "type"

#define PACKAGE_ELEMENT_WmsSource           "WmsSource"
#define WMS_SOURCE_TYPE                     L"wms"


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

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
    BentleyApi::RealityPackage::RealityPackageStatus Store(RealityDataSourceCR source, BeXmlNodeR parentNode);

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
    template<class RefCountedData_T>
    static RefCountedData_T Load(BeXmlNodeR node)
        {
        if(0 != BeStringUtilities::Stricmp(node.GetName(), RefCountedData_T::element_type::ElementName))
            return NULL;

        RefCountedData_T pData = new RefCountedData_T::element_type();

        if(RealityPackageStatus::Success != pData->_Read(node))
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