//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/TextAnnotation.cpp $ 
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationSeedPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFramePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/TextAnnotationPersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotation::TextAnnotation(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check TextAnnotation::Reset.

    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
TextAnnotationPtr TextAnnotation::Create(DgnDbR project, DgnStyleId seedID)
    {
    auto annotation = TextAnnotation::Create(project);
    annotation->ApplySeed(seedID);

    return annotation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotation::Reset()
    {
    // Making additions or changes? Please check TextAnnotation::TextAnnotation.
    m_text = NULL;
    m_frame = NULL;
    m_leaders.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
void TextAnnotation::ApplySeed(DgnStyleId value)
    {
    auto seed = m_dgndb->Styles().TextAnnotationSeeds().QueryById(value);
    PRECONDITION(seed.IsValid(), );
    
    if (m_text.IsValid())
        m_text->SetStyleId(seed->GetTextStyleId(), SetAnnotationTextStyleOptions::Default);

    if (m_frame.IsValid())
        m_frame->SetStyleId(seed->GetFrameStyleId(), SetAnnotationFrameStyleOptions::Default);
    
    for (auto const& leader : m_leaders)
        leader->SetStyleId(seed->GetLeaderStyleId(), SetAnnotationLeaderStyleOptions::Default);
    }

//***************************************************************************************************************************************************
//***************************************************************************************************************************************************

static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2014
//---------------------------------------------------------------------------------------
BentleyStatus TextAnnotationPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextAnnotationCR annotation)
    {
    FlatBufferBuilder encoder;

    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    Offset<FB::AnnotationTextBlock> documentOffset;
    if (annotation.m_text.IsValid())
        EXPECTED_CONDITION(SUCCESS == AnnotationTextBlockPersistence::EncodeAsFlatBuf(documentOffset, encoder, *annotation.m_text));

    //.............................................................................................
    Offset<FB::AnnotationFrame> frameOffset;
    if (annotation.m_frame.IsValid())
        EXPECTED_CONDITION(SUCCESS == AnnotationFramePersistence::EncodeAsFlatBuf(frameOffset, encoder, *annotation.m_frame));

    //.............................................................................................
    FB::AnnotationLeaderOffsets leaderOffsets;
    for (auto const& leader : annotation.m_leaders)
        {
        FB::AnnotationLeaderOffset leaderOffset;
        EXPECTED_CONDITION(SUCCESS == AnnotationLeaderPersistence::EncodeAsFlatBuf(leaderOffset, encoder, *leader));
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
// @bsimethod                                                   Jeff.Marker     07/2014
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
        
        EXPECTED_CONDITION(SUCCESS == AnnotationTextBlockPersistence::DecodeFromFlatBuf(*annotation.m_text, *fbAnnotation.document()));
        }
    else if (annotation.m_text.IsValid())
        {
        annotation.m_text = NULL;
        }

    if (fbAnnotation.has_frame())
        {
        if (!annotation.m_frame.IsValid())
            annotation.m_frame = AnnotationFrame::Create(*annotation.m_dgndb);
        
        EXPECTED_CONDITION(SUCCESS == AnnotationFramePersistence::DecodeFromFlatBuf(*annotation.m_frame, *fbAnnotation.frame()));
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
            if (UNEXPECTED_CONDITION(SUCCESS != AnnotationLeaderPersistence::DecodeFromFlatBuf(*leader, fbLeader)))
                continue;

            annotation.m_leaders.push_back(leader);
            }
        }

    return SUCCESS;
    }
