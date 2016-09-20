/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BentleyWMSPackageNet.h"

#include <RealityPackage/WMSSource.h>
#include <RealityPackage/OsmSource.h>

using namespace RealityDataPackageWrapper;
using namespace System;
using namespace System::Runtime::InteropServices;


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::CreateV1(String^  location,
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
    List<DataSourceNet^>^ imgSources = imageryGroup->GetData()[0]->GetSources();
    for each(DataSourceNet^ source in imgSources)
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

            if (0 != wmsSourceNet->GetSize())
                {
                uint64_t size = wmsSourceNet->GetSize();
                pWmsDataSource->SetSize(size);
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

            if (0 != source->GetSize())
                {
                uint64_t size = source->GetSize();
                pDataSource->SetSize(size);
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
    List<DataSourceNet^>^ modelSources = modelGroup->GetData()[0]->GetSources();
    for each(DataSourceNet^ source in modelSources)
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

            if (0 != osmSourceNet->GetSize())
                {
                uint64_t size = osmSourceNet->GetSize();
                pOsmDataSource->SetSize(size);
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

            if (0 != source->GetSize())
                {
                uint64_t size = source->GetSize();
                pDataSource->SetSize(size);
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
    List<DataSourceNet^>^ pinnedSources = pinnedGroup->GetData()[0]->GetSources();
    for each(DataSourceNet^ source in pinnedSources)
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

        if (0 != source->GetSize())
            {
            uint64_t size = source->GetSize();
            pDataSource->SetSize(size);
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
    List<DataSourceNet^>^ terrainSources = terrainGroup->GetData()[0]->GetSources();
    for each(DataSourceNet^ source in terrainSources)
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

        if (0 != source->GetSize())
            {
            uint64_t size = source->GetSize();
            pDataSource->SetSize(size);
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
void RealityDataPackageNet::CreateV2(String^ location,
                                     String^ origin,
                                     String^ name,
                                     String^ description,
                                     String^ copyright,
                                     String^ id,
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

    Utf8String originUtf8;
    BeStringUtilities::WCharToUtf8(originUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(origin).ToPointer()));
    pDataPackage->SetOrigin(originUtf8.c_str());

    Utf8String descriptionUtf8;
    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));
    pDataPackage->SetDescription(descriptionUtf8.c_str());

    Utf8String copyrightUtf8;
    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));
    pDataPackage->SetCopyright(copyrightUtf8.c_str());

    Utf8String idUtf8;
    BeStringUtilities::WCharToUtf8(idUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(id).ToPointer()));
    pDataPackage->SetId(idUtf8.c_str());

    // Create bounding polygon and add it to the package.
    DPoint2d pts[4];
    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
    RealityPackage::BoundingPolygonPtr boundingPolygon = RealityPackage::BoundingPolygon::Create(pts, 4);
    pDataPackage->SetBoundingPolygon(*boundingPolygon);

    // Create imagery data sources and add them to the package.
    for each(DataGroupNet^ dataGroup in imageryGroup->GetData())
        {
        if (0 == dataGroup->GetNumSources())
            continue;       

        // Create data group and add primary source.
        RealityPackage::ImageryDataPtr pImageryData;
        DataSourceNet^ source = dataGroup->GetSources()[0];
        if ("wms" == source->GetSourceType())
            {
            RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityDataPackageNet::CreateWmsDataSource(source);
            pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
            }
        else
            {
            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
            pImageryData = RealityPackage::ImageryData::Create(*pDataSource, NULL);
            }         

        // Add alternate sources to group.
        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
            {
            DataSourceNet^ source = dataGroup->GetSources()[i];
            if ("wms" == source->GetSourceType())
                {
                RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityDataPackageNet::CreateWmsDataSource(source);
                pImageryData->AddSource(*pWmsDataSource);
                }
            else
                {
                RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);                
                pImageryData->AddSource(*pDataSource);
                }                
            }

        pDataPackage->GetImageryGroupR().push_back(pImageryData);
        }

    // Create model data sources and add them to the package.
    for each(DataGroupNet^ dataGroup in modelGroup->GetData())
        {
        if (0 == dataGroup->GetNumSources())
            continue;

        // Create data group and add primary source.
        RealityPackage::ModelDataPtr pModelData;
        DataSourceNet^ source = dataGroup->GetSources()[0];
        if ("osm" == source->GetSourceType())
            {
            RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityDataPackageNet::CreateOsmDataSource(source, boundingPolygon);
            pModelData = RealityPackage::ModelData::Create(*pOsmDataSource);
            }
        else
            {
            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
            pModelData = RealityPackage::ModelData::Create(*pDataSource);
            }

        // Add alternate sources to group.
        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
            {
            DataSourceNet^ source = dataGroup->GetSources()[i];
            if ("osm" == source->GetSourceType())
                {
                RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityDataPackageNet::CreateOsmDataSource(source, boundingPolygon);
                pModelData->AddSource(*pOsmDataSource);
                }
            else
                {
                RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
                pModelData->AddSource(*pDataSource);
                }
            }

        pDataPackage->GetModelGroupR().push_back(pModelData);
        }

    // Create pinned data sources and add them to the package.
    for each(DataGroupNet^ dataGroup in pinnedGroup->GetData())
        {
        if (0 == dataGroup->GetNumSources())
            continue;

        // Create data group and add primary source.
        RealityPackage::PinnedDataPtr pPinnedData;
        DataSourceNet^ source = dataGroup->GetSources()[0];
        RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
        pPinnedData = RealityPackage::PinnedData::Create(*pDataSource, 0.0, 0.0); //&&JFC Implement pinned data longitude and latitude for the wrapper.

        // Add alternate sources to group.
        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
            {
            DataSourceNet^ source = dataGroup->GetSources()[i];
            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
            pPinnedData->AddSource(*pDataSource);
            }

        pDataPackage->GetPinnedGroupR().push_back(pPinnedData);
        }

    // Create terrain data sources and add them to the package.
    for each(DataGroupNet^ dataGroup in terrainGroup->GetData())
        {
        if (0 == dataGroup->GetNumSources())
            continue;

        // Create data group and add primary source.
        RealityPackage::TerrainDataPtr pTerrainData;
        DataSourceNet^ source = dataGroup->GetSources()[0];
        RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
        pTerrainData = RealityPackage::TerrainData::Create(*pDataSource);

        // Add alternate sources to group.
        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
            {
            DataSourceNet^ source = dataGroup->GetSources()[i];
            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
            pTerrainData->AddSource(*pDataSource);
            }

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
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
RealityPackage::RealityDataSourcePtr RealityDataPackageNet::CreateDataSource(DataSourceNet^ source)
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

    if (0 != source->GetSize())
        {
        uint64_t filesize = source->GetSize();
        pDataSource->SetSize(filesize);
        }

    if (!String::IsNullOrEmpty(source->GetMetadata()))
        {
        Utf8String metadata;
        BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
        pDataSource->SetMetadata(metadata.c_str());
        }

    if (!String::IsNullOrEmpty(source->GetGeoCS()))
        {
        Utf8String geocs;
        BeStringUtilities::WCharToUtf8(geocs, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetGeoCS()).ToPointer()));
        pDataSource->SetGeoCS(geocs.c_str());
        }

    if (0 != source->GetSisterFiles()->Count)
        {
        //List<String^>^ sisterFilesNet = source->GetSisterFiles();

        bvector<Utf8String> sisterFiles;
        pDataSource->SetSisterFiles(sisterFiles);
        }

    return pDataSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
