//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextBlockLayout.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include "AnnotationTextBlock.h"

DGNPLATFORM_TYPEDEFS(AnnotationLayoutLine);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLayoutLine);
DGNPLATFORM_TYPEDEFS(AnnotationLayoutRun);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLayoutRun);
DGNPLATFORM_TYPEDEFS(AnnotationTextBlockLayout);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextBlockLayout);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

typedef bvector<AnnotationLayoutLinePtr> AnnotationLayoutLineCollection;
typedef AnnotationLayoutLineCollection& AnnotationLayoutLineCollectionR;
typedef AnnotationLayoutLineCollection const& AnnotationLayoutLineCollectionCR;

typedef bvector<AnnotationLayoutRunPtr> AnnotationLayoutRunCollection;
typedef AnnotationLayoutRunCollection& AnnotationLayoutRunCollectionR;
typedef AnnotationLayoutRunCollection const& AnnotationLayoutRunCollectionCR;

//=======================================================================================
//! Represents a run or a subset of a run that can exist in a AnnotationLayoutLine. If a single logical run (e.g. AnnotationTextRun) spans multiple lines due to word wrapping, multiple AnnotationLayoutRun objects are created (one per-line) that reference the same "seed" run, but have different offsets and lengths.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLayoutRun : public RefCountedBase
{
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     05/2014
    //=======================================================================================
    enum struct SubRange
    {
        FractionDenominator,
        FractionNumerator,
        JustificationRange
    };

private:
    DEFINE_T_SUPER(RefCountedBase)
    
    AnnotationRunBaseCP m_seedRun;
    size_t m_charOffset;
    size_t m_numChars;

    bool m_isValid;
    DRange2d m_layoutRange;
    bmap<SubRange,DRange2d> m_subRanges;
    DVec2d m_offsetFromLine;

    void CopyFrom(AnnotationLayoutRunCR);
    void Invalidate();
    void Update();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLayoutRun(AnnotationRunBaseCR);
    DGNPLATFORM_EXPORT AnnotationLayoutRun(AnnotationLayoutRunCR);
    DGNPLATFORM_EXPORT AnnotationLayoutRunR operator=(AnnotationLayoutRunCR);
    DGNPLATFORM_EXPORT static AnnotationLayoutRunPtr Create(AnnotationRunBaseCR);
    DGNPLATFORM_EXPORT AnnotationLayoutRunPtr Clone() const;
    
    DGNPLATFORM_EXPORT AnnotationRunBaseCR GetSeedRun() const;
    DGNPLATFORM_EXPORT size_t GetCharOffset() const;
    DGNPLATFORM_EXPORT void SetCharOffset(size_t);
    DGNPLATFORM_EXPORT size_t GetNumChars() const;
    DGNPLATFORM_EXPORT void SetNumChars(size_t);
    DGNPLATFORM_EXPORT DRange2dCR GetLayoutRange() const;
    DGNPLATFORM_EXPORT BentleyStatus GetSubRange(DRange2dR, SubRange) const;
    DGNPLATFORM_EXPORT DVec2dCR GetOffsetFromLine() const;
    DGNPLATFORM_EXPORT void SetOffsetFromLine(DVec2dCR);
    bool CanWrap() const; //!< @private
    bool Wrap(AnnotationLayoutRunPtr& leftOver, double width, bool shouldForceLeadingUnit); //!< @private
    bool AffectsJustification() const; //!< @private
};

//=======================================================================================
//! Represents a sequence or sub-sequence of runs that visually comprise a line of text on screen. When a single run is spliced between multiple lines, unique AnnotationLayoutRun objects will exist in both lines with appropriate offset and size values. There is no concept of a layout "paragraph"; when computing layout information, only lines matter, not paragraphs.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLayoutLine : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    
    AnnotationLayoutRunCollection m_runs;

    bool m_isValid;
    DRange2d m_layoutRange;
    DRange2d m_justificationRange;
    DVec2d m_offsetFromDocument;

    void CopyFrom(AnnotationLayoutLineCR);
    void Invalidate();
    void Update();

public:
    DGNPLATFORM_EXPORT AnnotationLayoutLine();
    DGNPLATFORM_EXPORT AnnotationLayoutLine(AnnotationLayoutLineCR);
    DGNPLATFORM_EXPORT AnnotationLayoutLineR operator=(AnnotationLayoutLineCR);
    DGNPLATFORM_EXPORT static AnnotationLayoutLinePtr Create();
    DGNPLATFORM_EXPORT AnnotationLayoutLinePtr Clone() const;

    DGNPLATFORM_EXPORT DRange2dCR GetLayoutRange() const;
    DGNPLATFORM_EXPORT DRange2dCR GetJustificationRange() const;
    DGNPLATFORM_EXPORT DVec2dCR GetOffsetFromDocument() const;
    DGNPLATFORM_EXPORT void SetOffsetFromDocument(DVec2dCR);
    DGNPLATFORM_EXPORT AnnotationLayoutRunCollectionCR GetRuns() const;

    DGNPLATFORM_EXPORT void AppendRun(AnnotationLayoutRunR);
};

//=======================================================================================
//! Computes size, lines, and layout information for an AnnotationTextBlock. Layout information is computed on-demand and cached, so it is (a) cheap to create instances of this object up-front, and (b) cheap to re-query this object between calls to Invalidate.
//! @note If the underlying AnnotationTextBlock changes after you create this object, it is your responsibility to call Invalidate.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlockLayout : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)

    AnnotationTextBlockCP m_doc;
    bool m_isValid;
    AnnotationLayoutLineCollection m_lines;
    DRange2d m_layoutRange;

    void CopyFrom(AnnotationTextBlockLayoutCR);
    AnnotationLayoutLinePtr FlushLine(AnnotationLayoutLineR);
    void PopulateLines();
    void JustifyLines();
    void Update();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlockLayout(AnnotationTextBlockCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockLayout(AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockLayoutR operator=(AnnotationTextBlockLayoutCR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockLayoutPtr Create(AnnotationTextBlockCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockLayoutPtr Clone() const;

    DGNPLATFORM_EXPORT AnnotationTextBlockCR GetDocument() const;
    DGNPLATFORM_EXPORT AnnotationLayoutLineCollectionCR GetLines() const;
    DGNPLATFORM_EXPORT DRange2dCR GetLayoutRange() const;

    DGNPLATFORM_EXPORT void Invalidate();
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
