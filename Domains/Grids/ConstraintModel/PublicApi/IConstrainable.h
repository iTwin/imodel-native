/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/IConstrainable.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "ConstraintModelMacros.h"
#include <DgnPlatform/DgnModel.h>

BEGIN_CONSTRAINTMODEL_NAMESPACE

//=======================================================================================
//! A physical Extrusion element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConstrainable
    {
    //! @private
    explicit CONSTRAINTMODEL_EXPORT IConstrainable () {}
    CONSTRAINTMODEL_EXPORT virtual bool GetGeomIdPlane (int geomId, DPlane3dR planeOut) const = 0; // NEEDS WORK: coincident non-planar faces?
    CONSTRAINTMODEL_EXPORT virtual bool StretchGeomIdToPlane (int geomId, DPlane3dR targetPlane) = 0;
    }; // IConstrainable

END_CONSTRAINTMODEL_NAMESPACE