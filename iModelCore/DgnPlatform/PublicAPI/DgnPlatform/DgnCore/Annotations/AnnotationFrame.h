/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationFrame.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
//! This enumerates all possible ways to apply an AnnotationFrameStyle to an AnnotationFrame.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class SetAnnotationFrameStyleOptions
{
    PreserveOverrides = 1 << 0,
    
    Default = 0,
    Direct = PreserveOverrides
};

//=======================================================================================
//! An AnnotationFrame is an enclosing piece of geometry around an AnnotationTextBlock, used in a TextAnnotation. A frame also enables the TextAnnotation to have leaders, as the frame defines attachment points based on its type. AnnotationFrame is merely a data object; see AnnotationFrameLayout for size/position/geometry, and AnnotationFrameDraw for drawing.
//! @note An AnnotationFrame must be created with an AnnotationFrameStyle; if a style does not exist, you must first create and store one, and then used its ID to create an AnnotationFrame. While an AnnotationFrame must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
// @bsiclass                                                    Jeff.Marker     06/2014
//=======================================================================================
struct AnnotationFrame : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationFramePersistence;
    
    DgnDbP m_dgndb;
    
    DgnStyleId m_styleID;
    AnnotationFrameStylePropertyBag m_styleOverrides;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFrameCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationFrame(DgnDbR);
    AnnotationFrame(AnnotationFrameCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFrameR operator=(AnnotationFrameCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFramePtr Create(DgnDbR project) { return new AnnotationFrame(project); }
    DGNPLATFORM_EXPORT static AnnotationFramePtr Create(DgnDbR, DgnStyleId);
    AnnotationFramePtr Clone() const { return new AnnotationFrame(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }
    DgnStyleId GetStyleId() const { return m_styleID; }
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationFrameStyleOptions);
    AnnotationFrameStylePtr CreateEffectiveStyle() const { return m_dgndb->Styles().AnnotationFrameStyles().QueryById(m_styleID)->CreateEffectiveStyle(m_styleOverrides); }
    AnnotationFrameStylePropertyBagCR GetStyleOverrides() const { return m_styleOverrides; }
    AnnotationFrameStylePropertyBagR GetStyleOverridesR() { return m_styleOverrides; }
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
