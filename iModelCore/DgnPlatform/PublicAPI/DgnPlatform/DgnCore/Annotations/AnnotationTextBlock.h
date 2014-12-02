//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/DgnCore/Annotations/AnnotationTextBlock.h $
//  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum struct SetAnnotationTextStyleOptions
{
    PreserveOverrides = 1 << 0,
    DontPropogate = 1 << 1,
    
    Default = 0,
    Direct = PreserveOverrides | DontPropogate

}; // SetAnnotationTextStyleOptions

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum struct AnnotationRunType
{
    Text = 1,
    Fraction = 2,
    LineBreak = 3

}; // AnnotationRunType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationRunBase : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;
    
    void CopyFrom(AnnotationRunBaseCR);

protected:
    DgnProjectP m_project;

    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    DGNPLATFORM_EXPORT explicit AnnotationRunBase(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationRunBase(AnnotationRunBaseCR);
    DGNPLATFORM_EXPORT AnnotationRunBaseR operator=(AnnotationRunBaseCR);

    virtual AnnotationRunBasePtr _Clone() const = 0;
    virtual AnnotationRunType _GetType() const = 0;

public:
//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT AnnotationRunBasePtr Clone() const;
    
    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT AnnotationRunType GetType() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationTextStyleOptions);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR GetStyleOverridesR();
    
}; // AnnotationRunBase

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationTextRun : public AnnotationRunBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
    Utf8String m_content;

    void CopyFrom(AnnotationTextRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationTextRun(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationTextRun(AnnotationTextRunCR);
    DGNPLATFORM_EXPORT AnnotationTextRunR operator=(AnnotationTextRunCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextRunPtr Create(DgnProjectR, DgnStyleId, Utf8CP);
    DGNPLATFORM_EXPORT AnnotationTextRunPtr CloneAsTextRun() const;

    DGNPLATFORM_EXPORT Utf8StringCR GetContent() const;
    DGNPLATFORM_EXPORT void SetContent(Utf8CP);

}; // AnnotationTextRun

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationFractionRun: public AnnotationRunBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
    Utf8String m_denominatorContent;
    Utf8String m_numeratorContent;

    void CopyFrom(AnnotationFractionRunCR);

protected:
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationFractionRun(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationFractionRun(AnnotationFractionRunCR);
    DGNPLATFORM_EXPORT AnnotationFractionRunR operator=(AnnotationFractionRunCR);
    
//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationFractionRunPtr Create(DgnProjectR, DgnStyleId, Utf8CP numerator, Utf8CP denominator);
    DGNPLATFORM_EXPORT AnnotationFractionRunPtr CloneAsFractionRun() const;

    DGNPLATFORM_EXPORT Utf8StringCR GetDenominatorContent() const;
    DGNPLATFORM_EXPORT void SetDenominatorContent(Utf8CP);
    DGNPLATFORM_EXPORT Utf8StringCR GetNumeratorContent() const;
    DGNPLATFORM_EXPORT void SetNumeratorContent(Utf8CP);

}; // AnnotationFractionRun

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationLineBreakRun : public AnnotationRunBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(AnnotationRunBase);
    
protected:
    virtual AnnotationRunBasePtr _Clone() const override;
    virtual AnnotationRunType _GetType() const override;
    
public:
    DGNPLATFORM_EXPORT explicit AnnotationLineBreakRun(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationLineBreakRun(AnnotationLineBreakRunCR);
    DGNPLATFORM_EXPORT AnnotationLineBreakRunR operator=(AnnotationLineBreakRunCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationLineBreakRunPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationLineBreakRunPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT AnnotationLineBreakRunPtr CloneAsLineBreakRun() const;

}; // AnnotationLineBreakRun

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
struct AnnotationParagraph : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;

    DgnProjectP m_project;

    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;

    AnnotationRunCollection m_runs;

    void CopyFrom(AnnotationParagraphCR);

public:
    DGNPLATFORM_EXPORT explicit AnnotationParagraph(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationParagraph(AnnotationParagraphCR);
    DGNPLATFORM_EXPORT AnnotationParagraphR operator=(AnnotationParagraphCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationParagraphPtr Create(DgnProjectR, DgnStyleId, AnnotationRunBaseR);
    DGNPLATFORM_EXPORT AnnotationParagraphPtr Clone() const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
    DGNPLATFORM_EXPORT DgnStyleId GetStyleId() const;
    DGNPLATFORM_EXPORT void SetStyleId(DgnStyleId, SetAnnotationTextStyleOptions);
    DGNPLATFORM_EXPORT AnnotationTextStylePtr CreateEffectiveStyle() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagCR GetStyleOverrides() const;
    DGNPLATFORM_EXPORT AnnotationTextStylePropertyBagR GetStyleOverridesR();
    DGNPLATFORM_EXPORT AnnotationRunCollectionCR GetRuns() const;
    DGNPLATFORM_EXPORT AnnotationRunCollectionR GetRunsR();

}; // AnnotationParagraph

//=======================================================================================
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
    
    }; // HorizontalAlignment

//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct AnnotationTextBlockPersistence;

    DgnProjectP m_project;
    
    double m_documentWidth;
    HorizontalJustification m_justification;
    
    DgnStyleId m_styleID;
    AnnotationTextStylePropertyBag m_styleOverrides;
    
    AnnotationParagraphCollection m_paragraphs;

    void CopyFrom(AnnotationTextBlockCR);
    void Reset();

public:
    DGNPLATFORM_EXPORT explicit AnnotationTextBlock(DgnProjectR);
    DGNPLATFORM_EXPORT AnnotationTextBlock(AnnotationTextBlockCR);
    DGNPLATFORM_EXPORT AnnotationTextBlockR operator=(AnnotationTextBlockCR);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnProjectR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnProjectR, DgnStyleId);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnProjectR, DgnStyleId, AnnotationParagraphR);
    DGNPLATFORM_EXPORT static AnnotationTextBlockPtr Create(DgnProjectR, DgnStyleId, Utf8CP);
    DGNPLATFORM_EXPORT AnnotationTextBlockPtr Clone() const;

    DGNPLATFORM_EXPORT DgnProjectR GetDgnProjectR() const;
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

}; // AnnotationTextBlock

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
