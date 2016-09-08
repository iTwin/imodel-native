/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/HttpTraversalEngineNet/HttpTraversalEngineCli.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpTraversalEngineCli.h"

#include <msclr/marshal.h>

using namespace msclr::interop;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

using namespace RealityPlatform;
using namespace RealityServicesCli;


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpTraversalObserverWrapper::HttpTraversalObserverWrapper(gcroot<IHttpTraversalObserverWrapper^> managedHttpObserver)
    :m_managedHttpObserver(managedHttpObserver)
    {}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpTraversalObserverWrapper::OnFileListed(bvector<Utf8String>& fileList, Utf8CP file)
    {
    marshal_context ctx;

    // Process listed file.
    if (m_managedHttpObserver->OnFileListed_AddToQueue(ctx.marshal_as<String^>(file)))
        fileList.push_back(file);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpTraversalObserverWrapper::OnFileDownloaded(Utf8CP file)
    {
    marshal_context ctx;

    // Native to managed.
    m_managedHttpObserver->OnFileDownloaded(ctx.marshal_as<String^>(file));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpTraversalObserverWrapper::OnDataExtracted(HttpDataCR data)
    {
    marshal_context ctx;

    // Native to managed.
    HttpDataWrapper^ dataWrapper = HttpDataWrapper::Create();

    dataWrapper->SetName(ctx.marshal_as<String^>(data.GetName().c_str()));
    dataWrapper->SetUrl(ctx.marshal_as<String^>(data.GetUrl().c_str()));
    dataWrapper->SetCompoundType(ctx.marshal_as<String^>(data.GetCompoundType().c_str()));
    dataWrapper->SetSize(data.GetSize());
    dataWrapper->SetResolution(ctx.marshal_as<String^>(data.GetResolution().c_str()));
    dataWrapper->SetProvider(ctx.marshal_as<String^>(data.GetProvider().c_str()));
    dataWrapper->SetDataType(ctx.marshal_as<String^>(data.GetDataType().c_str()));
    dataWrapper->SetLocationInCompound(ctx.marshal_as<String^>(data.GetLocationInCompound().c_str()));
    dataWrapper->SetDate(ctx.marshal_as<String^>(data.GetDate().ToString().c_str()));

    // Convert DRange2d to List<double>.
    List<double>^ ptList = gcnew List<double>();
    DPoint2d footprintPts[4];
    data.GetFootprint().Get4Corners(footprintPts);
    for (size_t i = 0; i < 4; ++i)
        {
        ptList->Add(footprintPts[i].x);
        ptList->Add(footprintPts[i].y);
        }
    ptList->Add(footprintPts[0].x);
    ptList->Add(footprintPts[0].y);
    dataWrapper->SetFootprint(ptList);

    HttpThumbnailWrapper^ thumbnailWrapper = HttpThumbnailWrapper::Create();
    thumbnailWrapper->SetProvenance(ctx.marshal_as<String^>(data.GetThumbnail().GetProvenance().c_str()));
    thumbnailWrapper->SetFormat(ctx.marshal_as<String^>(data.GetThumbnail().GetFormat().c_str()));
    thumbnailWrapper->SetWidth(data.GetThumbnail().GetWidth());
    thumbnailWrapper->SetHeight(data.GetThumbnail().GetHeight());
    thumbnailWrapper->SetStamp(ctx.marshal_as<String^>(data.GetThumbnail().GetStamp().ToString().c_str()));    
    thumbnailWrapper->SetGenerationDetails(ctx.marshal_as<String^>(data.GetThumbnail().GetGenerationDetails().c_str()));

    // Convert bvector to array.    
    bvector<System::Byte> nativeData = data.GetThumbnail().GetData();
    int size = static_cast<int>(nativeData.size());
    List<System::Byte>^ managedData = gcnew List<System::Byte>(size);
    for (int i = 0; i < size; ++i)
        {
        managedData->Add(nativeData[i]);
        }
    thumbnailWrapper->SetData(managedData);

    dataWrapper->SetThumbnail(thumbnailWrapper);

    HttpMetadataWrapper^ metadataWrapper = HttpMetadataWrapper::Create();
    metadataWrapper->SetProvenance(ctx.marshal_as<String^>(data.GetMetadata().GetProvenance().c_str()));
    metadataWrapper->SetDescription(ctx.marshal_as<String^>(data.GetMetadata().GetDescription().c_str()));
    metadataWrapper->SetContactInfo(ctx.marshal_as<String^>(data.GetMetadata().GetContactInfo().c_str()));
    metadataWrapper->SetLegal(ctx.marshal_as<String^>(data.GetMetadata().GetLegal().c_str()));
    metadataWrapper->SetFormat(ctx.marshal_as<String^>(data.GetMetadata().GetFormat().c_str()));
    metadataWrapper->SetData(ctx.marshal_as<String^>(data.GetMetadata().GetData().c_str()));
    dataWrapper->SetMetadata(metadataWrapper);
    
    HttpServerWrapper^ serverWrapper = HttpServerWrapper::Create();
    serverWrapper->SetProtocol(ctx.marshal_as<String^>(data.GetServer().GetProtocol().c_str()));
    serverWrapper->SetName(ctx.marshal_as<String^>(data.GetServer().GetName().c_str()));
    serverWrapper->SetUrl(ctx.marshal_as<String^>(data.GetServer().GetUrl().c_str()));
    serverWrapper->SetContactInfo(ctx.marshal_as<String^>(data.GetServer().GetContactInfo().c_str()));
    serverWrapper->SetLegal(ctx.marshal_as<String^>(data.GetServer().GetLegal().c_str()));
    serverWrapper->SetOnline(data.GetServer().IsOnline());
    serverWrapper->SetLastCheck(ctx.marshal_as<String^>(data.GetServer().GetLastCheck().ToString().c_str()));
    serverWrapper->SetLastTimeOnline(ctx.marshal_as<String^>(data.GetServer().GetLastTimeOnline().ToString().c_str()));
    serverWrapper->SetLatency(data.GetServer().GetLatency());
    serverWrapper->SetState(ctx.marshal_as<String^>(data.GetServer().GetState().c_str()));
    serverWrapper->SetServerType(ctx.marshal_as<String^>(data.GetServer().GetType().c_str()));
    dataWrapper->SetServer(serverWrapper);
    
    m_managedHttpObserver->OnDataExtracted(dataWrapper);
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientWrapper^ HttpClientWrapper::ConnectTo(String^ serverUrl)
    {
    return gcnew HttpClientWrapper(serverUrl, nullptr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientWrapper^ HttpClientWrapper::ConnectTo(String^ serverUrl, String^ serverName)
    {
    return gcnew HttpClientWrapper(serverUrl, serverName);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpStatusWrapper HttpClientWrapper::DownloadContent(String^ outputPath)
    {
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return HttpStatusWrapper::ClientError;

    Utf8String outputPathUtf8;
    BeStringUtilities::WCharToUtf8(outputPathUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(outputPath).ToPointer()));

    return static_cast<HttpStatusWrapper>((*m_pClient)->DownloadContent());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
String^ HttpClientWrapper::GetServerUrl()
    {
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return "";

    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pClient)->GetServerUrl().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    7/2016
//-------------------------------------------------------------------------------------
String^ HttpClientWrapper::GetServerName()
    {
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return "";

    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pClient)->GetServerName().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpStatusWrapper HttpClientWrapper::GetFileList(List<System::String^>^ fileList)
    {
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return HttpStatusWrapper::ClientError;

    bvector<Utf8String> nativeFileList;
    HttpStatusWrapper status = static_cast<HttpStatusWrapper>((*m_pClient)->GetFileList(nativeFileList));

    // Convert native to managed.
    //&&JFC: TODO

    return status;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpStatusWrapper HttpClientWrapper::GetData()
    {   
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return HttpStatusWrapper::ClientError;

    return static_cast<HttpStatusWrapper>((*m_pClient)->GetData());        
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpClientWrapper::SetObserver(IHttpTraversalObserverWrapper^ traversalObserver)
    {
    // Make sure client is init and valid.
    if (NULL == m_pClient && (*m_pClient).IsValid())
        return;

    HttpTraversalObserverWrapper* pTraversalObserverWrapper = new HttpTraversalObserverWrapper(traversalObserver);
    (*m_pClient)->SetObserver(pTraversalObserverWrapper);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientWrapper::HttpClientWrapper(String^ serverUrl, String^ serverName)
    {
    Utf8String urlUtf8, nameUtf8;
    BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(serverUrl).ToPointer()));
    BeStringUtilities::WCharToUtf8(nameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(serverName).ToPointer()));

    m_pClient = new HttpClientPtr(HttpClient::ConnectTo(urlUtf8.c_str(), nameUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientWrapper::~HttpClientWrapper()
    {
    this->!HttpClientWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpClientWrapper::!HttpClientWrapper()
    {
    if (0 != m_pClient)
        {
        delete m_pClient;
        m_pClient = 0;
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpDataWrapper^ HttpDataWrapper::Create()
    {
    return gcnew HttpDataWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetName()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetName().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetName(System::String^ name)
    {
    Utf8String nameUtf8;
    BeStringUtilities::WCharToUtf8(nameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));

    (*m_pData)->SetName(nameUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetUrl()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetUrl().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetUrl(System::String^ url)
    {
    Utf8String urlUtf8;
    BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer()));

    (*m_pData)->SetUrl(urlUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetCompoundType()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetCompoundType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetCompoundType(System::String^ type)
    {
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    (*m_pData)->SetCompoundType(typeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
uint64_t HttpDataWrapper::GetSize()
    {
    return (*m_pData)->GetSize();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetSize(uint64_t size)
    {
    (*m_pData)->SetSize(size);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetResolution()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetResolution().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetResolution(System::String^ resolution)
    {
    Utf8String resUtf8;
    BeStringUtilities::WCharToUtf8(resUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(resolution).ToPointer()));

    (*m_pData)->SetResolution(resUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetProvider()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetProvider().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetProvider(System::String^ provider)
    {
    Utf8String providerUtf8;
    BeStringUtilities::WCharToUtf8(providerUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(provider).ToPointer()));

    (*m_pData)->SetProvider(providerUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetDataType()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetDataType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetDataType(System::String^ type)
    {
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    (*m_pData)->SetDataType(typeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetLocationInCompound()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetLocationInCompound().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetLocationInCompound(System::String^ location)
    {
    Utf8String locationUtf8;
    BeStringUtilities::WCharToUtf8(locationUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(location).ToPointer()));

    (*m_pData)->SetLocationInCompound(locationUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpDataWrapper::GetDate()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pData)->GetDate().ToString().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetDate(System::String^ date)
    {
    Utf8String dateUtf8;
    BeStringUtilities::WCharToUtf8(dateUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(date).ToPointer()));

    BentleyApi::DateTime dateTime;
    BentleyApi::DateTime::FromString(dateTime, dateUtf8.c_str());

    (*m_pData)->SetDate(dateTime);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
List<double>^ HttpDataWrapper::GetFootprint()
    {
    List<double>^ ptList = gcnew List<double>();
    DRange2d footprint = (*m_pData)->GetFootprint();

    DPoint2d footprintPts[4];
    footprint.Get4Corners(footprintPts);
    for (size_t i = 0; i < 4; ++i)
        {
        ptList->Add(footprintPts[i].x);
        ptList->Add(footprintPts[i].y);
        }

    // Complete polygon (first point == last point).
    ptList->Add(footprintPts[0].x);
    ptList->Add(footprintPts[0].y);

    return ptList;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetFootprint(List<double>^ footprint)
    {
    BeAssert(10 == footprint->Count);

    DPoint2d footprintPts[5];

    int j = 0;
    for (int i = 0; i < 5; ++i)
        {
        footprintPts[i].x = footprint[j++];
        footprintPts[i].y = footprint[j++];
        }

    DRange2d nativeFootprint = DRange2d::From(footprintPts, 5);
    (*m_pData)->SetFootprint(nativeFootprint);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpThumbnailWrapper^ HttpDataWrapper::GetThumbnail()
    {
    marshal_context ctx;
    HttpThumbnailCR nativeThumbnail = (*m_pData)->GetThumbnail();

    // Convert native to managed.
    HttpThumbnailWrapper^ managedThumbnail = HttpThumbnailWrapper::Create();
    managedThumbnail->SetProvenance(ctx.marshal_as<String^>(nativeThumbnail.GetProvenance().c_str()));
    managedThumbnail->SetFormat(ctx.marshal_as<String^>(nativeThumbnail.GetFormat().c_str()));
    managedThumbnail->SetWidth(nativeThumbnail.GetWidth());
    managedThumbnail->SetHeight(nativeThumbnail.GetHeight());
    managedThumbnail->SetStamp(ctx.marshal_as<String^>(nativeThumbnail.GetStamp().ToString().c_str()));    
    managedThumbnail->SetGenerationDetails(ctx.marshal_as<String^>(nativeThumbnail.GetGenerationDetails().c_str()));

    bvector<System::Byte> nativeData = nativeThumbnail.GetData();
    int size = static_cast<int>(nativeData.size());
    List<System::Byte>^ data = gcnew List<System::Byte>(size); 
    for (int i = 0; i < size; ++i)
        {
        data->Add(nativeData[i]);
        }
    managedThumbnail->SetData(data);

    return managedThumbnail;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetThumbnail(HttpThumbnailWrapper^ thumbnail)
    {
    HttpThumbnailPtr nativeThumbnail = HttpThumbnail::Create();

    // Convert managed to native.
    Utf8String provenance;
    BeStringUtilities::WCharToUtf8(provenance, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(thumbnail->GetProvenance()).ToPointer()));
    nativeThumbnail->SetProvenance(provenance.c_str());

    Utf8String format;
    BeStringUtilities::WCharToUtf8(format, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(thumbnail->GetFormat()).ToPointer()));
    nativeThumbnail->SetFormat(format.c_str());

    nativeThumbnail->SetWidth(thumbnail->GetWidth());
    nativeThumbnail->SetHeight(thumbnail->GetHeight());

    Utf8String dateStr;
    BeStringUtilities::WCharToUtf8(dateStr, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(thumbnail->GetStamp()).ToPointer()));
    BentleyApi::DateTime date;
    BentleyApi::DateTime::FromString(date, dateStr.c_str());
    nativeThumbnail->SetStamp(date);

    Utf8String details;
    BeStringUtilities::WCharToUtf8(details, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(thumbnail->GetGenerationDetails()).ToPointer()));
    nativeThumbnail->SetGenerationDetails(details.c_str());

    List<System::Byte> data = thumbnail->GetData();
    bvector<System::Byte> nativeData = bvector<System::Byte>();
    for (int i = 0; i < data.Count; ++i)
        {
        nativeData.push_back(data[i]);
        }
    nativeThumbnail->SetData(nativeData);

    (*m_pData)->SetThumbnail(*nativeThumbnail);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpMetadataWrapper^ HttpDataWrapper::GetMetadata()
    {
    marshal_context ctx;
    HttpMetadataCR nativeMetadata = (*m_pData)->GetMetadata();

    // Convert native to managed.
    HttpMetadataWrapper^ managedMetadata = HttpMetadataWrapper::Create();
    managedMetadata->SetProvenance(ctx.marshal_as<String^>(nativeMetadata.GetProvenance().c_str()));
    managedMetadata->SetDescription(ctx.marshal_as<String^>(nativeMetadata.GetDescription().c_str()));
    managedMetadata->SetContactInfo(ctx.marshal_as<String^>(nativeMetadata.GetContactInfo().c_str()));
    managedMetadata->SetLegal(ctx.marshal_as<String^>(nativeMetadata.GetLegal().c_str()));
    managedMetadata->SetFormat(ctx.marshal_as<String^>(nativeMetadata.GetFormat().c_str()));
    managedMetadata->SetData(ctx.marshal_as<String^>(nativeMetadata.GetData().c_str()));

    return managedMetadata;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetMetadata(HttpMetadataWrapper^ metadata)
    {
    HttpMetadataPtr nativeMetadata = HttpMetadata::Create();

    // Convert managed to native
    Utf8String provenance;
    BeStringUtilities::WCharToUtf8(provenance, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetProvenance()).ToPointer()));
    nativeMetadata->SetProvenance(provenance.c_str());

    Utf8String description;
    BeStringUtilities::WCharToUtf8(description, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetDescription()).ToPointer()));
    nativeMetadata->SetDescription(description.c_str());

    Utf8String info;
    BeStringUtilities::WCharToUtf8(info, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetContactInfo()).ToPointer()));
    nativeMetadata->SetContactInfo(info.c_str());

    Utf8String legal;
    BeStringUtilities::WCharToUtf8(legal, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetLegal()).ToPointer()));
    nativeMetadata->SetLegal(legal.c_str());

    Utf8String format;
    BeStringUtilities::WCharToUtf8(format, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetFormat()).ToPointer()));
    nativeMetadata->SetFormat(format.c_str());

    Utf8String data;
    BeStringUtilities::WCharToUtf8(data, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(metadata->GetData()).ToPointer()));
    nativeMetadata->SetData(data.c_str());

    (*m_pData)->SetMetadata(*nativeMetadata);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpServerWrapper^ HttpDataWrapper::GetServer()
    {
    marshal_context ctx;
    HttpServerCR nativeServer = (*m_pData)->GetServer();

    // Convert native to managed.
    HttpServerWrapper^ managedServer = HttpServerWrapper::Create();
    managedServer->SetProtocol(ctx.marshal_as<String^>(nativeServer.GetProtocol().c_str()));
    managedServer->SetName(ctx.marshal_as<String^>(nativeServer.GetName().c_str()));
    managedServer->SetUrl(ctx.marshal_as<String^>(nativeServer.GetUrl().c_str()));
    managedServer->SetContactInfo(ctx.marshal_as<String^>(nativeServer.GetContactInfo().c_str()));
    managedServer->SetLegal(ctx.marshal_as<String^>(nativeServer.GetLegal().c_str()));
    managedServer->SetOnline(nativeServer.IsOnline());
    managedServer->SetLastCheck(ctx.marshal_as<String^>(nativeServer.GetLastCheck().ToString().c_str()));
    managedServer->SetLastTimeOnline(ctx.marshal_as<String^>(nativeServer.GetLastTimeOnline().ToString().c_str()));
    managedServer->SetLatency(nativeServer.GetLatency());
    managedServer->SetState(ctx.marshal_as<String^>(nativeServer.GetState().c_str()));
    managedServer->SetServerType(ctx.marshal_as<String^>(nativeServer.GetType().c_str()));

    return managedServer;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpDataWrapper::SetServer(HttpServerWrapper^ server)
    {
    HttpServerPtr nativeServer = HttpServer::Create("");

    // Convert managed to native
    Utf8String protocol;
    BeStringUtilities::WCharToUtf8(protocol, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetProtocol()).ToPointer()));
    nativeServer->SetProtocol(protocol.c_str());

    Utf8String name;
    BeStringUtilities::WCharToUtf8(name, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetName()).ToPointer()));
    nativeServer->SetName(name.c_str());

    Utf8String url;
    BeStringUtilities::WCharToUtf8(url, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetUrl()).ToPointer()));
    nativeServer->SetUrl(url.c_str());

    Utf8String info;
    BeStringUtilities::WCharToUtf8(info, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetContactInfo()).ToPointer()));
    nativeServer->SetContactInfo(info.c_str());

    Utf8String legal;
    BeStringUtilities::WCharToUtf8(legal, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetLegal()).ToPointer()));
    nativeServer->SetLegal(legal.c_str());

    nativeServer->SetOnline(server->IsOnline());

    Utf8String lastCheckStr;
    BeStringUtilities::WCharToUtf8(lastCheckStr, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetLastCheck()).ToPointer()));
    BentleyApi::DateTime lastCheck;
    BentleyApi::DateTime::FromString(lastCheck, lastCheckStr.c_str());
    nativeServer->SetLastCheck(lastCheck);

    Utf8String lastTimeOnlineStr;
    BeStringUtilities::WCharToUtf8(lastTimeOnlineStr, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetLastTimeOnline()).ToPointer()));
    BentleyApi::DateTime lastTimeOnline;
    BentleyApi::DateTime::FromString(lastTimeOnline, lastTimeOnlineStr.c_str());
    nativeServer->SetLastTimeOnline(lastTimeOnline);

    nativeServer->SetLatency(server->GetLatency());

    Utf8String state;
    BeStringUtilities::WCharToUtf8(state, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetState()).ToPointer()));
    nativeServer->SetState(state.c_str());

    Utf8String type;
    BeStringUtilities::WCharToUtf8(type, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(server->GetServerType()).ToPointer()));
    nativeServer->SetType(type.c_str());

    (*m_pData)->SetServer(*nativeServer);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    4/2016
//-------------------------------------------------------------------------------------
HttpDataWrapper::HttpDataWrapper()
    {
    m_pData = new HttpDataPtr(HttpData::Create());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpDataWrapper::~HttpDataWrapper()
    {
    this->!HttpDataWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpDataWrapper::!HttpDataWrapper()
    {
    if (0 != m_pData)
        {
        delete m_pData;
        m_pData = 0;
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpThumbnailWrapper^ HttpThumbnailWrapper::Create()
    {
    return gcnew HttpThumbnailWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpThumbnailWrapper::GetProvenance()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pThumbnail)->GetProvenance().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetProvenance(System::String^ provenance)
    {
    Utf8String provenanceUtf8;
    BeStringUtilities::WCharToUtf8(provenanceUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(provenance).ToPointer()));

    (*m_pThumbnail)->SetProvenance(provenanceUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpThumbnailWrapper::GetFormat()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pThumbnail)->GetFormat().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetFormat(System::String^ format)
    {
    Utf8String formatUtf8;
    BeStringUtilities::WCharToUtf8(formatUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer()));

    (*m_pThumbnail)->SetFormat(formatUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
uint32_t HttpThumbnailWrapper::GetWidth()
    {
    return (*m_pThumbnail)->GetWidth();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetWidth(uint32_t width)
    {
    (*m_pThumbnail)->SetWidth(width);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
uint32_t HttpThumbnailWrapper::GetHeight()
    {
    return (*m_pThumbnail)->GetHeight();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetHeight(uint32_t height)
    {
    (*m_pThumbnail)->SetHeight(height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpThumbnailWrapper::GetStamp()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pThumbnail)->GetStamp().ToString().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetStamp(System::String^ date)
    {
    Utf8String dateUtf8;
    BeStringUtilities::WCharToUtf8(dateUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(date).ToPointer()));

    BentleyApi::DateTime dateTime;
    BentleyApi::DateTime::FromString(dateTime, dateUtf8.c_str());

    (*m_pThumbnail)->SetStamp(dateTime);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
List<System::Byte>^ HttpThumbnailWrapper::GetData()
    {
    bvector<System::Byte> nativeData = (*m_pThumbnail)->GetData();
    int size = static_cast<int>(nativeData.size());
    List<System::Byte>^ data = gcnew List<System::Byte>(size);

    // Native to managed.    
    for (int i = 0; i < size; ++i)
        {
        data->Add(nativeData[i]);
        }

    return data;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetData(List<System::Byte>^ data)
    {
    // Managed to native.
    bvector<System::Byte> nativeData = bvector<System::Byte>();
    for (int i = 0; i < data->Count; ++i)
        {
        nativeData.push_back(data[i]);
        }

    (*m_pThumbnail)->SetData(nativeData);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpThumbnailWrapper::GetGenerationDetails()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pThumbnail)->GetGenerationDetails().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpThumbnailWrapper::SetGenerationDetails(System::String^ details)
    {
    Utf8String detailsUtf8;
    BeStringUtilities::WCharToUtf8(detailsUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(details).ToPointer()));

    (*m_pThumbnail)->SetGenerationDetails(detailsUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpThumbnailWrapper::HttpThumbnailWrapper()
    {
    m_pThumbnail = new HttpThumbnailPtr(HttpThumbnail::Create());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpThumbnailWrapper::~HttpThumbnailWrapper()
    {
    this->!HttpThumbnailWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpThumbnailWrapper::!HttpThumbnailWrapper()
    {
    if (0 != m_pThumbnail)
        {
        delete m_pThumbnail;
        m_pThumbnail = 0;
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpMetadataWrapper^ HttpMetadataWrapper::Create()
    {
    return gcnew HttpMetadataWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetProvenance()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetProvenance().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetProvenance(System::String^ provenance)
    {
    Utf8String provenanceUtf8;
    BeStringUtilities::WCharToUtf8(provenanceUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(provenance).ToPointer()));

    (*m_pMetadata)->SetProvenance(provenanceUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetDescription()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetDescription().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetDescription(System::String^ description)
    {
    Utf8String descriptionUtf8;
    BeStringUtilities::WCharToUtf8(descriptionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(description).ToPointer()));

    (*m_pMetadata)->SetDescription(descriptionUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetContactInfo()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetContactInfo().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetContactInfo(System::String^ info)
    {
    Utf8String infoUtf8;
    BeStringUtilities::WCharToUtf8(infoUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(info).ToPointer()));

    (*m_pMetadata)->SetContactInfo(infoUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetLegal()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetLegal().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetLegal(System::String^ legal)
    {
    Utf8String legalUtf8;
    BeStringUtilities::WCharToUtf8(legalUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(legal).ToPointer()));

    (*m_pMetadata)->SetLegal(legalUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetFormat()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetFormat().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetFormat(System::String^ format)
    {
    Utf8String formatUtf8;
    BeStringUtilities::WCharToUtf8(formatUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer()));

    (*m_pMetadata)->SetFormat(formatUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpMetadataWrapper::GetData()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pMetadata)->GetData().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpMetadataWrapper::SetData(System::String^ data)
    {
    Utf8String dataUtf8;
    BeStringUtilities::WCharToUtf8(dataUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(data).ToPointer()));

    (*m_pMetadata)->SetData(dataUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpMetadataWrapper::HttpMetadataWrapper()
    {
    m_pMetadata = new HttpMetadataPtr(HttpMetadata::Create());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpMetadataWrapper::~HttpMetadataWrapper()
    {
    this->!HttpMetadataWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpMetadataWrapper::!HttpMetadataWrapper()
    {
    if (0 != m_pMetadata)
        {
        delete m_pMetadata;
        m_pMetadata = 0;
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpServerWrapper^ HttpServerWrapper::Create()
    {
    return gcnew HttpServerWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetProtocol()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetProtocol().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetProtocol(System::String^ protocol)
    {
    Utf8String protocolUtf8;
    BeStringUtilities::WCharToUtf8(protocolUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(protocol).ToPointer()));

    (*m_pServer)->SetProtocol(protocolUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetName()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetName().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetName(System::String^ name)
    {
    Utf8String nameUtf8;
    BeStringUtilities::WCharToUtf8(nameUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(name).ToPointer()));

    (*m_pServer)->SetName(nameUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetUrl()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetUrl().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetUrl(System::String^ url)
    {
    Utf8String urlUtf8;
    BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer()));

    (*m_pServer)->SetUrl(urlUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetContactInfo()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetContactInfo().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetContactInfo(System::String^ info)
    {
    Utf8String infoUtf8;
    BeStringUtilities::WCharToUtf8(infoUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(info).ToPointer()));

    (*m_pServer)->SetContactInfo(infoUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetLegal()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetLegal().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetLegal(System::String^ legal)
    {
    Utf8String legalUtf8;
    BeStringUtilities::WCharToUtf8(legalUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(legal).ToPointer()));

    (*m_pServer)->SetLegal(legalUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
bool HttpServerWrapper::IsOnline()
    {
    return (*m_pServer)->IsOnline();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetOnline(bool online)
    {
    (*m_pServer)->SetOnline(online);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetLastCheck()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetLastCheck().ToString().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetLastCheck(System::String^ time)
    {
    Utf8String timeUtf8;
    BeStringUtilities::WCharToUtf8(timeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(time).ToPointer()));

    BentleyApi::DateTime dateTime;
    BentleyApi::DateTime::FromString(dateTime, timeUtf8.c_str());

    (*m_pServer)->SetLastCheck(dateTime);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetLastTimeOnline()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetLastTimeOnline().ToString().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetLastTimeOnline(System::String^ time)
    {
    Utf8String timeUtf8;
    BeStringUtilities::WCharToUtf8(timeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(time).ToPointer()));

    BentleyApi::DateTime dateTime;
    BentleyApi::DateTime::FromString(dateTime, timeUtf8.c_str());

    (*m_pServer)->SetLastTimeOnline(dateTime);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
double HttpServerWrapper::GetLatency()
    {
    return (*m_pServer)->GetLatency();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetLatency(double latency)
    {
    (*m_pServer)->SetLatency(latency);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetState()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetState().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetState(System::String^ state)
    {
    Utf8String stateUtf8;
    BeStringUtilities::WCharToUtf8(stateUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(state).ToPointer()));

    (*m_pServer)->SetState(stateUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
System::String^ HttpServerWrapper::GetServerType()
    {
    marshal_context ctx;
    return ctx.marshal_as<System::String^>((*m_pServer)->GetType().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
void HttpServerWrapper::SetServerType(System::String^ type)
    {
    Utf8String typeUtf8;
    BeStringUtilities::WCharToUtf8(typeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(type).ToPointer()));

    (*m_pServer)->SetType(typeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpServerWrapper::HttpServerWrapper()
    {
    m_pServer = new HttpServerPtr(HttpServer::Create(""));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpServerWrapper::~HttpServerWrapper()
    {
    this->!HttpServerWrapper();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    5/2016
//-------------------------------------------------------------------------------------
HttpServerWrapper::!HttpServerWrapper()
    {
    if (0 != m_pServer)
        {
        delete m_pServer;
        m_pServer = 0;
        }
    }

