/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/OsmSourceNet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Package.
#include "OsmSourceNet.h"

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
OsmResourceNet^ OsmResourceNet::Create(List<double>^ bbox)
    {
    return gcnew OsmResourceNet(bbox);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
List<String^>^ OsmResourceNet::GetAlternateUrlList()
    {
    marshal_context ctx;
    List<String^>^ managedUrls = gcnew List<String^>();
    String^ managedUrl;

    bvector<Utf8String> nativeUrls = (*m_pOsmResource)->GetAlternateUrlList();
    for (Utf8StringCR nativeUrl : nativeUrls)
        {
        managedUrl = ctx.marshal_as<String^>(nativeUrl.c_str());
        managedUrls->Add(managedUrl);
        }

    return managedUrls;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
void OsmResourceNet::SetAlternateUrlList(List<String^>^ urlList)
    {
    marshal_context ctx;
    bvector<Utf8String> nativeUrls;
    Utf8String urlUtf8;

    for each (String^ url in urlList)
        {
        BeStringUtilities::WCharToUtf8(urlUtf8, static_cast<wchar_t*>(Marshal::StringToHGlobalUni(url).ToPointer()));
        nativeUrls.push_back(urlUtf8);
        }

    (*m_pOsmResource)->SetAlternateUrlList(nativeUrls);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
String^ OsmResourceNet::ToXml()
    {
    marshal_context ctx;

    Utf8String nativeXmlFragment;
    (*m_pOsmResource)->ToXml(nativeXmlFragment);

    return ctx.marshal_as<String^>(nativeXmlFragment.c_str());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmResourceNet::OsmResourceNet(List<double>^ bbox)
    {
    // Managed to native reality data source.
    DRange2d nativeBbox = DRange2d::From(bbox[0], bbox[1], bbox[2], bbox[3]);

    m_pOsmResource = new OsmResourcePtr(OsmResource::Create(nativeBbox));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmResourceNet::~OsmResourceNet()
    {
    this->!OsmResourceNet();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2016
//-------------------------------------------------------------------------------------
OsmResourceNet::!OsmResourceNet()
    {
    if (0 != m_pOsmResource)
        {
        delete m_pOsmResource;
        m_pOsmResource = 0;
        }
    }