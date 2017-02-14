/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/TypeRecipe_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//========================================================================================
// @bsiclass                                                    Shaun.Sewall    02/2017
//========================================================================================
struct TypeTests : public DgnDbTestFixture
{
    static Utf8CP USERPROP_IsInstanceSpecific() {return "IsInstanceSpecific";}
    static Utf8CP USERPROP_ValueExpression() {return "ValueExpression";}

    GraphicalRecipe2dCPtr InsertRecipe2d(DefinitionModelR, Utf8CP, DgnCategoryId);
    GraphicalType2dCPtr InsertType2d(DefinitionModelR, Utf8CP, GraphicalRecipe2dCR);

    DrawingModelPtr InsertTemplate2A(GraphicalRecipe2dCR, DgnCategoryId);
    DrawingModelPtr InsertTemplate2B(GraphicalRecipe2dCR, DgnCategoryId, GraphicalType2dCR);

    DrawingGraphicPtr CreateTemplateGraphic(DrawingModelR, DgnCategoryId, DPoint2dCR, double, Utf8CP);
    DrawingGraphicPtr CreateTemplateGraphic(DrawingModelR, DgnCategoryId, DSegment3dCR);
    DrawingGraphicPtr CreateTemplateGraphic(DrawingModelR, DgnCategoryId, DEllipse3dCR);
    DrawingGraphicPtr CreateTemplateGraphic(DrawingModelR, DgnCategoryId, ICurvePrimitiveCR);
    DrawingGraphicPtr CreateTemplateGraphic(DrawingModelR, DgnCategoryId, GeometryBuilderR);
    DrawingGraphicPtr CreateDrawingGraphic(DrawingModelR, DgnCategoryId, GraphicalType2dCR, DPoint2dCR, Utf8CP userLabel=nullptr);
    
    PhysicalRecipeCPtr InsertRecipe3d(DefinitionModelR, Utf8CP, DgnCategoryId);
    PhysicalTypeCPtr InsertType3d(DefinitionModelR, Utf8CP, PhysicalRecipeCR);

    PhysicalModelPtr InsertTemplate3A(PhysicalRecipeCR);
    PhysicalModelPtr InsertTemplate3B(PhysicalRecipeCR, DgnCategoryId);

    PhysicalElementPtr CreateTemplateObject(PhysicalModelR, DgnCategoryId, DgnBoxDetailCR);
    PhysicalElementPtr CreateTemplateObject(PhysicalModelR, DgnCategoryId, DgnConeDetailCR);
    PhysicalElementPtr CreateTemplateObject(PhysicalModelR, DgnCategoryId, DgnTorusPipeDetailCR);
    PhysicalElementPtr CreateTemplateObject(PhysicalModelR, DgnCategoryId, ISolidPrimitiveCR);
    PhysicalElementPtr CreateTemplateObject(PhysicalModelR, DgnCategoryId, GeometryBuilderR);
    PhysicalElementPtr CreatePhysicalObject(PhysicalModelR, PhysicalTypeCR, DPoint3dCR, YawPitchRollAnglesCR, Utf8CP userLabel=nullptr);
    DgnDbStatus CloneTemplate3d(PhysicalModelR, PhysicalTypeCR, DPoint3dCR, YawPitchRollAnglesCR);

