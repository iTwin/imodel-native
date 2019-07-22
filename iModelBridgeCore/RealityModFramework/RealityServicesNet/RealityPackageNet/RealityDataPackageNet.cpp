/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "RealityDataPackageNet.h"
#include "SpatialEntityDataSourceNet.h"

#include <Bentley/BeFileName.h>


using namespace RealityPlatform;
using namespace RealityPackageNet;

// System.
using namespace System;
using namespace System::Collections::Generic;

// Interop.
#include <msclr/marshal.h>

using namespace msclr::interop;
using namespace System::Runtime::InteropServices;


//=======================================================================================
//                              Utility functions
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriPtr ManagedToNativeUri2(UriNet^ managedUri)
    {
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedUri->ToStr()).ToPointer()));

    return RealityPlatform::Uri::Create(uriUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Christian.Tye-gingras         	02/2017
//-------------------------------------------------------------------------------------
SpatialEntityDataSourcePtr ManagedToNativeSpatialEntityDataSource3(SpatialEntityDataSourceNet^ managedSource)
    {
    return *(SpatialEntityDataSourcePtr*) managedSource->GetPeer().ToPointer();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourcePtr ManagedToNativeMultiBandSource(MultiBandSourceNet^ managedSource)
    {
    RealityPlatform::UriPtr nativeUri = ManagedToNativeUri2(managedSource->GetUri());

    Utf8String nativeType;
    BeStringUtilities::WCharToUtf8(nativeType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetSourceType()).ToPointer()));

    // Create source with required parameters.
    MultiBandSourcePtr nativeSource = MultiBandSource::Create(*nativeUri, nativeType.c_str());

    // Id.
    Utf8String nativeId;
    BeStringUtilities::WCharToUtf8(nativeId, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetId()).ToPointer()));
    nativeSource->SetId(nativeId.c_str());

    SpatialEntityMetadataPtr pMetadata = SpatialEntityMetadata::Create();

    // Copyright.
    Utf8String nativeCopyright;
    BeStringUtilities::WCharToUtf8(nativeCopyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetCopyright()).ToPointer()));
    pMetadata->SetLegal(nativeCopyright.c_str());

    // Term of use.
    Utf8String nativeTermOfUse;
    BeStringUtilities::WCharToUtf8(nativeTermOfUse, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetTermOfUse()).ToPointer()));
    pMetadata->SetTermsOfUse(nativeTermOfUse.c_str());

    // Provider.
    Utf8String nativeProvider;
    BeStringUtilities::WCharToUtf8(nativeProvider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetProvider()).ToPointer()));
    nativeSource->SetProvider(nativeProvider.c_str());

    // Visibility.
    Utf8String nativeVisibilityTag;
    BeStringUtilities::WCharToUtf8(nativeVisibilityTag, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetVisibilityTag()).ToPointer()));
    nativeSource->SetVisibilityByTag(nativeVisibilityTag.c_str());

    // Size.
    nativeSource->SetSize(managedSource->GetSize());

    // Metadata.
    Utf8String nativeMetadata;
    BeStringUtilities::WCharToUtf8(nativeMetadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadata()).ToPointer()));
    pMetadata->SetDescription(nativeMetadata.c_str());

    // Metadata type.
    Utf8String nativeMetadataType;
    BeStringUtilities::WCharToUtf8(nativeMetadataType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadataType()).ToPointer()));
    pMetadata->SetMetadataType(nativeMetadataType.c_str());

    nativeSource->SetMetadata(pMetadata);

    // GeoCS.
    Utf8String nativeGeoCS;
    BeStringUtilities::WCharToUtf8(nativeGeoCS, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetGeoCS()).ToPointer()));
    nativeSource->SetGeoCS(nativeGeoCS.c_str());

    // No data value.
    Utf8String nativeNoDataValue;
    BeStringUtilities::WCharToUtf8(nativeNoDataValue, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetNoDataValue()).ToPointer()));
    nativeSource->SetNoDataValue(nativeNoDataValue.c_str());

    // Sister files.
    List<UriNet^>^ managedSisterFiles = managedSource->GetSisterFiles();
    bvector<UriPtr> nativeSisterFiles;
    for each (UriNet^ managedSisterFile in managedSisterFiles)
        {
        nativeSisterFiles.push_back(ManagedToNativeUri2(managedSisterFile));
        }
    nativeSource->SetSisterFiles(nativeSisterFiles);

    // Multiband source specific info.
    nativeSource->SetRedBand(*ManagedToNativeSpatialEntityDataSource3(managedSource->GetRedBand()));
    nativeSource->SetGreenBand(*ManagedToNativeSpatialEntityDataSource3(managedSource->GetGreenBand()));
    nativeSource->SetBlueBand(*ManagedToNativeSpatialEntityDataSource3(managedSource->GetBlueBand()));
    nativeSource->SetPanchromaticBand(*ManagedToNativeSpatialEntityDataSource3(managedSource->GetPanchromaticBand()));

    return nativeSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PackageRealityDataPtr ManagedToNativeImageryData(ImageryDataNet^ managedData)
    {
    PackageRealityDataPtr pData;

    // Create with main source.
    MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(0));
    if (nullptr != pMultiBandSource)
        {
        pData = PackageRealityData::CreateImagery(*ManagedToNativeMultiBandSource(pMultiBandSource), bvector<GeoPoint2d>());
        }
    else
        {
        pData = PackageRealityData::CreateImagery(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(0)), bvector<GeoPoint2d>());
        }

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetIdentifier(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    Utf8String resolution;
    BeStringUtilities::WCharToUtf8(resolution, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetResolution()).ToPointer()));
    pData->SetResolution(resolution.c_str());

    // Set corners.
    List<double>^ corners = managedData->GetCornersCP();
    if (0 != corners->Count)
        {
        bvector<GeoPoint2d> cornerPts = bvector<GeoPoint2d>(4);

        int j = 0;
        for (int i = 0; i < 4; ++i)
            {
            cornerPts[i].longitude = corners[j++];
            cornerPts[i].latitude = corners[j++];
            }

        pData->SetFootprint(cornerPts);
        }

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(i));
        if (nullptr != pMultiBandSource)
            {
            pData->AddDataSource(*ManagedToNativeMultiBandSource(pMultiBandSource));
            }
        else
            {
            pData->AddDataSource(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(i)));
            }        
        }    

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PackageRealityDataPtr ManagedToNativeModelData(ModelDataNet^ managedData)
    {
    PackageRealityDataPtr pData;

    // Create with main source.
    MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(0));
    if (nullptr != pMultiBandSource)
        {
        pData = PackageRealityData::CreateModel(*ManagedToNativeMultiBandSource(pMultiBandSource));
        }
    else
        {
        pData = PackageRealityData::CreateModel(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(0)));
        }

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetIdentifier(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    Utf8String resolution;
    BeStringUtilities::WCharToUtf8(resolution, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetResolution()).ToPointer()));
    pData->SetResolution(resolution.c_str());

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(i));
        if (nullptr != pMultiBandSource)
            {
            pData->AddDataSource(*ManagedToNativeMultiBandSource(pMultiBandSource));
            }
        else
            {
            pData->AddDataSource(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(i)));
            }
        }

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PackageRealityDataPtr ManagedToNativePinnedData(PinnedDataNet^ managedData)
    {
    PackageRealityDataPtr pData;

    // Create with main source.
    MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(0));
    if (nullptr != pMultiBandSource)
        {
        pData = PackageRealityData::CreatePinned(*ManagedToNativeMultiBandSource(pMultiBandSource), 0, 0);
        }
    else
        {
        pData = PackageRealityData::CreatePinned(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(0)), 0, 0);
        }

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetIdentifier(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    Utf8String resolution;
    BeStringUtilities::WCharToUtf8(resolution, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetResolution()).ToPointer()));
    pData->SetResolution(resolution.c_str());

    // Set location.
    List<double>^ location = managedData->GetLocation();
    GeoPoint2d longlat;
    longlat.longitude = location[0];
    longlat.latitude = location[1];
    pData->SetLocation(longlat);

    // Set area.
    List<double>^ polygonPts = managedData->GetAreaCP();
    bvector<GeoPoint2d> pts = bvector<GeoPoint2d>(4);
    int j = 0;
    for (int i = 0; i < polygonPts->Count; ++i)
        {
        pts[i].longitude = polygonPts[j++];
        pts[i].latitude = polygonPts[j++];
        }
    pData->SetFootprint(pts);

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
     {
        MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(i));
        if (nullptr != pMultiBandSource)
            {
            pData->AddDataSource(*ManagedToNativeMultiBandSource(pMultiBandSource));
            }
        else
            {
            pData->AddDataSource(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(i)));
            }
        }

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PackageRealityDataPtr ManagedToNativeTerrainData(TerrainDataNet^ managedData)
    {
    PackageRealityDataPtr pData;

    // Create with main source.
    MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(0));
    if (nullptr != pMultiBandSource)
        {
        pData = PackageRealityData::CreateTerrain(*ManagedToNativeMultiBandSource(pMultiBandSource));
        }
    else
        {
        pData = PackageRealityData::CreateTerrain(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(0)));
        }

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetIdentifier(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    Utf8String resolution;
    BeStringUtilities::WCharToUtf8(resolution, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetResolution()).ToPointer()));
    pData->SetResolution(resolution.c_str());

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(i));
        if (nullptr != pMultiBandSource)
            {
            pData->AddDataSource(*ManagedToNativeMultiBandSource(pMultiBandSource));
            }
        else
            {
            pData->AddDataSource(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(i)));
            }
        }

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PackageRealityDataPtr ManagedToNativeUndefinedData(UndefinedDataNet^ managedData)
    {
    PackageRealityDataPtr pData;

    // Create with main source.
    // In theory multiband is only useable for terrain and imagery but not forbidden for any type
    MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(0));
    if (nullptr != pMultiBandSource)
        {
        pData = PackageRealityData::CreateUndefined(*ManagedToNativeMultiBandSource(pMultiBandSource));
        }
    else
        {
        pData = PackageRealityData::CreateUndefined(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(0)));
        }

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetIdentifier(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    Utf8String resolution;
    BeStringUtilities::WCharToUtf8(resolution, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetResolution()).ToPointer()));
    pData->SetResolution(resolution.c_str());

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        // In theory multiband is only useable for terrain and imagery but not forbidden for any type
        MultiBandSourceNet^ pMultiBandSource = dynamic_cast<MultiBandSourceNet^>(managedData->GetSource(i));
        if (nullptr != pMultiBandSource)
            {
            pData->AddDataSource(*ManagedToNativeMultiBandSource(pMultiBandSource));
            }
        else
            {
            pData->AddDataSource(*ManagedToNativeSpatialEntityDataSource3(managedData->GetSource(i)));
            }
        }

    return pData;
    }
