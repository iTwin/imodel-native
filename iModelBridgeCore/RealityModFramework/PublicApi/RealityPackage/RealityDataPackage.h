/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityDataPackage.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPackage/RealityPlatformAPI.h>
#include <RealityPackage/RealityDataSource.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <Bentley/DateTime.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct RealityDataSerializer;

//=====================================================================================
//!
//! @bsiclass                                     Mathieu.Marchand              03/2015
//=====================================================================================
struct BoundingPolygon : public RefCountedBase
{
public:    
    //! Empty polygon
    static BoundingPolygonPtr Create();

    //! Create a polygon from points. Duplication will be forced on first/last point. 
    //! Return NULL if count is less than 3, if points value are not within long/lat range or
    //! if the points form an invalid polygon that is autoconguous or autcrosses.
    REALITYDATAPLATFORM_EXPORT static BoundingPolygonPtr Create(GeoPoint2dCP pPoints, size_t count);

    //! The polygon points in lat/long including the duplicated closing point.
    REALITYDATAPLATFORM_EXPORT GeoPoint2dCP GetPointCP() const;

    //! The number of points including duplicated closing point.
    REALITYDATAPLATFORM_EXPORT size_t GetPointCount() const;

    REALITYDATAPLATFORM_EXPORT bool IsValid() const; 
    
    REALITYDATAPLATFORM_EXPORT WString ToString() const;

    //! Return nullptr if a parsing error occurs
    static BoundingPolygonPtr FromString(Utf8StringCR);

private:
    BoundingPolygon(){};
    BoundingPolygon(bvector<GeoPoint2d>& pts):m_points(std::move(pts)){};
    BoundingPolygon(GeoPoint2dCP pPoints, size_t count);
    ~BoundingPolygon();

    bvector<GeoPoint2d> m_points;
};


//=======================================================================================
//! A RealityDataPackage holds a list of reality data source. Each source is categorized and 
//! additional meta-data can be attached.  
//! 
//! File version(Major.Minor). 
//! Major version upgrade should occurs when the change would be incompatible with the
//! current implementation. In these case a new XSD schema and namespace are required.
//! Minor version upgrade must remain foreword and backward compatible.
//! 
//! @bsiclass
//=======================================================================================
struct RealityDataPackage : public RefCountedBase
{
public:
    typedef bvector<ImageryDataPtr>   ImageryGroup;
    typedef bvector<ModelDataPtr>     ModelGroup;
    typedef bvector<PinnedDataPtr>    PinnedGroup;
    typedef bvector<TerrainDataPtr>   TerrainGroup;
    typedef bvector<UndefinedDataPtr> UndefinedGroup;
    
    //! Create a new empty package.
    REALITYDATAPLATFORM_EXPORT static RealityDataPackagePtr Create(Utf8CP name);

    //! Create an instance from an existing package file.
    //! @param[out] status      Success if the package was opened and parsed successfully.
    //! @param[in]  fileName    The name of the package file.
    //! @param[out] pParseError Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @return NULL if the file cannot be found or successfully parsed.
    REALITYDATAPLATFORM_EXPORT static RealityDataPackagePtr CreateFromFile(RealityPackageStatus& status, BeFileNameCR filename, WStringP pParseError = NULL);

    //! Store the content of this instance to disk. 
    //! If at the time of writing the creation date is invalid a valid one will be created with the current date.
    REALITYDATAPLATFORM_EXPORT RealityPackageStatus Write(BeFileNameCR filename);

    //! The origin of this package file. Specified the Context service or GeoCoordinate Service server.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetOrigin() const;
    REALITYDATAPLATFORM_EXPORT void SetOrigin(Utf8CP origin);

    //! The requesting application of this package file. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetRequestingApplication() const;
    REALITYDATAPLATFORM_EXPORT void SetRequestingApplication(Utf8CP requestingApplication);

    //! The name of this package file.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;
    REALITYDATAPLATFORM_EXPORT void SetName(Utf8CP name);

    //! Package description. Might be empty.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDescription() const;
    REALITYDATAPLATFORM_EXPORT void SetDescription(Utf8CP description);

    //! Copyright information, Might be empty.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetCopyright() const;
    REALITYDATAPLATFORM_EXPORT void SetCopyright(Utf8CP copyright);

    //! Package tracking Id. Might be empty.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetId() const;
    REALITYDATAPLATFORM_EXPORT void SetId(Utf8CP id);

    //! Package creation date. DateTime object might be invalid during creation. 
    REALITYDATAPLATFORM_EXPORT DateTimeCR GetCreationDate() const;
    REALITYDATAPLATFORM_EXPORT void SetCreationDate(DateTimeCR date);

    //! The bounding polygon, in latitude/longitude, defines the location of the package, the region of interest. 
    //! Note that individual sources of data may not be fully contained in the package bounding polygon.
    REALITYDATAPLATFORM_EXPORT BoundingPolygonCR GetBoundingPolygon() const;    
    REALITYDATAPLATFORM_EXPORT void SetBoundingPolygon(BoundingPolygonR polygon);    // Package object will increment ref count of 'polygon'.

    //! A read-only access to data source container. Might be empty.
    REALITYDATAPLATFORM_EXPORT ImageryGroup const& GetImageryGroup() const;
    REALITYDATAPLATFORM_EXPORT ModelGroup const& GetModelGroup() const;
    REALITYDATAPLATFORM_EXPORT PinnedGroup const& GetPinnedGroup() const;
    REALITYDATAPLATFORM_EXPORT TerrainGroup const& GetTerrainGroup() const;
    REALITYDATAPLATFORM_EXPORT UndefinedGroup const& GetUndefinedGroup() const;

    //! A read-write access to the data source container.
    REALITYDATAPLATFORM_EXPORT ImageryGroup& GetImageryGroupR();    
    REALITYDATAPLATFORM_EXPORT ModelGroup& GetModelGroupR();    
    REALITYDATAPLATFORM_EXPORT PinnedGroup& GetPinnedGroupR();    
    REALITYDATAPLATFORM_EXPORT TerrainGroup& GetTerrainGroupR();    
    REALITYDATAPLATFORM_EXPORT UndefinedGroup& GetUndefinedGroupR();    

    //! Return true if during parsing unknown element(s) were found. That may indicate that new elements were added in future version of the package
    //! and these elements were ignored. Package is valid but only known elements were loaded.
    REALITYDATAPLATFORM_EXPORT bool HasUnknownElements() const;
    REALITYDATAPLATFORM_EXPORT void SetUnknownElements(bool hasUnknownElements);


    //! Get package version.
    REALITYDATAPLATFORM_EXPORT uint32_t GetMajorVersion() const;
    REALITYDATAPLATFORM_EXPORT void SetMajorVersion(uint32_t major);
    REALITYDATAPLATFORM_EXPORT uint32_t GetMinorVersion() const;
    REALITYDATAPLATFORM_EXPORT void SetMinorVersion(uint32_t minor);

    REALITYDATAPLATFORM_EXPORT static RealityDataPackagePtr CreateFromString(RealityPackageStatus& status, Utf8CP pSource, WStringP pParseError = NULL);
    
private:
    RealityDataPackage() = delete;
    RealityDataPackage(Utf8CP name);
    ~RealityDataPackage();

    static RealityDataPackagePtr CreateFromDom(RealityPackageStatus& status, BeXmlDomR xmlDom, Utf8CP defaultName, WStringP pParseError);

    Utf8String BuildCreationDateUTC();   // May update m_creationDate.


    uint32_t m_majorVersion;
    uint32_t m_minorVersion;
    Utf8String m_origin;
    Utf8String m_requestingApplication;
    Utf8String m_name;
    Utf8String m_description;
    Utf8String m_copyright;

