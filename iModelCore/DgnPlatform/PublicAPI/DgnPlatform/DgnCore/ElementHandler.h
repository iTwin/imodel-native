/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A transformation matrix. Identifies a few special cases.
// @bsiclass
//=======================================================================================
struct TransformInfo
{
private:
    Transform   m_trans;
    bool        m_haveMirrorPlane;
    RotMatrix   m_mirrorPlane;

public:
    //! Initialize to the identity transform
    DGNPLATFORM_EXPORT TransformInfo();

    //! Initialize to the specified transform
    DGNPLATFORM_EXPORT explicit TransformInfo(TransformCR t);

    //! Get pointer to the transformation matrix
    TransformCP GetTransform() const {return &m_trans;}

    //! Get a reference to the transformation matrix
    TransformR GetTransformR() {return m_trans;}

    //! If this is a mirroring transform, get the plane across which the element is to be mirrored.
    RotMatrixCP GetMirrorPlane() const {return m_haveMirrorPlane ? &m_mirrorPlane : NULL;}

    //! If this is a mirroring transform, set the plane across which the element is to be mirrored.
    void SetMirrorPlane(RotMatrixCR mirrorPlane) {m_mirrorPlane = mirrorPlane, m_haveMirrorPlane = true;}
};

//! This macro us usually the entire contents of an ElementHandler class declaration.
//! @param[in] __ECClassName__ the name of the ECClass this 
#define ELEMENTHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        private: virtual DgnElementP _CreateInstance(DgnPlatform::DgnElement::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,__handlerclass__,__handlersuperclass__,__exporter__) 

/*=================================================================================**//**
//! An ElementHandler supplies instances of (a subclass of) DgnElement.
 @bsiclass                                                     SamWilson       11/04
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ElementHandler : DgnDomain::Handler
{
    friend struct DgnElement;
    DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Element, ElementHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

private: 
    virtual DgnElement* _CreateInstance(DgnElement::CreateParams const& params) {return new DgnElement(params);}

protected:
    virtual ElementHandlerP _ToElementHandler() {return this;}

public:
    //! Create a new instance of a DgnElement from a CreateParams. 
    //! @note The actual type of the returned DgnElement will depend on the DgnClassId in @a params.
    DgnElementPtr Create(DgnElement::CreateParams const& params) {return (DgnElementP) _CreateInstance(params);}

    //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
    DGNPLATFORM_EXPORT static ElementHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElementHandler : ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalElement, PhysicalElement, PhysicalElementHandler, ElementHandler, DGNPLATFORM_EXPORT)
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DrawingElementHandler : ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingElement, DrawingElement, DrawingElementHandler, ElementHandler, DGNPLATFORM_EXPORT)
};

/*=================================================================================**//**
* @bsiclass                                                     Shaun.Sewall    05/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ElementGroupHandler : ElementHandler
{
    ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_ElementGroup, ElementGroup, ElementGroupHandler, ElementHandler, DGNPLATFORM_EXPORT)
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
