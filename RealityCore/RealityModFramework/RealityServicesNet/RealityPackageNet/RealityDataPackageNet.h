/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityDataPackage.h>

// Forward declaration
namespace RealityPackageNet { ref class ImageryDataNet; }
namespace RealityPackageNet { ref class ModelDataNet; }
namespace RealityPackageNet { ref class PinnedDataNet; }
namespace RealityPackageNet { ref class TerrainDataNet; }
namespace RealityPackageNet { ref class UndefinedDataNet; }
namespace RealityPackageNet { ref class SpatialEntityDataSourceNet; }

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

            //! Creation date. Might be empty.
            System::String^ GetCreationDate();
            void SetCreationDate(System::String^ date);

            //! Package bounding polygon in latitude/longitude.
            //! The bounding polygon for the RealityDataPackage represents the region of interest selected by the user in the RealityModelingNavigator. 
            //! This can be seen as the clipping shape for all the data that the package will contain.
            System::Collections::Generic::List<double>^ GetBoundingPolygon();
            //! Package object will increment ref count of 'polygon'.
            void SetBoundingPolygon(System::Collections::Generic::List<double>^ polygonPts);

            //! Package context. Might be empty.
            System::String^ GetContext();
            void SetContext(System::String^ context);

            //! Add data sources to corresponding group.
            void AddImageryData(ImageryDataNet^ data);
            void AddModelData(ModelDataNet^ data);
            void AddPinnedData(PinnedDataNet^ data);
            void AddTerrainData(TerrainDataNet^ data);
            void AddUndefinedData(UndefinedDataNet^ data);

            //! Get package version.
            int GetMajorVersion();
            void SetMajorVersion(int major);
            int GetMinorVersion();
            void SetMinorVersion(int minor);
   
        private:
            RealityDataPackageNet(System::String^ name);
            ~RealityDataPackageNet();
            !RealityDataPackageNet();

            RealityPlatform::RealityDataPackagePtr* m_pPackage;
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

            System::String^ GetResolution();
            void SetResolution(System::String^ resolution);

            //! Data can contain many sources (at least one). This returns the number of sources.
            int GetNumSources();

            //! Returns the source. index start at 0 up to GetNumSource()-1
            //SpatialEntityDataSourceNet^ GetSourceR(int index);
            SpatialEntityDataSourceNet^ GetSource(int index);

            //! Adds an alternate source to the data.
            void AddSource(SpatialEntityDataSourceNet^ dataSource);

        protected:
            RealityDataNet(SpatialEntityDataSourceNet^ dataSource);
            ~RealityDataNet() {};
            !RealityDataNet() {};

        private:
            System::Collections::Generic::List<SpatialEntityDataSourceNet^>^ m_sources;
            System::String^ m_id;
            System::String^ m_name;
            System::String^ m_dataset;
            System::String^ m_resolution;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ImageryDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array
            static ImageryDataNet^ Create(SpatialEntityDataSourceNet^ dataSource, System::Collections::Generic::List<double>^ corners);

            //! Imagery corners in lat/long.
            //! May return NULL. In such a case the corners should be read from the file header. The pointer returned is the 
            //! start of a 4 GeoPoint2d array.
            System::Collections::Generic::List<double>^ GetCornersCP();

            //! Set Imagery corners. nullptr can be passed to remove corners. If corners are provided then
            //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array
            void SetCorners(System::Collections::Generic::List<double>^ pCorners);

            static Utf8CP ElementName;

        private:
            ImageryDataNet(SpatialEntityDataSourceNet^ dataSource, System::Collections::Generic::List<double>^ corners);
            ~ImageryDataNet();
            !ImageryDataNet();

            RealityPlatform::PackageRealityDataPtr* m_pImageryData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class ModelDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array
            static ModelDataNet^ Create(SpatialEntityDataSourceNet^ dataSource);

            static Utf8CP ElementName;

        private:
            ModelDataNet(SpatialEntityDataSourceNet^ dataSource);
            ~ModelDataNet();
            !ModelDataNet();

            RealityPlatform::PackageRealityDataPtr* m_pModelData;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class PinnedDataNet : public RealityDataNet
        {
        public:
            //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
            //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array
            static PinnedDataNet^ Create(SpatialEntityDataSourceNet^ dataSource, double longitude, double latitude);

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
            PinnedDataNet(SpatialEntityDataSourceNet^ dataSource, double longitude, double latitude);
            ~PinnedDataNet();
            !PinnedDataNet();

            RealityPlatform::PackageRealityDataPtr* m_pPinnedData;
        };    

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              9/2016
    //=====================================================================================
    public ref class TerrainDataNet : public RealityDataNet
        {
        public:
            //! Create a new TerrainData. 
            static TerrainDataNet^ Create(SpatialEntityDataSourceNet^ dataSource);

            static Utf8CP ElementName;

        private:
            TerrainDataNet(SpatialEntityDataSourceNet^ dataSource);
            ~TerrainDataNet();
            !TerrainDataNet();

            RealityPlatform::PackageRealityDataPtr* m_pTerrainData;
        };

    //=====================================================================================
    //! @bsiclass                                   Alain.Robert              9/2016
    //=====================================================================================
    public ref class UndefinedDataNet : public RealityDataNet
        {
        public:
            //! Create a new UndefinedData. 
            static UndefinedDataNet^ Create(SpatialEntityDataSourceNet^ dataSource);

            static Utf8CP ElementName;

        private:
            UndefinedDataNet(SpatialEntityDataSourceNet^ dataSource);
            ~UndefinedDataNet();
            !UndefinedDataNet();

            RealityPlatform::PackageRealityDataPtr* m_pUndefinedData;
        };
    }
