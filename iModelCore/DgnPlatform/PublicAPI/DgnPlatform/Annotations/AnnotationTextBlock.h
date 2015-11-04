//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationTextBlock.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include "AnnotationTextStyle.h"

DGNPLATFORM_TYPEDEFS(AnnotationRunBase);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationRunBase);
DGNPLATFORM_TYPEDEFS(AnnotationTextRun);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextRun);
DGNPLATFORM_TYPEDEFS(AnnotationFractionRun);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationFractionRun);
DGNPLATFORM_TYPEDEFS(AnnotationLineBreakRun);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationLineBreakRun);
DGNPLATFORM_TYPEDEFS(AnnotationParagraph);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationParagraph);
DGNPLATFORM_TYPEDEFS(AnnotationTextBlock);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationTextBlock);

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup
   
typedef bvector<AnnotationParagraphPtr> AnnotationParagraphCollection;
typedef AnnotationParagraphCollection& AnnotationParagraphCollectionR;
typedef AnnotationParagraphCollection const& AnnotationParagraphCollectionCR;

typedef bvector<AnnotationRunBasePtr> AnnotationRunCollection;
typedef AnnotationRunCollection& AnnotationRunCollectionR;
typedef AnnotationRunCollection const& AnnotationRunCollectionCR;

//=======================================================================================
//! This enumerates all possible ways to apply an AnnotationTextStyle to an AnnotationTextBlock objects.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class SetAnnotationTextStyleOptions
{
    PreserveOverrides = 1 << 0,
    DontPropogate = 1 << 1,
    
    Default = 0,
    Direct = PreserveOverrides | DontPropogate
};

//=======================================================================================
//! This enumerates all possible annotation run types.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class AnnotationRunType
{
    Text = 1,
    Fraction = 2,
    LineBreak = 3
};

//=======================================================================================
//! A "run" is typically a sequence of characters that share a single format/style, but other specialized run types exist. In the hierarchy of block/paragraph/run, run is the most granular piece of the block, and contains the actual character data. When laid out on screen, a single run may span multiple lines, but it can never span different formats/styles.
//! @note A run must be created with an AnnotationTextStyle; if a style does not exist, you must first create and store one, and then used its ID to create a run. Not all properties of an AnnotationTextStyle apply to runs; see AnnotationTextStyleProperty to determine which properties pertain to runs. While a run must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationRunBase : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;
    
    DGNPLATFORM_EXPORT void CopyFrom(AnnotationRunBaseCR);

protected:
    DgnDbP m_dgndb;

    AnnotationTextStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    DGNPLATFORM_EXPORT explicit AnnotationRunBase(DgnDbR);
    AnnotationRunBase(AnnotationRunBaseCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationRunBaseR operator=(AnnotationRunBaseCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}

    virtual AnnotationRunBasePtr _Clone() const = 0;
    virtual AnnotationRunType _GetType() const = 0;

public:
    AnnotationRunBasePtr Clone() const { return _Clone(); }
    
    DgnDbR GetDbR() const { return *m_dgndb; }
    AnnotationRunType GetType() const { return _GetType(); }
    AnnotationTextStyleId GetStyleId() const { return m_styleID; }
    DGNPLATFORM_EXPORT void SetStyleId(AnnotationTextStyleId, SetAnnotationTextStyleOptions);
    AnnotationTextStylePtr CreateEffectiveStyle() const { return AnnotationTextStyle::QueryStyle(m_styleID, *m_dgndb)->CreateEffectiveStyle(m_styleOverrides); }
    AnnotationTextStylePropertyBagCR GetStyleOverrides() const { return m_styleOverrides; }
    AnnotationTextStylePropertyBagR GetStyleOverridesR() { return m_styleOverrides; }
};

//=======================================================================================
//! Specifies if an AnnotationTextRun is normal, subscript, or superscript.
// The values of the members are expected to match the flatbuffers AnnotationTextRunSubSuperScript.
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
enum class AnnotationTextRunSubSuperScript
    {
    Neither = 0,
    SubScript = 1,
    SuperScript = 2
};

