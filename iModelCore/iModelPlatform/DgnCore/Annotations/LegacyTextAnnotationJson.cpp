/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/Annotations/LegacyTextAnnotationJson.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFramePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h>

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

// Highest major versions of the legacy flatbuffer tables that this converter (and the legacy decoders it uses) understand.
static const uint8_t MAX_TEXT_ANNOTATION_MAJOR_VERSION = 1;
static const uint8_t MAX_TEXT_BLOCK_MAJOR_VERSION = 1;
static const uint8_t MAX_FRAME_MAJOR_VERSION = 1;
static const uint8_t MAX_LEADER_MAJOR_VERSION = 1;

namespace {

//=======================================================================================
// @bsiclass
//=======================================================================================
struct LegacyTextAnnotationConverter
{
private:
    struct Issue
        {
        Utf8String m_code;
        Utf8String m_message;
        Utf8String m_path;
        };

    DgnDbR m_db;
    // Issues are buffered and written last: with RapidJSON, adding members to an object can move its existing members.
    bvector<Issue> m_issues;
    bmap<DgnElementId, AnnotationTextStyleCPtr> m_textStyles;
    bmap<DgnElementId, AnnotationFrameStyleCPtr> m_frameStyles;
    bmap<DgnElementId, AnnotationLeaderStyleCPtr> m_leaderStyles;
    bset<uint64_t> m_reportedFonts;
    bool m_allTextStylesResolved = true;
    bool m_frameStyleResolved = true;

public:
    explicit LegacyTextAnnotationConverter(DgnDbR db) : m_db(db) {}

    void AddIssue(Utf8CP code, Utf8StringCR message, Utf8StringCR path)
        {
        m_issues.push_back({code, message, path});
        }

    void WriteIssues(BeJsValue json) const
        {
        json.SetEmptyArray();
        for (auto const& issue : m_issues)
            {
            BeJsValue issueJson = json.appendObject();
            issueJson["code"] = issue.m_code.c_str();
            issueJson["message"] = issue.m_message.c_str();
            if (!issue.m_path.empty())
                issueJson["path"] = issue.m_path.c_str();
            }
        }

    //---------------------------------------------------------------------------------------
    // Styles are cached by ID. A style that cannot be loaded is reported once, and replaced by a default style.
    //---------------------------------------------------------------------------------------
    AnnotationTextStyleCPtr GetTextStyle(AnnotationTextStyleId id, Utf8StringCR path)
        {
        auto found = m_textStyles.find(id);
        if (m_textStyles.end() != found)
            return found->second;

        AnnotationTextStyleCPtr style;
        if (id.IsValid())
            style = AnnotationTextStyle::Get(m_db, id);

        if (!style.IsValid())
            {
            m_allTextStylesResolved = false;
            AddIssue("missingTextStyle", Utf8PrintfString("AnnotationTextStyle %s could not be loaded; default text style values were used.", id.ToHexStr().c_str()), path);
            style = AnnotationTextStyle::Create(m_db.GetDictionaryModel()).get();
            }

        m_textStyles[id] = style;
        return style;
        }

    AnnotationFrameStyleCPtr GetFrameStyle(AnnotationFrameStyleId id, Utf8StringCR path)
        {
        auto found = m_frameStyles.find(id);
        if (m_frameStyles.end() != found)
            return found->second;

        AnnotationFrameStyleCPtr style;
        if (id.IsValid())
            style = AnnotationFrameStyle::Get(m_db, id);

        if (!style.IsValid())
            {
            m_frameStyleResolved = false;
            AddIssue("missingFrameStyle", Utf8PrintfString("AnnotationFrameStyle %s could not be loaded; default frame style values were used.", id.ToHexStr().c_str()), path);
            style = AnnotationFrameStyle::Create(m_db.GetDictionaryModel()).get();
            }

        m_frameStyles[id] = style;
        return style;
        }

