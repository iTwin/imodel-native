/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Mtg/MtgStructs.h>
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <vector>
#include <list>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines, bvector<DTMFeatureType>& types, bvector<DPoint3d>& removedPoints, double distanceTol = 0.5, size_t targetNumPoints = 10000);

END_BENTLEY_SCALABLEMESH_NAMESPACE