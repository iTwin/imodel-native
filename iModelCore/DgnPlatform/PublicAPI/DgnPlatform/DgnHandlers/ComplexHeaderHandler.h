/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ComplexHeaderHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/ElementHandle.h>
#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup DisplayHandler
/// @beginGroup

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Base class for displayable complex element types. Encapsulates behavior that is
* common to complex elements types such as propagating changes to child/component
* elements.
* @note ComplexHeaderDisplayHandler is never the element handler for any element, it
*       is simply a base class for displayable complex header types.
* @bsiclass                                                     Sam.Wilson      11/04
+===============+===============+===============+===============+===============+======*/
struct          ComplexHeaderDisplayHandler : DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ComplexHeaderDisplayHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual void        _QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context);
DGNPLATFORM_EXPORT virtual void        _EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context);

// Handler
DGNPLATFORM_EXPORT virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnChangeOfUnits (EditElementHandleR, DgnModelP source, DgnModelP dest) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual void        _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void        _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual bool            _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual BentleyStatus   _ValidateElementRange (EditElementHandleR elHandle) override;

public:

DGNPLATFORM_EXPORT static bool       GetComponentForDisplayParams (ElementHandleR templateElHandle, ElementHandleCR elHandle);
                   static StatusInt  DropComplex (ElementHandleCR eh, ElementAgendaR dropGeom);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // ComplexHeaderDisplayHandler
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