    AnnotationLeaderStyleCPtr GetLeaderStyle(AnnotationLeaderStyleId id, Utf8StringCR path)
        {
        auto found = m_leaderStyles.find(id);
        if (m_leaderStyles.end() != found)
            return found->second;

        AnnotationLeaderStyleCPtr style;
        if (id.IsValid())
            style = AnnotationLeaderStyle::Get(m_db, id);

        if (!style.IsValid())
            {
            AddIssue("missingLeaderStyle", Utf8PrintfString("AnnotationLeaderStyle %s could not be loaded; default leader style values were used.", id.ToHexStr().c_str()), path);
            style = AnnotationLeaderStyle::Create(m_db.GetDictionaryModel()).get();
            }

        m_leaderStyles[id] = style;
        return style;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void WriteColor(BeJsValue json, AnnotationColorType type, ColorDef value, Utf8StringCR path)
        {
        switch (type)
            {
            case AnnotationColorType::ByCategory:
                json["type"] = "byCategory";
                return;
            case AnnotationColorType::RGBA:
                json["type"] = "rgba";
                json["value"] = value.GetValue();
                return;
            case AnnotationColorType::ViewBackground:
                json["type"] = "viewBackground";
                return;
            }

        AddIssue("invalidValue", Utf8PrintfString("Unknown color type %d; byCategory was used.", (int)type), path);
        json["type"] = "byCategory";
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void WriteTextStyle(BeJsValue json, AnnotationTextStyleCR style, Utf8StringCR path)
        {
        WriteColor(json["color"], style.GetColorType(), style.GetColorValue(), path);

        // An invalid font ID means "no font specified"; legacy rendering uses the fallback font, so omit "font" without an issue.
        FontId fontId = style.GetFontId();
        if (fontId.IsValid())
            {
            auto fontInfo = m_db.Fonts().GetFontInfo(fontId);
            if (nullptr != fontInfo)
                {
                BeJsValue font = json["font"];
                font["name"] = fontInfo->m_fontName.c_str();
                switch (fontInfo->m_type)
                    {
                    case FontType::Rsc: font["type"] = "rsc"; break;
                    case FontType::Shx: font["type"] = "shx"; break;
                    default: font["type"] = "truetype"; break;
                    }
                }
            else if (m_reportedFonts.insert(fontId.GetValue()).second)
                {
                AddIssue("missingFont", Utf8PrintfString("Font %s was not found in the font table; the font was omitted.", fontId.ToHexStr().c_str()), path);
                }
            }

        json["height"] = style.GetHeight();
        json["lineSpacingFactor"] = style.GetLineSpacingFactor();
        json["widthFactor"] = style.GetWidthFactor();
        json["isBold"] = style.IsBold();
        json["isItalic"] = style.IsItalic();
        json["isUnderlined"] = style.IsUnderlined();
        json["stackedFractionScale"] = style.GetStackedFractionScale();
        json["stackedFractionType"] = (AnnotationStackedFractionType::DiagonalBar == style.GetStackedFractionType()) ? "diagonal" : "horizontal";
        json["subScriptOffsetFactor"] = style.GetSubScriptOffsetFactor();
        json["subScriptScale"] = style.GetSubScriptScale();
        json["superScriptOffsetFactor"] = style.GetSuperScriptOffsetFactor();
        json["superScriptScale"] = style.GetSuperScriptScale();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void WriteTextBlock(BeJsValue json, AnnotationTextBlockCR doc, FB::AnnotationTextBlock const& fbDoc)
        {
        json["documentWidth"] = doc.GetDocumentWidth();
        switch (doc.GetJustification())
            {
            case AnnotationTextBlock::HorizontalJustification::Left: json["justification"] = "left"; break;
            case AnnotationTextBlock::HorizontalJustification::Center: json["justification"] = "center"; break;
            case AnnotationTextBlock::HorizontalJustification::Right: json["justification"] = "right"; break;
            default:
                AddIssue("invalidValue", Utf8PrintfString("Unknown justification %d; left was used.", (int)doc.GetJustification()), "textBlock");
                json["justification"] = "left";
                break;
            }

        // The legacy decoder silently skips runs of unknown types; detect them in the raw flatbuffer so they can be reported.
        if (fbDoc.has_paragraphs())
            {
            uint32_t iParagraph = 0;
            for (auto const& fbParagraph : *fbDoc.paragraphs())
                {
                if (fbParagraph->has_runs())
                    {
                    uint32_t iRun = 0;
                    for (auto const& fbRun : *fbParagraph->runs())
                        {
                        auto type = fbRun->type();
                        if (FB::AnnotationTextBlockRunType_Text != type && FB::AnnotationTextBlockRunType_Fraction != type && FB::AnnotationTextBlockRunType_LineBreak != type)
                            AddIssue("unknownRunType", Utf8PrintfString("Run %u has unsupported type %d and was skipped.", iRun, (int)type), Utf8PrintfString("textBlock.paragraphs[%u]", iParagraph));

                        ++iRun;
                        }
                    }

                ++iParagraph;
                }
            }

        AnnotationTextStyleCPtr docStyle = GetTextStyle(doc.GetStyleId(), "textBlock");

        BeJsValue paragraphs = json["paragraphs"];
        paragraphs.SetEmptyArray();

        uint32_t iParagraph = 0;
        for (auto const& paragraph : doc.GetParagraphs())
            {
            Utf8PrintfString paragraphPath("textBlock.paragraphs[%u]", iParagraph);
            AnnotationTextStyleCPtr paragraphStyle = GetTextStyle(paragraph->GetStyleId(), paragraphPath);

            BeJsValue paragraphJson = paragraphs.appendObject();
            BeJsValue runs = paragraphJson["runs"];
            runs.SetEmptyArray();

            uint32_t iRun = 0;
            for (auto const& run : paragraph->GetRuns())
                {
                Utf8PrintfString runPath("%s.runs[%u]", paragraphPath.c_str(), iRun);
                AnnotationTextStyleCPtr runStyle = GetTextStyle(run->GetStyleId(), runPath);

                BeJsValue runJson = runs.appendObject();
                switch (run->GetType())
                    {
                    case AnnotationRunType::Text:
                        {
                        auto const& textRun = static_cast<AnnotationTextRunCR>(*run);
                        runJson["type"] = "text";
                        runJson["content"] = textRun.GetContent().c_str();
                        switch (textRun.GetSubSuperScript())
                            {
                            case AnnotationTextRunSubSuperScript::SubScript: runJson["subSuperScript"] = "sub"; break;
                            case AnnotationTextRunSubSuperScript::SuperScript: runJson["subSuperScript"] = "super"; break;
                            default: runJson["subSuperScript"] = "none"; break;
                            }
                        break;
                        }
                    case AnnotationRunType::Fraction:
                        {
                        auto const& fractionRun = static_cast<AnnotationFractionRunCR>(*run);
                        runJson["type"] = "fraction";
                        runJson["numerator"] = fractionRun.GetNumeratorContent().c_str();
                        runJson["denominator"] = fractionRun.GetDenominatorContent().c_str();
                        break;
                        }
                    default:
                        runJson["type"] = "lineBreak";
                        break;
                    }

                // Same merge as AnnotationTextBlockDraw: height, line spacing, and width factor come from the document level; everything else from the run level.
                AnnotationTextStylePtr effectiveStyle = AnnotationTextStyle::CreateEffectiveStyle(*docStyle, doc.GetStyleOverrides(), *paragraphStyle, paragraph->GetStyleOverrides(), *runStyle, run->GetStyleOverrides());
                WriteTextStyle(runJson["style"], *effectiveStyle, runPath);

                ++iRun;
                }

            ++iParagraph;
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void WriteFrame(BeJsValue json, AnnotationFrameCR frame)
        {
        AnnotationFrameStyleCPtr style = GetFrameStyle(frame.GetStyleId(), "frame");
        AnnotationFrameStylePtr effective = style->CreateEffectiveStyle(frame.GetStyleOverrides());

        switch (effective->GetType())
            {
            case AnnotationFrameType::InvisibleBox: json["type"] = "invisibleBox"; break;
            case AnnotationFrameType::Box: json["type"] = "box"; break;
            case AnnotationFrameType::Circle: json["type"] = "circle"; break;
            case AnnotationFrameType::Ellipse: json["type"] = "ellipse"; break;
            default:
                AddIssue("invalidValue", Utf8PrintfString("Unknown frame type %d; invisibleBox was used.", (int)effective->GetType()), "frame");
                json["type"] = "invisibleBox";
                break;
            }

        json["horizontalPadding"] = effective->GetHorizontalPadding();
        json["verticalPadding"] = effective->GetVerticalPadding();
        json["isFillEnabled"] = effective->IsFillEnabled();
        WriteColor(json["fillColor"], effective->GetFillColorType(), effective->GetFillColorValue(), "frame");
        json["fillTransparency"] = effective->GetFillTransparency();
        json["isStrokeEnabled"] = effective->IsStrokeEnabled();
        json["isStrokeCloud"] = effective->IsStrokeCloud();
        WriteColor(json["strokeColor"], effective->GetStrokeColorType(), effective->GetStrokeColorValue(), "frame");
        json["strokeWeight"] = effective->GetStrokeWeight();
        json["cloudBulgeFactor"] = effective->GetCloudBulgeFactor();
        json["cloudDiameterFactor"] = effective->GetCloudDiameterFactor();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void WriteLeader(BeJsValue json, AnnotationLeaderCR leader, uint32_t index)
        {
        Utf8PrintfString path("leaders[%u]", index);
        AnnotationLeaderStyleCPtr style = GetLeaderStyle(leader.GetStyleId(), path);
        AnnotationLeaderStylePtr effective = style->CreateEffectiveStyle(leader.GetStyleOverrides());

        // The legacy decoder only accepts ID source attachments and physical-point target attachments.
        BeJsValue source = json["sourceAttachment"];
        source["type"] = "byId";
        source["id"] = leader.GetSourceAttachmentDataForId();

        DPoint3dCR target = leader.GetTargetAttachmentDataForPhysicalPoint();
        BeJsValue targetJson = json["targetPoint"];
        targetJson.SetEmptyArray();
        targetJson.appendValue() = target.x;
        targetJson.appendValue() = target.y;
        targetJson.appendValue() = target.z;

        switch (effective->GetLineType())
            {
            case AnnotationLeaderLineType::None: json["lineType"] = "none"; break;
            case AnnotationLeaderLineType::Straight: json["lineType"] = "straight"; break;
            case AnnotationLeaderLineType::Curved: json["lineType"] = "curved"; break;
            default:
                AddIssue("invalidValue", Utf8PrintfString("Unknown leader line type %d; straight was used.", (int)effective->GetLineType()), path);
                json["lineType"] = "straight";
                break;
            }

        WriteColor(json["lineColor"], effective->GetLineColorType(), effective->GetLineColorValue(), path);
        json["lineWeight"] = effective->GetLineWeight();

        switch (effective->GetTerminatorType())
            {
            case AnnotationLeaderTerminatorType::None: json["terminatorType"] = "none"; break;
            case AnnotationLeaderTerminatorType::OpenArrow: json["terminatorType"] = "openArrow"; break;
            case AnnotationLeaderTerminatorType::ClosedArrow: json["terminatorType"] = "closedArrow"; break;
            default:
                AddIssue("invalidValue", Utf8PrintfString("Unknown leader terminator type %d; none was used.", (int)effective->GetTerminatorType()), path);
                json["terminatorType"] = "none";
                break;
            }

        WriteColor(json["terminatorColor"], effective->GetTerminatorColorType(), effective->GetTerminatorColorValue(), path);
        json["terminatorWeight"] = effective->GetTerminatorWeight();
        json["terminatorScaleFactor"] = effective->GetTerminatorScaleFactor();
        }

    //---------------------------------------------------------------------------------------
    // Legacy layout dereferences styles by ID from the db, so it is only run when every referenced style was loaded.
    //---------------------------------------------------------------------------------------
    void WriteLayout(BeJsValue json, TextAnnotationCR annotation)
        {
        AnnotationTextBlockCP doc = annotation.GetTextCP();
        if (nullptr == doc || doc->IsEmpty())
            return;

        if (!m_allTextStylesResolved)
            {
            AddIssue("layoutFailed", "Layout was not computed because a text style could not be loaded.", "layout");
            return;
            }

        AnnotationTextBlockLayoutPtr docLayout = AnnotationTextBlockLayout::Create(*doc);
        DRange2dCR textRange = docLayout->GetLayoutRange();

        BeJsValue layout = json["layout"];
        WriteRange(layout["textRange"], textRange.low, textRange.high);

        AnnotationFrameCP frame = annotation.GetFrameCP();
        if (nullptr == frame)
            {
            layout["leaderSourcePoints"].SetEmptyArray();
            return;
            }

        if (!m_frameStyleResolved)
            {
            layout["leaderSourcePoints"].SetEmptyArray();
            AddIssue("layoutFailed", "Frame layout and leader source points were not computed because the frame style could not be loaded.", "layout");
            return;
            }

        AnnotationFrameLayoutPtr frameLayout = AnnotationFrameLayout::Create(*frame, *docLayout);
        DRange3d frameRange;
        if (frameLayout->GetFrameGeometry().GetRange(frameRange))
            WriteRange(layout["frameRange"], DPoint2d::From(frameRange.low.x, frameRange.low.y), DPoint2d::From(frameRange.high.x, frameRange.high.y));

        BeJsValue sourcePoints = layout["leaderSourcePoints"];
        sourcePoints.SetEmptyArray();
        for (auto const& leader : annotation.GetLeaders())
            {
            DPoint3d point;
            DVec3d direction;
            frameLayout->ComputePhysicalPointForAttachmentId(point, direction, leader->GetSourceAttachmentDataForId());

            BeJsValue pointJson = sourcePoints.appendArray();
            pointJson.appendValue() = point.x;
            pointJson.appendValue() = point.y;
            pointJson.appendValue() = point.z;
            }
        }

    static void WriteRange(BeJsValue json, DPoint2dCR low, DPoint2dCR high)
        {
        BeJsValue lowJson = json["low"];
        lowJson.SetEmptyArray();
        lowJson.appendValue() = low.x;
        lowJson.appendValue() = low.y;

        BeJsValue highJson = json["high"];
        highJson.SetEmptyArray();
        highJson.appendValue() = high.x;
        highJson.appendValue() = high.y;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static DgnDbStatus setError(Utf8StringP errorMessage, DgnDbStatus status, Utf8StringCR message)
    {
    if (nullptr != errorMessage)
        *errorMessage = message;

    return status;
    }

} // anonymous namespace

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus LegacyTextAnnotationJson::Read(BeJsValue out, DgnDbR db, DgnElementId elementId, Utf8StringP errorMessage)
    {
    if (!elementId.IsValid())
        return setError(errorMessage, DgnDbStatus::InvalidId, "Invalid element ID.");

    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT TextAnnotation FROM " BIS_SCHEMA(BIS_CLASS_TextAnnotationData) " WHERE Element.Id=?");
    if (!select.IsValid())
        return setError(errorMessage, DgnDbStatus::ReadError, "Failed to prepare the TextAnnotationData query.");

    select->BindId(1, elementId);
    if (BE_SQLITE_ROW != select->Step() || select->IsValueNull(0))
        return DgnDbStatus::NotFound;

    int dataSize = 0;
    ByteCP data = static_cast<ByteCP>(select->GetValueBlob(0, &dataSize));
    if (nullptr == data || dataSize <= 0)
        return DgnDbStatus::NotFound;

    Utf8String elementIdStr = elementId.ToHexStr();

    Verifier verifier(data, static_cast<size_t>(dataSize));
    if (!verifier.VerifyBuffer<FB::TextAnnotation>(nullptr))
        return setError(errorMessage, DgnDbStatus::ReadError, Utf8PrintfString("The TextAnnotationData of element %s is corrupt.", elementIdStr.c_str()));

    // Check every version ourselves: the legacy decoders assert on unexpected data, and TextAnnotationPersistence ignores nested decode failures.
    auto const& fbAnnotation = *GetRoot<FB::TextAnnotation>(data);
    if (!fbAnnotation.has_majorVersion())
        return setError(errorMessage, DgnDbStatus::ReadError, Utf8PrintfString("The TextAnnotationData of element %s has no version.", elementIdStr.c_str()));

    if (fbAnnotation.majorVersion() > MAX_TEXT_ANNOTATION_MAJOR_VERSION
        || (fbAnnotation.has_document() && fbAnnotation.document()->majorVersion() > MAX_TEXT_BLOCK_MAJOR_VERSION)
        || (fbAnnotation.has_frame() && fbAnnotation.frame()->majorVersion() > MAX_FRAME_MAJOR_VERSION))
        return setError(errorMessage, DgnDbStatus::VersionTooNew, Utf8PrintfString("The TextAnnotationData of element %s was written by a newer version.", elementIdStr.c_str()));

    if (fbAnnotation.has_leaders())
        {
        for (auto const& fbLeader : *fbAnnotation.leaders())
            {
            if (fbLeader->majorVersion() > MAX_LEADER_MAJOR_VERSION)
                return setError(errorMessage, DgnDbStatus::VersionTooNew, Utf8PrintfString("The TextAnnotationData of element %s was written by a newer version.", elementIdStr.c_str()));
            }
        }

    TextAnnotationPtr annotation = TextAnnotation::Create(db);
    AnnotationTextBlockPtr doc;
    if (fbAnnotation.has_document())
        {
        doc = AnnotationTextBlock::Create(db);
        if (SUCCESS != AnnotationTextBlockPersistence::DecodeFromFlatBuf(*doc, *fbAnnotation.document()))
            return setError(errorMessage, DgnDbStatus::ReadError, Utf8PrintfString("Failed to decode the text of element %s.", elementIdStr.c_str()));

        annotation->SetText(doc.get());
        }

    if (fbAnnotation.has_frame())
        {
        AnnotationFramePtr frame = AnnotationFrame::Create(db);
        if (SUCCESS != AnnotationFramePersistence::DecodeFromFlatBuf(*frame, *fbAnnotation.frame()))
            return setError(errorMessage, DgnDbStatus::ReadError, Utf8PrintfString("Failed to decode the frame of element %s.", elementIdStr.c_str()));

        annotation->SetFrame(frame.get());
        }

    if (fbAnnotation.has_leaders())
        {
        for (auto const& fbLeader : *fbAnnotation.leaders())
            {
            AnnotationLeaderPtr leader = AnnotationLeader::Create(db);
            if (SUCCESS != AnnotationLeaderPersistence::DecodeFromFlatBuf(*leader, *fbLeader))
                return setError(errorMessage, DgnDbStatus::ReadError, Utf8PrintfString("Failed to decode a leader of element %s.", elementIdStr.c_str()));

            annotation->GetLeadersR().push_back(leader);
            }
        }

    out.SetEmptyObject();
    out["version"] = LegacyTextAnnotationJson::CURRENT_VERSION;

    LegacyTextAnnotationConverter converter(db);

    if (doc.IsValid())
        converter.WriteTextBlock(out["textBlock"], *doc, *fbAnnotation.document());

    if (nullptr != annotation->GetFrameCP())
        converter.WriteFrame(out["frame"], *annotation->GetFrameCP());

    BeJsValue leaders = out["leaders"];
    leaders.SetEmptyArray();
    uint32_t iLeader = 0;
    for (auto const& leader : annotation->GetLeaders())
        converter.WriteLeader(leaders.appendObject(), *leader, iLeader++);

    converter.WriteLayout(out, *annotation);
    converter.WriteIssues(out["issues"]);

    return DgnDbStatus::Success;
    }
