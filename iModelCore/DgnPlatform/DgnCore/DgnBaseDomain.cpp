/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnBaseDomain.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <DgnPlatform/DgnCore/Annotations/TextAnnotationElement.h>

BEGIN_BENTLEY_DGN_NAMESPACE

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
HANDLER_DEFINE_MEMBERS(Graphics2d)
HANDLER_DEFINE_MEMBERS(PlanarPhysical)
HANDLER_DEFINE_MEMBERS(SectionDrawing)
HANDLER_DEFINE_MEMBERS(StreetMap)
HANDLER_DEFINE_MEMBERS(WebMercator)
HANDLER_DEFINE_MEMBERS(PointCloud)
HANDLER_DEFINE_MEMBERS(Raster)
HANDLER_DEFINE_MEMBERS(Resource)
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

END_BENTLEY_DGN_NAMESPACE

HANDLER_DEFINE_MEMBERS(ViewHandler)

HANDLER_EXTENSION_DEFINE_MEMBERS(IEditManipulatorExtension)
HANDLER_EXTENSION_DEFINE_MEMBERS(ViewHandlerOverride)

DOMAIN_DEFINE_MEMBERS(DgnBaseDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnBaseDomain::DgnBaseDomain() : DgnDomain(DGN_ECSCHEMA_NAME, "Base DgnDb Domain",1) 
    {
    // Note: Handlers must be registered in class heiarchy order (base classes before subclasses)
    RegisterHandler(ViewHandler::GetHandler());
    RegisterHandler(DgnElementDependencyHandler::GetHandler());
    RegisterHandler(dgn_AspectHandler::Aspect::GetHandler());

    RegisterHandler(dgn_ModelHandler::Model::GetHandler());
    RegisterHandler(dgn_ModelHandler::Physical::GetHandler());
    RegisterHandler(dgn_ModelHandler::Component::GetHandler());
    RegisterHandler(dgn_ModelHandler::Sheet::GetHandler());
    RegisterHandler(dgn_ModelHandler::Graphics2d::GetHandler());
    RegisterHandler(dgn_ModelHandler::PlanarPhysical::GetHandler());
    RegisterHandler(dgn_ModelHandler::SectionDrawing::GetHandler());
    RegisterHandler(dgn_ModelHandler::Redline::GetHandler());
    RegisterHandler(dgn_ModelHandler::PhysicalRedline::GetHandler());
    RegisterHandler(dgn_ModelHandler::WebMercator::GetHandler());
    RegisterHandler(dgn_ModelHandler::StreetMap::GetHandler());
    RegisterHandler(dgn_ModelHandler::PointCloud::GetHandler());
    RegisterHandler(dgn_ModelHandler::Raster::GetHandler());
    RegisterHandler(dgn_ModelHandler::Resource::GetHandler());

    RegisterHandler(dgn_ElementHandler::Element::GetHandler());
    RegisterHandler(dgn_ElementHandler::Physical::GetHandler());
    RegisterHandler(dgn_ElementHandler::Drawing::GetHandler());
    RegisterHandler(dgn_ElementHandler::Group::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotation::GetHandler());
    RegisterHandler(dgn_ElementHandler::PhysicalTextAnnotation::GetHandler());

    RegisterHandler(dgn_AuthorityHandler::Authority::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Local::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Namespace::GetHandler());

    RegisterTableHandler(dgn_TableHandler::Element::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Model::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ModelDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ElementDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::BeProperties::GetHandler());
    }
