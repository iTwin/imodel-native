/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataPackage.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntity.h>
#include <RealityPlatform/RealityDataObjects.h>
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

    REALITYDATAPLATFORM_EXPORT bvector<GeoPoint2d> GetFootprint() const;

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
//! Abstract class for classified reality data.
//! @bsiclass
//=======================================================================================
struct PackageRealityData : public SpatialEntity
    {
public:
    friend RealityDataSerializer;
    
    enum Corners
        {
        LowerLeft = 0,    // (0,0)
        LowerRight = 1,    // (1,0)
        UpperLeft = 2,    // (0,1)
        UpperRight = 3     // (1,1)
        };

    REALITYDATAPLATFORM_EXPORT static PackageRealityDataPtr CreateImagery(SpatialEntityDataSourceR dataSource, bvector<GeoPoint2d> pCorners);
    REALITYDATAPLATFORM_EXPORT static PackageRealityDataPtr CreateModel(SpatialEntityDataSourceR dataSource);
    REALITYDATAPLATFORM_EXPORT static PackageRealityDataPtr CreatePinned(SpatialEntityDataSourceR dataSource, double longitude, double latitude);
    REALITYDATAPLATFORM_EXPORT static PackageRealityDataPtr CreateTerrain(SpatialEntityDataSourceR dataSource);
    REALITYDATAPLATFORM_EXPORT static PackageRealityDataPtr CreateUndefined(SpatialEntityDataSourceR dataSource);
    
    //! Get the object location in long/lat coordinate. 
    REALITYDATAPLATFORM_EXPORT GeoPoint2dCR GetLocation() const;

    //! Set the position of the pin. It is expressed as pair of numbers longitude/latitude.
    //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
    //! False is returned if location is invalid.
    REALITYDATAPLATFORM_EXPORT bool SetLocation(GeoPoint2dCR location);

    REALITYDATAPLATFORM_EXPORT void SetFootprint(bvector<GeoPoint2d> const& footprint, Utf8String coordSys = "4326") override;
    REALITYDATAPLATFORM_EXPORT Utf8String GetFootprintString() const override;

    //! Returns true if the location has a defined area (a polygon).
    REALITYDATAPLATFORM_EXPORT bool HasArea() const;


protected:
    explicit PackageRealityData(){}; // for persistence.
    PackageRealityData(SpatialEntityDataSourceR dataSource);
    virtual ~PackageRealityData();

    PackageRealityData(SpatialEntityDataSourceR dataSource, bvector<GeoPoint2d> pCorners);
    PackageRealityData(SpatialEntityDataSourceR dataSource, double longitude, double latitude);
      
private:
    static bool HasValidCorners(const bvector<GeoPoint2d>& pCorners);
    GeoPoint2d                      m_location;
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

    //! Context provided when the package was created.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetContext() const;
    REALITYDATAPLATFORM_EXPORT void SetContext(Utf8CP context);



    //! A read-only access to data source container. Might be empty.
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr> const& GetImageryGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr> const& GetModelGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr> const& GetPinnedGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr> const& GetTerrainGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr> const& GetUndefinedGroup() const;

    //! A read-write access to the data source container.
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr>& GetImageryGroupR();
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr>& GetModelGroupR();
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr>& GetPinnedGroupR();
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr>& GetTerrainGroupR();
    REALITYDATAPLATFORM_EXPORT bvector<PackageRealityDataPtr>& GetUndefinedGroupR();

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
    Utf8String m_context;

    Utf8String m_id;
    DateTime m_creationDate;
    BoundingPolygonPtr m_pBoundingPolygon;
    bool m_hasUnknownElements;
    
    bmap<RealityDataBase::Classification, bvector<PackageRealityDataPtr>> m_entities;
    };
END_BENTLEY_REALITYPLATFORM_NAMESPACE
