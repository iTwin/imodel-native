/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/ContextServicesWorkbench.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/RealityPlatformAPI.h>
#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>

/*#include <WebServices/Client/WSClient.h>
#include <WebServices/Connect/Authentication.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectSetup.h>
#include <WebServices/Connect/ConnectSpaces.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Licensing/UsageTracking.h>*/

#include <curl/curl.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct GeoCoordinationParams;
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeoCoordinationParams)

enum class ServerType
{
    DEV,
    QA,
    PROD
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GeoCoordinationParams
    {
    private:
        bvector<GeoPoint2d> m_filterPolygon;
        ServerType m_serverType;
        Utf8String m_filterString;
    public:
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParams() :m_filterPolygon(bvector<GeoPoint2d>()), m_serverType(ServerType::QA), m_filterString("") {}
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParams(bvector<GeoPoint2d> params, ServerType serverType = ServerType::QA, Utf8String filterString = "");
        REALITYDATAPLATFORM_EXPORT bvector<GeoPoint2d> GetPolygonVector() const { return m_filterPolygon; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetPolygonAsString(bool urlEncode) const;
        REALITYDATAPLATFORM_EXPORT ServerType GetServerType() const { return m_serverType; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetFilterString() const { return m_filterString; }
    };

struct ContextServicesWorkbench
    {
    private:
        Json::Value m_errorObj;
        Utf8String m_authorizationToken;
        BeFileName m_certificatePath;
        Utf8String m_spatialEntityWithDetailsJson;
        GeoCoordinationParams m_uiParams;
        bool m_downloadedSEWD;

        Utf8String m_instanceId;
        BeFileName m_packageFileName;
        bool m_downloadedPackage;

        ContextServicesWorkbench(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params);

        Utf8String getUrl();
        BeFileName getBaseDownloadFolder();
        CURLcode performCurl(Utf8StringCR url, Utf8StringCP writeString = nullptr, FILE* fp = nullptr, Utf8StringCR postFields = Utf8String());
        
        Utf8String createSpatialEntityWithDetailsViewUrl();
    public:
        REALITYDATAPLATFORM_EXPORT static ContextServicesWorkbench* Create(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT void SetGeoParam(GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT Json::Value GetError() { return m_errorObj; }

        REALITYDATAPLATFORM_EXPORT GeoCoordinationParamsCR GetUiParameters() const { return m_uiParams; }

        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadSpatialEntityWithDetails();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE
