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
#include "HitDetail.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface to supply additional topology information that describes the subsequent geometry.
//! The ViewContext's current IElemTopology will be cloned and saved as part of the HitDetail
//! when picking. Can be used to make transient geometry locatable; set context.SetElemTopology
//! before drawing the geometry (ex. IViewTransients) and implement ITransientGeometryHandler.
//! @note Always call context.SetElemTopology(nullptr) after drawing geometry.
//=======================================================================================
struct IElemTopology : BentleyApi::IRefCounted
{
//! Create a deep copy of this object.
virtual IElemTopologyP _Clone() const = 0;

//! Compare objects and return true if they should be considered the same.
virtual bool _IsEqual (IElemTopologyCR) const = 0;

//! Return an object for handling requests related to locate of transient geometry where we don't have an element handler.
virtual ITransientGeometryHandlerP _GetTransientGeometryHandler() const = 0;

}; // IElemTopology

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

