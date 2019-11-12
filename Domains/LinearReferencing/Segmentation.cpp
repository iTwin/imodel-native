/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/Segmentation.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearSegment> ISegmentableLinearElement::_QueryLinearSegments(QueryParams const& params) const
    {
    auto linearLocations = QueryLinearLocations(params);

    bvector<bpair<double, double>> currentStartStops;
    for (auto const& linearLocation : linearLocations)
        currentStartStops.push_back({ linearLocation.GetStartDistanceAlong(), linearLocation.GetStopDistanceAlong() });

    bvector<LinearSegment> retVal;
    
    bvector<LinearLocationReference> segmentReferences;
    while (!currentStartStops.empty())
        {
        double minStart = DBL_MAX;
        for (auto const& startStop : currentStartStops)
            minStart = std::min(minStart, startStop.first);

        double minStopAfterStart = DBL_MAX;
        for (auto const& startStop : currentStartStops)
            {
            if (startStop.first > minStart)
                minStopAfterStart = std::min(minStopAfterStart, startStop.first);
            else
                minStopAfterStart = std::min(minStopAfterStart, startStop.second);
            }

        segmentReferences.clear();
        for (int i = (int)currentStartStops.size() - 1; i >= 0; --i)
            {
            if (fabs(currentStartStops.at(i).first - minStart) < DBL_EPSILON)
                {
                segmentReferences.push_back(linearLocations.at(i));
                
                if (fabs(currentStartStops.at(i).second - minStopAfterStart) < DBL_EPSILON)
                    {
                    currentStartStops.erase(&currentStartStops.at(i));
                    linearLocations.erase(&linearLocations.at(i));
                    }
                else
                    currentStartStops.at(i).first = minStopAfterStart;
                }
            }

        retVal.push_back(LinearSegment(minStart, minStopAfterStart, segmentReferences));
        }

    return retVal;
    }