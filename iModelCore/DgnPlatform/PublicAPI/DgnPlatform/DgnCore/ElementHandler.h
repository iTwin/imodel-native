/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#if defined(_MSC_VER)
    #pragma warning(disable:4265) // Class has virtual function, but destructor is not virtual
#endif // defined(_MSC_VER)

#include "DgnDomain.h"
#include "DgnElement.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
//=======================================================================================
//! A transformation matrix. Identifies a few special cases.
// @bsiclass
//=======================================================================================
struct TransformInfo
{
private:
    Transform   m_trans;
    uint32_t    m_options;
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

    //! Set element-specific transform options. @see TransformOptionValues
    void SetOptions(uint32_t options) {m_options = options;}

    //! Check element-specific transform options. @see TransformOptionValues
    uint32_t GetOptions() const {return m_options;}

}; // TransformInfo

/*=================================================================================**//**
  @addtogroup ElementHandler
An Element Handler defines the behavior of the elements that it controls, including how they
display, transform, respond to snaps, and other user interactions.

An element handler is C++ singleton object that is an instance of a class that is derived from Handler.

A Handler's methods normally take an ElementHandle or an EditElementHandle reference as an argument,
which identifies the element that the Handler should process. A Handler has no state of its own.
A Handler is expected to apply its logic to the element that it is asked to process,
based on the state of the element. The Handler design is based on the Flyweight Pattern.

  @bsiclass
*//*+===============+===============+===============+===============+===============+======*/

//__PUBLISH_SECTION_START__
/// @beginGroup
/*=================================================================================**//**
 ElementHandler defines the standard queries and operations available on all elements,
 whether graphical or non-graphical, internal or application defined.
 Every element has a Handler. A handler may have more capabilities
 than what is defined by ElementHandler, but it will have at least these capabilities.
 @see ElementHandle::GetHandler
 @bsiclass                                                     SamWilson       11/04
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ElementHandler : DgnDomain::Handler
{
    HANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Element, ElementHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

public:
    friend struct DgnElement;

protected:
    virtual ElementHandlerP _ToElementHandler() {return this;}
    DGNPLATFORM_EXPORT virtual DgnElementP _CreateInstance(DgnElement::CreateParams const& params);

public:
    DgnElementPtr Create(DgnElement::CreateParams const& params) {return _CreateInstance(params);}

public:
    //! Find the ElemenntHandler for a DgnClassId within a supplied DgnDb.
    DGNPLATFORM_EXPORT static ElementHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);

}; // Handler

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PhysicalElementHandler : ElementHandler
{
    HANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_PhysicalElement, PhysicalElementHandler, ElementHandler, DGNPLATFORM_EXPORT)
protected:
    DGNPLATFORM_EXPORT DgnElementP _CreateInstance(DgnElement::CreateParams const& params) override;
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE DrawingElementHandler : ElementHandler
{
    HANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_DrawingElement, DrawingElementHandler, ElementHandler, DGNPLATFORM_EXPORT)

protected:
    DGNPLATFORM_EXPORT DgnElementP _CreateInstance(DgnElement::CreateParams const& params) override;
};

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
