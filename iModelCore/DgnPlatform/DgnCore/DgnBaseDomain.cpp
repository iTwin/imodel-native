/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnBaseDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/AnnotationTable.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_TableHandler
{
TABLEHANDLER_DEFINE_MEMBERS(Element)
TABLEHANDLER_DEFINE_MEMBERS(Model)
TABLEHANDLER_DEFINE_MEMBERS(ModelDep)
TABLEHANDLER_DEFINE_MEMBERS(ElementDep)
TABLEHANDLER_DEFINE_MEMBERS(BeProperties)
};

namespace dgn_ModelHandler
{
HANDLER_DEFINE_MEMBERS(Model)
HANDLER_DEFINE_MEMBERS(Physical)
HANDLER_DEFINE_MEMBERS(Component)
HANDLER_DEFINE_MEMBERS(PhysicalRedline)
HANDLER_DEFINE_MEMBERS(Sheet)
HANDLER_DEFINE_MEMBERS(Redline)
HANDLER_DEFINE_MEMBERS(PlanarPhysical)
HANDLER_DEFINE_MEMBERS(SectionDrawing)
HANDLER_DEFINE_MEMBERS(StreetMap)
HANDLER_DEFINE_MEMBERS(WebMercator)
HANDLER_DEFINE_MEMBERS(Definition)
HANDLER_DEFINE_MEMBERS(Dictionary)
HANDLER_DEFINE_MEMBERS(Model2d)
HANDLER_DEFINE_MEMBERS(System)
};

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(Element)
HANDLER_DEFINE_MEMBERS(Group)
HANDLER_DEFINE_MEMBERS(Physical)
HANDLER_DEFINE_MEMBERS(Drawing)
};

namespace dgn_AspectHandler
{
HANDLER_DEFINE_MEMBERS(Aspect)
};

namespace dgn_AuthorityHandler
{
HANDLER_DEFINE_MEMBERS(Authority)
HANDLER_DEFINE_MEMBERS(Local)
HANDLER_DEFINE_MEMBERS(Namespace)
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

HANDLER_EXTENSION_DEFINE_MEMBERS(IEditManipulatorExtension)

DOMAIN_DEFINE_MEMBERS(DgnBaseDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseDomain::DgnBaseDomain() : DgnDomain(DGN_ECSCHEMA_NAME, "Base DgnDb Domain",1) 
    {
    // Note: Handlers must be registered in class heiarchy order (base classes before subclasses)
    RegisterHandler(DgnElementDependencyHandler::GetHandler());
    RegisterHandler(dgn_AspectHandler::Aspect::GetHandler());
    RegisterHandler(dgn_AspectHandler::TextAnnotationDataHandler::GetHandler());

    RegisterHandler(dgn_ModelHandler::Model::GetHandler());
    RegisterHandler(dgn_ModelHandler::Physical::GetHandler());
    RegisterHandler(dgn_ModelHandler::Component::GetHandler());
    RegisterHandler(dgn_ModelHandler::Sheet::GetHandler());
    RegisterHandler(dgn_ModelHandler::Model2d::GetHandler());
    RegisterHandler(dgn_ModelHandler::PlanarPhysical::GetHandler());
    RegisterHandler(dgn_ModelHandler::SectionDrawing::GetHandler());
    RegisterHandler(dgn_ModelHandler::Redline::GetHandler());
    RegisterHandler(dgn_ModelHandler::PhysicalRedline::GetHandler());
    RegisterHandler(dgn_ModelHandler::WebMercator::GetHandler());
    RegisterHandler(dgn_ModelHandler::StreetMap::GetHandler());
    RegisterHandler(dgn_ModelHandler::Definition::GetHandler());
    RegisterHandler(dgn_ModelHandler::Dictionary::GetHandler());
    RegisterHandler(dgn_ModelHandler::System::GetHandler());

    RegisterHandler(dgn_ElementHandler::Element::GetHandler());
    RegisterHandler(dgn_ElementHandler::Physical::GetHandler());
    RegisterHandler(dgn_ElementHandler::Drawing::GetHandler());
    RegisterHandler(dgn_ElementHandler::Group::GetHandler());
    RegisterHandler(dgn_ElementHandler::VolumeElementHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotationHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::PhysicalTextAnnotationHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationTableHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::Material::GetHandler());
    RegisterHandler(dgn_ElementHandler::Texture::GetHandler());
    RegisterHandler(dgn_ElementHandler::LightDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::Category::GetHandler());
    RegisterHandler(dgn_ElementHandler::SubCategory::GetHandler());
    RegisterHandler(dgn_ElementHandler::TrueColor::GetHandler());

    RegisterHandler(dgn_ElementHandler::AnnotationTextStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationFrameStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationLeaderStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotationSeedHandler::GetHandler());

    RegisterHandler(dgn_ElementHandler::PhysicalViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::CameraViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::DrawingViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::SheetViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::RedlineViewDef::GetHandler());

    RegisterHandler(dgn_AuthorityHandler::Authority::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Local::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Namespace::GetHandler());

    RegisterTableHandler(dgn_TableHandler::Element::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Model::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ModelDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ElementDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::BeProperties::GetHandler());
    }

