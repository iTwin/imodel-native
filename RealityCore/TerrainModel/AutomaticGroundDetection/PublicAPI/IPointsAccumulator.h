/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/NonCopyableClass.h>
#include <Bentley/BeFileName.h>

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>

GROUND_DETECTION_TYPEDEF(IGroundPointsAccumulator)


BEGIN_GROUND_DETECTION_NAMESPACE
    
/*=================================================================================**//**
* @bsiclass                                             Mathieu.St-Pierre     10/2016
+===============+===============+===============+===============+===============+======*/
struct IGroundPointsAccumulator : public RefCountedBase
    {
    protected : 

        virtual void _AddPoints(const bvector<DPoint3d>& points) = 0;        

        virtual void _GetPreviewTransform(Transform& transform) const = 0;

        virtual void _OutputPreview(PolyfaceQueryCR currentGround) const = 0;

        virtual bool _ShouldContinue() const = 0;

    public : 
    
        void AddPoints(const bvector<DPoint3d>& points);     

        void GetPreviewTransform(Transform& transform) const;

        void OutputPreview(PolyfaceQueryCR currentGround) const;

        bool ShouldContinue() const;
    };

END_GROUND_DETECTION_NAMESPACE
