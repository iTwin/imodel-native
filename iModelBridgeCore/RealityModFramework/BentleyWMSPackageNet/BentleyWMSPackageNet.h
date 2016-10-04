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
namespace RealityDataPackageWrapper { ref class ImageryDataNet; }
namespace RealityDataPackageWrapper { ref class ModelDataNet; }
namespace RealityDataPackageWrapper { ref class PinnedDataNet; }
namespace RealityDataPackageWrapper { ref class TerrainDataNet; }
namespace RealityDataPackageWrapper { ref class RealityDataSourceNet; }

namespace RealityDataPackageWrapper
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               6/2015
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            typedef List<ImageryDataNet^>   ImageryGroupNet;
            typedef List<ModelDataNet^>     ModelGroupNet;
            typedef List<PinnedDataNet^>    PinnedGroupNet;
            typedef List<TerrainDataNet^>   TerrainGroupNet;

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
            RealityDataPackageNet() {};
            ~RealityDataPackageNet() {};

            static RealityPackage::RealityDataSourcePtr CreateDataSource(RealityDataSourceNet^ source);
            static RealityPackage::WmsDataSourcePtr CreateWmsDataSource(RealityDataSourceNet^ source);
            static RealityPackage::OsmDataSourcePtr CreateOsmDataSource(RealityDataSourceNet^ source, RealityPackage::BoundingPolygonPtr boundingPolygon);
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              7/2016
    //=====================================================================================
    public ref class RealityDataNet
        {
        public:
            //! Get id.
            System::String^ GetId() { return m_id; };

            //! Get name.
            System::String^ GetName() { return m_name; }

            //! Get the number of sources.
            int GetNumSources() { return m_sources->Count; }

            //! Get the sources.
            List<RealityDataSourceNet^>^ GetSources() { return m_sources; }

            //! Adds an alternate source to the data.
            void AddSource(RealityDataSourceNet^ dataSource);

        protected:
            RealityDataNet(System::String^ id,
                           System::String^ name,
                           RealityDataSourceNet^ source);

            ~RealityDataNet() {};

        private:
            System::String^ m_id;
            System::String^ m_name;
            List<RealityDataSourceNet^>^ m_sources;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ImageryDataNet : public RealityDataNet
        {
        public:
            //! Create data of imagery type with main source.
            static ImageryDataNet^ Create(System::String^ id,
                                          System::String^ name,
                                          RealityDataSourceNet^ dataSource, 
                                          List<double>^ corners);

            //! Get corners.
            List<double>^ GetCorners() { return m_corners; }

        protected:
            ImageryDataNet(System::String^ id, System::String^ name, RealityDataSourceNet^ dataSource, List<double>^ corners);
            ~ImageryDataNet() {}

        private:
            List<double>^ m_corners;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ModelDataNet : public RealityDataNet
        {
        public:
            //! Create data of model type.
            static ModelDataNet^ Create(System::String^ id,
                                        System::String^ name,
                                        RealityDataSourceNet^ dataSource);

        protected:
            ModelDataNet(System::String^ id, System::String^ name, RealityDataSourceNet^ dataSource);
            ~ModelDataNet() {};

        private:

        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class PinnedDataNet : public RealityDataNet
        {
        public:
            //! Create data of pinned type.
            static PinnedDataNet^ Create(System::String^ id,
                                         System::String^ name,
                                         RealityDataSourceNet^ dataSource,
                                         double longitude,
                                         double latitude);

            //! Get position.
            List<double>^ GetPosition() { return m_position; }

            //! Get area.
            List<double>^ GetArea() { return m_area; }

        protected:
            PinnedDataNet(System::String^ id, System::String^ name, RealityDataSourceNet^ dataSource, double longitude, double latitude);
            ~PinnedDataNet() {};

        private:
            List<double>^ m_position;
            List<double>^ m_area;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class TerrainDataNet : public RealityDataNet
        {
    public:
        //! Create data of imagery type.
        static TerrainDataNet^ Create(System::String^ id,
                                      System::String^ name,
                                      RealityDataSourceNet^ dataSource);

    protected:
        TerrainDataNet(System::String^ id, System::String^ name, RealityDataSourceNet^ dataSource);
        ~TerrainDataNet() {};

    private:

        };

    
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class RealityDataSourceNet
        {
        public:
            //! Create generic data source.
            static RealityDataSourceNet^ Create(System::String^ uri,
                                                System::String^ fileInCompound,
                                                System::String^ type,
                                                System::String^ id,
                                                System::String^ copyright,                                                
                                                System::String^ provider,
                                                uint64_t size,
                                                System::String^ metadata,
                                                System::String^ metadataType,
                                                System::String^ geocs,
                                                System::String^ nodatavalue,
                                                List<System::String^>^ sisterFiles);

            //! Get the source uri. It could be a full URL or a path relative to the package file.
            System::String^ GetUri() { return m_uri; };

            //! Get main file in compound. Might be empty.
            System::String^ GetFileInCompound() { return m_fileInCompound; }

            //! Get the source type.
            System::String^ GetSourceType() { return m_type; }

            //! Get the id. Might be empty.
            System::String^ GetId() { return m_id; }

            //! Get the copyright. Might be empty.
            System::String^ GetCopyright() { return m_copyright; }

            //! Get the provider. Might be empty.
            System::String^ GetProvider() { return m_provider; }

            //! Get the size in kilobytes. Default is 0. 
            uint64_t GetSize() { return m_size; }            

            //! Get the metadata. Might be empty.
            System::String^ GetMetadata() { return m_metadata; }

            //! Get the metadata type. Might be empty.
            System::String^ GetMetadataType() { return m_metadataType; }

            //! Get the geocs. Might be empty.
            System::String^ GetGeoCS() { return m_geocs; }

            //! Get the nodata value. Might be empty.
            System::String^ GetNoDataValue() { return m_nodatavalue; }

            //! Get the sister files. Might be empty.
            List<System::String^>^ GetSisterFiles() { return m_sisterFiles; }

        protected:
            RealityDataSourceNet(System::String^ uri,
                                 System::String^ fileInCompound,
                                 System::String^ type,
                                 System::String^ id,
                                 System::String^ copyright,
                                 System::String^ provider,
                                 uint64_t size,                                 
                                 System::String^ metadata,
                                 System::String^ metadataType,
                                 System::String^ geocs,
                                 System::String^ nodatavalue,
                                 List<System::String^>^ sisterFiles);

            ~RealityDataSourceNet() {};

        private:
            System::String^ m_uri;
            System::String^ m_fileInCompound;
            System::String^ m_type;
            System::String^ m_copyright;
            System::String^ m_id;
            System::String^ m_provider;
            uint64_t m_size;            
            System::String^ m_metadata;
            System::String^ m_metadataType;
            System::String^ m_geocs;
            System::String^ m_nodatavalue;
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
                                        System::String^ fileInCompound,
                                        System::String^ type,
                                        System::String^ id,
                                        System::String^ copyright,
                                        System::String^ provider,
                                        uint64_t size,
                                        System::String^ metadata,
                                        System::String^ metadataType,
                                        System::String^ geocs,
                                        System::String^ nodatavalue,
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
                         System::String^ fileInCompound,
                         System::String^ type,
                         System::String^ id,
                         System::String^ copyright,
                         System::String^ provider,
                         uint64_t size,
                         System::String^ metadata,
                         System::String^ metadataType,
                         System::String^ geocs,
                         System::String^ nodatavalue,
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
                                        System::String^ fileInCompound,
                                        System::String^ type,
                                        System::String^ id,
                                        System::String^ copyright,
                                        System::String^ provider,
                                        uint64_t size,
                                        System::String^ metadata,
                                        System::String^ metadataType,
                                        System::String^ geocs,
                                        System::String^ nodatavalue,
                                        List<System::String^>^ sisterFiles,
                                        List<double>^ regionOfInterest,
                                        List<System::String^>^ urls);

            //! Get XML representation.
            System::String^ GetXmlFragment() { return m_xmlFragment; }

        private:
            OsmSourceNet(System::String^ uri,
                         System::String^ fileInCompound,
                         System::String^ type,
                         System::String^ id,
                         System::String^ copyright,
                         System::String^ provider,
                         uint64_t size,
                         System::String^ metadata,
                         System::String^ metadataType,
                         System::String^ geocs,
                         System::String^ nodatavalue,
                         List<System::String^>^ sisterFiles,
                         List<double>^ regionOfInterest,
                         List<System::String^>^ urls);

            ~OsmSourceNet();

            System::String^ m_xmlFragment;
        };
    }
