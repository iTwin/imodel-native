//-------------------------------------------------------------------------------------- 
//  Copyright (c) Bentley Systems, Incorporated. All rights reserved. 
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

BEGIN_BENTLEY_DGN_NAMESPACE

typedef bvector<AnnotationLeaderPtr> AnnotationLeaderCollection;
typedef AnnotationLeaderCollection& AnnotationLeaderCollectionR;
typedef AnnotationLeaderCollection const& AnnotationLeaderCollectionCR;

//=======================================================================================
//! TextAnnotation objects are used to place markup text (vs. physical text) in a drawing. They are comprised of a piece of text (AnnotationTextBlock), an optional frame (AnnotationFrame), and 0 or more leaders (AnnotationLeader). TextAnnotation is merely a data object; see TextAnnotationDraw for drawing, and TextAnnotationHandler for persistence.
//! TextAnnotation objects themselves do not have "styles"; they can instead be created from "seeds" (TextAnnotationSeed). Unlike a style, a seed is only used when creating the TextAnnotation. TextAnnotation objects do not automatically react to changes in their seed (unlike how a style system normally operates), but it is a convenient way to create multiple TextAnnotation objects with the same look and feel. The individual pieces of a TextAnnotation (the text, frame, and leaders) all have styles, which do behave like a classical style system.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     07/2014
//=======================================================================================
struct TextAnnotation : RefCountedBase
{
    //=======================================================================================
    //! @ingroup GROUP_Annotation
    // @bsiclass                                                    Josh.Schifter   04/2016
    //=======================================================================================
    enum class AnchorPoint
    {
        LeftTop = 1,
        LeftMiddle = 2,
        LeftBottom = 3,
        CenterTop = 4,
        CenterMiddle = 5,
        CenterBottom = 6,
        RightTop = 7,
        RightMiddle = 8,
        RightBottom = 9
    };

private:
    DEFINE_T_SUPER(RefCountedBase)

    DgnDbP m_dgndb;

    AnnotationTextBlockPtr m_text;
    AnnotationFramePtr m_frame;
    AnnotationLeaderCollection m_leaders;
    DPoint3d m_origin;
    YawPitchRollAngles m_orientation;
    AnchorPoint m_anchorPoint;

    DGNPLATFORM_EXPORT void CopyFrom(TextAnnotationCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit TextAnnotation(DgnDbR);
    TextAnnotation(TextAnnotationCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextAnnotationR operator=(TextAnnotationCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static TextAnnotationPtr Create(DgnDbR project) { return new TextAnnotation(project); }
    DGNPLATFORM_EXPORT static TextAnnotationPtr Create(DgnDbR, DgnElementId);
    TextAnnotationPtr Clone() const { return new TextAnnotation(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }                      
    void SetOrigin(DPoint3dCR value) { m_origin = value; }
    DPoint3dCR GetOrigin() const { return m_origin; }
    void SetOrientation(YawPitchRollAnglesCR value) { m_orientation = value; }
    YawPitchRollAngles GetOrientation() const { return m_orientation; }
    void SetAnchorPoint(AnchorPoint value) { m_anchorPoint = value; }
    AnchorPoint GetAnchorPoint() const { return m_anchorPoint; }
    AnnotationTextBlockCP GetTextCP() const { return m_text.get(); }
    AnnotationTextBlockP GetTextP() { return m_text.get(); }
    void SetText(AnnotationTextBlockCP value) { m_text = const_cast<AnnotationTextBlockP>(value); }
    AnnotationFrameCP GetFrameCP() const { return m_frame.get(); }
    AnnotationFrameP GetFrameP() { return m_frame.get(); }
    void SetFrame(AnnotationFrameCP value) { m_frame = const_cast<AnnotationFrameP>(value); }
    AnnotationLeaderCollectionCR GetLeaders() const { return m_leaders; }
    AnnotationLeaderCollectionR GetLeadersR() { return m_leaders; }
    DGNPLATFORM_EXPORT void ApplySeed(DgnElementId);
    DGNPLATFORM_EXPORT void RemapIds(DgnImportContext&);
};

END_BENTLEY_DGN_NAMESPACE
