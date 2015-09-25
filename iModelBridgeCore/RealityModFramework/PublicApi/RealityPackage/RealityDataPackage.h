/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityDataPackage.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPackage/RealityPackage.h>
#include <RealityPackage/RealityDataSource.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <Bentley/DateTime.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

struct RealityDataSerializer;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct BoundingPolygon : public RefCountedBase
{
public:    
    //! Empty polygon
    static BoundingPolygonPtr Create();

    //! Create a polygon from points. Duplication will be forced on first/last point. 
    //! Return NULL if count is less than 3 or if points value are not within long/lat range.
    REALITYPACKAGE_EXPORT static BoundingPolygonPtr Create(DPoint2dCP pPoints, size_t count);

    //! The polygon points in lat/long including the duplicated closing point.
    REALITYPACKAGE_EXPORT DPoint2dCP GetPointCP() const;

    //! The number of points including duplicated closing point.
    REALITYPACKAGE_EXPORT size_t GetPointCount() const;

    REALITYPACKAGE_EXPORT bool IsValid() const; 
    
    REALITYPACKAGE_EXPORT WString ToString() const;

    //! Return NULL is a parsing error occurs
    static BoundingPolygonPtr FromString(WStringCR);
private:
    BoundingPolygon(){};
    BoundingPolygon(bvector<DPoint2d>& pts):m_points(std::move(pts)){};
    BoundingPolygon(DPoint2dCP pPoints, size_t count);
    ~BoundingPolygon();

    bvector<DPoint2d> m_points;
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
    
    //! Create a new empty package.
    REALITYPACKAGE_EXPORT static RealityDataPackagePtr Create(WCharCP name);

    //! Create an instance from an existing package file.
    //! @param[out] status      Success if the package was opened and parsed successfully.
    //! @param[in]  fileName    The name of the package file.
    //! @param[out] pParseError Optional. If supplied, and a parsing error occurs, diagnostic information.
    //! @return NULL if the file cannot be found or successfully parsed.
    REALITYPACKAGE_EXPORT static RealityDataPackagePtr CreateFromFile(RealityPackageStatus& status, BeFileNameCR filename, WStringP pParseError = NULL);

    //! Store the content of this instance to disk. 
    //! If at the time of writing the creation date is invalid a valid one will be created with the current date.
    REALITYPACKAGE_EXPORT RealityPackageStatus Write(BeFileNameCR filename);

    //! The name of this package file.
    REALITYPACKAGE_EXPORT WStringCR GetName() const;
    REALITYPACKAGE_EXPORT void SetName(WCharCP name);

    //! Package description. Might be empty.
    REALITYPACKAGE_EXPORT WStringCR GetDescription() const;
    REALITYPACKAGE_EXPORT void SetDescription(WCharCP description);

    //! Copyright information, Might be empty.
    REALITYPACKAGE_EXPORT WStringCR GetCopyright() const;
    REALITYPACKAGE_EXPORT void SetCopyright(WCharCP copyright);

    //! Package tracking Id. Might be empty.
    REALITYPACKAGE_EXPORT WStringCR GetPackageId() const;
    REALITYPACKAGE_EXPORT void SetPackageId(WCharCP packageId);

    //! Package creation date. DateTime object might be invalid during creation. 
    REALITYPACKAGE_EXPORT DateTimeCR GetCreationDate() const;
    REALITYPACKAGE_EXPORT void SetCreationDate(DateTimeCR date);

    //! Package bounding polygon in latitude/longitude.
    //! The bounding polygon for the RealityDataPackage represents the region of interest selected by the user in the RealityModelingNavigator. 
    //! This can be seen as the clipping shape for all the data that the package will contain.
    REALITYPACKAGE_EXPORT BoundingPolygonCR GetBoundingPolygon() const;
    //! Package object will increment ref count of 'polygon'.
    REALITYPACKAGE_EXPORT void SetBoundingPolygon(BoundingPolygonR polygon);

    //! A read-only access to data source container. Might be empty.
    REALITYPACKAGE_EXPORT ImageryGroup const& GetImageryGroup() const;
    REALITYPACKAGE_EXPORT ModelGroup const& GetModelGroup() const;
    REALITYPACKAGE_EXPORT PinnedGroup const& GetPinnedGroup() const;
    REALITYPACKAGE_EXPORT TerrainGroup const& GetTerrainGroup() const;

    //! A read-write access to the data source container.
    REALITYPACKAGE_EXPORT ImageryGroup& GetImageryGroupR();    
    REALITYPACKAGE_EXPORT ModelGroup& GetModelGroupR();    
    REALITYPACKAGE_EXPORT PinnedGroup& GetPinnedGroupR();    
    REALITYPACKAGE_EXPORT TerrainGroup& GetTerrainGroupR();    

    //! Return true if during parsing unknown element(s) were found. That may indicate that new elements were added in future version of the package
    //! and these elements were ignored. Package is valid but only known elements were loaded.
    REALITYPACKAGE_EXPORT bool HasUnknownElements() const;

    //! Get package version.
    REALITYPACKAGE_EXPORT uint32_t GetMajorVersion() const;
    REALITYPACKAGE_EXPORT uint32_t GetMinorVersion() const;

    REALITYPACKAGE_EXPORT static RealityDataPackagePtr CreateFromString(RealityPackageStatus& status, Utf8CP pSource, WStringP pParseError = NULL);
    
private:
    RealityDataPackage() = delete;
    RealityDataPackage(WCharCP name);
    ~RealityDataPackage();

    static RealityDataPackagePtr CreateFromDom(RealityPackageStatus& status, BeXmlDomR xmlDom, WCharCP defaultName, WStringP pParseError);

    WString BuildCreationDateUTC();   // May update m_creationDate.

    template<class Group_T>
    RealityPackageStatus ReadDataGroup_T(Group_T& group, Utf8CP nodePath, BeXmlNodeR parent);

    template<class Group_T>
    RealityPackageStatus WriteDataGroup_T(Group_T const& group, Utf8CP nodeName, BeXmlNodeR parent);

    RealityPackageStatus ReadVersion(BeXmlNodeR node);

    uint32_t m_majorVersion;
    uint32_t m_minorVersion;
    WString m_name;
    WString m_description;
    WString m_copyright;
    WString m_packageId;
    DateTime m_creationDate;
    BoundingPolygonPtr m_pBoundingPolygon;
    bool m_hasUnknownElements;
    
    ImageryGroup m_imagery;
    ModelGroup m_model;
    PinnedGroup m_pinned;
    TerrainGroup m_terrain;
};

//=======================================================================================
//! Abstract class for classified reality data.
//! @bsiclass
//=======================================================================================
struct RealityData : public RefCountedBase
{
public:      
    REALITYPACKAGE_EXPORT RealityDataSourceR GetSourceR();
    REALITYPACKAGE_EXPORT RealityDataSourceCR GetSource() const;

    //! Optional data copyright. Should appear in the package only if known.
    REALITYPACKAGE_EXPORT Utf8StringCR GetCopyright() const;
    REALITYPACKAGE_EXPORT void SetCopyright(Utf8CP dataCopyright);

    //! Optional data size in kilobytes. Should appear in the package only if known. Default to 0.
    REALITYPACKAGE_EXPORT uint64_t GetFilesize() const;
    REALITYPACKAGE_EXPORT void SetFilesize(uint64_t size);
    
protected:
    explicit RealityData(){}; // for persistence.
    RealityData(RealityDataSourceR dataSource);
    virtual ~RealityData();

