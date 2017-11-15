/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/WmsSourceNet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "WmsSourceNet.h"

using namespace RealityPlatform;
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
WmsMapSettingsNet^ WmsMapSettingsNet::Create(String^ uri, List<double>^ bbox, String^ version, String^ layers, String^ csType, String^ csLabel)
    {
    return gcnew WmsMapSettingsNet(uri, bbox, version, layers, csType, csLabel);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetUri()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetUri().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetUri(String^ uri)
    {
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer()));

    (*m_pWmsMapSettings)->SetUri(uriUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<double>^ WmsMapSettingsNet::GetBBox()
    {
    DRange2d bbox = (*m_pWmsMapSettings)->GetBBox();

    List<double>^ managedBbox = gcnew List<double>();
    managedBbox->Add(bbox.low.x);
    managedBbox->Add(bbox.low.y);
    managedBbox->Add(bbox.high.x);
    managedBbox->Add(bbox.high.y);

    return managedBbox;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetBBox(List<double>^ bbox)
    {
    DRange2d nativeBbox = DRange2d::From(bbox[0], bbox[1], bbox[2], bbox[3]);

    (*m_pWmsMapSettings)->SetBBox(&nativeBbox);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
uint32_t WmsMapSettingsNet::GetMetaWidth()
    {
    return (*m_pWmsMapSettings)->GetMetaWidth();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetMetaWidth(uint32_t width)
    {
    (*m_pWmsMapSettings)->SetMetaWidth(width);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
uint32_t WmsMapSettingsNet::GetMetaHeight()
    {
    return (*m_pWmsMapSettings)->GetMetaHeight();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetMetaHeight(uint32_t height)
    {
    (*m_pWmsMapSettings)->SetMetaHeight(height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetVersion()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetVersion().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetVersion(String^ version)
    {
    Utf8String versionUtf8;
    BeStringUtilities::WCharToUtf8(versionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(version).ToPointer()));

    (*m_pWmsMapSettings)->SetVersion(versionUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetLayers()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetLayers().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetLayers(String^ layers)
    {
    Utf8String layersUtf8;
    BeStringUtilities::WCharToUtf8(layersUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(layers).ToPointer()));

    (*m_pWmsMapSettings)->SetLayers(layersUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetStyles()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetStyles().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetStyles(String^ styles)
    {
    Utf8String stylesUtf8;
    BeStringUtilities::WCharToUtf8(stylesUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(styles).ToPointer()));

    (*m_pWmsMapSettings)->SetStyles(stylesUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetCoordSysType()
{
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetCoordSysType().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetCoordSysType(String^ csType)
    {
    Utf8String csTypeUtf8;
    BeStringUtilities::WCharToUtf8(csTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csType).ToPointer()));

    (*m_pWmsMapSettings)->SetCoordSysType(csTypeUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetCoordSysLabel()
{
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetCoordSysLabel().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetCoordSysLabel(String^ csLabel)
    {
    Utf8String csLabelUtf8;
    BeStringUtilities::WCharToUtf8(csLabelUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csLabel).ToPointer()));

    (*m_pWmsMapSettings)->SetCoordSysLabel(csLabelUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetFormat()
    {
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetFormat().c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetFormat(String^ format)
    {
    Utf8String formatUtf8;
    BeStringUtilities::WCharToUtf8(formatUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(format).ToPointer()));

    (*m_pWmsMapSettings)->SetFormat(formatUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::GetVendorSpecific()
{
    marshal_context ctx;
    return ctx.marshal_as<String^>((*m_pWmsMapSettings)->GetVendorSpecific().c_str());
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetVendorSpecific(String^ vendorSpecific)
    {
    Utf8String vendorSpecificUtf8;
    BeStringUtilities::WCharToUtf8(vendorSpecificUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(vendorSpecific).ToPointer()));

    (*m_pWmsMapSettings)->SetVendorSpecific(vendorSpecificUtf8.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
bool WmsMapSettingsNet::IsTransparent()
    {
    return (*m_pWmsMapSettings)->IsTransparent();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void WmsMapSettingsNet::SetTransparency(bool isTransparent)
    {
    (*m_pWmsMapSettings)->SetTransparency(isTransparent);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ WmsMapSettingsNet::ToXml()
    {
    Utf8String nativeXmlFragment;
    (*m_pWmsMapSettings)->ToXml(nativeXmlFragment);

    marshal_context ctx;
    return ctx.marshal_as<String^>(nativeXmlFragment.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsMapSettingsNet::WmsMapSettingsNet(System::String^ uri, System::Collections::Generic::List<double>^ bbox, System::String^ version, System::String^ layers, System::String^ csType, System::String^ csLabel)
    {
    // Managed to native reality data source.
    Utf8String uriUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(uri).ToPointer()));

    DRange2d nativeBbox = DRange2d::From(bbox[0], bbox[1], bbox[2], bbox[3]);

    Utf8String versionUtf8;
    BeStringUtilities::WCharToUtf8(versionUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(version).ToPointer()));

    Utf8String layersUtf8;
    BeStringUtilities::WCharToUtf8(uriUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(layers).ToPointer()));

    Utf8String csTypeUtf8;
    BeStringUtilities::WCharToUtf8(csTypeUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csType).ToPointer()));

    Utf8String csLabelUtf8;
    BeStringUtilities::WCharToUtf8(csLabelUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(csLabel).ToPointer()));

    m_pWmsMapSettings = new WmsMapSettingsPtr(WmsMapSettings::Create(uriUtf8.c_str(), nativeBbox, versionUtf8.c_str(), layersUtf8.c_str(), csTypeUtf8.c_str(), csLabelUtf8.c_str()));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsMapSettingsNet::~WmsMapSettingsNet()
    {
    this->!WmsMapSettingsNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
WmsMapSettingsNet::!WmsMapSettingsNet()
    {
    if (0 != m_pWmsMapSettings)
        {
        delete m_pWmsMapSettings;
        m_pWmsMapSettings = 0;
        }
    }