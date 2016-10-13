/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/RealityDataPackageNet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "RealityDataPackageNet.h"
#include "RealityDataSourceNet.h"

#include <Bentley/BeFileName.h>


using namespace RealityPackage;
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

    return RealityPackage::Uri::Create(uriUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourcePtr ManagedToNativeRealityDataSource2(RealityDataSourceNet^ managedSource)
    {
    RealityPackage::UriPtr nativeUri = ManagedToNativeUri2(managedSource->GetUri());

    Utf8String nativeType;
    BeStringUtilities::WCharToUtf8(nativeType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetSourceType()).ToPointer()));

    // Create source with required parameters.
    RealityDataSourcePtr nativeSource = RealityDataSource::Create(*nativeUri, nativeType.c_str());

    // Id.
    Utf8String nativeId;
    BeStringUtilities::WCharToUtf8(nativeId, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetId()).ToPointer()));
    nativeSource->SetId(nativeId.c_str());

    // Copyright.
    Utf8String nativeCopyright;
    BeStringUtilities::WCharToUtf8(nativeCopyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetCopyright()).ToPointer()));
    nativeSource->SetCopyright(nativeCopyright.c_str());

    // Provider.
    Utf8String nativeProvider;
    BeStringUtilities::WCharToUtf8(nativeProvider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetProvider()).ToPointer()));
    nativeSource->SetProvider(nativeProvider.c_str());

    // Size.
    nativeSource->SetSize(managedSource->GetSize());

    // Metadata.
    Utf8String nativeMetadata;
    BeStringUtilities::WCharToUtf8(nativeMetadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadata()).ToPointer()));
    nativeSource->SetMetadata(nativeMetadata.c_str());

    // Metadata type.
    Utf8String nativeMetadataType;
    BeStringUtilities::WCharToUtf8(nativeMetadataType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadataType()).ToPointer()));
    nativeSource->SetMetadataType(nativeMetadataType.c_str());

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

    return nativeSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
ImageryDataPtr ManagedToNativeImageryData(ImageryDataNet^ managedData)
    {
    // Create with main source.
    ImageryDataPtr pData = ImageryData::Create(*ManagedToNativeRealityDataSource2(managedData->GetSource(0)), NULL);

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetDataId(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetDataName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    // Set corners.
    List<double>^ corners = managedData->GetCornersCP();
    if (0 != corners->Count)
        {
        DPoint2d cornerPts[4];

        int j = 0;
        for (int i = 0; i < 4; ++i)
            {
            cornerPts[i].x = corners[j++];
            cornerPts[i].y = corners[j++];
            }

        pData->SetCorners(cornerPts);
        }

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        pData->AddSource(*ManagedToNativeRealityDataSource2(managedData->GetSource(i)));
        }    

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
ModelDataPtr ManagedToNativeModelData(ModelDataNet^ managedData)
    {
    // Create with main source.
    ModelDataPtr pData = ModelData::Create(*ManagedToNativeRealityDataSource2(managedData->GetSource(0)));

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetDataId(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetDataName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        pData->AddSource(*ManagedToNativeRealityDataSource2(managedData->GetSource(i)));
        }

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
PinnedDataPtr ManagedToNativePinnedData(PinnedDataNet^ managedData)
    {
    // Create with main source.
    PinnedDataPtr pData = PinnedData::Create(*ManagedToNativeRealityDataSource2(managedData->GetSource(0)), 0, 0);

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetDataId(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetDataName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    // Set location.
    List<double>^ location = managedData->GetLocation();
    DPoint2d longlat;
    longlat.x = location[0];
    longlat.y = location[1];
    pData->SetLocation(longlat);

    // Set area.
    List<double>^ polygonPts = managedData->GetAreaCP();
    DPoint2d pts[4];
    int j = 0;
    for (int i = 0; i < polygonPts->Count; ++i)
        {
        pts[i].x = polygonPts[j++];
        pts[i].y = polygonPts[j++];
        }
    pData->SetArea(*BoundingPolygon::Create(pts, polygonPts->Count));

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        pData->AddSource(*ManagedToNativeRealityDataSource2(managedData->GetSource(i)));
        }

    return pData;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
TerrainDataPtr ManagedToNativeTerrainData(TerrainDataNet^ managedData)
    {
    // Create with main source.
    TerrainDataPtr pData = TerrainData::Create(*ManagedToNativeRealityDataSource2(managedData->GetSource(0)));

    // Set basic members.
    Utf8String id;
    BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataId()).ToPointer()));
    pData->SetDataId(id.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataName()).ToPointer()));
    pData->SetDataName(name.c_str());

    Utf8String dataset;
    BeStringUtilities::WCharToUtf8(dataset, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedData->GetDataset()).ToPointer()));
    pData->SetDataset(dataset.c_str());

    // Add alternate sources.
    for (int i = 1; i < managedData->GetNumSources(); ++i)
        {
        pData->AddSource(*ManagedToNativeRealityDataSource2(managedData->GetSource(i)));
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
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
List<double>^ RealityDataPackageNet::GetBoundingPolygon()
    {
    List<double>^ polygonPts;

    BoundingPolygonCR polygon = (*m_pPackage)->GetBoundingPolygon();
    for (int i = 0; i < polygon.GetPointCount(); ++i)
        {
        polygonPts->Add(polygon.GetPointCP()[i].x);
        polygonPts->Add(polygon.GetPointCP()[i].y);
        }
    
    return polygonPts;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataPackageNet::SetBoundingPolygon(List<double>^ polygonPts)
    {
    DPoint2d pts[4];
    pts[0].x = polygonPts[0]; pts[0].y = polygonPts[1];
    pts[1].x = polygonPts[2]; pts[1].y = polygonPts[3];
    pts[2].x = polygonPts[4]; pts[2].y = polygonPts[5];
    pts[3].x = polygonPts[6]; pts[3].y = polygonPts[7];

    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(pts, 4);
    (*m_pPackage)->SetBoundingPolygon(*pBoundingPolygon);
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
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
int RealityDataNet::GetNumSources()
    {
    return m_sources->Count;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ RealityDataNet::GetSource(int index)
    {
    return m_sources[index];
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
void RealityDataNet::AddSource(RealityDataSourceNet^ dataSource)
    {
    m_sources->Add(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
RealityDataNet::RealityDataNet(RealityDataSourceNet^ dataSource)
    {
    m_sources = gcnew List<RealityDataSourceNet^>();
    m_sources->Add(dataSource);
    }


//=======================================================================================
//                                    Imagery Data
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet^ ImageryDataNet::Create(RealityDataSourceNet^ dataSource, List<double>^ corners)
    {
    return gcnew ImageryDataNet(dataSource, corners);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
List<double>^ ImageryDataNet::GetCornersCP()
    {
    List<double>^ managedCorners = gcnew List<double>();

    DPoint2dCP pNativeCorners = (*m_pImageryData)->GetCornersCP();
    if (pNativeCorners != NULL)
        {
        for (size_t i = 0; i < 4; ++i)
            {
            managedCorners->Add(pNativeCorners[i].x);
            managedCorners->Add(pNativeCorners[i].y);
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

    DPoint2d cornerPts[4];

    int j = 0;
    for (int i = 0; i < 4; ++i)
        {
        cornerPts[i].x = corners[j++];
        cornerPts[i].y = corners[j++];
        }

    (*m_pImageryData)->SetCorners(cornerPts);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ImageryDataNet::ImageryDataNet(RealityDataSourceNet^ dataSource, List<double>^ corners)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    RealityDataSourcePtr pNativeSource = RealityDataSource::Create("", "");


    // Managed to native corners.
    DPoint2d cornerPts[4];
    if (0 != corners->Count)
        {
        BeAssert(8 == corners->Count);        

        int j = 0;
        for (int i = 0; i < 4; ++i)
            {
            cornerPts[i].x = corners[j++];
            cornerPts[i].y = corners[j++];
            }
        }

    m_pImageryData = new ImageryDataPtr(ImageryData::Create(*pNativeSource, cornerPts));
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
ModelDataNet^ ModelDataNet::Create(RealityDataSourceNet^ dataSource)
    {
    return gcnew ModelDataNet(dataSource);
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
ModelDataNet::ModelDataNet(RealityDataSourceNet^ dataSource)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    RealityDataSourcePtr pNativeSource = RealityDataSource::Create("", "");

    m_pModelData = new ModelDataPtr(ModelData::Create(*pNativeSource));
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
PinnedDataNet^ PinnedDataNet::Create(RealityDataSourceNet^ dataSource, double longitude, double latitude)
    {
    return gcnew PinnedDataNet(dataSource, longitude, latitude);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<double>^ PinnedDataNet::GetLocation()
    {
    List<double>^ location = gcnew List<double>();

    DPoint2dCR nativeLocation = (*m_pPinnedData)->GetLocation();

    location->Add(nativeLocation.x);
    location->Add(nativeLocation.y);

    return location;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool PinnedDataNet::SetLocation(double longitude, double latitude)
    {
    DPoint2d location;
    location.x = longitude;
    location.y = latitude;

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

    BoundingPolygonCP polygon = (*m_pPinnedData)->GetAreaCP();
    for (int i = 0; i < polygon->GetPointCount(); ++i)
        {
        polygonPts->Add(polygon->GetPointCP()[i].x);
        polygonPts->Add(polygon->GetPointCP()[i].y);
        }

    return polygonPts;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool PinnedDataNet::SetArea(List<double>^ polygonPts)
    {
    DPoint2d pts[4];

    int j = 0;
    for (int i = 0; i < polygonPts->Count; ++i)
        {
        pts[i].x = polygonPts[j++];
        pts[i].y = polygonPts[j++];
        }

    BoundingPolygonPtr pBoundingPolygon = BoundingPolygon::Create(pts, polygonPts->Count);
    return (*m_pPinnedData)->SetArea(*pBoundingPolygon);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
PinnedDataNet::PinnedDataNet(RealityDataSourceNet^ dataSource, double longitude, double latitude)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    RealityDataSourcePtr pNativeSource = RealityDataSource::Create("", "");

    m_pPinnedData = new PinnedDataPtr(PinnedData::Create(*pNativeSource, longitude, latitude));
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
TerrainDataNet^ TerrainDataNet::Create(RealityDataSourceNet^ dataSource)
    {
    return gcnew TerrainDataNet(dataSource);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    9/2016
//-------------------------------------------------------------------------------------
TerrainDataNet::TerrainDataNet(RealityDataSourceNet^ dataSource)
    : RealityDataNet(dataSource)
    {
    // Managed to native reality data source.
    RealityDataSourcePtr pNativeSource = RealityDataSource::Create("", "");

    m_pTerrainData = new TerrainDataPtr(TerrainData::Create(*pNativeSource));
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















//#include <RealityPackage/WMSSource.h>
//#include <RealityPackage/OsmSource.h>

//using namespace RealityDataPackageWrapper;
//using namespace System;
//using namespace System::Runtime::InteropServices;
//
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//void RealityDataPackageNet::CreateV1(String^  location,
//                                     String^  name,
//                                     String^  description,
//                                     String^  copyright,
//                                     String^  packageId,
//                                     List<double>^    regionOfInterest,
//                                     ImageryGroupNet^ imageryGroup,
//                                     ModelGroupNet^   modelGroup,
//                                     PinnedGroupNet^  pinnedGroup,
//                                     TerrainGroupNet^ terrainGroup)
//    {ddddddddddddd
//    // Construct package location.
//    WString fullPath;
//    WCharCP locationStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(location).ToPointer());
//    WCharCP nameStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer());
//    fullPath += locationStr;
//    fullPath += nameStr;
//    fullPath.AppendUtf8(".xrdp");
//    BeFileName packageFullPath(fullPath.c_str());
//
//    // Create package.
//    Utf8String nameUtf8;
//    BeStringUtilities::WCharToUtf8(nameUtf8, nameStr);
//    RealityPackage::RealityDataPackagePtr pDataPackage = RealityDataPackage::Create(nameUtf8.c_str());
//
//    Utf8String descriptionUtf8;
//    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));
//    pDataPackage->SetDescription(descriptionUtf8.c_str());
//
//    Utf8String copyrightUtf8;
//    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));
//    pDataPackage->SetCopyright(copyrightUtf8.c_str());
//
//    // Create bounding polygon and it to the package.
//    DPoint2d pts[4];
//    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
//    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
//    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
//    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
//    RealityPackage::BoundingPolygonPtr boundingPolygon = RealityPackage::BoundingPolygon::Create(pts, 4);
//    pDataPackage->SetBoundingPolygon(*boundingPolygon);
//
//    // Create imagery data sources and add them to the package.
//    List<DataSourceNet^>^ imgSources = imageryGroup->GetData()[0]->GetSources();
//    for each(DataSourceNet^ source in imgSources)
//        {
//        if ("wms" == source->GetSourceType())
//            {
//            WmsSourceNet^ wmsSourceNet = dynamic_cast<WmsSourceNet^>(source);
//
//            // Create source with required parameters.
//            Utf8String uri;
//            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetUri()).ToPointer()));
//            RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityPackage::WmsDataSource::Create(uri.c_str());
//
//            // Add optional parameters.
//            if (!String::IsNullOrEmpty(wmsSourceNet->GetCopyright()))
//                {
//                Utf8String copyright;
//                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetCopyright()).ToPointer()));
//                pWmsDataSource->SetCopyright(copyright.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(source->GetId()))
//                {
//                Utf8String id;
//                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetId()).ToPointer()));
//                pWmsDataSource->SetId(id.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(wmsSourceNet->GetProvider()))
//                {
//                Utf8String provider;
//                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetProvider()).ToPointer()));
//                pWmsDataSource->SetProvider(provider.c_str());
//                }
//
//            if (0 != wmsSourceNet->GetSize())
//                {
//                uint64_t size = wmsSourceNet->GetSize();
//                pWmsDataSource->SetSize(size);
//                }
//
//            if (!String::IsNullOrEmpty(wmsSourceNet->GetMetadata()))
//                {
//                Utf8String metadata;
//                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetMetadata()).ToPointer()));
//                pWmsDataSource->SetMetadata(metadata.c_str());
//                }
//
//            if (0 != wmsSourceNet->GetSisterFiles()->Count)
//                {
//                //List<String^>^ sisterFilesNet = wmsSourceNet->GetSisterFiles();
//
//                bvector<Utf8String> sisterFiles;
//                pWmsDataSource->SetSisterFiles(sisterFiles);
//                }
//
//            // Xml Fragment.
//            Utf8String xmlFragment;
//            BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetXmlFragment()).ToPointer()));
//            pWmsDataSource->SetMapSettings(xmlFragment.c_str());
//
//            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
//            pDataPackage->GetImageryGroupR().push_back(pImageryData);
//            }
//        else
//            {
//            // Create source with required parameters.
//            Utf8String uri;
//            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));
//
//            Utf8String type;
//            BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());
//
//            // Add optional parameters.
//            if (!String::IsNullOrEmpty(source->GetCopyright()))
//                {
//                Utf8String copyright;
//                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
//                pDataSource->SetCopyright(copyright.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(source->GetId()))
//                {
//                Utf8String id;
//                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
//                pDataSource->SetId(id.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(source->GetProvider()))
//                {
//                Utf8String provider;
//                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
//                pDataSource->SetProvider(provider.c_str());
//                }
//
//            if (0 != source->GetSize())
//                {
//                uint64_t size = source->GetSize();
//                pDataSource->SetSize(size);
//                }
//
//            if (!String::IsNullOrEmpty(source->GetMetadata()))
//                {
//                Utf8String metadata;
//                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
//                pDataSource->SetMetadata(metadata.c_str());
//                }
//
//            if (0 != source->GetSisterFiles()->Count)
//                {
//                //List<String^>^ sisterFilesNet = source->GetSisterFiles();
//
//                bvector<Utf8String> sisterFiles;
//                pDataSource->SetSisterFiles(sisterFiles);
//                }
//
//            RealityPackage::ImageryDataPtr pImageryData = RealityPackage::ImageryData::Create(*pDataSource, NULL);
//            pDataPackage->GetImageryGroupR().push_back(pImageryData);
//            }
//        }
//
//    // Create model data sources and add them to the package.
//    List<DataSourceNet^>^ modelSources = modelGroup->GetData()[0]->GetSources();
//    for each(DataSourceNet^ source in modelSources)
//        {
//        if ("osm" == source->GetSourceType())
//            {
//            OsmSourceNet^ osmSourceNet = dynamic_cast<OsmSourceNet^>(source);
//
//            // Find min/max for bbox.
//            double minX = DBL_MAX;
//            double minY = DBL_MAX;
//            double maxX = -DBL_MAX;
//            double maxY = -DBL_MAX;
//
//            DPoint2dCP bboxPts = boundingPolygon->GetPointCP();
//            for (size_t i = 0; i < boundingPolygon->GetPointCount(); ++i)
//                {
//                if (bboxPts[i].x < minX)
//                    minX = bboxPts[i].x;
//                
//                if (bboxPts[i].x > maxX)
//                    maxX = bboxPts[i].x;
//
//                if (bboxPts[i].y < minY)
//                    minY = bboxPts[i].y;
//
//                if (bboxPts[i].y > maxY)
//                    maxY = bboxPts[i].y;
//                }
//
//            DRange2d bbox;
//            bbox.InitFrom(minX, minY, maxX, maxY);
//            
//            // Create source with required parameters.
//            Utf8String uri;
//            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetUri()).ToPointer()));
//            RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityPackage::OsmDataSource::Create(uri.c_str(), &bbox);
//
//            // Add optional parameters.
//            if (!String::IsNullOrEmpty(osmSourceNet->GetCopyright()))
//                {
//                Utf8String copyright;
//                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetCopyright()).ToPointer()));
//                pOsmDataSource->SetCopyright(copyright.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(osmSourceNet->GetId()))
//                {
//                Utf8String id;
//                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetId()).ToPointer()));
//                pOsmDataSource->SetId(id.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(osmSourceNet->GetProvider()))
//                {
//                Utf8String provider;
//                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetProvider()).ToPointer()));
//                pOsmDataSource->SetProvider(provider.c_str());
//                }
//
//            if (0 != osmSourceNet->GetSize())
//                {
//                uint64_t size = osmSourceNet->GetSize();
//                pOsmDataSource->SetSize(size);
//                }
//
//            if (!String::IsNullOrEmpty(osmSourceNet->GetMetadata()))
//                {
//                Utf8String metadata;
//                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetMetadata()).ToPointer()));
//                pOsmDataSource->SetMetadata(metadata.c_str());
//                }
//
//            if (0 != osmSourceNet->GetSisterFiles()->Count)
//                {
//                //List<String^>^ sisterFilesNet = osmSourceNet->GetSisterFiles();
//
//                bvector<Utf8String> sisterFiles;
//                pOsmDataSource->SetSisterFiles(sisterFiles);
//                }
//
//            // Xml Fragment.
//            Utf8String xmlFragment;
//            BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetXmlFragment()).ToPointer()));
//            pOsmDataSource->SetOsmResource(xmlFragment.c_str());
//
//            RealityPackage::ModelDataPtr pModelData = RealityPackage::ModelData::Create(*pOsmDataSource);
//            pDataPackage->GetModelGroupR().push_back(pModelData);
//            }
//        else
//            {
//            // Create source with required parameters.
//            Utf8String uri;
//            BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));
//
//            Utf8String type;
//            BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
//
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());
//
//            // Add optional parameters.
//            if (!String::IsNullOrEmpty(source->GetCopyright()))
//                {
//                Utf8String copyright;
//                BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
//                pDataSource->SetCopyright(copyright.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(source->GetId()))
//                {
//                Utf8String id;
//                BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
//                pDataSource->SetId(id.c_str());
//                }
//
//            if (!String::IsNullOrEmpty(source->GetProvider()))
//                {
//                Utf8String provider;
//                BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
//                pDataSource->SetProvider(provider.c_str());
//                }
//
//            if (0 != source->GetSize())
//                {
//                uint64_t size = source->GetSize();
//                pDataSource->SetSize(size);
//                }
//
//            if (!String::IsNullOrEmpty(source->GetMetadata()))
//                {
//                Utf8String metadata;
//                BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
//                pDataSource->SetMetadata(metadata.c_str());
//                }
//
//            if (0 != source->GetSisterFiles()->Count)
//                {
//                //List<String^>^ sisterFilesNet = source->GetSisterFiles();
//
//                bvector<Utf8String> sisterFiles;
//                pDataSource->SetSisterFiles(sisterFiles);
//                }
//
//            RealityPackage::ModelDataPtr pModelData = RealityPackage::ModelData::Create(*pDataSource);
//            pDataPackage->GetModelGroupR().push_back(pModelData);
//            }
//        }
//
//    // Create pinned data sources and add them to the package.
//    List<DataSourceNet^>^ pinnedSources = pinnedGroup->GetData()[0]->GetSources();
//    for each(DataSourceNet^ source in pinnedSources)
//        {
//        // Create source with required parameters.
//        Utf8String uri;
//        BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));
//
//        Utf8String type;
//        BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
//
//        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());
//
//        // Add optional parameters.
//        if (!String::IsNullOrEmpty(source->GetCopyright()))
//            {
//            Utf8String copyright;
//            BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
//            pDataSource->SetCopyright(copyright.c_str());
//            }
//
//        if (!String::IsNullOrEmpty(source->GetId()))
//            {
//            Utf8String id;
//            BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
//            pDataSource->SetId(id.c_str());
//            }
//
//        if (!String::IsNullOrEmpty(source->GetProvider()))
//            {
//            Utf8String provider;
//            BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
//            pDataSource->SetProvider(provider.c_str());
//            }
//
//        if (0 != source->GetSize())
//            {
//            uint64_t size = source->GetSize();
//            pDataSource->SetSize(size);
//            }
//
//        if (!String::IsNullOrEmpty(source->GetMetadata()))
//            {
//            Utf8String metadata;
//            BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
//            pDataSource->SetMetadata(metadata.c_str());
//            }
//
//        if (0 != source->GetSisterFiles()->Count)
//            {
//            //List<String^>^ sisterFilesNet = source->GetSisterFiles();
//
//            bvector<Utf8String> sisterFiles;
//            pDataSource->SetSisterFiles(sisterFiles);
//            }
//
//        //&&JFC Implement pinned data longitude and latitude for the wrapper.
//        RealityPackage::PinnedDataPtr pPinnedData = RealityPackage::PinnedData::Create(*pDataSource, 0.0, 0.0);
//        pDataPackage->GetPinnedGroupR().push_back(pPinnedData);
//        }
//
//    // Create terrain data sources and add them to the package.
//    List<DataSourceNet^>^ terrainSources = terrainGroup->GetData()[0]->GetSources();
//    for each(DataSourceNet^ source in terrainSources)
//        {
//        // Create source with required parameters.
//        Utf8String uri;
//        BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));
//
//        Utf8String type;
//        BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
//
//        RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());
//
//        // Add optional parameters.
//        if (!String::IsNullOrEmpty(source->GetCopyright()))
//            {
//            Utf8String copyright;
//            BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
//            pDataSource->SetCopyright(copyright.c_str());
//            }
//
//        if (!String::IsNullOrEmpty(source->GetId()))
//            {
//            Utf8String id;
//            BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
//            pDataSource->SetId(id.c_str());
//            }
//
//        if (!String::IsNullOrEmpty(source->GetProvider()))
//            {
//            Utf8String provider;
//            BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
//            pDataSource->SetProvider(provider.c_str());
//            }
//
//        if (0 != source->GetSize())
//            {
//            uint64_t size = source->GetSize();
//            pDataSource->SetSize(size);
//            }
//
//        if (!String::IsNullOrEmpty(source->GetMetadata()))
//            {
//            Utf8String metadata;
//            BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
//            pDataSource->SetMetadata(metadata.c_str());
//            }
//
//        if (0 != source->GetSisterFiles()->Count)
//            {
//            //List<String^>^ sisterFilesNet = source->GetSisterFiles();
//
//            bvector<Utf8String> sisterFiles;
//            pDataSource->SetSisterFiles(sisterFiles);
//            }
//
//        RealityPackage::TerrainDataPtr pTerrainData = RealityPackage::TerrainData::Create(*pDataSource);
//        pDataPackage->GetTerrainGroupR().push_back(pTerrainData);
//        }
//
//    pDataPackage->Write(packageFullPath);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 6/2015
////-------------------------------------------------------------------------------------
//void RealityDataPackageNet::CreateV2(String^ location,
//                                     String^ origin,
//                                     String^ name,
//                                     String^ description,
//                                     String^ copyright,
//                                     String^ id,
//                                     List<double>^    regionOfInterest,
//                                     ImageryGroupNet^ imageryGroup,
//                                     ModelGroupNet^   modelGroup,
//                                     PinnedGroupNet^  pinnedGroup,
//                                     TerrainGroupNet^ terrainGroup)
//    {
//    // Construct package location.
//    WString fullPath;
//    WCharCP locationStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(location).ToPointer());
//    WCharCP nameStr = static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer());
//    fullPath += locationStr;
//    fullPath += nameStr;
//    fullPath.AppendUtf8(".xrdp");
//    BeFileName packageFullPath(fullPath.c_str());
//
//    // Create package.
//    Utf8String nameUtf8;
//    BeStringUtilities::WCharToUtf8(nameUtf8, nameStr);
//    RealityPackage::RealityDataPackagePtr pDataPackage = RealityDataPackage::Create(nameUtf8.c_str());
//
//    Utf8String originUtf8;
//    BeStringUtilities::WCharToUtf8(originUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(origin).ToPointer()));
//    pDataPackage->SetOrigin(originUtf8.c_str());
//
//    Utf8String descriptionUtf8;
//    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));
//    pDataPackage->SetDescription(descriptionUtf8.c_str());
//
//    Utf8String copyrightUtf8;
//    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));
//    pDataPackage->SetCopyright(copyrightUtf8.c_str());
//
//    Utf8String idUtf8;
//    BeStringUtilities::WCharToUtf8(idUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(id).ToPointer()));
//    pDataPackage->SetId(idUtf8.c_str());
//
//    // Create bounding polygon and add it to the package.
//    DPoint2d pts[4];
//    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
//    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
//    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
//    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
//    RealityPackage::BoundingPolygonPtr boundingPolygon = RealityPackage::BoundingPolygon::Create(pts, 4);
//    pDataPackage->SetBoundingPolygon(*boundingPolygon);
//
//    // Create imagery data sources and add them to the package.
//    for each(DataGroupNet^ dataGroup in imageryGroup->GetData())
//        {
//        if (0 == dataGroup->GetNumSources())
//            continue;       
//
//        // Create data group and add primary source.
//        RealityPackage::ImageryDataPtr pImageryData;
//        DataSourceNet^ source = dataGroup->GetSources()[0];
//        if ("wms" == source->GetSourceType())
//            {
//            RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityDataPackageNet::CreateWmsDataSource(source);
//            pImageryData = RealityPackage::ImageryData::Create(*pWmsDataSource, NULL);
//            }
//        else
//            {
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//            pImageryData = RealityPackage::ImageryData::Create(*pDataSource, NULL);
//            }         
//
//        // Add alternate sources to group.
//        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
//            {
//            DataSourceNet^ source = dataGroup->GetSources()[i];
//            if ("wms" == source->GetSourceType())
//                {
//                RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityDataPackageNet::CreateWmsDataSource(source);
//                pImageryData->AddSource(*pWmsDataSource);
//                }
//            else
//                {
//                RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);                
//                pImageryData->AddSource(*pDataSource);
//                }                
//            }
//
//        pDataPackage->GetImageryGroupR().push_back(pImageryData);
//        }
//
//    // Create model data sources and add them to the package.
//    for each(DataGroupNet^ dataGroup in modelGroup->GetData())
//        {
//        if (0 == dataGroup->GetNumSources())
//            continue;
//
//        // Create data group and add primary source.
//        RealityPackage::ModelDataPtr pModelData;
//        DataSourceNet^ source = dataGroup->GetSources()[0];
//        if ("osm" == source->GetSourceType())
//            {
//            RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityDataPackageNet::CreateOsmDataSource(source, boundingPolygon);
//            pModelData = RealityPackage::ModelData::Create(*pOsmDataSource);
//            }
//        else
//            {
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//            pModelData = RealityPackage::ModelData::Create(*pDataSource);
//            }
//
//        // Add alternate sources to group.
//        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
//            {
//            DataSourceNet^ source = dataGroup->GetSources()[i];
//            if ("osm" == source->GetSourceType())
//                {
//                RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityDataPackageNet::CreateOsmDataSource(source, boundingPolygon);
//                pModelData->AddSource(*pOsmDataSource);
//                }
//            else
//                {
//                RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//                pModelData->AddSource(*pDataSource);
//                }
//            }
//
//        pDataPackage->GetModelGroupR().push_back(pModelData);
//        }
//
//    // Create pinned data sources and add them to the package.
//    for each(DataGroupNet^ dataGroup in pinnedGroup->GetData())
//        {
//        if (0 == dataGroup->GetNumSources())
//            continue;
//
//        // Create data group and add primary source.
//        RealityPackage::PinnedDataPtr pPinnedData;
//        DataSourceNet^ source = dataGroup->GetSources()[0];
//        RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//        pPinnedData = RealityPackage::PinnedData::Create(*pDataSource, 0.0, 0.0); //&&JFC Implement pinned data longitude and latitude for the wrapper.
//
//        // Add alternate sources to group.
//        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
//            {
//            DataSourceNet^ source = dataGroup->GetSources()[i];
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//            pPinnedData->AddSource(*pDataSource);
//            }
//
//        pDataPackage->GetPinnedGroupR().push_back(pPinnedData);
//        }
//
//    // Create terrain data sources and add them to the package.
//    for each(DataGroupNet^ dataGroup in terrainGroup->GetData())
//        {
//        if (0 == dataGroup->GetNumSources())
//            continue;
//
//        // Create data group and add primary source.
//        RealityPackage::TerrainDataPtr pTerrainData;
//        DataSourceNet^ source = dataGroup->GetSources()[0];
//        RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//        pTerrainData = RealityPackage::TerrainData::Create(*pDataSource);
//
//        // Add alternate sources to group.
//        for (int i = 1; i < dataGroup->GetNumSources(); ++i)
//            {
//            DataSourceNet^ source = dataGroup->GetSources()[i];
//            RealityPackage::RealityDataSourcePtr pDataSource = RealityDataPackageNet::CreateDataSource(source);
//            pTerrainData->AddSource(*pDataSource);
//            }
//
//        pDataPackage->GetTerrainGroupR().push_back(pTerrainData);
//        }
//
//    pDataPackage->Write(packageFullPath);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 6/2015
////-------------------------------------------------------------------------------------
//RealityDataPackageNet::RealityDataPackageNet()  {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 6/2015
////-------------------------------------------------------------------------------------
//RealityDataPackageNet::~RealityDataPackageNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//RealityPackage::RealityDataSourcePtr RealityDataPackageNet::CreateDataSource(DataSourceNet^ source)
//    {
//    // Create source with required parameters.
//    Utf8String uri;
//    BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetUri()).ToPointer()));
//
//    Utf8String type;
//    BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetSourceType()).ToPointer()));
//    RealityPackage::RealityDataSourcePtr pDataSource = RealityPackage::RealityDataSource::Create(uri.c_str(), type.c_str());
//
//    // Add optional parameters.
//    if (!String::IsNullOrEmpty(source->GetCopyright()))
//        {
//        Utf8String copyright;
//        BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetCopyright()).ToPointer()));
//        pDataSource->SetCopyright(copyright.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(source->GetId()))
//        {
//        Utf8String id;
//        BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetId()).ToPointer()));
//        pDataSource->SetId(id.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(source->GetProvider()))
//        {
//        Utf8String provider;
//        BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetProvider()).ToPointer()));
//        pDataSource->SetProvider(provider.c_str());
//        }
//
//    if (0 != source->GetSize())
//        {
//        uint64_t filesize = source->GetSize();
//        pDataSource->SetSize(filesize);
//        }
//
//    if (!String::IsNullOrEmpty(source->GetMetadata()))
//        {
//        Utf8String metadata;
//        BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetMetadata()).ToPointer()));
//        pDataSource->SetMetadata(metadata.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(source->GetGeoCS()))
//        {
//        Utf8String geocs;
//        BeStringUtilities::WCharToUtf8(geocs, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source->GetGeoCS()).ToPointer()));
//        pDataSource->SetGeoCS(geocs.c_str());
//        }
//
//    if (0 != source->GetSisterFiles()->Count)
//        {
//        //List<String^>^ sisterFilesNet = source->GetSisterFiles();
//
//        bvector<Utf8String> sisterFiles;
//        pDataSource->SetSisterFiles(sisterFiles);
//        }
//
//    return pDataSource;
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//RealityPackage::WmsDataSourcePtr RealityDataPackageNet::CreateWmsDataSource(DataSourceNet^ source)
//    {
//    WmsSourceNet^ wmsSourceNet = dynamic_cast<WmsSourceNet^>(source);
//
//    // Create source with required parameters.
//    Utf8String uri;
//    BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetUri()).ToPointer()));
//    RealityPackage::WmsDataSourcePtr pWmsDataSource = RealityPackage::WmsDataSource::Create(uri.c_str());
//
//    // Add optional parameters.
//    if (!String::IsNullOrEmpty(wmsSourceNet->GetCopyright()))
//        {
//        Utf8String copyright;
//        BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetCopyright()).ToPointer()));
//        pWmsDataSource->SetCopyright(copyright.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(source->GetId()))
//        {
//        Utf8String id;
//        BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetId()).ToPointer()));
//        pWmsDataSource->SetId(id.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(wmsSourceNet->GetProvider()))
//        {
//        Utf8String provider;
//        BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetProvider()).ToPointer()));
//        pWmsDataSource->SetProvider(provider.c_str());
//        }
//
//    if (0 != wmsSourceNet->GetSize())
//        {
//        uint64_t size = wmsSourceNet->GetSize();
//        pWmsDataSource->SetSize(size);
//        }
//
//    if (!String::IsNullOrEmpty(wmsSourceNet->GetMetadata()))
//        {
//        Utf8String metadata;
//        BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetMetadata()).ToPointer()));
//        pWmsDataSource->SetMetadata(metadata.c_str());
//        }
//
//    if (0 != wmsSourceNet->GetSisterFiles()->Count)
//        {
//        //List<String^>^ sisterFilesNet = wmsSourceNet->GetSisterFiles();
//
//        bvector<Utf8String> sisterFiles;
//        pWmsDataSource->SetSisterFiles(sisterFiles);
//        }
//
//    // Xml Fragment.
//    Utf8String xmlFragment;
//    BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(wmsSourceNet->GetXmlFragment()).ToPointer()));
//    pWmsDataSource->SetMapSettings(xmlFragment.c_str());
//
//    return pWmsDataSource;
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//RealityPackage::OsmDataSourcePtr RealityDataPackageNet::CreateOsmDataSource(DataSourceNet^ source, RealityPackage::BoundingPolygonPtr boundingPolygon)
//    {
//    OsmSourceNet^ osmSourceNet = dynamic_cast<OsmSourceNet^>(source);
//
//    // Find min/max for bbox.
//    double minX = DBL_MAX;
//    double minY = DBL_MAX;
//    double maxX = -DBL_MAX;
//    double maxY = -DBL_MAX;
//
//    DPoint2dCP bboxPts = boundingPolygon->GetPointCP();
//    for (size_t i = 0; i < boundingPolygon->GetPointCount(); ++i)
//        {
//        if (bboxPts[i].x < minX)
//            minX = bboxPts[i].x;
//
//        if (bboxPts[i].x > maxX)
//            maxX = bboxPts[i].x;
//
//        if (bboxPts[i].y < minY)
//            minY = bboxPts[i].y;
//
//        if (bboxPts[i].y > maxY)
//            maxY = bboxPts[i].y;
//        }
//
//    DRange2d bbox;
//    bbox.InitFrom(minX, minY, maxX, maxY);
//
//    // Create source with required parameters.
//    Utf8String uri;
//    BeStringUtilities::WCharToUtf8(uri, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetUri()).ToPointer()));
//    RealityPackage::OsmDataSourcePtr pOsmDataSource = RealityPackage::OsmDataSource::Create(uri.c_str(), &bbox);
//
//    // Add optional parameters.
//    if (!String::IsNullOrEmpty(osmSourceNet->GetCopyright()))
//        {
//        Utf8String copyright;
//        BeStringUtilities::WCharToUtf8(copyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetCopyright()).ToPointer()));
//        pOsmDataSource->SetCopyright(copyright.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(osmSourceNet->GetId()))
//        {
//        Utf8String id;
//        BeStringUtilities::WCharToUtf8(id, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetId()).ToPointer()));
//        pOsmDataSource->SetId(id.c_str());
//        }
//
//    if (!String::IsNullOrEmpty(osmSourceNet->GetProvider()))
//        {
//        Utf8String provider;
//        BeStringUtilities::WCharToUtf8(provider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetProvider()).ToPointer()));
//        pOsmDataSource->SetProvider(provider.c_str());
//        }
//
//    if (0 != osmSourceNet->GetSize())
//        {
//        uint64_t size = osmSourceNet->GetSize();
//        pOsmDataSource->SetSize(size);
//        }
//
//    if (!String::IsNullOrEmpty(osmSourceNet->GetMetadata()))
//        {
//        Utf8String metadata;
//        BeStringUtilities::WCharToUtf8(metadata, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetMetadata()).ToPointer()));
//        pOsmDataSource->SetMetadata(metadata.c_str());
//        }
//
//    if (0 != osmSourceNet->GetSisterFiles()->Count)
//        {
//        //List<String^>^ sisterFilesNet = osmSourceNet->GetSisterFiles();
//
//        bvector<Utf8String> sisterFiles;
//        pOsmDataSource->SetSisterFiles(sisterFiles);
//        }
//
//    // Xml Fragment.
//    Utf8String xmlFragment;
//    BeStringUtilities::WCharToUtf8(xmlFragment, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmSourceNet->GetXmlFragment()).ToPointer()));
//    pOsmDataSource->SetOsmResource(xmlFragment.c_str());
//
//    return pOsmDataSource;
//    }
//
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ImageryGroupNet^ ImageryGroupNet::Create()
//    {
//    return gcnew ImageryGroupNet();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//void ImageryGroupNet::AddData(DataGroupNet^ imageryDataGroup)
//    {
//    m_imageryDataList->Add(imageryDataGroup);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ImageryGroupNet::ImageryGroupNet() 
//    {
//    m_imageryDataList = gcnew List<DataGroupNet^>();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ImageryGroupNet::~ImageryGroupNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ModelGroupNet^ ModelGroupNet::Create()
//    {
//    return gcnew ModelGroupNet();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//void ModelGroupNet::AddData(DataGroupNet^ modelDataGroup)
//    {
//    m_modelDataList->Add(modelDataGroup);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ModelGroupNet::ModelGroupNet() 
//    {
//    m_modelDataList = gcnew List<DataGroupNet^>();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//ModelGroupNet::~ModelGroupNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//PinnedGroupNet^ PinnedGroupNet::Create()
//    {
//    return gcnew PinnedGroupNet();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//void PinnedGroupNet::AddData(DataGroupNet^ pinnedDataGroup)
//    {
//    m_pinnedDataList->Add(pinnedDataGroup);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//PinnedGroupNet::PinnedGroupNet() 
//    {
//    m_pinnedDataList = gcnew List<DataGroupNet^>();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//PinnedGroupNet::~PinnedGroupNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//TerrainGroupNet^ TerrainGroupNet::Create()
//    {
//    return gcnew TerrainGroupNet();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//void TerrainGroupNet::AddData(DataGroupNet^ terrainDataGroup)
//    {
//    m_terrainDataList->Add(terrainDataGroup);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//TerrainGroupNet::TerrainGroupNet() 
//    {
//    m_terrainDataList = gcnew List<DataGroupNet^>();
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//TerrainGroupNet::~TerrainGroupNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//DataGroupNet^ DataGroupNet::Create(System::String^ id,
//                                   System::String^ name,
//                                   DataSourceNet^ source)
//    {
//    return gcnew DataGroupNet(id, name, source);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//void DataGroupNet::AddSource(DataSourceNet^ dataSource)
//    {
//    m_sources->Add(dataSource);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//DataGroupNet::DataGroupNet(System::String^ id,
//                           System::String^ name, 
//                           DataSourceNet^ source)
//    : m_id(id), 
//      m_name(name)
//    {
//    m_sources->Add(source);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    7/2016
////-------------------------------------------------------------------------------------
//DataGroupNet::~DataGroupNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//DataSourceNet^ DataSourceNet::Create(System::String^ uri,
//                                     System::String^ type, 
//                                     System::String^ copyright,
//                                     System::String^ id,
//                                     System::String^ provider,
//                                     uint64_t filesize,
//                                     System::String^ fileInCompound,
//                                     System::String^ metadata, 
//                                     System::String^ geocs,
//                                     List<System::String^>^ sisterFiles)
//    {
//    return gcnew DataSourceNet(uri, type, copyright, id, provider, filesize, fileInCompound, metadata, geocs, sisterFiles);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//DataSourceNet::DataSourceNet(System::String^ uri,
//                             System::String^ type,
//                             System::String^ copyright,
//                             System::String^ id,
//                             System::String^ provider,
//                             uint64_t size,
//                             System::String^ fileInCompound,
//                             System::String^ metadata,
//                             System::String^ geocs,
//                             List<System::String^>^ sisterFiles)
//    : m_uri(uri),
//      m_type(type),
//      m_copyright(copyright),
//      m_id(id),
//      m_provider(provider),
//      m_size(size),
//      m_fileInCompound(fileInCompound),
//      m_metadata(metadata),
//      m_geocs(geocs),
//      m_sisterFiles(sisterFiles)
//    {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 9/2015
////-------------------------------------------------------------------------------------
//DataSourceNet::~DataSourceNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 5/2015
////-------------------------------------------------------------------------------------
//WmsSourceNet::WmsSourceNet(System::String^ uri,
//                           System::String^ copyright,
//                           System::String^ id,
//                           System::String^ provider,
//                           uint64_t size,
//                           System::String^ metadata,
//                           System::String^ geocs,
//                           List<System::String^>^ sisterFiles,
//                           System::String^ mapUri,
//                           double bboxMinX,
//                           double bboxMinY,
//                           double bboxMaxX,
//                           double bboxMaxY,
//                           System::String^ version,
//                           System::String^ layers,
//                           System::String^ csType,
//                           System::String^ csLabel,
//                           size_t metaWidth,
//                           size_t metaHeight,
//                           System::String^ styles,
//                           System::String^ format,
//                           System::String^ vendorSpecific,
//                           bool isTransparent)
//    : DataSourceNet(uri, "wms", copyright, id, provider, size, "", metadata, geocs, sisterFiles)
//    {
//    // Create range from min and max values.
//    DRange2d bbox;
//    bbox.InitFrom(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);
//
//    // Create WmsMapSettings with required parameters.
//    Utf8String mapUriUtf8;
//    BeStringUtilities::WCharToUtf8(mapUriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(mapUri).ToPointer()));
//
//    Utf8String versionUtf8;
//    BeStringUtilities::WCharToUtf8(versionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(version).ToPointer()));
//    
//    Utf8String layersUtf8;
//    BeStringUtilities::WCharToUtf8(layersUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(layers).ToPointer()));
//
//    Utf8String csTypeUtf8;
//    BeStringUtilities::WCharToUtf8(csTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csType).ToPointer()));
//
//    Utf8String csLabelUtf8;
//    BeStringUtilities::WCharToUtf8(csLabelUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csLabel).ToPointer()));
//
//    RealityPackage::WmsMapSettingsPtr pMapSettings = RealityPackage::WmsMapSettings::Create(mapUriUtf8.c_str(),
//                                                                                            bbox,
//                                                                                            versionUtf8.c_str(),
//                                                                                            layersUtf8.c_str(),
//                                                                                            csTypeUtf8.c_str(),
//                                                                                            csLabelUtf8.c_str());
//    
//    // Optional parameters.
//    Utf8String stylesUtf8;
//    BeStringUtilities::WCharToUtf8(stylesUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(styles).ToPointer()));
//    pMapSettings->SetStyles(stylesUtf8.c_str());
//    
//    Utf8String formatUtf8;
//    BeStringUtilities::WCharToUtf8(formatUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer()));
//    pMapSettings->SetFormat(formatUtf8.c_str());
//    
//    Utf8String vendorSpecificUtf8;
//    BeStringUtilities::WCharToUtf8(vendorSpecificUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(vendorSpecific).ToPointer()));
//    pMapSettings->SetVendorSpecific(vendorSpecificUtf8.c_str());
//    
//    pMapSettings->SetTransparency(isTransparent);
//
//    // Convert to xml fragment and store the info.
//    Utf8String xmlFragment;
//    pMapSettings->ToXml(xmlFragment);
//    m_xmlFragment = gcnew System::String(xmlFragment.c_str());
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 5/2015
////-------------------------------------------------------------------------------------
//WmsSourceNet::~WmsSourceNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         		 6/2015
////-------------------------------------------------------------------------------------
//WmsSourceNet^ WmsSourceNet::Create(System::String^ uri,
//                                   System::String^ copyright,
//                                   System::String^ id,
//                                   System::String^ provider,
//                                   uint64_t size,
//                                   System::String^ metadata,
//                                   System::String^ geocs,
//                                   List<System::String^>^ sisterFiles,
//                                   System::String^ mapUri,
//                                   double bboxMinX,
//                                   double bboxMinY,
//                                   double bboxMaxX,
//                                   double bboxMaxY,
//                                   System::String^ version,
//                                   System::String^ layers,
//                                   System::String^ csType,
//                                   System::String^ csLabel,
//                                   size_t metaWidth,
//                                   size_t metaHeight,
//                                   System::String^ styles,
//                                   System::String^ format,
//                                   System::String^ vendorSpecific,
//                                   bool isTransparent)
//    {
//    return gcnew WmsSourceNet(uri,
//                              copyright,
//                              id,
//                              provider,
//                              size,
//                              metadata,
//                              geocs,
//                              sisterFiles,
//                              mapUri,
//                              bboxMinX,
//                              bboxMinY,
//                              bboxMaxX,
//                              bboxMaxY,
//                              version,
//                              layers,
//                              csType,
//                              csLabel,
//                              metaWidth,
//                              metaHeight,
//                              styles,
//                              format,
//                              vendorSpecific,
//                              isTransparent);
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    11/2015
////-------------------------------------------------------------------------------------
//OsmSourceNet::OsmSourceNet(System::String^ uri,
//                           System::String^ copyright,
//                           System::String^ id,
//                           System::String^ provider,
//                           uint64_t size,
//                           System::String^ metadata,
//                           System::String^ geocs,
//                           List<System::String^>^ sisterFiles,
//                           List<double>^ regionOfInterest,
//                           List<System::String^>^ urls)
//    : DataSourceNet(uri, "osm", copyright, id, provider, size, "", metadata, geocs, sisterFiles)
//    {
//    // Create range from min and max values.
//    DPoint2d pts[4];
//    pts[0].x = regionOfInterest[0]; pts[0].y = regionOfInterest[1];
//    pts[1].x = regionOfInterest[2]; pts[1].y = regionOfInterest[3];
//    pts[2].x = regionOfInterest[4]; pts[2].y = regionOfInterest[5];
//    pts[3].x = regionOfInterest[6]; pts[3].y = regionOfInterest[7];
//
//    double minX = DBL_MAX;
//    double minY = DBL_MAX;
//    double maxX = -DBL_MAX;
//    double maxY = -DBL_MAX;
//
//    for (size_t i = 0; i < 4; ++i)
//        {
//        if (pts[i].x < minX)
//            minX = pts[i].x;
//
//        if (pts[i].x > maxX)
//            maxX = pts[i].x;
//
//        if (pts[i].y < minY)
//            minY = pts[i].y;
//
//        if (pts[i].y > maxY)
//            maxY = pts[i].y;
//        }
//
//    DRange2d bbox;
//    bbox.InitFrom(minX, minY, maxX, maxY);
//
//    // Create OsmResource with required parameters.
//    RealityPackage::OsmResourcePtr pOsmResource = RealityPackage::OsmResource::Create(bbox);
//
//    // Optional parameters.    
//    if (0 != urls->Count)
//        {
//        bvector<Utf8String> urlList;
//        for each(System::String^ url in urls)
//            {
//            Utf8String urlUtf8;
//            BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer()));
//            urlList.push_back(urlUtf8);
//            }
//        pOsmResource->SetAlternateUrlList(urlList);
//        }
//
//    // Convert to xml fragment and store the info.
//    Utf8String xmlFragment;
//    pOsmResource->ToXml(xmlFragment);
//    m_xmlFragment = gcnew System::String(xmlFragment.c_str());
//    }
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    11/2015
////-------------------------------------------------------------------------------------
//OsmSourceNet::~OsmSourceNet() {}
//
////-------------------------------------------------------------------------------------
//// @bsimethod                                   Jean-Francois.Cote         	    11/2015
////-------------------------------------------------------------------------------------
//OsmSourceNet^ OsmSourceNet::Create(System::String^ uri,
//                                   System::String^ copyright,
//                                   System::String^ id,
//                                   System::String^ provider,
//                                   uint64_t size,
//                                   System::String^ metadata,
//                                   System::String^ geocs,
//                                   List<System::String^>^ sisterFiles, 
//                                   List<double>^ regionOfInterest,
//                                   List<System::String^>^ urls)
//    {
//    return gcnew OsmSourceNet(uri, copyright, id, provider, size, metadata, geocs, sisterFiles, regionOfInterest, urls);
//    }