RealityPackage::WmsDataSourcePtr RealityDataPackageNet::CreateWmsDataSource(DataSourceNet^ source)
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

    if (0 != wmsSourceNet->GetSize())
        {
        uint64_t size = wmsSourceNet->GetSize();
        pWmsDataSource->SetSize(size);
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

    return pWmsDataSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
RealityPackage::OsmDataSourcePtr RealityDataPackageNet::CreateOsmDataSource(DataSourceNet^ source, RealityPackage::BoundingPolygonPtr boundingPolygon)
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

    if (0 != osmSourceNet->GetSize())
        {
        uint64_t size = osmSourceNet->GetSize();
        pOsmDataSource->SetSize(size);
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

    return pOsmDataSource;
    }


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
void ImageryGroupNet::AddData(DataGroupNet^ imageryDataGroup)
    {
    m_imageryDataList->Add(imageryDataGroup);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ImageryGroupNet::ImageryGroupNet() 
    {
    m_imageryDataList = gcnew List<DataGroupNet^>();
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
void ModelGroupNet::AddData(DataGroupNet^ modelDataGroup)
    {
    m_modelDataList->Add(modelDataGroup);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
ModelGroupNet::ModelGroupNet() 
    {
    m_modelDataList = gcnew List<DataGroupNet^>();
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
void PinnedGroupNet::AddData(DataGroupNet^ pinnedDataGroup)
    {
    m_pinnedDataList->Add(pinnedDataGroup);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
PinnedGroupNet::PinnedGroupNet() 
    {
    m_pinnedDataList = gcnew List<DataGroupNet^>();
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
void TerrainGroupNet::AddData(DataGroupNet^ terrainDataGroup)
    {
    m_terrainDataList->Add(terrainDataGroup);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TerrainGroupNet::TerrainGroupNet() 
    {
    m_terrainDataList = gcnew List<DataGroupNet^>();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
TerrainGroupNet::~TerrainGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
DataGroupNet^ DataGroupNet::Create(System::String^ id,
                                   System::String^ name,
                                   DataSourceNet^ source)
    {
    return gcnew DataGroupNet(id, name, source);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
void DataGroupNet::AddSource(DataSourceNet^ dataSource)
    {
    m_sources->Add(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
DataGroupNet::DataGroupNet(System::String^ id,
                           System::String^ name, 
                           DataSourceNet^ source)
    : m_id(id), 
      m_name(name)
    {
    m_sources->Add(source);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
DataGroupNet::~DataGroupNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
DataSourceNet^ DataSourceNet::Create(System::String^ uri,
                                     System::String^ type, 
                                     System::String^ copyright,
                                     System::String^ id,
                                     System::String^ provider,
                                     uint64_t filesize,
                                     System::String^ fileInCompound,
                                     System::String^ metadata, 
                                     System::String^ geocs,
                                     List<System::String^>^ sisterFiles)
    {
    return gcnew DataSourceNet(uri, type, copyright, id, provider, filesize, fileInCompound, metadata, geocs, sisterFiles);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
DataSourceNet::DataSourceNet(System::String^ uri,
                             System::String^ type,
                             System::String^ copyright,
                             System::String^ id,
                             System::String^ provider,
                             uint64_t size,
                             System::String^ fileInCompound,
                             System::String^ metadata,
                             System::String^ geocs,
                             List<System::String^>^ sisterFiles)
    : m_uri(uri),
      m_type(type),
      m_copyright(copyright),
      m_id(id),
      m_provider(provider),
      m_size(size),
      m_fileInCompound(fileInCompound),
      m_metadata(metadata),
      m_geocs(geocs),
      m_sisterFiles(sisterFiles)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
DataSourceNet::~DataSourceNet() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 5/2015
//-------------------------------------------------------------------------------------
WmsSourceNet::WmsSourceNet(System::String^ uri,
                           System::String^ copyright,
                           System::String^ id,
                           System::String^ provider,
                           uint64_t size,
                           System::String^ metadata,
                           System::String^ geocs,
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
    : DataSourceNet(uri, "wms", copyright, id, provider, size, "", metadata, geocs, sisterFiles)
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

    RealityPackage::WmsMapSettingsPtr pMapSettings = RealityPackage::WmsMapSettings::Create(mapUriUtf8.c_str(),
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
                                   uint64_t size,
                                   System::String^ metadata,
                                   System::String^ geocs,
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
                              size,
                              metadata,
                              geocs,
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
                           uint64_t size,
                           System::String^ metadata,
                           System::String^ geocs,
                           List<System::String^>^ sisterFiles,
                           List<double>^ regionOfInterest,
                           List<System::String^>^ urls)
    : DataSourceNet(uri, "osm", copyright, id, provider, size, "", metadata, geocs, sisterFiles)
    {
    // Create range from min and max values.
    DPoint2d pts[4];
    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];

    double minX = DBL_MAX;
    double minY = DBL_MAX;
    double maxX = -DBL_MAX;
    double maxY = -DBL_MAX;

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
    RealityPackage::OsmResourcePtr pOsmResource = RealityPackage::OsmResource::Create(bbox);

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
                                   uint64_t size,
                                   System::String^ metadata,
                                   System::String^ geocs,
                                   List<System::String^>^ sisterFiles, 
                                   List<double>^ regionOfInterest,
                                   List<System::String^>^ urls)
    {
    return gcnew OsmSourceNet(uri, copyright, id, provider, size, metadata, geocs, sisterFiles, regionOfInterest, urls);
    }
