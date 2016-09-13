/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeFileName.h>
#include <RealityPackage/RealityDataPackage.h>

#using  <mscorlib.dll>

using RealityPackage::RealityDataPackage;
using namespace System::Collections::Generic;


// Forward declaration
namespace RealityDataPackageWrapper { ref class ImageryGroupNet; }
namespace RealityDataPackageWrapper { ref class ModelGroupNet; }
namespace RealityDataPackageWrapper { ref class PinnedGroupNet; }
namespace RealityDataPackageWrapper { ref class TerrainGroupNet; }
namespace RealityDataPackageWrapper { ref class RealityDataSourceNet; }

namespace RealityDataPackageWrapper
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               6/2015
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            //! Create package.
            static void Create(System::String^ location, 
                               System::String^ name,
                               System::String^ description,
                               System::String^ copyright,
                               System::String^ packageId,
                               List<double>^ regionOfInterest,
                               ImageryGroupNet^ imageryData,
                               ModelGroupNet^ modelData,
                               PinnedGroupNet^ pinnedData,
                               TerrainGroupNet^ terrainData);
        
        private:
            RealityDataPackageNet();
            ~RealityDataPackageNet();
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ImageryGroupNet
        {
        public:
            //! Create imagery group.
            static ImageryGroupNet^ Create();

            void AddData(RealityDataSourceNet^ imageryData);

            List<RealityDataSourceNet^>^ GetData() { return m_imageryDataList; }

        private:
            ImageryGroupNet();
            ~ImageryGroupNet();

            List<RealityDataSourceNet^>^ m_imageryDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ModelGroupNet
        {
        public:
            //! Create model group.
            static ModelGroupNet^ Create();

            void AddData(RealityDataSourceNet^ modelData);

            List<RealityDataSourceNet^>^ GetData() { return m_modelDataList; }

        private:
            ModelGroupNet();
            ~ModelGroupNet();

            List<RealityDataSourceNet^>^ m_modelDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class PinnedGroupNet
        {
        public:
            //! Create pinned group.
            static PinnedGroupNet^ Create();

            void AddData(RealityDataSourceNet^ pinnedData);

            List<RealityDataSourceNet^>^ GetData() { return m_pinnedDataList; }

        private:
            PinnedGroupNet();
            ~PinnedGroupNet();

            List<RealityDataSourceNet^>^ m_pinnedDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class TerrainGroupNet
        {
        public:
            //! Create terrain group.
            static TerrainGroupNet^ Create();

            void AddData(RealityDataSourceNet^ terrainData);

            List<RealityDataSourceNet^>^ GetData() { return m_terrainDataList; }

        private:
            TerrainGroupNet();
            ~TerrainGroupNet();

            List<RealityDataSourceNet^>^ m_terrainDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class RealityDataSourceNet
        {
        public:
            //! Create generic data source.
            static RealityDataSourceNet^ Create(System::String^ uri,
                                                System::String^ type,
                                                System::String^ copyright,
                                                System::String^ id,
                                                System::String^ provider,
                                                uint64_t filesize,
                                                System::String^ fileInCompound,
                                                System::String^ metadata, 
                                                List<System::String^>^ sisterFiles);

            //! Get the source uri. It could be a full URL or a path relative to the package file.
            System::String^ GetUri() { return m_uri; };

            //! Get the source type.
            System::String^ GetSourceType() { return m_type; }

            //! Get the copyright. Might be empty.
            System::String^ GetCopyright() { return m_copyright; }

            //! Get the id. Might be empty.
            System::String^ GetId() { return m_id; }

            //! Get the provider. Might be empty.
            System::String^ GetProvider() { return m_provider; }

            //! Get the size in kilobytes. Default is 0. 
            uint64_t GetFileSize() { return m_filesize; }

            //! Get main file in compound. Might be empty.
            System::String^ GetFileInCompound() { return m_fileInCompound; }

            //! Get the metadata. Might be empty.
            System::String^ GetMetadata() { return m_metadata; }

            //! Get the sister files. Might be empty.
            List<System::String^>^ GetSisterFiles() { return m_sisterFiles; }

        protected:
            RealityDataSourceNet(System::String^ uri,
                                 System::String^ type,
                                 System::String^ copyright,
                                 System::String^ id,
                                 System::String^ provider,
                                 uint64_t filesize,
                                 System::String^ fileInCompound,
                                 System::String^ metadata,
                                 List<System::String^>^ sisterFiles);

            ~RealityDataSourceNet();

        private:
            System::String^ m_uri;
            System::String^ m_type;
            System::String^ m_copyright;
            System::String^ m_id;
            System::String^ m_provider;
            uint64_t m_filesize;
            System::String^ m_fileInCompound;
            System::String^ m_metadata;
            List<System::String^>^ m_sisterFiles;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               5/2015
    //=====================================================================================
    public ref class WmsSourceNet : public RealityDataSourceNet
        {
        public:
            //! Create WMS data source.
            static WmsSourceNet^ Create(System::String^ uri,
                                        System::String^ copyright,
                                        System::String^ id,
                                        System::String^ provider,
                                        uint64_t filesize,
                                        System::String^ metadata,
                                        List<System::String^>^ sisterFiles,
                                        System::String^ mapUri,
                                        double bboxMinX,
                                        double bboxMinY,
                                        double bboxMaxX,
                                        double bboxMaxY,
                                        System::String^ version,
                                        System::String^ layers,
                                        System::String^ csType,
                                        System::String^ csLabel,
                                        size_t metaWidth,              
                                        size_t metaHeight,             
                                        System::String^ styles,        
                                        System::String^ format,        
                                        System::String^ vendorSpecific,
                                        bool isTransparent);           

            //! Get XML representation.
            System::String^ GetXmlFragment() { return m_xmlFragment; }

        private:
            WmsSourceNet(System::String^ uri,
                         System::String^ copyright,
                         System::String^ id,
                         System::String^ provider,
                         uint64_t filesize,
                         System::String^ metadata,
                         List<System::String^>^ sisterFiles,
                         System::String^ mapUri,
                         double bboxMinX,
                         double bboxMinY,
                         double bboxMaxX,
                         double bboxMaxY,
                         System::String^ version,
                         System::String^ layers,
                         System::String^ csType,
                         System::String^ csLabel,
                         size_t metaWidth,              
                         size_t metaHeight,             
                         System::String^ styles,        
                         System::String^ format,        
                         System::String^ vendorSpecific,
                         bool isTransparent);     

            ~WmsSourceNet();

            System::String^ m_xmlFragment;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              11/2015
    //=====================================================================================
    public ref class OsmSourceNet : public RealityDataSourceNet
        {
        public:
            //! Create OSM data source.
            static OsmSourceNet^ Create(System::String^ uri,
                                        System::String^ copyright,
                                        System::String^ id,
                                        System::String^ provider,
                                        uint64_t filesize,
                                        System::String^ metadata,
                                        List<System::String^>^ sisterFiles,
                                        List<double>^ regionOfInterest,
                                        List<System::String^>^ urls);

            //! Get XML representation.
            System::String^ GetXmlFragment() { return m_xmlFragment; }

        private:
            OsmSourceNet(System::String^ uri,
                         System::String^ copyright,
                         System::String^ id,
                         System::String^ provider,
                         uint64_t filesize,
                         System::String^ metadata,
                         List<System::String^>^ sisterFiles,
                         List<double>^ regionOfInterest,
                         List<System::String^>^ urls);

            ~OsmSourceNet();

            System::String^ m_xmlFragment;
        };
    }
