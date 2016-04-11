//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationLeader.h $
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationLeaderStyle.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeader);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeader);

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! This enumerates all possible ways to apply an AnnotationLeaderStyle to an AnnotationLeader.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class SetAnnotationLeaderStyleOptions
{
    PreserveOverrides = 1 << 0,
    
    Default = 0,
    Direct = PreserveOverrides
};

//=======================================================================================
//! This enumerates all possible annotation leader source attachment types (e.g. how a leader connects to the frame).
// Members should match AnnotationLeader::SourceAttachmentType in Annotations.proto.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationLeaderSourceAttachmentType
{
    Invalid = 0,
    Id = 1 //<! Uses the ID of an attachment point defined by the frame. To get attachment point IDs, see AnnotationFrameLayout.
};

//=======================================================================================
//! This enumerates all possible annotation leader target attachment types (e.g. how a leader connects to its target).
// Members should match AnnotationLeader::TargetAttachmentType in Annotations.proto.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum class AnnotationLeaderTargetAttachmentType
{
    Invalid = 0,
    PhysicalPoint = 1 //!< Uses a physical 3D point to connect to; no association is kept to what it is pointing at.
};

//=======================================================================================
//! An AnnotationLeader is a line and optional terminator from an AnnotationFrame to another object in the project for a TextAnnotation. AnnotationLeader is merely a data object; see AnnotationLeaderLayout for size and geometry, and AnnotationLeaderDraw for drawing.
//! @note An AnnotationLeader must be created with an AnnotationLeaderStyle; if a style does not exist, you must first create and store one, and then used its ID to create an AnnotationLeader. While an AnnotationLeader must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
//! @note Since different data is needed for each attachment type, use the appropriate Get/Set...AttachmentDataFor... methods based on the type of attachment you have set.
//! @note It is invalid to set an attachment type without also setting its data.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeader : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationLeaderPersistence;

    DgnDbP m_dgndb;
    
    AnnotationLeaderStyleId m_styleID;
    AnnotationLeaderStylePropertyBag m_styleOverrides;

    AnnotationLeaderSourceAttachmentType m_sourceAttachmentType;
    uint32_t m_sourceAttachmentDataForId;
    
    AnnotationLeaderTargetAttachmentType m_targetAttachmentType;
    DPoint3d m_targetAttachmentDataForPhysicalPoint;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLeaderCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeader(DgnDbR);
    AnnotationLeader(AnnotationLeaderCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLeaderR operator=(AnnotationLeaderCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLeaderPtr Create(DgnDbR project) { return new AnnotationLeader(project); }
    DGNPLATFORM_EXPORT static AnnotationLeaderPtr Create(DgnDbR, AnnotationLeaderStyleId);
    AnnotationLeaderPtr Clone() const { return new AnnotationLeader(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }
    AnnotationLeaderStyleId GetStyleId() const { return m_styleID; }
    DGNPLATFORM_EXPORT void SetStyleId(AnnotationLeaderStyleId, SetAnnotationLeaderStyleOptions);
    AnnotationLeaderStylePtr CreateEffectiveStyle() const { return AnnotationLeaderStyle::Get(*m_dgndb, m_styleID)->CreateEffectiveStyle(m_styleOverrides); }
    AnnotationLeaderStylePropertyBagCR GetStyleOverrides() const { return m_styleOverrides; }
    AnnotationLeaderStylePropertyBagR GetStyleOverridesR() { return m_styleOverrides; }
    AnnotationLeaderSourceAttachmentType GetSourceAttachmentType() const { return m_sourceAttachmentType; }
    void SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType value) { m_sourceAttachmentType = value; }
    uint32_t GetSourceAttachmentDataForId() const { return m_sourceAttachmentDataForId; }
    void SetSourceAttachmentDataForId(uint32_t value) { m_sourceAttachmentDataForId = value; }
    AnnotationLeaderTargetAttachmentType GetTargetAttachmentType() const { return m_targetAttachmentType; }
    void SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType value) { m_targetAttachmentType = value; }
    DPoint3dCR GetTargetAttachmentDataForPhysicalPoint() const { return m_targetAttachmentDataForPhysicalPoint; }
    void SetTargetAttachmentDataForPhysicalPoint(DPoint3dCR value) { m_targetAttachmentDataForPhysicalPoint = value; }
};

END_BENTLEY_DGN_NAMESPACE
