/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BentleyWMSPackageNet.h"

#include <RealityPlatform/WMSSource.h>
#include <RealityPlatform/UsgsSource.h>

using namespace BentleyRealityDataPackage;
using namespace System::Runtime::InteropServices;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::Create(System::String^ location, 
                                   System::String^ name, 
                                   List<double>^ regionOfInterest, 
                                   List<WmsMapInfoNet^>^ wmsMapInfoList, 
                                   List<UsgsSourceNet^>^ usgsSourceList)
    {
    // Construct file location.
    WString fullPath;
    WCharCP locationStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(location).ToPointer());
    WCharCP nameStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer());
    fullPath += locationStr;
    fullPath += nameStr;
    fullPath.AppendUtf8(".xrdp");
    BeFileName packageFullPath(fullPath.c_str());

    // Create bounding polygon.
    DPoint2d pts[4];
    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
    RealityPackage::BoundingPolygonPtr boundingPolygon = RealityPackage::BoundingPolygon::Create(pts, 4);

    // Create package.
    RealityPackage::RealityDataPackagePtr pDataPackage = RealityDataPackage::Create(static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));
    pDataPackage->SetBoundingPolygon(*boundingPolygon);

    // Create the Wms data source and add it to the package.
    for each(WmsMapInfoNet^ wmsMapInfo in wmsMapInfoList)
        {
        // System::String^ to Ut8String conversion.
        // Url.
        Utf8String utf8Url = "";
        WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsMapInfo->GetUrl()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Url, temp);
        // Xml Fragment.
        Utf8String utf8XmlFragment = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsMapInfo->GetXml()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8XmlFragment, temp);
        // Copyright.
        Utf8String utf8Copyright = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsMapInfo->GetCopyright()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Copyright, temp);


        RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityPackage::WmsDataSource::Create(utf8Url.c_str());
        pWmsDataSource->SetMapInfo(utf8XmlFragment.c_str());

        RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
        pImageryData->SetCopyright(utf8Copyright.c_str());

        pDataPackage->GetImageryGroupR().push_back(pImageryData);
        }

    // Create the Usgs data source and add it to the package.
    for each(UsgsSourceNet^ usgsSource in usgsSourceList)
        {
        // System::String^ to Ut8String conversion.
        // Url.
        Utf8String utf8Url = "";
        WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSource->GetUrl()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Url, temp);
        // Xml Fragment.
        Utf8String utf8XmlFragment = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSource->GetXml()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8XmlFragment, temp);
        // Copyright.
        Utf8String utf8Copyright = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSource->GetCopyright()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Copyright, temp);


        RealityPackage::CompoundDataSourcePtr pCompoundDataSource = RealityPackage::CompoundDataSource::Create(utf8Url.c_str(), L"usgs");
        pCompoundDataSource->Set(utf8XmlFragment.c_str());

        RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pCompoundDataSource, NULL);
        pImageryData->SetCopyright(utf8Copyright.c_str());

        pDataPackage->GetImageryGroupR().push_back(pImageryData);
        }

    pDataPackage->Write(packageFullPath);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
RealityDataPackageNet::RealityDataPackageNet()  {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
RealityDataPackageNet::~RealityDataPackageNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapInfoNet::WmsMapInfoNet(System::String^ copyright,
                             System::String^ url,
                             double bboxMinX,
                             double bboxMinY,
                             double bboxMaxX,
                             double bboxMaxY,
                             System::String^ version,
                             System::String^ layers,
                             System::String^ csType,
                             System::String^ csLabel,
                             size_t metaWidth,
                             size_t metaHeight,
                             System::String^ styles,
                             System::String^ format,
                             System::String^ vendorSpecific,
                             bool isTransparent)
    {
    // Create range from min and max values.
    DRange2d bbox;
    bbox.InitFrom(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

    // Create WmsMapInfo with required parameters.
    WCharCP temp = 0;

    Utf8String utf8Url = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Url, temp);

    Utf8String utf8Version = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(version).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Version, temp);

    Utf8String utf8Layers = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(layers).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Layers, temp);

    Utf8String utf8CsType = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csType).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8CsType, temp);

    Utf8String utf8CsLabel = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csLabel).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8CsLabel, temp);

    RealityPlatform::WmsMapInfoPtr pMapInfo = RealityPlatform::WmsMapInfo::Create(utf8Url.c_str(),
                                                                                  bbox,
                                                                                  utf8Version.c_str(),
                                                                                  utf8Layers.c_str(),
                                                                                  utf8CsType.c_str(),
                                                                                  utf8CsLabel.c_str());

    //pMapInfo->SetMetaWidth(metaWidth);
    //pMapInfo->SetMetaHeight(metaHeight);

    Utf8String utf8Styles = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(styles).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Styles, temp);
    pMapInfo->SetStyles(utf8Styles.c_str());

    Utf8String utf8Format = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Format, temp);
    pMapInfo->SetFormat(utf8Format.c_str());

    Utf8String utf8VendorSpecific = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(vendorSpecific).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8VendorSpecific, temp);
    pMapInfo->SetVendorSpecific(utf8VendorSpecific.c_str());

    pMapInfo->SetTransparency(isTransparent);    

    // Set members.
    m_url = gcnew System::String(url);

    m_copyright = gcnew System::String(copyright);

    Utf8String xmlFragment;
    pMapInfo->ToXml(xmlFragment);
    m_xmlFragment = gcnew System::String(xmlFragment.c_str());

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsMapInfoNet::~WmsMapInfoNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WmsMapInfoNet^ WmsMapInfoNet::Create(System::String^ copyright,
                                     System::String^ url,
                                     double bboxMinX,
                                     double bboxMinY,
                                     double bboxMaxX,
                                     double bboxMaxY,
                                     System::String^ version,
                                     System::String^ layers,
                                     System::String^ csType,
                                     System::String^ csLabel,
                                     size_t metaWidth,
                                     size_t metaHeight,
                                     System::String^ styles,
                                     System::String^ format,
                                     System::String^ vendorSpecific,
                                     bool isTransparent)
    {
    return gcnew WmsMapInfoNet(copyright,
                               url,
                               bboxMinX,
                               bboxMinY,
                               bboxMaxX,
                               bboxMaxY,
                               version,
                               layers,
                               csType,
                               csLabel,
                               metaWidth,
                               metaHeight,
                               styles,
                               format,
                               vendorSpecific,
                               isTransparent);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSourceNet::UsgsSourceNet(System::String^ copyright, 
                             System::String^ url, 
                             System::String^ dataType, 
                             System::String^ dataLocation, 
                             List<System::String^>^ sisterFiles, 
                             System::String^ metadata)
    {
    // Create Usgs source with required parameters.
    WCharCP temp = 0;

    Utf8String utf8Url = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Url, temp);

    Utf8String utf8DataType = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(dataType).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8DataType, temp);

    Utf8String utf8Metadata = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Metadata, temp);

    RealityPlatform::UsgsSourcePtr pUsgsSource = RealityPlatform::UsgsSource::Create(utf8Url.c_str(), utf8DataType.c_str(), utf8Metadata.c_str());

    // Optional parameters.
    if (!System::String::IsNullOrEmpty(dataLocation))
        {
        Utf8String utf8Location = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(dataLocation).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Location, temp);
        pUsgsSource->SetDataLocation(utf8Location.c_str());
        }
    
    if (0 != sisterFiles->Count)
        {
        bvector<Utf8String> filenameList;
        for each(System::String^ filename in sisterFiles)
            {
            WCharCP temp = 0;
            Utf8String utf8Filename = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(filename).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Filename, temp);

            filenameList.push_back(utf8Filename);
            }
        pUsgsSource->SetSisterFiles(filenameList);
        }

    // Set members.
    m_url = gcnew System::String(url);

    m_copyright = gcnew System::String(copyright);

    Utf8String xmlFragment;
    pUsgsSource->ToXml(xmlFragment);
    m_xmlFragment = gcnew System::String(xmlFragment.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSourceNet::~UsgsSourceNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
UsgsSourceNet^ UsgsSourceNet::Create(System::String^ copyright, 
                                     System::String^ url, 
                                     System::String^ dataType, 
                                     System::String^ dataLocation, 
                                     List<System::String^>^ sisterFiles, 
                                     System::String^ metadata)
    {
    return gcnew UsgsSourceNet(copyright, url, dataType, dataLocation, sisterFiles, metadata);
    }
