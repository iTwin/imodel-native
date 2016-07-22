/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnBaseDomain.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/LinkElement.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/AnnotationTable.h>
#if defined (NEEDSWORK_DIMENSION)
#include <DgnPlatform/Dimension/Dimension.h>
#endif
#include <DgnPlatform/VolumeElement.h>

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
HANDLER_DEFINE_MEMBERS(Spatial)
HANDLER_DEFINE_MEMBERS(Component)
HANDLER_DEFINE_MEMBERS(Sheet)
HANDLER_DEFINE_MEMBERS(SectionDrawing)
HANDLER_DEFINE_MEMBERS(Definition)
HANDLER_DEFINE_MEMBERS(Dictionary)
HANDLER_DEFINE_MEMBERS(Geometric2d)
HANDLER_DEFINE_MEMBERS(GroupInformation)
HANDLER_DEFINE_MEMBERS(Functional)
};

namespace WebMercator
{
HANDLER_DEFINE_MEMBERS(ModelHandler)
HANDLER_DEFINE_MEMBERS(StreetMapHandler)
};

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(Element)
HANDLER_DEFINE_MEMBERS(Geometric2d)
HANDLER_DEFINE_MEMBERS(Geometric3d)
HANDLER_DEFINE_MEMBERS(GeometryPart)
HANDLER_DEFINE_MEMBERS(Annotation2d)
HANDLER_DEFINE_MEMBERS(DrawingGraphic)
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
HANDLER_DEFINE_MEMBERS(Material)
HANDLER_DEFINE_MEMBERS(Component)
HANDLER_DEFINE_MEMBERS(Model)
HANDLER_DEFINE_MEMBERS(TrueColor)
HANDLER_DEFINE_MEMBERS(Resource)
HANDLER_DEFINE_MEMBERS(Category)
HANDLER_DEFINE_MEMBERS(GeometryPart)
};

END_BENTLEY_DGN_NAMESPACE

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
    RegisterHandler(dgn_ModelHandler::Spatial::GetHandler());
    RegisterHandler(dgn_ModelHandler::Component::GetHandler());
    RegisterHandler(dgn_ModelHandler::Geometric2d::GetHandler());
    RegisterHandler(dgn_ModelHandler::Sheet::GetHandler());
    RegisterHandler(dgn_ModelHandler::SectionDrawing::GetHandler());
    RegisterHandler(WebMercator::ModelHandler::GetHandler());
    RegisterHandler(WebMercator::StreetMapHandler::GetHandler());
    RegisterHandler(dgn_ModelHandler::Definition::GetHandler());
    RegisterHandler(dgn_ModelHandler::Link::GetHandler());
    RegisterHandler(dgn_ModelHandler::Dictionary::GetHandler());
    RegisterHandler(dgn_ModelHandler::GroupInformation::GetHandler());
    RegisterHandler(dgn_ModelHandler::Functional::GetHandler());

    RegisterHandler(dgn_ElementHandler::Element::GetHandler());
    RegisterHandler(dgn_ElementHandler::Geometric2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::Geometric3d::GetHandler());
    RegisterHandler(dgn_ElementHandler::GeometryPart::GetHandler());
    RegisterHandler(dgn_ElementHandler::Annotation2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::DrawingGraphic::GetHandler());
    RegisterHandler(dgn_ElementHandler::UrlLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::EmbeddedFileLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::VolumeElementHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotation2dHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotation3dHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationTableHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::Material::GetHandler());
    RegisterHandler(dgn_ElementHandler::Texture::GetHandler());
    RegisterHandler(dgn_ElementHandler::LightDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::LineStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::Category::GetHandler());
    RegisterHandler(dgn_ElementHandler::SubCategory::GetHandler());
    RegisterHandler(dgn_ElementHandler::TrueColor::GetHandler());

#if defined (NEEDSWORK_DIMENSION)
    RegisterHandler(dgn_ElementHandler::DimensionStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::LinearDimensionHandler2d::GetHandler());
    RegisterHandler(dgn_ElementHandler::LinearDimensionHandler3d::GetHandler());
