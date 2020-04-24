/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/SimpleWSGBase.cpp"
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Vishal.Shingare                04/2020
//-------------------------------------------------------------------------------------
Utf8String WSGRequestManager::MakeBuddiCall(WString serviceName)
    {
    Utf8String RequestUrl = Utf8String();
    if (serviceName.EqualsI(L"RealityDataServices"))
        RequestUrl = UrlProvider::Urls::RealityDataServices.Get();
    else if (serviceName.EqualsI(L"ContextServices"))
        RequestUrl = UrlProvider::Urls::ContextServices.Get();

    return RequestUrl;
    }