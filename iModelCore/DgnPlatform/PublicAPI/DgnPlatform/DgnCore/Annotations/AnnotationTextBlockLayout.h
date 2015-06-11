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

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLayoutRunCR);
    void Invalidate() { m_isValid = false; }
    DGNPLATFORM_EXPORT void Update();

public:
    DGNPLATFORM_EXPORT explicit AnnotationLayoutRun(AnnotationRunBaseCR);
    AnnotationLayoutRun(AnnotationLayoutRunCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLayoutRunR operator=(AnnotationLayoutRunCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLayoutRunPtr Create(AnnotationRunBaseCR seedRun) { return new AnnotationLayoutRun(seedRun); }
    AnnotationLayoutRunPtr Clone() const { return new AnnotationLayoutRun(*this); }

    AnnotationRunBaseCR GetSeedRun() const { return *m_seedRun; }
    size_t GetCharOffset() const { return m_charOffset; }
    void SetCharOffset(size_t value) { m_charOffset = value; Invalidate(); }
    size_t GetNumChars() const { return m_numChars; }
    void SetNumChars(size_t value) { m_numChars = value; Invalidate(); }
    DRange2dCR GetLayoutRange() const { const_cast<AnnotationLayoutRunP>(this)->Update(); return m_layoutRange; }
    DGNPLATFORM_EXPORT BentleyStatus GetSubRange(DRange2dR, SubRange) const;
    DVec2dCR GetOffsetFromLine() const { return m_offsetFromLine; }
    void SetOffsetFromLine(DVec2dCR value) { m_offsetFromLine = value; }
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

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationLayoutLineCR);
    void Invalidate() { m_isValid = false; }
    DGNPLATFORM_EXPORT void Update();

public:
    DGNPLATFORM_EXPORT AnnotationLayoutLine();
    AnnotationLayoutLine(AnnotationLayoutLineCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationLayoutLineR operator=(AnnotationLayoutLineCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationLayoutLinePtr Create() { return new AnnotationLayoutLine(); }
    AnnotationLayoutLinePtr Clone() const { return new AnnotationLayoutLine(*this); }

    DRange2dCR GetLayoutRange() const { const_cast<AnnotationLayoutLineP>(this)->Update(); return m_layoutRange; }
    DRange2dCR GetJustificationRange() const { const_cast<AnnotationLayoutLineP>(this)->Update(); return m_justificationRange; }
    DVec2dCR GetOffsetFromDocument() const { return m_offsetFromDocument; }
    void SetOffsetFromDocument(DVec2dCR value) { m_offsetFromDocument = value; }
    AnnotationLayoutRunCollectionCR GetRuns() const { return m_runs; }
    void AppendRun(AnnotationLayoutRunR value) { Invalidate(); m_runs.push_back(&value); }
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

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationTextBlockLayoutCR);
    AnnotationLayoutLinePtr FlushLine(AnnotationLayoutLineR);
    void PopulateLines();
    void JustifyLines();
    void Invalidate() { m_isValid = false; }
    DGNPLATFORM_EXPORT void Update();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlockLayout(AnnotationTextBlockCR);
    AnnotationTextBlockLayout(AnnotationTextBlockLayoutCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationTextBlockLayoutR operator=(AnnotationTextBlockLayoutCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationTextBlockLayoutPtr Create(AnnotationTextBlockCR doc) { return new AnnotationTextBlockLayout(doc); }
    AnnotationTextBlockLayoutPtr Clone() const { return new AnnotationTextBlockLayout(*this); }

    AnnotationTextBlockCR GetDocument() const { return *m_doc; }
    AnnotationLayoutLineCollectionCR GetLines() const { const_cast<AnnotationTextBlockLayoutP>(this)->Update(); return m_lines; }
    DRange2dCR GetLayoutRange() const { const_cast<AnnotationTextBlockLayoutP>(this)->Update(); return m_layoutRange; }
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
