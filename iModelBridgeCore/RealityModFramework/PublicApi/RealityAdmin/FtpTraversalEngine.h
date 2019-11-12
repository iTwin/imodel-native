/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <RealityPlatform/SpatialEntity.h>
#include "SpatialEntityClient.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Implement a way to connect to a ftp server and retrieve the content.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpClient : public SpatialEntityClient
    {
    public:
        //! Create ftp client by setting the url/root.
        REALITYDATAPLATFORM_EXPORT static FtpClientPtr ConnectTo(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed);

    private:
        FtpClient(Utf8CP serverUrl, Utf8CP serverName, Utf8CP providerName, Utf8CP datasetName, Utf8CP filePattern, bool extractThumbnails, Utf8CP classification, SpatialEntityMetadataCR metadataSeed);

        //! Recurse into sub directories and create a list of all files.
        SpatialEntityHandlerStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const override;

        SpatialEntityPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const override;

        //! Callback when a file is downloaded to construct the data repository mapping.
        static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
    };

//=====================================================================================
//! Handle a CURL ftp request with all the necessary parameters and response.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpRequest : public SpatialEntityRequest
    {
    public:
        //! Create curl request.
        static FtpRequestPtr Create(Utf8CP url);

        //! Get response from request.
        //! See PublicApi/RealityAdmin/SpatialEntityClient.h for more details on the structure of a spatial entity response.
        SpatialEntityResponsePtr Perform() override;
        
    private:
        FtpRequest(Utf8CP url);
    };

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpDataHandler : public SpatialEntityHandler
    {
    public:
        //! Ftp data extraction.
        //! See PublicApi/RealityPlatform/SpatialEntity.h for more details on the structure of a spatial entity data.
        REALITYDATAPLATFORM_EXPORT static SpatialEntityPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath, Utf8CP filePattern, bool extractThumbnail, SpatialEntityMetadataCR metadataSeed);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE