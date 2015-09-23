//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/Annotations/AnnotationLeader.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeader::AnnotationLeader(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    m_dgndb = &project;
    m_sourceAttachmentType = AnnotationLeaderSourceAttachmentType::Invalid;
    m_targetAttachmentType = AnnotationLeaderTargetAttachmentType::Invalid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
AnnotationLeaderPtr AnnotationLeader::Create(DgnDbR project, DgnStyleId styleID)
    {
    auto leader = AnnotationLeader::Create(project);
    leader->SetStyleId(styleID, SetAnnotationLeaderStyleOptions::Direct);

    return leader;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeader::CopyFrom(AnnotationLeaderCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_dgndb = rhs.m_dgndb;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;
    m_sourceAttachmentType = rhs.m_sourceAttachmentType;
    m_sourceAttachmentDataForId.reset(rhs.m_sourceAttachmentDataForId ? new uint32_t(*rhs.m_sourceAttachmentDataForId) : NULL);
    m_targetAttachmentType = rhs.m_targetAttachmentType;
    m_targetAttachmentDataForPhysicalPoint.reset(rhs.m_targetAttachmentDataForPhysicalPoint ? new DPoint3d(*rhs.m_targetAttachmentDataForPhysicalPoint) : NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2014
//---------------------------------------------------------------------------------------
void AnnotationLeader::Reset()
    {
    // Making additions or changes? Please check constructor and CopyFrom.
    m_styleID.Invalidate();
    m_styleOverrides.ClearAllProperties();
    m_sourceAttachmentType = AnnotationLeaderSourceAttachmentType::Invalid;
    m_sourceAttachmentDataForId.reset(NULL);
    m_targetAttachmentType = AnnotationLeaderTargetAttachmentType::Invalid;
    m_targetAttachmentDataForPhysicalPoint.reset(NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
void AnnotationLeader::SetStyleId(DgnStyleId value, SetAnnotationLeaderStyleOptions options)
    {
    m_styleID = value;

    if (!isEnumFlagSet(SetAnnotationLeaderStyleOptions::PreserveOverrides, options))
        m_styleOverrides.ClearAllProperties();
    }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const uint32_t CURRENT_MAJOR_VERSION = 1;
static const uint32_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::EncodeAsFlatBuf(Offset<FB::AnnotationLeader>& leaderOffset, FlatBufferBuilder& encoder, AnnotationLeaderCR leader)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationLeaderStyleSetters styleOverrides;
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(styleOverrides, leader.m_styleOverrides), ERROR);

    FB::AnnotationLeaderStyleSetterVectorOffset styleOverridesOffset;
    if (!styleOverrides.empty())
        styleOverridesOffset = encoder.CreateVectorOfStructs(styleOverrides);

    //.............................................................................................
    FB::AnnotationLeaderBuilder fbLeader(encoder);
    fbLeader.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbLeader.add_minorVersion(CURRENT_MINOR_VERSION);

    fbLeader.add_styleId(leader.m_styleID.GetValue());
    if (!styleOverrides.empty())
        fbLeader.add_styleOverrides(styleOverridesOffset);

    switch (leader.m_sourceAttachmentType)
        {
        case AnnotationLeaderSourceAttachmentType::Id:
            PRECONDITION(leader.m_sourceAttachmentDataForId, ERROR);

            fbLeader.add_sourceAttachmentType(FB::AnnotationLeaderSourceAttachmentType_ById);
            fbLeader.add_sourceAttachmentId(*leader.m_sourceAttachmentDataForId.get());
            break;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderSourceAttachmentType
            return ERROR;
        }

    switch (leader.m_targetAttachmentType)
        {
        case AnnotationLeaderTargetAttachmentType::PhysicalPoint:
            PRECONDITION(leader.m_targetAttachmentDataForPhysicalPoint, ERROR);
            
            fbLeader.add_targetAttachmentType(FB::AnnotationLeaderTargetAttachmentType_ByPhysicalPoint);
            fbLeader.add_targetAttachmentPt(reinterpret_cast<FB::AnnotationLeaderDPoint3d*>(leader.m_targetAttachmentDataForPhysicalPoint.get()));
            break;
            
        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderTargetAttachmentType
            return ERROR;
        }
    
    leaderOffset = fbLeader.Finish();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, AnnotationLeaderCR leader)
    {
    FlatBufferBuilder encoder;
    
    //.............................................................................................
    Offset<FB::AnnotationLeader> leaderOffset;
    POSTCONDITION(SUCCESS == EncodeAsFlatBuf(leaderOffset, encoder, leader), ERROR);

    //.............................................................................................
    encoder.Finish(leaderOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::DecodeFromFlatBuf(AnnotationLeaderR leader, FB::AnnotationLeader const& fbLeader)
    {
    leader.Reset();

    PRECONDITION(fbLeader.has_majorVersion(), ERROR);
    if (fbLeader.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    PRECONDITION(fbLeader.has_styleId(), ERROR);
    leader.SetStyleId(DgnStyleId((uint64_t)fbLeader.styleId()), SetAnnotationLeaderStyleOptions::Direct);
    
    if (fbLeader.has_styleOverrides())
        AnnotationLeaderStylePersistence::DecodeFromFlatBuf(leader.m_styleOverrides, *fbLeader.styleOverrides());

    PRECONDITION(fbLeader.has_sourceAttachmentType(), ERROR);
    switch (fbLeader.sourceAttachmentType())
        {
        case FB::AnnotationLeaderSourceAttachmentType_ById:
            {
            PRECONDITION(fbLeader.has_sourceAttachmentId(), ERROR);
            uint32_t sourceAttachmentId = fbLeader.sourceAttachmentId();

            leader.SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType::Id);
            leader.SetSourceAttachmentDataForId(&sourceAttachmentId);

            break;
            }

        default:
            BeAssert(false); // Unknown/unexpected PB::AnnotationLeader::SourceAttachmentType
            return ERROR;
        }

    switch (fbLeader.targetAttachmentType())
        {
        case FB::AnnotationLeaderTargetAttachmentType_ByPhysicalPoint:
            {
            PRECONDITION(fbLeader.has_targetAttachmentPt(), ERROR);

            leader.SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType::PhysicalPoint);
            leader.SetTargetAttachmentDataForPhysicalPoint((DPoint3dCP)fbLeader.targetAttachmentPt());

            break;
            }

        default:
            BeAssert(false); // Unknown/unexpected PB::AnnotationLeader::TargetAttachmentType
            return ERROR;
        }

    return SUCCESS;
    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2014
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::DecodeFromFlatBuf(AnnotationLeaderR leader, ByteCP buffer, size_t numBytes)
    {
    auto const& fbLeader = *GetRoot<FB::AnnotationLeader>(buffer);
    return DecodeFromFlatBuf(leader, fbLeader);
    }
