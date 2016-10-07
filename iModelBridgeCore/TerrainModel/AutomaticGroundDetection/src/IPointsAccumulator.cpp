/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/IPointsAccumulator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel\AutomaticGroundDetection\GroundDetectionMacros.h>
#include <TerrainModel\AutomaticGroundDetection\IPointsAccumulator.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IGroundPointsAccumulator::AddPoints(const bvector<DPoint3d>& points)
    {
    return _AddPoints(points);
    }

END_GROUND_DETECTION_NAMESPACE
