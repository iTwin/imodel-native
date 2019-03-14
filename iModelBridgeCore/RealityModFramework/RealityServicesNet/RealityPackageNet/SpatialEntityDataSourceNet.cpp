/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/SpatialEntityDataSourceNet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "SpatialEntityDataSourceNet.h"

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
SpatialEntityDataSourceNet^ NativeToManagedSpatialEntityDataSource(SpatialEntityDataSourceCR nativeSource)
    {
    UriNet^ managedUri = NativeToManagedUri(nativeSource.GetUri());

    marshal_context ctx;
    String^ managedType = ctx.marshal_as<String^>(nativeSource.GetDataType().c_str());

    // Create source with required parameters.
    SpatialEntityDataSourceNet^ managedSource = SpatialEntityDataSourceNet::Create(managedUri, managedType);

    // Streamed.
    managedSource->SetStreamed(nativeSource.GetServerCP()->IsStreamed());

    // Id.
    String^ managedId = ctx.marshal_as<String^>(nativeSource.GetId().c_str());
    managedSource->SetId(managedId);

    // Copyright.
    String^ managedCopyright = ctx.marshal_as<String^>(nativeSource.GetMetadataCP()->GetLegal().c_str());
    managedSource->SetCopyright(managedCopyright);

    // Term of use.
    String^ managedTermOfUse = ctx.marshal_as<String^>(nativeSource.GetMetadataCP()->GetTermsOfUse().c_str());
    managedSource->SetTermOfUse(managedTermOfUse);

    // Provider.
    String^ managedProvider = ctx.marshal_as<String^>(nativeSource.GetProvider().c_str());
    managedSource->SetProvider(managedProvider);

    // Server login key.
    String^ managedServerLoginKey = ctx.marshal_as<String^>(nativeSource.GetServerCP()->GetLoginKey().c_str());
    managedSource->SetServerLoginKey(managedServerLoginKey);

    // Server login method.
    String^ managedServerLoginMethod = ctx.marshal_as<String^>(nativeSource.GetServerCP()->GetLoginMethod().c_str());
    managedSource->SetServerLoginMethod(managedServerLoginMethod);

    // Server registration page.
    String^ managedServerRegPage = ctx.marshal_as<String^>(nativeSource.GetServerCP()->GetRegistrationPage().c_str());
    managedSource->SetServerRegistrationPage(managedServerRegPage);

    // Server organisation page.
    String^ managedServerOrgPage = ctx.marshal_as<String^>(nativeSource.GetServerCP()->GetOrganisationPage().c_str());
    managedSource->SetServerOrganisationPage(managedServerOrgPage);

    // Visibility.
    String^ sourceVisibilityTag = ctx.marshal_as<String^>(nativeSource.GetVisibilityTag().c_str());
    managedSource->SetVisibilityByTag(sourceVisibilityTag);

    // Size.
    uint64_t size = (uint64_t)nativeSource.GetSize();
    managedSource->SetSize(size);

    // Metadata.
    String^ managedMetadata = ctx.marshal_as<String^>(nativeSource.GetMetadataCP()->GetDescription().c_str());
    managedSource->SetMetadata(managedMetadata);

    // Metadata type.
    String^ managedMetadataType = ctx.marshal_as<String^>(nativeSource.GetMetadataCP()->GetMetadataType().c_str());
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
SpatialEntityDataSourcePtr ManagedToNativeSpatialEntityDataSource(SpatialEntityDataSourceNet^ managedSource)
    {
    RealityPlatform::UriPtr nativeUri = ManagedToNativeUri(managedSource->GetUri());

    Utf8String nativeType;
    BeStringUtilities::WCharToUtf8(nativeType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetSourceType()).ToPointer()));

    // Create source with required parameters.
    SpatialEntityDataSourcePtr nativeSource = SpatialEntityDataSource::Create(*nativeUri, nativeType.c_str());

    SpatialEntityServerPtr nativeServer = SpatialEntityServer::Create();

    SpatialEntityMetadataPtr nativeMetadata = SpatialEntityMetadata::Create();

    // Streamed.
    nativeServer->SetStreamed(managedSource->IsStreamed());

    nativeSource->SetServer(nativeServer);

    // Id.
    Utf8String nativeId;
    BeStringUtilities::WCharToUtf8(nativeId, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetId()).ToPointer()));
    nativeSource->SetId(nativeId.c_str());

    // Copyright.
    Utf8String nativeCopyright;
    BeStringUtilities::WCharToUtf8(nativeCopyright, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetCopyright()).ToPointer()));
    nativeMetadata->SetLegal(nativeCopyright.c_str());

    // Term of use.
    Utf8String nativeTermOfUse;
    BeStringUtilities::WCharToUtf8(nativeTermOfUse, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetTermOfUse()).ToPointer()));
    nativeMetadata->SetTermsOfUse(nativeTermOfUse.c_str());

    nativeSource->SetMetadata(nativeMetadata);

    // Provider.
    Utf8String nativeProvider;
    BeStringUtilities::WCharToUtf8(nativeProvider, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetProvider()).ToPointer()));
    nativeSource->SetProvider(nativeProvider.c_str());

    // Server login key.
    Utf8String nativeServerLoginKey;
    BeStringUtilities::WCharToUtf8(nativeServerLoginKey, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerLoginKey()).ToPointer()));
    nativeServer->SetLoginKey(nativeServerLoginKey.c_str());

    // Server login method.
    Utf8String nativeServerLoginMethod;
    BeStringUtilities::WCharToUtf8(nativeServerLoginMethod, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerLoginMethod()).ToPointer()));
    nativeServer->SetLoginMethod(nativeServerLoginMethod.c_str());

    // Server registration page.
    Utf8String nativeServerRegPage;
    BeStringUtilities::WCharToUtf8(nativeServerRegPage, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerRegistrationPage()).ToPointer()));
    nativeServer->SetRegistrationPage(nativeServerRegPage.c_str());

    // Server organisation page.
    Utf8String nativeServerOrgPage;
    BeStringUtilities::WCharToUtf8(nativeServerOrgPage, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetServerOrganisationPage()).ToPointer()));
    nativeServer->SetOrganisationPage(nativeServerOrgPage.c_str());

    // Size.
    nativeSource->SetSize(managedSource->GetSize());

    // Metadata.
    Utf8String nativeMetadataStr;
    BeStringUtilities::WCharToUtf8(nativeMetadataStr, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadata()).ToPointer()));
    nativeMetadata->SetDescription(nativeMetadataStr.c_str());

    // Metadata type.
    Utf8String nativeMetadataType;
    BeStringUtilities::WCharToUtf8(nativeMetadataType, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(managedSource->GetMetadataType()).ToPointer()));
    nativeMetadata->SetMetadataType(nativeMetadataType.c_str());

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
//                               SpatialEntityDataSource
//=======================================================================================
//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet^ SpatialEntityDataSourceNet::Create(UriNet^ uri, String^ type)
    {
    return gcnew SpatialEntityDataSourceNet(uri, type);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
UriNet^ SpatialEntityDataSourceNet::GetUri()
    {    
    return NativeToManagedUri((*m_pSource)->GetUri());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetUri(UriNet^ uri)
    {
    (*m_pSource)->SetUri(*ManagedToNativeUri(uri));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetSourceType()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetDataType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetSourceType(String^ type)
    {
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    (*m_pSource)->SetDataType(typeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool SpatialEntityDataSourceNet::IsStreamed()
    {
    return (*m_pSource)->GetServerCP()->IsStreamed();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetStreamed(bool isStreamed)
    {
    (*m_pSource)->GetServerP()->SetStreamed(isStreamed);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetId()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetId().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetId(String^ id)
    {
    Utf8String idUtf8;
    BeStringUtilities::WCharToUtf8(idUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(id).ToPointer()));

    (*m_pSource)->SetId(idUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetCopyright()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadataCP()->GetLegal().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetCopyright(String^ copyright)
    {
    Utf8String copyrightUtf8;
    BeStringUtilities::WCharToUtf8(copyrightUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(copyright).ToPointer()));

    (*m_pSource)->GetMetadataP()->SetLegal(copyrightUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetTermOfUse()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadataCP()->GetTermsOfUse().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetTermOfUse(String^ termOfUse)
    {
    Utf8String termOfUseUtf8;
    BeStringUtilities::WCharToUtf8(termOfUseUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(termOfUse).ToPointer()));

    (*m_pSource)->GetMetadataP()->SetTermsOfUse(termOfUseUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetProvider()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetProvider().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetProvider(String^ provider)
    {
    Utf8String providerUtf8;
    BeStringUtilities::WCharToUtf8(providerUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(provider).ToPointer()));

    (*m_pSource)->SetProvider(providerUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetServerLoginKey()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerCP()->GetLoginKey().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetServerLoginKey(String^ key)
    {
    Utf8String keyUtf8;
    BeStringUtilities::WCharToUtf8(keyUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(key).ToPointer()));

    (*m_pSource)->GetServerP()->SetLoginKey(keyUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetServerLoginMethod()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerCP()->GetLoginMethod().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetServerLoginMethod(String^ method)
    {
    Utf8String methodUtf8;
    BeStringUtilities::WCharToUtf8(methodUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(method).ToPointer()));

    (*m_pSource)->GetServerP()->SetLoginMethod(methodUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetServerRegistrationPage()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerCP()->GetRegistrationPage().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetServerRegistrationPage(String^ link)
    {
    Utf8String linkUtf8;
    BeStringUtilities::WCharToUtf8(linkUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(link).ToPointer()));

    (*m_pSource)->GetServerP()->SetRegistrationPage(linkUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetServerOrganisationPage()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetServerCP()->GetOrganisationPage().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetServerOrganisationPage(String^ link)
    {
    Utf8String linkUtf8;
    BeStringUtilities::WCharToUtf8(linkUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(link).ToPointer()));

    (*m_pSource)->GetServerP()->SetOrganisationPage(linkUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain Robert          	    10/2018
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetVisibilityTag()
{
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetVisibilityTag().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain Robert         	    10/2018
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetVisibilityByTag(String^ visibilityTag)
{
    Utf8String tagUtf8;
    BeStringUtilities::WCharToUtf8(tagUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(visibilityTag).ToPointer()));

    (*m_pSource)->SetVisibilityByTag(tagUtf8.c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
uint64_t SpatialEntityDataSourceNet::GetSize()
    {
    return (uint64_t)(*m_pSource)->GetSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetSize(uint64_t sizeInKB)
    {
    (*m_pSource)->SetSize(sizeInKB);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetMetadata()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadataCP()->GetDescription().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetMetadata(String^ metadata)
    {
    Utf8String metadataUtf8;
    BeStringUtilities::WCharToUtf8(metadataUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata).ToPointer()));

    (*m_pSource)->GetMetadataP()->SetDescription(metadataUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetMetadataType()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetMetadataCP()->GetMetadataType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetMetadataType(String^ metadataType)
    {
    Utf8String metadataTypeUtf8;
    BeStringUtilities::WCharToUtf8(metadataTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadataType).ToPointer()));

    (*m_pSource)->GetMetadataP()->SetMetadataType(metadataTypeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetGeoCS()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetGeoCS().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetGeoCS(String^ geoCS)
    {
    Utf8String geoCSUtf8;
    BeStringUtilities::WCharToUtf8(geoCSUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(geoCS).ToPointer()));

    (*m_pSource)->SetGeoCS(geoCSUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ SpatialEntityDataSourceNet::GetNoDataValue()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetNoDataValue().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetNoDataValue(String^ nodatavalue)
    {
    Utf8String nodatavalueUtf8;
    BeStringUtilities::WCharToUtf8(nodatavalueUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(nodatavalue).ToPointer()));

    (*m_pSource)->SetNoDataValue(nodatavalueUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<UriNet^>^ SpatialEntityDataSourceNet::GetSisterFiles()
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
void SpatialEntityDataSourceNet::SetSisterFiles(List<UriNet^>^ sisterFiles)
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
String^ SpatialEntityDataSourceNet::GetElementName()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pSource)->GetElementName());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet::SpatialEntityDataSourceNet(UriNet^ uri, String^ type)
    {
    // Managed to native reality data source.
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    m_pSource = new SpatialEntityDataSourcePtr(SpatialEntityDataSource::Create(*ManagedToNativeUri(uri), typeUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Christian.Tye-Gingras         	02/2017
//-------------------------------------------------------------------------------------
System::IntPtr SpatialEntityDataSourceNet::GetPeer()
    { 
        return System::IntPtr((void *) m_pSource); 
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Christian.Tye-Gingras         	02/2017
//-------------------------------------------------------------------------------------
void SpatialEntityDataSourceNet::SetPeer(System::IntPtr newRDSN)
        {
        if (NULL != m_pSource)
            delete m_pSource;

        m_pSource = new SpatialEntityDataSourcePtr((*(SpatialEntityDataSourcePtr*) newRDSN.ToPointer()));
        }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet::~SpatialEntityDataSourceNet()
    {
    this->!SpatialEntityDataSourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet::!SpatialEntityDataSourceNet()
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
    : SpatialEntityDataSourceNet(UriNet::Create(uri), "wms")
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
    : SpatialEntityDataSourceNet(uri, "wms")
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
    : SpatialEntityDataSourceNet(UriNet::Create(uri), "osm")
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
SpatialEntityDataSourceNet^ MultiBandSourceNet::GetRedBand()
    {
    return NativeToManagedSpatialEntityDataSource(*(*m_pSource)->GetRedBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetRedBand(SpatialEntityDataSourceNet^ band)
    {
    (*m_pSource)->SetRedBand(*ManagedToNativeSpatialEntityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet^ MultiBandSourceNet::GetGreenBand()
    {
    return NativeToManagedSpatialEntityDataSource(*(*m_pSource)->GetGreenBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetGreenBand(SpatialEntityDataSourceNet^ band)
    {
    (*m_pSource)->SetGreenBand(*ManagedToNativeSpatialEntityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet^ MultiBandSourceNet::GetBlueBand()
    {
    return NativeToManagedSpatialEntityDataSource(*(*m_pSource)->GetBlueBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetBlueBand(SpatialEntityDataSourceNet^ band)
    {
    (*m_pSource)->SetBlueBand(*ManagedToNativeSpatialEntityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
SpatialEntityDataSourceNet^ MultiBandSourceNet::GetPanchromaticBand()
    {
    return NativeToManagedSpatialEntityDataSource(*(*m_pSource)->GetPanchromaticBand());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void MultiBandSourceNet::SetPanchromaticBand(SpatialEntityDataSourceNet^ band)
    {
    (*m_pSource)->SetPanchromaticBand(*ManagedToNativeSpatialEntityDataSource(band));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
MultiBandSourceNet::MultiBandSourceNet(UriNet^ uri, System::String^ type)
    : SpatialEntityDataSourceNet(uri, type)
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