#endif

    RegisterHandler(dgn_ElementHandler::AnnotationTextStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationFrameStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::AnnotationLeaderStyleHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::TextAnnotationSeedHandler::GetHandler());

    RegisterHandler(dgn_ElementHandler::SpatialViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::CameraViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::DrawingViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::SheetViewDef::GetHandler());

    RegisterHandler(dgn_ElementHandler::ViewAttachmentHandler::GetHandler());

    RegisterHandler(dgn_AuthorityHandler::Authority::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Local::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Namespace::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Material::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Component::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Model::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::TrueColor::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Resource::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::Category::GetHandler());
    RegisterHandler(dgn_AuthorityHandler::GeometryPart::GetHandler());

    RegisterTableHandler(dgn_TableHandler::Element::GetHandler());
    RegisterTableHandler(dgn_TableHandler::Model::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ModelDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::ElementDep::GetHandler());
    RegisterTableHandler(dgn_TableHandler::BeProperties::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnBaseDomain::AddMissingCustomAttributes(DgnDbR db) const
    {
    if (!CustomPropertyRegistry::HasOldDgnSchema(db))
        return;

    CustomPropertyRegistry prop;
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element);
    prop.Register("ModelId", ECSqlClassParams::StatementType::Insert);
    prop.Register("Code", ECSqlClassParams::StatementType::InsertUpdate);
    prop.Register("Label", ECSqlClassParams::StatementType::InsertUpdate);
    prop.Register("ParentId", ECSqlClassParams::StatementType::InsertUpdate);
    prop.Register("UserProperties", ECSqlClassParams::StatementType::None);
    prop.Register("LastMod", ECSqlClassParams::StatementType::None);
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Category);
    prop.Register("Descr");
    prop.Register("Scope");
    prop.Register("Rank");
    prop.Register("LastMod", ECSqlClassParams::StatementType::None);
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SubCategory);
    prop.Register("Descr");
    prop.Register("Properties");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricElement2d);
    prop.Register("CategoryId");
    prop.Register("Origin");
    prop.Register("BBoxLow");
    prop.Register("BBoxHigh");
    prop.Register("GeometryStream");
    prop.Register("Rotation");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricElement3d);
    prop.Register("CategoryId");
    prop.Register("Origin");
    prop.Register("BBoxLow");
    prop.Register("BBoxHigh");
    prop.Register("GeometryStream");
    prop.Register("InSpatialIndex");
    prop.Register("Yaw");
    prop.Register("Pitch");
    prop.Register("Roll");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationFrameStyle);
    prop.Register("Data");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationLeaderStyle);
    prop.Register("Data");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_AnnotationTextStyle);
    prop.Register("Data");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TextAnnotationSeed);
    prop.Register("Data");
    prop.Register("Descr");
    /*
    prop.SetClass(db, DGN_ECSCHEMA_NAME, "DimensionStyle");
    prop.Register("TextStyleId");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, "LinearDimension");
    prop.Register("StyleId");
    prop.Register("Points");
    */
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_LineStyle);
    prop.Register("Data");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_TrueColor);
    prop.Register("Data", ECSqlClassParams::StatementType::ReadOnly);
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_LightDefinition);
    prop.Register("Descr");
    prop.Register("Value");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_MaterialElement);
    prop.Register("Descr");
    prop.Register("Data");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Texture);
    prop.Register("Data");
    prop.Register("Descr");
    prop.Register("Format");
    prop.Register("Width");
    prop.Register("Height");
    prop.Register("Flags");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ViewDefinition);
    prop.Register("Descr");
    prop.Register("Source");
    prop.Register("BaseModelId");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometryPart);
    prop.Register("GeometryStream");
    prop.Register("BBoxLow");
    prop.Register("BBoxHigh");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_UrlLink);
    prop.Register("Url");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_EmbeddedFileLink);
    prop.Register("Name");
    prop.Register("Descr");
    prop.SetClass(db, DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ViewAttachment);
    prop.Register("ViewId");
    prop.Register("Scale");
    }
