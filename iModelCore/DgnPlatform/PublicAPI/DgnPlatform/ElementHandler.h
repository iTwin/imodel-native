/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    void SetMirrorPlane(RotMatrixCR mirrorPlane) {m_mirrorPlane = mirrorPlane; m_haveMirrorPlane = true;}
};

#define ELEMENTHANDLER_DECLARE_MEMBERS_0(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        protected: uint64_t _ParseRestrictedAction(Utf8CP name) const override {return __classname__::RestrictedAction::Parse(name); }\
        std::type_info const& _ElementType() const override {return typeid(__classname__);}\
        DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,__handlerclass__,__handlersuperclass__,__exporter__) 

// This macro declares the required members for an ElementHandler. It is often the entire contents of an ElementHandler's class declaration.
// @param[in] __ECClassName__ a string with the ECClass this ElementHandler manaages
// @param[in] __classname__ the name of the C++ class (must be a subclass of DgnElement) this ElementHandler creates
// @param[in] __handlerclass__ This ElementHandler's C++ classname
// @param[in] __handlersuperclass__ This ElementHandler's C++ superclass' classname
// @param[in] __exporter__ Macro name that exports this class in its implementing DLL (may be blank)
#define ELEMENTHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        ELEMENTHANDLER_DECLARE_MEMBERS_0(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        private: Dgn::DgnElementP _CreateInstance(Dgn::DgnElement::CreateParams const& params) override {return new __classname__(__classname__::CreateParams(params));}\
        public:

#define ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        ELEMENTHANDLER_DECLARE_MEMBERS_0(__ECClassName__,__classname__,__handlerclass__,__handlersuperclass__,__exporter__) \
        private: Dgn::DgnElementP _CreateInstance(Dgn::DgnElement::CreateParams const& params) override {return nullptr;}\
        public:

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
    struct EXPORT_VTABLE_ATTRIBUTE Element : DgnDomain::Handler
    {
        friend struct Dgn::DgnElement;
        friend struct Dgn::DgnElements;
        DOMAINHANDLER_DECLARE_MEMBERS(BIS_CLASS_Element, Element, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    protected:
        virtual DgnElement* _CreateInstance(DgnElement::CreateParams const& params) {return new DgnElement(params);}
        DGNPLATFORM_EXPORT virtual DgnElementPtr _CreateNewElement(DgnDbR db, ECN::IECInstanceCR, bool ignoreErrors, DgnDbStatus* stat);
        // WIP
        //DGNPLATFORM_EXPORT virtual DgnElementPtr _CreateNewElement(DgnDbR db, JsonValueCR, DgnDbStatus* stat);
        ElementHandlerP _ToElementHandler() override {return this;}
        virtual std::type_info const& _ElementType() const {return typeid(DgnElement);}
        DGNPLATFORM_EXPORT DgnDbStatus _VerifySchema(DgnDomains&) override;
        uint64_t _ParseRestrictedAction(Utf8CP name) const override { return DgnElement::RestrictedAction::Parse(name); }

    public:
        //! This function is called when a handler is registered by its domain in order to allow the handler to refister get and set functions 
        //! for custom-handled properties or validator functions for auto-handled properties.
        //! @note The implementation must also invoke T_Super::_RegisterPropertyAccessors
        //! @see ECSqlClassInfo::RegisterPropertyAccessors, ECSqlClassInfo::RegisterPropertyValidator
        DGNPLATFORM_EXPORT virtual void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR);

        //! Create a new instance of a DgnElement from a CreateParams. 
        //! @note The actual type of the returned DgnElement will depend on the DgnClassId in @a params.
        DgnElementPtr Create(DgnElement::CreateParams const& params) {return (DgnElementP) _CreateInstance(params);}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static ElementHandlerP FindHandler(DgnDb const& dgndb, DgnClassId classId);
    };

    //! The ElementHandler for GeometricElement3d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric3d : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GeometricElement3d, GeometricElement3d, Geometric3d, Element, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The ElementHandler for PhysicalElement
    struct EXPORT_VTABLE_ATTRIBUTE Physical : Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_PhysicalElement, PhysicalElement, Physical, Geometric3d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for SpatialLocationElement
    struct EXPORT_VTABLE_ATTRIBUTE SpatialLocation : Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationElement, SpatialLocationElement, SpatialLocation, Geometric3d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GeometricElement2d
    struct EXPORT_VTABLE_ATTRIBUTE Geometric2d : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GeometricElement2d, GeometricElement2d, Geometric2d, Element, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The ElementHandler for AnnotationElement2d
    struct EXPORT_VTABLE_ATTRIBUTE Annotation2d : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_AnnotationElement2d, AnnotationElement2d, Annotation2d, Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DrawingGraphic
    struct EXPORT_VTABLE_ATTRIBUTE DrawingGraphic : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DrawingGraphic, Dgn::DrawingGraphic, DrawingGraphic, Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for RoleElement
    struct EXPORT_VTABLE_ATTRIBUTE Role : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_RoleElement, RoleElement, Role, Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for InformationContentElement
    struct EXPORT_VTABLE_ATTRIBUTE InformationContent : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationContentElement, InformationContentElement, InformationContent, Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for InformationCarrierElement
    struct EXPORT_VTABLE_ATTRIBUTE InformationCarrier : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationCarrierElement, InformationCarrierElement, InformationCarrier, Element, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for Document
    struct EXPORT_VTABLE_ATTRIBUTE Document : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Document, Dgn::Document, Document, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for Drawing
    struct EXPORT_VTABLE_ATTRIBUTE Drawing : Document
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Drawing, Dgn::Drawing, Drawing, Document, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for SectionDrawing
    struct EXPORT_VTABLE_ATTRIBUTE SectionDrawing : Drawing
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SectionDrawing, Dgn::SectionDrawing, SectionDrawing, Drawing, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for InformationRecordElement
    struct EXPORT_VTABLE_ATTRIBUTE InformationRecord : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationRecordElement, InformationRecordElement, InformationRecord, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DriverBundleElement
    struct EXPORT_VTABLE_ATTRIBUTE DriverBundle : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DriverBundleElement, DriverBundleElement, DriverBundle, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DefinitionElement
    struct EXPORT_VTABLE_ATTRIBUTE Definition : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DefinitionElement, DefinitionElement, Definition, InformationContent, DGNPLATFORM_EXPORT)
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };

    //! The ElementHandler for GroupInformationElement
    struct EXPORT_VTABLE_ATTRIBUTE GroupInformation : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GroupInformationElement, Dgn::GroupInformationElement, GroupInformation, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for PhysicalType
    struct EXPORT_VTABLE_ATTRIBUTE PhysicalType : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_PhysicalType, Dgn::PhysicalType, PhysicalType, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for TemplateRecipe3d
    struct EXPORT_VTABLE_ATTRIBUTE TemplateRecipe3d : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TemplateRecipe3d, Dgn::TemplateRecipe3d, TemplateRecipe3d, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GraphicalType2d
    struct EXPORT_VTABLE_ATTRIBUTE GraphicalType2d : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GraphicalType2d, Dgn::GraphicalType2d, GraphicalType2d, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for TemplateRecipe2d
    struct EXPORT_VTABLE_ATTRIBUTE TemplateRecipe2d : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_TemplateRecipe2d, Dgn::TemplateRecipe2d, TemplateRecipe2d, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for SpatialLocationType
    struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationType : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationType, Dgn::SpatialLocationType, SpatialLocationType, Definition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for Subject
    struct EXPORT_VTABLE_ATTRIBUTE Subject : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Subject, Dgn::Subject, Subject, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for InformationPartitionElement
    struct EXPORT_VTABLE_ATTRIBUTE InformationPartition : InformationContent
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationPartitionElement, InformationPartitionElement, InformationPartition, InformationContent, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DefinitionPartition
    struct EXPORT_VTABLE_ATTRIBUTE DefinitionPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DefinitionPartition, Dgn::DefinitionPartition, DefinitionPartition, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for DocumentPartition
    struct EXPORT_VTABLE_ATTRIBUTE DocumentPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_DocumentPartition, Dgn::DocumentPartition, DocumentPartition, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GroupInformationPartition
    struct EXPORT_VTABLE_ATTRIBUTE GroupInformationPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GroupInformationPartition, Dgn::GroupInformationPartition, GroupInformationPartition, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for InformationRecordPartition
    struct EXPORT_VTABLE_ATTRIBUTE InformationRecordPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_InformationRecordPartition, Dgn::InformationRecordPartition, InformationRecordPartition, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for PhysicalPartition
    struct EXPORT_VTABLE_ATTRIBUTE PhysicalPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_PhysicalPartition, Dgn::PhysicalPartition, PhysicalPartition, InformationPartition, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for SpatialLocationPartition
    struct EXPORT_VTABLE_ATTRIBUTE SpatialLocationPartition : InformationPartition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SpatialLocationPartition, Dgn::SpatialLocationPartition, SpatialLocationPartition, InformationPartition, DGNPLATFORM_EXPORT)
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
        DOMAINHANDLER_DECLARE_MEMBERS(BIS_CLASS_ElementAspect, Aspect, DgnDomain::Handler, DGNPLATFORM_EXPORT)
    protected:
        DGNPLATFORM_EXPORT DgnDbStatus _VerifySchema(DgnDomains&) override;
    public:
        //! The subclass must override this method in order to create an instance using its default constructor.
        //! (The caller will populate and/or persist the returned instance by invoking virtual methods on it.)
        virtual RefCountedPtr<DgnElement::Aspect> _CreateInstance() {return nullptr;}

        //! Find the ElementHandler for a DgnClassId within a supplied DgnDb.
        DGNPLATFORM_EXPORT static Aspect* FindHandler(DgnDbR dgndb, DgnClassId classId);
    };
};

END_BENTLEY_DGN_NAMESPACE
