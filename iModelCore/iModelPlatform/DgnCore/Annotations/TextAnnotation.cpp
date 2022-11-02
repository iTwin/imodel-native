/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatform/Annotations/TextAnnotationPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFramePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h>

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextAnnotation::TextAnnotation(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check TextAnnotation::Reset.
    m_dgndb = &project;
    m_origin.Zero();
    m_anchorPoint = AnchorPoint::LeftTop;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextAnnotationPtr TextAnnotation::Create(DgnDbR project, DgnElementId seedID)
    {
    auto annotation = TextAnnotation::Create(project);
    annotation->ApplySeed(seedID);

    return annotation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextAnnotation::CopyFrom(TextAnnotationCR rhs)
    {
    m_dgndb = rhs.m_dgndb;

    m_text = (rhs.m_text.IsValid() ? rhs.m_text->Clone() : NULL);
    m_frame = (rhs.m_frame.IsValid() ? rhs.m_frame->Clone() : NULL);

    m_leaders.clear();
    m_leaders.reserve(rhs.m_leaders.size());
    for (auto const& rhsLeader : rhs.m_leaders)
        m_leaders.push_back(rhsLeader->Clone());

    m_origin = rhs.m_origin;
    m_orientation = rhs.m_orientation;
    m_anchorPoint = rhs.m_anchorPoint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextAnnotation::Reset()
    {
    // Making additions or changes? Please check TextAnnotation::TextAnnotation.
    m_text = NULL;
    m_frame = NULL;
    m_leaders.clear();

    m_origin.Zero();
    m_orientation = YawPitchRollAngles::FromDegrees(0.0, 0.0, 0.0);
    m_anchorPoint = AnchorPoint::LeftTop;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextAnnotation::ApplySeed(DgnElementId value)
    {
    auto seed = TextAnnotationSeed::Get(*m_dgndb, value);
    PRECONDITION(seed.IsValid(), );

    if (m_text.IsValid())
        m_text->SetStyleId(seed->GetTextStyleId(), SetAnnotationTextStyleOptions::Default);

    if (m_frame.IsValid())
        m_frame->SetStyleId(seed->GetFrameStyleId(), SetAnnotationFrameStyleOptions::Default);

    for (auto const& leader : m_leaders)
        leader->SetStyleId(seed->GetLeaderStyleId(), SetAnnotationLeaderStyleOptions::Default);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void TextAnnotation::RemapIds(DgnImportContext& context)
    {
    if (m_text.IsValid())
        {
        DgnElementId oldDocId = m_text->GetStyleId();
        DgnElementId targetDocId = context.RemapAnnotationStyleId(oldDocId);
        m_text->SetStyleId(targetDocId, SetAnnotationTextStyleOptions::Direct);

        for (AnnotationParagraphPtr paragraph : m_text->GetParagraphs())
            {
            DgnElementId oldParaId = paragraph->GetStyleId();
            DgnElementId targetParaId = context.RemapAnnotationStyleId(oldParaId);
            paragraph->SetStyleId(targetParaId, SetAnnotationTextStyleOptions::Direct);
            for (AnnotationRunBasePtr run : paragraph->GetRuns())
                {
                DgnElementId oldRunId = run->GetStyleId();
                DgnElementId targetRunId = context.RemapAnnotationStyleId(oldRunId);
                run->SetStyleId(targetRunId, SetAnnotationTextStyleOptions::Direct);

                if (!run->GetStyleOverrides().HasProperty(AnnotationTextStyleProperty::FontId))
                    continue;

                FontId srcFontId = FontId((uint64_t) run->GetStyleOverrides().GetIntegerProperty(AnnotationTextStyleProperty::FontId));
                FontId dstFontId = context.RemapFont(srcFontId);

                run->GetStyleOverridesR().SetIntegerProperty(AnnotationTextStyleProperty::FontId, (int64_t) dstFontId.GetValue());
                }
            }
        }

    if (m_frame.IsValid())
        {
        DgnElementId oldFrameId = m_frame->GetStyleId();
        DgnElementId targetFrameId = context.RemapAnnotationStyleId(oldFrameId);
        m_frame->SetStyleId(targetFrameId, SetAnnotationFrameStyleOptions::Direct);
        }

    for (auto const& leader : m_leaders)
        {
        DgnElementId oldLeaderId = leader->GetStyleId();
        DgnElementId targetLeaderId = context.RemapAnnotationStyleId(oldLeaderId);
        leader->SetStyleId(targetLeaderId, SetAnnotationLeaderStyleOptions::Direct);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TextAnnotation::Equals(TextAnnotationCR rhs, TextAnnotation::EqualityCheckMode mode) const
    {
    switch (mode)
        {
        // Optimized for the case of checking if an element changed in memory. We don't care about
        // floating point fuzz or DB style ID differences... we just want to know if the text part
        // of an element changed at all and the element has to actually get updated.
        case EqualityCheckMode::ExactInDb:
            {
            bvector<Byte> lhsBits;
            if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(lhsBits, *this))
                {
                BeAssert(false);
                return false;
                }

            bvector<Byte> rhsBits;
            if (SUCCESS != TextAnnotationPersistence::EncodeAsFlatBuf(rhsBits, rhs))
                {
                BeAssert(false);
                return false;
                }

            // Vector compare of Byte is okay.
            return lhsBits == rhsBits;
        }
        }

    BeAssert(false);
    return false;
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextAnnotationCR annotation)
    {
    FlatBufferBuilder encoder;

    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    Offset<FB::AnnotationTextBlock> documentOffset;
    if (annotation.m_text.IsValid())
        if (SUCCESS != AnnotationTextBlockPersistence::EncodeAsFlatBuf(documentOffset, encoder, *annotation.m_text))
            { BeAssert(false); }

    //.............................................................................................
    Offset<FB::AnnotationFrame> frameOffset;
    if (annotation.m_frame.IsValid())
        if (SUCCESS != AnnotationFramePersistence::EncodeAsFlatBuf(frameOffset, encoder, *annotation.m_frame))
            { BeAssert(false); }

    //.............................................................................................
    FB::AnnotationLeaderOffsets leaderOffsets;
    for (auto const& leader : annotation.m_leaders)
        {
        FB::AnnotationLeaderOffset leaderOffset;
        if (SUCCESS != AnnotationLeaderPersistence::EncodeAsFlatBuf(leaderOffset, encoder, *leader))
            { BeAssert(false); }

        leaderOffsets.push_back(leaderOffset);
        }

    FB::AnnotationLeaderOffsetVectorOffset leadersOffset;
    if (!leaderOffsets.empty())
        leadersOffset = encoder.CreateVector(leaderOffsets);

    //.............................................................................................
    FB::TextAnnotationBuilder fbAnnotation(encoder);
    fbAnnotation.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbAnnotation.add_minorVersion(CURRENT_MINOR_VERSION);

    if (annotation.m_text.IsValid())
        fbAnnotation.add_document(documentOffset);

    if (annotation.m_frame.IsValid())
        fbAnnotation.add_frame(frameOffset);

    if (!annotation.m_leaders.empty())
        fbAnnotation.add_leaders(leadersOffset);

    auto annotationOffset = fbAnnotation.Finish();

    //.............................................................................................
    encoder.Finish(annotationOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationPersistence::DecodeFromFlatBuf(TextAnnotationR annotation, ByteCP buffer, size_t numBytes)
    {
    annotation.Reset();

    auto const& fbAnnotation = *GetRoot<FB::TextAnnotation>(buffer);

    PRECONDITION(fbAnnotation.has_majorVersion(), ERROR);
    if (fbAnnotation.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    if (fbAnnotation.has_document())
        {
        if (!annotation.m_text.IsValid())
            annotation.m_text = AnnotationTextBlock::Create(*annotation.m_dgndb);

        if (SUCCESS != AnnotationTextBlockPersistence::DecodeFromFlatBuf(*annotation.m_text, *fbAnnotation.document()))
            { BeAssert(false); }
        }
    else if (annotation.m_text.IsValid())
        {
        annotation.m_text = NULL;
        }

    if (fbAnnotation.has_frame())
        {
        if (!annotation.m_frame.IsValid())
            annotation.m_frame = AnnotationFrame::Create(*annotation.m_dgndb);

        if (SUCCESS != AnnotationFramePersistence::DecodeFromFlatBuf(*annotation.m_frame, *fbAnnotation.frame()))
            { BeAssert(false); }
        }
    else if (annotation.m_frame.IsValid())
        {
        annotation.m_frame = NULL;
        }

    if (!annotation.m_leaders.empty())
        annotation.m_leaders.clear();

    if (fbAnnotation.has_leaders())
        {
        for (auto const& fbLeader : *fbAnnotation.leaders())
            {
            auto leader = AnnotationLeader::Create(*annotation.m_dgndb);
            if (SUCCESS != AnnotationLeaderPersistence::DecodeFromFlatBuf(*leader, fbLeader))
                {
                BeAssert(false);
                continue;
                }

            annotation.m_leaders.push_back(leader);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus TextAnnotationPersistence::DecodeFromFlatBufWithRemap(TextAnnotationR annotation, ByteCP data, size_t numBytes, DgnImportContext& importContext)
    {
    if (SUCCESS != TextAnnotationPersistence::DecodeFromFlatBuf(annotation, data, (size_t) numBytes))
        return ERROR;
    annotation.RemapIds(importContext);

    return SUCCESS;
    }
