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
#include <RealityPlatform/SpatioTemporalSelector.h>
#include <RealityPlatform/SpatioTemporalData.h>

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

//! Callback function to filter spatialentities
typedef std::function<bool(RealityPlatform::SpatioTemporalDataPtr entity)> ContextServicesWorkbench_FilterFunction;


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

// Forward declaration.
struct GeoCoordinationParams;
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeoCoordinationParams)

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
enum class ServerType
    {
    DEV,
    QA,
    PROD
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
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

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
struct ContextServicesWorkbench
    {
    private:
        static bool default_filter(RealityPlatform::SpatioTemporalDataPtr entity) { return false; }
        Json::Value m_errorObj;
        Utf8String m_authorizationToken;
        BeFileName m_certificatePath;
        Utf8String m_spatialEntityWithDetailsJson;
        GeoCoordinationParams m_params;
        bool m_downloadedSEWD;
        RealityPlatform::SpatioTemporalSelector::ResolutionMap m_selectedIds;
        RealityPlatform::ResolutionCriteria m_selectedResolution = RealityPlatform::ResolutionCriteria::Low;

        //DetailedSpatialEntityListPtr m_contextList;
        Utf8String m_instanceId;
        BeFileName m_packageFileName;
        bool m_downloadedPackage;

        ContextServicesWorkbench(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params);

        Utf8String getBaseUrl();
        BeFileName getBaseFolder();
        CURLcode performCurl(Utf8StringCR url, Utf8StringCP writeString = nullptr, FILE* fp = nullptr, Utf8StringCR postFields = Utf8String());
        
        Utf8String createSpatialEntityWithDetailsViewUrl(Utf8String filter = "");
        BentleyStatus downloadPackageId();
        BentleyStatus downloadPackageFile();
        BentleyStatus handlePackageFile();
    public:
        REALITYDATAPLATFORM_EXPORT static ContextServicesWorkbench* Create(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT void SetGeoParam(GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT Json::Value GetError() { return m_errorObj; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetSpatialEntityWithDetailsJson() { return m_spatialEntityWithDetailsJson; }
        REALITYDATAPLATFORM_EXPORT BeFileName GetPackageFileName() { return m_packageFileName; }
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParamsCR GetUiParameters() const { return m_params; }

        //REALITYDATAPLATFORM_EXPORT DetailedSpatialEntityListPtr GetSpatialEntityWithDetails() const { return m_contextList; }
        REALITYDATAPLATFORM_EXPORT RealityPlatform::SpatioTemporalSelector::ResolutionMap GetSelectedIds() const { return m_selectedIds; }
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadSpatialEntityWithDetails(Utf8String filter = "");
        REALITYDATAPLATFORM_EXPORT void FilterSpatialEntity(ContextServicesWorkbench_FilterFunction pi_Func = default_filter);
        REALITYDATAPLATFORM_EXPORT Utf8String GetPackageParameters(bvector<Utf8String> selectedIds) const;
        REALITYDATAPLATFORM_EXPORT RealityPlatform::ResolutionCriteria GetResolution() { return m_selectedResolution; }
        REALITYDATAPLATFORM_EXPORT void SetResolution(RealityPlatform::ResolutionCriteria resolution) { m_selectedResolution = resolution; }

        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadPackage();
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadFiles();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE
