/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/HttpTraversalEngineNet/HttpTraversalEngineCli.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <vcclr.h>

#include <RealityPlatform/HttpTraversalEngine.h>

using namespace System::Collections::Generic;

// Forward declaration
namespace RealityServicesCli { ref class HttpDataWrapper; }
namespace RealityServicesCli { ref class HttpThumbnailWrapper; }
namespace RealityServicesCli { ref class HttpMetadataWrapper; }
namespace RealityServicesCli { ref class HttpServerWrapper; }

namespace RealityServicesCli
    {
    public enum class HttpStatusWrapper
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
    //=====================================================================================
    public interface class IHttpTraversalObserverWrapper
        {
        bool OnFileListed_AddToQueue(System::String^ file);
        void OnFileDownloaded(System::String^ file);
        void OnDataExtracted(HttpDataWrapper^ data);
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    class HttpTraversalObserverWrapper : public RealityPlatform::IHttpTraversalObserver
        {
        public:
            HttpTraversalObserverWrapper(gcroot<IHttpTraversalObserverWrapper^> managedHttpObserver);

            virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file);
            virtual void OnFileDownloaded(Utf8CP file);
            virtual void OnDataExtracted(RealityPlatform::HttpDataCR data);

        private:
            gcroot<IHttpTraversalObserverWrapper^> m_managedHttpObserver;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public ref class HttpClientWrapper
        {
        public:
            //! Create http client by setting the http url/root.
            static HttpClientWrapper^ ConnectTo(System::String^ serverUrl);

            //! Create http client by setting the http url/root and name.
            static HttpClientWrapper^ ConnectTo(System::String^ serverUrl, System::String^ serverName);

            //! Download all files from root and saved them to an optional output path. Default is temp directory.
            HttpStatusWrapper DownloadContent(System::String^ outputPath);

            //! Get a list of all files from all directories starting at root.
            HttpStatusWrapper GetFileList(System::Collections::Generic::List<System::String^>^ fileList);

            //! Get complete data from root. This includes base, source, thumbnail, metadata and server details.
            //! An observer must be set to catch the data after each extraction.
            HttpStatusWrapper GetData();

            //! Get server url.
            System::String^ GetServerUrl();

            //! Get server name.
            System::String^ GetServerName();

            //! Set the IHttpTraversalObserver to be called after each extraction. Only one observer
            //! can be set. Typically, the user of the handler would implement the IHttpTraversalObserver
            //! interface and send "this" as the argument to this method.
            void SetObserver(IHttpTraversalObserverWrapper^ traversalObserver);

        private:
            HttpClientWrapper(System::String^ serverUrl, System::String^ serverName);
            ~HttpClientWrapper();
            !HttpClientWrapper();

            RealityPlatform::HttpClientPtr* m_pClient;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public ref class HttpDataWrapper
        {
        public:
            //! Create invalid data.
            static HttpDataWrapper^ Create();
    
            //! Get/Set
            System::String^ GetName();
            void SetName(System::String^ name);

            //! Get/Set
            System::String^ GetUrl();
            void SetUrl(System::String^ url);
    
            //! Get/Set
            System::String^ GetCompoundType();
            void SetCompoundType(System::String^ type);

            //! Get/Set
            uint64_t GetSize();  // in bytes.
            void SetSize(uint64_t size);  // in bytes.

            //! Get/Set
            System::String^ GetResolution();  // in meters.
            void SetResolution(System::String^ resolution);  // in meters.

            //! Get/Set
            System::String^ GetProvider();
            void SetProvider(System::String^ provider);

            //! Get/Set
            System::String^ GetDataType();
            void SetDataType(System::String^ type);

            //! Get/Set
            System::String^ GetLocationInCompound();
            void SetLocationInCompound(System::String^ location);

            //! Get/Set
            System::String^ GetDate();
            void SetDate(System::String^ date);

            //! Get/Set
            List<double>^ GetFootprint();
            void SetFootprint(List<double>^ footprint);

            //! Get/Set
            HttpThumbnailWrapper^ GetThumbnail();
            void SetThumbnail(HttpThumbnailWrapper^ thumbnail);

            //! Get/Set
            HttpMetadataWrapper^ GetMetadata();
            void SetMetadata(HttpMetadataWrapper^ metadata);

            //! Get/Set
            HttpServerWrapper^ GetServer();
            void SetServer(HttpServerWrapper^ server);
    
        private: 
            HttpDataWrapper();
            ~HttpDataWrapper();
            !HttpDataWrapper();

            RealityPlatform::HttpDataPtr* m_pData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class HttpThumbnailWrapper
        {
        public:
            //! Create invalid thumbnail.
            static HttpThumbnailWrapper^ Create();

            //! Get/Set
            System::String^ GetProvenance();
            void SetProvenance(System::String^ provenance);

            //! Get/Set
            System::String^ GetFormat();
            void SetFormat(System::String^ format);

            //! Get/Set
            uint32_t GetWidth();
            void SetWidth(uint32_t width);

            //! Get/Set
            uint32_t GetHeight();
            void SetHeight(uint32_t height);

            //! Get/Set
            System::String^ GetStamp();
            void SetStamp(System::String^ date);

            //! Get/Set
            List<Byte>^ GetData();
            void SetData(List<Byte>^ data);

            //! Get/Set
            System::String^ GetGenerationDetails();
            void SetGenerationDetails(System::String^ details);

        private:
            HttpThumbnailWrapper();
            ~HttpThumbnailWrapper();
            !HttpThumbnailWrapper();

            RealityPlatform::HttpThumbnailPtr* m_pThumbnail;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class HttpMetadataWrapper
        {
        public:
            //! Create empty metadata.
            static HttpMetadataWrapper^ Create();

            //! Get/Set
            System::String^ GetProvenance();
            void SetProvenance(System::String^ provenance);

            //! Get/Set
            System::String^ GetDescription();
            void SetDescription(System::String^ description);

            //! Get/Set
            System::String^ GetContactInfo();
            void SetContactInfo(System::String^ info);

            //! Get/Set
            System::String^ GetLegal();
            void SetLegal(System::String^ legal);

            //! Get/Set
            System::String^ GetFormat();
            void SetFormat(System::String^ format);

            //! Get/Set
            System::String^ GetData();
            void SetData(System::String^ data);

        private:
            HttpMetadataWrapper();
            ~HttpMetadataWrapper();
            !HttpMetadataWrapper();

            RealityPlatform::HttpMetadataPtr* m_pMetadata;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class HttpServerWrapper
        {
        public:
            //! Create invalid server.
            static HttpServerWrapper^ Create();

            //! Get/Set
            System::String^ GetProtocol();
            void SetProtocol(System::String^ protocol);

            //! Get/Set
            System::String^ GetName();
            void SetName(System::String^ name);

            //! Get/Set
            System::String^ GetUrl();
            void SetUrl(System::String^ url);

            //! Get/Set
            System::String^ GetContactInfo();
            void SetContactInfo(System::String^ info);

            //! Get/Set
            System::String^ GetLegal();
            void SetLegal(System::String^ legal);

            //! Get/Set
            bool IsOnline();
            void SetOnline(bool online);

            //! Get/Set
            System::String^ GetLastCheck();
            void SetLastCheck(System::String^ time);

            //! Get/Set
            System::String^ GetLastTimeOnline();
            void SetLastTimeOnline(System::String^ time);

            //! Get/Set
            double GetLatency();
            void SetLatency(double latency);

            //! Get/Set
            System::String^ GetState();
            void SetState(System::String^ state);

            //! Get/Set
            System::String^ GetServerType();
            void SetServerType(System::String^ type);

        private:
            HttpServerWrapper();
            ~HttpServerWrapper();
            !HttpServerWrapper();

            RealityPlatform::HttpServerPtr* m_pServer;
        };
    }