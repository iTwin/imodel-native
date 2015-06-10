/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IPickGeom.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include "HitPath.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface for supplying additional topology information for an element being located.
//! Can be stored with the hit detail and later used to interpet the hit information.
//=======================================================================================
struct IElemTopology : Bentley::IRefCounted
{
//! Create a deep copy of this object.
virtual IElemTopologyP _Clone() const = 0;

//! Compare objects and return true if they should be considered the same.
virtual bool _IsEqual (IElemTopologyCR) const = 0;
};

typedef RefCountedPtr<IElemTopology> IElemTopologyPtr; //!< Reference counted type to manage the life-cycle of the IElemTopology.

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

}; // IPickGeom

END_BENTLEY_DGNPLATFORM_NAMESPACE

