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

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

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
    //! Select and return the data IDs that best fit the region of interest (footprint) and the criteria (date, resolution, etc.).
    static const bvector<Utf8String> Select(const bvector<GeoPoint2d>& regionOfInterest,
                                            const bvector<SpatioTemporalDataPtr>& dataset,
                                            SelectionCriteria criteria);

    static const bvector<Utf8String> SelectByDate(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    static const bvector<Utf8String> SelectByResolution(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    static const bvector<Utf8String> SelectByDateAndResolution(const bvector<GeoPoint2d>& regionOfInterest, const bvector<SpatioTemporalDataPtr>& dataset);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE