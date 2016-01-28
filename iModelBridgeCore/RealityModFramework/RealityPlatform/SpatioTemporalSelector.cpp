/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalSelector.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "SpatioTemporalData.h"

#include <RealityPlatform/SpatioTemporalSelector.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDsFromJson(Utf8CP data,
                                                                 const bvector<GeoPoint2d>& regionOfInterest,
                                                                 ResolutionCriteria qualityCriteria,
                                                                 DateCriteria captureDateCriteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // Create shape that represents the region of interest.
    HFCPtr<HGF2DShape> pROIShape = CreateShapeFromGeoPoints(regionOfInterest);
    if (pROIShape->IsEmpty())
        return selectedIDs;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr pDataset = SpatioTemporalDataset::CreateFromJson(data);
    if (NULL == pDataset.get())
        return selectedIDs;

    // Select imagery data.
    bvector<Utf8String> imageryIDs = GetIDs(pDataset->GetImageryGroup(), *pROIShape, qualityCriteria, captureDateCriteria);
    if (!imageryIDs.empty())
        selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

    // Select terrain data.
    bvector<Utf8String> terrainIDs = GetIDs(pDataset->GetTerrainGroup(), *pROIShape, ResolutionCriteria::High, DateCriteria::UpToDate);
    if (!terrainIDs.empty())
        selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    12/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDsFromJson(Utf8CP data, 
                                                                 const bvector<GeoPoint2d>& regionOfInterest,                                                                 
                                                                 const double minResolution,
                                                                 const double maxResolution,
                                                                 const DateTime& minDate,
                                                                 const DateTime& maxDate)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // Validate parameters.
    if (minResolution < 0 || maxResolution < 0 || minResolution > maxResolution || !minDate.IsValid() || !maxDate.IsValid())
        return selectedIDs;

    // Create shape that represents the region of interest.
    HFCPtr<HGF2DShape> pROIShape = CreateShapeFromGeoPoints(regionOfInterest);
    if (pROIShape->IsEmpty())
        return selectedIDs;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr pDataset = SpatioTemporalDataset::CreateFromJson(data);
    if (NULL == pDataset.get())
        return selectedIDs;

    // Select imagery data.
    bvector<Utf8String> imageryIDs = GetIDs(pDataset->GetImageryGroup(), *pROIShape, minResolution, maxResolution, minDate, maxDate);
    if (!imageryIDs.empty())
        selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

    // Select terrain data.
    bvector<Utf8String> terrainIDs = GetIDs(pDataset->GetTerrainGroup(), *pROIShape, minResolution, maxResolution, minDate, maxDate);
    if (!terrainIDs.empty())
        selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    1/2016
//-------------------------------------------------------------------------------------
const SpatioTemporalSelector::ResolutionMap SpatioTemporalSelector::GetIDsByResFromJson(Utf8CP data,
                                                                                        const bvector<GeoPoint2d>& regionOfInterest)
    {
    ResolutionMap selectedIDsByRes = ResolutionMap();

    // Create shape that represents the region of interest.
    HFCPtr<HGF2DShape> pROIShape = CreateShapeFromGeoPoints(regionOfInterest);
    if (pROIShape->IsEmpty())
        return selectedIDsByRes;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr pDataset = SpatioTemporalDataset::CreateFromJson(data);
    if (NULL == pDataset.get())
        return selectedIDsByRes;

    // Select high res data.
    bvector<Utf8String> highResImageryIDs = GetIDs(pDataset->GetImageryGroup(), *pROIShape, ResolutionCriteria::High, DateCriteria::UpToDate);
    bvector<Utf8String> highResTerrainIDs = GetIDs(pDataset->GetTerrainGroup(), *pROIShape, ResolutionCriteria::High, DateCriteria::UpToDate);

    bvector<Utf8String> highResIDs = bvector<Utf8String>();
    if (!highResImageryIDs.empty())
        highResIDs.insert(highResIDs.end(), highResImageryIDs.begin(), highResImageryIDs.end());
    if (!highResTerrainIDs.empty())
        highResIDs.insert(highResIDs.end(), highResTerrainIDs.begin(), highResTerrainIDs.end());

    // Select medium res data.
    bvector<Utf8String> mediumResimageryIDs = GetIDs(pDataset->GetImageryGroup(), *pROIShape, ResolutionCriteria::Medium, DateCriteria::UpToDate);
    bvector<Utf8String> mediumResTerrainIDs = GetIDs(pDataset->GetTerrainGroup(), *pROIShape, ResolutionCriteria::Medium, DateCriteria::UpToDate);

    bvector<Utf8String> mediumResIDs = bvector<Utf8String>();
    if (!mediumResimageryIDs.empty())
        mediumResIDs.insert(mediumResIDs.end(), mediumResimageryIDs.begin(), mediumResimageryIDs.end());
    if (!mediumResTerrainIDs.empty())
        mediumResIDs.insert(mediumResIDs.end(), mediumResTerrainIDs.begin(), mediumResTerrainIDs.end());

    // Select low res data.
    bvector<Utf8String> lowResImageryIDs = GetIDs(pDataset->GetImageryGroup(), *pROIShape, ResolutionCriteria::Low, DateCriteria::UpToDate);
    bvector<Utf8String> lowResTerrainIDs = GetIDs(pDataset->GetTerrainGroup(), *pROIShape, ResolutionCriteria::Low, DateCriteria::UpToDate);

    bvector<Utf8String> lowResIDs = bvector<Utf8String>();
    if (!lowResImageryIDs.empty())
        lowResIDs.insert(highResIDs.end(), lowResImageryIDs.begin(), lowResImageryIDs.end());
    if (!lowResTerrainIDs.empty())
        lowResIDs.insert(highResIDs.end(), lowResTerrainIDs.begin(), lowResTerrainIDs.end());

    // Create resolution map.
    selectedIDsByRes.Insert(ResolutionCriteria::High, highResIDs);
    selectedIDsByRes.Insert(ResolutionCriteria::Medium, mediumResIDs);
    selectedIDsByRes.Insert(ResolutionCriteria::Low, lowResIDs);

    return selectedIDsByRes;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
HFCPtr<HGF2DShape> SpatioTemporalSelector::CreateShapeFromGeoPoints(const bvector<GeoPoint2d>& regionOfInterest)
    {
    HGF2DPositionCollection ptsCollection;
    for (const auto& pt : regionOfInterest)
        {
        ptsCollection.push_back(HGF2DCoord<double>(pt.longitude, pt.latitude));
        }

    return new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDs(const bvector<SpatioTemporalDataPtr>& dataset,
                                                         const HGF2DShape& regionOfInterest,
                                                         ResolutionCriteria qualityCriteria,
                                                         DateCriteria captureDateCriteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // First draft: Spatial position filtering - Included in region of interest.
    bvector<SpatioTemporalDataPtr> firstDraftDataset = PositionFiltering(dataset, regionOfInterest);
    if (firstDraftDataset.empty())
        return selectedIDs;

    // Second draft: Resolution and date filtering - Sort data according to quality and capture date criteria..
    bvector<SpatioTemporalDataPtr> secondDraftDataset = CriteriaFiltering(firstDraftDataset, qualityCriteria, captureDateCriteria);
    if (secondDraftDataset.empty())
        return selectedIDs;

    // Third draft: Data selection - Fill region of interest with best matching data.
    bvector<SpatioTemporalDataPtr> thirdDraftDataset = Select(secondDraftDataset, regionOfInterest);
    if (thirdDraftDataset.empty())
        return selectedIDs;

    // Extract IDs from selected dataset.
    for (const auto& data : thirdDraftDataset)
        {
        selectedIDs.push_back(data->GetIdentifier());
        }

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    12/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDs(const bvector<SpatioTemporalDataPtr>& dataset,
                                                         const ImagePP::HGF2DShape& regionOfInterest,
                                                         const double minResolution,
                                                         const double maxResolution,
                                                         const DateTime& minDate,
                                                         const DateTime& maxDate)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // First draft: Spatial position filtering - Included in region of interest.
    bvector<SpatioTemporalDataPtr> firstDraftDataset = PositionFiltering(dataset, regionOfInterest);

    // Second draft: Resolution and date filtering - Sort data according to resolution and capture date criteria..
    bvector<SpatioTemporalDataPtr> secondDraftDataset = CriteriaFiltering(firstDraftDataset, minResolution, maxResolution, minDate, maxDate);

    // Third draft: Data selection - Fill region of interest with best matching data.
    bvector<SpatioTemporalDataPtr> thirdDraftDataset = Select(secondDraftDataset, regionOfInterest);

    // Extract IDs from selected dataset.
    for (const auto& data : thirdDraftDataset)
        {
        selectedIDs.push_back(data->GetIdentifier());
        }

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::PositionFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                               const HGF2DShape& regionOfInterest)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // Position filtering, keep only the data included in region of interest.
    for (const auto& data : dataset)
        {
        // We only take the extent into account in the calculation of the spatial position and not the area that is covered. That is
        // why we need to do the inverse if the first operation is returning "out". The inverse may return "in".
        if (HGF2DShape::SpatialPosition::S_OUT != regionOfInterest.CalculateSpatialPositionOf(*(data->GetFootprint())))
            selectedDataset.push_back(data);
        else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(regionOfInterest))
            selectedDataset.push_back(data);
        }

    return selectedDataset;
    }

