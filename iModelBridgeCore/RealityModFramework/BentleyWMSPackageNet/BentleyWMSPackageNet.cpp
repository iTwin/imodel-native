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

using namespace RealityDataPackageWrapper;
using namespace System::Runtime::InteropServices;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::Create(System::String^  location,
                                   System::String^  name,
                                   System::String^  description,
                                   System::String^  copyright,
                                   List<double>^    regionOfInterest,
                                   ImageryGroupNet^ imageryGroup,
                                   ModelGroupNet^   modelGroup,
                                   PinnedGroupNet^  pinnedGroup,
                                   TerrainGroupNet^ terrainGroup)
    {
    // Construct package location.
    WString fullPath;
    WCharCP locationStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(location).ToPointer());
    WCharCP nameStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer());
    fullPath += locationStr;
    fullPath += nameStr;
    fullPath.AppendUtf8(".xrdp");
    BeFileName packageFullPath(fullPath.c_str());

    // Create package.
    RealityPackage::RealityDataPackagePtr pDataPackage = RealityDataPackage::Create(static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));
    pDataPackage->SetDescription(static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));
    pDataPackage->SetCopyright(static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));

    // Create bounding polygon and it to the package.
    DPoint2d pts[4];
    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
    RealityPackage::BoundingPolygonPtr boundingPolygon = RealityPackage::BoundingPolygon::Create(pts, 4);
    pDataPackage->SetBoundingPolygon(*boundingPolygon);

    // Create imagery data sources and add them to the package.
    for each(RealityDataSourceNet^ source in imageryGroup->GetData())
        {
        if ("wms" == source->GetSourceType())
            {
            WmsSourceNet^ wmsSourceNet = dynamic_cast<WmsSourceNet^>(source);

            // Url.
            Utf8String utf8Url = "";
            WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetUri()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Url, temp);

            // Copyright.
            Utf8String utf8Copyright = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetCopyright()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Copyright, temp);

            // Xml Fragment.
            Utf8String utf8XmlFragment = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetXmlFragment()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8XmlFragment, temp);

            RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityPackage::WmsDataSource::Create(utf8Url.c_str());
            pWmsDataSource->SetMapInfo(utf8XmlFragment.c_str());

            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
            pImageryData->SetCopyright(utf8Copyright.c_str());

            pImageryData->SetFilesize(wmsSourceNet->GetFilesize());

            pDataPackage->GetImageryGroupR().push_back(pImageryData);
            }
        else if ("usgs" == source->GetSourceType())
            {
            UsgsSourceNet^ usgsSourceNet = dynamic_cast<UsgsSourceNet^>(source);
            // System::String^ to Ut8String conversion.
            // Url.
            Utf8String utf8Uri = "";
            WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSourceNet->GetUri()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Uri, temp);
            // Xml Fragment.
            Utf8String utf8XmlFragment = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSourceNet->GetXmlFragment()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8XmlFragment, temp);
            // Copyright.
            Utf8String utf8Copyright = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(usgsSourceNet->GetCopyright()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Copyright, temp);


            RealityPackage::CompoundDataSourcePtr pCompoundDataSource = RealityPackage::CompoundDataSource::Create(utf8Uri.c_str(), L"usgs");
            pCompoundDataSource->Set(utf8XmlFragment.c_str());

            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pCompoundDataSource, NULL);
            pImageryData->SetCopyright(utf8Copyright.c_str());

            pImageryData->SetFilesize(usgsSourceNet->GetFilesize());

            pDataPackage->GetImageryGroupR().push_back(pImageryData);
            }
        else
            {
            // System::String^ to Ut8String conversion.
            // Url.
            Utf8String utf8Uri = "";
            WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Uri, temp);
            // Type.
            WString typeW = L"";
            typeW = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer());
            // Copyright.
            Utf8String utf8Copyright = "";
            temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer());
            BeStringUtilities::WCharToUtf8(utf8Copyright, temp);

            RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(utf8Uri.c_str(), typeW.c_str());

            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pDataSource, NULL);
            pImageryData->SetCopyright(utf8Copyright.c_str());

            pImageryData->SetFilesize(source->GetFilesize());

            pDataPackage->GetImageryGroupR().push_back(pImageryData);
            }
        }

    // Create model data sources and add them to the package.
    for each(RealityDataSourceNet^ source in modelGroup->GetData())
        {
        // System::String^ to Ut8String conversion.
        // Url.
        Utf8String utf8Url = "";
        WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Url, temp);
        // Type.
        WString typeW = L"";
        typeW = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer());
        // Copyright.
        Utf8String utf8Copyright = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Copyright, temp);

        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(utf8Url.c_str(), typeW.c_str());

        RealityPackage::ModelDataPtr pModelData = RealityPackage::ModelData::Create(*pDataSource);
        pModelData->SetCopyright(utf8Copyright.c_str());

        pModelData->SetFilesize(source->GetFilesize());

        pDataPackage->GetModelGroupR().push_back(pModelData);
        }

    // Create pinned data sources and add them to the package.
    for each(RealityDataSourceNet^ source in pinnedGroup->GetData())
        {
        // System::String^ to Ut8String conversion.
        // Url.
        Utf8String utf8Url = "";
        WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Url, temp);
        // Type.
        WString typeW = L"";
        typeW = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer());
        // Copyright.
        Utf8String utf8Copyright = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Copyright, temp);

        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(utf8Url.c_str(), typeW.c_str());

        //&&JFC Implement pinned data longitude and latitude for the wrapper.
        RealityPackage::PinnedDataPtr pPinnedData = RealityPackage::PinnedData::Create(*pDataSource, 0.0, 0.0);
        pPinnedData->SetCopyright(utf8Copyright.c_str());

        pPinnedData->SetFilesize(source->GetFilesize());

        pDataPackage->GetPinnedGroupR().push_back(pPinnedData);
        }

    // Create terrain data sources and add them to the package.
    for each(RealityDataSourceNet^ source in terrainGroup->GetData())
        {
        // System::String^ to Ut8String conversion.
        // Url.
        Utf8String utf8Url = "";
        WCharCP temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Url, temp);
        // Type.
        WString typeW = L"";
        typeW = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer());
        // Copyright.
        Utf8String utf8Copyright = "";
        temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer());
        BeStringUtilities::WCharToUtf8(utf8Copyright, temp);

        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(utf8Url.c_str(), typeW.c_str());

        RealityPackage::TerrainDataPtr pTerrainData = RealityPackage::TerrainData::Create(*pDataSource);
        pTerrainData->SetCopyright(utf8Copyright.c_str());

        pTerrainData->SetFilesize(source->GetFilesize());

        pDataPackage->GetTerrainGroupR().push_back(pTerrainData);
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
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ImageryGroupNet^ ImageryGroupNet::Create()
    {
    return gcnew ImageryGroupNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void ImageryGroupNet::AddData(RealityDataSourceNet^ imageryData)
    {
    m_imageryDataList->Add(imageryData);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ImageryGroupNet::ImageryGroupNet() 
    {
    m_imageryDataList = gcnew List<RealityDataSourceNet^>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ImageryGroupNet::~ImageryGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ModelGroupNet^ ModelGroupNet::Create()
    {
    return gcnew ModelGroupNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void ModelGroupNet::AddData(RealityDataSourceNet^ modelData)
    {
    m_modelDataList->Add(modelData);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ModelGroupNet::ModelGroupNet() 
    {
    m_modelDataList = gcnew List<RealityDataSourceNet^>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ModelGroupNet::~ModelGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
PinnedGroupNet^ PinnedGroupNet::Create()
    {
    return gcnew PinnedGroupNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void PinnedGroupNet::AddData(RealityDataSourceNet^ pinnedData)
    {
    m_pinnedDataList->Add(pinnedData);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
PinnedGroupNet::PinnedGroupNet() 
    {
    m_pinnedDataList = gcnew List<RealityDataSourceNet^>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
PinnedGroupNet::~PinnedGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TerrainGroupNet^ TerrainGroupNet::Create()
    {
    return gcnew TerrainGroupNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
void TerrainGroupNet::AddData(RealityDataSourceNet^ terrainData)
    {
    m_terrainDataList->Add(terrainData);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TerrainGroupNet::TerrainGroupNet() 
    {
    m_terrainDataList = gcnew List<RealityDataSourceNet^>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TerrainGroupNet::~TerrainGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ RealityDataSourceNet::Create(System::String^ uri, 
                                                   System::String^ type, 
                                                   System::String^ copyright,
                                                   double size)
    {
    return gcnew RealityDataSourceNet(uri, type, copyright, size);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
RealityDataSourceNet::RealityDataSourceNet(System::String^ uri,
                                           System::String^ type,
                                           System::String^ copyright,
                                           double size)
    : m_uri(uri),
      m_type(type),
      m_copyright(copyright),
      m_size(size)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
RealityDataSourceNet::~RealityDataSourceNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsSourceNet::WmsSourceNet(System::String^ uri,
                           System::String^ copyright,
                           double size,
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
    : RealityDataSourceNet(uri, "wms", copyright, size)
    {
    // Create range from min and max values.
    DRange2d bbox;
    bbox.InitFrom(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

    // Create WmsMapInfo with required parameters.
    WCharCP temp = 0;

    Utf8String utf8Uri = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Uri, temp);

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

    RealityPlatform::WmsMapInfoPtr pMapInfo = RealityPlatform::WmsMapInfo::Create(utf8Uri.c_str(),
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

    // Convert to xml fragment and store the info.
    Utf8String xmlFragment;
    pMapInfo->ToXml(xmlFragment);
    m_xmlFragment = gcnew System::String(xmlFragment.c_str());

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsSourceNet::~WmsSourceNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
WmsSourceNet^ WmsSourceNet::Create(System::String^ uri,
                                   System::String^ copyright,
                                   double size,
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
    return gcnew WmsSourceNet(uri,
                              copyright,
                              size,
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
UsgsSourceNet::UsgsSourceNet(System::String^ uri,
                             System::String^ copyright,
                             double size,
                             System::String^ dataType, 
                             System::String^ dataLocation, 
                             List<System::String^>^ sisterFiles, 
                             System::String^ metadata)
    : RealityDataSourceNet(uri, "usgs", copyright, size)
    {
    // Create Usgs source with required parameters.
    WCharCP temp = 0;

    Utf8String utf8Uri = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Uri, temp);

    Utf8String utf8DataType = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(dataType).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8DataType, temp);

    Utf8String utf8Metadata = "";
    temp = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata).ToPointer());
    BeStringUtilities::WCharToUtf8(utf8Metadata, temp);

    RealityPlatform::UsgsSourcePtr pUsgsSource = RealityPlatform::UsgsSource::Create(utf8Uri.c_str(), utf8DataType.c_str(), utf8Metadata.c_str());

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

    // Convert to xml fragment and store the info.
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
UsgsSourceNet^ UsgsSourceNet::Create(System::String^ uri,
                                     System::String^ copyright,
                                     double size,
                                     System::String^ dataType, 
                                     System::String^ dataLocation, 
                                     List<System::String^>^ sisterFiles, 
                                     System::String^ metadata)
    {
    return gcnew UsgsSourceNet(uri, copyright, size, dataType, dataLocation, sisterFiles, metadata);
    }
