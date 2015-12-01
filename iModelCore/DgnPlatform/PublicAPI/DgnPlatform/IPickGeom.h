/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/IPickGeom.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include "HitDetail.h"
#include "IManipulator.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Geometry data associated with a pick.
//=======================================================================================
struct IPickGeom
{
    virtual DPoint4dCR _GetPickPointView() const = 0;
    virtual DPoint3dCR _GetPickPointWorld() const = 0;
    virtual GeomDetailR _GetGeomDetail() = 0;
    virtual bool _IsPointVisible(DPoint3dCP screenPt) = 0;
    virtual void _SetHitPriorityOverride(HitPriority priority) = 0;
    virtual void _AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority) = 0;
    virtual bool _IsSnap() const = 0;
    virtual DRay3d _GetBoresite() const = 0;
};

END_BENTLEY_DGN_NAMESPACE

