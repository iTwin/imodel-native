/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Licensing/FeatureTracking.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Licensing/FeatureTracking.h>
#include <Bentley/Base64Utilities.h>
#include <DgnClientFx/Utils/Http/HttpClient.h>

#ifndef BENTLEY_WINRT
#include <openssl/evp.h>
#endif

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

static IHttpHandlerPtr s_httpHandler;
static bool s_featureTrackingInitialized = false;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureTracking::Initialize(IHttpHandlerPtr customHttpHandler)
    {
    if (!s_featureTrackingInitialized)
        {
        s_httpHandler = UrlProvider::GetSecurityConfigurator(customHttpHandler);
        s_featureTrackingInitialized = true;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
void FeatureTracking::Uninitialize()
    {
    s_httpHandler = nullptr;
    s_featureTrackingInitialized = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FeatureTracking::GetServiceUrl()
    {
    return UrlProvider::Urls::FeatureTracking.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FeatureTracking::RegisterFeatureUsage(Utf8StringCR dev, Utf8StringCR userId, Utf8StringCR prodId, Utf8StringCR prodVer, Utf8StringCR projId, Utf8StringCR featureId, DateTimeCR usageStartDate, DateTimeCR usageEndDate)
    {
    FeatureTrackingData ftd(dev, userId, prodId, prodVer, projId, featureId, usageStartDate, usageEndDate);
    return RegisterFeatureUsage(ftd);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FeatureTracking::RegisterFeatureUsage(bvector<FeatureTrackingData> usages)
    {
    Json::Value usageList;
    for (FeatureTrackingData ftd : usages)
        {
        if (!ftd.IsEmpty())
            {
            usageList.append(ftd.ToJson());
            }
        }

    if (0 >= usageList.size())
        {
        return FEATURE_TRACKING_NO_USAGES;
        }

    HttpClient client(nullptr, s_httpHandler);
    HttpRequest request = client.CreatePostRequest(GetServiceUrl());
    request.GetHeaders().SetContentType("application/json");

    Utf8String body = Json::FastWriter().write(usageList);
    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(usageList));
    request.SetRequestBody(requestBody);

    HttpResponse httpResponse = request.Perform();

    if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
        {
        return FEATURE_TRACKING_ERROR;
        }

    return FEATURE_TRACKING_SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt FeatureTracking::RegisterFeatureUsage(FeatureTrackingData usage)
    {
    bvector<FeatureTrackingData> list;
    list.push_back(usage);
    return RegisterFeatureUsage(list);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FeatureTracking::GetUserFeatureUsages(Utf8StringCR userGuid, Utf8StringCR deviceId)
    {
    Utf8PrintfString user(userGuid.c_str());
    user.ToLower();
    Utf8PrintfString getURL("%s/%s/%s", GetServiceUrl().c_str(), user.c_str(), deviceId.c_str());
    Utf8StringCR ver = VerifyClientMobile(userGuid, deviceId);

    HttpClient client(nullptr, s_httpHandler);
    HttpRequest request = client.CreateGetRequest(getURL);
    request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().AddValue("ClientAuth", ver.c_str());

    HttpResponse httpResponse = request.Perform();

    Json::Value usages = httpResponse.GetBody().AsJson();
    return usages;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FeatureTracking::GetUserFeatureUsages(Utf8StringCR userGuid, Utf8StringCR deviceId, Utf8StringCR date)
    {
    Utf8PrintfString user(userGuid.c_str());
    user.ToLower();
    Utf8PrintfString url("%s/%s/%s/%s", GetServiceUrl().c_str(), user.c_str(), deviceId.c_str(), date.c_str());
    Utf8StringCR ver = VerifyClientMobile(userGuid, deviceId);

    HttpClient client(nullptr, s_httpHandler);
    HttpRequest request = client.CreateGetRequest(url);
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
* @bsimethod                                                    Wil.Maier       04/2106
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FeatureTracking::VerifyClientMobile(Utf8StringCR userGuid, Utf8StringCR deviceId)
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
