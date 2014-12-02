//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrame.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationFrameStyle.h"

DGNPLATFORM_TYPEDEFS(AnnotationFrame);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFrame);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum struct SetAnnotationFrameStyleOptions
{
    PreserveOverrides = 1 << 0,
    
    Default = 0,
    Direct = PreserveOverrides

}; // SetAnnotationFrameStyleOptions

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrame : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationFramePersistence;
    
    DgnProjectP m_project;
    
    DgnStyleId m_styleID;
    AnnotationFrameStylePropertyBag m_styleOverrides;

    void CopyFrom(AnnotationFrameCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrame(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationFrame(AnnotationFrameCR);
    DGNPLATFORM_EXPORT AnnotationFrameR operator=(AnnotationFrameCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFramePtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationFramePtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationFramePtr Clone() const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationFrameStyleOptions);
    DGNPLATFORM_EXPORT AnnotationFrameStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationFrameStylePropertyBagR GetStyleOverridesR();

}; // AnnotationFrame

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
