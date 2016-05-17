/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BentleyWMSPackageNet.h"

#include <RealityPlatform/WMSSource.h>
#include <RealityPlatform/OsmSource.h>

using namespace RealityDataPackageWrapper;
using namespace System;
using namespace System::Runtime::InteropServices;

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 6/2015
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::Create(String^  location,
                                   String^  name,
                                   String^  description,
                                   String^  copyright,
                                   String^  packageId,
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
    Utf8String nameUtf8;
    BeStringUtilities::WCharToUtf8(nameUtf8, nameStr);
    RealityPackage::RealityDataPackagePtr pDataPackage = RealityDataPackage::Create(nameUtf8.c_str());

    Utf8String descriptionUtf8;
    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));
    pDataPackage->SetDescription(descriptionUtf8.c_str());

    Utf8String copyrightUtf8;
    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));
    pDataPackage->SetCopyright(copyrightUtf8.c_str());

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

            // Create source with required parameters.
            Utf8String uri;
            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetUri()).ToPointer()));
            RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityPackage::WmsDataSource::Create(uri.c_str());

            // Add optional parameters.
            if (!String::IsNullOrEmpty(wmsSourceNet->GetCopyright()))
                {
                Utf8String copyright;
                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetCopyright()).ToPointer()));
                pWmsDataSource->SetCopyright(copyright.c_str());
                }

            if (!String::IsNullOrEmpty(source->GetId()))
                {
                Utf8String id;
                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetId()).ToPointer()));
                pWmsDataSource->SetId(id.c_str());
                }

            if (!String::IsNullOrEmpty(wmsSourceNet->GetProvider()))
                {
                Utf8String provider;
                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetProvider()).ToPointer()));
                pWmsDataSource->SetProvider(provider.c_str());
                }

            if (0 != wmsSourceNet->GetFilesize())
                {
                uint64_t filesize = wmsSourceNet->GetFilesize();
                pWmsDataSource->SetFilesize(filesize);
                }

            if (!String::IsNullOrEmpty(wmsSourceNet->GetMetadata()))
                {
                Utf8String metadata;
                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetMetadata()).ToPointer()));
                pWmsDataSource->SetMetadata(metadata.c_str());
                }

            if (0 != wmsSourceNet->GetSisterFiles()->Count)
                {
                //List<String^>^ sisterFilesNet = wmsSourceNet->GetSisterFiles();

                bvector<Utf8String> sisterFiles;
                pWmsDataSource->SetSisterFiles(sisterFiles);
                }

            // Xml Fragment.
            Utf8String xmlFragment;
            BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetXmlFragment()).ToPointer()));
            pWmsDataSource->SetMapSettings(xmlFragment.c_str());

            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
            pDataPackage->GetImageryGroupR().push_back(pImageryData);
            }
        else
            {
            // Create source with required parameters.
            Utf8String uri;
            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));

            Utf8String type;
            BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
            RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());

            // Add optional parameters.
            if (!String::IsNullOrEmpty(source->GetCopyright()))
                {
                Utf8String copyright;
                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
                pDataSource->SetCopyright(copyright.c_str());
                }

            if (!String::IsNullOrEmpty(source->GetId()))
                {
                Utf8String id;
                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
                pDataSource->SetId(id.c_str());
                }

            if (!String::IsNullOrEmpty(source->GetProvider()))
                {
                Utf8String provider;
                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
                pDataSource->SetProvider(provider.c_str());
                }

            if (0 != source->GetFilesize())
                {
                uint64_t filesize = source->GetFilesize();
                pDataSource->SetFilesize(filesize);
                }

            if (!String::IsNullOrEmpty(source->GetMetadata()))
                {
                Utf8String metadata;
                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
                pDataSource->SetMetadata(metadata.c_str());
                }

            if (0 != source->GetSisterFiles()->Count)
                {
                //List<String^>^ sisterFilesNet = source->GetSisterFiles();

                bvector<Utf8String> sisterFiles;
                pDataSource->SetSisterFiles(sisterFiles);
                }

            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pDataSource, NULL);
            pDataPackage->GetImageryGroupR().push_back(pImageryData);
            }
        }

    // Create model data sources and add them to the package.
    for each(RealityDataSourceNet^ source in modelGroup->GetData())
        {
        if ("osm" == source->GetSourceType())
            {
            OsmSourceNet^ osmSourceNet = dynamic_cast<OsmSourceNet^>(source);

            // Find min/max for bbox.
            double minX = DBL_MAX;
            double minY = DBL_MAX;
            double maxX = -DBL_MAX;
            double maxY = -DBL_MAX;

            DPoint2dCP bboxPts = boundingPolygon->GetPointCP();
            for (size_t i = 0; i < boundingPolygon->GetPointCount(); ++i)
                {
                if (bboxPts[i].x < minX)
                    minX = bboxPts[i].x;
                
                if (bboxPts[i].x > maxX)
                    maxX = bboxPts[i].x;

                if (bboxPts[i].y < minY)
                    minY = bboxPts[i].y;

                if (bboxPts[i].y > maxY)
                    maxY = bboxPts[i].y;
                }

            DRange2d bbox;
            bbox.InitFrom(minX, minY, maxX, maxY);
            
            // Create source with required parameters.
            Utf8String uri;
            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetUri()).ToPointer()));
            RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityPackage::OsmDataSource::Create(uri.c_str(), &bbox);

            // Add optional parameters.
            if (!String::IsNullOrEmpty(osmSourceNet->GetCopyright()))
                {
                Utf8String copyright;
                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetCopyright()).ToPointer()));
                pOsmDataSource->SetCopyright(copyright.c_str());
                }

            if (!String::IsNullOrEmpty(osmSourceNet->GetId()))
                {
                Utf8String id;
                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetId()).ToPointer()));
                pOsmDataSource->SetId(id.c_str());
                }

            if (!String::IsNullOrEmpty(osmSourceNet->GetProvider()))
                {
                Utf8String provider;
                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetProvider()).ToPointer()));
                pOsmDataSource->SetProvider(provider.c_str());
                }

            if (0 != osmSourceNet->GetFilesize())
                {
                uint64_t filesize = osmSourceNet->GetFilesize();
                pOsmDataSource->SetFilesize(filesize);
                }

            if (!String::IsNullOrEmpty(osmSourceNet->GetMetadata()))
                {
                Utf8String metadata;
                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetMetadata()).ToPointer()));
                pOsmDataSource->SetMetadata(metadata.c_str());
                }

            if (0 != osmSourceNet->GetSisterFiles()->Count)
                {
                //List<String^>^ sisterFilesNet = osmSourceNet->GetSisterFiles();

                bvector<Utf8String> sisterFiles;
                pOsmDataSource->SetSisterFiles(sisterFiles);
                }

            // Xml Fragment.
            Utf8String xmlFragment;
            BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetXmlFragment()).ToPointer()));
            pOsmDataSource->SetOsmResource(xmlFragment.c_str());

            RealityPackage::ModelDataPtr pModelData = RealityPackage::ModelData::Create(*pOsmDataSource);
            pDataPackage->GetModelGroupR().push_back(pModelData);
            }
        else
            {
            // Create source with required parameters.
            Utf8String uri;
            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));

            Utf8String type;
            BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));

            RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());

            // Add optional parameters.
            if (!String::IsNullOrEmpty(source->GetCopyright()))
                {
                Utf8String copyright;
                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
                pDataSource->SetCopyright(copyright.c_str());
                }

            if (!String::IsNullOrEmpty(source->GetId()))
                {
                Utf8String id;
                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
                pDataSource->SetId(id.c_str());
                }

            if (!String::IsNullOrEmpty(source->GetProvider()))
                {
                Utf8String provider;
                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
                pDataSource->SetProvider(provider.c_str());
                }

            if (0 != source->GetFilesize())
                {
                uint64_t filesize = source->GetFilesize();
                pDataSource->SetFilesize(filesize);
                }

            if (!String::IsNullOrEmpty(source->GetMetadata()))
                {
                Utf8String metadata;
                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
                pDataSource->SetMetadata(metadata.c_str());
                }

            if (0 != source->GetSisterFiles()->Count)
                {
                //List<String^>^ sisterFilesNet = source->GetSisterFiles();

                bvector<Utf8String> sisterFiles;
                pDataSource->SetSisterFiles(sisterFiles);
                }

            RealityPackage::ModelDataPtr pModelData = RealityPackage::ModelData::Create(*pDataSource);
            pDataPackage->GetModelGroupR().push_back(pModelData);
            }
        }

    // Create pinned data sources and add them to the package.
    for each(RealityDataSourceNet^ source in pinnedGroup->GetData())
        {
        // Create source with required parameters.
        Utf8String uri;
        BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));

        Utf8String type;
        BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));

        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());

        // Add optional parameters.
        if (!String::IsNullOrEmpty(source->GetCopyright()))
            {
            Utf8String copyright;
            BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
            pDataSource->SetCopyright(copyright.c_str());
            }

        if (!String::IsNullOrEmpty(source->GetId()))
            {
            Utf8String id;
            BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
            pDataSource->SetId(id.c_str());
            }

        if (!String::IsNullOrEmpty(source->GetProvider()))
            {
            Utf8String provider;
            BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
            pDataSource->SetProvider(provider.c_str());
            }

        if (0 != source->GetFilesize())
            {
            uint64_t filesize = source->GetFilesize();
            pDataSource->SetFilesize(filesize);
            }

        if (!String::IsNullOrEmpty(source->GetMetadata()))
            {
            Utf8String metadata;
            BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
            pDataSource->SetMetadata(metadata.c_str());
            }

        if (0 != source->GetSisterFiles()->Count)
            {
            //List<String^>^ sisterFilesNet = source->GetSisterFiles();

            bvector<Utf8String> sisterFiles;
            pDataSource->SetSisterFiles(sisterFiles);
            }

        //&&JFC Implement pinned data longitude and latitude for the wrapper.
        RealityPackage::PinnedDataPtr pPinnedData = RealityPackage::PinnedData::Create(*pDataSource, 0.0, 0.0);
        pDataPackage->GetPinnedGroupR().push_back(pPinnedData);
        }

    // Create terrain data sources and add them to the package.
    for each(RealityDataSourceNet^ source in terrainGroup->GetData())
        {
        // Create source with required parameters.
        Utf8String uri;
        BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));

        Utf8String type;
        BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));

        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());

        // Add optional parameters.
        if (!String::IsNullOrEmpty(source->GetCopyright()))
            {
            Utf8String copyright;
            BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
            pDataSource->SetCopyright(copyright.c_str());
            }

        if (!String::IsNullOrEmpty(source->GetId()))
            {
            Utf8String id;
            BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
            pDataSource->SetId(id.c_str());
            }

        if (!String::IsNullOrEmpty(source->GetProvider()))
            {
            Utf8String provider;
            BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
            pDataSource->SetProvider(provider.c_str());
            }

        if (0 != source->GetFilesize())
            {
            uint64_t filesize = source->GetFilesize();
            pDataSource->SetFilesize(filesize);
            }

        if (!String::IsNullOrEmpty(source->GetMetadata()))
            {
            Utf8String metadata;
            BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
            pDataSource->SetMetadata(metadata.c_str());
            }

        if (0 != source->GetSisterFiles()->Count)
            {
            //List<String^>^ sisterFilesNet = source->GetSisterFiles();

            bvector<Utf8String> sisterFiles;
            pDataSource->SetSisterFiles(sisterFiles);
            }

        RealityPackage::TerrainDataPtr pTerrainData = RealityPackage::TerrainData::Create(*pDataSource);
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
                                                   System::String^ id,
                                                   System::String^ provider,
                                                   uint64_t filesize,
                                                   System::String^ fileInCompound,
                                                   System::String^ metadata, 
                                                   List<System::String^>^ sisterFiles)
    {
    return gcnew RealityDataSourceNet(uri, type, copyright, id, provider, filesize, fileInCompound, metadata, sisterFiles);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
RealityDataSourceNet::RealityDataSourceNet(System::String^ uri,
                                           System::String^ type,
                                           System::String^ copyright,
                                           System::String^ id,
                                           System::String^ provider,
                                           uint64_t filesize,
                                           System::String^ fileInCompound,
                                           System::String^ metadata,
                                           List<System::String^>^ sisterFiles)
    : m_uri(uri),
      m_type(type),
      m_copyright(copyright),
      m_id(id),
      m_provider(provider),
      m_filesize(filesize),
      m_fileInCompound(fileInCompound),
      m_metadata(metadata),
      m_sisterFiles(sisterFiles)
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
                           System::String^ id,
                           System::String^ provider,
                           uint64_t filesize,
                           System::String^ metadata,
                           List<System::String^>^ sisterFiles,
                           System::String^ mapUri,
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
    : RealityDataSourceNet(uri, "wms", copyright, id, provider, filesize, "", metadata, sisterFiles)
    {
    // Create range from min and max values.
    DRange2d bbox;
    bbox.InitFrom(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

    // Create WmsMapSettings with required parameters.
    Utf8String mapUriUtf8;
    BeStringUtilities::WCharToUtf8(mapUriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(mapUri).ToPointer()));

    Utf8String versionUtf8;
    BeStringUtilities::WCharToUtf8(versionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(version).ToPointer()));
    
    Utf8String layersUtf8;
    BeStringUtilities::WCharToUtf8(layersUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(layers).ToPointer()));

    Utf8String csTypeUtf8;
    BeStringUtilities::WCharToUtf8(csTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csType).ToPointer()));

    Utf8String csLabelUtf8;
    BeStringUtilities::WCharToUtf8(csLabelUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csLabel).ToPointer()));

    RealityPlatform::WmsMapSettingsPtr pMapSettings = RealityPlatform::WmsMapSettings::Create(mapUriUtf8.c_str(),
                                                                                              bbox,
                                                                                              versionUtf8.c_str(),
                                                                                              layersUtf8.c_str(),
                                                                                              csTypeUtf8.c_str(),
                                                                                              csLabelUtf8.c_str());
    
    // Optional parameters.
    Utf8String stylesUtf8;
    BeStringUtilities::WCharToUtf8(stylesUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(styles).ToPointer()));
    pMapSettings->SetStyles(stylesUtf8.c_str());
    
    Utf8String formatUtf8;
    BeStringUtilities::WCharToUtf8(formatUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer()));
    pMapSettings->SetFormat(formatUtf8.c_str());
    
    Utf8String vendorSpecificUtf8;
    BeStringUtilities::WCharToUtf8(vendorSpecificUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(vendorSpecific).ToPointer()));
    pMapSettings->SetVendorSpecific(vendorSpecificUtf8.c_str());
    
    pMapSettings->SetTransparency(isTransparent);

    // Convert to xml fragment and store the info.
    Utf8String xmlFragment;
    pMapSettings->ToXml(xmlFragment);
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
                                   System::String^ id,
                                   System::String^ provider,
                                   uint64_t filesize,
                                   System::String^ metadata,
                                   List<System::String^>^ sisterFiles,
                                   System::String^ mapUri,
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
                              id,
                              provider,
                              filesize,
                              metadata,
                              sisterFiles,
                              mapUri,
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
// @bsimethod                                   Jean-Francois.Cote         	    11/2015
//-------------------------------------------------------------------------------------
OsmSourceNet::OsmSourceNet(System::String^ uri,
                           System::String^ copyright,
                           System::String^ id,
                           System::String^ provider,
                           uint64_t filesize,
                           System::String^ metadata,
                           List<System::String^>^ sisterFiles,
                           List<double>^ regionOfInterest,
                           List<System::String^>^ urls)
    : RealityDataSourceNet(uri, "osm", copyright, id, provider, filesize, "", metadata, sisterFiles)
    {
    // Create range from min and max values.
    DPoint2d pts[4];
    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];

    double minX = DBL_MAX;
    double minY = DBL_MAX;
    double maxX = DBL_MIN;
    double maxY = DBL_MIN;

    for (size_t i = 0; i < 4; ++i)
        {
        if (pts[i].x < minX)
            minX = pts[i].x;

        if (pts[i].x > maxX)
            maxX = pts[i].x;

        if (pts[i].y < minY)
            minY = pts[i].y;

        if (pts[i].y > maxY)
            maxY = pts[i].y;
        }

    DRange2d bbox;
    bbox.InitFrom(minX, minY, maxX, maxY);

    // Create OsmResource with required parameters.
    RealityPlatform::OsmResourcePtr pOsmResource = RealityPlatform::OsmResource::Create(bbox);

    // Optional parameters.    
    if (0 != urls->Count)
        {
        bvector<Utf8String> urlList;
        for each(System::String^ url in urls)
            {
            Utf8String urlUtf8;
            BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer()));
            urlList.push_back(urlUtf8);
            }
        pOsmResource->SetAlternateUrlList(urlList);
        }

    // Convert to xml fragment and store the info.
    Utf8String xmlFragment;
    pOsmResource->ToXml(xmlFragment);
    m_xmlFragment = gcnew System::String(xmlFragment.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    11/2015
//-------------------------------------------------------------------------------------
OsmSourceNet::~OsmSourceNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    11/2015
//-------------------------------------------------------------------------------------
OsmSourceNet^ OsmSourceNet::Create(System::String^ uri,
                                   System::String^ copyright,
                                   System::String^ id,
                                   System::String^ provider,
                                   uint64_t filesize,
                                   System::String^ metadata,
                                   List<System::String^>^ sisterFiles, 
                                   List<double>^ regionOfInterest,
                                   List<System::String^>^ urls)
    {
    return gcnew OsmSourceNet(uri, copyright, id, provider, filesize, metadata, sisterFiles, regionOfInterest, urls);
    }
