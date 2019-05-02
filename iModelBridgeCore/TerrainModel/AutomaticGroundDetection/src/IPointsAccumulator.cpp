/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsAccumulator.h>

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void IGroundPointsAccumulator::GetPreviewTransform(Transform& transform) const
    {
    return _GetPreviewTransform(transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void IGroundPointsAccumulator::OutputPreview(PolyfaceQueryCR currentGround) const
    {
    return _OutputPreview(currentGround);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool IGroundPointsAccumulator::ShouldContinue() const
    {
    return _ShouldContinue();
    }

END_GROUND_DETECTION_NAMESPACE
