/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/HttpTraversalEngine.h $
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
#include "SpatialEntityClient.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Implement a way to connect to a http server and retrieve the content.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpClient : public SpatialEntityClient
    {
    public:
        //! Create http client by setting the url/root.
        REALITYDATAPLATFORM_EXPORT static HttpClientPtr ConnectTo(Utf8CP serverUrl, Utf8CP serverName = NULL, Utf8CP datasetName = NULL, Utf8CP filePattern = NULL, bool extractThumbnails = false, Utf8CP classification = NULL);
        
    protected:
        HttpClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification);

        //! Recurse into sub directories and create a list of all files.
        SpatialEntityStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const override;

        SpatialEntityDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const override;
        
        //! Callback when a file is downloaded to construct the data repository mapping.
        static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
    };

//=====================================================================================
//! Handle a CURL http request with all the necessary parameters and response.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpRequest : public SpatialEntityRequest
    {
    public:
        //! Create curl request.
        static HttpRequestPtr Create(Utf8CP url);

        //! Get response from a request.
        //! See PublicApi/RealityAdmin/SpatialEntityClient.h for more details on the structure of a spatial entity response.
        SpatialEntityResponsePtr Perform() override;

        void SetCertificatePath(Utf8CP path) { m_caPath = path; }

    protected:
        HttpRequest(Utf8CP url);

        Utf8String m_caPath;
    };

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct HttpDataHandler : public SpatialEntityDataHandler
    {
    public:
        //! Http data extraction.
        //! See PublicApi/RealityPlatform/SpatialEntityData.h for more details on the structure of a spatial entity data.
        REALITYDATAPLATFORM_EXPORT static SpatialEntityDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath, Utf8CP filePattern, bool extractThumbnails);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE