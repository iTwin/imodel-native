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

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct FtpThumbnail;
struct FtpMetadata;
struct FtpServer;

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
        virtual void OnDataExtracted(FtpDataCR data) = 0;    
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
        
        FtpServerPtr m_pServer;         
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
//! @bsiclass                                   Jean-Francois.Cote              4/2016
//=====================================================================================
struct FtpData : public RefCountedBase
    {
    public:
        //! Create invalid data.
        REALITYDATAPLATFORM_EXPORT static FtpDataPtr Create();

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
        REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
        REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCompoundType() const;
        REALITYDATAPLATFORM_EXPORT void SetCompoundType(Utf8CP type);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT uint64_t GetSize() const; // in bytes.
        REALITYDATAPLATFORM_EXPORT void SetSize(uint64_t size); // in bytes.

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetResolution() const; // in meters.
        REALITYDATAPLATFORM_EXPORT void SetResolution(Utf8CP resolution); // in meters.

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvider() const;
        REALITYDATAPLATFORM_EXPORT void SetProvider(Utf8CP provider);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
        REALITYDATAPLATFORM_EXPORT void SetDataType(Utf8CP type);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLocationInCompound() const;
        REALITYDATAPLATFORM_EXPORT void SetLocationInCompound(Utf8CP location);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT DateTimeCR GetDate() const;
        REALITYDATAPLATFORM_EXPORT void SetDate(DateTimeCR date);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT DRange2dCR GetFootprint() const;
        REALITYDATAPLATFORM_EXPORT void SetFootprint(DRange2dCR footprint);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT FtpThumbnailCR GetThumbnail() const;
        REALITYDATAPLATFORM_EXPORT void SetThumbnail(FtpThumbnailR thumbnail);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT FtpMetadataCR GetMetadata() const;
        REALITYDATAPLATFORM_EXPORT void SetMetadata(FtpMetadataR metadata);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT FtpServerCR GetServer() const;
        REALITYDATAPLATFORM_EXPORT void SetServer(FtpServerR server);

    private:
        FtpData();

        Utf8String m_name;
        Utf8String m_url;
        Utf8String m_compoundType;
        uint64_t m_size;
        Utf8String m_resolution;
        Utf8String m_provider;
        Utf8String m_dataType;
        Utf8String m_locationInCompound;        
        DateTime m_date;
        DRange2d m_footprint;
        FtpThumbnailPtr m_pThumbnail;
        FtpMetadataPtr m_pMetadata;
        FtpServerPtr m_pServer;
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct FtpThumbnail : public RefCountedBase
    {
    public:
        //! Create invalid thumbnail.
        REALITYDATAPLATFORM_EXPORT static FtpThumbnailPtr Create();

        //! Create from raster file.
        REALITYDATAPLATFORM_EXPORT static FtpThumbnailPtr Create(RealityDataCR rasterData);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
        REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
        REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT uint32_t GetWidth() const;
        REALITYDATAPLATFORM_EXPORT void SetWidth(uint32_t width);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT uint32_t GetHeight() const;
        REALITYDATAPLATFORM_EXPORT void SetHeight(uint32_t height);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT DateTimeCR GetStamp() const;
        REALITYDATAPLATFORM_EXPORT void SetStamp(DateTimeCR date);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT const bvector<Byte>& GetData() const;
        REALITYDATAPLATFORM_EXPORT void SetData(const bvector<Byte>& data);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetGenerationDetails() const;
        REALITYDATAPLATFORM_EXPORT void SetGenerationDetails(Utf8CP details);

    private:
        FtpThumbnail();
        FtpThumbnail(RealityDataCR rasterData);

        Utf8String m_provenance;
        Utf8String m_format;
        uint32_t m_width;
        uint32_t m_height;
        DateTime m_stamp;
        bvector<Byte> m_data;
        Utf8String m_generationDetails;
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct FtpMetadata : public RefCountedBase
    {
    public:
        //! Create empty metadata.
        REALITYDATAPLATFORM_EXPORT static FtpMetadataPtr Create();

        //! Create from xml file.
        REALITYDATAPLATFORM_EXPORT static FtpMetadataPtr CreateFromFile(Utf8CP filePath);
    
        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProvenance() const;
        REALITYDATAPLATFORM_EXPORT void SetProvenance(Utf8CP provenance);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
        REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
        REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
        REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetFormat() const;
        REALITYDATAPLATFORM_EXPORT void SetFormat(Utf8CP format);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetData() const;
        REALITYDATAPLATFORM_EXPORT void SetData(Utf8CP data);
    
    private:
        FtpMetadata();
        FtpMetadata(Utf8CP filePath);

        Utf8String m_provenance;        
        Utf8String m_description;
        Utf8String m_contactInfo;
        Utf8String m_legal;
        Utf8String m_format;
        Utf8String m_data;
    };


//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              5/2016
//=====================================================================================
struct FtpServer : public RefCountedBase
    {
    public:
        //! Create invalid server.
        REALITYDATAPLATFORM_EXPORT static FtpServerPtr Create();

        //! Create from url.
        REALITYDATAPLATFORM_EXPORT static FtpServerPtr Create(Utf8CP url, Utf8CP name = NULL);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetProtocol() const;
        REALITYDATAPLATFORM_EXPORT void SetProtocol(Utf8CP protocol);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
        REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
        REALITYDATAPLATFORM_EXPORT void SetUrl(Utf8CP url);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContactInfo() const;
        REALITYDATAPLATFORM_EXPORT void SetContactInfo(Utf8CP info);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetLegal() const;
        REALITYDATAPLATFORM_EXPORT void SetLegal(Utf8CP legal);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT bool IsOnline() const;
        REALITYDATAPLATFORM_EXPORT void SetOnline(bool online);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastCheck() const;
        REALITYDATAPLATFORM_EXPORT void SetLastCheck(DateTimeCR time);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT DateTimeCR GetLastTimeOnline() const;
        REALITYDATAPLATFORM_EXPORT void SetLastTimeOnline(DateTimeCR time);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT double GetLatency() const;
        REALITYDATAPLATFORM_EXPORT void SetLatency(double latency);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetState() const;
        REALITYDATAPLATFORM_EXPORT void SetState(Utf8CP state);

        //! Get/Set
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetType() const;
        REALITYDATAPLATFORM_EXPORT void SetType(Utf8CP type);

    private:
        FtpServer();
        FtpServer(Utf8CP url, Utf8CP name);

        Utf8String m_protocol;
        Utf8String m_name;
        Utf8String m_url;
        Utf8String m_contactInfo;
        Utf8String m_legal;
        bool m_online;
        DateTime m_lastCheck;
        DateTime m_lastTimeOnline;
        double m_latency;
        Utf8String m_state;
        Utf8String m_type;
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
        REALITYDATAPLATFORM_EXPORT static FtpDataPtr ExtractDataFromPath(Utf8CP inputDirPath, Utf8CP outputDirPath);

        //! Unzip files.
        REALITYDATAPLATFORM_EXPORT static FtpStatus UnzipFiles(Utf8CP inputDirPath, Utf8CP outputDirPath);

    private:
        //! Ftp data extraction.
        static BeFileName BuildMetadataFilename(Utf8CP filePath);

        //! Geocoding lookup.
        static Utf8String RetrieveGeocodingFromMetadata(BeFileNameCR filename);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE