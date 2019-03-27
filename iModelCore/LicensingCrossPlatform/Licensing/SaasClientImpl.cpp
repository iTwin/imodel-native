/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/SaasClientImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SaasClientImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "LicensingDb.h"
#include "FreeApplicationPolicyHelper.h"

#include <Licensing/Utils/LogFileHelper.h>

#include <Bentley/BeSystemInfo.h>
#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

SaasClientImpl::SaasClientImpl
    (
    int productId,
    Utf8StringCR featureString,
    IUlasProviderPtr ulasProvider
    )
    {
    m_deviceId = BeSystemInfo::GetDeviceId();
    if (m_deviceId.Equals(""))
        m_deviceId = "DefaultDevice";
    m_productId = productId;
    m_featureString = featureString;
    m_ulasProvider = ulasProvider;
    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Jason.Wichert           3/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClientImpl::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
    {
    LOG.debug("UlasProvider::RealtimeTrackUsage");
    return m_ulasProvider->RealtimeTrackUsage(accessToken, m_productId, m_featureString, m_deviceId, version, projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Jason.Wichert            3/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClientImpl::MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent)
    {
    LOG.debug("SaasClientImpl::MarkFeature");
    return m_ulasProvider->RealtimeMarkFeature(accessToken, featureEvent, m_productId, m_featureString, m_deviceId);
    }
