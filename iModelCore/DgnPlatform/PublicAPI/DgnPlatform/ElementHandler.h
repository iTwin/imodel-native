/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ElementHandler.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDomain.h"
#include "DgnElement.h"
#include "ECSqlClassParams.h"

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
        protected: virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override {return __classname__::RestrictedAction::Parse(name); }\
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
    //! @ingroup GROUP_DgnElement
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Element : DgnDomain::Handler, TEMPORARY_IECSqlClassParamsAutoHandlerInfoProvider
    {
        friend struct Dgn::DgnElement;
        friend struct Dgn::DgnElements;
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Element, Element, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual DgnElement* _CreateInstance(DgnElement::CreateParams const& params) {return new DgnElement(params);}
        virtual ElementHandlerP _ToElementHandler() {return this;}
        virtual std::type_info const& _ElementType() const {return typeid(DgnElement);}
        DGNPLATFORM_EXPORT virtual DgnDbStatus _VerifySchema(DgnDomains&) override;
        virtual uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnElement::RestrictedAction::Parse(name); }

        //! Add the names of any subclass properties used by ECSql INSERT, UPDATE, and/or SELECT statements to the ECSqlClassParams list.
        //! If you override this method, you @em must invoke T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes().
        DGNPLATFORM_EXPORT virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) override; // *** WIP_AUTO_HANDLED_PROPERTIES
    public:
        void TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) {return _TEMPORARY_GetPropertyHandlingCustomAttributes(params);} // *** WIP_AUTO_HANDLED_PROPERTIES

        //! Create a new instance of a DgnElement from a CreateParams. 
        //! @note The actual type of the returned DgnElement will depend on the DgnClassId in @a params.
        DgnElementPtr Create(DgnElement::CreateParams const& params) {return (DgnElementP) _CreateInstance(params);}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static ElementHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);
    };

    //! The ElementHandler for GeometricElement3d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric3d : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_GeometricElement3d, GeometricElement3d, Geometric3d, Element, DGNPLATFORM_EXPORT)
        virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) override { T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params); GeometricElement3d::_TEMPORARY_GetPropertyHandlingCustomAttributes(params); } // *** WIP_AUTO_HANDLED_PROPERTIES
    };

    //! The ElementHandler for GeometricElement2d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric2d : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_GeometricElement2d, GeometricElement2d, Geometric2d, Element, DGNPLATFORM_EXPORT)
        virtual void _TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) override { T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params); GeometricElement2d::_TEMPORARY_GetPropertyHandlingCustomAttributes(params); } // *** WIP_AUTO_HANDLED_PROPERTIES
    };

    //! The ElementHandler for AnnotationElement2d
    struct EXPORT_VTABLE_ATTRIBUTE Annotation2d : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_AnnotationElement2d, AnnotationElement2d, Annotation2d, Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DrawingGraphic
    struct EXPORT_VTABLE_ATTRIBUTE DrawingGraphic : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_DrawingGraphic, Dgn::DrawingGraphic, DrawingGraphic, Geometric2d, DGNPLATFORM_EXPORT)
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
    //! @ingroup GROUP_DgnElement
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE Aspect : DgnDomain::Handler
    {
        friend struct DgnElement;
        DOMAINHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_ElementAspect, Aspect, DgnDomain::Handler, DGNPLATFORM_EXPORT)
    protected:
        DGNPLATFORM_EXPORT virtual DgnDbStatus _VerifySchema(DgnDomains&) override;
    public:
        //! The subclass must override this method in order to create an instance using its default constructor.
        //! (The caller will populate and/or persist the returned instance by invoking virtual methods on it.)
        virtual RefCountedPtr<DgnElement::Aspect> _CreateInstance() {return nullptr;}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static Aspect* FindHandler(DgnDbR dgndb, DgnClassId classId);
    };
};

END_BENTLEY_DGN_NAMESPACE
