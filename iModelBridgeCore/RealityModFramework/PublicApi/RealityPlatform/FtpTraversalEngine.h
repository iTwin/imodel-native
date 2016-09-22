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
//! Status codes for FTP traversal operations.
//=====================================================================================
enum class FtpStatus
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
//! @bsiclass                                   Jean-Francois.Cote              5/2016
// Defines the traversal observer interface as an abstract class.
// The traversal observer that must inherit from the present one must implement
// a behavior for the OnFileListed, OnFileDownloaded and OnDataExtracted method.
//=====================================================================================
struct IFtpTraversalObserver
    {
    public:
        //! OnFileListed is called whenever a file was listed for download. It is up to
        //! the effective traversal observer to do whatever process is required on the data.
        virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file) = 0;

        //! OnFileDownloaded is called whenever a download is completed. It is up to
        //! the effective traversal observer to do whatever process is required on the data.
        virtual void OnFileDownloaded(Utf8CP file) = 0;

        //! OnDataExtracted is called whenever an extraction is completed by the 
        //! data handler. The data object given defines the data discovered. It is up to
        //! the effective traversal observer to do whatever process is required on the data.
        virtual void OnDataExtracted(WebResourceDataCR data) = 0;    
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpClient : public RefCountedBase
    {
    public:
        //! Mapping of the remote and local link to a file.
        typedef bvector<bpair<Utf8String, Utf8String>> RepositoryMapping;

        //! Create ftp client by setting the url/root.
        REALITYDATAPLATFORM_EXPORT static FtpClientPtr ConnectTo(Utf8CP serverUrl, Utf8CP serverName = NULL);

        //! Download all files from root and saved them in the specified directory. Default is temp directory.
        REALITYDATAPLATFORM_EXPORT FtpStatus DownloadContent(Utf8CP outputDirPath = NULL) const;

        //! Get a list of all files from all directories starting at root.
        REALITYDATAPLATFORM_EXPORT FtpStatus GetFileList(bvector<Utf8String>& fileList) const;

        //! Get complete data from root. This includes base, source, thumbnail, metadata and server details.
        //! An observer must be set to catch the data after each extraction.
        REALITYDATAPLATFORM_EXPORT FtpStatus GetData() const;

        //! Get server url.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerUrl() const;

        //! Get server name.
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetServerName() const;

        //! Get repository mapping (remote and local repository).
        REALITYDATAPLATFORM_EXPORT const RepositoryMapping& GetRepositoryMapping() const;

        //! Set the IFtpTraversalObserver to be called after each extraction. Only one observer
        //! can be set. Typically, the user of the handler would implement the IFtpTraversalObserver
        //! interface and send "this" as the argument to this method.
        REALITYDATAPLATFORM_EXPORT void SetObserver(IFtpTraversalObserver* pObserver);    

    private:
        FtpClient(Utf8CP serverUrl, Utf8CP serverName);
        ~FtpClient();

        //! Recurse into sub directories and create a list of all files.
        FtpStatus GetFileList(Utf8CP url, bvector<Utf8String>& fileList) const;

        //! Check if content is a directory or not.
        bool IsDirectory(Utf8CP content) const;

        //! Callback when a file is downloaded to construct the data repository mapping.
        static void ConstructRepositoryMapping(int index, void *pClient, int ErrorCode, const char* pMsg);
        
        WebResourceServerPtr m_pServer;         
        static IFtpTraversalObserver* m_pObserver;        
        static RepositoryMapping m_dataRepositories;
        static int m_retryCount;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpRequest : public RefCountedBase
    {
    public:
        //! Create curl request.
        static FtpRequestPtr Create(Utf8CP url);

        //! Get response from request.
        FtpResponsePtr Perform();

        //! Get url.
        Utf8StringCR GetUrl() const { return m_url; }

        //! Get/Set method option.
        Utf8StringCR GetMethod() const { return m_method; }
        void SetMethod(Utf8CP method) { m_method = method; }

        //! Get/Set dirList option.
        bool IsDirListOnly() const { return m_dirListOnly; }
        void SetDirListOnly(bool isDirListOnly) { m_dirListOnly = isDirListOnly; }

        //! Get/Set verbose option.
        bool IsVerbose() const { return m_verbose; }
        void SetVerbose(bool isVerbose) { m_verbose = isVerbose; }
        
    private:
        FtpRequest(Utf8CP url);

        Utf8String m_url;
        Utf8String m_method;
        bool m_dirListOnly;
        bool m_verbose;
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpResponse : public RefCountedBase
    {
    public:
        //! Create invalid response with FtpStatus::UnknownError.
        static FtpResponsePtr Create();

        //! Create response.
        static FtpResponsePtr Create(Utf8CP effectiveUrl, Utf8CP m_content, FtpStatus traversalStatus);

        //! Get url.
        Utf8StringCR GetUrl() const;

        //! Get content.
        Utf8StringCR GetContent() const;

        //! Get status.
        FtpStatus GetStatus() const;

        //! Return if request was a success or not.
        bool IsSuccess() const;

    private:
        FtpResponse(Utf8CP effectiveUrl, Utf8CP content, FtpStatus status);

        Utf8String m_effectiveUrl;
        Utf8String m_content;
        FtpStatus m_status;
    };

//=====================================================================================
//! Utility class to extract the required data from a zip file.
//!
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpDataHandler
    {
    public:
        //! Ftp data extraction.
        REALITYDATAPLATFORM_EXPORT static WebResourceDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath);

        //! Unzip files.
        REALITYDATAPLATFORM_EXPORT static FtpStatus UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath);

    private:
        //! Ftp data extraction.
        static BeFileName BuildMetadataFilename(Utf8CP filePath);

        //! Geocoding lookup.
        static Utf8String RetrieveGeocodingFromMetadata(BeFileNameCR filename);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE