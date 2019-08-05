/*--------------------------------------------------------------------------------------+
|    $RCSfile: CGALEdgeCollapse.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/11/30 14:54:23 $
|     $Author: Elenie.Godzaridis $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Mtg/MtgStructs.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <vector>
#include <list>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct DPoint3dLinestringSortByAreaComparison
    {
    const std::list<DPoint3d>* m_polyPtr;
    DPoint3dLinestringSortByAreaComparison(const std::list<DPoint3d>& poly) : m_polyPtr(&poly) {}
    double ComputeAreaPoint(const std::list<DPoint3d>::iterator& pointItr) const
        {
        if(pointItr != m_polyPtr->begin() && pointItr != --m_polyPtr->end())
            {
            auto left = pointItr;
            --left;
            auto right = pointItr;
            ++right;
            double dX0 = left->x, dY0 = left->y;
            double dX1 = pointItr->x, dY1 = pointItr->y;
            double dX2 = right->x, dY2 = right->y;
            double dArea = ((dX1 - dX0)*(dY2 - dY0) - (dX2 - dX0)*(dY1 - dY0)) / 2.0;
            return (dArea > 0.0) ? dArea : -dArea;
            }
        return 0;
        }
    bool operator()(std::list<DPoint3d>::iterator first, std::list<DPoint3d>::iterator second)
        {
        return (ComputeAreaPoint(first) > ComputeAreaPoint(second));
        }
    };

bool CGALEdgeCollapse(MTGGraph* inoutMesh, std::vector<DPoint3d>& pts, uint64_t id =0);
void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines);

END_BENTLEY_SCALABLEMESH_NAMESPACE