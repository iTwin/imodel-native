/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AnnotationTestFixture.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/Annotations/LegacyTextAnnotationJson.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* Tests LegacyTextAnnotationJson, which converts the deprecated BisCore:TextAnnotationData aspect to ID-free JSON.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct LegacyTextAnnotationJsonTest : DgnDbTestFixture
{
    DgnModelId m_modelId;
    DgnCategoryId m_categoryId;

    void SetupDrawing()
        {
        SetupSeedProject();
        m_categoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "LegacyTextCategory");
        DocumentListModelPtr listModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "LegacyTextDrawings");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*listModel, "LegacyTextDrawing");
        m_modelId = DgnDbTestUtils::InsertDrawingModel(*drawing)->GetModelId();
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DgnElementId insertTextStyle(DgnDbR db, Utf8CP name, double height, ColorDef color, bool isBold = false, bool isItalic = false, bool isUnderlined = false)
    {
    AnnotationTextStyle style(db.GetDictionaryModel());
    style.SetName(name);
    style.SetHeight(height);
    style.SetColorType(AnnotationColorType::RGBA);
    style.SetColorValue(color);
    style.SetFontId(db.Fonts().GetId(FontType::TrueType, "Arial"));
    style.SetIsBold(isBold);
    style.SetIsItalic(isItalic);
    style.SetIsUnderlined(isUnderlined);
    AnnotationTextStyleCPtr inserted = style.Insert();
    EXPECT_TRUE(inserted.IsValid());
    return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DgnElementId insertFrameStyle(DgnDbR db, Utf8CP name, AnnotationFrameType type, bool isFillEnabled, bool isStrokeEnabled)
    {
    AnnotationFrameStyle style(db.GetDictionaryModel());
    style.SetName(name);
    style.SetType(type);
    style.SetHorizontalPadding(0.25);
    style.SetVerticalPadding(0.5);
    style.SetIsFillEnabled(isFillEnabled);
    style.SetFillColorType(AnnotationColorType::RGBA);
    style.SetFillColorValue(ColorDef(0xff, 0xff, 0x00));
    style.SetFillTransparency(0.5);
    style.SetIsStrokeEnabled(isStrokeEnabled);
    style.SetStrokeColorType(AnnotationColorType::RGBA);
    style.SetStrokeColorValue(ColorDef(0x00, 0x00, 0xff));
    style.SetStrokeWeight(2);
    AnnotationFrameStyleCPtr inserted = style.Insert();
    EXPECT_TRUE(inserted.IsValid());
    return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DgnElementId insertLeaderStyle(DgnDbR db, Utf8CP name, AnnotationLeaderLineType lineType, AnnotationLeaderTerminatorType terminatorType)
    {
    AnnotationLeaderStyle style(db.GetDictionaryModel());
    style.SetName(name);
    style.SetLineType(lineType);
    style.SetLineColorType(AnnotationColorType::RGBA);
    style.SetLineColorValue(ColorDef(0xff, 0x00, 0x00));
    style.SetLineWeight(1);
    style.SetTerminatorType(terminatorType);
    style.SetTerminatorColorType(AnnotationColorType::ByCategory);
    style.SetTerminatorWeight(3);
    style.SetTerminatorScaleFactor(1.5);
    AnnotationLeaderStyleCPtr inserted = style.Insert();
    EXPECT_TRUE(inserted.IsValid());
    return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static AnnotationLeaderPtr createLeader(DgnDbR db, DgnElementId styleId, uint32_t attachmentId, DPoint3dCR target)
    {
    AnnotationLeaderPtr leader = AnnotationLeader::Create(db, styleId);
    leader->SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
    leader->SetSourceAttachmentDataForId(attachmentId);
    leader->SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
    leader->SetTargetAttachmentDataForPhysicalPoint(target);
    return leader;
    }

//---------------------------------------------------------------------------------------
// Pass nullptr for annotation to insert an element without a TextAnnotationData aspect.
//---------------------------------------------------------------------------------------
static DgnElementId insertAnnotation2d(DgnDbR db, DgnModelId modelId, DgnCategoryId categoryId, Utf8CP label, TextAnnotationCP annotation, DPoint2dCR origin = DPoint2d::From(0, 0))
    {
    TextAnnotation2dPtr element = new TextAnnotation2d(TextAnnotation2d::CreateParams(db, modelId, TextAnnotation2d::QueryDgnClassId(db), categoryId, Placement2d(origin, AngleInDegrees())));
    element->SetUserLabel(label);
    if (nullptr != annotation)
        element->SetAnnotation(annotation);

    TextAnnotation2dCPtr inserted = element->Insert();
    EXPECT_TRUE(inserted.IsValid()) << label;
    return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static TextAnnotation createSimpleAnnotation(DgnDbR db, DgnElementId textStyleId, Utf8CP text)
    {
    TextAnnotation annotation(db);
    annotation.SetText(AnnotationTextBlock::Create(db, textStyleId, text).get());
    return annotation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ReturnsNotFoundWithoutLegacyContent)
    {
    SetupDrawing();

    DgnElementId noAspectId = insertAnnotation2d(*m_db, m_modelId, m_categoryId, "noAspect", nullptr);
    ASSERT_TRUE(noAspectId.IsValid());

    BeJsDocument json;
    EXPECT_EQ(DgnDbStatus::NotFound, LegacyTextAnnotationJson::Read(json, *m_db, noAspectId));
    EXPECT_EQ(DgnDbStatus::NotFound, LegacyTextAnnotationJson::Read(json, *m_db, DgnElementId((uint64_t)0x7fffffff)));
    EXPECT_EQ(DgnDbStatus::NotFound, LegacyTextAnnotationJson::Read(json, *m_db, m_db->Elements().GetRootSubjectId()));
    EXPECT_EQ(DgnDbStatus::InvalidId, LegacyTextAnnotationJson::Read(json, *m_db, DgnElementId()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ResolvesTextStyle)
    {
    SetupDrawing();

    DgnElementId styleId = insertTextStyle(*m_db, "BasicStyle", 2.5, ColorDef(0xff, 0x00, 0x00), false, true, true);
    TextAnnotation annotation = createSimpleAnnotation(*m_db, styleId, "Hello");
    DgnElementId elementId = insertAnnotation2d(*m_db, m_modelId, m_categoryId, "basic", &annotation);

    BeJsDocument json;
    Utf8String errorMessage;
    ASSERT_EQ(DgnDbStatus::Success, LegacyTextAnnotationJson::Read(json, *m_db, elementId, &errorMessage)) << errorMessage.c_str();

    BeJsConst root(json);
    EXPECT_EQ((int32_t)LegacyTextAnnotationJson::CURRENT_VERSION, root["version"].asInt());
    EXPECT_FALSE(root.isMember("frame"));
    ASSERT_TRUE(root["leaders"].isArray());
    EXPECT_EQ(0u, root["leaders"].size());
    ASSERT_TRUE(root["issues"].isArray());
    EXPECT_EQ(0u, root["issues"].size()) << root["issues"].Stringify().c_str();

    BeJsConst textBlock = root["textBlock"];
    EXPECT_EQ(0.0, textBlock["documentWidth"].asDouble(-1.0));
    EXPECT_STREQ("left", textBlock["justification"].asCString());
    ASSERT_EQ(1u, textBlock["paragraphs"].size());
    ASSERT_EQ(1u, textBlock["paragraphs"][0u]["runs"].size());

    BeJsConst run = textBlock["paragraphs"][0u]["runs"][0u];
    EXPECT_STREQ("text", run["type"].asCString());
    EXPECT_STREQ("Hello", run["content"].asCString());
    EXPECT_STREQ("none", run["subSuperScript"].asCString());

    BeJsConst style = run["style"];
    EXPECT_DOUBLE_EQ(2.5, style["height"].asDouble());
    EXPECT_STREQ("rgba", style["color"]["type"].asCString());
    EXPECT_EQ(ColorDef(0xff, 0x00, 0x00).GetValue(), style["color"]["value"].asUInt());
    EXPECT_STREQ("Arial", style["font"]["name"].asCString());
    EXPECT_STREQ("truetype", style["font"]["type"].asCString());
    EXPECT_FALSE(style["isBold"].asBool(true));
    EXPECT_TRUE(style["isItalic"].asBool());
    EXPECT_TRUE(style["isUnderlined"].asBool());
    EXPECT_STREQ("horizontal", style["stackedFractionType"].asCString());
    EXPECT_FALSE(style.isMember("styleId"));

    BeJsConst layout = root["layout"];
    ASSERT_TRUE(layout.isObject());
    ASSERT_EQ(2u, layout["textRange"]["low"].size());
    ASSERT_EQ(2u, layout["textRange"]["high"].size());
    EXPECT_GT(layout["textRange"]["high"][0u].asDouble(), layout["textRange"]["low"][0u].asDouble());
    EXPECT_GT(layout["textRange"]["high"][1u].asDouble(), layout["textRange"]["low"][1u].asDouble());
    EXPECT_FALSE(layout.isMember("frameRange"));
    EXPECT_EQ(0u, layout["leaderSourcePoints"].size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ConvertsRunsAndParagraphs)
    {
    SetupDrawing();

    DgnElementId regularId = insertTextStyle(*m_db, "Regular", 1.0, ColorDef(0x00, 0x80, 0x00));
    DgnElementId boldId = insertTextStyle(*m_db, "Bold", 3.0, ColorDef(0x00, 0x00, 0xff), true);

    AnnotationParagraphPtr paragraph1 = AnnotationParagraph::Create(*m_db, regularId);
    paragraph1->AppendRun(*AnnotationTextRun::Create(*m_db, regularId, "Regular "));
    paragraph1->AppendRun(*AnnotationTextRun::Create(*m_db, boldId, "bold"));
    AnnotationTextRunPtr superRun = AnnotationTextRun::Create(*m_db, regularId, "2");
    superRun->SetSubSuperScript(AnnotationTextRunSubSuperScript::SuperScript);
    paragraph1->AppendRun(*superRun);
    AnnotationTextRunPtr subRun = AnnotationTextRun::Create(*m_db, regularId, "i");
    subRun->SetSubSuperScript(AnnotationTextRunSubSuperScript::SubScript);
    subRun->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::IsBold, 1);
    paragraph1->AppendRun(*subRun);

    AnnotationParagraphPtr paragraph2 = AnnotationParagraph::Create(*m_db, regularId);
    paragraph2->AppendRun(*AnnotationFractionRun::Create(*m_db, regularId, "1", "2"));
    paragraph2->AppendRun(*AnnotationLineBreakRun::Create(*m_db, regularId));
    paragraph2->AppendRun(*AnnotationTextRun::Create(*m_db, regularId, "after break"));

    AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(*m_db, regularId);
    doc->AppendParagraph(*paragraph1);
    doc->AppendParagraph(*paragraph2);
    doc->SetDocumentWidth(10.0);
    doc->SetJustification(AnnotationTextBlock::HorizontalJustification::Center);
    // Height comes from the document level, regardless of run styles.
    doc->GetStyleOverridesR().SetRealProperty(AnnotationTextStyleProperty::Height, 4.0);

    TextAnnotation annotation(*m_db);
    annotation.SetText(doc.get());
    DgnElementId elementId = insertAnnotation2d(*m_db, m_modelId, m_categoryId, "runs", &annotation);

    BeJsDocument json;
    ASSERT_EQ(DgnDbStatus::Success, LegacyTextAnnotationJson::Read(json, *m_db, elementId));

    BeJsConst root(json);
    EXPECT_EQ(0u, root["issues"].size()) << root["issues"].Stringify().c_str();

    BeJsConst textBlock = root["textBlock"];
    EXPECT_DOUBLE_EQ(10.0, textBlock["documentWidth"].asDouble());
    EXPECT_STREQ("center", textBlock["justification"].asCString());
    ASSERT_EQ(2u, textBlock["paragraphs"].size());

    BeJsConst runs1 = textBlock["paragraphs"][0u]["runs"];
    ASSERT_EQ(4u, runs1.size());
    EXPECT_STREQ("Regular ", runs1[0u]["content"].asCString());
    EXPECT_FALSE(runs1[0u]["style"]["isBold"].asBool(true));
    EXPECT_EQ(ColorDef(0x00, 0x80, 0x00).GetValue(), runs1[0u]["style"]["color"]["value"].asUInt());
    EXPECT_STREQ("bold", runs1[1u]["content"].asCString());
    EXPECT_TRUE(runs1[1u]["style"]["isBold"].asBool());
    EXPECT_EQ(ColorDef(0x00, 0x00, 0xff).GetValue(), runs1[1u]["style"]["color"]["value"].asUInt());
    EXPECT_STREQ("super", runs1[2u]["subSuperScript"].asCString());
    EXPECT_STREQ("sub", runs1[3u]["subSuperScript"].asCString());
    EXPECT_TRUE(runs1[3u]["style"]["isBold"].asBool());

    for (BeJsConst::ArrayIndex i = 0; i < runs1.size(); ++i)
        EXPECT_DOUBLE_EQ(4.0, runs1[i]["style"]["height"].asDouble());

    BeJsConst runs2 = textBlock["paragraphs"][1u]["runs"];
    ASSERT_EQ(3u, runs2.size());
    EXPECT_STREQ("fraction", runs2[0u]["type"].asCString());
    EXPECT_STREQ("1", runs2[0u]["numerator"].asCString());
    EXPECT_STREQ("2", runs2[0u]["denominator"].asCString());
    EXPECT_STREQ("lineBreak", runs2[1u]["type"].asCString());
    EXPECT_TRUE(runs2[1u]["style"].isObject());
    EXPECT_STREQ("text", runs2[2u]["type"].asCString());
    EXPECT_STREQ("after break", runs2[2u]["content"].asCString());

    EXPECT_TRUE(root["layout"]["textRange"].isObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ConvertsFrameAndLeaders)
    {
    SetupDrawing();

    DgnElementId textStyleId = insertTextStyle(*m_db, "FramedText", 1.0, ColorDef(0, 0, 0));
    DgnElementId frameStyleId = insertFrameStyle(*m_db, "BoxFrame", AnnotationFrameType::Box, true, true);
    DgnElementId straightId = insertLeaderStyle(*m_db, "Straight", AnnotationLeaderLineType::Straight, AnnotationLeaderTerminatorType::ClosedArrow);
    DgnElementId curvedId = insertLeaderStyle(*m_db, "Curved", AnnotationLeaderLineType::Curved, AnnotationLeaderTerminatorType::OpenArrow);

    TextAnnotation annotation = createSimpleAnnotation(*m_db, textStyleId, "Framed");
    AnnotationFramePtr frame = AnnotationFrame::Create(*m_db, frameStyleId);
    frame->GetStyleOverridesR().SetRealProperty(AnnotationFrameStyleProperty::HorizontalPadding, 0.75);
    annotation.SetFrame(frame.get());
    annotation.GetLeadersR().push_back(createLeader(*m_db, straightId, 0, DPoint3d::From(10, 0, 0)));
    annotation.GetLeadersR().push_back(createLeader(*m_db, curvedId, 3, DPoint3d::From(-5, 5, 0)));

    DgnElementId elementId = insertAnnotation2d(*m_db, m_modelId, m_categoryId, "frameAndLeaders", &annotation);

    BeJsDocument json;
    ASSERT_EQ(DgnDbStatus::Success, LegacyTextAnnotationJson::Read(json, *m_db, elementId));

    BeJsConst root(json);
    EXPECT_EQ(0u, root["issues"].size()) << root["issues"].Stringify().c_str();

    BeJsConst frameJson = root["frame"];
    EXPECT_STREQ("box", frameJson["type"].asCString());
    EXPECT_DOUBLE_EQ(0.75, frameJson["horizontalPadding"].asDouble());
    EXPECT_DOUBLE_EQ(0.5, frameJson["verticalPadding"].asDouble());
    EXPECT_TRUE(frameJson["isFillEnabled"].asBool());
    EXPECT_EQ(ColorDef(0xff, 0xff, 0x00).GetValue(), frameJson["fillColor"]["value"].asUInt());
    EXPECT_DOUBLE_EQ(0.5, frameJson["fillTransparency"].asDouble());
    EXPECT_TRUE(frameJson["isStrokeEnabled"].asBool());
    EXPECT_FALSE(frameJson["isStrokeCloud"].asBool(true));
    EXPECT_EQ(ColorDef(0x00, 0x00, 0xff).GetValue(), frameJson["strokeColor"]["value"].asUInt());
    EXPECT_EQ(2u, frameJson["strokeWeight"].asUInt());

    BeJsConst leaders = root["leaders"];
    ASSERT_EQ(2u, leaders.size());
    EXPECT_STREQ("byId", leaders[0u]["sourceAttachment"]["type"].asCString());
    EXPECT_EQ(0u, leaders[0u]["sourceAttachment"]["id"].asUInt(99));
    EXPECT_DOUBLE_EQ(10.0, leaders[0u]["targetPoint"][0u].asDouble());
    EXPECT_STREQ("straight", leaders[0u]["lineType"].asCString());
    EXPECT_EQ(ColorDef(0xff, 0x00, 0x00).GetValue(), leaders[0u]["lineColor"]["value"].asUInt());
    EXPECT_EQ(1u, leaders[0u]["lineWeight"].asUInt());
    EXPECT_STREQ("closedArrow", leaders[0u]["terminatorType"].asCString());
    EXPECT_STREQ("byCategory", leaders[0u]["terminatorColor"]["type"].asCString());
    EXPECT_EQ(3u, leaders[0u]["terminatorWeight"].asUInt());
    EXPECT_DOUBLE_EQ(1.5, leaders[0u]["terminatorScaleFactor"].asDouble());
    EXPECT_EQ(3u, leaders[1u]["sourceAttachment"]["id"].asUInt());
    EXPECT_STREQ("curved", leaders[1u]["lineType"].asCString());
    EXPECT_STREQ("openArrow", leaders[1u]["terminatorType"].asCString());

    BeJsConst layout = root["layout"];
    ASSERT_TRUE(layout["frameRange"].isObject());
    // The frame is padded outward from the text.
    EXPECT_LT(layout["frameRange"]["low"][0u].asDouble(), layout["textRange"]["low"][0u].asDouble());
    EXPECT_GT(layout["frameRange"]["high"][0u].asDouble(), layout["textRange"]["high"][0u].asDouble());
    ASSERT_EQ(2u, layout["leaderSourcePoints"].size());
    ASSERT_EQ(3u, layout["leaderSourcePoints"][0u].size());
    // Attachment 0 is the middle of the right side of a box; attachment 3 is its top-left corner.
    EXPECT_NEAR(layout["frameRange"]["high"][0u].asDouble(), layout["leaderSourcePoints"][0u][0u].asDouble(), 1.0e-9);
    EXPECT_NEAR(layout["frameRange"]["low"][0u].asDouble(), layout["leaderSourcePoints"][1u][0u].asDouble(), 1.0e-9);
    EXPECT_NEAR(layout["frameRange"]["high"][1u].asDouble(), layout["leaderSourcePoints"][1u][1u].asDouble(), 1.0e-9);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ReportsMissingStyles)
    {
    SetupDrawing();

    DgnElementId textStyleId = insertTextStyle(*m_db, "ToBeDeleted", 2.0, ColorDef(0xff, 0x00, 0xff));
    DgnElementId frameStyleId = insertFrameStyle(*m_db, "FrameToBeDeleted", AnnotationFrameType::Box, false, true);
    DgnElementId leaderStyleId = insertLeaderStyle(*m_db, "LeaderToBeDeleted", AnnotationLeaderLineType::Straight, AnnotationLeaderTerminatorType::OpenArrow);

    TextAnnotation annotation = createSimpleAnnotation(*m_db, textStyleId, "Orphaned");
    annotation.SetFrame(AnnotationFrame::Create(*m_db, frameStyleId).get());
    annotation.GetLeadersR().push_back(createLeader(*m_db, leaderStyleId, 0, DPoint3d::From(5, 0, 0)));
    DgnElementId elementId = insertAnnotation2d(*m_db, m_modelId, m_categoryId, "missingStyles", &annotation);

    // Styles can only be deleted by a purge; geometry was already generated on insert, so the element keeps its graphics.
        {
        DgnDb::PurgeOperation purge(*m_db);
        ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(textStyleId));
        ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(frameStyleId));
        ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Delete(leaderStyleId));
        }

    BeJsDocument json;
    ASSERT_EQ(DgnDbStatus::Success, LegacyTextAnnotationJson::Read(json, *m_db, elementId));

    BeJsConst root(json);
    bset<Utf8String> codes;
    BeJsConst issues = root["issues"];
    for (BeJsConst::ArrayIndex i = 0; i < issues.size(); ++i)
        {
        codes.insert(issues[i]["code"].asString());
        EXPECT_FALSE(issues[i]["message"].asString().empty());
        }

    EXPECT_TRUE(codes.end() != codes.find("missingTextStyle"));
    EXPECT_TRUE(codes.end() != codes.find("missingFrameStyle"));
    EXPECT_TRUE(codes.end() != codes.find("missingLeaderStyle"));
    EXPECT_TRUE(codes.end() != codes.find("layoutFailed"));

    // The missing text style is reported once, even though the document, paragraph, and run all reference it.
    size_t missingTextStyleCount = 0;
    for (BeJsConst::ArrayIndex i = 0; i < issues.size(); ++i)
        {
        if (0 == strcmp("missingTextStyle", issues[i]["code"].asCString()))
            ++missingTextStyleCount;
        }
    EXPECT_EQ(1u, missingTextStyleCount);

    // Default style values are substituted so that the content is still converted.
    EXPECT_STREQ("Orphaned", root["textBlock"]["paragraphs"][0u]["runs"][0u]["content"].asCString());
    EXPECT_TRUE(root["textBlock"]["paragraphs"][0u]["runs"][0u]["style"]["height"].isNumeric());
    EXPECT_TRUE(root["frame"].isObject());
    EXPECT_EQ(1u, root["leaders"].size());
    EXPECT_FALSE(root.isMember("layout"));
    }

//---------------------------------------------------------------------------------------
// Covers each legacy feature in one iModel: run types, justifications, frame types, leaders, a scaled drawing, a 3d annotation, and a missing style.
//---------------------------------------------------------------------------------------
TEST_F(LegacyTextAnnotationJsonTest, ConvertsAllLegacyCases)
    {
    SetupSeedProject();
    DgnDbR db = *m_db;

    DgnCategoryId drawingCategoryId = DgnDbTestUtils::InsertDrawingCategory(db, "LegacyText");
    DgnCategoryId spatialCategoryId = DgnDbTestUtils::InsertSpatialCategory(db, "LegacyText3d");
    DocumentListModelPtr listModel = DgnDbTestUtils::InsertDocumentListModel(db, "LegacyTextDrawings");

    DrawingPtr unscaledDrawing = DgnDbTestUtils::InsertDrawing(*listModel, "Unscaled");
    DgnModelId unscaledModelId = DgnDbTestUtils::InsertDrawingModel(*unscaledDrawing)->GetModelId();

    DrawingPtr scaledDrawing = Drawing::Create(*listModel, "Scaled50");
    ASSERT_EQ(DgnDbStatus::Success, scaledDrawing->SetPropertyValue("ScaleFactor", 50.0));
    ASSERT_TRUE(scaledDrawing->Insert().IsValid());
    DgnModelId scaledModelId = DgnDbTestUtils::InsertDrawingModel(*scaledDrawing)->GetModelId();

    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(db, "LegacyText3d");

    // Styles
    DgnElementId regularId = insertTextStyle(db, "Regular", 1.0, ColorDef(0x00, 0x00, 0x00));
    DgnElementId emphasisId = insertTextStyle(db, "Emphasis", 1.5, ColorDef(0xc0, 0x00, 0x00), true, true, true);
    DgnElementId scaledTextId = insertTextStyle(db, "ScaledText", 50.0, ColorDef(0x00, 0x00, 0x80));
    DgnElementId deletedTextId = insertTextStyle(db, "DeletedText", 2.0, ColorDef(0x80, 0x80, 0x80));

    AnnotationTextStyle diagonalStyle(db.GetDictionaryModel());
    diagonalStyle.SetName("DiagonalFraction");
    diagonalStyle.SetHeight(1.0);
    diagonalStyle.SetStackedFractionType(AnnotationStackedFractionType::DiagonalBar);
    diagonalStyle.SetStackedFractionScale(0.6);
    AnnotationTextStyleCPtr insertedDiagonalStyle = diagonalStyle.Insert();
    ASSERT_TRUE(insertedDiagonalStyle.IsValid());
    DgnElementId diagonalId = insertedDiagonalStyle->GetElementId();

    DgnElementId boxFrameId = insertFrameStyle(db, "BoxFillStroke", AnnotationFrameType::Box, true, true);
    DgnElementId circleFrameId = insertFrameStyle(db, "CircleFillStroke", AnnotationFrameType::Circle, true, true);
    DgnElementId ellipseFrameId = insertFrameStyle(db, "EllipseStroke", AnnotationFrameType::Ellipse, false, true);
    DgnElementId invisibleFrameId = insertFrameStyle(db, "InvisibleBoxFill", AnnotationFrameType::InvisibleBox, true, false);
    DgnElementId straightLeaderId = insertLeaderStyle(db, "StraightClosedArrow", AnnotationLeaderLineType::Straight, AnnotationLeaderTerminatorType::ClosedArrow);
    DgnElementId curvedLeaderId = insertLeaderStyle(db, "CurvedOpenArrow", AnnotationLeaderLineType::Curved, AnnotationLeaderTerminatorType::OpenArrow);

    bvector<DgnElementId> withContent;
    double y = 0.0;
    auto nextOrigin = [&y]() { y -= 10.0; return DPoint2d::From(0.0, y); };

    // Multi-paragraph text with mixed run styles, sub/superscript, fraction, and line break.
        {
        AnnotationParagraphPtr paragraph1 = AnnotationParagraph::Create(db, regularId);
        paragraph1->AppendRun(*AnnotationTextRun::Create(db, regularId, "Regular and "));
        paragraph1->AppendRun(*AnnotationTextRun::Create(db, emphasisId, "emphasis"));
        AnnotationTextRunPtr overridden = AnnotationTextRun::Create(db, regularId, " red override");
        overridden->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorType, (int64_t)AnnotationColorType::RGBA);
        overridden->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::ColorValue, (int64_t)ColorDef(0xff, 0x00, 0x00).GetValue());
        paragraph1->AppendRun(*overridden);

        AnnotationParagraphPtr paragraph2 = AnnotationParagraph::Create(db, regularId);
        paragraph2->AppendRun(*AnnotationTextRun::Create(db, regularId, "x"));
        AnnotationTextRunPtr superRun = AnnotationTextRun::Create(db, regularId, "2");
        superRun->SetSubSuperScript(AnnotationTextRunSubSuperScript::SuperScript);
        paragraph2->AppendRun(*superRun);
        paragraph2->AppendRun(*AnnotationTextRun::Create(db, regularId, " + H"));
        AnnotationTextRunPtr subRun = AnnotationTextRun::Create(db, regularId, "2");
        subRun->SetSubSuperScript(AnnotationTextRunSubSuperScript::SubScript);
        paragraph2->AppendRun(*subRun);
        paragraph2->AppendRun(*AnnotationTextRun::Create(db, regularId, "O"));

        AnnotationParagraphPtr paragraph3 = AnnotationParagraph::Create(db, regularId);
        paragraph3->AppendRun(*AnnotationTextRun::Create(db, regularId, "Fractions "));
        paragraph3->AppendRun(*AnnotationFractionRun::Create(db, regularId, "1", "2"));
        paragraph3->AppendRun(*AnnotationTextRun::Create(db, regularId, " and "));
        paragraph3->AppendRun(*AnnotationFractionRun::Create(db, diagonalId, "3", "4"));
        paragraph3->AppendRun(*AnnotationLineBreakRun::Create(db, regularId));
        paragraph3->AppendRun(*AnnotationTextRun::Create(db, regularId, "after line break"));

        AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(db, regularId);
        doc->AppendParagraph(*paragraph1);
        doc->AppendParagraph(*paragraph2);
        doc->AppendParagraph(*paragraph3);

        TextAnnotation annotation(db);
        annotation.SetText(doc.get());
        withContent.push_back(insertAnnotation2d(db, unscaledModelId, drawingCategoryId, "mixedRuns", &annotation, nextOrigin()));
        }

    // Wrapped text and each justification.
    struct { AnnotationTextBlock::HorizontalJustification m_justification; Utf8CP m_label; } justifications[] =
        {
        { AnnotationTextBlock::HorizontalJustification::Left, "documentWidthLeft" },
        { AnnotationTextBlock::HorizontalJustification::Center, "documentWidthCenter" },
        { AnnotationTextBlock::HorizontalJustification::Right, "documentWidthRight" },
        };
    for (auto const& justification : justifications)
        {
        AnnotationTextBlockPtr doc = AnnotationTextBlock::Create(db, regularId, "This text is long enough to wrap onto several lines within the document width.");
        doc->SetDocumentWidth(12.0);
        doc->SetJustification(justification.m_justification);
        TextAnnotation annotation(db);
        annotation.SetText(doc.get());
        withContent.push_back(insertAnnotation2d(db, unscaledModelId, drawingCategoryId, justification.m_label, &annotation, nextOrigin()));
        }

    // Frames, with leaders using several attachment IDs.
    struct { DgnElementId m_frameStyleId; Utf8CP m_label; DgnElementId m_leaderStyleId; uint32_t m_attachmentIds[3]; } frames[] =
        {
        { boxFrameId, "frameBox", straightLeaderId, { 0, 3, 6 } },
        { circleFrameId, "frameCircle", curvedLeaderId, { 0, 1, 2 } },
        { ellipseFrameId, "frameEllipse", straightLeaderId, { 1, 3, 3 } },
        { invisibleFrameId, "frameInvisibleBox", curvedLeaderId, { 2, 5, 7 } },
        };
    for (auto const& frameCase : frames)
        {
        TextAnnotation annotation = createSimpleAnnotation(db, regularId, frameCase.m_label);
        annotation.SetFrame(AnnotationFrame::Create(db, frameCase.m_frameStyleId).get());
        for (uint32_t attachmentId : frameCase.m_attachmentIds)
            annotation.GetLeadersR().push_back(createLeader(db, frameCase.m_leaderStyleId, attachmentId, DPoint3d::From(8.0 + attachmentId, -3.0 - attachmentId, 0.0)));

        withContent.push_back(insertAnnotation2d(db, unscaledModelId, drawingCategoryId, frameCase.m_label, &annotation, nextOrigin()));
        }

    // Mixed straight and curved leaders on one frame.
        {
        TextAnnotation annotation = createSimpleAnnotation(db, regularId, "leadersMixed");
        annotation.SetFrame(AnnotationFrame::Create(db, boxFrameId).get());
        annotation.GetLeadersR().push_back(createLeader(db, straightLeaderId, 0, DPoint3d::From(15.0, 0.0, 0.0)));
        annotation.GetLeadersR().push_back(createLeader(db, curvedLeaderId, 4, DPoint3d::From(-10.0, -2.0, 0.0)));
        withContent.push_back(insertAnnotation2d(db, unscaledModelId, drawingCategoryId, "leadersMixed", &annotation, nextOrigin()));
        }

    // Text in a drawing with ScaleFactor 50; the stored height is not adjusted by the scale factor.
        {
        TextAnnotation annotation = createSimpleAnnotation(db, scaledTextId, "scaledDrawing");
        annotation.SetFrame(AnnotationFrame::Create(db, boxFrameId).get());
        withContent.push_back(insertAnnotation2d(db, scaledModelId, drawingCategoryId, "scaledDrawing", &annotation));
        }

    // A TextAnnotation3d.
        {
        TextAnnotation annotation = createSimpleAnnotation(db, emphasisId, "textAnnotation3d");
        annotation.SetFrame(AnnotationFrame::Create(db, ellipseFrameId).get());
        TextAnnotation3dPtr element = new TextAnnotation3d(TextAnnotation3d::CreateParams(db, physicalModel->GetModelId(), TextAnnotation3d::QueryDgnClassId(db), spatialCategoryId,
            Placement3d(DPoint3d::From(1.0, 2.0, 3.0), YawPitchRollAngles(), ElementAlignedBox3d())));
        element->SetUserLabel("textAnnotation3d");
        element->SetAnnotation(&annotation);
        TextAnnotation3dCPtr inserted = element->Insert();
        ASSERT_TRUE(inserted.IsValid());
        withContent.push_back(inserted->GetElementId());
        }

    // A text style that was deleted after the element was created.
        {
        TextAnnotation annotation = createSimpleAnnotation(db, deletedTextId, "missingTextStyle");
        withContent.push_back(insertAnnotation2d(db, unscaledModelId, drawingCategoryId, "missingTextStyle", &annotation, nextOrigin()));

        DgnDb::PurgeOperation purge(db);
        ASSERT_EQ(DgnDbStatus::Success, db.Elements().Delete(deletedTextId));
        }

    // An element without the legacy aspect.
    DgnElementId noAspectId = insertAnnotation2d(db, unscaledModelId, drawingCategoryId, "noAspect", nullptr, nextOrigin());
    ASSERT_TRUE(noAspectId.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());

    for (DgnElementId elementId : withContent)
        {
        ASSERT_TRUE(elementId.IsValid());
        BeJsDocument json;
        Utf8String errorMessage;
        EXPECT_EQ(DgnDbStatus::Success, LegacyTextAnnotationJson::Read(json, db, elementId, &errorMessage)) << elementId.ToHexStr().c_str() << " " << errorMessage.c_str();
        }

    BeJsDocument json;
    EXPECT_EQ(DgnDbStatus::NotFound, LegacyTextAnnotationJson::Read(json, db, noAspectId));
    }
