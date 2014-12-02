//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationFrame.cpp $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFrameStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationFramePersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFramePtr AnnotationFrame::Create(DgnProjectR project) { return new AnnotationFrame(project); }
AnnotationFrame::AnnotationFrame(DgnProjectR project) :
    T_Super()
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    m_project = &project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFramePtr AnnotationFrame::Create(DgnProjectR project, DgnStyleId styleID)
    {
    auto frame = AnnotationFrame::Create(project);
    frame->SetStyleId(styleID, SetAnnotationFrameStyleOptions::Direct);

    return frame;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationFramePtr AnnotationFrame::Clone() const { return new AnnotationFrame(*this); }
AnnotationFrame::AnnotationFrame(AnnotationFrameCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
AnnotationFrameR AnnotationFrame::operator=(AnnotationFrameCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
void AnnotationFrame::CopyFrom(AnnotationFrameCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_project = rhs.m_project;
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
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
DgnProjectR AnnotationFrame::GetDgnProjectR() const { return *m_project; }
DgnStyleId AnnotationFrame::GetStyleId() const { return m_styleID; }
AnnotationFrameStylePtr AnnotationFrame::CreateEffectiveStyle() const { return m_project->Styles().AnnotationFrameStyles().QueryById(m_styleID)->CreateEffectiveStyle(m_styleOverrides); }
AnnotationFrameStylePropertyBagCR AnnotationFrame::GetStyleOverrides() const { return m_styleOverrides; }
AnnotationFrameStylePropertyBagR AnnotationFrame::GetStyleOverridesR() { return m_styleOverrides; }

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

static const UInt32 CURRENT_MAJOR_VERSION = 1;
static const UInt32 CURRENT_MINOR_VERSION = 0;

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
    frame.SetStyleId(DgnStyleId(fbFrame.styleId()), SetAnnotationFrameStyleOptions::Direct);

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
