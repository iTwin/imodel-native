/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/CGALEdgeCollapse.h $
|    $RCSfile: CGALEdgeCollapse.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/11/30 14:54:23 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Mtg/MtgStructs.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <vector>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

bool CGALEdgeCollapse(MTGGraph* inoutMesh, std::vector<DPoint3d>& pts, uint64_t id =0);
void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines);

END_BENTLEY_SCALABLEMESH_NAMESPACE