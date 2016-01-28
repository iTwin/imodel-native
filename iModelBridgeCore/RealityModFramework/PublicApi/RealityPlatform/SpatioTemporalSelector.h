/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatioTemporalSelector.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HGF2DShape.h>


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//=====================================================================================
//! Resolution is a high-priority criteria. It is the first taken into account when filtering.
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
enum class ResolutionCriteria
    {
    Low,        //!< Lowest level of quality.
    Medium,     //!< Medium level of quality.
    High,       //!< Best level of quality.
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
enum class DateCriteria
    {
    Old,        //!< Oldest capture date.
    Recent,     //!< Recent capture date.
    UpToDate,   //!< Latest capture date.
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               9/2015
//=====================================================================================
struct SpatioTemporalSelector
    {
public:
    typedef bmap<ResolutionCriteria, bvector<Utf8String>> ResolutionMap;

    //! Create dataset from a JSON document and get the best matching IDs for a selected region.
    REALITYDATAPLATFORM_EXPORT static const bvector<Utf8String> GetIDsFromJson(Utf8CP data,
                                                                               const bvector<GeoPoint2d>& regionOfInterest,
                                                                               ResolutionCriteria qualityCriteria = ResolutionCriteria::High,
                                                                               DateCriteria captureDateCriteria = DateCriteria::UpToDate);

    //! Create dataset from a JSON document and get the best matching IDs for a selected region.
    REALITYDATAPLATFORM_EXPORT static const bvector<Utf8String> GetIDsFromJson(Utf8CP data, 
                                                                               const bvector<GeoPoint2d>& regionOfInterest,
                                                                               const double minResolution,
                                                                               const double maxResolution,
                                                                               const DateTime& minDate,
                                                                               const DateTime& maxDate);

    //! Create dataset from a JSON document and get the best matching IDs for every resolution for a selected region.
    //! High res mosaic: High res first, but can be completed by medium and low res if there is any hole left by the high res.
    //! Medium res mosaic: Medium res first, but can be completed by low res if there is any hole left by the medium res.
    //! Low res mosaic: Low res only, a hole may exists if there is no suitable data.
    REALITYDATAPLATFORM_EXPORT static const ResolutionMap GetIDsByResFromJson(Utf8CP data,
                                                                              const bvector<GeoPoint2d>& regionOfInterest);

private:
    //! Take a list of GeoPoints and create a shape from it.
    static ImagePP::HFCPtr<ImagePP::HGF2DShape> CreateShapeFromGeoPoints(const bvector<GeoPoint2d>& regionOfInterest);

    //! Filter and select the data IDs that best fit the region of interest 
    //! and based on resolution and capture date.
    static const bvector<Utf8String> GetIDs(const bvector<SpatioTemporalDataPtr>& dataset,
                                            const ImagePP::HGF2DShape& regionOfInterest,
                                            ResolutionCriteria qualityCriteria,
                                            DateCriteria captureDateCriteria);

    static const bvector<Utf8String> GetIDs(const bvector<SpatioTemporalDataPtr>& dataset,
                                            const ImagePP::HGF2DShape& regionOfInterest,
                                            const double minResolution,
                                            const double maxResolution,
                                            const DateTime& minDate,
                                            const DateTime& maxDate);

    //! Keep only the data included in a region of interest.
    static const bvector<SpatioTemporalDataPtr> PositionFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                  const ImagePP::HGF2DShape& regionOfInterest);


    //! Filter according to specified criteria. Resolution is a priority over capture date.
    static const bvector<SpatioTemporalDataPtr> CriteriaFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                  ResolutionCriteria qualityCriteria,
                                                                  DateCriteria captureDateCriteria);

    static const bvector<SpatioTemporalDataPtr> CriteriaFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                  const double minResolution,
                                                                  const double maxResolution,
                                                                  const DateTime& minDate,
                                                                  const DateTime& maxDate);

    //! Filter by resolution (low, medium, high).
    static const StatusInt ResolutionFiltering(bvector<SpatioTemporalDataPtr>& lowResDataset,
                                               bvector<SpatioTemporalDataPtr>& mediumResDataset,
                                               bvector<SpatioTemporalDataPtr>& highResDataset,
                                               const bvector<SpatioTemporalDataPtr>& dataset);

    //! Filter by date (old, recent, up-to-date).
    static const StatusInt DateFiltering(bvector<SpatioTemporalDataPtr>& oldDataset,
                                         bvector<SpatioTemporalDataPtr>& recentDataset,
                                         bvector<SpatioTemporalDataPtr>& upToDateDataset,
                                         const bvector<SpatioTemporalDataPtr>& dataset);

    //! Fill the region of interest with best matching data (create a mosaic).
    static const bvector<SpatioTemporalDataPtr> Select(const bvector<SpatioTemporalDataPtr>& dataset,
                                                       const ImagePP::HGF2DShape& regionOfInterest);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE