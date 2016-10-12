/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/RealityDataPackageNet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityDataPackage.h>

//#using  <mscorlib.dll>

// Forward declaration
namespace RealityPackageNet { ref class ImageryDataNet; }
namespace RealityPackageNet { ref class ModelDataNet; }
namespace RealityPackageNet { ref class PinnedDataNet; }
namespace RealityPackageNet { ref class TerrainDataNet; }
namespace RealityPackageNet { ref class RealityDataSourceNet; }

namespace RealityPackageNet
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            //! Create a new empty package.
            static RealityDataPackageNet^ Create(System::String^ name);

            //! Store the content of this instance to disk. 
            //! If at the time of writing the creation date is invalid a valid one will be created with the current date.
            bool Write(System::String^ filename);

            //! The origin of this package file. Specified the Context service or GeoCoordinate Service server.
            System::String^ GetOrigin();
            void SetOrigin(System::String^ origin);

            //! The requesting application of this package file. 
            System::String^ GetRequestingApplication();
            void SetRequestingApplication(System::String^ requestingApplication);

            //! The name of this package file.
            System::String^ GetName();
            void SetName(System::String^ name);

            //! Package description. Might be empty.
            System::String^ GetDescription();
            void SetDescription(System::String^ description);

            //! Copyright information, Might be empty.
            System::String^ GetCopyright();
            void SetCopyright(System::String^ copyright);

            //! Package tracking Id. Might be empty.
            System::String^ GetId();
            void SetId(System::String^ id);

            //! Package bounding polygon in latitude/longitude.
            //! The bounding polygon for the RealityDataPackage represents the region of interest selected by the user in the RealityModelingNavigator. 
            //! This can be seen as the clipping shape for all the data that the package will contain.
            System::Collections::Generic::List<double>^ GetBoundingPolygon();
            //! Package object will increment ref count of 'polygon'.
            void SetBoundingPolygon(System::Collections::Generic::List<double>^ polygonPts);

            //! Add data sources to corresponding group.
            void AddImageryData(ImageryDataNet^ data);
            void AddModelData(ModelDataNet^ data);
            void AddPinnedData(PinnedDataNet^ data);
            void AddTerrainData(TerrainDataNet^ data);

            //! Get package version.
            int GetMajorVersion();
            void SetMajorVersion(int major);
            int GetMinorVersion();
            void SetMinorVersion(int minor);
   
        private:
            RealityDataPackageNet(System::String^ name);
            ~RealityDataPackageNet();
            !RealityDataPackageNet();

            RealityPackage::RealityDataPackagePtr* m_pPackage;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class RealityDataNet
        {
        public:
            System::String^ GetDataId();
            void SetDataId(System::String^ dataId);

            System::String^ GetDataName();
            void SetDataName(System::String^ dataName);

            System::String^ GetDataset();
            void SetDataset(System::String^ dataset);

            //! Data can contain many sources (at least one). This returns the number of sources.
            int GetNumSources();

            //! Returns the source. index start at 0 up to GetNumSource()-1
            //RealityDataSourceNet^ GetSourceR(int index);
            RealityDataSourceNet^ GetSource(int index);

            //! Adds an alternate source to the data.
            void AddSource(RealityDataSourceNet^ dataSource);

        protected:
            RealityDataNet(RealityDataSourceNet^ dataSource);
            ~RealityDataNet() {};
            !RealityDataNet() {};

        private:
            System::Collections::Generic::List<RealityDataSourceNet^>^ m_sources;
            System::String^ m_id;
            System::String^ m_name;
            System::String^ m_dataset;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ImageryDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive DPoint2d structures in an array
            static ImageryDataNet^ Create(RealityDataSourceNet^ dataSource, System::Collections::Generic::List<double>^ corners);

            //! Imagery corners in lat/long.
            //! May return NULL. In such a case the corners should be read from the file header. The pointer returned is the 
            //! start of a 4 DPoint2d array.
            System::Collections::Generic::List<double>^ GetCornersCP();

            //! Set Imagery corners. nullptr can be passed to remove corners. If corners are provided then
            //! the pointer given must point to 4 consecutive DPoint2d structures in an array
            void SetCorners(System::Collections::Generic::List<double>^ pCorners);

            static Utf8CP ElementName;

        private:
            ImageryDataNet(RealityDataSourceNet^ dataSource, System::Collections::Generic::List<double>^ corners);
            ~ImageryDataNet();
            !ImageryDataNet();

            RealityPackage::ImageryDataPtr* m_pImageryData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ModelDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive DPoint2d structures in an array
            static ModelDataNet^ Create(RealityDataSourceNet^ dataSource);

            static Utf8CP ElementName;

        private:
            ModelDataNet(RealityDataSourceNet^ dataSource);
            ~ModelDataNet();
            !ModelDataNet();

            RealityPackage::ModelDataPtr* m_pModelData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class PinnedDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive DPoint2d structures in an array
            static PinnedDataNet^ Create(RealityDataSourceNet^ dataSource, double longitude, double latitude);

            //! Get the object location in long/lat coordinate. 
            System::Collections::Generic::List<double>^ GetLocation();

            //! Set the object location in long/lat coordinate. 
            //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
            //! False is returned if location is invalid.
            bool SetLocation(double longitude, double latitude);

            //! Returns true if the location has a defined area (a polygon)
            bool HasArea();

            //! Get the object polygon location in long/lat coordinate. If the location is not defined by a polygon 
            //! a nullptr is returned. First check with HasPolygonLocation() before calling this method.
            System::Collections::Generic::List<double>^ GetAreaCP();

            //! Set the object location as a polygon in a sequence long/lat coordinate. 
            //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
            //! The polygon can be convex or not but it must not be autocontiguous or auto crossing
            //! False is returned if polygon is invalid.
            bool SetArea(System::Collections::Generic::List<double>^ polygonPts);

            static Utf8CP ElementName;

        private:
            PinnedDataNet(RealityDataSourceNet^ dataSource, double longitude, double latitude);
            ~PinnedDataNet();
            !PinnedDataNet();

            RealityPackage::PinnedDataPtr* m_pPinnedData;
        };    

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class TerrainDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive DPoint2d structures in an array
            static TerrainDataNet^ Create(RealityDataSourceNet^ dataSource);

            static Utf8CP ElementName;

        private:
            TerrainDataNet(RealityDataSourceNet^ dataSource);
            ~TerrainDataNet();
            !TerrainDataNet();

            RealityPackage::TerrainDataPtr* m_pTerrainData;
        };
    }

