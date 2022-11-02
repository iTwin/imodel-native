/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/LinkElement.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/VolumeElement.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_TableHandler
{
TABLEHANDLER_DEFINE_MEMBERS(Element)
TABLEHANDLER_DEFINE_MEMBERS(Geometric3d)
TABLEHANDLER_DEFINE_MEMBERS(Geometric2d)
TABLEHANDLER_DEFINE_MEMBERS(Model)
TABLEHANDLER_DEFINE_MEMBERS(ElementDep)
};

namespace dgn_ModelHandler
{
HANDLER_DEFINE_MEMBERS(Model)
HANDLER_DEFINE_MEMBERS(Geometric3d)
HANDLER_DEFINE_MEMBERS(Graphical3d)
HANDLER_DEFINE_MEMBERS(Spatial)
HANDLER_DEFINE_MEMBERS(SpatialLocation)
HANDLER_DEFINE_MEMBERS(Physical)
HANDLER_DEFINE_MEMBERS(SectionDrawing)
HANDLER_DEFINE_MEMBERS(Role)
HANDLER_DEFINE_MEMBERS(Information)
HANDLER_DEFINE_MEMBERS(InformationRecord)
HANDLER_DEFINE_MEMBERS(Definition)
HANDLER_DEFINE_MEMBERS(Dictionary)
HANDLER_DEFINE_MEMBERS(DocumentList)
HANDLER_DEFINE_MEMBERS(GroupInformation)
HANDLER_DEFINE_MEMBERS(Geometric2d)
HANDLER_DEFINE_MEMBERS(Drawing)
HANDLER_DEFINE_MEMBERS(Repository)
};

namespace WebMercator
{
HANDLER_DEFINE_MEMBERS(ModelHandler)
};

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(Element)
HANDLER_DEFINE_MEMBERS(Geometric2d)
HANDLER_DEFINE_MEMBERS(Geometric3d)
HANDLER_DEFINE_MEMBERS(Physical)
HANDLER_DEFINE_MEMBERS(SpatialLocation)
HANDLER_DEFINE_MEMBERS(GeometryPart)
HANDLER_DEFINE_MEMBERS(Annotation2d)
HANDLER_DEFINE_MEMBERS(DrawingGraphic)
HANDLER_DEFINE_MEMBERS(Role)
HANDLER_DEFINE_MEMBERS(InformationContent)
HANDLER_DEFINE_MEMBERS(InformationRecord)
HANDLER_DEFINE_MEMBERS(RenderTimeline)
HANDLER_DEFINE_MEMBERS(GroupInformation)
HANDLER_DEFINE_MEMBERS(InformationCarrier)
HANDLER_DEFINE_MEMBERS(Document)
HANDLER_DEFINE_MEMBERS(Drawing)
HANDLER_DEFINE_MEMBERS(SectionDrawing)
HANDLER_DEFINE_MEMBERS(SectionDrawingLocation)
HANDLER_DEFINE_MEMBERS(DriverBundle)
HANDLER_DEFINE_MEMBERS(Definition)
HANDLER_DEFINE_MEMBERS(Category);
HANDLER_DEFINE_MEMBERS(DrawingCategory);
HANDLER_DEFINE_MEMBERS(SpatialCategory);
HANDLER_DEFINE_MEMBERS(SubCategory);
HANDLER_DEFINE_MEMBERS(PhysicalMaterial)
HANDLER_DEFINE_MEMBERS(PhysicalType)
HANDLER_DEFINE_MEMBERS(TemplateRecipe3d)
HANDLER_DEFINE_MEMBERS(GraphicalType2d)
HANDLER_DEFINE_MEMBERS(TemplateRecipe2d)
HANDLER_DEFINE_MEMBERS(SpatialLocationType)
HANDLER_DEFINE_MEMBERS(ColorBook)
HANDLER_DEFINE_MEMBERS(Subject)
HANDLER_DEFINE_MEMBERS(InformationPartition)
HANDLER_DEFINE_MEMBERS(DefinitionPartition)
HANDLER_DEFINE_MEMBERS(DocumentPartition)
HANDLER_DEFINE_MEMBERS(GroupInformationPartition)
HANDLER_DEFINE_MEMBERS(InformationRecordPartition)
HANDLER_DEFINE_MEMBERS(PhysicalPartition)
HANDLER_DEFINE_MEMBERS(SpatialLocationPartition)
HANDLER_DEFINE_MEMBERS(ExternalSource)
HANDLER_DEFINE_MEMBERS(ExternalSourceGroup)
HANDLER_DEFINE_MEMBERS(ExternalSourceAttachment)
HANDLER_DEFINE_MEMBERS(SynchronizationConfigLinkHandler)
};

namespace dgn_AspectHandler
{
HANDLER_DEFINE_MEMBERS(Aspect)
};

namespace dgn_CodeSpecHandler
{
HANDLER_DEFINE_MEMBERS(CodeSpec)
};

END_BENTLEY_DGN_NAMESPACE

DOMAIN_DEFINE_MEMBERS(BisCoreDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BisCoreDomain::BisCoreDomain() : DgnDomain(BIS_ECSCHEMA_NAME, "BIS Core Domain", 1)
    {
    // Note: Handlers must be registered in class heirarchy order (base classes before subclasses)
    RegisterHandler(ElementDependency::Handler::GetHandler());
    RegisterHandler(dgn_AspectHandler::Aspect::GetHandler());
    RegisterHandler(dgn_AspectHandler::TextAnnotationDataHandler::GetHandler());

    RegisterHandler(dgn_ModelHandler::Model::GetHandler());
    RegisterHandler(dgn_ModelHandler::Geometric3d::GetHandler());
    RegisterHandler(dgn_ModelHandler::Graphical3d::GetHandler());
    RegisterHandler(dgn_ModelHandler::Spatial::GetHandler());
    RegisterHandler(dgn_ModelHandler::SpatialLocation::GetHandler());
    RegisterHandler(dgn_ModelHandler::Physical::GetHandler());
    RegisterHandler(dgn_ModelHandler::Geometric2d::GetHandler());
    RegisterHandler(dgn_ModelHandler::Drawing::GetHandler());
    RegisterHandler(Sheet::Handlers::Model::GetHandler());
    RegisterHandler(dgn_ModelHandler::SectionDrawing::GetHandler());
    RegisterHandler(WebMercator::ModelHandler::GetHandler());
    RegisterHandler(dgn_ModelHandler::Role::GetHandler());
    RegisterHandler(dgn_ModelHandler::Information::GetHandler());
    RegisterHandler(dgn_ModelHandler::InformationRecord::GetHandler());
    RegisterHandler(dgn_ModelHandler::Definition::GetHandler());
    RegisterHandler(dgn_ModelHandler::DocumentList::GetHandler());
    RegisterHandler(dgn_ModelHandler::GroupInformation::GetHandler());
    RegisterHandler(dgn_ModelHandler::Link::GetHandler());
    RegisterHandler(dgn_ModelHandler::Dictionary::GetHandler());
    RegisterHandler(dgn_ModelHandler::Repository::GetHandler());

    RegisterHandler(dgn_ElementHandler::Element::GetHandler());
    RegisterHandler(dgn_ElementHandler::InformationContent::GetHandler());
    RegisterHandler(dgn_ElementHandler::InformationRecord::GetHandler());
    RegisterHandler(dgn_ElementHandler::RenderTimeline::GetHandler());
    RegisterHandler(dgn_ElementHandler::GroupInformation::GetHandler());
    RegisterHandler(dgn_ElementHandler::InformationCarrier::GetHandler());
    RegisterHandler(dgn_ElementHandler::Document::GetHandler());
    RegisterHandler(dgn_ElementHandler::Drawing::GetHandler());
    RegisterHandler(dgn_ElementHandler::SectionDrawing::GetHandler());
    RegisterHandler(Sheet::Handlers::Element::GetHandler());
    RegisterHandler(dgn_ElementHandler::DriverBundle::GetHandler());
    RegisterHandler(dgn_ElementHandler::Definition::GetHandler());
    RegisterHandler(dgn_ElementHandler::Geometric2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::Geometric3d::GetHandler());
    RegisterHandler(dgn_ElementHandler::GeometryPart::GetHandler());
    RegisterHandler(dgn_ElementHandler::Physical::GetHandler());
    RegisterHandler(dgn_ElementHandler::SpatialLocation::GetHandler());
    RegisterHandler(dgn_ElementHandler::SectionDrawingLocation::GetHandler());
    RegisterHandler(dgn_ElementHandler::Annotation2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::DrawingGraphic::GetHandler());
    RegisterHandler(dgn_ElementHandler::UrlLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::RepositoryLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::EmbeddedFileLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::VolumeElementHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotation2dHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotation3dHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::PhysicalType::GetHandler());
    RegisterHandler(dgn_ElementHandler::TemplateRecipe3d::GetHandler());
    RegisterHandler(dgn_ElementHandler::GraphicalType2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::TemplateRecipe2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::SpatialLocationType::GetHandler());
    RegisterHandler(dgn_ElementHandler::PhysicalMaterial::GetHandler());
    RegisterHandler(dgn_ElementHandler::RenderMaterial::GetHandler());
    RegisterHandler(dgn_ElementHandler::Texture::GetHandler());
    RegisterHandler(Lighting::Handlers::LightLoc::GetHandler());
    RegisterHandler(dgn_ElementHandler::LineStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::Category::GetHandler());
    RegisterHandler(dgn_ElementHandler::DrawingCategory::GetHandler());
    RegisterHandler(dgn_ElementHandler::SpatialCategory::GetHandler());
    RegisterHandler(dgn_ElementHandler::SubCategory::GetHandler());
    RegisterHandler(dgn_ElementHandler::ColorBook::GetHandler());
    RegisterHandler(dgn_ElementHandler::Subject::GetHandler());
    RegisterHandler(dgn_ElementHandler::Role::GetHandler());
    RegisterHandler(dgn_ElementHandler::InformationPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::DefinitionPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::DocumentPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::GroupInformationPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::InformationRecordPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::ExternalSource::GetHandler());
    RegisterHandler(dgn_ElementHandler::ExternalSourceGroup::GetHandler());
    RegisterHandler(dgn_ElementHandler::ExternalSourceAttachment::GetHandler());
    RegisterHandler(dgn_ElementHandler::SynchronizationConfigLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::LinkPartitionHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::PhysicalPartition::GetHandler());
    RegisterHandler(dgn_ElementHandler::SpatialLocationPartition::GetHandler());

    RegisterHandler(dgn_ElementHandler::AnnotationTextStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationFrameStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationLeaderStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotationSeedHandler::GetHandler());

    RegisterHandler(ViewElementHandler::View::GetHandler());
    RegisterHandler(ViewElementHandler::View2d::GetHandler());
    RegisterHandler(ViewElementHandler::View3d::GetHandler());
    RegisterHandler(ViewElementHandler::TemplateView2d::GetHandler());
    RegisterHandler(ViewElementHandler::TemplateView3d::GetHandler());
    RegisterHandler(ViewElementHandler::SpatialView::GetHandler());
    RegisterHandler(ViewElementHandler::DrawingView::GetHandler());
    RegisterHandler(ViewElementHandler::SheetView::GetHandler());
    RegisterHandler(ViewElementHandler::ViewModels::GetHandler());
    RegisterHandler(ViewElementHandler::ViewCategories::GetHandler());
    RegisterHandler(ViewElementHandler::ViewDisplayStyle::GetHandler());
    RegisterHandler(ViewElementHandler::ViewDisplayStyle2d::GetHandler());
    RegisterHandler(ViewElementHandler::ViewDisplayStyle3d::GetHandler());
    RegisterHandler(ViewElementHandler::OrthographicView::GetHandler());
    RegisterHandler(Sheet::Handlers::AttachmentElement::GetHandler());

    RegisterHandler(ACSElementHandler::CoordSys2d::GetHandler());
    RegisterHandler(ACSElementHandler::CoordSys3d::GetHandler());
    RegisterHandler(ACSElementHandler::CoordSysSpatial::GetHandler());

    RegisterHandler(dgn_CodeSpecHandler::CodeSpec::GetHandler());

    RegisterTableHandler(dgn_TableHandler::Element::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Geometric3d::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Geometric2d::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Model::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ElementDep::GetHandler());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BisCoreDomain::_OnSchemaImported(DgnDbR db) const
    {
    BeAssert(m_createParams != nullptr && "SetCreateParams() before importing the BisCoreDomain");
    db.OnBisCoreSchemaImported(*m_createParams);
    }
