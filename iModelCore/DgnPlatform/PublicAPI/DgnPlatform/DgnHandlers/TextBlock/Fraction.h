/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Fraction.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"
#include "Run.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! In the TextBlock DOM, a base class for stacked fractions representing a like-formatted numerator and denominator (supports the presence of both or either).
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendFraction to effectively create and append fractions to a TextBlock.
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct Fraction : public Run
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Run)
    protected:  mutable CharStreamP m_numeratorRun;
    protected:  mutable CharStreamP m_denominatorRun;
    protected:          DPoint2d    m_textScale;
    protected:  mutable bool        m_isDirty;

    public:                                                         Fraction                            (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);
    public:                                                         Fraction                            (RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:     DGNPLATFORM_EXPORT                                  Fraction                            (FractionCR);
    public:     DGNPLATFORM_EXPORT  virtual                         ~Fraction                           ();

    protected:                                  void                Init                                (WCharCP numeratorText, WCharCP denominatorText, StackedFractionAlignment, DPoint2dCP textScaleFactor);
    protected:                                  bool                FitsInLine                          (LineCR, ProcessContextCR);
    protected:                                  void                PreprocessCharStream                (CharStreamR) const;

    protected:                      virtual     void                _SetupNumeratorLocation             () const = 0;
    protected:                      virtual     void                _SetupDenominatorLocation           () const = 0;
    protected:                      virtual     void                _SetupNumeratorProperties           () const = 0;
    protected:                      virtual     void                _SetupDenominatorProperties         () const = 0;
    protected:                      virtual     StackedFractionType _GetFractionType                    () const = 0;
    protected:                      virtual     WChar               _GetSeparatorChar                   () const = 0;

    protected:                      virtual     void                _GenerateNumeratorElementParams     (TextParamWideR, DPoint2dR textScale, DgnProjectR) const;
    protected:                      virtual     void                _GenerateDenominatorElementParams   (TextParamWideR, DPoint2dR textScale, DgnProjectR) const;
    protected:                      virtual     double              _GetMaxDisplacementAboveOrigin      () const override;
    protected:                      virtual     double              _GetMaxDisplacementBelowOrigin      () const override;
    protected:                      virtual     double              _GetMaxExactHeightAboveOrigin       () const override;
    protected:                      virtual     double              _GetMaxExactDepthBelowOrigin        () const override;
    protected:                      virtual     WString             _ToString                           (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;
    protected:                      virtual     size_t              _GetCharacterCount                  () const override;
    protected:                      virtual     WChar               _GetCharacter                       (size_t) const override;
    protected:                      virtual     void                _ComputeOffsetToChar                (DVec3dR, double& scale, CaretCR) const override;
    protected:                      virtual     void                _Splice                             (RunP& firstRun, RunP& secondRun, size_t index) override;
    protected:                      virtual     size_t              _GetNextWordBreakIndex              (size_t) const override;
    protected:                      virtual     AppendStatus        _AppendNextRunToLine                (LineR, RunP& nextSplitRun, ProcessContextCR) override;
    protected:                      virtual     AppendStatus        _AppendToLine                       (LineR, RunP& nextRun, ProcessContextCR) override;
    protected:                      virtual     bool                _CanFit                             (LineR, ProcessContextCR) override;
    protected:                      virtual     void                _ComputeRange                       () override;
    protected:                      virtual     void                _Preprocess                         () const override;
    protected:                      virtual     void                _Draw                               (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const override;
    protected:                      virtual     void                _SetProperties                      (RunPropertiesCR) override;
    protected:                      virtual     bool                _ContainsOnlyWhitespace             () const override;
    protected:                      virtual     bool                _Equals                             (RunCR, TextBlockCompareOptionsCR) const override;
    protected:                      virtual     bool                _IsAtomic                           () const override;
    protected:                      virtual     void                _GetElementRange                    (DRange3dR) const;

    public:                                     CharStreamP         GetCharStream                       (size_t) const;
    public:     DGNPLATFORM_EXPORT              DPoint2dCR          GetTextScale                        () const;
    public:     DGNPLATFORM_EXPORT              void                SetTextScale                        (DPoint2dCR);
    public:                                     void                SetNumeratorText                    (WCharCP);
    public:                                     void                SetDenominatorText                  (WCharCP);
    public:                                     void                GenerateElementParameters           (TextParamWideR, DPoint2dR textScale, size_t index, DgnProjectR) const;
    public:     DGNPLATFORM_EXPORT              WChar               GetSeparatorChar                    () const;
    public:     DGNPLATFORM_EXPORT              bool                IsUnderlined                        () const;
    public:                         static      DPoint2d            GetDefaultFractionScaleFactor       ();
    public:     DGNPLATFORM_EXPORT  static      StackedFractionType ComputeTypeFromSeparatorChar        (WChar);
    public:     DGNPLATFORM_EXPORT  static      WChar               ComputeSeparatorCharFromType        (StackedFractionType);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Gets the type of this fraction.
    //! @note   This directly corresponds to the sub-class type, so a dynamic_cast would also suffice.
    public: DGNPLATFORM_EXPORT StackedFractionType GetFractionType () const;

    //! Gets the alignment of the fraction with other runs in the line.
    //! @note   Since stacked fractions are typically much taller then surrounding runs, this property is important to vertical alignment within the line.
    public: DGNPLATFORM_EXPORT StackedFractionAlignment GetAlignment () const;

    //! Sets the alignment of the fraction with other runs in the line.
    //! @see GetAlignment for additional notes
    public: DGNPLATFORM_EXPORT void SetAlignment (StackedFractionAlignment);

    //! Gets the numerator character stream.
    //! @note   While the numerator and denominator are represented as character streams, their formatting is kept in sync, as fractions do not currently support mixed formatting.
    public: DGNPLATFORM_EXPORT CharStreamCP GetNumerator () const;

    //! Gets the denominator character stream.
    //! @note   While the numerator and denominator are represented as character streams, their formatting is kept in sync, as fractions do not currently support mixed formatting.
    public: DGNPLATFORM_EXPORT CharStreamCP GetDenominator () const;

    }; // Fraction

//=======================================================================================
//! In the TextBlock DOM, a class for normal (non-diagonal) stacked fractions, where the numerator and denominator are stacked and horizontally left-aligned (with NO separator). This class directly cooresponds to StackedFractionType::NoBar.
//! @note You likely want a Fraction object instead; you can then ask Fraction::GetFractionType if you need to know a specific type (instead of additional casting).
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendFraction to effectively create and append fractions to a TextBlock.
// @bsiclass                                                    Venkat.Kalyan   06/2007
//=======================================================================================
struct NoBarFraction : public Fraction
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Fraction)

    public:                                                         NoBarFraction               (RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:                                                         NoBarFraction               (WCharCP numText, WCharCP denText, TextParamWideCR, DPoint2dCR fontSize, StackedFractionAlignment, DPoint2dCP textScaleFactor, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  NoBarFraction               (WCharCP numText, WCharCP denText, RunPropertiesCR, DgnGlyphRunLayoutFlags, StackedFractionAlignment, DPoint2dCP textScaleFactor);

    protected:                      virtual     void                _Preprocess                 () const override;
    protected:                      virtual     RunP                _Clone                      () const override;

    protected:                      virtual     void                _SetupNumeratorLocation     () const override;
    protected:                      virtual     void                _SetupDenominatorLocation   () const override;
    protected:                      virtual     void                _SetupNumeratorProperties   () const override;
    protected:                      virtual     void                _SetupDenominatorProperties () const override;
    protected:                      virtual     StackedFractionType _GetFractionType            () const override;
    protected:                      virtual     WChar               _GetSeparatorChar           () const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // NoBarFraction

//=======================================================================================
//! In the TextBlock DOM, a class for normal (non-diagonal) stacked fractions, where the numerator and denominator are individually horizontally centered (with separator). This class directly cooresponds to StackedFractionType::HorizontalBar.
//! @note You likely want a Fraction object instead; you can then ask Fraction::GetFractionType if you need to know a specific type (instead of additional casting).
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendFraction to effectively create and append fractions to a TextBlock.
// @bsiclass                                                    Venkat.Kalyan   06/2007
//=======================================================================================
struct HorizontalBarFraction : public Fraction
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Fraction)

    public:                                                         HorizontalBarFraction       (RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:                                                         HorizontalBarFraction       (WCharCP numText, WCharCP denText, TextParamWideCR, DPoint2dCR fontSize, StackedFractionAlignment, DPoint2dCP textScaleFactor, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  HorizontalBarFraction       (WCharCP numText, WCharCP denText, RunPropertiesCR, DgnGlyphRunLayoutFlags, StackedFractionAlignment, DPoint2dCP textScaleFactor);

    protected:                      virtual     void                _Preprocess                 () const override;
    protected:                      virtual     RunP                _Clone                      () const override;

    protected:                      virtual     void                _SetupNumeratorLocation     () const override;
    protected:                      virtual     void                _SetupDenominatorLocation   () const override;
    protected:                      virtual     void                _SetupNumeratorProperties   () const override;
    protected:                      virtual     void                _SetupDenominatorProperties () const override;
    protected:                      virtual     StackedFractionType _GetFractionType            () const override;
    protected:                      virtual     WChar               _GetSeparatorChar           () const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // HorizontalBarFraction

//=======================================================================================
//! In the TextBlock DOM, a class for diagonal stacked fractions, where the numerator and denominator are separated by a diagnoal line (hard-coded slant angle). This class directly cooresponds to StackedFractionType::DiagonalBar.
//! @note You likely want a Fraction object instead; you can then ask Fraction::GetFractionType if you need to know a specific type (instead of additional casting).
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendFraction to effectively create and append fractions to a TextBlock.
// @bsiclass                                                    Venkat.Kalyan   06/2007
//=======================================================================================
struct DiagonalBarFraction : public Fraction
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Fraction)

    public:                                                         DiagonalBarFraction                 (RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:                                                         DiagonalBarFraction                 (WCharCP numText, WCharCP denText, TextParamWideCR, DPoint2dCR fontSize, StackedFractionAlignment, DPoint2dCP textScaleFactor, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  DiagonalBarFraction                 (WCharCP numText, WCharCP denText, RunPropertiesCR, DgnGlyphRunLayoutFlags, StackedFractionAlignment, DPoint2dCP textScaleFactor);

    protected:                      virtual     double              _GetNominalHeight                   () const override;

    protected:                      virtual     void                _Preprocess                         () const override;
    protected:                      virtual     RunP                _Clone                              () const override;

    protected:                      virtual     void                _SetupNumeratorLocation             () const override;
    protected:                      virtual     void                _SetupDenominatorLocation           () const override;
    protected:                      virtual     void                _SetupNumeratorProperties           () const override;
    protected:                      virtual     void                _SetupDenominatorProperties         () const override;
    protected:                      virtual     StackedFractionType _GetFractionType                    () const override;
    protected:                      virtual     WChar               _GetSeparatorChar                   () const override;
    protected:                      virtual     void                _GenerateNumeratorElementParams     (TextParamWideR, DPoint2dR textScale, DgnProjectR) const override;
    protected:                      virtual     void                _GenerateDenominatorElementParams   (TextParamWideR, DPoint2dR textScale, DgnProjectR) const override;

    public:                                     double              GetNumeratorOffsetFactor            () const;
    public:                                     double              GetDenominatorOffsetFactor          () const;
    public:                                     double              ComputeAdditionalRunYOffset         () const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // DiagonalBarFraction

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