//-------------------------------------------------------------------------------------
// &&JFC TODO: Refactor.
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::CriteriaFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                               ResolutionCriteria qualityCriteria,
                                                                               DateCriteria captureDateCriteria)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // Resolution filtering, create a dataset for each criteria (low, medium and high res).
    bvector<SpatioTemporalDataPtr> lowResDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResDataset = bvector<SpatioTemporalDataPtr>();
    if (SUCCESS != ResolutionFiltering(lowResDataset, mediumResDataset, highResDataset, dataset))
        return selectedDataset;

    // Date filtering, create a dataset for each date criteria (old, recent and up-to-date data)
    // and resolution criteria (low, medium and high res).
    bvector<SpatioTemporalDataPtr> lowResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> lowResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> lowResUpToDateDataset = bvector<SpatioTemporalDataPtr>();
    if (SUCCESS != DateFiltering(lowResOldDataset, lowResRecentDataset, lowResUpToDateDataset, lowResDataset))
        return selectedDataset;

    bvector<SpatioTemporalDataPtr> mediumResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResUpToDateDataset = bvector<SpatioTemporalDataPtr>();
    if (SUCCESS != DateFiltering(mediumResOldDataset, mediumResRecentDataset, mediumResUpToDateDataset, mediumResDataset))
        return selectedDataset;

    bvector<SpatioTemporalDataPtr> highResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResUpToDateDataset = bvector<SpatioTemporalDataPtr>();
    if (SUCCESS != DateFiltering(highResOldDataset, highResRecentDataset, highResUpToDateDataset, highResDataset))
        return selectedDataset;

    // Sort according to criteria.
    if (ResolutionCriteria::Low == qualityCriteria &&
        DateCriteria::Old == captureDateCriteria)
        {
        // Low res only. Oldest to latest capture date.
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        }
    else if (ResolutionCriteria::Low == qualityCriteria &&
             DateCriteria::Recent == captureDateCriteria)
        {
        // Low res only. Recent capture date first, then up-to-date and old.
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (ResolutionCriteria::Low == qualityCriteria &&
             DateCriteria::UpToDate == captureDateCriteria)
        {
        // Low res only. Latest to oldest capture date.
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (ResolutionCriteria::Medium == qualityCriteria &&
             DateCriteria::Old == captureDateCriteria)
        {
        // Medium res first, then low res if necessary. Oldest to latest capture date.
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        }
    else if (ResolutionCriteria::Medium == qualityCriteria &&
             DateCriteria::Recent == captureDateCriteria)
        {
        // Medium res first, then low res if necessary. Recent capture date first, then up-to-date and old.
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (ResolutionCriteria::Medium == qualityCriteria &&
             DateCriteria::UpToDate == captureDateCriteria)
        {
        // Medium res first, then low res if necessary. Latest to oldest capture date.
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (ResolutionCriteria::High == qualityCriteria &&
             DateCriteria::Old == captureDateCriteria)
        {
        // Highest to lowest res. Oldest to latest capture date.
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResUpToDateDataset.begin(), highResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        }
    else if (ResolutionCriteria::High == qualityCriteria &&
             DateCriteria::Recent == captureDateCriteria)
        {
        // Highest to lowest res. Recent capture date first, then up-to-date and old.
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResUpToDateDataset.begin(), highResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (ResolutionCriteria::High == qualityCriteria &&
             DateCriteria::UpToDate == captureDateCriteria)
        {
        // Highest to lowest res. Latest to oldest capture date.
        selectedDataset.insert(selectedDataset.end(), highResUpToDateDataset.begin(), highResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResUpToDateDataset.begin(), mediumResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResUpToDateDataset.begin(), lowResUpToDateDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }

    return selectedDataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::Select(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                    const HGF2DShape& regionOfInterest)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // Create 2 shapes, one that will represent the filled region and the other the area to complete. When the filled area
    // is equal to the region of interest and that the missing area is empty, that means that we have found all the data we need.
    HFCPtr<HGF2DShape> pFilledArea;
    HFCPtr<HGF2DShape> pMissingArea;
    for (const auto& data : dataset)
        {
        if (NULL == pFilledArea)
            {
            selectedDataset.push_back(data);

            pFilledArea = data->GetFootprint();
            pMissingArea = regionOfInterest.DifferentiateShape(*pFilledArea);
            }
        else
            {
            // We only take the extent into account in the calculation of the spatial position and not the area that is covered. That is
            // why we need to do the inverse if the first operation is returning "out". The inverse may return "in".
            if (HGF2DShape::SpatialPosition::S_OUT != pMissingArea->CalculateSpatialPositionOf(*(data->GetFootprint())))
                {
                // Data is covering a part that is not already there, add it.
                selectedDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = regionOfInterest.DifferentiateShape(*pFilledArea);
                }
            else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(*pMissingArea))
                {
                // Data is covering a part that is not already there, add it.
                selectedDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = regionOfInterest.DifferentiateShape(*pFilledArea);
                }
            }

        if (HGF2DShapeId_Void == pMissingArea->GetShapeType())
            break;
        }

    return selectedDataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const StatusInt SpatioTemporalSelector::ResolutionFiltering(bvector<SpatioTemporalDataPtr>& lowResDataset,
                                                            bvector<SpatioTemporalDataPtr>& mediumResDataset,
                                                            bvector<SpatioTemporalDataPtr>& highResDataset,
                                                            const bvector<SpatioTemporalDataPtr>& dataset)
    {
    // Find min/max resolution.
    double minRes = DBL_MIN;
    double maxRes = DBL_MAX;
    double resolution = 0.0;
    for (const auto& data : dataset)
        {
        resolution = log(data->GetResolution() * 100);

        if (resolution > minRes)
            minRes = resolution;
        if (resolution < maxRes)
            maxRes = resolution;
        }

    // Find range for every category (low, medium, high).
    double interval = (minRes - maxRes) / 4;
    double lowResMaxRange = minRes + interval;
    double highResMinRange = maxRes - interval;

    interval = (lowResMaxRange - highResMinRange) / 3;
    double lowResMinRange = lowResMaxRange - interval;
    double highResMaxRange = highResMinRange + interval;

    // Sort according to resolution.
    for (const auto& data : dataset)
        {
        resolution = log(data->GetResolution() * 100);
        if (resolution >= lowResMinRange &&
            resolution <= lowResMaxRange)
            {
            lowResDataset.push_back(data);
            }
        else if (resolution < lowResMinRange &&
                    resolution > highResMaxRange)
            {
            mediumResDataset.push_back(data);
            }
        else if (resolution >= highResMinRange &&
                    resolution <= highResMaxRange)
            {
            highResDataset.push_back(data);
            }
        }

    // Make sure every data was sort and none is missing.
    if (dataset.size() != (lowResDataset.size() + mediumResDataset.size() + highResDataset.size()))
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const StatusInt SpatioTemporalSelector::DateFiltering(bvector<SpatioTemporalDataPtr>& oldDataset,
                                                        bvector<SpatioTemporalDataPtr>& recentDataset,
                                                        bvector<SpatioTemporalDataPtr>& upToDateDataset,
                                                        const bvector<SpatioTemporalDataPtr>& dataset)
    {
    // Find oldest/latest date.
    double minDateInJulian = DBL_MAX;
    double maxDateInJulian = DBL_MIN;
    DateTime date = DateTime();
    double dateInJulian = 0.0;
    for (const auto& data : dataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian < minDateInJulian)
                minDateInJulian = dateInJulian;
            if (dateInJulian > maxDateInJulian)
                maxDateInJulian = dateInJulian;
            }
        }

    // Find range for every category (old, recent, up-to-date).
    double interval = (maxDateInJulian - minDateInJulian) / 3;
    double oldDateMinRange = minDateInJulian - interval;
    double oldDateMaxRange = minDateInJulian + interval;
    double upToDateMinRange = maxDateInJulian - interval;
    double upToDateMaxRange = maxDateInJulian + interval;

    // Sort according to date.
    for (const auto& data : dataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian >= oldDateMinRange &&
                dateInJulian <= oldDateMaxRange)
                {
                oldDataset.push_back(data);
                }
            else if (dateInJulian > oldDateMaxRange &&
                        dateInJulian < upToDateMinRange)
                {
                recentDataset.push_back(data);
                }
            else if (dateInJulian >= upToDateMinRange &&
                        dateInJulian <= upToDateMaxRange)
                {
                upToDateDataset.push_back(data);
                }
            }
        }

    // Make sure every data was sort and none is missing.
    if (dataset.size() != (oldDataset.size() + recentDataset.size() + upToDateDataset.size()))
        return ERROR;

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    12/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::CriteriaFiltering(const bvector<SpatioTemporalDataPtr>& dataset,
                                                                               const double minResolution,
                                                                               const double maxResolution,
                                                                               const DateTime& minDate,
                                                                               const DateTime& maxDate)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // Resolution filtering, sort according to criteria.
    double resolution = 0.0;
    bvector<SpatioTemporalDataPtr> resDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : dataset)
        {
        resolution = log(data->GetResolution() * 100);
        if (resolution >= minResolution &&
            resolution <= maxResolution)
            {
            resDataset.push_back(data);
            }
        }

    // Date filtering, sort according to criteria.
    DateTime date = DateTime();
    double dateInJulian = 0.0;
    double minDateInJulian = 0.0;
    double maxDateInJulian = 0.0;
    minDate.ToJulianDay(minDateInJulian);
    maxDate.ToJulianDay(maxDateInJulian);
    for (const auto& data : resDataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian >= minDateInJulian &&
                dateInJulian <= maxDateInJulian)
                {
                selectedDataset.push_back(data);
                }
            }
        }

    return selectedDataset;
    }
