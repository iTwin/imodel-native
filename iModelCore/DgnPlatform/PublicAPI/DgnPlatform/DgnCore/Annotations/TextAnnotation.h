//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotation.h $ 
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "AnnotationFrame.h"
#include "AnnotationLeader.h"
#include "AnnotationTextBlock.h"
#include "TextAnnotationSeed.h"

DGNPLATFORM_TYPEDEFS(TextAnnotation);
DGNPLATFORM_REF_COUNTED_PTR(TextAnnotation);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

typedef bvector<AnnotationLeaderPtr> AnnotationLeaderCollection;
typedef AnnotationLeaderCollection& AnnotationLeaderCollectionR;
typedef AnnotationLeaderCollection const& AnnotationLeaderCollectionCR;

//=======================================================================================
//! TextAnnotation objects are used to place markup text (vs. physical text) in a drawing. They are comprised of a piece of text (AnnotationTextBlock), an optional frame (AnnotationFrame), and 0 or more leaders (AnnotationLeader). TextAnnotation is merely a data object; see TextAnnotationDraw for drawing, and TextAnnotationHandler for persistence.
//! TextAnnotation objects themselves do not have "styles"; they can instead be created from "seeds" (TextAnnotationSeed). Unlike a style, a seed is only used when creating the TextAnnotation. TextAnnotation objects do not automatically react to changes in their seed (unlike how a style system normally operates), but it is a convenient way to create multiple TextAnnotation objects with the same look and feel. The individual pieces of a TextAnnotation (the text, frame, and leaders) all have styles, which do behave like a classical style system.
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotation : RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    DgnDbP m_dgndb;

    AnnotationTextBlockPtr m_text;
    AnnotationFramePtr m_frame;
    AnnotationLeaderCollection m_leaders;

    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit TextAnnotation(DgnDbR);
    TextAnnotation(TextAnnotationCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextAnnotationR operator=(TextAnnotationCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static TextAnnotationPtr Create(DgnDbR project) { return new TextAnnotation(project); }
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnDbR, TextAnnotationSeedId);
    TextAnnotationPtr Clone() const { return new TextAnnotation(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }                      
    AnnotationTextBlockCP GetTextCP() const { return m_text.get(); }
    AnnotationTextBlockP GetTextP() { return m_text.get(); }
    void SetText(AnnotationTextBlockCP value) { m_text = const_cast<AnnotationTextBlockP>(value); }
    AnnotationFrameCP GetFrameCP() const { return m_frame.get(); }
    AnnotationFrameP GetFrameP() { return m_frame.get(); }
    void SetFrame(AnnotationFrameCP value) { m_frame = const_cast<AnnotationFrameP>(value); }
    AnnotationLeaderCollectionCR GetLeaders() const { return m_leaders; }
    AnnotationLeaderCollectionR GetLeadersR() { return m_leaders; }
    DGNPLATFORM_EXPORT void ApplySeed(TextAnnotationSeedId);
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
