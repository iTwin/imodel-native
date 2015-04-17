/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/Annotations/TextAnnotationHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnCore/Annotations/Annotations.h>

DGNPLATFORM_TYPEDEFS(TextAnnotationHandler)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

#if defined (V10_WIP_ELEMENTHANDLER)
//=======================================================================================
//! This is an element handler that can persist a TextAnnotation object to the file.
//! When creating a TextAnnotation, all of its positional information is local to the annotation itself, with the exception of some leader attachment modes. The Transform provided to various handler methods is how you can position the annotation in the model.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct TextAnnotationHandler : public GraphicElementHandler
{
    DEFINE_T_SUPER(GraphicElementHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS(TextAnnotationHandler, DGNPLATFORM_EXPORT)

//__PUBLISH_SECTION_END__
protected:
    // Handler
    virtual void _EditProperties(EditElementHandleR, PropertyContextR) override;
    virtual void _GetTypeName(Utf8StringR, uint32_t desiredLength) override;
    virtual StatusInt _OnTransform(EditElementHandleR, TransformInfoCR) override;
    virtual void _QueryProperties(ElementHandleCR, PropertyContextR) override;

    // DisplayHandler
    virtual bool _IsPlanar(ElementHandleCR, DVec3dP, DPoint3dP, DVec3dCP) override;
    virtual void _GetOrientation(ElementHandleCR, RotMatrixR) override;
    virtual void _GetSnapOrigin(ElementHandleCR, DPoint3dR) override;
    virtual void _GetTransformOrigin(ElementHandleCR, DPoint3dR) override;

public:
    static DgnClassId const& GetHandlerId();

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static BentleyStatus UpdateElement(EditElementHandleR, TextAnnotationCR, TransformCR, DgnStyleId);
    DGNPLATFORM_EXPORT static BentleyStatus CreateElement(EditElementHandleR, ElementHandleCP templateEh, TextAnnotationCR, DgnModelR, TransformCR, DgnStyleId);
    DGNPLATFORM_EXPORT static BentleyStatus FromElement(TextAnnotationP, TransformP, DgnStyleId*, ElementHandleCR);

}; // TextAnnotationHandler
#endif

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
