//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrame.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFrameStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFramePersistence.h>

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFrame::AnnotationFrame(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    m_dgndb = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFramePtr AnnotationFrame::Create(DgnDbR project, DgnStyleId styleID)
    {
    auto frame = AnnotationFrame::Create(project);
    frame->SetStyleId(styleID, SetAnnotationFrameStyleOptions::Direct);

    return frame;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrame::CopyFrom(AnnotationFrameCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_dgndb = rhs.m_dgndb;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationFrame::Reset()
    {
    // Making additions or changes? Please check constructor and CopyFrom.
    m_styleID.Invalidate();
    m_styleOverrides.ClearAllProperties();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationFrame::SetStyleId(DgnStyleId value, SetAnnotationFrameStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationFrameStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint32_t CURRENT_MAJOR_VERSION = 1;
static const uint32_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFramePersistence::EncodeAsFlatBuf(Offset<FB::AnnotationFrame>& frameOffset, FlatBufferBuilder& encoder, AnnotationFrameCR frame)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationFrameStyleSetters styleOverrides;
    POSTCONDITION(SUCCESS == AnnotationFrameStylePersistence::EncodeAsFlatBuf(styleOverrides, frame.m_styleOverrides), ERROR);

    FB::AnnotationFrameStyleSetterVectorOffset styleOverridesOffset;
    if (!styleOverrides.empty())
        styleOverridesOffset = encoder.CreateVectorOfStructs(styleOverrides);

    //.............................................................................................
    FB::AnnotationFrameBuilder fbFrame(encoder);
    fbFrame.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbFrame.add_minorVersion(CURRENT_MINOR_VERSION);

    fbFrame.add_styleId(frame.m_styleID.GetValue());
    if (!styleOverrides.empty())
        fbFrame.add_styleOverrides(styleOverridesOffset);
    
    //.............................................................................................
    frameOffset = fbFrame.Finish();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFramePersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationFrameCR frame)
    {
    FlatBufferBuilder encoder;
    
    //.............................................................................................
    Offset<FB::AnnotationFrame> frameOffset;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(frameOffset, encoder, frame), ERROR);

    //.............................................................................................
    encoder.Finish(frameOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFramePersistence::DecodeFromFlatBuf(AnnotationFrameR frame, FB::AnnotationFrame const& fbFrame)
    {
    frame.Reset();

    PRECONDITION(fbFrame.has_majorVersion(), ERROR);
    if (fbFrame.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    PRECONDITION(fbFrame.has_styleId(), ERROR);
    frame.SetStyleId(DgnStyleId((uint64_t)fbFrame.styleId()), SetAnnotationFrameStyleOptions::Direct);

    if (fbFrame.has_styleOverrides())
        AnnotationFrameStylePersistence::DecodeFromFlatBuf(frame.m_styleOverrides, *fbFrame.styleOverrides());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationFramePersistence::DecodeFromFlatBuf(AnnotationFrameR frame, ByteCP buffer, size_t numBytes)
    {
    auto const& fbFrame = *GetRoot<FB::AnnotationFrame>(buffer);
    return DecodeFromFlatBuf(frame, fbFrame);
    }