//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               9/2015
//    //=====================================================================================
//    public ref class ImageryGroupNet
//        {
//        public:
//            //! Create imagery group.
//            static ImageryGroupNet^ Create();
//
//            void AddData(DataGroupNet^ imageryData);
//
//            List<DataGroupNet^>^ GetData() { return m_imageryDataList; }
//
//        private:
//            ImageryGroupNet();
//            ~ImageryGroupNet();
//
//            List<DataGroupNet^>^ m_imageryDataList;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               9/2015
//    //=====================================================================================
//    public ref class ModelGroupNet
//        {
//        public:
//            //! Create model group.
//            static ModelGroupNet^ Create();
//
//            void AddData(DataGroupNet^ modelData);
//
//            List<DataGroupNet^>^ GetData() { return m_modelDataList; }
//
//        private:
//            ModelGroupNet();
//            ~ModelGroupNet();
//
//            List<DataGroupNet^>^ m_modelDataList;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               9/2015
//    //=====================================================================================
//    public ref class PinnedGroupNet
//        {
//        public:
//            //! Create pinned group.
//            static PinnedGroupNet^ Create();
//
//            void AddData(DataGroupNet^ pinnedData);
//
//            List<DataGroupNet^>^ GetData() { return m_pinnedDataList; }
//
//        private:
//            PinnedGroupNet();
//            ~PinnedGroupNet();
//
//            List<DataGroupNet^>^ m_pinnedDataList;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               9/2015
//    //=====================================================================================
//    public ref class TerrainGroupNet
//        {
//        public:
//            //! Create terrain group.
//            static TerrainGroupNet^ Create();
//
//            void AddData(DataGroupNet^ terrainData);
//
//            List<DataGroupNet^>^ GetData() { return m_terrainDataList; }
//
//        private:
//            TerrainGroupNet();
//            ~TerrainGroupNet();
//
//            List<DataGroupNet^>^ m_terrainDataList;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote              7/2016
//    //=====================================================================================
//    public ref class DataGroupNet
//        {
//        public:
//            //! Create data group with main source.
//            static DataGroupNet^ Create(System::String^ id,
//                                        System::String^ name,
//                                        DataSourceNet^ source);
//
//            //! Get id.
//            System::String^ GetId() { return m_id; };
//
//            //! Get name.
//            System::String^ GetName() { return m_name; }
//
//            //! Get the number of sources.
//            int GetNumSources() { return m_sources->Count; }
//
//            //! Get the sources.
//            List<DataSourceNet^>^ GetSources() { return m_sources; }
//
//            //! Adds an alternate source to the data.
//            void AddSource(DataSourceNet^ dataSource);
//
//        protected:
//            DataGroupNet(System::String^ id,
//                           System::String^ name,
//                           DataSourceNet^ source);
//
//            ~DataGroupNet();
//
//        private:
//            System::String^ m_id;
//            System::String^ m_name;
//            List<DataSourceNet^>^ m_sources;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               9/2015
//    //=====================================================================================
//    public ref class DataSourceNet
//        {
//        public:
//            //! Create generic data source.
//            static DataSourceNet^ Create(System::String^ uri,
//                                         System::String^ type,
//                                         System::String^ copyright,
//                                         System::String^ id,
//                                         System::String^ provider,
//                                         uint64_t size,
//                                         System::String^ fileInCompound,
//                                         System::String^ metadata,
//                                         System::String^ geocs,
//                                         List<System::String^>^ sisterFiles);
//
//            //! Get the source uri. It could be a full URL or a path relative to the package file.
//            System::String^ GetUri() { return m_uri; };
//
//            //! Get the source type.
//            System::String^ GetSourceType() { return m_type; }
//
//            //! Get the copyright. Might be empty.
//            System::String^ GetCopyright() { return m_copyright; }
//
//            //! Get the id. Might be empty.
//            System::String^ GetId() { return m_id; }
//
//            //! Get the provider. Might be empty.
//            System::String^ GetProvider() { return m_provider; }
//
//            //! Get the size in kilobytes. Default is 0. 
//            uint64_t GetSize() { return m_size; }
//
//            //! Get main file in compound. Might be empty.
//            System::String^ GetFileInCompound() { return m_fileInCompound; }
//
//            //! Get the metadata. Might be empty.
//            System::String^ GetMetadata() { return m_metadata; }
//
//            //! Get the geocs. Might be empty.
//            System::String^ GetGeoCS() { return m_geocs; }
//
//            //! Get the sister files. Might be empty.
//            List<System::String^>^ GetSisterFiles() { return m_sisterFiles; }
//
//        protected:
//            DataSourceNet(System::String^ uri,
//                          System::String^ type,
//                          System::String^ copyright,
//                          System::String^ id,
//                          System::String^ provider,
//                          uint64_t size,
//                          System::String^ fileInCompound,
//                          System::String^ metadata,
//                          System::String^ geocs,
//                          List<System::String^>^ sisterFiles);
//
//            ~DataSourceNet();
//
//        private:
//            System::String^ m_uri;
//            System::String^ m_type;
//            System::String^ m_copyright;
//            System::String^ m_id;
//            System::String^ m_provider;
//            uint64_t m_size;
//            System::String^ m_fileInCompound;
//            System::String^ m_metadata;
//            System::String^ m_geocs;
//            List<System::String^>^ m_sisterFiles;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote               5/2015
//    //=====================================================================================
//    public ref class WmsSourceNet : public DataSourceNet
//        {
//        public:
//            //! Create WMS data source.
//            static WmsSourceNet^ Create(System::String^ uri,
//                                        System::String^ copyright,
//                                        System::String^ id,
//                                        System::String^ provider,
//                                        uint64_t filesize,
//                                        System::String^ metadata,
//                                        System::String^ geocs,
//                                        List<System::String^>^ sisterFiles,
//                                        System::String^ mapUri,
//                                        double bboxMinX,
//                                        double bboxMinY,
//                                        double bboxMaxX,
//                                        double bboxMaxY,
//                                        System::String^ version,
//                                        System::String^ layers,
//                                        System::String^ csType,
//                                        System::String^ csLabel,
//                                        size_t metaWidth,              
//                                        size_t metaHeight,             
//                                        System::String^ styles,        
//                                        System::String^ format,        
//                                        System::String^ vendorSpecific,
//                                        bool isTransparent);           
//
//            //! Get XML representation.
//            System::String^ GetXmlFragment() { return m_xmlFragment; }
//
//        private:
//            WmsSourceNet(System::String^ uri,
//                         System::String^ copyright,
//                         System::String^ id,
//                         System::String^ provider,
//                         uint64_t filesize,
//                         System::String^ metadata,
//                         System::String^ geocs,
//                         List<System::String^>^ sisterFiles,
//                         System::String^ mapUri,
//                         double bboxMinX,
//                         double bboxMinY,
//                         double bboxMaxX,
//                         double bboxMaxY,
//                         System::String^ version,
//                         System::String^ layers,
//                         System::String^ csType,
//                         System::String^ csLabel,
//                         size_t metaWidth,              
//                         size_t metaHeight,             
//                         System::String^ styles,        
//                         System::String^ format,        
//                         System::String^ vendorSpecific,
//                         bool isTransparent);     
//
//            ~WmsSourceNet();
//
//            System::String^ m_xmlFragment;
//        };
//
//    //=====================================================================================
//    //! @bsiclass                                   Jean-Francois.Cote              11/2015
//    //=====================================================================================
//    public ref class OsmSourceNet : public DataSourceNet
//        {
//        public:
//            //! Create OSM data source.
//            static OsmSourceNet^ Create(System::String^ uri,
//                                        System::String^ copyright,
//                                        System::String^ id,
//                                        System::String^ provider,
//                                        uint64_t filesize,
//                                        System::String^ metadata,
//                                        System::String^ geocs,
//                                        List<System::String^>^ sisterFiles,
//                                        List<double>^ regionOfInterest,
//                                        List<System::String^>^ urls);
//
//            //! Get XML representation.
//            System::String^ GetXmlFragment() { return m_xmlFragment; }
//
//        private:
//            OsmSourceNet(System::String^ uri,
//                         System::String^ copyright,
//                         System::String^ id,
//                         System::String^ provider,
//                         uint64_t filesize,
//                         System::String^ metadata,
//                         System::String^ geocs,
//                         List<System::String^>^ sisterFiles,
//                         List<double>^ regionOfInterest,
//                         List<System::String^>^ urls);
//
//            ~OsmSourceNet();
//
//            System::String^ m_xmlFragment;
//        };
//    }