    DgnGeometryPartId QueryGeometryPartId(TypeDefinitionElementCR);
    ElementIterator MakeElementIteratorWhereModel(Utf8CP, DgnModelCR);
    bool IsNestedTypeLocation(DgnElementCR);
    bool IsInstanceSpecific(DgnElementCR);
    void SetInstanceSpecific(DgnElementR, bool);
    DgnDbStatus AppendInstanceSpecificGeometry(GeometryBuilderR, DgnElementCR, GraphicalType2dCR, DPoint2dCR);
    Utf8CP GetValueExpression(DgnElementCR);
    void SetValueExpression(DgnElementR, Utf8CP);
    DgnViewId InsertSpatialView(SpatialModelR, Utf8CP);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GraphicalRecipe2dCPtr TypeTests::InsertRecipe2d(DefinitionModelR model, Utf8CP recipeName, DgnCategoryId outputCategoryId)
    {
    GraphicalRecipe2dPtr recipeFromCreate = TestGraphicalRecipe2d::Create(model, recipeName);
    recipeFromCreate->SetOutputCategory(outputCategoryId);
    BeAssert(recipeFromCreate.IsValid());
    GraphicalRecipe2dCPtr recipeFromInsert = model.GetDgnDb().Elements().Insert<GraphicalRecipe2d>(*recipeFromCreate);
    BeAssert(recipeFromInsert.IsValid());
    return recipeFromInsert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalRecipeCPtr TypeTests::InsertRecipe3d(DefinitionModelR model, Utf8CP recipeName, DgnCategoryId outputCategoryId)
    {
    PhysicalRecipePtr recipeFromCreate = TestPhysicalRecipe::Create(model, recipeName);
    recipeFromCreate->SetOutputCategory(outputCategoryId);
    BeAssert(recipeFromCreate.IsValid());
    PhysicalRecipeCPtr recipeFromInsert = model.GetDgnDb().Elements().Insert<PhysicalRecipe>(*recipeFromCreate);
    BeAssert(recipeFromInsert.IsValid());
    return recipeFromInsert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertTemplate2A(GraphicalRecipe2dCR recipe, DgnCategoryId categoryId)
    {
    DrawingModelPtr model = DrawingModel::Create(recipe);
    BeAssert(model.IsValid());
    BeAssert(DgnDbStatus::Success == model->Insert());
    BeAssert(model->IsTemplate());

    const double width = 0.2;
    DrawingGraphicPtr rectangle = CreateTemplateGraphic(*model, categoryId, *ICurvePrimitive::CreateRectangle(-width/2, -width/2, width/2, width/2, 0));
    DrawingGraphicPtr line1 = CreateTemplateGraphic(*model, categoryId, DSegment3d::From(DPoint2d::From(-width/2, -width/2), DPoint2d::From(width/2, width/2)));
    DrawingGraphicPtr line2 = CreateTemplateGraphic(*model, categoryId, DSegment3d::From(DPoint2d::From(-width/2, width/2), DPoint2d::From(width/2, -width/2)));
    BeAssert(rectangle.IsValid() && line1.IsValid() && line2.IsValid());
    BeAssert(rectangle->Insert().IsValid() && line1->Insert().IsValid() && line2->Insert().IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingModelPtr TypeTests::InsertTemplate2B(GraphicalRecipe2dCR recipe, DgnCategoryId categoryId, GraphicalType2dCR nestedType)
    {
    DrawingModelPtr model = DrawingModel::Create(recipe);
    BeAssert(model.IsValid());
    BeAssert(DgnDbStatus::Success == model->Insert());
    BeAssert(model->IsTemplate());

    const double radius = 1.0;
    DrawingGraphicPtr line1 = CreateTemplateGraphic(*model, categoryId, DSegment3d::From(DPoint2d::From(-radius, 0), DPoint2d::From(radius, 0)));
    DrawingGraphicPtr line2 = CreateTemplateGraphic(*model, categoryId, DSegment3d::From(DPoint2d::From(0, -radius), DPoint2d::From(0, radius)));
    DrawingGraphicPtr circle = CreateTemplateGraphic(*model, categoryId, DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), radius));
    DrawingGraphicPtr field = CreateTemplateGraphic(*model, categoryId, DPoint2d::From(0, 1.5*radius), radius/4, "_XXX_");
    NestedTypeLocation2dPtr nested1 = NestedTypeLocation2d::Create(*model, categoryId, nestedType, DPoint2d::From(-1.2*radius, 0));
    NestedTypeLocation2dPtr nested2 = NestedTypeLocation2d::Create(*model, categoryId, nestedType, DPoint2d::From(0, -1.2*radius));
    NestedTypeLocation2dPtr nested3 = NestedTypeLocation2d::Create(*model, categoryId, nestedType, DPoint2d::From(1.2*radius, 0));
    BeAssert(line1.IsValid() && line2.IsValid() && circle.IsValid() && field.IsValid() && nested1.IsValid() && nested2.IsValid() && nested3.IsValid());

    SetInstanceSpecific(*field, true);
    SetValueExpression(*field, "UserLabel");

    BeAssert(line1->Insert().IsValid() && line2->Insert().IsValid() && circle->Insert().IsValid() && field->Insert().IsValid() && nested1->Insert().IsValid() && nested2->Insert().IsValid() && nested3->Insert().IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTemplateGraphic(DrawingModelR model, DgnCategoryId categoryId, DPoint2dCR origin, double textHeight, Utf8CP text)
    {
    TextStringPtr textString = TextString::Create();
    textString->SetOrigin(DPoint3d::From(origin));
    textString->GetStyleR().SetSize(textHeight);
    textString->GetStyleR().SetFont(DgnFontManager::GetAnyLastResortFont());
    textString->SetText(text);

    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint2d::FromZero());
    if (!builder.IsValid() || !builder->Append(*textString))
        return nullptr;

    return CreateTemplateGraphic(model, categoryId, *builder);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTemplateGraphic(DrawingModelR model, DgnCategoryId categoryId, DSegment3dCR segment)
    {
    ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(segment);
    return curve.IsValid() ? CreateTemplateGraphic(model, categoryId, *curve) : nullptr;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTemplateGraphic(DrawingModelR model, DgnCategoryId categoryId, DEllipse3dCR ellipse)
    {
    ICurvePrimitivePtr curve = ICurvePrimitive::CreateArc(ellipse);
    return curve.IsValid() ? CreateTemplateGraphic(model, categoryId, *curve) : nullptr;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTemplateGraphic(DrawingModelR model, DgnCategoryId categoryId, ICurvePrimitiveCR curve)
    {
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint2d::FromZero());
    if (!builder.IsValid() || !builder->Append(curve))
        return nullptr;

    return CreateTemplateGraphic(model, categoryId, *builder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateTemplateGraphic(DrawingModelR model, DgnCategoryId categoryId, GeometryBuilderR builder)
    {
    DrawingGraphicPtr element = DrawingGraphic::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    return (BentleyStatus::SUCCESS == builder.Finish(*element)) ? element : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
GraphicalType2dCPtr TypeTests::InsertType2d(DefinitionModelR typeModel, Utf8CP typeName, GraphicalRecipe2dCR recipe)
    {
    TestGraphicalType2dPtr typeFromCreate = TestGraphicalType2d::Create(typeModel, typeName);
    BeAssert(typeFromCreate.IsValid());
    typeFromCreate->SetRecipe(recipe.GetElementId(), m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_GraphicalType2dHasRecipe));
    BeAssert(typeFromCreate->GetRecipe().IsValid());
    GraphicalType2dCPtr typeFromInsert = recipe.GetDgnDb().Elements().Insert<GraphicalType2d>(*typeFromCreate);
    BeAssert(typeFromInsert.IsValid());
    return typeFromInsert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalTypeCPtr TypeTests::InsertType3d(DefinitionModelR typeModel, Utf8CP typeName, PhysicalRecipeCR recipe)
    {
    TestPhysicalTypePtr typeFromCreate = TestPhysicalType::Create(typeModel, typeName);
    BeAssert(typeFromCreate.IsValid());
    typeFromCreate->SetRecipe(recipe.GetElementId(), m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalTypeHasRecipe));
    BeAssert(typeFromCreate->GetRecipe().IsValid());
    PhysicalTypeCPtr typeFromInsert = recipe.GetDgnDb().Elements().Insert<PhysicalType>(*typeFromCreate);
    BeAssert(typeFromInsert.IsValid());
    return typeFromInsert;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DrawingGraphicPtr TypeTests::CreateDrawingGraphic(DrawingModelR model, DgnCategoryId categoryId, GraphicalType2dCR type, DPoint2dCR origin, Utf8CP userLabel)
    {
    DrawingGraphicPtr graphic = DrawingGraphic::Create(model, categoryId);
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint2d::FromZero());
    if (!graphic.IsValid() || !builder.IsValid())
        return nullptr;

    if (userLabel && *userLabel)
        graphic->SetUserLabel(userLabel);

    if (DgnDbStatus::Success != AppendInstanceSpecificGeometry(*builder, *graphic, type, origin))
        return nullptr;

    return (BentleyStatus::SUCCESS == builder->Finish(*graphic)) ? graphic : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreatePhysicalObject(PhysicalModelR model, PhysicalTypeCR type, DPoint3dCR origin, YawPitchRollAnglesCR angles, Utf8CP userLabel)
    {
    DgnGeometryPartId geometryPartId = QueryGeometryPartId(type);
    if (!geometryPartId.IsValid())
        return nullptr;

    RecipeElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return nullptr;

    DgnCategoryId categoryId = recipe->GetOutputCategoryId();
    if (!categoryId.IsValid())
        return nullptr;

    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint3d::FromZero());
    if (!element.IsValid() || !builder.IsValid())
        return nullptr;

    if (userLabel && *userLabel)
        element->SetUserLabel(userLabel);

    if (!builder->Append(geometryPartId, origin, angles))
        return nullptr;

    //if (DgnDbStatus::Success != AppendInstanceSpecificGeometry(*builder, *element, type, origin))
    //    return nullptr;

    return (BentleyStatus::SUCCESS == builder->Finish(*element)) ? element : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TypeTests::CloneTemplate3d(PhysicalModelR instanceModel, PhysicalTypeCR type, DPoint3dCR origin, YawPitchRollAnglesCR angles)
    {
    RecipeElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return DgnDbStatus::BadRequest;

    GeometricModelPtr templateModel = recipe->GetSub<GeometricModel>();
    if (!templateModel.IsValid())
        return DgnDbStatus::BadRequest;

    DgnDbR db = type.GetDgnDb();
    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_SpatialElement), *templateModel))
        {
        SpatialElementCPtr templateElement = db.Elements().Get<SpatialElement>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return DgnDbStatus::BadRequest;

        DgnElement::CreateParams cloneParams(db, instanceModel.GetModelId(), templateElement->GetElementClassId(), DgnCode(), templateElement->GetUserLabel());
        DgnElementPtr instanceElement = templateElement->Clone(nullptr, &cloneParams);
        if (!instanceElement.IsValid())
            return DgnDbStatus::BadRequest;

        GeometrySource3dP instanceGeometrySource = instanceElement->ToGeometrySource3dP();
        Placement3d instancePlacement = templateElement->GetPlacement();
        instancePlacement.GetOriginR() = origin;
        instancePlacement.GetAnglesR() = angles;
        instanceGeometrySource->SetPlacement(instancePlacement);

        if (!instanceElement->Insert().IsValid())
            return DgnDbStatus::BadRequest;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertTemplate3A(PhysicalRecipeCR recipe)
    {
    DgnDbR db = recipe.GetDgnDb();
    DgnCategoryId orangeCategory3d = DgnDbTestUtils::InsertSpatialCategory(db, "OrangeCategory3d", ColorDef::Orange());

    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    BeAssert(model.IsValid());
    BeAssert(model->IsTemplate());

    const bool CAPPED = true;
    PhysicalElementPtr torusPipe = CreateTemplateObject(*model, orangeCategory3d, DgnTorusPipeDetail(DEllipse3d::FromCenterRadiusXY(DPoint3d::FromZero(), 1.0), 0.1, CAPPED));
    BeAssert(torusPipe.IsValid());
    BeAssert(torusPipe->Insert().IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalModelPtr TypeTests::InsertTemplate3B(PhysicalRecipeCR recipe, DgnCategoryId categoryId)
    {
    PhysicalModelPtr model = PhysicalModel::CreateAndInsert(recipe);
    BeAssert(model.IsValid());
    BeAssert(model->IsTemplate());

    const bool CAPPED = true;
    const double cubeWidth = 1.0;
    const double terminalRadius = 0.05;
    const double terminalHeight = 0.1;
    PhysicalElementPtr cube = CreateTemplateObject(*model, categoryId, DgnBoxDetail::InitFromCenterAndSize(DPoint3d::From(cubeWidth/2, cubeWidth/2, cubeWidth/2), DPoint3d::From(cubeWidth, cubeWidth, cubeWidth), CAPPED));
    PhysicalElementPtr terminal1 = CreateTemplateObject(*model, categoryId, DgnConeDetail(DPoint3d::From(0.25, 0.25, cubeWidth), DPoint3d::From(0.25, 0.25, cubeWidth+terminalHeight), terminalRadius, terminalRadius, CAPPED));
    PhysicalElementPtr terminal2 = CreateTemplateObject(*model, categoryId, DgnConeDetail(DPoint3d::From(0.25, 0.75, cubeWidth), DPoint3d::From(0.25, 0.75, cubeWidth+terminalHeight), terminalRadius, terminalRadius, CAPPED));
    PhysicalElementPtr terminal3 = CreateTemplateObject(*model, categoryId, DgnConeDetail(DPoint3d::From(0.75, 0.75, cubeWidth), DPoint3d::From(0.75, 0.75, cubeWidth+terminalHeight), terminalRadius, terminalRadius, CAPPED));
    PhysicalElementPtr terminal4 = CreateTemplateObject(*model, categoryId, DgnConeDetail(DPoint3d::From(0.75, 0.25, cubeWidth), DPoint3d::From(0.75, 0.25, cubeWidth+terminalHeight), terminalRadius, terminalRadius, CAPPED));
    BeAssert(cube.IsValid() && terminal1.IsValid() && terminal2.IsValid() && terminal3.IsValid() && terminal4.IsValid());

    BeAssert(cube->Insert().IsValid() && terminal1->Insert().IsValid() && terminal2->Insert().IsValid() && terminal3->Insert().IsValid() && terminal4->Insert().IsValid());
    return model;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreateTemplateObject(PhysicalModelR model, DgnCategoryId categoryId, DgnBoxDetailCR boxDetail)
    {
    ISolidPrimitivePtr box = ISolidPrimitive::CreateDgnBox(boxDetail);
    return box.IsValid() ? CreateTemplateObject(model, categoryId, *box) : nullptr;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreateTemplateObject(PhysicalModelR model, DgnCategoryId categoryId, DgnConeDetailCR coneDetail)
    {
    ISolidPrimitivePtr cone = ISolidPrimitive::CreateDgnCone(coneDetail);
    return cone.IsValid() ? CreateTemplateObject(model, categoryId, *cone) : nullptr;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreateTemplateObject(PhysicalModelR model, DgnCategoryId categoryId, DgnTorusPipeDetailCR torusPipeDetail)
    {
    ISolidPrimitivePtr torusPipe = ISolidPrimitive::CreateDgnTorusPipe(torusPipeDetail);
    return torusPipe.IsValid() ? CreateTemplateObject(model, categoryId, *torusPipe) : nullptr;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreateTemplateObject(PhysicalModelR model, DgnCategoryId categoryId, ISolidPrimitiveCR solid)
    {
    GeometryBuilderPtr builder = GeometryBuilder::Create(model, categoryId, DPoint3d::FromZero());
    if (!builder.IsValid() || !builder->Append(solid))
        return nullptr;

    return CreateTemplateObject(model, categoryId, *builder);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
PhysicalElementPtr TypeTests::CreateTemplateObject(PhysicalModelR model, DgnCategoryId categoryId, GeometryBuilderR builder)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(generic_ElementHandler::GenericPhysicalObjectHandler::GetHandler());
    GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(model, categoryId);
    if (!element.IsValid())
        return nullptr;

    return (BentleyStatus::SUCCESS == builder.Finish(*element)) ? element.get() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnGeometryPartId TypeTests::QueryGeometryPartId(TypeDefinitionElementCR type)
    {
    DgnCode geometryPartCode = CodeSpec::CreateCode(BIS_CODESPEC_GeometryPart, type, type.GetCode().GetValue());
    if (!geometryPartCode.IsValid())
        return DgnGeometryPartId();

    DgnDbR db = type.GetDgnDb();
    DgnGeometryPartId geometryPartId = DgnGeometryPart::QueryGeometryPartId(db, geometryPartCode);
    if (geometryPartId.IsValid())
        return geometryPartId; // geometry part already exists

    RecipeElementCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return DgnGeometryPartId(); // no recipe to create the type

    bool is3d = nullptr != recipe->ToPhysicalRecipe();
    DgnGeometryPartPtr geometryPart = DgnGeometryPart::Create(db, geometryPartCode);
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(db, is3d);
    if (!geometryPart.IsValid() || !builder.IsValid())
        return DgnGeometryPartId();

    DgnModelPtr templateModel = recipe->GetSubModel();
    if (!templateModel.IsValid())
        return DgnGeometryPartId();

    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_GeometricElement), *templateModel))
        {
        GeometricElementCPtr templateElement = db.Elements().Get<GeometricElement>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return DgnGeometryPartId();

        if ((IsNestedTypeLocation(*templateElement)) || (IsInstanceSpecific(*templateElement)))
            continue;

        for (GeometryCollection::Iterator const& geometryEntry : GeometryCollection(*templateElement->ToGeometrySource()))
            {
            GeometricPrimitivePtr geometry = geometryEntry.GetGeometryPtr();
            if (!geometry.IsValid())
                continue;

            builder->Append(*geometry);
            }
        }

    builder->Finish(*geometryPart);
    db.Elements().Insert<DgnGeometryPart>(*geometryPart);
    return geometryPart->GetId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
ElementIterator TypeTests::MakeElementIteratorWhereModel(Utf8CP elementClassName, DgnModelCR model)
    {
    Utf8PrintfString whereClause("WHERE Model.Id=%" PRIu64, model.GetModelId());
    return model.GetDgnDb().Elements().MakeIterator(elementClassName, whereClause.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnDbStatus TypeTests::AppendInstanceSpecificGeometry(GeometryBuilderR builder, DgnElementCR element, GraphicalType2dCR type, DPoint2dCR origin)
    {
    DgnGeometryPartId geometryPartId = QueryGeometryPartId(type);
    if (geometryPartId.IsValid())
        {
        if (!builder.Append(geometryPartId, origin))
            return DgnDbStatus::BadRequest;
        }

    GraphicalRecipe2dCPtr recipe = type.GetRecipe();
    if (!recipe.IsValid())
        return DgnDbStatus::BadRequest;

    GeometricModelPtr templateModel = recipe->GetSub<GeometricModel>();
    if (!templateModel.IsValid())
        return DgnDbStatus::BadRequest;

    DgnDbR db = type.GetDgnDb();
    for (ElementIteratorEntryCR elementEntry : MakeElementIteratorWhereModel(BIS_SCHEMA(BIS_CLASS_GraphicalElement2d), *templateModel))
        {
        NestedTypeLocation2dCPtr nestedTypeLocation = db.Elements().Get<NestedTypeLocation2d>(elementEntry.GetElementId());
        if (nestedTypeLocation.IsValid())
            {
            GraphicalType2dCPtr nestedType = nestedTypeLocation->GetNestedType();
            DPoint2d location = nestedTypeLocation->GetLocation();
            location.Add(origin);
            DgnDbStatus status = AppendInstanceSpecificGeometry(builder, element, *nestedType, location);
            if (DgnDbStatus::Success != status)
                return status;

            continue;
            }

        DrawingGraphicCPtr templateElement = db.Elements().Get<DrawingGraphic>(elementEntry.GetElementId());
        if (!templateElement.IsValid())
            return DgnDbStatus::BadRequest;

        if (!IsInstanceSpecific(*templateElement))
            continue;

        for (GeometryCollection::Iterator const& geometryEntry : GeometryCollection(*templateElement))
            {
            GeometricPrimitivePtr geometry = geometryEntry.GetGeometryPtr();
            if (!geometry.IsValid())
                continue;

            if (GeometryCollection::Iterator::EntryType::TextString == geometryEntry.GetEntryType())
                {
                Utf8CP expression = GetValueExpression(*templateElement);
                if (expression && *expression)
                    {
                    TextStringPtr textString = geometry->GetAsTextString();
                    if (textString.IsValid())
                        {
                        ECN::ECValue value;
                        Utf8String valueAsString = "???";

                        if ((DgnDbStatus::Success == element.GetPropertyValue(value, expression)) && !value.IsNull())
                            valueAsString = value.ToString();

                        textString->SetText(valueAsString.c_str());
                        geometry = GeometricPrimitive::Create(*textString);
                        }
                    }
                }

            geometry->TransformInPlace(Transform::From(DPoint3d::From(origin)));
            builder.Append(*geometry);
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
bool TypeTests::IsNestedTypeLocation(DgnElementCR element)
    {
    if (nullptr != dynamic_cast<NestedTypeLocation2dCP>(&element))
        return true;

    // WIP: check for 3D nested types also!

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
bool TypeTests::IsInstanceSpecific(DgnElementCR element)
    {
    return element.GetUserProperty(USERPROP_IsInstanceSpecific()).GetValueBoolean();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
void TypeTests::SetInstanceSpecific(DgnElementR element, bool isInstanceSpecific)
    {
    element.GetUserProperty(USERPROP_IsInstanceSpecific()).SetValueBoolean(isInstanceSpecific);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
Utf8CP TypeTests::GetValueExpression(DgnElementCR element)
    {
    return element.GetUserProperty(USERPROP_ValueExpression()).GetValueText();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
void TypeTests::SetValueExpression(DgnElementR element, Utf8CP expression)
    {
    element.GetUserProperty(USERPROP_ValueExpression()).SetValueText(expression);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2017
//---------------------------------------------------------------------------------------
DgnViewId TypeTests::InsertSpatialView(SpatialModelR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();

    ModelSelectorPtr modelSelector = new ModelSelector(db, "");
    modelSelector->AddModel(model.GetModelId());

    OrthographicViewDefinition view(db, name, *new CategorySelector(db,""), *new DisplayStyle3d(db,""), *modelSelector);

    for (ElementIteratorEntryCR categoryEntry : SpatialCategory::MakeIterator(db))
        view.GetCategorySelector().AddCategory(categoryEntry.GetId<DgnCategoryId>());

    view.SetStandardViewRotation(StandardView::Iso);
    view.SetSource(Dgn::DgnViewSource::Generated);
    view.LookAtVolume(model.QueryModelRange());
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
    ASSERT_EQ(BentleyStatus::SUCCESS, m_db->Schemas().CreateECClassViewsInDb());
    DgnCategoryId tbdCategory2d = DgnDbTestUtils::InsertDrawingCategory(*m_db, "ToBeDetermined"); // indicates that the real category won't be known until placement
    DgnCategoryId blueCategory2d = DgnDbTestUtils::InsertDrawingCategory(*m_db, "BlueCategory2d", ColorDef::Blue());
    DgnCategoryId redCategory2d = DgnDbTestUtils::InsertDrawingCategory(*m_db, "RedCategory2d", ColorDef::Red());
    DgnCategoryId greenCategory3d = DgnDbTestUtils::InsertSpatialCategory(*m_db, "GreenCategory3d", ColorDef::Green());

    DefinitionModelPtr typeModel2d = DgnDbTestUtils::InsertDefinitionModel(*m_db, "2D Types");
    GraphicalRecipe2dCPtr recipe2A = InsertRecipe2d(*typeModel2d, "Recipe2-A", tbdCategory2d);
    GraphicalRecipe2dCPtr recipe2B = InsertRecipe2d(*typeModel2d, "Recipe2-B", tbdCategory2d);
    GraphicalType2dCPtr type2A1 = InsertType2d(*typeModel2d, "Type2-A-1", *recipe2A);
    GraphicalType2dCPtr type2A2 = InsertType2d(*typeModel2d, "Type2-A-2", *recipe2A);
    GraphicalType2dCPtr type2B1 = InsertType2d(*typeModel2d, "Type2-B-1", *recipe2B);

    DrawingModelPtr templateModel2A = InsertTemplate2A(*recipe2A, tbdCategory2d);
    DrawingModelPtr templateModel2B = InsertTemplate2B(*recipe2B, tbdCategory2d, *type2A1);

    DefinitionModelPtr typeModel3d = DgnDbTestUtils::InsertDefinitionModel(*m_db, "3D Types");
    PhysicalRecipeCPtr recipe3A = InsertRecipe3d(*typeModel3d, "Recipe3-A", greenCategory3d);
    PhysicalRecipeCPtr recipe3B = InsertRecipe3d(*typeModel3d, "Recipe3-B", greenCategory3d);
    PhysicalTypeCPtr type3A1 = InsertType3d(*typeModel3d, "Type3-A-1", *recipe3A);
    PhysicalTypeCPtr type3B1 = InsertType3d(*typeModel3d, "Type3-B-1", *recipe3B);
    PhysicalTypeCPtr type3B2 = InsertType3d(*typeModel3d, "Type3-B-2", *recipe3B);

    PhysicalModelPtr templateModel3A = InsertTemplate3A(*recipe3A);
    PhysicalModelPtr templateModel3B = InsertTemplate3B(*recipe3B, greenCategory3d);

    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "2D Instances");
    DrawingModelPtr instanceModel2d = DgnDbTestUtils::InsertDrawingModel(*drawing);

    for (int i=0; i<5; i++)
        {
        DPoint2d origin = DPoint2d::From(i*3+1, i*3+1);
        Utf8PrintfString userLabel("Symbol%" PRIi32, i);
        DrawingGraphicPtr instance2B1 = CreateDrawingGraphic(*instanceModel2d, blueCategory2d, *type2B1, origin, userLabel.c_str());
        ASSERT_TRUE(instance2B1.IsValid());
        ASSERT_TRUE(instance2B1->Insert().IsValid());

        origin.Add(DPoint2d::From(0, 3));
        DrawingGraphicPtr instance2A1 = CreateDrawingGraphic(*instanceModel2d, redCategory2d, *type2A1, origin);
        ASSERT_TRUE(instance2A1.IsValid());
        ASSERT_TRUE(instance2A1->Insert().IsValid());
        }

    DrawingViewDefinitionPtr view2d = DgnDbTestUtils::InsertDrawingView(*instanceModel2d, "2D View");
    view2d->LookAtVolume(instanceModel2d->QueryModelRange());
    ASSERT_TRUE(view2d->Update().IsValid());

    PhysicalModelPtr instanceModel3d = DgnDbTestUtils::InsertPhysicalModel(*m_db, "3D Instances");

    for (int i=0; i<5; i++)
        {
        DPoint3d origin = DPoint3d::From(i*2, 0.0, i*2);
        Utf8PrintfString userLabel("Equipment%" PRIi32, i);
        PhysicalElementPtr element = CreatePhysicalObject(*instanceModel3d, *type3B1, origin, YawPitchRollAngles(), userLabel.c_str());
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());

        origin.Add(DPoint3d::From(0, 3, 0));
        DgnDbStatus status = CloneTemplate3d(*instanceModel3d, *type3A1, origin, YawPitchRollAngles());
        ASSERT_EQ(DgnDbStatus::Success, status);
        }

    InsertSpatialView(*instanceModel3d, "3D View");
    }
