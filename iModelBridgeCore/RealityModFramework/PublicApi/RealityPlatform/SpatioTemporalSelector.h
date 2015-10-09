/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatioTemporalSelector.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <Bentley/DateTime.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct SpatioTemporalData;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
enum class SelectionCriteria
    {
    Date,                   //!< Take latest dataset first.
    Resolution,             //!< Take dataset with highest resolution first.
    DateAndResolution,      //!< Take dataset with highest resolution first and then complete with the latest ones.
    // *** Add new here.
    Default,                //!< DateAndResolution. Date: 5 years. Resolution: High-res.
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               9/2015
//=====================================================================================
struct SpatioTemporalSelector
    {
public:
    //!
    REALITYDATAPLATFORM_EXPORT static const bvector<Utf8String> GetIDsFromJson(const bvector<GeoPoint2d>& regionOfInterest,
                                                                               Utf8CP data, 
                                                                               SelectionCriteria criteria = SelectionCriteria::Default);
                                                                      
private:
    //! Select and return the data IDs that best fit the region of interest (footprint) and the criteria (resolutions and dates).
    static const bvector<Utf8String> Select(const bvector<GeoPoint2d>& regionOfInterest,
                                            const bvector<SpatioTemporalDataPtr>& dataset,
                                            SelectionCriteria criteria);

    static const bvector<Utf8String> SelectByDate(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    static const bvector<Utf8String> SelectByResolution(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    static const bvector<Utf8String> SelectByDateAndResolution(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatioTemporalDataset : public RefCountedBase
    {
public:
    //! Create from Json.
    static SpatioTemporalDatasetPtr CreateFromJson(Utf8CP data);

    //! Get dataset.
    const bvector<SpatioTemporalDataPtr>& GetImageryGroup() { return m_imageryGroup; }
    const bvector<SpatioTemporalDataPtr>& GetTerrainGroup() { return m_terrainGroup; }

private:
    SpatioTemporalDataset();
    ~SpatioTemporalDataset();

    bvector<SpatioTemporalDataPtr> m_imageryGroup;
    bvector<SpatioTemporalDataPtr> m_terrainGroup;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatioTemporalData : public RefCountedBase
    {
public:
    //! Create from Json.
    static SpatioTemporalDataPtr Create(Utf8CP identifier, const DateTime& date, Utf8CP resolution, DRange2dCR footprint);

    //! Get identifier.
    Utf8StringCR GetIdentifier() { return m_identifier; }

    //! Get date.
    const DateTime& GetDate() { return m_date; }

    //! Get resolution.
    Utf8StringCR GetResolution() { return m_resolution; }

    //! Get footprint.
    DRange2dCR GetFootprint() { return m_footprint; }

private:
    SpatioTemporalData(Utf8CP identifier, const DateTime& date, Utf8CP resolution, DRange2dCR footprint);
    //&&JFC error C2248 ? ~SpatioTemporalData();

    Utf8String m_identifier;
    DateTime m_date;
    Utf8String m_resolution; 
    DRange2d m_footprint;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE