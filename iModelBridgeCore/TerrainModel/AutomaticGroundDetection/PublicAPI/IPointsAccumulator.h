/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/PublicAPI/IPointsAccumulator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/NonCopyableClass.h>
#include <Bentley/BeFileName.h>

#include <TerrainModel\AutomaticGroundDetection\GroundDetectionMacros.h>

GROUND_DETECTION_TYPEDEF(IGroundPointsAccumulator)


BEGIN_GROUND_DETECTION_NAMESPACE
    
/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     12/2015
+===============+===============+===============+===============+===============+======*/
struct IGroundPointsAccumulator : public RefCountedBase
    {
    protected : 

        virtual void _AddPoints(const bvector<DPoint3d>& points) = 0;

    public : 
    
        void AddPoints(const bvector<DPoint3d>& points);        
    };

END_GROUND_DETECTION_NAMESPACE
