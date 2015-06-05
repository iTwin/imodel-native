//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextBlock.h $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include "AnnotationTextStyle.h"

DGNPLATFORM_TYPEDEFS(AnnotationInsertionPoint);
DGNPLATFORM_REF_COUNTED_PTR(AnnotationInsertionPoint);
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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
enum struct SetAnnotationTextStyleOptions
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
enum struct AnnotationRunType
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
    
    void CopyFrom(AnnotationRunBaseCR);

protected:
    DgnDbP m_dgndb;

    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    DGNPLATFORM_EXPORT explicit AnnotationRunBase(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationRunBase(AnnotationRunBaseCR);
    DGNPLATFORM_EXPORT AnnotationRunBaseR operator=(AnnotationRunBaseCR);

    virtual AnnotationRunBasePtr _Clone() const = 0;
    virtual AnnotationRunType _GetType() const = 0;

public:
    DGNPLATFORM_EXPORT AnnotationRunBasePtr Clone() const;
    
    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT AnnotationRunType GetType() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationTextStyleOptions);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR GetStyleOverridesR();
};

//=======================================================================================
//! Specifies if an AnnotationTextRun is normal, subscript, or superscript.
// The values of the members are expected to match the flatbuffers AnnotationTextRunSubSuperScript.
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
enum struct AnnotationTextRunSubSuperScript
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

    void CopyFrom(AnnotationTextRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationTextRun(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationTextRun(AnnotationTextRunCR);
    DGNPLATFORM_EXPORT AnnotationTextRunR operator=(AnnotationTextRunCR);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnDbR, DgnStyleId, Utf8CP);
    DGNPLATFORM_EXPORT AnnotationTextRunPtr CloneAsTextRun() const;

    DGNPLATFORM_EXPORT Utf8StringCR GetContent() const;
    DGNPLATFORM_EXPORT void SetContent(Utf8CP);
    DGNPLATFORM_EXPORT AnnotationTextRunSubSuperScript GetSubSuperScript() const;
    DGNPLATFORM_EXPORT void SetSubSuperScript(AnnotationTextRunSubSuperScript);
    DGNPLATFORM_EXPORT bool IsSubScript() const;
    DGNPLATFORM_EXPORT void SetIsSubScript(bool);
    DGNPLATFORM_EXPORT bool IsSuperScript() const;
    DGNPLATFORM_EXPORT void SetIsSuperScript(bool);
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

    void CopyFrom(AnnotationFractionRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationFractionRun(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationFractionRun(AnnotationFractionRunCR);
    DGNPLATFORM_EXPORT AnnotationFractionRunR operator=(AnnotationFractionRunCR);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnDbR, DgnStyleId, Utf8CP numerator, Utf8CP denominator);
    DGNPLATFORM_EXPORT AnnotationFractionRunPtr CloneAsFractionRun() const;

    DGNPLATFORM_EXPORT Utf8StringCR GetDenominatorContent() const;
    DGNPLATFORM_EXPORT void SetDenominatorContent(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetNumeratorContent() const;
    DGNPLATFORM_EXPORT void SetNumeratorContent(Utf8CP);
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
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationLineBreakRun(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationLineBreakRun(AnnotationLineBreakRunCR);
    DGNPLATFORM_EXPORT AnnotationLineBreakRunR operator=(AnnotationLineBreakRunCR);
    DGNPLATFORM_EXPORT static AnnotationLineBreakRunPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationLineBreakRunPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationLineBreakRunPtr CloneAsLineBreakRun() const;
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

    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;

    AnnotationRunCollection m_runs;

    void CopyFrom(AnnotationParagraphCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationParagraph(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationParagraph(AnnotationParagraphCR);
    DGNPLATFORM_EXPORT AnnotationParagraphR operator=(AnnotationParagraphCR);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnDbR, DgnStyleId, AnnotationRunBaseR);
    DGNPLATFORM_EXPORT AnnotationParagraphPtr Clone() const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationTextStyleOptions);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR GetStyleOverridesR();
    DGNPLATFORM_EXPORT AnnotationRunCollectionCR GetRuns() const;
    DGNPLATFORM_EXPORT AnnotationRunCollectionR GetRunsR();
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
    enum struct HorizontalJustification
    {
        Left = 1,
        Center = 2,
        Right = 3
    };

private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;

    DgnDbP m_dgndb;
    
    double m_documentWidth;
    HorizontalJustification m_justification;
    
    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    AnnotationParagraphCollection m_paragraphs;

    void CopyFrom(AnnotationTextBlockCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlock(DgnDbR);
    DGNPLATFORM_EXPORT AnnotationTextBlock(AnnotationTextBlockCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockR operator=(AnnotationTextBlockCR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, DgnStyleId, AnnotationParagraphR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnDbR, DgnStyleId, Utf8CP);
    DGNPLATFORM_EXPORT AnnotationTextBlockPtr Clone() const;

    DGNPLATFORM_EXPORT DgnDbR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT double GetDocumentWidth() const;
    DGNPLATFORM_EXPORT void SetDocumentWidth(double);
    DGNPLATFORM_EXPORT HorizontalJustification GetJustification() const;
    DGNPLATFORM_EXPORT void SetJustification(HorizontalJustification);
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationTextStyleOptions);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR GetStyleOverridesR();
    DGNPLATFORM_EXPORT AnnotationParagraphCollectionCR GetParagraphs() const;
    DGNPLATFORM_EXPORT AnnotationParagraphCollectionR GetParagraphsR();
    DGNPLATFORM_EXPORT void AppendParagraph();
    DGNPLATFORM_EXPORT void AppendParagraph(AnnotationParagraphR);
    DGNPLATFORM_EXPORT void AppendRun(AnnotationRunBaseR);
    DGNPLATFORM_EXPORT bool IsEmpty() const;
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
