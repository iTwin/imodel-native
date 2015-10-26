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
    Resolution_Good,        //!< Good level of quality.
    Resolution_Better,      //!< Better level of quality.
    Resolution_Best,        //!< Best level of quality.
    Date_Less,              //!< Oldest capture date.
    Date_Recent,            //!< Recent capture date.
    Date_Most,              //!< Latest capture date.
    // *** Add new here.
    Default,                //!< Best level of quality and latest capture date.
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
                                                                               SelectionCriteria qualityCriteria = SelectionCriteria::Default,
                                                                               SelectionCriteria captureDateCriteria = SelectionCriteria::Default);
                                                                      
private:
    //! Select and return the data IDs that best fit the region of interest.
    //! High resolution and latest capture date first.
    static const bvector<Utf8String> GetIDs(const bvector<GeoPoint2d>& regionOfInterest,
                                            const bvector<SpatioTemporalDataPtr>& dataset);

    //! Select and return the data IDs that best fit the region of interest 
    //! and based on quality (resolution) and capture date.
    static const bvector<Utf8String> GetIDsByCriteria(const bvector<GeoPoint2d>& regionOfInterest,
                                                      const bvector<SpatioTemporalDataPtr>& dataset,
                                                      SelectionCriteria qualityCriteria,
                                                      SelectionCriteria captureDateCriteria);


    static const bvector<SpatioTemporalDataPtr> PositionFiltering(const bvector<GeoPoint2d>& regionOfInterest,
                                                                  const bvector<SpatioTemporalDataPtr>& dataset);

    static const bvector<SpatioTemporalDataPtr> CriteriaFiltering(const bvector<GeoPoint2d>& regionOfInterest,
                                                                  const bvector<SpatioTemporalDataPtr>& dataset,
                                                                  SelectionCriteria qualityCriteria,
                                                                  SelectionCriteria captureDateCriteria);

    static const bvector<SpatioTemporalDataPtr> Select(const bvector<GeoPoint2d>& regionOfInterest,
                                                       const bvector<SpatioTemporalDataPtr>& dataset);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE