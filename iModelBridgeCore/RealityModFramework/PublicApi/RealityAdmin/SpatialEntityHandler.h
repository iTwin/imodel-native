/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/SpatialEntityHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <sql.h>
#include <sqlext.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct CurlHolder
    {
public:
    CurlHolder() : m_curl(curl_easy_init()) {}
    ~CurlHolder() { if (NULL != m_curl) curl_easy_cleanup(m_curl); }
    CURL* Get() const { return m_curl; }

private:
    CURL* m_curl;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//! Status codes for SpatialEntity traversal operations.
//=====================================================================================
enum class SpatialEntityHandlerStatus
    {
    Success = SUCCESS,      // The operation was successful.
    ClientError,
    CurlError,
    DataExtractError,
    DownloadError,
    // *** Add new here.
    UnknownError = ERROR,   // The operation failed with an unspecified error.
    };





   

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//!
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct SpatialEntityHandler
    {
public:
    //! Unzip files.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityHandlerStatus UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath);

protected:
    //! Http data extraction.
    REALITYDATAPLATFORM_EXPORT static BeFileName BuildMetadataFilename(Utf8CP filePath);

    //! Geocoding lookup.
    REALITYDATAPLATFORM_EXPORT static Utf8String RetrieveGeocodingFromMetadata(BeFileNameCR filename);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE