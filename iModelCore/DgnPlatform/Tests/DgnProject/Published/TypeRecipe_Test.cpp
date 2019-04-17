/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

#define JSON_NAMESPACE_TypeTests        "TypeTests"
#define JSON_PROP_IsInstanceSpecific    "IsInstanceSpecific"

USING_NAMESPACE_BENTLEY_DPTEST

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    02/2017
//========================================================================================
struct TypeTests : public DgnDbTestFixture
{
    static const bool CAPPED = true;
    static const bool GEOMETRY3D = true;
    static const bool GEOMETRY2D = false;

    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, DgnBoxDetailCR);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, DgnConeDetailCR);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, DgnSphereDetailCR);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, DgnTorusPipeDetailCR);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, DEllipse3dCR, bool);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, ICurvePrimitiveCR, bool);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, CurveVectorCR, bool);
    DgnGeometryPartCPtr InsertGeometryPart(DefinitionModelR, Utf8StringCR, GeometricPrimitiveCR, bool);

    TemplateRecipe2dCPtr InsertTemplateRecipe2d(DefinitionModelR, Utf8CP);
    GraphicalType2dCPtr InsertType2d(DefinitionModelR, Utf8CP, TemplateRecipe2dCR);

    DrawingModelPtr InsertRectangleAndLinesTemplate2d(TemplateRecipe2dCR);
    DrawingModelPtr InsertCircleAndCrossTemplate2d(TemplateRecipe2dCR, GraphicalType2dCR);
    DrawingModelPtr InsertCircleTemplate2d(TemplateRecipe2dCR);
    DrawingModelPtr InsertTriangleTemplate2d(TemplateRecipe2dCR);
    DrawingModelPtr InsertRectangleTemplate2d(TemplateRecipe2dCR);

    DrawingGraphicPtr CreateTextElement2d(DrawingModelR, DgnSubCategoryId, DPoint2dCR, double, Utf8CP, bool isExpression=false);
    DrawingGraphicPtr CreateGeometricElement2d(DrawingModelR, DgnSubCategoryId, DgnGeometryPartId, DPoint2dCR, AngleInDegreesCR rotation=AngleInDegrees());
    DrawingGraphicPtr CreateGeometricElement2d(DrawingModelR, DgnCategoryId, GeometryBuilderR);
    DrawingGraphicPtr CreateDrawingGraphic(DrawingModelR, GraphicalType2dCR, DPoint2dCR, AngleInDegreesCR rotation=AngleInDegrees(), DgnSubCategoryId subCategoryOverride=DgnSubCategoryId(), Utf8CP userLabel=nullptr);
    
    TemplateRecipe3dCPtr InsertTemplateRecipe3d(DefinitionModelR, Utf8CP);
    PhysicalTypeCPtr InsertType3d(DefinitionModelR, Utf8CP, TemplateRecipe3dCR);

    PhysicalModelPtr InsertTorusPipeTemplate(TemplateRecipe3dCR);
    PhysicalModelPtr InsertCubeAndCylindersTemplate(TemplateRecipe3dCR);
    PhysicalModelPtr InsertThreeSpheresTemplate(TemplateRecipe3dCR);
    PhysicalModelPtr InsertSlabAndColumnsTemplate(TemplateRecipe3dCR);

    GeometricElement3dPtr CreateGeometricElement3d(GeometricModel3dR, DgnSubCategoryId, DgnGeometryPartId, DPoint3dCR, YawPitchRollAnglesCR angles=YawPitchRollAngles());
    GeometricElement3dPtr CreateGeometricElement3d(GeometricModel3dR, GeometryCollection::Iterator const&);
    GeometricElement3dPtr CreateGeometricElement3d(GeometricModel3dR, DgnCategoryId, GeometryBuilderR);
    PhysicalElementPtr CreatePhysicalObject(PhysicalModelR, PhysicalTypeCR, DPoint3dCR, YawPitchRollAnglesCR, Utf8CP userLabel=nullptr);
    DgnDbStatus InstantiateTemplate3d(PhysicalModelR, PhysicalTypeCR, GenericGroupModelP, DPoint3dCR, YawPitchRollAnglesCR);
    DgnDbStatus DropSpatialElementToGeometry(SpatialModelR, SpatialElementCR);

    DgnCategoryId DetermineCategoryId(TypeDefinitionElementCR);     // Only valid for types that have templates that resolve to a single element

    ElementIterator MakeElementIteratorWhereModel(Utf8CP, DgnModelCR);
    bool IsInstanceSpecific(DgnElementCR);
    void SetInstanceSpecific(DgnElementR, bool);
    DgnViewId InsertSpatialView(SpatialModelR, Utf8CP);
    DgnViewId InsertTemplateView2d(DefinitionModelR, Utf8CP);
    DgnViewId InsertTemplateView3d(DefinitionModelR, Utf8CP);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
TemplateRecipe2dCPtr TypeTests::InsertTemplateRecipe2d(DefinitionModelR model, Utf8CP recipeName)
    {
    TemplateRecipe2dPtr recipe = TemplateRecipe2d::Create(model, recipeName);
    BeAssert(recipe.IsValid());
    return recipe.IsValid() ? model.GetDgnDb().Elements().Insert<TemplateRecipe2d>(*recipe) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
TemplateRecipe3dCPtr TypeTests::InsertTemplateRecipe3d(DefinitionModelR model, Utf8CP recipeName)
    {
    TemplateRecipe3dPtr recipe = TemplateRecipe3d::Create(model, recipeName);
    BeAssert(recipe.IsValid());
    return recipe.IsValid() ? model.GetDgnDb().Elements().Insert<TemplateRecipe3d>(*recipe) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertRectangleAndLinesTemplate2d(TemplateRecipe2dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double width = 0.2;
    Utf8PrintfString rectanglePartName("%s-Rectangle", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr rectanglePart = InsertGeometryPart(*definitionModel, rectanglePartName, *ICurvePrimitive::CreateRectangle(-width/2, -width/2, width/2, width/2, 0), GEOMETRY2D);
    if (!rectanglePart.IsValid())
        return nullptr;

    bvector<DSegment3d> segments;
    segments.push_back(DSegment3d::From(DPoint2d::From(-width/2, -width/2), DPoint2d::From(width/2, width/2)));
    segments.push_back(DSegment3d::From(DPoint2d::From(-width/2, width/2), DPoint2d::From(width/2, -width/2)));
    Utf8PrintfString linesPartName("%s-Lines", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr linesPart = InsertGeometryPart(*definitionModel, linesPartName, *CurveVector::Create(segments), GEOMETRY2D);
    if (!linesPart.IsValid())
        return nullptr;

    DrawingModelPtr model = DrawingModel::Create(recipe);
    if (!model.IsValid() || (DgnDbStatus::Success != model->Insert()) || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(db, "RedCategory2d");
    DgnSubCategoryId rectangleSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Rectangles", ColorDef::Red());
    DgnSubCategoryId lineSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Lines", ColorDef::DarkRed());

    DrawingGraphicPtr rectangle = CreateGeometricElement2d(*model, rectangleSubCategoryId, rectanglePart->GetId(), DPoint2d::FromZero());
    DrawingGraphicPtr lines = CreateGeometricElement2d(*model, lineSubCategoryId, linesPart->GetId(), DPoint2d::FromZero());
    if (!rectangle.IsValid() || !lines.IsValid())
        return nullptr;

    if (!rectangle->Insert().IsValid() || !lines->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertCircleAndCrossTemplate2d(TemplateRecipe2dCR recipe, GraphicalType2dCR nestedType)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double radius = 1.0;
    Utf8PrintfString circlePartName("%s-Circle", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr circlePart = InsertGeometryPart(*definitionModel, circlePartName, DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), radius), GEOMETRY2D);
    if (!circlePart.IsValid())
        return nullptr;

    bvector<DSegment3d> segments;
    segments.push_back(DSegment3d::From(DPoint2d::From(-radius, 0), DPoint2d::From(radius, 0)));
    segments.push_back(DSegment3d::From(DPoint2d::From(0, -radius), DPoint2d::From(0, radius)));
    Utf8PrintfString crossPartName("%s-Cross", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr crossPart = InsertGeometryPart(*definitionModel, crossPartName, *CurveVector::Create(segments), GEOMETRY2D);
    if (!crossPart.IsValid())
        return nullptr;

    DrawingModelPtr model = DrawingModel::Create(recipe);
    if (!model.IsValid() || (DgnDbStatus::Success != model->Insert()) || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(db, "BlueCategory2d");
    DgnSubCategoryId lineSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Lines", ColorDef::Blue());
    DgnSubCategoryId circleSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Circles", ColorDef::DarkBlue());
    DgnSubCategoryId fieldSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Fields", ColorDef::Blue());

    DrawingGraphicPtr circle = CreateGeometricElement2d(*model, circleSubCategoryId, circlePart->GetId(), DPoint2d::FromZero());
    DrawingGraphicPtr cross = CreateGeometricElement2d(*model, lineSubCategoryId, crossPart->GetId(), DPoint2d::FromZero());
    DrawingGraphicPtr field = CreateTextElement2d(*model, fieldSubCategoryId, DPoint2d::From(0, 1.5*radius), radius/4, "UserLabel", true);
    DrawingGraphicPtr nested1 = CreateDrawingGraphic(*model, nestedType, DPoint2d::From(-1.2*radius, 0), AngleInDegrees(), lineSubCategoryId);
    DrawingGraphicPtr nested2 = CreateDrawingGraphic(*model, nestedType, DPoint2d::From(0, -1.2*radius), AngleInDegrees(), lineSubCategoryId);
    DrawingGraphicPtr nested3 = CreateDrawingGraphic(*model, nestedType, DPoint2d::From(1.2*radius, 0), AngleInDegrees(), lineSubCategoryId);
    if (!cross.IsValid() || !circle.IsValid() || !field.IsValid() || !nested1.IsValid() || !nested2.IsValid() || !nested3.IsValid())
        return nullptr;

    if (!cross->Insert().IsValid() || !circle->Insert().IsValid() || !field->Insert().IsValid() || !nested1->Insert().IsValid() || !nested2->Insert().IsValid() || !nested3->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertCircleTemplate2d(TemplateRecipe2dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double radius = 1.0;
    Utf8PrintfString circlePartName("%s-Circle", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr circlePart = InsertGeometryPart(*definitionModel, circlePartName, DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), radius), GEOMETRY2D);
    if (!circlePart.IsValid())
        return nullptr;

    DrawingModelPtr model = DrawingModel::Create(recipe);
    if (!model.IsValid() || (DgnDbStatus::Success != model->Insert()) || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(db, "BlueCategory2d");
    DgnSubCategoryId circleSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Circles", ColorDef::DarkBlue());
    DrawingGraphicPtr circle = CreateGeometricElement2d(*model, circleSubCategoryId, circlePart->GetId(), DPoint2d::FromZero());
    if (!circle.IsValid() || !circle->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertTriangleTemplate2d(TemplateRecipe2dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double length = 1.0;
    Utf8PrintfString trianglePartName("%s-Triangle", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr trianglePart = InsertGeometryPart(*definitionModel, trianglePartName, *ICurvePrimitive::CreateRegularPolygonXY(DPoint3d::FromZero(), length, 3, true), GEOMETRY2D);
    if (!trianglePart.IsValid())
        return nullptr;

    DrawingModelPtr model = DrawingModel::Create(recipe);
    if (!model.IsValid() || (DgnDbStatus::Success != model->Insert()) || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(db, "GreenCategory2d");
    DgnSubCategoryId triangleSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Triangles", ColorDef::Green());
    DrawingGraphicPtr triangle = CreateGeometricElement2d(*model, triangleSubCategoryId, trianglePart->GetId(), DPoint2d::FromZero());
    if (!triangle.IsValid() || !triangle->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertRectangleTemplate2d(TemplateRecipe2dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double width = 1.0;
    Utf8PrintfString rectanglePartName("%s-Rectangle", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr rectanglePart = InsertGeometryPart(*definitionModel, rectanglePartName, *ICurvePrimitive::CreateRectangle(-width/2, -width/2, width/2, width/2, 0), GEOMETRY2D);
    if (!rectanglePart.IsValid())
        return nullptr;

    DrawingModelPtr model = DrawingModel::Create(recipe);
    if (!model.IsValid() || (DgnDbStatus::Success != model->Insert()) || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(db, "RedCategory2d");
    DgnSubCategoryId rectangleSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Rectangles", ColorDef::Red());
    DrawingGraphicPtr rectangle = CreateGeometricElement2d(*model, rectangleSubCategoryId, rectanglePart->GetId(), DPoint2d::FromZero());
    if (!rectangle.IsValid() || !rectangle->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTextElement2d(DrawingModelR model, DgnSubCategoryId subCategoryId, DPoint2dCR origin, double textHeight, Utf8CP text, bool isExpression)
    {
    TextStringPtr textString = TextString::Create();
    textString->SetOrigin(DPoint3d::From(origin));
    textString->GetStyleR().SetSize(textHeight);
    textString->GetStyleR().SetFont(DgnFontManager::GetAnyLastResortFont());
    textString->SetText(text);

    DgnDbR db = model.GetDgnDb();
    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(db, subCategoryId);
    if (!categoryId.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint2d::FromZero());
    if (!builder.IsValid() || !builder->Append(subCategoryId) || !builder->Append(*textString))
        return nullptr;

    DrawingGraphicPtr element = CreateGeometricElement2d(model, categoryId, *builder);
    if (!element.IsValid())
        return nullptr;

    if (isExpression)
        SetInstanceSpecific(*element, true);

    return element;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateGeometricElement2d(DrawingModelR model, DgnSubCategoryId subCategoryId, DgnGeometryPartId geometryPartId, DPoint2dCR origin, AngleInDegreesCR rotation)
    {
    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(model.GetDgnDb(), subCategoryId);
    if (!categoryId.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, origin, rotation);
    if (!builder.IsValid() || !builder->Append(subCategoryId) || !builder->Append(geometryPartId, DPoint2d::FromZero()))
        return nullptr;

    return CreateGeometricElement2d(model, categoryId, *builder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateGeometricElement2d(DrawingModelR model, DgnCategoryId categoryId, GeometryBuilderR builder)
    {
    DrawingGraphicPtr element = DrawingGraphic::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    return (BentleyStatus::SUCCESS == builder.Finish(*element)) ? element : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GraphicalType2dCPtr TypeTests::InsertType2d(DefinitionModelR typeModel, Utf8CP typeName, TemplateRecipe2dCR recipe)
    {
    TestGraphicalType2dPtr type = TestGraphicalType2d::Create(typeModel, typeName);
    if (!type.IsValid())
        return nullptr;

    type->SetRecipe(recipe.GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_GraphicalType2dHasTemplateRecipe));
    return recipe.GetDgnDb().Elements().Insert<GraphicalType2d>(*type);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalTypeCPtr TypeTests::InsertType3d(DefinitionModelR typeModel, Utf8CP typeName, TemplateRecipe3dCR recipe)
    {
    GenericPhysicalTypePtr type = GenericPhysicalType::Create(typeModel, typeName);
    if (!type.IsValid())
        return nullptr;

    type->SetRecipe(recipe.GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalTypeHasTemplateRecipe));
    return recipe.GetDgnDb().Elements().Insert<PhysicalType>(*type);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateDrawingGraphic(DrawingModelR model, GraphicalType2dCR type, DPoint2dCR origin, AngleInDegreesCR rotation, DgnSubCategoryId subCategoryOverride, Utf8CP userLabel)
    {
    RecipeDefinitionElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return nullptr;

    DgnModelPtr templateModel = recipe->GetSubModel();
    if (!templateModel.IsValid())
        return nullptr;

    DgnDbR db = model.GetDgnDb();
    DgnCategoryId categoryId = subCategoryOverride.IsValid() ? DgnSubCategory::QueryCategoryId(db, subCategoryOverride) : DetermineCategoryId(type);
    if (!categoryId.IsValid())
        return nullptr;

    DrawingGraphicPtr element = DrawingGraphic::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, origin, rotation);
    if (!builder.IsValid())
        return nullptr;

    if (subCategoryOverride.IsValid())
        builder->Append(subCategoryOverride);

    element->SetTypeDefinition(type.GetElementId(), db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_GraphicalElement2dIsOfType));

    if (userLabel && *userLabel)
        element->SetUserLabel(userLabel);

    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_GeometricElement2d), *templateModel))
        {
        GeometricElement2dCPtr templateElement = db.Elements().Get<GeometricElement2d>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return nullptr;

        for (GeometryCollection::Iterator const& geometryEntry : GeometryCollection(*templateElement->ToGeometrySource()))
            {
            if (!subCategoryOverride.IsValid())
                builder->Append(geometryEntry.GetGeometryParams().GetSubCategoryId());

            DgnGeometryPartCPtr geometryPart = geometryEntry.GetGeometryPartCPtr();
            if (geometryPart.IsValid())
                {
                builder->Append(geometryPart->GetId(), geometryEntry.GetGeometryToWorld());
                }
            else
                {
                GeometricPrimitivePtr geometry = geometryEntry.GetGeometryPtr();
                if (geometry.IsValid())
                    {
                    if (IsInstanceSpecific(*templateElement) && GeometryCollection::Iterator::EntryType::TextString == geometryEntry.GetEntryType())
                        {
                        TextStringPtr textString = geometry->GetAsTextString();
                        if (textString.IsValid())
                            {
                            ECN::ECValue value;
                            Utf8String valueAsString = "???";

                            if ((DgnDbStatus::Success == element->GetPropertyValue(value, textString->GetText().c_str())) && !value.IsNull())
                                valueAsString = value.ToString();

                            textString->SetText(valueAsString.c_str());
                            geometry = GeometricPrimitive::Create(*textString);
                            }
                        }

                    builder->Append(*geometry);
                    }
                }
            }
        }

    return (BentleyStatus::SUCCESS == builder->Finish(*element)) ? element : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreatePhysicalObject(PhysicalModelR model, PhysicalTypeCR type, DPoint3dCR origin, YawPitchRollAnglesCR angles, Utf8CP userLabel)
    {
    RecipeDefinitionElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return nullptr;

    DgnModelPtr templateModel = recipe->GetSubModel();
    if (!templateModel.IsValid())
        return nullptr;

    DgnDbR db = model.GetDgnDb();
    DgnCategoryId categoryId = DetermineCategoryId(type);
    if (!categoryId.IsValid())
        return nullptr;

    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, origin, angles);
    if (!builder.IsValid())
        return nullptr;

    element->SetTypeDefinition(type.GetElementId(), db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementIsOfType));

    if (userLabel && *userLabel)
        element->SetUserLabel(userLabel);

    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_GeometricElement3d), *templateModel))
        {
        GeometricElement3dCPtr templateElement = db.Elements().Get<GeometricElement3d>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return nullptr;

        for (GeometryCollection::Iterator const& geometryEntry : GeometryCollection(*templateElement->ToGeometrySource()))
            {
            builder->Append(geometryEntry.GetGeometryParams().GetSubCategoryId());

            DgnGeometryPartCPtr geometryPart = geometryEntry.GetGeometryPartCPtr();
            if (geometryPart.IsValid())
                {
                builder->Append(geometryPart->GetId(), geometryEntry.GetGeometryToWorld());
                }
            else
                {
                GeometricPrimitivePtr geometry = geometryEntry.GetGeometryPtr();
                if (geometry.IsValid())
                    builder->Append(*geometry);
                }
            }
        }

    return (BentleyStatus::SUCCESS == builder->Finish(*element)) ? element : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TypeTests::InstantiateTemplate3d(PhysicalModelR instanceModel, PhysicalTypeCR type, GenericGroupModelP groupModel, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    RecipeDefinitionElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return DgnDbStatus::BadRequest;

    GeometricModelPtr templateModel = recipe->GetSub<GeometricModel>();
    if (!templateModel.IsValid())
        return DgnDbStatus::BadRequest;

    GenericGroupPtr group;
    if (nullptr != groupModel)
        {
        group = GenericGroup::Create(*groupModel);
        if (!group.IsValid())
            return DgnDbStatus::BadRequest;

        group->SetUserLabel(type.GetCode().GetValueUtf8CP()); // WIP: should be Propery in TemplateInstanceGroup subclass
        if (!group->Insert().IsValid())
            return DgnDbStatus::BadRequest;
        }

    DgnDbR db = type.GetDgnDb();
    DgnCloneContext cloneContext;
    ElementCopier elementCopier(cloneContext);

    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_SpatialElement), *templateModel))
        {
        SpatialElementCPtr templateElement = db.Elements().Get<SpatialElement>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return DgnDbStatus::BadRequest;

        DgnDbStatus copyStatus; 
        DgnElementCPtr instanceElement = elementCopier.MakeCopy(&copyStatus, instanceModel, *templateElement, DgnCode());
        if (!instanceElement.IsValid())
            return copyStatus;

        SpatialElementPtr instanceElementEdit = instanceElement->MakeCopy<SpatialElement>();
        if (!instanceElementEdit.IsValid())
            return DgnDbStatus::BadRequest;

        Transform templateTransform = templateElement->GetPlacementTransform();
        Transform instanceTransform = angles.ToTransform(origin);
        Transform placementTransform = Transform::FromProduct(templateTransform, instanceTransform);
        Placement3d instancePlacement = instanceElementEdit->GetPlacement();
        YawPitchRollAngles::TryFromTransform(instancePlacement.GetOriginR(), instancePlacement.GetAnglesR(), placementTransform);
        instanceElementEdit->SetPlacement(instancePlacement);

        if (!instanceElementEdit->Update().IsValid())
            return DgnDbStatus::BadRequest;

        if (group.IsValid())
            {
            if (DgnDbStatus::Success != ElementGroupsMembers::Insert(*group, *instanceElementEdit, 0))
                return DgnDbStatus::BadRequest;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, DEllipse3dCR ellipse, bool is3d)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(ellipse);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, is3d) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, ICurvePrimitiveCR curve, bool is3d)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(curve);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, is3d) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, CurveVectorCR curveVector, bool is3d)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(curveVector);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, is3d) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, DgnBoxDetailCR boxDetail)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(boxDetail);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, GEOMETRY3D) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, DgnConeDetailCR coneDetail)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(coneDetail);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, GEOMETRY3D) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, DgnSphereDetailCR sphereDetail)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(sphereDetail);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, GEOMETRY3D) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, DgnTorusPipeDetailCR torusPipeDetail)
    {
    GeometricPrimitivePtr geometry = GeometricPrimitive::Create(torusPipeDetail);
    BeAssert(geometry.IsValid());
    return geometry.IsValid() ? InsertGeometryPart(model, geometryPartName, *geometry, GEOMETRY3D) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartCPtr TypeTests::InsertGeometryPart(DefinitionModelR model, Utf8StringCR geometryPartName, GeometricPrimitiveCR geometry, bool is3d)
    {
    DgnDbR db = model.GetDgnDb();
    DgnGeometryPartPtr geometryPart = DgnGeometryPart::Create(model, geometryPartName);
    GeometryBuilderPtr geometryPartBuilder = GeometryBuilder::CreateGeometryPart(db, is3d);
    BeAssert(geometryPart.IsValid() && geometryPartBuilder.IsValid());
    if (!geometryPart.IsValid() || !geometryPartBuilder.IsValid())
        return nullptr;

    geometryPartBuilder->Append(geometry);
    geometryPartBuilder->Finish(*geometryPart);
    DgnGeometryPartCPtr result = db.Elements().Insert<DgnGeometryPart>(*geometryPart);
    BeAssert(result.IsValid());
    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertTorusPipeTemplate(TemplateRecipe3dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    Utf8PrintfString torusPipePartName("%s-TorusPipe", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr torusPipePart = InsertGeometryPart(*definitionModel, torusPipePartName, DgnTorusPipeDetail(DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), 1.0), 0.1, CAPPED));
    if (!torusPipePart.IsValid())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "OrangeCategory3d", ColorDef::Orange());
    DgnSubCategoryId defaultSubCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId);

    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    if (!model.IsValid() || !model->IsTemplate())
        return nullptr;

    GeometricElement3dPtr torusPipe = CreateGeometricElement3d(*model, defaultSubCategoryId, torusPipePart->GetId(), DPoint3d::FromZero());
    if (!torusPipe.IsValid() || !torusPipe->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertCubeAndCylindersTemplate(TemplateRecipe3dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double cubeWidth = 1.0;
    Utf8PrintfString cubePartName("%s-Cube", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr cubePart = InsertGeometryPart(*definitionModel, cubePartName, DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(cubeWidth/2, cubeWidth/2, cubeWidth/2), DPoint3d::From(cubeWidth, cubeWidth, cubeWidth), CAPPED));
    if (!cubePart.IsValid())
        return nullptr;

    const double cylinderRadius = 0.05;
    const double cylinderHeight = 0.1;
    Utf8PrintfString cylinderPartName("%s-Cylinder", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr cylinderPart = InsertGeometryPart(*definitionModel, cylinderPartName, DgnConeDetail(DPoint3d::FromZero(), DPoint3d::From(0, 0, cylinderHeight), cylinderRadius, cylinderRadius, CAPPED));
    if (!cylinderPart.IsValid())
        return nullptr;

    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    if (!model.IsValid() || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "GreenCategory3d");
    DgnSubCategoryId cubeSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Cubes", ColorDef::Green());
    DgnSubCategoryId cylinderSubCategoryId = DgnDbTestUtils::InsertSubCategory(db, categoryId, "Cylinders", ColorDef::DarkGreen());

    GeometricElement3dPtr cube = CreateGeometricElement3d(*model, cubeSubCategoryId, cubePart->GetId(), DPoint3d::FromZero());
    GeometricElement3dPtr cylinder1 = CreateGeometricElement3d(*model, cylinderSubCategoryId, cylinderPart->GetId(), DPoint3d::From(0.25, 0.25, cubeWidth));
    GeometricElement3dPtr cylinder2 = CreateGeometricElement3d(*model, cylinderSubCategoryId, cylinderPart->GetId(), DPoint3d::From(0.25, 0.75, cubeWidth));
    GeometricElement3dPtr cylinder3 = CreateGeometricElement3d(*model, cylinderSubCategoryId, cylinderPart->GetId(), DPoint3d::From(0.75, 0.75, cubeWidth));
    GeometricElement3dPtr cylinder4 = CreateGeometricElement3d(*model, cylinderSubCategoryId, cylinderPart->GetId(), DPoint3d::From(0.75, 0.25, cubeWidth));
    if (!cube.IsValid() || !cylinder1.IsValid() || !cylinder2.IsValid() || !cylinder3.IsValid() || !cylinder4.IsValid())
        return nullptr;

    if (!cube->Insert().IsValid() || !cylinder1->Insert().IsValid() || !cylinder2->Insert().IsValid() || !cylinder3->Insert().IsValid() || !cylinder4->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertThreeSpheresTemplate(TemplateRecipe3dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double radius = 0.25;
    Utf8PrintfString spherePartName("%s-Sphere", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr spherePart = InsertGeometryPart(*definitionModel, spherePartName, DgnSphereDetail(DPoint3d::FromZero(), radius));
    if (!spherePart.IsValid())
        return nullptr;

    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    if (!model.IsValid() || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "YellowCategory3d", ColorDef::Yellow());
    DgnSubCategoryId defaultSubCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId);

    GeometricElement3dPtr sphere1 = CreateGeometricElement3d(*model, defaultSubCategoryId, spherePart->GetId(), DPoint3d::From(0, 0, 1));
    GeometricElement3dPtr sphere2 = CreateGeometricElement3d(*model, defaultSubCategoryId, spherePart->GetId(), DPoint3d::From(0, 0, 2));
    GeometricElement3dPtr sphere3 = CreateGeometricElement3d(*model, defaultSubCategoryId, spherePart->GetId(), DPoint3d::From(0, 0, 3));
    if (!sphere1.IsValid() || !sphere2.IsValid() || !sphere3.IsValid())
        return nullptr;

    if (!sphere1->Insert().IsValid() || !sphere2->Insert().IsValid() || !sphere3->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertSlabAndColumnsTemplate(TemplateRecipe3dCR recipe)
    {
    DefinitionModelPtr definitionModel = recipe.GetModel()->ToDefinitionModelP();
    if (!definitionModel.IsValid())
        return nullptr;

    const double slabWidth = 1.0;
    const double slabHeight = 0.1;
    Utf8PrintfString slabPartName("%s-Slab", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr slabPart = InsertGeometryPart(*definitionModel, slabPartName, DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(slabWidth/2, slabWidth/2, slabHeight/2), DPoint3d::From(slabWidth, slabWidth, slabHeight), CAPPED));
    if (!slabPart.IsValid())
        return nullptr;

    const double columnRadius = 0.1;
    const double columnHeight = 0.25;
    Utf8PrintfString columnPartName("%s-Column", recipe.GetCode().GetValueUtf8CP());
    DgnGeometryPartCPtr columnPart = InsertGeometryPart(*definitionModel, columnPartName, DgnConeDetail(DPoint3d::FromZero(), DPoint3d::From(0, 0, columnHeight), columnRadius, columnRadius, CAPPED));
    if (!columnPart.IsValid())
        return nullptr;

    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    if (!model.IsValid() || !model->IsTemplate())
        return nullptr;

    DgnDbR db = recipe.GetDgnDb();
    DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementAssemblesElements);
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "BrownCategory3d");
    DgnSubCategoryId slabSubCategoryId = DgnDbTestUtils::InsertSubCategory(*m_db, categoryId, "Slabs", ColorDef::Brown());
    DgnSubCategoryId columnSubCategoryId = DgnDbTestUtils::InsertSubCategory(*m_db, categoryId, "Columns", ColorDef::DarkBrown());

    GeometricElement3dPtr slab = CreateGeometricElement3d(*model, slabSubCategoryId, slabPart->GetId(), DPoint3d::From(0, 0, columnHeight));
    GeometricElement3dPtr column1 = CreateGeometricElement3d(*model, columnSubCategoryId, columnPart->GetId(), DPoint3d::From(slabWidth/4, slabWidth/4, 0));
    GeometricElement3dPtr column2 = CreateGeometricElement3d(*model, columnSubCategoryId, columnPart->GetId(), DPoint3d::From(slabWidth/4, 3*slabWidth/4, 0));
    GeometricElement3dPtr column3 = CreateGeometricElement3d(*model, columnSubCategoryId, columnPart->GetId(), DPoint3d::From(3*slabWidth/4, 3*slabWidth/4, 0));
    GeometricElement3dPtr column4 = CreateGeometricElement3d(*model, columnSubCategoryId, columnPart->GetId(), DPoint3d::From(3*slabWidth/4, slabWidth/4, 0));
    if (!slab.IsValid() || !column1.IsValid() || !column2.IsValid() || !column3.IsValid() || !column4.IsValid())
        return nullptr;

    if (!slab->Insert().IsValid())
        return nullptr;

    column1->SetParentId(slab->GetElementId(), parentRelClassId);
    column2->SetParentId(slab->GetElementId(), parentRelClassId);
    column3->SetParentId(slab->GetElementId(), parentRelClassId);
    column4->SetParentId(slab->GetElementId(), parentRelClassId);

    if (!column1->Insert().IsValid() || !column2->Insert().IsValid() || !column3->Insert().IsValid() || !column4->Insert().IsValid())
        return nullptr;

    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GeometricElement3dPtr TypeTests::CreateGeometricElement3d(GeometricModel3dR model, GeometryCollection::Iterator const& iter)
    {
    DgnCategoryId categoryId = iter.GetGeometryParams().GetCategoryId();
    DgnSubCategoryId subCategoryId = iter.GetGeometryParams().GetSubCategoryId();
    GeometricPrimitivePtr geometry = iter.GetGeometryPtr();
    if (!categoryId.IsValid() || !subCategoryId.IsValid() || !geometry.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, iter.GetGeometryToWorld());
    if (!builder.IsValid() || !builder->Append(subCategoryId) || !builder->Append(*geometry))
        return nullptr;

    return CreateGeometricElement3d(model, categoryId, *builder);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GeometricElement3dPtr TypeTests::CreateGeometricElement3d(GeometricModel3dR model, DgnSubCategoryId subCategoryId, DgnGeometryPartId geometryPartId, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(model.GetDgnDb(), subCategoryId);
    if (!categoryId.IsValid())
        return nullptr;

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, origin, angles);
    if (!builder.IsValid() || !builder->Append(subCategoryId) || !builder->Append(geometryPartId, DPoint3d::FromZero()))
        return nullptr;

    return CreateGeometricElement3d(model, categoryId, *builder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GeometricElement3dPtr TypeTests::CreateGeometricElement3d(GeometricModel3dR model, DgnCategoryId categoryId, GeometryBuilderR builder)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(db, model.GetModelId(), classId, categoryId));
    if (!element.IsValid())
        return nullptr;

    return (BentleyStatus::SUCCESS == builder.Finish(*element)) ? element.get() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnCategoryId TypeTests::DetermineCategoryId(TypeDefinitionElementCR type)
    {
    DgnDbR db = type.GetDgnDb();
    RecipeDefinitionElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return DgnCategoryId();

    GeometricModelPtr templateModel = recipe->GetSub<GeometricModel>();
    if (!templateModel.IsValid())
        return DgnCategoryId();

    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_GeometricElement), *templateModel))
        {
        GeometricElementCPtr element = db.Elements().Get<GeometricElement>(elementEntry.GetElementId());
        if (element.IsValid())
            return templateModel->Is2dModel() ? element->ToGeometrySource2d()->GetCategoryId() : element->ToGeometrySource3d()->GetCategoryId();
        }

    return DgnCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
ElementIterator TypeTests::MakeElementIteratorWhereModel(Utf8CP elementClassName, DgnModelCR model)
    {
    Utf8PrintfString whereClause("WHERE Model.Id=%" PRIu64, model.GetModelId().GetValue());
    return model.GetDgnDb().Elements().MakeIterator(elementClassName, whereClause.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
bool TypeTests::IsInstanceSpecific(DgnElementCR element)
    {
    ECN::AdHocJsonValueCR jsonObj = element.GetJsonProperties(JSON_NAMESPACE_TypeTests);
    return jsonObj.get(JSON_PROP_IsInstanceSpecific, false).asBool();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
void TypeTests::SetInstanceSpecific(DgnElementR element, bool isInstanceSpecific)
    {
    Json::Value jsonObj = element.GetJsonProperties(JSON_NAMESPACE_TypeTests);
    jsonObj[JSON_PROP_IsInstanceSpecific] = isInstanceSpecific;
    element.SetJsonProperties(JSON_NAMESPACE_TypeTests, jsonObj);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TypeTests::DropSpatialElementToGeometry(SpatialModelR model, SpatialElementCR elementToDrop)
    {
    for (GeometryCollection::Iterator const& iter : GeometryCollection(elementToDrop))
        {
        DgnGeometryPartCPtr geometryPart = iter.GetGeometryPartCPtr();
        if (geometryPart.IsValid())
            {
            GeometryCollection partCollection(geometryPart->GetGeometryStream(), model.GetDgnDb());
            partCollection.SetNestedIteratorContext(iter); // Iterate part GeometryStream in context of parent...

            for (GeometryCollection::Iterator const& partIter : partCollection)
                {
                GeometricElement3dPtr geometricElement = CreateGeometricElement3d(model, partIter);
                if (!geometricElement.IsValid() || !geometricElement->Insert().IsValid())
                    return DgnDbStatus::BadRequest;
                }
            }
        else
            {
            GeometricElement3dPtr geometricElement = CreateGeometricElement3d(model, iter);
            if (!geometricElement.IsValid() || !geometricElement->Insert().IsValid())
                return DgnDbStatus::BadRequest;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
DgnViewId TypeTests::InsertTemplateView2d(DefinitionModelR model, Utf8CP name)
    {
    TemplateViewDefinition2dPtr view = TemplateViewDefinition2d::Create(model, name);
    if (view.IsValid())
        {
        view->SetStandardViewRotation(StandardView::Top);
        view->Insert();
        }

    BeAssert(view.IsValid() && view->GetViewId().IsValid());
    return view.IsValid() ? view->GetViewId() : DgnViewId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnViewId TypeTests::InsertTemplateView3d(DefinitionModelR model, Utf8CP name)
    {
    TemplateViewDefinition3dPtr view = TemplateViewDefinition3d::Create(model, name);
    if (view.IsValid())
        {
        view->SetStandardViewRotation(StandardView::Iso);
        view->Insert();
        }

    BeAssert(view.IsValid() && view->GetViewId().IsValid());
    return view.IsValid() ? view->GetViewId() : DgnViewId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnViewId TypeTests::InsertSpatialView(SpatialModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();
    ModelSelectorPtr modelSelector = new ModelSelector(dictionary, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinition view(dictionary, name, *new CategorySelector(dictionary, ""), *new DisplayStyle3d(dictionary, ""), *modelSelector);

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        view.GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    view.SetStandardViewRotation(StandardView::Iso);
    view.LookAtVolume(model.QueryElementsRange());
    view.Insert();
    DgnViewId viewId = view.GetViewId();
    BeAssert(viewId.IsValid());
    return viewId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
TEST_F(TypeTests, CreateSampleBim)
    {
    SetupSeedProject();
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    DefinitionModelPtr typeModel2d = DgnDbTestUtils::InsertDefinitionModel(*m_db, "2D Types");
    TemplateRecipe2dCPtr recipe2A = InsertTemplateRecipe2d(*typeModel2d, "Recipe2-A");
    ASSERT_TRUE(recipe2A.IsValid());
    DrawingModelPtr templateModel2A = InsertRectangleAndLinesTemplate2d(*recipe2A);
    ASSERT_TRUE(templateModel2A.IsValid());
    GraphicalType2dCPtr type2A1 = InsertType2d(*typeModel2d, "Type2-A-1", *recipe2A);
    GraphicalType2dCPtr type2A2 = InsertType2d(*typeModel2d, "Type2-A-2", *recipe2A);
    ASSERT_TRUE(type2A1.IsValid());
    ASSERT_TRUE(type2A2.IsValid());

    TemplateRecipe2dCPtr recipe2B = InsertTemplateRecipe2d(*typeModel2d, "Recipe2-B");
    ASSERT_TRUE(recipe2B.IsValid());
    DrawingModelPtr templateModel2B = InsertCircleAndCrossTemplate2d(*recipe2B, *type2A1);
    ASSERT_TRUE(templateModel2B.IsValid());
    GraphicalType2dCPtr type2B1 = InsertType2d(*typeModel2d, "Type2-B-1", *recipe2B);
    ASSERT_TRUE(type2B1.IsValid());

    DefinitionModelPtr typeModel3d = DgnDbTestUtils::InsertDefinitionModel(*m_db, "3D Types");
    TemplateRecipe3dCPtr recipe3A = InsertTemplateRecipe3d(*typeModel3d, "Recipe3-A");
    TemplateRecipe3dCPtr recipe3B = InsertTemplateRecipe3d(*typeModel3d, "Recipe3-B");
    TemplateRecipe3dCPtr recipe3C = InsertTemplateRecipe3d(*typeModel3d, "Recipe3-C");
    TemplateRecipe3dCPtr recipe3E = InsertTemplateRecipe3d(*typeModel3d, "Recipe3-E");

    PhysicalModelPtr templateModel3A = InsertTorusPipeTemplate(*recipe3A);
    PhysicalModelPtr templateModel3B = InsertCubeAndCylindersTemplate(*recipe3B);
    PhysicalModelPtr templateModel3C = InsertThreeSpheresTemplate(*recipe3C);
    PhysicalModelPtr templateModel3E = InsertSlabAndColumnsTemplate(*recipe3E);

    PhysicalTypeCPtr type3A1 = InsertType3d(*typeModel3d, "Type3-A-1", *recipe3A);
    PhysicalTypeCPtr type3B1 = InsertType3d(*typeModel3d, "Type3-B-1", *recipe3B);
    PhysicalTypeCPtr type3B2 = InsertType3d(*typeModel3d, "Type3-B-2", *recipe3B);
    PhysicalTypeCPtr type3C1 = InsertType3d(*typeModel3d, "Type3-C-1", *recipe3C);
    PhysicalTypeCPtr type3E1 = InsertType3d(*typeModel3d, "Type3-E-1", *recipe3E);

    GenericGroupModelPtr groupModel = DgnDbTestUtils::InsertGroupInformationModel(*m_db, "Template Instance Groups");

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "2D Instances");
    DrawingModelPtr instanceModel2d = DgnDbTestUtils::InsertDrawingModel(*drawing);

    for (int i=0; i<5; i++)
        {
        DPoint2d origin = DPoint2d::From(i*3+1, i*3+1);
        Utf8PrintfString userLabel("Symbol%" PRIi32, i);
        DrawingGraphicPtr instance2B1 = CreateDrawingGraphic(*instanceModel2d, *type2B1, origin, AngleInDegrees(), DgnSubCategoryId(), userLabel.c_str());
        ASSERT_TRUE(instance2B1.IsValid());
        ASSERT_TRUE(instance2B1->Insert().IsValid());
        ASSERT_EQ(instance2B1->GetGraphicalType()->GetElementId().GetValue(), type2B1->GetElementId().GetValue());

        origin.Add(DPoint2d::From(0, 3));
        DrawingGraphicPtr instance2A1 = CreateDrawingGraphic(*instanceModel2d, *type2A1, origin);
        ASSERT_TRUE(instance2A1.IsValid());
        ASSERT_TRUE(instance2A1->Insert().IsValid());
        ASSERT_EQ(instance2A1->GetGraphicalType()->GetElementId().GetValue(), type2A1->GetElementId().GetValue());
        }

    DrawingViewDefinitionPtr view2d = DgnDbTestUtils::InsertDrawingView(*instanceModel2d, "2D Instances View");
    view2d->LookAtVolume(instanceModel2d->QueryElementsRange());
    ASSERT_TRUE(view2d->Update().IsValid());

    PhysicalModelPtr instanceModel3d = DgnDbTestUtils::InsertPhysicalModel(*m_db, "3D Instances");
    DgnElementId toDropElementId;

    for (int i=0; i<5; i++)
        {
        DPoint3d origin = DPoint3d::From(i*2, 0.0, i*2);
        Utf8PrintfString userLabel("Equipment%" PRIi32, i);
        PhysicalElementPtr element = CreatePhysicalObject(*instanceModel3d, *type3B1, origin, YawPitchRollAngles(), userLabel.c_str());
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_EQ(element->GetPhysicalType()->GetElementId().GetValue(), type3B1->GetElementId().GetValue());
        if (!toDropElementId.IsValid())
            toDropElementId = element->GetElementId();

        origin.Add(DPoint3d::From(0, 3, 0));
        DgnDbStatus status = InstantiateTemplate3d(*instanceModel3d, *type3A1, nullptr, origin, YawPitchRollAngles());
        ASSERT_EQ(DgnDbStatus::Success, status);

        origin.Add(DPoint3d::From(0, 3, 0));
        status = InstantiateTemplate3d(*instanceModel3d, *type3C1, groupModel.get(), origin, YawPitchRollAngles());
        ASSERT_EQ(DgnDbStatus::Success, status);

        origin.Add(DPoint3d::From(0, 3, 0));
        status = InstantiateTemplate3d(*instanceModel3d, *type3E1, groupModel.get(), origin, YawPitchRollAngles());
        ASSERT_EQ(DgnDbStatus::Success, status);
        }

    InsertSpatialView(*instanceModel3d, "3D Instances View");

    PhysicalModelPtr drop3B1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Drop 3B1 Instance to Geometry");

    SpatialElementCPtr elementToDrop = m_db->Elements().Get<SpatialElement>(toDropElementId);
    ASSERT_TRUE(elementToDrop.IsValid());
    DgnDbStatus dropStatus = DropSpatialElementToGeometry(*drop3B1, *elementToDrop);
    ASSERT_EQ(DgnDbStatus::Success, dropStatus);
    InsertSpatialView(*drop3B1, "Drop 3B1 View");

    InsertTemplateView2d(*typeModel2d, "2D Template View");
    InsertTemplateView3d(*typeModel3d, "3D Template View");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2017
//---------------------------------------------------------------------------------------
TEST_F(TypeTests, MimicCellLibraryImport)
    {
    SetupSeedProject();
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateClassViewsInDb());

    SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();
    SubjectCPtr symbolsSubject = Subject::CreateAndInsert(*rootSubject, "My Symbols");
    DefinitionPartitionCPtr symbolLibrary1 = DefinitionPartition::CreateAndInsert(*symbolsSubject, "Symbol Library 1");
    DefinitionPartitionCPtr symbolLibrary2 = DefinitionPartition::CreateAndInsert(*symbolsSubject, "Symbol Library 2");
    ASSERT_TRUE(symbolLibrary1.IsValid());
    ASSERT_TRUE(symbolLibrary2.IsValid());

    DefinitionModelPtr symbolLibraryModel1 = DefinitionModel::CreateAndInsert(*symbolLibrary1);
    DefinitionModelPtr symbolLibraryModel2 = DefinitionModel::CreateAndInsert(*symbolLibrary2);
    ASSERT_TRUE(symbolLibraryModel1.IsValid());
    ASSERT_TRUE(symbolLibraryModel2.IsValid());
    EXPECT_EQ(symbolLibrary1->GetSubModelId(), symbolLibraryModel1->GetModelId());
    EXPECT_EQ(symbolLibrary2->GetSubModelId(), symbolLibraryModel2->GetModelId());

    TemplateRecipe2dCPtr recipe1A = InsertTemplateRecipe2d(*symbolLibraryModel1, "A");
    TemplateRecipe2dCPtr recipe1B = InsertTemplateRecipe2d(*symbolLibraryModel1, "B");
    TemplateRecipe2dCPtr recipe1C = InsertTemplateRecipe2d(*symbolLibraryModel1, "C");

    GraphicalType2dCPtr type1A = InsertType2d(*symbolLibraryModel1, "A", *recipe1A);
    GraphicalType2dCPtr type1B = InsertType2d(*symbolLibraryModel1, "B", *recipe1B);
    GraphicalType2dCPtr type1C = InsertType2d(*symbolLibraryModel1, "C", *recipe1C);
    ASSERT_TRUE(type1A.IsValid());
    ASSERT_TRUE(type1B.IsValid());
    ASSERT_TRUE(type1C.IsValid());

    DrawingModelPtr templateModel1A = InsertCircleTemplate2d(*recipe1A);
    DrawingModelPtr templateModel1B = InsertTriangleTemplate2d(*recipe1B);
    DrawingModelPtr templateModel1C = InsertRectangleTemplate2d(*recipe1C);

    TemplateRecipe2dCPtr recipe2A = InsertTemplateRecipe2d(*symbolLibraryModel2, "A");
    TemplateRecipe2dCPtr recipe2B = InsertTemplateRecipe2d(*symbolLibraryModel2, "B");

    GraphicalType2dCPtr type2A = InsertType2d(*symbolLibraryModel2, "A", *recipe2A);
    GraphicalType2dCPtr type2B = InsertType2d(*symbolLibraryModel2, "B", *recipe2B);
    ASSERT_TRUE(type2A.IsValid());
    ASSERT_TRUE(type2B.IsValid());
    }
