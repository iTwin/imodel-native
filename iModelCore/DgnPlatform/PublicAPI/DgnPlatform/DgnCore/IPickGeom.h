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
// @bsiclass 
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
// @bsiclass 
//=======================================================================================
struct IPickGeom
{
private:
DECLARE_KEY_METHOD
//__PUBLISH_SECTION_END__
protected:

virtual DPoint4dCR              _GetPickPointView       () const = 0;
virtual DPoint3dCR              _GetPickPointWorld      () const = 0;
virtual GeomDetail&             _GetGeomDetail          () = 0;
virtual bool                    _IsPointVisible         (DPoint3dCP screenPt) = 0;
virtual void                    _SetHitPriorityOverride (HitPriority priority) = 0;
virtual void                    _AddHit                 (DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority) = 0;
virtual bool                    _IsSnap                 () const {return false;}
virtual DRay3d                  _GetBoresite            () const = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

DGNPLATFORM_EXPORT DPoint4dCR       GetPickPointView        () const;
DGNPLATFORM_EXPORT DPoint3dCR       GetPickPointWorld       () const;
DGNPLATFORM_EXPORT GeomDetail&      GetGeomDetail           ();
DGNPLATFORM_EXPORT bool             IsPointVisible          (DPoint3dCP screenPt);
DGNPLATFORM_EXPORT void             SetHitPriorityOverride  (HitPriority priority);
DGNPLATFORM_EXPORT void             AddHit                  (DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority);
DGNPLATFORM_EXPORT bool             IsSnap                  () const;
DGNPLATFORM_EXPORT DRay3d           GetBoresite             () const;

}; // IPickGeom

END_BENTLEY_DGNPLATFORM_NAMESPACE

