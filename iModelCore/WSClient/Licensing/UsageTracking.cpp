/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Licensing/UsageTracking.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Licensing/UsageTracking.h>
#include <Bentley/Base64Utilities.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

#ifndef BENTLEY_WINRT
#include <openssl/evp.h>
#endif

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

static IHttpHandlerPtr s_customHttpHandler;
static bool s_usageTrackingInitialized = false;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UsageTracking::Initialize(IHttpHandlerPtr customHttpHandler)
    {
    if (!s_usageTrackingInitialized)
        {
        s_customHttpHandler = customHttpHandler;
        s_usageTrackingInitialized = true;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UsageTracking::Uninintialize()
    {
    s_customHttpHandler = nullptr;
    s_usageTrackingInitialized = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt UsageTracking::RegisterUserUsages(Utf8StringCR dev, Utf8StringCR userId, Utf8StringCR prodId, Utf8StringCR projId, DateTimeCR usageDate, Utf8StringCR prodVer)
    {
    MobileTracking mt(dev, userId, prodId, projId, usageDate, prodVer);
    return RegisterUserUsages(mt);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt UsageTracking::RegisterUserUsages(bvector<MobileTracking> usages)
    {
    Json::Value usageList;
    for (MobileTracking mt : usages)
        {
        if (!mt.IsEmpty())
            {
            usageList.append(mt.ToJson());
            }
        }

    if (0 >= usageList.size())
        {
        return USAGE_NO_USAGES;
        }

    HttpClient client(nullptr, s_customHttpHandler);
    HttpRequest request = client.CreatePostRequest(GetUsageTrackingUrl());
    request.GetHeaders().SetContentType("application/json");

    Utf8String body = Json::FastWriter().write(usageList);
    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(usageList));
    request.SetRequestBody(requestBody);

    HttpResponse httpResponse = request.Perform();

    if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
        {
        return USAGE_ERROR;
        }

    return USAGE_SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt UsageTracking::RegisterUserUsages(MobileTracking usage)
    {
    bvector<MobileTracking> list;
    list.push_back(usage);
    return RegisterUserUsages(list);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UsageTracking::GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId)
    {
    Utf8PrintfString user(userGuid.c_str());
    user.ToLower();
    Utf8PrintfString getURL("%s/%s/%s", GetUsageTrackingUrl().c_str(), user.c_str(), deviceId.c_str());
    Utf8StringCR ver = VerifyClientMobile(userGuid, deviceId);

    HttpClient client(nullptr, s_customHttpHandler);
    HttpRequest request = client.CreateGetRequest(getURL);
    request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().AddValue("ClientAuth", ver.c_str());

    HttpResponse httpResponse = request.Perform();

    Json::Value usages = httpResponse.GetBody().AsJson();
    return usages;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UsageTracking::GetUserUsages(Utf8StringCR userGuid, Utf8StringCR deviceId, Utf8StringCR date)
    {
    Utf8PrintfString user(userGuid.c_str());
    user.ToLower();
    Utf8PrintfString getURL("%s/%s/%s/%s", GetUsageTrackingUrl().c_str(), user.c_str(), deviceId.c_str(), date.c_str());
    Utf8StringCR ver = VerifyClientMobile(userGuid, deviceId);

    HttpClient client(nullptr, s_customHttpHandler);
    HttpRequest request = client.CreateGetRequest(getURL);
    request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().AddValue("ClientAuth", ver.c_str());

    HttpResponse httpResponse = request.Perform();

    Json::Value usages = httpResponse.GetBody().AsJson();
    return usages;
    }

#ifndef BENTLEY_WINRT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ToHexString(const void *data, size_t dataSize)
    {
    const Byte* dataBytes = (Byte*) data;
    Utf8String hexString;
    hexString.reserve(dataSize * 2);
    char buf[3];
    for (size_t i = 0; i < dataSize; ++i)
        {
        sprintf(buf, "%02X", dataBytes[i]);
        hexString.append(buf);
        }
    return hexString;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dalius.Dobravolskas             09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CalcSha1(Utf8StringCR input, Utf8StringR hash)
    {
#ifdef BENTLEY_WINRT
    return false;

#else
    EVP_MD_CTX *mdctx;
    if ((mdctx = EVP_MD_CTX_create()) == NULL)
        return false;
    if (!EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL))
        return false;
    if (!EVP_DigestUpdate(mdctx, &input[0], input.size()))
        return false;

    unsigned int hashLen;
    unsigned char binaryHash[EVP_MAX_MD_SIZE];
    if (!EVP_DigestFinal_ex(mdctx, binaryHash, &hashLen))
        return false;

    EVP_MD_CTX_cleanup(mdctx);
    EVP_MD_CTX_destroy(mdctx);
    hash = ToHexString(binaryHash, hashLen);
    return true;
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UsageTracking::VerifyClientMobile(Utf8StringCR userGuid, Utf8StringCR deviceId)
    {
    const int arrayLen = 20;

    int pickValues[arrayLen] = {35, 20, 46, 30, 34, 14, 36, 21, 3, 37, 13, 12, 19, 47, 8, 42, 23, 18, 24, 45};
    const int numIterations = 6;

    Utf8PrintfString fullString("%s%s", userGuid.c_str(), deviceId.c_str());
    Utf8String pwString;
    for (int i = 0; i < arrayLen; i++)
        {
        pwString[i] = fullString[pickValues[i] % fullString.length()];
        }

    Utf8String hash;
    for (int i = 0; i < numIterations; i++)
        {
        CalcSha1(pwString, hash);
        pwString = hash;
        }

    return hash;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UsageTracking::GetUsageTrackingUrl()
    {
    return UrlProvider::GetUsageTrackingUrl();
    }
