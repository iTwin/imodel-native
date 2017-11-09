/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/RealityDataSourceNet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "RealityDataSourceNet.h"

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
UriNet^ NativeToManagedUri(UriCR nativeUri)
    {
    marshal_context ctx;
    return UriNet::Create(ctx.marshal_as<String^>(nativeUri.ToString().c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriPtr ManagedToNativeUri(UriNet^ managedUri)
    {
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedUri->ToStr()).ToPointer()));

    return RealityPlatform::Uri::Create(uriUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ NativeToManagedRealityDataSource(RealityDataSourceCR nativeSource)
    {
    UriNet^ managedUri = NativeToManagedUri(nativeSource.GetUri());

    marshal_context ctx;
    String^ managedType = ctx.marshal_as<String^>(nativeSource.GetType().c_str());

    // Create source with required parameters.
    RealityDataSourceNet^ managedSource = RealityDataSourceNet::Create(managedUri, managedType);

    // Streamed.
    managedSource->SetStreamed(nativeSource.IsStreamed());

    // Id.
    String^ managedId = ctx.marshal_as<String^>(nativeSource.GetId().c_str());
    managedSource->SetId(managedId);

    // Copyright.
    String^ managedCopyright = ctx.marshal_as<String^>(nativeSource.GetCopyright().c_str());
    managedSource->SetCopyright(managedCopyright);

    // Term of use.
    String^ managedTermOfUse = ctx.marshal_as<String^>(nativeSource.GetTermOfUse().c_str());
    managedSource->SetTermOfUse(managedTermOfUse);

    // Provider.
    String^ managedProvider = ctx.marshal_as<String^>(nativeSource.GetProvider().c_str());
    managedSource->SetProvider(managedProvider);

    // Server login key.
    String^ managedServerLoginKey = ctx.marshal_as<String^>(nativeSource.GetServerLoginKey().c_str());
    managedSource->SetServerLoginKey(managedServerLoginKey);

    // Server login method.
    String^ managedServerLoginMethod = ctx.marshal_as<String^>(nativeSource.GetServerLoginMethod().c_str());
    managedSource->SetServerLoginMethod(managedServerLoginMethod);

    // Server registration page.
    String^ managedServerRegPage = ctx.marshal_as<String^>(nativeSource.GetServerRegistrationPage().c_str());
    managedSource->SetServerRegistrationPage(managedServerRegPage);

    // Server organisation page.
    String^ managedServerOrgPage = ctx.marshal_as<String^>(nativeSource.GetServerOrganisationPage().c_str());
    managedSource->SetServerOrganisationPage(managedServerOrgPage);

    // Size.
    uint64_t size = (uint64_t)nativeSource.GetSize();
    managedSource->SetSize(size);

    // Metadata.
    String^ managedMetadata = ctx.marshal_as<String^>(nativeSource.GetMetadata().c_str());
    managedSource->SetMetadata(managedMetadata);

    // Metadata type.
    String^ managedMetadataType = ctx.marshal_as<String^>(nativeSource.GetMetadataType().c_str());
    managedSource->SetMetadataType(managedMetadataType);

    // GeoCS.
    String^ managedGeoCS = ctx.marshal_as<String^>(nativeSource.GetGeoCS().c_str());
    managedSource->SetGeoCS(managedGeoCS);

    // No data value.
    String^ managedNoDataValue = ctx.marshal_as<String^>(nativeSource.GetNoDataValue().c_str());
    managedSource->SetNoDataValue(managedNoDataValue);

    // Sister files.
    bvector<UriPtr> nativeSisterFiles = nativeSource.GetSisterFiles();
    List<UriNet^>^ managedSisterFiles = gcnew List<UriNet^>();
    for (UriPtr pNativeSisterFile : nativeSisterFiles)
        {
        managedSisterFiles->Add(NativeToManagedUri(*pNativeSisterFile));
        }
    managedSource->SetSisterFiles(managedSisterFiles);

    return managedSource;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourcePtr ManagedToNativeRealityDataSource(RealityDataSourceNet^ managedSource)
    {
    RealityPlatform::UriPtr nativeUri = ManagedToNativeUri(managedSource->GetUri());

    Utf8String nativeType;
    BeStringUtilities::WCharToUtf8(nativeType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetSourceType()).ToPointer()));

    // Create source with required parameters.
    RealityDataSourcePtr nativeSource = RealityDataSource::Create(*nativeUri, nativeType.c_str());

    // Streamed.
    nativeSource->SetStreamed(managedSource->IsStreamed());

    // Id.
    Utf8String nativeId;
    BeStringUtilities::WCharToUtf8(nativeId, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetId()).ToPointer()));
    nativeSource->SetId(nativeId.c_str());

    // Copyright.
    Utf8String nativeCopyright;
    BeStringUtilities::WCharToUtf8(nativeCopyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetCopyright()).ToPointer()));
    nativeSource->SetCopyright(nativeCopyright.c_str());

    // Term of use.
    Utf8String nativeTermOfUse;
    BeStringUtilities::WCharToUtf8(nativeTermOfUse, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetTermOfUse()).ToPointer()));
    nativeSource->SetTermOfUse(nativeTermOfUse.c_str());

    // Provider.
    Utf8String nativeProvider;
    BeStringUtilities::WCharToUtf8(nativeProvider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetProvider()).ToPointer()));
    nativeSource->SetProvider(nativeProvider.c_str());

    // Server login key.
    Utf8String nativeServerLoginKey;
    BeStringUtilities::WCharToUtf8(nativeServerLoginKey, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerLoginKey()).ToPointer()));
    nativeSource->SetServerLoginKey(nativeServerLoginKey.c_str());

    // Server login method.
    Utf8String nativeServerLoginMethod;
    BeStringUtilities::WCharToUtf8(nativeServerLoginMethod, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerLoginMethod()).ToPointer()));
    nativeSource->SetServerLoginMethod(nativeServerLoginMethod.c_str());

    // Server registration page.
    Utf8String nativeServerRegPage;
    BeStringUtilities::WCharToUtf8(nativeServerRegPage, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerRegistrationPage()).ToPointer()));
    nativeSource->SetServerRegistrationPage(nativeServerRegPage.c_str());

    // Server organisation page.
    Utf8String nativeServerOrgPage;
    BeStringUtilities::WCharToUtf8(nativeServerOrgPage, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerOrganisationPage()).ToPointer()));
    nativeSource->SetServerOrganisationPage(nativeServerOrgPage.c_str());

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
        nativeSisterFiles.push_back(ManagedToNativeUri(managedSisterFile));
        }
    nativeSource->SetSisterFiles(nativeSisterFiles);

    return nativeSource;
    }


//=======================================================================================
//                                      Uri
//=======================================================================================
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

    m_pUri = new RealityPlatform::UriPtr(RealityPlatform::Uri::Create(resourceIdentifierUtf8.c_str()));
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

    m_pUri = new RealityPlatform::UriPtr(RealityPlatform::Uri::Create(sourceUtf8.c_str(), fileInCompoundUtf8.c_str()));
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


//=======================================================================================
//                               RealityDataSource
//=======================================================================================
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
    return NativeToManagedUri((*m_pSource)->GetUri());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetUri(UriNet^ uri)
    {
    (*m_pSource)->SetUri(*ManagedToNativeUri(uri));
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
bool RealityDataSourceNet::IsStreamed()
    {
    return (*m_pSource)->IsStreamed();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetStreamed(bool isStreamed)
    {
    (*m_pSource)->SetStreamed(isStreamed);
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
String^ RealityDataSourceNet::GetTermOfUse()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetTermOfUse().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetTermOfUse(String^ termOfUse)
    {
    Utf8String termOfUseUtf8;
    BeStringUtilities::WCharToUtf8(termOfUseUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(termOfUse).ToPointer()));

    (*m_pSource)->SetTermOfUse(termOfUseUtf8.c_str());
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
String^ RealityDataSourceNet::GetServerLoginKey()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerLoginKey().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetServerLoginKey(String^ key)
    {
    Utf8String keyUtf8;
    BeStringUtilities::WCharToUtf8(keyUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(key).ToPointer()));

    (*m_pSource)->SetServerLoginKey(keyUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetServerLoginMethod()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerLoginMethod().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetServerLoginMethod(String^ method)
    {
    Utf8String methodUtf8;
    BeStringUtilities::WCharToUtf8(methodUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(method).ToPointer()));

    (*m_pSource)->SetServerLoginMethod(methodUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetServerRegistrationPage()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerRegistrationPage().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetServerRegistrationPage(String^ link)
    {
    Utf8String linkUtf8;
    BeStringUtilities::WCharToUtf8(linkUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(link).ToPointer()));

    (*m_pSource)->SetServerRegistrationPage(linkUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ RealityDataSourceNet::GetServerOrganisationPage()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerOrganisationPage().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetServerOrganisationPage(String^ link)
    {
    Utf8String linkUtf8;
    BeStringUtilities::WCharToUtf8(linkUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(link).ToPointer()));

    (*m_pSource)->SetServerOrganisationPage(linkUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
uint64_t RealityDataSourceNet::GetSize()
    {
    return (uint64_t)(*m_pSource)->GetSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetSize(uint64_t sizeInKB)
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
List<UriNet^>^ RealityDataSourceNet::GetSisterFiles()
    {
    marshal_context ctx;
    List<UriNet^>^ managedSisterFiles = gcnew List<UriNet^>();

    bvector<UriPtr> sisterFiles = (*m_pSource)->GetSisterFiles();
    for (UriPtr pSisterFile : sisterFiles)
        {
        managedSisterFiles->Add(NativeToManagedUri(*pSisterFile));
        }

    return managedSisterFiles;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetSisterFiles(List<UriNet^>^ sisterFiles)
    {
    bvector<UriPtr> nativeSisterFiles;
    Utf8String sisterFileUtf8;    

    for each (UriNet^ sisterFile in sisterFiles)
        {
        nativeSisterFiles.push_back(ManagedToNativeUri(sisterFile));
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
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    m_pSource = new RealityDataSourcePtr(RealityDataSource::Create(*ManagedToNativeUri(uri), typeUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Christian.Tye-Gingras         	02/2017
//-------------------------------------------------------------------------------------
System::IntPtr RealityDataSourceNet::GetPeer()
    { 
        return System::IntPtr((void *) m_pSource); 
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Christian.Tye-Gingras         	02/2017
//-------------------------------------------------------------------------------------
void RealityDataSourceNet::SetPeer(System::IntPtr newRDSN)
        {
        if (NULL != m_pSource)
            delete m_pSource;

        m_pSource = new RealityDataSourcePtr((*(RealityDataSourcePtr*) newRDSN.ToPointer()));
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


//=======================================================================================
//                                  WmsDataSource
//=======================================================================================
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
    : RealityDataSourceNet(UriNet::Create(uri), "wms")
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
    : RealityDataSourceNet(uri, "wms")
    {
    // Managed to native reality data source.    
    m_pSource = new WmsDataSourcePtr(WmsDataSource::Create(*ManagedToNativeUri(uri)));
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


//=======================================================================================
//                                  OsmDataSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmDataSourceNet^ OsmDataSourceNet::Create(String^ uri, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY)
    {
    return gcnew OsmDataSourceNet(uri, bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ OsmDataSourceNet::GetOsmResource()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetOsmResource().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void OsmDataSourceNet::SetOsmResource(String^ osmResource)
    {
    Utf8String osmResourceUtf8;
    BeStringUtilities::WCharToUtf8(osmResourceUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(osmResource).ToPointer()));

    (*m_pSource)->SetOsmResource(osmResourceUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmDataSourceNet::OsmDataSourceNet(String^ uri, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY)
    : RealityDataSourceNet(UriNet::Create(uri), "osm")
    {
    // Managed to native reality data source.
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer()));

    DRange2d bbox = DRange2d::From(bboxMinX, bboxMinY, bboxMaxX, bboxMaxY);

    m_pSource = new OsmDataSourcePtr(OsmDataSource::Create(uriUtf8.c_str(), &bbox));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmDataSourceNet::~OsmDataSourceNet()
    {
    this->!OsmDataSourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmDataSourceNet::!OsmDataSourceNet()
    {
    if (0 != m_pSource)
        {
        delete m_pSource;
        m_pSource = 0;
        }
    }


//=======================================================================================
//                                  MultiBandSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourceNet^ MultiBandSourceNet::Create(UriNet^ uri, System::String^ type)
    {
    return gcnew MultiBandSourceNet(uri, type);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ MultiBandSourceNet::GetRedBand()
    {
    return NativeToManagedRealityDataSource(*(*m_pSource)->GetRedBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetRedBand(RealityDataSourceNet^ band)
    {
    (*m_pSource)->SetRedBand(*ManagedToNativeRealityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ MultiBandSourceNet::GetGreenBand()
    {
    return NativeToManagedRealityDataSource(*(*m_pSource)->GetGreenBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetGreenBand(RealityDataSourceNet^ band)
    {
    (*m_pSource)->SetGreenBand(*ManagedToNativeRealityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ MultiBandSourceNet::GetBlueBand()
    {
    return NativeToManagedRealityDataSource(*(*m_pSource)->GetBlueBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetBlueBand(RealityDataSourceNet^ band)
    {
    (*m_pSource)->SetBlueBand(*ManagedToNativeRealityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
RealityDataSourceNet^ MultiBandSourceNet::GetPanchromaticBand()
    {
    return NativeToManagedRealityDataSource(*(*m_pSource)->GetPanchromaticBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetPanchromaticBand(RealityDataSourceNet^ band)
    {
    (*m_pSource)->SetPanchromaticBand(*ManagedToNativeRealityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourceNet::MultiBandSourceNet(UriNet^ uri, System::String^ type)
    : RealityDataSourceNet(uri, type)
    {
    // Managed to native reality data source.
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    m_pSource = new MultiBandSourcePtr(MultiBandSource::Create(*ManagedToNativeUri(uri), typeUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourceNet::~MultiBandSourceNet()
    {
    this->!MultiBandSourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourceNet::!MultiBandSourceNet()
    {
    if (0 != m_pSource)
        {
        delete m_pSource;
        m_pSource = 0;
        }
    }