//=======================================================================================
//! A text run is the most common specialization of AnnotationRunBase, and contains a sequence of characters.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextRun : public AnnotationRunBase
{
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
    Utf8String m_content;
    AnnotationTextRunSubSuperScript m_subsuperscript;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationTextRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override { return CloneAsTextRun(); }
    virtual AnnotationRunType _GetType() const override { return AnnotationRunType::Text; }
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationTextRun(DgnDbR);
    AnnotationTextRun(AnnotationTextRunCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationTextRunR operator=(AnnotationTextRunCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationTextRunPtr Create(DgnDbR project) { return new AnnotationTextRun(project); }
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnDbR, AnnotationTextStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnDbR, AnnotationTextStyleId, Utf8CP);
    AnnotationTextRunPtr CloneAsTextRun() const { return new AnnotationTextRun(*this); }

    Utf8StringCR GetContent() const { return m_content; }
    void SetContent(Utf8CP value) { m_content = value; }
    AnnotationTextRunSubSuperScript GetSubSuperScript() const { return m_subsuperscript; }
    void SetSubSuperScript(AnnotationTextRunSubSuperScript value) { m_subsuperscript = value; }
    bool IsSubScript() const { return AnnotationTextRunSubSuperScript::SubScript == m_subsuperscript; }
    void SetIsSubScript(bool value) { m_subsuperscript = AnnotationTextRunSubSuperScript::SubScript; }
    bool IsSuperScript() const { return AnnotationTextRunSubSuperScript::SuperScript == m_subsuperscript; }
    void SetIsSuperScript(bool value) { m_subsuperscript = AnnotationTextRunSubSuperScript::SuperScript; }
};

//=======================================================================================
//! A fraction run is a specialization of AnnotationRunBase that represents a stacked fraction.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationFractionRun: public AnnotationRunBase
{
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
    Utf8String m_denominatorContent;
    Utf8String m_numeratorContent;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationFractionRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override { return CloneAsFractionRun(); }
    virtual AnnotationRunType _GetType() const override { return AnnotationRunType::Fraction; }
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationFractionRun(DgnDbR);
    AnnotationFractionRun(AnnotationFractionRunCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationFractionRunR operator=(AnnotationFractionRunCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationFractionRunPtr Create(DgnDbR project) { return new AnnotationFractionRun(project); }
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnDbR, AnnotationTextStyleId);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnDbR, AnnotationTextStyleId, Utf8CP numerator, Utf8CP denominator);
    AnnotationFractionRunPtr CloneAsFractionRun() const { return new AnnotationFractionRun(*this); }

    Utf8StringCR GetDenominatorContent() const { return m_denominatorContent; }
    void SetDenominatorContent(Utf8CP value) { m_denominatorContent = value; }
    Utf8StringCR GetNumeratorContent() const { return m_numeratorContent; }
    void SetNumeratorContent(Utf8CP value) { m_numeratorContent = value; }
};

//=======================================================================================
//! A line break run is a specialization of AnnotationRunBase that manually breaks a line within a paragraph.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLineBreakRun : public AnnotationRunBase
{
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
protected:
    virtual AnnotationRunBasePtr _Clone() const override { return CloneAsLineBreakRun(); }
    virtual AnnotationRunType _GetType() const override { return AnnotationRunType::LineBreak; }
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationLineBreakRun(DgnDbR);
    AnnotationLineBreakRun(AnnotationLineBreakRunCR rhs) : T_Super(rhs) { }
    AnnotationLineBreakRunR operator=(AnnotationLineBreakRunCR rhs) { T_Super::operator=(rhs); return *this;}
    static AnnotationLineBreakRunPtr Create(DgnDbR project) { return new AnnotationLineBreakRun(project); }
    DGNPLATFORM_EXPORT static AnnotationLineBreakRunPtr Create(DgnDbR, AnnotationTextStyleId);
    AnnotationLineBreakRunPtr CloneAsLineBreakRun() const { return new AnnotationLineBreakRun(*this); }
};

//=======================================================================================
//! A paragraph is a collection of runs. In a block, individual paragraphs are started on their own "line", similar to how an AnnotationLineBreakRun operates within a paragraph.
//! @note A paragraph must be created with an AnnotationTextStyle; if a style does not exist, you must first create and store one, and then used its ID to create a run. Not all properties of an AnnotationTextStyle apply to paragraphs; see AnnotationTextStyleProperty to determine which properties pertain to paragraphs. While a paragraph must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationParagraph : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;

    DgnDbP m_dgndb;

    AnnotationTextStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;

    AnnotationRunCollection m_runs;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationParagraphCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationParagraph(DgnDbR);
    AnnotationParagraph(AnnotationParagraphCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationParagraphR operator=(AnnotationParagraphCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationParagraphPtr Create(DgnDbR project) { return new AnnotationParagraph(project); }
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnDbR, AnnotationTextStyleId);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnDbR, AnnotationTextStyleId, AnnotationRunBaseR);
    AnnotationParagraphPtr Clone() const { return new AnnotationParagraph(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }
    AnnotationTextStyleId GetStyleId() const { return m_styleID; }
    DGNPLATFORM_EXPORT void SetStyleId(AnnotationTextStyleId, SetAnnotationTextStyleOptions);
    AnnotationTextStylePtr CreateEffectiveStyle() const { return AnnotationTextStyle::QueryStyle(m_styleID, *m_dgndb)->CreateEffectiveStyle(m_styleOverrides); }
    AnnotationTextStylePropertyBagCR GetStyleOverrides() const { return m_styleOverrides; }
    AnnotationTextStylePropertyBagR GetStyleOverridesR() { return m_styleOverrides; }
    AnnotationRunCollectionCR GetRuns() const { return m_runs; }
    AnnotationRunCollectionR GetRunsR() { return m_runs; }
    void AppendRun(AnnotationRunBaseR run) { m_runs.push_back(&run); }
};

//=======================================================================================
//! A block is a collection of paragraphs, and contains some unique formatting properties that cannot be on paragraphs or runs, such as justification. AnnotationTextBlock is merely a data object; see AnnotationTextBlockLayout for size/position/lines, and AnnotationTextBlockDraw for drawing. By default, no word-wrapping occurs. You can define a physical word wrap distance by calling SetDocumentWidth; runs will then automatically be split according to line break rules when performing layout.
//! @note A block must be created with an AnnotationTextStyle; if a style does not exist, you must first create and store one, and then used its ID to create a block. Not all properties of an AnnotationTextStyle apply to blocks; see AnnotationTextStyleProperty to determine which properties pertain to blocks. While a block must have a style, it can override each individual style property as needed. Properties are not typically overridden in order to enforce project standards, however it is technically possible.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextBlock : public RefCountedBase
{
    //=======================================================================================
    // INTERNAL WARNING: The integer values are used for persistence; do not change them.
    // @bsiclass                                                    Jeff.Marker     05/2014
    //=======================================================================================
    enum class HorizontalJustification
    {
        Left = 1,
        Center = 2,
        Right = 3
    };

    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     10/2015
    //=======================================================================================
    struct ToStringOpts
    {
    private:
        Utf8String m_paragraphSeparator;
        Utf8String m_lineBreakString;
        Utf8String m_fractionSeparator;

    public:
        DGNPLATFORM_EXPORT static ToStringOpts CreateForPlainText();

        Utf8StringCR GetParagraphSeparator() const { return m_paragraphSeparator; }
        void SetParagraphSeparator(Utf8CP value) { m_paragraphSeparator.AssignOrClear(value); }
        Utf8StringCR GetLineBreakString() const { return m_lineBreakString; }
        void SetLineBreakString(Utf8CP value) { m_lineBreakString.AssignOrClear(value); }
        Utf8StringCR GetFractionSeparator() const { return m_fractionSeparator; }
        void SetFractionSeparator(Utf8CP value) { m_fractionSeparator.AssignOrClear(value); }
    };

private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;

    DgnDbP m_dgndb;
    
    double m_documentWidth;
    HorizontalJustification m_justification;
    
    AnnotationTextStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    AnnotationParagraphCollection m_paragraphs;

    DGNPLATFORM_EXPORT void CopyFrom(AnnotationTextBlockCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlock(DgnDbR);
    AnnotationTextBlock(AnnotationTextBlockCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    AnnotationTextBlockR operator=(AnnotationTextBlockCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this;}
    static AnnotationTextBlockPtr Create(DgnDbR project) { return new AnnotationTextBlock(project); }
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, AnnotationTextStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, AnnotationTextStyleId, AnnotationParagraphR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, AnnotationTextStyleId, Utf8CP);
    AnnotationTextBlockPtr Clone() const { return new AnnotationTextBlock(*this); }

    DgnDbR GetDbR() const { return *m_dgndb; }
    double GetDocumentWidth() const { return m_documentWidth; }
    void SetDocumentWidth(double value) { m_documentWidth = value; }
    HorizontalJustification GetJustification() const { return m_justification; }
    void SetJustification(HorizontalJustification value) { m_justification = value; }
    AnnotationTextStyleId GetStyleId() const { return m_styleID; }
    DGNPLATFORM_EXPORT void SetStyleId(AnnotationTextStyleId, SetAnnotationTextStyleOptions);
    AnnotationTextStylePtr CreateEffectiveStyle() const { return AnnotationTextStyle::QueryStyle(m_styleID, *m_dgndb)->CreateEffectiveStyle(m_styleOverrides); }
    AnnotationTextStylePropertyBagCR GetStyleOverrides() const { return m_styleOverrides; }
    AnnotationTextStylePropertyBagR GetStyleOverridesR() { return m_styleOverrides; }
    AnnotationParagraphCollectionCR GetParagraphs() const { return m_paragraphs; }
    AnnotationParagraphCollectionR GetParagraphsR() { return m_paragraphs; }
    void AppendParagraph(AnnotationParagraphR par) { m_paragraphs.push_back(&par); }
    DGNPLATFORM_EXPORT void AppendParagraph();
    DGNPLATFORM_EXPORT void AppendRun(AnnotationRunBaseR);
    bool IsEmpty() const { return ((0 == m_paragraphs.size()) || (0 == m_paragraphs[0]->GetRuns().size())); }
    Utf8String ToString() const { return ToString(ToStringOpts::CreateForPlainText()); }
    DGNPLATFORM_EXPORT Utf8String ToString(ToStringOpts const&) const;
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
