/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/FtpTraversalEngineCli.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <vcclr.h>

#include <RealityPlatform/FtpTraversalEngine.h>

using namespace System::Collections::Generic;

// Forward declaration
namespace RealityServicesCli { ref class FtpDataWrapper; }
namespace RealityServicesCli { ref class FtpThumbnailWrapper; }
namespace RealityServicesCli { ref class FtpMetadataWrapper; }
namespace RealityServicesCli { ref class FtpServerWrapper; }

namespace RealityServicesCli
    {
    public enum class FtpStatusWrapper
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
    public interface class IFtpTraversalObserverWrapper
        {
        bool OnFileListed_AddToQueue(System::String^ file);
        void OnFileDownloaded(System::String^ file);
        void OnDataExtracted(FtpDataWrapper^ data);
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    class FtpTraversalObserverWrapper : public RealityPlatform::IFtpTraversalObserver
        {
        public:
            FtpTraversalObserverWrapper(gcroot<IFtpTraversalObserverWrapper^> managedFtpObserver);

            virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file);
            virtual void OnFileDownloaded(Utf8CP file);
            virtual void OnDataExtracted(RealityPlatform::FtpDataCR data);

        private:
            gcroot<IFtpTraversalObserverWrapper^> m_managedFtpObserver;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public ref class FtpClientWrapper
        {
        public:
            //! Create ftp client by setting the ftp url/root.
            static FtpClientWrapper^ ConnectTo(System::String^ url);

            //! Download all files from root and saved them to an optional output path. Default is temp directory.
            FtpStatusWrapper DownloadContent(System::String^ outputPath);

            //! Get a list of all files from all directories starting at root.
            FtpStatusWrapper GetFileList(System::Collections::Generic::List<System::String^>^ fileList);

            //! Get complete data from root. This includes base, source, thumbnail, metadata and server details.
            //! An observer must be set to catch the data after each extraction.
            FtpStatusWrapper GetData();

            //! Get url/root.
            System::String^ GetServerUrl();

            //! Set the IFtpTraversalObserver to be called after each extraction. Only one observer
            //! can be set. Typically, the user of the handler would implement the IFtpTraversalObserver
            //! interface and send "this" as the argument to this method.
            void SetObserver(IFtpTraversalObserverWrapper^ traversalObserver);

        private:
            FtpClientWrapper(System::String^ url);
            ~FtpClientWrapper();
            !FtpClientWrapper();

            RealityPlatform::FtpClientPtr* m_pClient;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public ref class FtpDataWrapper
        {
        public:
            //! Create invalid data.
            static FtpDataWrapper^ Create();
    
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
            FtpThumbnailWrapper^ GetThumbnail();
            void SetThumbnail(FtpThumbnailWrapper^ thumbnail);

            //! Get/Set
            FtpMetadataWrapper^ GetMetadata();
            void SetMetadata(FtpMetadataWrapper^ metadata);

            //! Get/Set
            FtpServerWrapper^ GetServer();
            void SetServer(FtpServerWrapper^ server);
    
        private: 
            FtpDataWrapper();
            ~FtpDataWrapper();
            !FtpDataWrapper();

            RealityPlatform::FtpDataPtr* m_pData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class FtpThumbnailWrapper
        {
        public:
            //! Create invalid thumbnail.
            static FtpThumbnailWrapper^ Create();

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
            FtpThumbnailWrapper();
            ~FtpThumbnailWrapper();
            !FtpThumbnailWrapper();

            RealityPlatform::FtpThumbnailPtr* m_pThumbnail;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class FtpMetadataWrapper
        {
        public:
            //! Create empty metadata.
            static FtpMetadataWrapper^ Create();

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
            FtpMetadataWrapper();
            ~FtpMetadataWrapper();
            !FtpMetadataWrapper();

            RealityPlatform::FtpMetadataPtr* m_pMetadata;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              5/2016
    //=====================================================================================
    public ref class FtpServerWrapper
        {
        public:
            //! Create invalid server.
            static FtpServerWrapper^ Create();

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
            FtpServerWrapper();
            ~FtpServerWrapper();
            !FtpServerWrapper();

            RealityPlatform::FtpServerPtr* m_pServer;
        };
    }