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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface to supply additional topology information that describes the subsequent geometry.
//! The ViewContext's current IElemTopology will be cloned and saved as part of the HitDetail
//! when picking. Can be used to make transient geometry locatable; set context.SetElemTopology
//! before drawing the geometry (ex. IViewTransients) and implement _ToGeometrySource.
//! @note Always call context.SetElemTopology(nullptr) after drawing geometry.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IElemTopology : BentleyApi::IRefCounted
{
//! Create a deep copy of this object.
virtual IElemTopologyP _Clone() const = 0;

//! Compare objects and return true if they should be considered the same.
virtual bool _IsEqual (IElemTopologyCR) const = 0;

//! Return GeometrySource to handle requests related to transient geometry (like locate) where we don't have an DgnElement.
virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}

//! Return IEditManipulator for interacting with transient geometry.
//! @note Implementor is expected to check hit.GetDgnDb().IsReadonly().
virtual IEditManipulatorPtr _GetTransientManipulator (HitDetailCR) const {return nullptr;}

}; // IElemTopology

typedef RefCountedPtr<IElemTopology> IElemTopologyPtr;   //!< Reference counted type to manage the life-cycle of the IElemTopology.
typedef RefCountedCPtr<IElemTopology> IElemTopologyCPtr; //!< Reference counted type to manage the life-cycle of the IElemTopology.

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

