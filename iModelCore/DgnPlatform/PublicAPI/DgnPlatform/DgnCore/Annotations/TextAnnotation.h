//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/TextAnnotation.h $ 
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $ 
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
struct TextAnnotation : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)

    DgnDbP m_dgndb;

    AnnotationTextBlockPtr m_text;
    AnnotationFramePtr m_frame;
    AnnotationLeaderCollection m_leaders;

    void CopyFrom(TextAnnotationCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit TextAnnotation(DgnDbR);
    DGNPLATFORM_EXPORT TextAnnotation(TextAnnotationCR);
    DGNPLATFORM_EXPORT TextAnnotationR operator=(TextAnnotationCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT TextAnnotationPtr Clone() const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT void ApplySeed(DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationTextBlockCP GetTextCP() const;
    DGNPLATFORM_EXPORT AnnotationTextBlockP GetTextP();
    DGNPLATFORM_EXPORT void SetText(AnnotationTextBlockCP);
    DGNPLATFORM_EXPORT AnnotationFrameCP GetFrameCP() const;
    DGNPLATFORM_EXPORT AnnotationFrameP GetFrameP();
    DGNPLATFORM_EXPORT void SetFrame(AnnotationFrameCP);
    DGNPLATFORM_EXPORT AnnotationLeaderCollectionCR GetLeaders() const;
    DGNPLATFORM_EXPORT AnnotationLeaderCollectionR GetLeadersR();

}; // TextAnnotation

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