    Utf8String m_id;
    DateTime m_creationDate;
    BoundingPolygonPtr m_pBoundingPolygon;
    bool m_hasUnknownElements;
    
    ImageryGroup m_imagery;
    ModelGroup m_model;
    PinnedGroup m_pinned;
    TerrainGroup m_terrain;
    UndefinedGroup m_undefined;
};

//=======================================================================================
//! Abstract class for classified reality data.
//! @bsiclass
//=======================================================================================
struct PackageRealityData : public RefCountedBase
    {
    public:      
        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataId() const;
        REALITYDATAPLATFORM_EXPORT void SetDataId(Utf8CP dataId);

        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataName() const;
        REALITYDATAPLATFORM_EXPORT void SetDataName(Utf8CP dataName);

        REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataset() const;
        REALITYDATAPLATFORM_EXPORT void SetDataset(Utf8CP dataset);

        //! Data can contain many sources (at least one). This returns the number of sources.
        REALITYDATAPLATFORM_EXPORT size_t GetNumSources() const;

        //! Returns the source. index start at 0 up to GetNumSource()-1
        REALITYDATAPLATFORM_EXPORT RealityDataSourceR GetSourceR(size_t index);
        REALITYDATAPLATFORM_EXPORT RealityDataSourceCR GetSource(size_t index) const;

        //! Adds an alternate source to the data.
        REALITYDATAPLATFORM_EXPORT void AddSource(RealityDataSourceR dataSource);

    protected:
        explicit PackageRealityData(){}; // for persistence.
        PackageRealityData(RealityDataSourceR dataSource);
        virtual ~PackageRealityData();
      
    private:
        bvector<RealityDataSourcePtr> m_Sources;
        Utf8String m_id;
        Utf8String m_name;
        Utf8String m_dataset;
    };

//=====================================================================================
//! Imagery data type specify the data type for imagery data. It extends the generic 
//! data type by adding the optional footprint of the imagery.
//! @bsiclass                                     Mathieu.Marchand               3/2015
//=====================================================================================
struct ImageryData: public PackageRealityData
    {
    DEFINE_T_SUPER(PackageRealityData)
    
    public:
        friend RealityDataSerializer;

        //! Indicate the footprint extent of the Imagery. All four corners are specified allowing
        //! specification of a rotated square footprint.
        enum Corners
            {
            LowerLeft   = 0,    // (0,0)
            LowerRight  = 1,    // (1,0)
            UpperLeft   = 2,    // (0,1)
            UpperRight  = 3     // (1,1)
            };

        //! Create a new ImageryData. Optionally imagery corners in lat/long. If corners are provided then
        //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array.
        REALITYDATAPLATFORM_EXPORT static ImageryDataPtr Create(RealityDataSourceR dataSource, GeoPoint2dCP pCorners);
    
        //! Imagery corners in lat/long.
        //! May return NULL. In such case the corners should be read from the file header. The pointer returned is the 
        //! start of a 4 GeoPoint2d array.
        REALITYDATAPLATFORM_EXPORT GeoPoint2dCP GetCornersCP() const;

        //! Set Imagery corners. nullptr can be passed to remove corners. If corners are provided then
        //! the pointer given must point to 4 consecutive GeoPoint2d structures in an array.
        REALITYDATAPLATFORM_EXPORT void SetCorners(GeoPoint2dCP pCorners);
       
        static Utf8CP ElementName; 

    private:
        explicit ImageryData(){InvalidateCorners();}; // for persistence.
        ImageryData(RealityDataSourceR dataSource, GeoPoint2dCP pCorners);
        virtual ~ImageryData();

        static bool HasValidCorners(GeoPoint2dCP pCorners);
        void InvalidateCorners() {memset(m_corners, 0, sizeof(m_corners));}

        GeoPoint2d m_corners[4];
    };

//=====================================================================================
//! Model data type specify the data type for model data.
//! @bsiclass                                     Mathieu.Marchand               3/2015
//=====================================================================================
struct ModelData: public PackageRealityData
    {
    DEFINE_T_SUPER(PackageRealityData)

    public:
        friend RealityDataSerializer;

        //! Create a new ModelData.
        REALITYDATAPLATFORM_EXPORT static ModelDataPtr Create(RealityDataSourceR dataSource);

        //&&MM add table mapping.

        static Utf8CP ElementName;

    private:
        explicit ModelData(){}; // for persistence
        ModelData(RealityDataSourceR dataSource);
        virtual ~ModelData();
    };


//=====================================================================================
//! Pinned data type specify the data type for pinned data. It extends the generic data 
//! type by adding a mandatory pin position applicable to the non-geospatially located data.
//! @bsiclass                                     Mathieu.Marchand               3/2015
//=====================================================================================
struct PinnedData : public PackageRealityData
    {
    DEFINE_T_SUPER(PackageRealityData)

    public:
        friend RealityDataSerializer;

        //! Create a new PinnedData with required position.
        REALITYDATAPLATFORM_EXPORT static PinnedDataPtr Create(RealityDataSourceR dataSource, double longitude, double latitude);

        //! Get the object location in long/lat coordinate. 
        REALITYDATAPLATFORM_EXPORT GeoPoint2dCR GetLocation() const; 
 
        //! Set the position of the pin. It is expressed as pair of numbers longitude/latitude.
        //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
        //! False is returned if location is invalid.
        REALITYDATAPLATFORM_EXPORT bool SetLocation(GeoPoint2dCR location);

        //! Returns true if the location has a defined area (a polygon).
        REALITYDATAPLATFORM_EXPORT bool HasArea() const;

        //! Get the object polygon location in long/lat coordinate. If the location is not defined by a polygon 
        //! a nullptr is returned. First check with HasArea() before calling this method.
        REALITYDATAPLATFORM_EXPORT BoundingPolygonCP GetAreaCP() const;

        //! Set the object location as a polygon in a sequence long/lat coordinate. 
        //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
        //! The polygon can be convex or not but it must not be auto contiguous or auto crossing.
        //! False is returned if polygon is invalid.
        REALITYDATAPLATFORM_EXPORT bool SetArea(BoundingPolygonR polygon); 

        static Utf8CP ElementName;

    private:
        explicit PinnedData(){m_location.Init(0.0, 0.0);}; // for persistence
        PinnedData(RealityDataSourceR dataSource, double longitude, double latitude);
        virtual ~PinnedData();

        GeoPoint2d            m_location;
        BoundingPolygonPtr  m_area;
    };

//=====================================================================================
//! Terrain data type specify the data type for terrain data.
//! @bsiclass                                     Mathieu.Marchand               3/2015
//=====================================================================================
struct TerrainData: public PackageRealityData
    {
    DEFINE_T_SUPER(PackageRealityData)

    public:
        friend RealityDataSerializer;

        //! Create a new TerrainData.
        REALITYDATAPLATFORM_EXPORT static TerrainDataPtr Create(RealityDataSourceR dataSource);

        static Utf8CP ElementName;

    private:
        explicit TerrainData(){}; // for persistence
        TerrainData(RealityDataSourceR dataSource);
        virtual ~TerrainData();
    };

//=====================================================================================
//! Undefined data type specify the data type for undefined data.
//! @bsiclass                                     Alain.Robert               4/2017
//=====================================================================================
struct UndefinedData: public PackageRealityData
    {
    DEFINE_T_SUPER(PackageRealityData)

    public:
        friend RealityDataSerializer;

        //! Create a new UndefinedData.
        REALITYDATAPLATFORM_EXPORT static UndefinedDataPtr Create(RealityDataSourceR dataSource);

        static Utf8CP ElementName;

    private:
        explicit UndefinedData(){}; // for persistence
        UndefinedData(RealityDataSourceR dataSource);
        virtual ~UndefinedData();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE
