/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/RealityDataPackage.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityPackage.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <Bentley/DateTime.h>


BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

//! Status codes for RealityPackage operations
enum class RealityPackageStatus
    {
    Success             = SUCCESS,  //!< The operation was successful
    UnknownError        = ERROR,    //!< The operation failed with an unspecified error
    };

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct BoundingPolygon : public bvector<DPoint2d>
{
public:
    //! An empty invalid BoundingPolygon
    BoundingPolygon(){};

    //! Create a polygon from points. Duplication will be forced on first/last point.
    BoundingPolygon(DPoint2dCP pPoints, size_t count)
        {
        if(count > 2)
            {
            if(pPoints[0].AlmostEqual(pPoints[count-1]))
                {
                assign(pPoints, &pPoints[count-2]);
                }
            else
                {
                assign(pPoints, &pPoints[count-1]);
                }

            push_back(pPoints[0]);  // Explicitly assign the last for bitwise equality.
            }

        BeAssert(IsValid());
        }
    ~BoundingPolygon(){};
            
    DPoint2dCP GetPoints() const {return begin();}

    bool IsValid() const { return GetSize() > 3;}   // Need at least 4 points to have a closed polygon.

    //! The number of points including duplicated closing point.
    size_t GetSize() const {return size();}
};


//=======================================================================================
//!
//! @bsiclass
//=======================================================================================
struct RealityDataPackage : public RefCountedBase
{
public:
    typedef bvector<RealityDataSourcePtr> DataSources;

    //&&MM need to be able :
    //  - create a new one. mandatory field?
    //  - create from existing.
    //! Create a new one.
    REALITYPACKAGE_EXPORT RealityDataPackagePtr Create(WCharCP name/*&&MM define what is mandatory*/);

    //! Create an instance from an existing package file.
    REALITYPACKAGE_EXPORT RealityDataPackagePtr Create(RealityPackageStatus& status, BeFileNameCR filename);

    //! Populate this instance from the provided filename.
    REALITYPACKAGE_EXPORT RealityPackageStatus Read(BeFileNameCR filename);

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
    REALITYPACKAGE_EXPORT BoundingPolygonCR GetBoundingPolygon() const;
    REALITYPACKAGE_EXPORT void SetBoundingPolygon(BoundingPolygonCR polygon);

    //! A read-only access to data source container. Might be empty
    REALITYPACKAGE_EXPORT DataSources const& GetImageryGroup() const;
    REALITYPACKAGE_EXPORT DataSources const& GetTerrainGroup() const;
    REALITYPACKAGE_EXPORT DataSources const& GetModelGroup() const;
    REALITYPACKAGE_EXPORT DataSources const& GetPinnedGroup() const;

    //! A read-write access to the data source container.
    REALITYPACKAGE_EXPORT DataSources& GetImageryGroupR();    
    REALITYPACKAGE_EXPORT DataSources& GetTerrainGroupR();    
    REALITYPACKAGE_EXPORT DataSources& GetModelGroupR();    
    REALITYPACKAGE_EXPORT DataSources& GetPinnedGroupR();    


private:
    RealityDataPackage(WCharCP name);
    ~RealityDataPackage();

    WString m_name;
    WString m_description;
    WString m_copyright;
    WString m_packageId;        //&&MM should it be a GUID or UInt64 instead of a string?
    DateTime m_dateTime;
    BoundingPolygon m_boundingPolygon;
    
    DataSources m_imagery;
    DataSources m_terrain;
    DataSources m_model;
    DataSources m_pinned;
};

//=======================================================================================
//!
//! @bsiclass
//=======================================================================================
struct RealityDataSource : public RefCountedBase
{
public:
    REALITYPACKAGE_EXPORT RealityDataSourcePtr Create(WCharCP uri, WCharCP type);
    
    REALITYPACKAGE_EXPORT WStringCR GetUri() const;
    REALITYPACKAGE_EXPORT void SetUri(WCharCP uri);

    REALITYPACKAGE_EXPORT WStringCR GetType() const;
    REALITYPACKAGE_EXPORT void SetType(WCharCP type);

protected:
    RealityDataSource(WCharCP uri, WCharCP type);
    virtual ~RealityDataSource();

private:
    WString m_uri;
    WString m_type;     
};



END_BENTLEY_REALITYPACKAGE_NAMESPACE