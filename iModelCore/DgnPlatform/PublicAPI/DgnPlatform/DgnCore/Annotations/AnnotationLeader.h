//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationLeader.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationLeaderStyle.h"

DGNPLATFORM_TYPEDEFS(AnnotationLeader);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLeader);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct SetAnnotationLeaderStyleOptions
{
    PreserveOverrides = 1 << 0,
    
    Default = 0,
    Direct = PreserveOverrides

}; // SetAnnotationLeaderStyleOptions

//=======================================================================================
// Members should match AnnotationLeader::SourceAttachmentType in Annotations.proto.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderSourceAttachmentType
{
    Invalid = 0,
    Id = 1

}; // AnnotationLeaderSourceAttachmentType

//=======================================================================================
// Members should match AnnotationLeader::TargetAttachmentType in Annotations.proto.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderTargetAttachmentType
{
    Invalid = 0,
    PhysicalPoint = 1

}; // AnnotationLeaderTargetAttachmentType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeader : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationLeaderPersistence;

    DgnProjectP m_project;
    
    DgnStyleId m_styleID;
    AnnotationLeaderStylePropertyBag m_styleOverrides;

    AnnotationLeaderSourceAttachmentType m_sourceAttachmentType;
    std::unique_ptr<UInt32> m_sourceAttachmentDataForId;
    
    AnnotationLeaderTargetAttachmentType m_targetAttachmentType;
    std::unique_ptr<DPoint3d> m_targetAttachmentDataForPhysicalPoint;

    void CopyFrom(AnnotationLeaderCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeader(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationLeader(AnnotationLeaderCR);
    DGNPLATFORM_EXPORT AnnotationLeaderR operator=(AnnotationLeaderCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationLeaderPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationLeaderPtr Clone() const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationLeaderStyleOptions);
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagR GetStyleOverridesR();

    DGNPLATFORM_EXPORT AnnotationLeaderSourceAttachmentType GetSourceAttachmentType() const;
    DGNPLATFORM_EXPORT void SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType);
    DGNPLATFORM_EXPORT UInt32 const* GetSourceAttachmentDataForId() const;
    DGNPLATFORM_EXPORT void SetSourceAttachmentDataForId(UInt32 const*);
    
    DGNPLATFORM_EXPORT AnnotationLeaderTargetAttachmentType GetTargetAttachmentType() const;
    DGNPLATFORM_EXPORT void SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType);
    DGNPLATFORM_EXPORT DPoint3dCP GetTargetAttachmentDataForPhysicalPoint() const;
    DGNPLATFORM_EXPORT void SetTargetAttachmentDataForPhysicalPoint(DPoint3dCP);

}; // AnnotationLeader

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