//=======================================================================================
//                                      Package
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataPackageNet^ RealityDataPackageNet::Create(String^ name)
    {
    return gcnew RealityDataPackageNet(name);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
bool RealityDataPackageNet::Write(String^ filename)
    {
    Utf8String filenameUtf8;
    BeStringUtilities::WCharToUtf8(filenameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(filename).ToPointer()));
    BeFileName nativeFilename(filenameUtf8.c_str());

    RealityPackageStatus status = RealityPackageStatus::UnknownError;
    status = (*m_pPackage)->Write(nativeFilename);

    if (RealityPackageStatus::Success != status)
        return false;

    return true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetOrigin()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetOrigin().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetOrigin(String^ origin)
    {
    Utf8String originUtf8;
    BeStringUtilities::WCharToUtf8(originUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(origin).ToPointer()));

    (*m_pPackage)->SetOrigin(originUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetRequestingApplication()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetRequestingApplication().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetRequestingApplication(String^ requestingApplication)
    {
    Utf8String requestingApplicationUtf8;
    BeStringUtilities::WCharToUtf8(requestingApplicationUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(requestingApplication).ToPointer()));

    (*m_pPackage)->SetRequestingApplication(requestingApplicationUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetName()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetName().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetName(String^ name)
    {
    Utf8String nameUtf8;
    BeStringUtilities::WCharToUtf8(nameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));

    (*m_pPackage)->SetName(nameUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetDescription()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetDescription().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetDescription(String^ description)
    {
    Utf8String descriptionUtf8;
    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));

    (*m_pPackage)->SetDescription(descriptionUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetCopyright()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetCopyright().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetCopyright(String^ copyright)
    {
    Utf8String copyrightUtf8;
    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));

    (*m_pPackage)->SetCopyright(copyrightUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetId()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetId().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetId(String^ id)
    {
    Utf8String idUtf8;
    BeStringUtilities::WCharToUtf8(idUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(id).ToPointer()));

    (*m_pPackage)->SetId(idUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    12/2016
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetCreationDate()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetCreationDate().ToString().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetCreationDate(String^ date)
    {
    Utf8String dateUtf8;
    BeStringUtilities::WCharToUtf8(dateUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(date).ToPointer()));

    BentleyApi::DateTime dateTime;
    BentleyApi::DateTime::FromString(dateTime, dateUtf8.c_str());

    (*m_pPackage)->SetCreationDate(dateTime);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
List<double>^ RealityDataPackageNet::GetBoundingPolygon()
    {
    List<double>^ polygonPts;

    BoundingPolygonCR polygon = (*m_pPackage)->GetBoundingPolygon();
    for (int i = 0; i < polygon.GetPointCount(); ++i)
        {
        polygonPts->Add(polygon.GetPointCP()[i].longitude);
        polygonPts->Add(polygon.GetPointCP()[i].latitude);
        }
    
    return polygonPts;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetBoundingPolygon(List<double>^ polygonPts)
    {
    GeoPoint2d pts[4];
    pts[0].longitude = polygonPts[0]; pts[0].latitude = polygonPts[1];
    pts[1].longitude = polygonPts[2]; pts[1].latitude = polygonPts[3];
    pts[2].longitude = polygonPts[4]; pts[2].latitude = polygonPts[5];
    pts[3].longitude = polygonPts[6]; pts[3].latitude = polygonPts[7];

    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(pts, 4);
    (*m_pPackage)->SetBoundingPolygon(*pBoundingPolygon);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain Robert         	    9/2018
//-------------------------------------------------------------------------------------
String^ RealityDataPackageNet::GetContext()
{
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pPackage)->GetContext().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain Robert         	    9/2018
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetContext(String^ context)
{
    Utf8String contextUtf8;
    BeStringUtilities::WCharToUtf8(contextUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(context).ToPointer()));

    (*m_pPackage)->SetContext(contextUtf8.c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::AddImageryData(ImageryDataNet^ data)
    {
    (*m_pPackage)->GetImageryGroupR().push_back(ManagedToNativeImageryData(data));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::AddModelData(ModelDataNet^ data)
    {
    (*m_pPackage)->GetModelGroupR().push_back(ManagedToNativeModelData(data));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::AddPinnedData(PinnedDataNet^ data)
    {
    (*m_pPackage)->GetPinnedGroupR().push_back(ManagedToNativePinnedData(data));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::AddTerrainData(TerrainDataNet^ data)
    {
    (*m_pPackage)->GetTerrainGroupR().push_back(ManagedToNativeTerrainData(data));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert          	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::AddUndefinedData(UndefinedDataNet^ data)
    {
    (*m_pPackage)->GetUndefinedGroupR().push_back(ManagedToNativeUndefinedData(data));
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
int RealityDataPackageNet::GetMajorVersion()
    {
    return (*m_pPackage)->GetMajorVersion();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetMajorVersion(int major)
    {
    (*m_pPackage)->SetMajorVersion(major);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
int RealityDataPackageNet::GetMinorVersion()
    {
    return (*m_pPackage)->GetMinorVersion();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetMinorVersion(int minor)
    {
    (*m_pPackage)->SetMinorVersion(minor);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataPackageNet::RealityDataPackageNet(String^ name)
    {
    Utf8String nameUtf8;
    BeStringUtilities::WCharToUtf8(nameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));

    m_pPackage = new RealityDataPackagePtr(RealityDataPackage::Create(nameUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataPackageNet::~RealityDataPackageNet()
    {
    this->!RealityDataPackageNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataPackageNet::!RealityDataPackageNet()
    {
    if (0 != m_pPackage)
        {
        delete m_pPackage;
        m_pPackage = 0;
        }
    }


//=======================================================================================
//                                    Reality Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataNet::GetDataId()
    {
    return m_id;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataNet::SetDataId(String^ dataId)
    {
    m_id = dataId;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
String^ RealityDataNet::GetDataName()
    {
    return m_name;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataNet::SetDataName(String^ dataName)
    {
    m_name = dataName;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataNet::GetDataset()
    {
    return m_dataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataNet::SetDataset(String^ dataset)
    {
    m_dataset = dataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                    11/2017
//-------------------------------------------------------------------------------------
String^ RealityDataNet::GetResolution()
    {
    return m_resolution;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                    11/2017
//-------------------------------------------------------------------------------------
void RealityDataNet::SetResolution(String^ resolution)
    {
    m_resolution = resolution;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
int RealityDataNet::GetNumSources()
    {
    return m_sources->Count;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet^ RealityDataNet::GetSource(int index)
    {
    return m_sources[index];
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataNet::AddSource(SpatialEntityDataSourceNet^ dataSource)
    {
    m_sources->Add(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataNet::RealityDataNet(SpatialEntityDataSourceNet^ dataSource)
    {
    m_sources = gcnew List<SpatialEntityDataSourceNet^>();
    m_sources->Add(dataSource);
    }


//=======================================================================================
//                                    Imagery Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet^ ImageryDataNet::Create(SpatialEntityDataSourceNet^ dataSource, List<double>^ corners)
    {
    return gcnew ImageryDataNet(dataSource, corners);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
List<double>^ ImageryDataNet::GetCornersCP()
    {
    List<double>^ managedCorners = gcnew List<double>();

    bvector<GeoPoint2d> pNativeCorners = (*m_pImageryData)->GetFootprint();
    if (!pNativeCorners.empty())
        {
        for (size_t i = 0; i < 4; ++i)
            {
            managedCorners->Add(pNativeCorners[i].longitude);
            managedCorners->Add(pNativeCorners[i].latitude);
            }
        }

    return managedCorners;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void ImageryDataNet::SetCorners(List<double>^ corners)
    {
    BeAssert(8 == corners->Count);

    bvector<GeoPoint2d> cornerPts = bvector<GeoPoint2d>(4);

    int j = 0;
    for (int i = 0; i < 4; ++i)
        {
        cornerPts[i].longitude = corners[j++];
        cornerPts[i].latitude = corners[j++];
        }

    (*m_pImageryData)->SetFootprint(cornerPts);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet::ImageryDataNet(SpatialEntityDataSourceNet^ dataSource, List<double>^ corners)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    SpatialEntityDataSourcePtr pNativeSource = SpatialEntityDataSource::Create("", "");


    // Managed to native corners.
    bvector<GeoPoint2d> cornerPts = bvector<GeoPoint2d>(4);
    if (0 != corners->Count)
        {
        BeAssert(8 == corners->Count);        

        int j = 0;
        for (int i = 0; i < 4; ++i)
            {
            cornerPts[i].longitude = corners[j++];
            cornerPts[i].latitude = corners[j++];
            }
        }

    m_pImageryData = new PackageRealityDataPtr(PackageRealityData::CreateImagery(*pNativeSource, cornerPts));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet::~ImageryDataNet()
    {
    this->!ImageryDataNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet::!ImageryDataNet()
    {
    if (0 != m_pImageryData)
        {
        delete m_pImageryData;
        m_pImageryData = 0;
        }
    }
        

//=======================================================================================
//                                    Model Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ModelDataNet^ ModelDataNet::Create(SpatialEntityDataSourceNet^ dataSource)
    {
    return gcnew ModelDataNet(dataSource);
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ModelDataNet::ModelDataNet(SpatialEntityDataSourceNet^ dataSource)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    SpatialEntityDataSourcePtr pNativeSource = SpatialEntityDataSource::Create("", "");

    m_pModelData = new PackageRealityDataPtr(PackageRealityData::CreateModel(*pNativeSource));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ModelDataNet::~ModelDataNet()
    {
    this->!ModelDataNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ModelDataNet::!ModelDataNet()
    {
    if (0 != m_pModelData)
        {
        delete m_pModelData;
        m_pModelData = 0;
        }
    }


//=======================================================================================
//                                    Pinned Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
PinnedDataNet^ PinnedDataNet::Create(SpatialEntityDataSourceNet^ dataSource, double longitude, double latitude)
    {
    return gcnew PinnedDataNet(dataSource, longitude, latitude);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<double>^ PinnedDataNet::GetLocation()
    {
    List<double>^ location = gcnew List<double>();

    GeoPoint2dCR nativeLocation = (*m_pPinnedData)->GetLocation();

    location->Add(nativeLocation.longitude);
    location->Add(nativeLocation.latitude);

    return location;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool PinnedDataNet::SetLocation(double longitude, double latitude)
    {
    GeoPoint2d location;
    location.longitude = longitude;
    location.latitude = latitude;

    return (*m_pPinnedData)->SetLocation(location);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool PinnedDataNet::HasArea()
    {
    return (*m_pPinnedData)->HasArea();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<double>^ PinnedDataNet::GetAreaCP()
    {
    List<double>^ polygonPts = gcnew List<double>();

    bvector<GeoPoint2d> polygon = (*m_pPinnedData)->GetFootprint();
    for (int i = 0; i < polygon.size(); ++i)
        {
        polygonPts->Add(polygon[i].longitude);
        polygonPts->Add(polygon[i].latitude);
        }

    return polygonPts;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool PinnedDataNet::SetArea(List<double>^ polygonPts)
    {
    bvector<GeoPoint2d> pts = bvector<GeoPoint2d>(4);

    int j = 0;
    for (int i = 0; i < polygonPts->Count; ++i)
        {
        pts[i].longitude = polygonPts[j++];
        pts[i].latitude = polygonPts[j++];
        }

    (*m_pPinnedData)->SetFootprint(pts);
    return pts.size() > 3;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
PinnedDataNet::PinnedDataNet(SpatialEntityDataSourceNet^ dataSource, double longitude, double latitude)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    SpatialEntityDataSourcePtr pNativeSource = SpatialEntityDataSource::Create("", "");

    m_pPinnedData = new PackageRealityDataPtr(PackageRealityData::CreatePinned(*pNativeSource, longitude, latitude));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
PinnedDataNet::~PinnedDataNet()
    {
    this->!PinnedDataNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
PinnedDataNet::!PinnedDataNet()
    {
    if (0 != m_pPinnedData)
        {
        delete m_pPinnedData;
        m_pPinnedData = 0;
        }
    }


//=======================================================================================
//                                 Terrain Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
TerrainDataNet^ TerrainDataNet::Create(SpatialEntityDataSourceNet^ dataSource)
    {
    return gcnew TerrainDataNet(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
TerrainDataNet::TerrainDataNet(SpatialEntityDataSourceNet^ dataSource)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    SpatialEntityDataSourcePtr pNativeSource = SpatialEntityDataSource::Create("", "");

    m_pTerrainData = new PackageRealityDataPtr(PackageRealityData::CreateTerrain(*pNativeSource));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
TerrainDataNet::~TerrainDataNet()
    {
    this->!TerrainDataNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
TerrainDataNet::!TerrainDataNet()
    {
    if (0 != m_pTerrainData)
        {
        delete m_pTerrainData;
        m_pTerrainData = 0;
        }
    }

//=======================================================================================
//                                 Undefined Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert         	    04/2017
//-------------------------------------------------------------------------------------
UndefinedDataNet^ UndefinedDataNet::Create(SpatialEntityDataSourceNet^ dataSource)
    {
    return gcnew UndefinedDataNet(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
UndefinedDataNet::UndefinedDataNet(SpatialEntityDataSourceNet^ dataSource)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    SpatialEntityDataSourcePtr pNativeSource = SpatialEntityDataSource::Create("", "");

    m_pUndefinedData = new PackageRealityDataPtr(PackageRealityData::CreateUndefined(*pNativeSource));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
UndefinedDataNet::~UndefinedDataNet()
    {
    this->!UndefinedDataNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
UndefinedDataNet::!UndefinedDataNet()
    {
    if (0 != m_pUndefinedData)
        {
        delete m_pUndefinedData;
        m_pUndefinedData = 0;
        }
    }
