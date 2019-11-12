/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntity.h>
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
                                                                               ResolutionCriteria imageryQualityCriteria,
                                                                               DateCriteria imageryDateCriteria,
                                                                               ResolutionCriteria terrainQualityCriteria = ResolutionCriteria::High,
                                                                               DateCriteria terrainDateCriteria = DateCriteria::UpToDate);

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

    //! Get the best matching IDs for every resolution for a selected region from a user-defined dataset.
    //! High res mosaic: High res first, but can be completed by medium and low res if there is any hole left by the high res.
    //! Medium res mosaic: Medium res first, but can be completed by low res if there is any hole left by the medium res.
    //! Low res mosaic: Low res only, a hole may exists if there is no suitable data.
    //!
    //! hasNoDataValue:
    //! Temporary fix for filtering with data that contains no data value. This will prioritize the data that are the nearest 
    //! to the region of interest so that we have a better chance of obtaining a better coverage.
    //! &&JFC TODO: Remove this parameter when we will have a better footprint extraction.
    REALITYDATAPLATFORM_EXPORT static const ResolutionMap GetIDsByRes(SpatialEntityDatasetCR dataset,
                                                                      const bvector<GeoPoint2d>& regionOfInterest,
                                                                      bool hasNoDataValue = false);

private:
    //! Take a list of GeoPoints and create a shape from it.
    static ImagePP::HFCPtr<ImagePP::HGF2DShape> CreateShapeFromGeoPoints(const bvector<GeoPoint2d>& regionOfInterest);

    //! Filter and select the data IDs that best fit the region of interest 
    //! and based on resolution and capture date.
    //!
    //! hasNoDataValue:
    //! Temporary fix for filtering with data that contains no data value. This will prioritize the data that are the nearest 
    //! to the region of interest so that we have a better chance of obtaining a better coverage.
    //! &&JFC TODO: Remove this parameter when we will have a better footprint extraction.
    static const bvector<Utf8String> GetIDs(const bvector<SpatialEntityPtr>& dataset,
                                            const ImagePP::HGF2DShape& regionOfInterest,
                                            ResolutionCriteria qualityCriteria,
                                            DateCriteria captureDateCriteria,
                                            bool hasNoDataValue = false);

    static const bvector<Utf8String> GetIDs(const bvector<SpatialEntityPtr>& dataset,
                                            const ImagePP::HGF2DShape& regionOfInterest,
                                            const double minResolution,
                                            const double maxResolution,
                                            const DateTime& minDate,
                                            const DateTime& maxDate);

    //! Keep only the data included in a region of interest.
    static const bvector<SpatialEntityPtr> PositionFiltering(const bvector<SpatialEntityPtr>& dataset,
                                                                  const ImagePP::HGF2DShape& regionOfInterest);


    //! Filter according to specified criteria. Resolution is a priority over capture date.
    static const bvector<SpatialEntityPtr> CriteriaFiltering(const bvector<SpatialEntityPtr>& dataset,
                                                                  const ImagePP::HGF2DShape& regionOfInterest,
                                                                  ResolutionCriteria qualityCriteria,
                                                                  DateCriteria captureDateCriteria,
                                                                  bool hasNoDataValue);

    static const bvector<SpatialEntityPtr> CriteriaFiltering(const bvector<SpatialEntityPtr>& dataset,
                                                                  const double minResolution,
                                                                  const double maxResolution,
                                                                  const DateTime& minDate,
                                                                  const DateTime& maxDate);

    //! Filter by resolution (low, medium, high).
    static const StatusInt ResolutionFiltering(bvector<SpatialEntityPtr>& lowResDataset,
                                               bvector<SpatialEntityPtr>& mediumResDataset,
                                               bvector<SpatialEntityPtr>& highResDataset,
                                               const bvector<SpatialEntityPtr>& dataset);

    //! Filter by date (old, recent, up-to-date).
    static const StatusInt DateFiltering(bvector<SpatialEntityPtr>& oldDataset,
                                         bvector<SpatialEntityPtr>& recentDataset,
                                         bvector<SpatialEntityPtr>& upToDateDataset,
                                         const bvector<SpatialEntityPtr>& dataset);

    //! Filter by distance (nearest to region of interest first).
    static const StatusInt DistanceFiltering(bvector<SpatialEntityPtr>& dataset,
                                             const ImagePP::HGF2DShape& regionOfInterest);

    //! Fill the region of interest with best matching data (create a mosaic).
    //!
    //! hasNoDataValue:
    //! Temporary fix for filtering with data that contains no data value. This will prioritize the data that are the nearest 
    //! to the region of interest so that we have a better chance of obtaining a better coverage.
    //! &&JFC TODO: Remove this parameter when we will have a better footprint extraction.
    static const bvector<SpatialEntityPtr> Select(const bvector<SpatialEntityPtr>& dataset,
                                                       const ImagePP::HGF2DShape& regionOfInterest,
                                                       bool hasNoDataValue = false);
    };


#if 0
//=====================================================================================
//! @bsiclass                                   Alain.Robert               12/2016
//! This class provides functionality to filter a list of SpatialEntity objects.
//! Various filtering is possible depending on the method called.
//! The 
//=====================================================================================
struct SpatialEntityFilter
    {
public:
    //! Returns a list of SpatialEntity object that are of the specified classification.
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByClassification(bvector<SpatialEntityPtr> const & listToFilter, RealityDataBase::Classification classification);

    //! Returns a list of SpatialEntity object that are of the specified time range
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    //! Spatial entities that do not have a date set are not part of the result
    //! If no classification is provided (value zero) is given then only spatial entities
    //! with no classification will be returned
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByDate(bvector<SpatialEntityPtr> const & listToFilter, DateTimeCR minDate, DateTimeCR maxDate);

    //! Returns a list of SpatialEntity object that are of the specified resolution range
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    //! Spatial entities that do not have a resolution set
    //! If the filtering resolution is negative then only spatial entities that do not have 
    //! resolution set will be returned
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByResolution(bvector<SpatialEntityPtr> const & listToFilter, double minResolution, double maxResolution);

    //! Returns a list of SpatialEntity object that are of the specified occlusion or less
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    //! Spatial entities that do not have a occlusion set are assumed to have no occlusion and will be automatically returned
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByOcclusion(bvector<SpatialEntityPtr> const & listToFilter, double maximumOcclusion);

    //! Returns a list of SpatialEntity object that are of the specified accuracy range
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    //! Spatial entities that do not have a resolution set
    //! If the filtering accuracy is negative then only spatial entities that do not have 
    //! accuracy set will be returned
    //! NOTE: Normally the smaller the accuracy number to better the accuracy.
    //!       since this may be confusion, we simply select the spatial entities that have
    //!       a value between the two provided bounds.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByAccuracy(bvector<SpatialEntityPtr> const & listToFilter, double accuracyBound1, double accuracyBound2);

    //! Returns a list of SpatialEntity object that are of the specified provider.
    //! The list returned contains pointers to result of filtering.
    //! Spatial entities are not copied.
    //! Note that match is case sensitive
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByProvider(bvector<SpatialEntityPtr> const & listToFilter, Utf8String provider);

    //! Returns a list of SpatialEntity object that are of the specified dataset key
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied
    //! Note that match is case sensitive
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByDataset(bvector<SpatialEntityPtr> const & listToFilter, Utf8String dataset);

    //! Returns a list of SpatialEntity object that make use of data sources of a given file type
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    //! If the data sources are not present (a SpatialEntity object resulting from a SpatialEntityWithDetailsView) then
    //! The supported file type field is used.
    //! The filter string can contain many data type specifications separated by semi-colons.
    //! For example the following string 'tif;hgt;dem" will retain spatial entities that have at least one
    //! data source of the indicated format.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByDataType(bvector<SpatialEntityPtr> const & listToFilter, Utf8String datatypes);

    //! Returns a list of SpatialEntity object that overlap the region of interest
    //! The list returned contains pointers to result of filtering
    //! Spatial entities are not copied.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> FilterByDataType(bvector<SpatialEntityPtr> const & listToFilter, bvector<GeoPoint2d> regionOfInterest);

    //! Returns a list of SpatialEntity object that is the addition of both lists os spatial entities
    //! duplicates a retained once only from the result set. (The original lists are unmodified even if duplicates exists)
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> Add(bvector<SpatialEntityPtr> const & list1, bvector<SpatialEntityPtr> const &  list2);

    //! Returns a list of SpatialEntity object that is the substraction from first list of all spatial entities contained in the second list.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityPtr> Subtract(bvector<SpatialEntityPtr> const & listToRemoveFrom, bvector<SpatialEntityPtr> const & listToRemove);

    };

//=====================================================================================
//! @bsiclass                                   Alain.Robert               12/2016
//! This class provides functionality to filter a list of SpatialEntityDataSource objects.
//! Various filtering is possible depending on the method called.
//! The intent of this class is mainly to enable filtering out from a list of data sources
//=====================================================================================
struct SpatialEntityDataSourceFilter
    {
public:

    //! Returns a list of SpatialEntityDataSource object that make use of data sources of a given file type
    //! The list returned contains pointers to result of filtering
    //! Spatial data sources are not copied.
    //! The filter string can contain many data type specifications separated by semi-colons.
    //! For example the following string 'tif;hgt;dem" will retain spatial entities that have at least one
    //! data source of the indicated format.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityDataSourcePtr> FilterByDataType(bvector<SpatialEntityDataSourcePtr> const & listToFilter, Utf8String datatypes);

    //! Returns a list of SpatialEntityDataSource object that are streamed or not depending on filtering parameter
    //! The list returned contains pointers to result of filtering
    //! Spatial entity data sources are not copied.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityDataSourcePtr> FilterStreamed(bvector<SpatialEntityDataSourcePtr> const & listToFilter, bool keepStreamedOnly);


    //! Returns a list of SpatialEntityDataSource object that are from public access without authentication
    //! The list returned contains pointers to result of filtering
    //! Spatial entity data sources are not copied.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityDataSourcePtr> FilterServerRequiresAuthentication(bvector<SpatialEntityDataSourcePtr> const & listToFilter);

    //! Returns a list of SpatialEntityDataSource object that are from servers using the authentication method and type specified
    //! The list returned contains pointers to result of filtering
    //! Spatial entity data sources are not copied.
    REALITYDATAPLATFORM_EXPORT static const bvector<SpatialEntityDataSourcePtr> FilterByServerLoginKey(bvector<SpatialEntityDataSourcePtr> const & listToFilter, Utf8String loginKey);

    };

    #endif
END_BENTLEY_REALITYPLATFORM_NAMESPACE