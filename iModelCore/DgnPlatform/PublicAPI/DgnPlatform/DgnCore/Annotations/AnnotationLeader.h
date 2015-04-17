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
//! This enumerates all possible ways to apply an AnnotationLeaderStyle to an AnnotationLeader.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct SetAnnotationLeaderStyleOptions
{
    PreserveOverrides = 1 << 0,
    
    Default = 0,
    Direct = PreserveOverrides

}; // SetAnnotationLeaderStyleOptions

//=======================================================================================
//! This enumerates all possible annotation leader source attachment types (e.g. how a leader connects to the frame).
// Members should match AnnotationLeader::SourceAttachmentType in Annotations.proto.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderSourceAttachmentType
{
    Invalid = 0,
    Id = 1 //<! Uses the ID of an attachment point defined by the frame. To get attachment point IDs, see AnnotationFrameLayout.

}; // AnnotationLeaderSourceAttachmentType

//=======================================================================================
//! This enumerates all possible annotation leader target attachment types (e.g. how a leader connects to its target).
// Members should match AnnotationLeader::TargetAttachmentType in Annotations.proto.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
enum struct AnnotationLeaderTargetAttachmentType
{
    Invalid = 0,
    PhysicalPoint = 1 //!< Uses a physical 3D point to connect to; no association is kept to what it is pointing at.

}; // AnnotationLeaderTargetAttachmentType

//=======================================================================================
//! An AnnotationLeader is a line and optional terminator from an AnnotationFrame to another object in the project for a TextAnnotation. AnnotationLeader is merely a data object; see AnnotationLeaderLayout for size and geometry, and AnnotationLeaderDraw for drawing.
//! @note An AnnotationLeader must be created with an AnnotationLeaderStyle; if a style does not exist, you must first create and store one, and then used its ID to create an AnnotationLeader. While an AnnotationLeader must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
//! @note Since different data is needed for each attachment type, use the appropriate Get/Set...AttachmentDataFor... methods based on the type of attachment you have set.
//! @note It is invalid to set an attachment type without also setting its data.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationLeader : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationLeaderPersistence;

    DgnDbP m_dgndb;
    
    DgnStyleId m_styleID;
    AnnotationLeaderStylePropertyBag m_styleOverrides;

    AnnotationLeaderSourceAttachmentType m_sourceAttachmentType;
    std::unique_ptr<uint32_t> m_sourceAttachmentDataForId;
    
    AnnotationLeaderTargetAttachmentType m_targetAttachmentType;
    std::unique_ptr<DPoint3d> m_targetAttachmentDataForPhysicalPoint;

    void CopyFrom(AnnotationLeaderCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLeader(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationLeader(AnnotationLeaderCR);
    DGNPLATFORM_EXPORT AnnotationLeaderR operator=(AnnotationLeaderCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLeaderPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationLeaderPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationLeaderPtr Clone() const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationLeaderStyleOptions);
    DGNPLATFORM_EXPORT AnnotationLeaderStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationLeaderStylePropertyBagR GetStyleOverridesR();

    DGNPLATFORM_EXPORT AnnotationLeaderSourceAttachmentType GetSourceAttachmentType() const;
    DGNPLATFORM_EXPORT void SetSourceAttachmentType(AnnotationLeaderSourceAttachmentType);
    DGNPLATFORM_EXPORT uint32_t const* GetSourceAttachmentDataForId() const;
    DGNPLATFORM_EXPORT void SetSourceAttachmentDataForId(uint32_t const*);
    
    DGNPLATFORM_EXPORT AnnotationLeaderTargetAttachmentType GetTargetAttachmentType() const;
    DGNPLATFORM_EXPORT void SetTargetAttachmentType(AnnotationLeaderTargetAttachmentType);
    DGNPLATFORM_EXPORT DPoint3dCP GetTargetAttachmentDataForPhysicalPoint() const;
    DGNPLATFORM_EXPORT void SetTargetAttachmentDataForPhysicalPoint(DPoint3dCP);

}; // AnnotationLeader

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
