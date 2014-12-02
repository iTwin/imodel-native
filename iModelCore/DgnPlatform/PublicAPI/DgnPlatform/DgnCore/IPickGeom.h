/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/IPickGeom.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>
#include "HitPath.h"


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface to supply topology information for an element being located. A copy of this object is stored on the HitPaths so that it
//! can be used to interpet the hit information later if and when the user accepts a hit.
// @bsiclass 
//=======================================================================================
struct IElemTopology
{
//! During the locating process, many copies of this object are needed. This method must make an equivalent copy of the object that can be
//! independently deleted.
virtual IElemTopology* _Clone () const = 0;

//! All implementers of this interface must have a virtual destructor.
virtual ~IElemTopology () {}

//! Compare this object to another object implementing IElemTopology and return 0 the two should be considered "the same" and 1 otherwise.
virtual int _Compare (IElemTopologyCR otherTopo) const {return 0;}
};

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

