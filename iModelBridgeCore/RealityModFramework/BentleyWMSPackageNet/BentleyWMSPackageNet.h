/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeFileName.h>
#include <RealityPackage/RealityPackage.h>
#include <RealityPackage/RealityDataPackage.h>
#include <RealityPackage/RealityDataSource.h>


#using  <mscorlib.dll>

using RealityPackage::RealityDataPackage;
USING_NAMESPACE_BENTLEY_REALITYPACKAGE
using namespace System::Collections::Generic;


// Forward declaration
namespace RealityDataPackageWrapper { ref class ImageryGroupNet; }
namespace RealityDataPackageWrapper { ref class ModelGroupNet; }
namespace RealityDataPackageWrapper { ref class PinnedGroupNet; }
namespace RealityDataPackageWrapper { ref class TerrainGroupNet; }
namespace RealityDataPackageWrapper { ref class DataGroupNet; }
namespace RealityDataPackageWrapper { ref class DataSourceNet; }

namespace RealityDataPackageWrapper
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               6/2015
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            //! Create version 1 package.
            static void CreateV1(System::String^ location,
                                 System::String^ name,
                                 System::String^ description,
                                 System::String^ copyright,
                                 System::String^ id,
                                 List<double>^ regionOfInterest,
                                 ImageryGroupNet^ imageryGroup,
                                 ModelGroupNet^ modelGroup,
                                 PinnedGroupNet^ pinnedGroup,
                                 TerrainGroupNet^ terrainGroup);

            //! Create version 2 package.
            static void CreateV2(System::String^ location, 
                                 System::String^ origin,
                                 System::String^ name,
                                 System::String^ description,
                                 System::String^ copyright,
                                 System::String^ id,
                                 List<double>^ regionOfInterest,
                                 ImageryGroupNet^ imageryGroup,
                                 ModelGroupNet^ modelGroup,
                                 PinnedGroupNet^ pinnedGroup,
                                 TerrainGroupNet^ terrainGroup);
        
        private:
            RealityDataPackageNet();
            ~RealityDataPackageNet();

            static RealityPackage::RealityDataSourcePtr CreateDataSource(DataSourceNet^ source);
            static RealityPackage::WmsDataSourcePtr CreateWmsDataSource(DataSourceNet^ source);
            static RealityPackage::OsmDataSourcePtr CreateOsmDataSource(DataSourceNet^ source, RealityPackage::BoundingPolygonPtr boundingPolygon);
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ImageryGroupNet
        {
        public:
            //! Create imagery group.
            static ImageryGroupNet^ Create();

            void AddData(DataGroupNet^ imageryData);

            List<DataGroupNet^>^ GetData() { return m_imageryDataList; }

        private:
            ImageryGroupNet();
            ~ImageryGroupNet();

            List<DataGroupNet^>^ m_imageryDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ModelGroupNet
        {
        public:
            //! Create model group.
            static ModelGroupNet^ Create();

            void AddData(DataGroupNet^ modelData);

            List<DataGroupNet^>^ GetData() { return m_modelDataList; }

        private:
            ModelGroupNet();
            ~ModelGroupNet();

            List<DataGroupNet^>^ m_modelDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class PinnedGroupNet
        {
        public:
            //! Create pinned group.
            static PinnedGroupNet^ Create();

            void AddData(DataGroupNet^ pinnedData);

            List<DataGroupNet^>^ GetData() { return m_pinnedDataList; }

        private:
            PinnedGroupNet();
            ~PinnedGroupNet();

            List<DataGroupNet^>^ m_pinnedDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class TerrainGroupNet
        {
        public:
            //! Create terrain group.
            static TerrainGroupNet^ Create();

            void AddData(DataGroupNet^ terrainData);

            List<DataGroupNet^>^ GetData() { return m_terrainDataList; }

        private:
            TerrainGroupNet();
            ~TerrainGroupNet();

            List<DataGroupNet^>^ m_terrainDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              7/2016
    //=====================================================================================
    public ref class DataGroupNet
        {
        public:
            //! Create data group with main source.
            static DataGroupNet^ Create(System::String^ id,
                                        System::String^ name,
                                        DataSourceNet^ source);

            //! Get id.
            System::String^ GetId() { return m_id; };

            //! Get name.
            System::String^ GetName() { return m_name; }

            //! Get the number of sources.
            int GetNumSources() { return m_sources->Count; }

            //! Get the sources.
            List<DataSourceNet^>^ GetSources() { return m_sources; }

            //! Adds an alternate source to the data.
            void AddSource(DataSourceNet^ dataSource);

        protected:
            DataGroupNet(System::String^ id,
                           System::String^ name,
                           DataSourceNet^ source);

            ~DataGroupNet();

        private:
            System::String^ m_id;
            System::String^ m_name;
            List<DataSourceNet^>^ m_sources;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class DataSourceNet
        {
        public:
            //! Create generic data source.
            static DataSourceNet^ Create(System::String^ uri,
                                         System::String^ type,
                                         System::String^ copyright,
                                         System::String^ id,
                                         System::String^ provider,
                                         uint64_t size,
                                         System::String^ fileInCompound,
                                         System::String^ metadata,
                                         System::String^ geocs,
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
            uint64_t GetSize() { return m_size; }

            //! Get main file in compound. Might be empty.
            System::String^ GetFileInCompound() { return m_fileInCompound; }

            //! Get the metadata. Might be empty.
            System::String^ GetMetadata() { return m_metadata; }

            //! Get the geocs. Might be empty.
            System::String^ GetGeoCS() { return m_geocs; }

            //! Get the sister files. Might be empty.
            List<System::String^>^ GetSisterFiles() { return m_sisterFiles; }

        protected:
            DataSourceNet(System::String^ uri,
                          System::String^ type,
                          System::String^ copyright,
                          System::String^ id,
                          System::String^ provider,
                          uint64_t size,
                          System::String^ fileInCompound,
                          System::String^ metadata,
                          System::String^ geocs,
                          List<System::String^>^ sisterFiles);

            ~DataSourceNet();

        private:
            System::String^ m_uri;
            System::String^ m_type;
            System::String^ m_copyright;
            System::String^ m_id;
            System::String^ m_provider;
            uint64_t m_size;
            System::String^ m_fileInCompound;
            System::String^ m_metadata;
            System::String^ m_geocs;
            List<System::String^>^ m_sisterFiles;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               5/2015
    //=====================================================================================
    public ref class WmsSourceNet : public DataSourceNet
        {
        public:
            //! Create WMS data source.
            static WmsSourceNet^ Create(System::String^ uri,
                                        System::String^ copyright,
                                        System::String^ id,
                                        System::String^ provider,
                                        uint64_t filesize,
                                        System::String^ metadata,
                                        System::String^ geocs,
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
                         System::String^ geocs,
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
    public ref class OsmSourceNet : public DataSourceNet
        {
        public:
            //! Create OSM data source.
            static OsmSourceNet^ Create(System::String^ uri,
                                        System::String^ copyright,
                                        System::String^ id,
                                        System::String^ provider,
                                        uint64_t filesize,
                                        System::String^ metadata,
                                        System::String^ geocs,
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
                         System::String^ geocs,
                         List<System::String^>^ sisterFiles,
                         List<double>^ regionOfInterest,
                         List<System::String^>^ urls);

            ~OsmSourceNet();

            System::String^ m_xmlFragment;
        };
    }
