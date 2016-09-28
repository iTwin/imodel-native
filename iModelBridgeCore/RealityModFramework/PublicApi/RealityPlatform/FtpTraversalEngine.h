/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/FtpTraversalEngine.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <RealityPlatform/WebResourceData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpClient : public WebResourceClient
    {
    public:
        //! Create ftp client by setting the url/root.
        REALITYDATAPLATFORM_EXPORT static FtpClientPtr ConnectTo(Utf8CP serverUrl, Utf8CP serverName = NULL);

    private:
        FtpClient(Utf8CP serverUrl, Utf8CP serverName);

        //! Recurse into sub directories and create a list of all files.
        WebResourceStatus _GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const override;

        WebResourceDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath) const override;

        //! Callback when a file is downloaded to construct the data repository mapping.
        static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpRequest : public WebResourceRequest
    {
    public:
        //! Create curl request.
        static FtpRequestPtr Create(Utf8CP url);

        //! Get response from request.
        WebResourceResponsePtr Perform() override;
        
    private:
        FtpRequest(Utf8CP url);
    };

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//!
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpDataHandler : public WebResourceDataHandler
    {
    public:
        //! Ftp data extraction.
        REALITYDATAPLATFORM_EXPORT static WebResourceDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath);

    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE