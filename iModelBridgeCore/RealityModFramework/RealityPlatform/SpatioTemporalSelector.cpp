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
                                                                 SelectionCriteria criteria)
    {
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    // Make sure data exists.
    if (Utf8String::IsNullOrEmpty(data))
        return selectedIDs;

    // Parse JSON and create dataset.
    SpatioTemporalDatasetPtr pDataset = SpatioTemporalDataset::CreateFromJson(data);
    if (NULL == pDataset.get())
        return selectedIDs;

    // Select imagery data.
    bvector<Utf8String> imageryIDs = Select(regionOfInterest, pDataset->GetImageryGroup(), criteria);
    if (!imageryIDs.empty())
        selectedIDs.insert(selectedIDs.end(), imageryIDs.begin(), imageryIDs.end());

    // Select terrain data.
    bvector<Utf8String> terrainIDs = Select(regionOfInterest, pDataset->GetTerrainGroup(), criteria);
    if (!terrainIDs.empty())
        selectedIDs.insert(selectedIDs.end(), terrainIDs.begin(), terrainIDs.end());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::Select(const bvector<GeoPoint2d>& regionOfInterest,
                                                         const bvector<SpatioTemporalDataPtr>& dataset,
                                                         SelectionCriteria criteria)
    {
    switch (criteria)
        {
        case SelectionCriteria::Date:
            {
            return SelectByDate(regionOfInterest, dataset);
            }
        case SelectionCriteria::Resolution:
            {
            return SelectByResolution(regionOfInterest, dataset);
            }
        case SelectionCriteria::DateAndResolution:
            {
            return SelectByDateAndResolution(regionOfInterest, dataset);
            }
        default:
            {
            return SelectByDateAndResolution(regionOfInterest, dataset);
            }
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByDate(const bvector<GeoPoint2d>& regionOfInterest,
                                                               const bvector<SpatioTemporalDataPtr>& dataset)
    {
    //&&JFC TODO
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
        selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByResolution(const bvector<GeoPoint2d>& regionOfInterest,
                                                                     const bvector<SpatioTemporalDataPtr>& dataset)
    {
    //&&JFC TODO
    bvector<Utf8String> selectedIDs = bvector<Utf8String>();

    for (const auto& data : dataset)
        selectedIDs.push_back(data->GetIdentifier());

    return selectedIDs;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    10/2015
//-------------------------------------------------------------------------------------
const bvector<Utf8String> SpatioTemporalSelector::SelectByDateAndResolution(const bvector<GeoPoint2d>& regionOfInterest,
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
