/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
//__BENTLEY_INTERNAL_ONLY__
#include <RealityPlatform/RealityPlatformAPI.h>
#include <Bentley/Bentley.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <SpatioTemporalSelector/SpatioTemporalSelector.h>
#include <RealityPlatform/SpatioTemporalData.h>
#include <RealityPlatformTools/WSGServices.h>

#include <curl/curl.h>

//! Callback function to filter spatialentities
typedef std::function<bool(RealityPlatform::SpatialEntityPtr entity)> ContextServicesWorkbench_FilterFunction;


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

// Forward declaration.
struct GeoCoordinationParams;
DEFINE_POINTER_SUFFIX_TYPEDEFS(GeoCoordinationParams)



//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
struct GeoCoordinationParams
    {
    private:
        bvector<GeoPoint2d> m_filterPolygon;
        CONNECTServerType m_serverType;
        Utf8String m_filterString;
    public:
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParams() :m_filterPolygon(bvector<GeoPoint2d>()), m_serverType(CONNECTServerType::QA), m_filterString("") {}
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParams(bvector<GeoPoint2d> params, CONNECTServerType serverType = CONNECTServerType::QA, Utf8String filterString = "");
        REALITYDATAPLATFORM_EXPORT bvector<GeoPoint2d> GetPolygonVector() const { return m_filterPolygon; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetPolygonAsString(bool urlEncode) const;
        REALITYDATAPLATFORM_EXPORT CONNECTServerType GetServerType() const { return m_serverType; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetFilterString() const { return m_filterString; }
    };

//=====================================================================================
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
struct ContextServicesWorkbench
    {
    private:
        static bool default_filter(RealityPlatform::SpatialEntityPtr entity) { return false; }
        Json::Value m_errorObj;
        Utf8String m_authorizationToken;
        BeFileName m_certificatePath;
        Utf8String m_spatialEntityWithDetailsJson;
        Utf8String m_packageIdBuffer;
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
        
        BentleyStatus handlePackageFile();
    public:
        REALITYDATAPLATFORM_EXPORT Utf8String CreateSpatialEntityWithDetailsViewUrl(Utf8String filter = "");
        REALITYDATAPLATFORM_EXPORT Utf8String GetPackageIdUrl();
        REALITYDATAPLATFORM_EXPORT char* GetPackageFileUrl();
        REALITYDATAPLATFORM_EXPORT FILE* OpenPackageFile();
        REALITYDATAPLATFORM_EXPORT static ContextServicesWorkbench* Create(Utf8StringCR authorizationToken, GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT void SetGeoParam(GeoCoordinationParamsCR params);
        REALITYDATAPLATFORM_EXPORT Json::Value GetError() { return m_errorObj; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetSpatialEntityWithDetailsJson() { return m_spatialEntityWithDetailsJson; }
        REALITYDATAPLATFORM_EXPORT Utf8StringP GetSpatialEntityWithDetailsJsonPointer() { return &m_spatialEntityWithDetailsJson; }
        REALITYDATAPLATFORM_EXPORT Utf8StringP GetPackageIdPointer() { return &m_packageIdBuffer; }
        REALITYDATAPLATFORM_EXPORT BeFileName GetPackageFileName() { return m_packageFileName; }
        REALITYDATAPLATFORM_EXPORT GeoCoordinationParamsCR GetUiParameters() const { return m_params; }
        REALITYDATAPLATFORM_EXPORT Utf8String GetInstanceId() const { return m_instanceId; }
        REALITYDATAPLATFORM_EXPORT void SetInstanceId(Utf8String instanceId) { m_instanceId = instanceId; }
        REALITYDATAPLATFORM_EXPORT BeFileName GetCertificatePath() const { return m_certificatePath; }
        REALITYDATAPLATFORM_EXPORT void SetPackageDownloaded(bool downloaded) { m_downloadedPackage = downloaded; }

        REALITYDATAPLATFORM_EXPORT void SetToken(Utf8String token) { m_authorizationToken = token; }

        //REALITYDATAPLATFORM_EXPORT DetailedSpatialEntityListPtr GetSpatialEntityWithDetails() const { return m_contextList; }
        REALITYDATAPLATFORM_EXPORT RealityPlatform::SpatioTemporalSelector::ResolutionMap GetSelectedIds() const { return m_selectedIds; }
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadSpatialEntityWithDetails(Utf8String filter = "");
        REALITYDATAPLATFORM_EXPORT void FilterSpatialEntity(ContextServicesWorkbench_FilterFunction pi_Func = default_filter);
        REALITYDATAPLATFORM_EXPORT Utf8String GetPackageParameters();
        REALITYDATAPLATFORM_EXPORT Utf8String GetPackageParameters(bvector<Utf8String> selectedIds);
        REALITYDATAPLATFORM_EXPORT RealityPlatform::ResolutionCriteria GetResolution() { return m_selectedResolution; }
        REALITYDATAPLATFORM_EXPORT void SetResolution(RealityPlatform::ResolutionCriteria resolution) { m_selectedResolution = resolution; }

        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadPackageId();
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadPackageFile();
        REALITYDATAPLATFORM_EXPORT BentleyStatus DownloadFiles();
        REALITYDATAPLATFORM_EXPORT void SelectRandomResolution();
        REALITYDATAPLATFORM_EXPORT void Init();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE
