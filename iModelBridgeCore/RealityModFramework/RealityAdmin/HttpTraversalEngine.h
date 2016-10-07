/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/HttpTraversalEngine.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <RealityPlatform/SpatialEntityData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpClient : public SpatialEntityClient
    {
    public:
        //! Create http client by setting the url/root.
        REALITYDATAPLATFORM_EXPORT static HttpClientPtr ConnectTo(Utf8CP serverUrl, Utf8CP serverName = NULL);
        
    protected:
        HttpClient(Utf8CP serverUrl, Utf8CP serverName);

        //! Recurse into sub directories and create a list of all files.
        SpatialEntityStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const override;

        SpatialEntityDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const override;
        
        //! Callback when a file is downloaded to construct the data repository mapping.
        static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpRequest : public SpatialEntityRequest
    {
    public:
        //! Create curl request.
        static HttpRequestPtr Create(Utf8CP url);

        //! Get response from request.
        SpatialEntityResponsePtr Perform() override;

        void SetCertificatePath(Utf8CP path) { m_caPath = path; }

    protected:
        HttpRequest(Utf8CP url);

        Utf8String m_caPath;
    };

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//!
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpDataHandler : public SpatialEntityDataHandler
    {
    public:
        //! Http data extraction.
        REALITYDATAPLATFORM_EXPORT static SpatialEntityDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath);

    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE