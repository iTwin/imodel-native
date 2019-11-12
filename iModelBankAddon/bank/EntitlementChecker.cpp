/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "EntitlementChecker.h"
#include "../common/ConversionUtils.h"
#include <Bentley/BeSystemInfo.h>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <thread>

using namespace IModelBank;

static Napi::ObjectReference s_http;

#if defined(BENTLEY_WIN32)
/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                        Grigas.Petraitis            06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getHostName()
{
    WString computerName;
    DWORD nameLength = 0;
    ::GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, NULL, &nameLength);
    if (0 != nameLength)
    {
        computerName.resize(nameLength);
        if (TRUE != ::GetComputerNameExW(ComputerNamePhysicalDnsFullyQualified, (WCharP)computerName.data(), &nameLength))
            BeAssert(false);
    }
    if (computerName.empty())
    {
        WCharCP computerNameP = NULL;
        if (NULL != (computerNameP = ::_wgetenv(L"COMPUTERNAME")))
            computerName.assign(computerNameP);
        else
            computerName.assign(L"UnknownHostName");
    }
    return Utf8String(computerName.c_str());
}
#else
static Utf8String getHostName()
{
    return BeSystemInfo::GetMachineName();
}
#endif

static void handleEntitlementError(Utf8StringCR msg)
{
// #ifdef NDEBUG
//     // In a production build, terminate on an entitlement error.
//     // TODO: make sure that napi_fatal_error does not show a callstack in a production build. If it does, then just call exit.
//     // exit(1);
//     napi_fatal_error("", NAPI_AUTO_LENGTH, msg.c_str(), NAPI_AUTO_LENGTH);
// #else
    if (getenv("IMODEL_BANK_ENTITLEMENT_CHECK_ENFORCE"))
        napi_fatal_error("", NAPI_AUTO_LENGTH, msg.c_str(), NAPI_AUTO_LENGTH);

    ConversionUtils::GetNativeLogger().errorv("*** ENTITLEMENT CHECK FAILED (DEV): %s", msg.c_str());
//    ConversionUtils::GetNativeLogger().error("*** In a production build, this would have been a fatal error ***");
// #endif
}

static CURLcode checkEntitlement(Utf8CP url, Utf8StringCR jsonBody)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (nullptr == curl)
        return CURLE_UNSUPPORTED_PROTOCOL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // need this if url is redirected

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return res;
}

static void runEntitlementChecks(Utf8CP url, BeDuration checkInterval)
{
    try
    {
        auto hostname = getHostName();

        Json::Value json(Json::objectValue);
        json["dummy"] = "dummy";
        Utf8String queryString(Json::FastWriter().write(json));

        while (true)
        {
            auto res = checkEntitlement(url, queryString);
            if (CURLE_OK != res)
                handleEntitlementError(Utf8PrintfString("Entitlement check on %s failed with code %d: %s. Server=%s", hostname.c_str(), res, curl_easy_strerror(res), url));
            else
                ConversionUtils::GetNativeLogger().infov("Entitlement check on %s suceeded (server=%s)", hostname.c_str(), url);

            checkInterval.Sleep();
        }
    }
    catch (...)
    {
        handleEntitlementError("?");
    }
}

void EntitlementChecker::Run(Napi::Env env)
{
    Utf8CP url = getenv("IMODEL_BANK_ENTITLEMENT_SERVER_URL");
    if (nullptr == url)
    {
        handleEntitlementError("IMODEL_BANK_ENTITLEMENT_SERVER_URL must be defined");
        return;
    }

    Utf8CP intervalStr = getenv("IMODEL_BANK_ENTITLEMENT_CHECK_INTERVAL_HOURS");
    auto intervalHours = intervalStr ? atoi(intervalStr) : 0;
    if (intervalHours <= 0 || intervalHours > (24 * 30)) // cannot be more than 30 days, as that is the max lifetime of a checked-out license.
        intervalHours = 24;

    BeDuration checkInterval = BeDuration::Hours(intervalHours);

    std::thread([checkInterval, url] { runEntitlementChecks(url, checkInterval); }).detach();
}
