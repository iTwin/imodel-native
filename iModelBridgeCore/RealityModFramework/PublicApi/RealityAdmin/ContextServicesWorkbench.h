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
        
        Utf8String createSpatialEntityWithDetailsViewUrl();;
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
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadSpatialEntityWithDetails();
        REALITYDATAPLATFORM_EXPORT void FilterSpatialEntity();
        REALITYDATAPLATFORM_EXPORT Utf8String GetPackageParameters(bvector<Utf8String> selectedIds) const;
        REALITYDATAPLATFORM_EXPORT RealityPlatform::ResolutionCriteria GetResolution() { return m_selectedResolution; }
        REALITYDATAPLATFORM_EXPORT void SetResolution(RealityPlatform::ResolutionCriteria resolution) { m_selectedResolution = resolution; }

        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadPackage();
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadFiles();
    };

//=====================================================================================
//! @bsiclass                                   Raphael.Lemieux                 01/2016
// This should be provided and maintained by RealityModFramework
//=====================================================================================
/*struct DetailedSpatialEntityList : public RefCountedBase
{
private:
    DetailedSpatialEntityList() : m_entityList() {};
    bmap<Utf8String, DetailedSpatialEntityPtr> m_entityList;
    void AddEntity(Utf8StringCR id, DetailedSpatialEntityPtr entity) { m_entityList.Insert(id, entity); };

public:
    //! Create from Json.
    static DetailedSpatialEntityListPtr CreateFromJson(Utf8CP data);
    //static bool IsValidJson(Utf8CP data);
    //const DetailedSpatialEntityPtr GetEntity(Utf8StringCR id) const;
    //const bool ContainsUsgsImagery() const;
    const bmap<Utf8String, DetailedSpatialEntityPtr>& GetMap() const { return m_entityList; };
};*/

//=====================================================================================
//! @bsiclass                                   Raphael.Lemieux                 01/2016
// This should be provided and maintained by RealityModFramework
//=====================================================================================
/*struct DetailedSpatialEntity : public RefCountedBase
{
public:
    //! Create from Json.
    static DetailedSpatialEntityPtr Create(JsonValueCR instance) { return new DetailedSpatialEntity(instance); };

    const Utf8String GetIdentifier() const { return GetChecked("Id").asString(); };
    const DateTime& GetDate() const { return m_date; };
    const double& GetResolution() const { return m_resolution; };
    const Utf8String GetName() const { return GetUnchecked("Name").asString(); };
    const Utf8String GetDescription() const { return GetUnchecked("Description").asString(); };
    const Utf8String GetContactInformation() const { return GetUnchecked("ContactInformation").asString(); };
    const Utf8String GetKeywords() const { return GetUnchecked("Keywords").asString(); };
    const Utf8String GetLegal() const { return GetUnchecked("Legal").asString(); };
    const Utf8String GetTermsOfUse() const { return GetUnchecked("TermsOfUse").asString(); };
    const Utf8String GetDataSourceTypesAvailable() const { return GetUnchecked("DataSourceTypesAvailable").asString(); };
    const Utf8String GetClassification() const { return GetUnchecked("Classification").asString(); };
    const Utf8String GetDataProvider() const { return GetUnchecked("DataProvider").asString(); };
    const Utf8String GetDataProviderName() const { return GetUnchecked("DataProviderName").asString(); };
    const Utf8String GetThumbnailUrl() const { return GetUnchecked("ThumbnailURL").asString(); };
    const double GetOcclusion() const { return (!m_instance.isMember("Occlusion") || m_instance["Occlusion"].isNull()) ? 0.0 : GetUnchecked("Occlusion").asDouble(); };
    const UINT64 GetFileSize() const { return m_filesize; };

private:
    DetailedSpatialEntity(JsonValueCR instance);

    JsonValueCR GetChecked(Utf8CP key) const { BeAssert(m_instance.isMember(key)); return m_instance[key]; };
    JsonValueCR GetUnchecked(Utf8CP key) const { return m_instance[key]; };

    Json::Value m_instance;
    DateTime m_date;
    double m_resolution;
    UINT64 m_filesize;
};*/

END_BENTLEY_REALITYPLATFORM_NAMESPACE
