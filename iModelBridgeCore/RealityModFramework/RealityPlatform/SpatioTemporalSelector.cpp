/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalSelector.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "SpatioTemporalData.h"

#include <RealityPlatform/SpatioTemporalSelector.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM


//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDsFromJson(const bvector<GeoPoint2d>& regionOfInterest,
                                                                 Utf8CP data, 
                                                                 SelectionCriteria qualityCriteria,
                                                                 SelectionCriteria captureDateCriteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return selectedIDs;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr pDataset = SpatioTemporalDataset::CreateFromJson(data);
    if (NULL == pDataset.get())
        return selectedIDs;

    if (SelectionCriteria::Default == qualityCriteria ||
        SelectionCriteria::Default == captureDateCriteria)
        {
        // Select imagery data.
        bvector<Utf8String> imageryIDs = GetIDs(regionOfInterest, pDataset->GetImageryGroup());
        if (!imageryIDs.empty())
            selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

        // Select terrain data.
        bvector<Utf8String> terrainIDs = GetIDs(regionOfInterest, pDataset->GetTerrainGroup());
        if (!terrainIDs.empty())
            selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());
        }
    else
        {
        // Select imagery data.
        bvector<Utf8String> imageryIDs = GetIDsByCriteria(regionOfInterest, pDataset->GetImageryGroup(), qualityCriteria, captureDateCriteria);
        if (!imageryIDs.empty())
            selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

        // Select terrain data.
        bvector<Utf8String> terrainIDs = GetIDsByCriteria(regionOfInterest, pDataset->GetTerrainGroup(), qualityCriteria, captureDateCriteria);
        if (!terrainIDs.empty())
            selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());
        }

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDs(const bvector<GeoPoint2d>& regionOfInterest,
                                                         const bvector<SpatioTemporalDataPtr>& dataset)
    {    
    /*
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
    selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;
    */

    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // First draft: Spatial position selection - Included in region of interest.
    // Create shape that represents the region of interest.
    HGF2DPositionCollection ptsCollection;
    for (const auto& pt : regionOfInterest)
        {
        ptsCollection.push_back(HGF2DCoord<double>(pt.longitude, pt.latitude));
        }

    HFCPtr<HGF2DShape> pROIShape = new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    if (pROIShape->IsEmpty())
        return selectedIDs;

    bvector<SpatioTemporalDataPtr> firstDraftDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : dataset)
        {
        HGF2DShape::SpatialPosition bob1 = pROIShape->CalculateSpatialPositionOf(*(data->GetFootprint())); bob1;
        HGF2DShape::SpatialPosition bob2 = data->GetFootprint()->CalculateSpatialPositionOf(*pROIShape); bob2;
        //&&JFC Comment
        if (HGF2DShape::SpatialPosition::S_OUT != pROIShape->CalculateSpatialPositionOf(*(data->GetFootprint())))
            firstDraftDataset.push_back(data);
        else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(*pROIShape))
            firstDraftDataset.push_back(data);
        }

    // Second draft: Resolution selection - Compare resolutions and keep high-res only.
    // Sort by resolution. High-res first.
    bvector<SpatioTemporalDataPtr> sortByResDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& dataToSort : firstDraftDataset)
        {
        if (sortByResDataset.empty())
            {
            sortByResDataset.push_back(dataToSort);
            }
        else
            {
            double dataToSortRes = dataToSort->GetResolution();
            bvector<SpatioTemporalDataPtr>::iterator dataIterator = sortByResDataset.begin();
            for (; dataIterator != sortByResDataset.end(); ++dataIterator)
                {
                if (dataToSortRes < (*dataIterator)->GetResolution())
                    {
                    sortByResDataset.insert(dataIterator, dataToSort);
                    break;
                    }
                else if (*dataIterator == sortByResDataset.back())
                    {
                    sortByResDataset.push_back(dataToSort);
                    break;
                    }
                }
            }
        }

    HFCPtr<HGF2DShape> pFilledArea;
    HFCPtr<HGF2DShape> pMissingArea;
    bvector<SpatioTemporalDataPtr> secondDraftDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : sortByResDataset)
        {
        if (NULL == pFilledArea)
            {
            secondDraftDataset.push_back(data);

            pFilledArea = data->GetFootprint();
            pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
            }
        else
            {
            HGF2DShape::SpatialPosition bob = pMissingArea->CalculateSpatialPositionOf(*(data->GetFootprint())); bob;
            //&&JFC Comment
            if (HGF2DShape::SpatialPosition::S_OUT != pMissingArea->CalculateSpatialPositionOf(*(data->GetFootprint())))
                {
                // Data is covering a part that is not already there, add it.
                secondDraftDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
                }
            else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(*pMissingArea))
                {
                // Data is covering a part that is not already there, add it.
                secondDraftDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
                }
            }

        if (HGF2DShapeId_Void == pMissingArea->GetShapeType())
            break;
        }


    // Third draft: Date selection - If there is any hole, complete dataset with the latest data possible.



    for (const auto& data : secondDraftDataset)
        {
        selectedIDs.push_back(data->GetIdentifier());
        }

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::GetIDsByCriteria(const bvector<GeoPoint2d>& regionOfInterest,
                                                                   const bvector<SpatioTemporalDataPtr>& dataset,
                                                                   SelectionCriteria qualityCriteria,
                                                                   SelectionCriteria captureDateCriteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // First draft: Spatial position filtering - Included in region of interest.
    bvector<SpatioTemporalDataPtr> firstDraftDataset = PositionFiltering(regionOfInterest, dataset);

    // Second draft: Resolution and date filtering - Sort data according to quality and capture date criteria..
    bvector<SpatioTemporalDataPtr> secondDraftDataset = CriteriaFiltering(regionOfInterest, firstDraftDataset, qualityCriteria, captureDateCriteria);

    // Third draft: Data selection - Fill region of interest with best matching data.
    bvector<SpatioTemporalDataPtr> thirdDraftDataset = Select(regionOfInterest, secondDraftDataset);

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
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::PositionFiltering(const bvector<GeoPoint2d>& regionOfInterest,
                                                                               const bvector<SpatioTemporalDataPtr>& dataset)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // &&JFC Find a way to create the HGF2DShape representing the region of interest only one time.
    // Create shape that represents the region of interest.
    HGF2DPositionCollection ptsCollection;
    for (const auto& pt : regionOfInterest)
        {
        ptsCollection.push_back(HGF2DCoord<double>(pt.longitude, pt.latitude));
        }

    HFCPtr<HGF2DShape> pROIShape = new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    if (pROIShape->IsEmpty())
        return selectedDataset;

    // Position filtering, keep only the data included in region of interest.
    for (const auto& data : dataset)
        {
        //&&JFC Comment why we need to do the inverse.
        if (HGF2DShape::SpatialPosition::S_OUT != pROIShape->CalculateSpatialPositionOf(*(data->GetFootprint())))
            selectedDataset.push_back(data);
        else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(*pROIShape))
            selectedDataset.push_back(data);
        }

    return selectedDataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::CriteriaFiltering(const bvector<GeoPoint2d>& regionOfInterest,
                                                                               const bvector<SpatioTemporalDataPtr>& dataset,
                                                                               SelectionCriteria qualityCriteria,
                                                                               SelectionCriteria captureDateCriteria)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // &&JFC Find a way to create the HGF2DShape representing the region of interest only one time.
    // Create shape that represents the region of interest.
    HGF2DPositionCollection ptsCollection;
    for (const auto& pt : regionOfInterest)
        {
        ptsCollection.push_back(HGF2DCoord<double>(pt.longitude, pt.latitude));
        }

    HFCPtr<HGF2DShape> pROIShape = new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    if (pROIShape->IsEmpty())
        return selectedDataset;

    // Resolution filtering, sort according to criteria.
    // Low resolution: log(100cm) = 2
    // Medium resolution: log(25cm) = 1.39
    // High resolution: log(12cm) = 1.09
    double minRes = DBL_MIN;
    double maxRes = DBL_MAX;
    double resolution = 0.0;
    for (const auto& data : dataset)
        {
        // Find min/max resolution to determine the range.
        resolution = log(data->GetResolution() * 100);

        if (resolution > minRes)
            minRes = resolution;
        if (resolution < maxRes)
            maxRes = resolution;
        }

    // Find in which range the data resolution is.
    double interval = (minRes - maxRes) / 4;
    double lowResMaxRange = minRes + interval;
    double highResMinRange = maxRes - interval;

    interval = (lowResMaxRange - highResMinRange) / 3;
    double lowResMinRange = lowResMaxRange - interval;
    double highResMaxRange = highResMinRange + interval;

    bvector<SpatioTemporalDataPtr> lowResDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResDataset = bvector<SpatioTemporalDataPtr>();
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

    DateTime date = DateTime();
    double dateInJulian = 0.0;
    double minDateInJulian = DBL_MAX;
    double maxDateInJulian = DBL_MIN;
    // Date filtering, sort according to criteria.
    for (const auto& data : lowResDataset)
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

    // Find in which range the date is.
    interval = (maxDateInJulian - minDateInJulian) / 3;
    double oldDateMinRange = minDateInJulian - interval;
    double oldDateMaxRange = minDateInJulian + interval;
    double actualDateMinRange = maxDateInJulian - interval;
    double actualDateMaxRange = maxDateInJulian + interval;

    bvector<SpatioTemporalDataPtr> lowResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> lowResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> lowResActualDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : lowResDataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian >= oldDateMinRange &&
                dateInJulian <= oldDateMaxRange)
                {
                lowResOldDataset.push_back(data);
                }
            else if (dateInJulian > oldDateMaxRange &&
                     dateInJulian < actualDateMinRange)
                {
                lowResRecentDataset.push_back(data);
                }
            else if (dateInJulian >= actualDateMinRange &&
                     dateInJulian <= actualDateMaxRange)
                {
                lowResActualDataset.push_back(data);
                }
            }
        }

    for (const auto& data : mediumResDataset)
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

    // Find in which range the date is.
    interval = (maxDateInJulian - minDateInJulian) / 3;
    oldDateMinRange = minDateInJulian - interval;
    oldDateMaxRange = minDateInJulian + interval;
    actualDateMinRange = maxDateInJulian - interval;
    actualDateMaxRange = maxDateInJulian + interval;

    bvector<SpatioTemporalDataPtr> mediumResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> mediumResActualDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : mediumResDataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian >= oldDateMinRange &&
                dateInJulian <= oldDateMaxRange)
                {
                mediumResOldDataset.push_back(data);
                }
            else if (dateInJulian > oldDateMaxRange &&
                     dateInJulian < actualDateMinRange)
                {
                mediumResRecentDataset.push_back(data);
                }
            else if (dateInJulian >= actualDateMinRange &&
                     dateInJulian <= actualDateMaxRange)
                {
                mediumResActualDataset.push_back(data);
                }
            }
        }

    for (const auto& data : highResDataset)
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

    // Find in which range the date is.
    interval = (maxDateInJulian - minDateInJulian) / 3;
    oldDateMinRange = minDateInJulian - interval;
    oldDateMaxRange = minDateInJulian + interval;
    actualDateMinRange = maxDateInJulian - interval;
    actualDateMaxRange = maxDateInJulian + interval;

    bvector<SpatioTemporalDataPtr> highResOldDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResRecentDataset = bvector<SpatioTemporalDataPtr>();
    bvector<SpatioTemporalDataPtr> highResActualDataset = bvector<SpatioTemporalDataPtr>();
    for (const auto& data : highResDataset)
        {
        date = data->GetDate();
        if (date.IsValid())
            {
            date.ToJulianDay(dateInJulian);

            if (dateInJulian >= oldDateMinRange &&
                dateInJulian <= oldDateMaxRange)
                {
                highResOldDataset.push_back(data);
                }
            else if (dateInJulian > oldDateMaxRange &&
                     dateInJulian < actualDateMinRange)
                {
                highResRecentDataset.push_back(data);
                }
            else if (dateInJulian >= actualDateMinRange &&
                     dateInJulian <= actualDateMaxRange)
                {
                highResActualDataset.push_back(data);
                }
            }
        }

    if (SelectionCriteria::Resolution_Good == qualityCriteria &&
        SelectionCriteria::Date_Less == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        }
    else if (SelectionCriteria::Resolution_Good == qualityCriteria &&
             SelectionCriteria::Date_Recent == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (SelectionCriteria::Resolution_Good == qualityCriteria &&
             SelectionCriteria::Date_Most == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (SelectionCriteria::Resolution_Better == qualityCriteria &&
             SelectionCriteria::Date_Less == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        }
    else if (SelectionCriteria::Resolution_Better == qualityCriteria &&
             SelectionCriteria::Date_Recent == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (SelectionCriteria::Resolution_Better == qualityCriteria &&
             SelectionCriteria::Date_Most == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (SelectionCriteria::Resolution_Best == qualityCriteria &&
             SelectionCriteria::Date_Less == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResActualDataset.begin(), highResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        }
    else if (SelectionCriteria::Resolution_Best == qualityCriteria &&
             SelectionCriteria::Date_Recent == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResActualDataset.begin(), highResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }
    else if (SelectionCriteria::Resolution_Best == qualityCriteria &&
             SelectionCriteria::Date_Most == captureDateCriteria)
        {
        selectedDataset.insert(selectedDataset.end(), highResActualDataset.begin(), highResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResRecentDataset.begin(), highResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), highResOldDataset.begin(), highResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResActualDataset.begin(), mediumResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResRecentDataset.begin(), mediumResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), mediumResOldDataset.begin(), mediumResOldDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResActualDataset.begin(), lowResActualDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResRecentDataset.begin(), lowResRecentDataset.end());
        selectedDataset.insert(selectedDataset.end(), lowResOldDataset.begin(), lowResOldDataset.end());
        }

    return selectedDataset;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<SpatioTemporalDataPtr> SpatioTemporalSelector::Select(const bvector<GeoPoint2d>& regionOfInterest,
                                                                    const bvector<SpatioTemporalDataPtr>& dataset)
    {
    bvector<SpatioTemporalDataPtr> selectedDataset = bvector<SpatioTemporalDataPtr>();

    // &&JFC Find a way to create the HGF2DShape representing the region of interest only one time.
    // Create shape that represents the region of interest.
    HGF2DPositionCollection ptsCollection;
    for (const auto& pt : regionOfInterest)
        {
        ptsCollection.push_back(HGF2DCoord<double>(pt.longitude, pt.latitude));
        }

    HFCPtr<HGF2DShape> pROIShape = new HGF2DPolygonOfSegments(HGF2DPolySegment(ptsCollection));
    if (pROIShape->IsEmpty())
        return selectedDataset;

    HFCPtr<HGF2DShape> pFilledArea;
    HFCPtr<HGF2DShape> pMissingArea;
    for (const auto& data : dataset)
        {
        if (NULL == pFilledArea)
            {
            selectedDataset.push_back(data);

            pFilledArea = data->GetFootprint();
            pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
            }
        else
            {
            HGF2DShape::SpatialPosition bob = pMissingArea->CalculateSpatialPositionOf(*(data->GetFootprint())); bob;
            //&&JFC Comment
            if (HGF2DShape::SpatialPosition::S_OUT != pMissingArea->CalculateSpatialPositionOf(*(data->GetFootprint())))
                {
                // Data is covering a part that is not already there, add it.
                selectedDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
                }
            else if (HGF2DShape::SpatialPosition::S_OUT != data->GetFootprint()->CalculateSpatialPositionOf(*pMissingArea))
                {
                // Data is covering a part that is not already there, add it.
                selectedDataset.push_back(data);

                pFilledArea = pFilledArea->UnifyShape(*(data->GetFootprint()));
                pMissingArea = pROIShape->DifferentiateShape(*pFilledArea);
                }
            }

        if (HGF2DShapeId_Void == pMissingArea->GetShapeType())
            break;
        }

    return selectedDataset;
    }