    // read/write from a dataNode.
    virtual RealityPackageStatus _Read(BeXmlNodeR dataNode);
    virtual RealityPackageStatus _Write(BeXmlNodeR dataNode) const;
      
private:
    RealityDataSourcePtr m_pSource;
    Utf8String m_copyright;
    uint64_t m_size;
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ImageryData: public RealityData
{
    DEFINE_T_SUPER(RealityData)
public:
    friend RealityDataSerializer;

    //! Imagery corners.
    enum Corners
        {
        LowerLeft   = 0,    // (0,0)
        LowerRight  = 1,    // (1,0)
        UpperLeft   = 2,    // (0,1)
        UpperRight  = 3     // (1,1)
        };

    //! Create a new ImageryData. Optionally imagery corners in lat/long.
    REALITYPACKAGE_EXPORT static ImageryDataPtr Create(RealityDataSourceR dataSource, DPoint2dCP pCorners);
    
    //! Imagery corners in lat/long.
    //! May return NULL. In such a case the corners should be read from the file header.
    REALITYPACKAGE_EXPORT DPoint2dCP GetCornersCP() const;

    //! Set Imagery corners.
    REALITYPACKAGE_EXPORT void SetCorners(DPoint2dCP pCorners);
       
    static Utf8CP ElementName; 
private:
    explicit ImageryData(){InvalidateCorners();}; // for persistence.
    ImageryData(RealityDataSourceR dataSource, DPoint2dCP pCorners);
    virtual ~ImageryData();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataNode) override;
    virtual RealityPackageStatus _Write(BeXmlNodeR dataNode) const override;

    static bool HasValidCorners(DPoint2dCP pCorners);
    void InvalidateCorners() {memset(m_corners, 0, sizeof(m_corners));}

    DPoint2d m_corners[4];
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ModelData: public RealityData
{
    DEFINE_T_SUPER(RealityData)
public:
    friend RealityDataSerializer;

    //! Create a new ModelData.
    REALITYPACKAGE_EXPORT static ModelDataPtr Create(RealityDataSourceR dataSource);

    //&&MM add table mapping.

    static Utf8CP ElementName;
private:
    explicit ModelData(){}; // for persistence
    ModelData(RealityDataSourceR dataSource);
    virtual ~ModelData();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataNode) override;
    virtual RealityPackageStatus _Write(BeXmlNodeR dataNode) const override;
};


//=======================================================================================
//!
//! @bsiclass
//=======================================================================================
struct PinnedData : public RealityData
{
    DEFINE_T_SUPER(RealityData)

public:
    friend RealityDataSerializer;

    //! Create a new PinnedData.
    REALITYPACKAGE_EXPORT static PinnedDataPtr Create(RealityDataSourceR dataSource, double longitude, double latitude);

    //! Get the object location in long/lat coordinate. 
    REALITYPACKAGE_EXPORT DPoint2dCR GetLocation() const; 

    //! Set the object location in long/lat coordinate. 
    //! Longitudes range from -180 to 180. Latitudes range from -90 to 90.
    //! False is returned if location is invalid.
    REALITYPACKAGE_EXPORT bool SetLocation(DPoint2dCR location); 

    static Utf8CP ElementName;
private:
    explicit PinnedData(){m_location.Zero();}; // for persistence
    PinnedData(RealityDataSourceR dataSource, double longitude, double latitude);
    virtual ~PinnedData();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataNode) override;
    virtual RealityPackageStatus _Write(BeXmlNodeR dataNode) const override;

    DPoint2d m_location;        //!spatial location in long/lat
};

//=======================================================================================
//!
//! @bsiclass
//=======================================================================================
struct TerrainData: public RealityData
{
    DEFINE_T_SUPER(RealityData)
public:
    friend RealityDataSerializer;

    //! Create a new TerrainData.
    REALITYPACKAGE_EXPORT static TerrainDataPtr Create(RealityDataSourceR dataSource);

    static Utf8CP ElementName;

private:
    explicit TerrainData(){}; // for persistence
    TerrainData(RealityDataSourceR dataSource);
    virtual ~TerrainData();

    virtual RealityPackageStatus _Read(BeXmlNodeR dataNode) override;
    virtual RealityPackageStatus _Write(BeXmlNodeR dataNode) const override;
};

END_BENTLEY_REALITYPACKAGE_NAMESPACE
