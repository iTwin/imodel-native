/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/RealityDataSourceNet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "RealityDataSourceNet.h"

using namespace RealityPackage;
using namespace RealityPackageNet;

// System.
using namespace System;
using namespace System::Collections::Generic;

// Interop.
#include <msclr/marshal.h>

using namespace msclr::interop;
using namespace System::Runtime::InteropServices;


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet^ UriNet::Create(String^ resourceIdentifier)
    {
    return gcnew UriNet(resourceIdentifier);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet^ UriNet::Create(String^ source, String^ fileInCompound)
    {
    return gcnew UriNet(source, fileInCompound);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ UriNet::GetSource()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pUri)->GetSource().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ UriNet::GetFileInCompound()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pUri)->GetFileInCompound().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ UriNet::ToStr()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pUri)->ToString().c_str());
    }
     
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet::UriNet(String^ resourceIdentifier)
    {
    // Managed to native uri.
    Utf8String resourceIdentifierUtf8;
    BeStringUtilities::WCharToUtf8(resourceIdentifierUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(resourceIdentifier).ToPointer()));

    m_pUri = new RealityPackage::UriPtr(RealityPackage::Uri::Create(resourceIdentifierUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet::UriNet(String^ source, String^ fileInCompound)
    {
    // Managed to native uri.
    Utf8String sourceUtf8;
    BeStringUtilities::WCharToUtf8(sourceUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(source).ToPointer()));

    Utf8String fileInCompoundUtf8;
    BeStringUtilities::WCharToUtf8(fileInCompoundUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(fileInCompound).ToPointer()));

    m_pUri = new RealityPackage::UriPtr(RealityPackage::Uri::Create(sourceUtf8.c_str(), fileInCompoundUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet::~UriNet()
    {
    this->!UriNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet::!UriNet()
    {
    if (0 != m_pUri)
        {
        delete m_pUri;
        m_pUri = 0;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ RealityDataSourceNet::Create(UriNet^ uri, String^ type)
    {
    return gcnew RealityDataSourceNet(uri, type);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet^ RealityDataSourceNet::GetUri()
    {    
    marshal_context ctx;
    return UriNet::Create(ctx.marshal_as<String^>((*m_pSource)->GetUri().ToString().c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetUri(UriNet^ uri)
    {
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri->ToStr()).ToPointer()));

    (*m_pSource)->SetUri(*RealityPackage::Uri::Create(uriUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetSourceType()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetSourceType(String^ type)
    {
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    (*m_pSource)->SetType(typeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetId()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetId().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetId(String^ id)
    {
    Utf8String idUtf8;
    BeStringUtilities::WCharToUtf8(idUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(id).ToPointer()));

    (*m_pSource)->SetId(idUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetCopyright()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetCopyright().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetCopyright(String^ copyright)
    {
    Utf8String copyrightUtf8;
    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));

    (*m_pSource)->SetCopyright(copyrightUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetProvider()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetProvider().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetProvider(String^ provider)
    {
    Utf8String providerUtf8;
    BeStringUtilities::WCharToUtf8(providerUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(provider).ToPointer()));

    (*m_pSource)->SetProvider(providerUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
int RealityDataSourceNet::GetSize()
    {
    return (int)(*m_pSource)->GetSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetSize(int sizeInKB)
    {
    (*m_pSource)->SetSize(sizeInKB);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetMetadata()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadata().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetMetadata(String^ metadata)
    {
    Utf8String metadataUtf8;
    BeStringUtilities::WCharToUtf8(metadataUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata).ToPointer()));

    (*m_pSource)->SetMetadata(metadataUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetMetadataType()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadataType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetMetadataType(String^ metadataType)
    {
    Utf8String metadataTypeUtf8;
    BeStringUtilities::WCharToUtf8(metadataTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadataType).ToPointer()));

    (*m_pSource)->SetMetadataType(metadataTypeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetGeoCS()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetGeoCS().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetGeoCS(String^ geoCS)
    {
    Utf8String geoCSUtf8;
    BeStringUtilities::WCharToUtf8(geoCSUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(geoCS).ToPointer()));

    (*m_pSource)->SetGeoCS(geoCSUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetNoDataValue()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetNoDataValue().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetNoDataValue(String^ nodatavalue)
    {
    Utf8String nodatavalueUtf8;
    BeStringUtilities::WCharToUtf8(nodatavalueUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(nodatavalue).ToPointer()));

    (*m_pSource)->SetNoDataValue(nodatavalueUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<String^>^ RealityDataSourceNet::GetSisterFiles()
    {
    marshal_context ctx;
    List<String^>^ managedSisterFiles = gcnew List<String^>();

    bvector<Utf8String> sisterFiles = (*m_pSource)->GetSisterFiles();
    for (Utf8StringCR sisterFile : sisterFiles)
        {
        managedSisterFiles->Add(marshal_as<String^>(sisterFile.c_str()));
        }

    return managedSisterFiles;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetSisterFiles(List<String^>^ sisterFiles)
    {
    bvector<Utf8String> nativeSisterFiles;
    Utf8String sisterFileUtf8;    

    for each (String^ sisterFile in sisterFiles)
        {
        BeStringUtilities::WCharToUtf8(sisterFileUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(sisterFile).ToPointer()));
        nativeSisterFiles.push_back(sisterFileUtf8);
        }

    (*m_pSource)->SetSisterFiles(nativeSisterFiles);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetElementName()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetElementName());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet::RealityDataSourceNet(UriNet^ uri, String^ type)
    {
    // Managed to native reality data source.
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri->ToStr()).ToPointer()));

    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    m_pSource = new RealityDataSourcePtr(RealityDataSource::Create(*RealityPackage::Uri::Create(uriUtf8.c_str()), typeUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet::~RealityDataSourceNet()
    {
    this->!RealityDataSourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet::!RealityDataSourceNet()
    {
    if (0 != m_pSource)
        {
        delete m_pSource;
        m_pSource = 0;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet^ WmsDataSourceNet::Create(String^ uri)
    {
    return gcnew WmsDataSourceNet(uri);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet^ WmsDataSourceNet::Create(UriNet^ uri)
    {
    return gcnew WmsDataSourceNet(uri);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsDataSourceNet::GetMapSettings()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMapSettings().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsDataSourceNet::SetMapSettings(String^ mapSettings)
    {
    Utf8String mapSettingsUtf8;
    BeStringUtilities::WCharToUtf8(mapSettingsUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(mapSettings).ToPointer()));

    (*m_pSource)->SetMapSettings(mapSettingsUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet::WmsDataSourceNet(String^ uri)
    {
    // Managed to native reality data source.
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer()));

    m_pSource = new WmsDataSourcePtr(WmsDataSource::Create(uriUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet::WmsDataSourceNet(UriNet^ uri)
    {
    // Managed to native reality data source.
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri->ToStr()).ToPointer()));

    m_pSource = new WmsDataSourcePtr(WmsDataSource::Create(*RealityPackage::Uri::Create(uriUtf8.c_str())));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet::~WmsDataSourceNet()
    {
    this->!WmsDataSourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsDataSourceNet::!WmsDataSourceNet()
    {
    if (0 != m_pSource)
        {
        delete m_pSource;
        m_pSource = 0;
        }
    }
