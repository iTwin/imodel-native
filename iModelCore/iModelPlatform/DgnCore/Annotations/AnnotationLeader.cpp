//---------------------------------------------------------------------------------------------
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//  See LICENSE.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
 
#include <DgnPlatformInternal.h> 
#include <DgnPlatform/Annotations/Annotations.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderStylePersistence.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationLeaderPersistence.h>

USING_NAMESPACE_BENTLEY_DGN
using namespace flatbuffers;

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationLeader::AnnotationLeader(DgnDbR project) :
    T_Super()
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    m_dgndb = &project;
    m_sourceAttachmentType = AnnotationLeaderSourceAttachmentType::Invalid;
    m_targetAttachmentType = AnnotationLeaderTargetAttachmentType::Invalid;
    m_targetAttachmentDataForPhysicalPoint = DPoint3d::FromZero();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
AnnotationLeaderPtr AnnotationLeader::Create(DgnDbR project, AnnotationLeaderStyleId styleID)
    {
    auto leader = AnnotationLeader::Create(project);
    leader->SetStyleId(styleID, SetAnnotationLeaderStyleOptions::Direct);

    return leader;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationLeader::CopyFrom(AnnotationLeaderCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_dgndb = rhs.m_dgndb;
    m_styleID = rhs.m_styleID;
    m_styleOverrides = rhs.m_styleOverrides;
    m_sourceAttachmentType = rhs.m_sourceAttachmentType;
    m_sourceAttachmentDataForId = rhs.m_sourceAttachmentDataForId;
    m_targetAttachmentType = rhs.m_targetAttachmentType;
    m_targetAttachmentDataForPhysicalPoint = rhs.m_targetAttachmentDataForPhysicalPoint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationLeader::Reset()
    {
    // Making additions or changes? Please check constructor and CopyFrom.
    m_styleID.Invalidate();
    m_styleOverrides.ClearAllProperties();
    m_sourceAttachmentType = AnnotationLeaderSourceAttachmentType::Invalid;
    m_sourceAttachmentDataForId = 0;
    m_targetAttachmentType = AnnotationLeaderTargetAttachmentType::Invalid;
    m_targetAttachmentDataForPhysicalPoint.Zero();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void AnnotationLeader::SetStyleId(AnnotationLeaderStyleId value, SetAnnotationLeaderStyleOptions options)
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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::EncodeAsFlatBuf(Offset<FB::AnnotationLeader>& leaderOffset, FlatBufferBuilder& encoder, AnnotationLeaderCR leader)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    FB::AnnotationLeaderStyleSetters styleOverrides;
    POSTCONDITION(SUCCESS == AnnotationLeaderStylePersistence::EncodeAsFlatBuf(styleOverrides, leader.m_styleOverrides, AnnotationLeaderStylePersistence::FlatBufEncodeOptions::SettersAreOverrides), ERROR);

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
            fbLeader.add_sourceAttachmentType(FB::AnnotationLeaderSourceAttachmentType_ById);
            fbLeader.add_sourceAttachmentId(leader.m_sourceAttachmentDataForId);
            break;

        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderSourceAttachmentType
            return ERROR;
        }

    switch (leader.m_targetAttachmentType)
        {
        case AnnotationLeaderTargetAttachmentType::PhysicalPoint:
            fbLeader.add_targetAttachmentType(FB::AnnotationLeaderTargetAttachmentType_ByPhysicalPoint);
            fbLeader.add_targetAttachmentPt(reinterpret_cast<FB::AnnotationLeaderDPoint3d const*>(&leader.m_targetAttachmentDataForPhysicalPoint));
            break;
            
        default:
            BeAssert(false); // Unknown/unexpected AnnotationLeaderTargetAttachmentType
            return ERROR;
        }
    
    leaderOffset = fbLeader.Finish();
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::DecodeFromFlatBuf(AnnotationLeaderR leader, FB::AnnotationLeader const& fbLeader)
    {
    leader.Reset();

    PRECONDITION(fbLeader.has_majorVersion(), ERROR);
    if (fbLeader.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;

    PRECONDITION(fbLeader.has_styleId(), ERROR);
    leader.SetStyleId(AnnotationLeaderStyleId((uint64_t)fbLeader.styleId()), SetAnnotationLeaderStyleOptions::Direct);
    
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
            leader.SetSourceAttachmentDataForId(sourceAttachmentId);

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
            leader.SetTargetAttachmentDataForPhysicalPoint(*(DPoint3dCP)fbLeader.targetAttachmentPt());

            break;
            }

        default:
            BeAssert(false); // Unknown/unexpected PB::AnnotationLeader::TargetAttachmentType
            return ERROR;
        }

    return SUCCESS;
    
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationLeaderPersistence::DecodeFromFlatBuf(AnnotationLeaderR leader, ByteCP buffer, size_t numBytes)
    {
    auto const& fbLeader = *GetRoot<FB::AnnotationLeader>(buffer);
    return DecodeFromFlatBuf(leader, fbLeader);
    }
