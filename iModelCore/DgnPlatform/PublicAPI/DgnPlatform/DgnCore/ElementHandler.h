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

BEGIN_BENTLEY_DGN_NAMESPACE

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

// This macro declares the required members for an ElementHandler. It is often the entire contents of an ElementHandler's class declaration.
// @param[in] __ECClassName__ a string with the ECClass this ElementHandler manaages
// @param[in] __classname__ the name of the C++ class (must be a subclass of DgnElement) this ElementHandler creates
// @param[in] __handlerclass__ This ElementHandler's C++ classname
// @param[in] __handlersuperclass__ This ElementHandler's C++ superclass' classname
// @param[in] __exporter__ Macro name that exports this class in its implementing DLL (may be blank)
#define ELEMENTHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        private: virtual Dgn::DgnElementP _CreateInstance(Dgn::DgnElement::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        virtual std::type_info const& _ElementType() const override {return typeid(__classname__);}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,__handlerclass__,__handlersuperclass__,__exporter__) 

//=======================================================================================
//! @namespace BentleyApi::Dgn::dgn_ElementHandler DgnElement Handlers in the base "Dgn" domain. 
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The ElementHandler for DgnElement.
    //! An ElementHandler creates instances of (a subclass of) DgnElement.
    //! @see DgnElement
    //! @ingroup DgnElementGroup
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Element : DgnDomain::Handler
    {
        friend struct Dgn::DgnElement;
        friend struct Dgn::DgnElements;
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Element, Element, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual DgnElement* _CreateInstance(DgnElement::CreateParams const& params) {return new DgnElement(params);}
        virtual ElementHandlerP _ToElementHandler() {return this;}
        virtual std::type_info const& _ElementType() const {return typeid(DgnElement);}
        DGNPLATFORM_EXPORT virtual DgnDbStatus _VerifySchema(DgnDomains&) override;

    public:
        //! Create a new instance of a DgnElement from a CreateParams. 
        //! @note The actual type of the returned DgnElement will depend on the DgnClassId in @a params.
        DgnElementPtr Create(DgnElement::CreateParams const& params) {return (DgnElementP) _CreateInstance(params);}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static ElementHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);
    };

    //! The ElementHandler for PhysicalElement
    struct EXPORT_VTABLE_ATTRIBUTE Physical : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_PhysicalElement, PhysicalElement, Physical, Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DrawingElement
    struct EXPORT_VTABLE_ATTRIBUTE Drawing : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingElement, DrawingElement, Drawing, Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for ElementGroup
    struct EXPORT_VTABLE_ATTRIBUTE Group : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_ElementGroup, ElementGroup, Group, Element, DGNPLATFORM_EXPORT)
    };
};

//=======================================================================================
//! @namespace BentleyApi::Dgn::dgn_AspectHandler Aspect Handlers in the base "Dgn" domain. 
//! @note Only handlers from the base "Dgn" domain belong in this namespace.
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
namespace dgn_AspectHandler
{
    //=======================================================================================
    //! A dgn_AspectHandler::Aspect creates instances of (a subclass of) DgnElement::Aspect.
    //! @see DgnElement
    //! @ingroup DgnElementGroup
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Aspect : DgnDomain::Handler
    {
        friend struct DgnElement;
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_ElementAspect, Aspect, DgnDomain::Handler, DGNPLATFORM_EXPORT)

        //! The subclass must override this method in order to create an instance using its default constructor.
        //! (The caller will populate and/or persist the returned instance by invoking virtual methods on it.)
        virtual RefCountedPtr<DgnElement::Aspect> _CreateInstance() {return nullptr;}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static Aspect* FindHandler(DgnDbR dgndb, DgnClassId classId);
    };
};

END_BENTLEY_DGN_NAMESPACE
