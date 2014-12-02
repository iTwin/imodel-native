/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/CharStream.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"
#include "Run.h"
#include <DgnPlatform/DgnCore/DgnFontManager.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/2010
//=======================================================================================
struct TextFieldData : public RefCountedBase, public NonCopyableClass
    {
    //=======================================================================================
    // @bsiclass                                                    John.Gooding    11/2010
    //=======================================================================================
    private: enum PropertyFieldContentsFlags
        {
        NoneFlag                    = 0,
        EvaluatorIdFlag             = (1 << 0),
        ExpressionFlag              = (1 << 1),
        ErrorMessageFlag            = (1 << 2),
        HandlerPersistenceKeyFlag   = (1 << 3),
        HandlerPersistentDataFlag   = (1 << 4),
        CachedDisplayFlag           = (1 << 5),
        CachedValueFlag             = (1 << 6),

        }; // PropertyFieldContentsFlags

    //=======================================================================================
    // @bsiclass                                                    John.Gooding    11/2010
    //=======================================================================================
    private: enum PropertyFieldEvaluationReason
        {
        None            = 0,
        Editor          = (1 << 0),
        Update          = (1 << 1),
        Plot            = (1 << 2),
        ModelLoad       = (1 << 3),
        ModelSave       = (1 << 4),
        DesignHistory   = (1 << 5),
        Unconditional   = -1

        }; // PropertyFieldEvaluationReason

    //=======================================================================================
    // @bsiclass                                                    John.Gooding    11/2010
    //=======================================================================================
    private: enum TypeCode
        {
        NoValue         = 0,
        StringValue     = 1,
        Int32Value      = 2,
        Int64Value      = 3,
        Double          = 4,
        DateTimeValue   = 5

        // It looks like this has never been used:
        // BooleanValue = 6

        }; // TypeCode

    private:    WString             m_evaluatorId;
    private:    WString             m_expression;       // Requires class, access, and enabler; may also contain additional information.
    private:    WString             m_formatterName;
    private:    bvector<byte>       m_formatterBytes;
    private:    ElementId           m_sourceElementId;
    private:    ElementId           m_dependentElementId;

    public:             WCharCP             GetEvaluatorId  () const;
    public:             WCharCP             GetExpression   () const;
    public:             bvector<byte> const& GetFormatterBytes() const;
    public:             WCharCP             GetFormatterName () const;
    public:             ElementId           GetSourceElementId () const;
    public:             ElementId           GetDependentElementId() const;

    private:                                TextFieldData   (WCharCP evaluatorId, WCharCP expression, WCharCP formatterName, bvector<byte> const& formatterBytes, ElementId srcElementId, ElementId depElementId);
    public:     static  TextFieldDataPtr    Create          (WCharCP evaluatorId, WCharCP expression, WCharCP formatterName, bvector<byte> const& formatterBytes, ElementId srcElementId, ElementId depElementId);
    public:     static  TextFieldDataPtr    Create          (ElementHandleCR);

    public:             void                AppendToElement (EditElementHandleR) const;
    public:             TextFieldDataPtr    Clone           () const;

    }; // TextFieldData

//__PUBLISH_SECTION_START__

//=======================================================================================
//! In the TextBlock DOM, a collection of single-line, single-format characters.
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendText to effectively create and append a CharStream to a TextBlock.
// @bsiclass                                                    Venkat.Kalyan   05/2007
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CharStream : public Run
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Run)

    protected:          WString             m_string;
    protected:  mutable bool                m_isDirty;

    private:    mutable double              m_maxDisplacementBelowOrigin;
    private:    mutable double              m_maxHorizontalCellIncrement;
    private:    mutable double              m_trailingInterCharacterSpacing;
    private:    mutable T_DoubleVector      m_leadingCharacterOffsets;
    private:    mutable T_DoubleVector      m_trailingCharacterOffsets;
    private:    mutable double              m_computedTrailingCharacterOffset;
    private:            bool                m_containsOnlyWhitespace;
    private:            TextFieldDataPtr    m_fieldData;
    private:    mutable DRange3d            m_elementRange;

    private:    explicit                    CharStream                              (DgnModelR);
    public:                                 CharStream                              (WCharCP, RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:                                 CharStream                              (WCharCP, TextParamWideCR, DPoint2dCR textSize, DgnModelR);
    public:                                 CharStream                              (CharStreamCR);

    private:                void            Init                                    (WCharCP);
    private:                void            UpdateContainsOnlyWhitespace            ();
    private:                void            GetNextWordChars                        (WStringR, size_t offset) const;
    private:                void            LayoutGlyphs                        (size_t offset, size_t length, DRange3dR nominalRange, DRange3dR exactRange, DRange3dP elementRange) const;
    private:                void            LayoutGlyphs                        (size_t offset, size_t length, DRange3dR nominalRange, DRange3dR exactRange, DRange3dP elementRange, T_DoubleVectorP leadingCharacterOffsets, T_DoubleVectorP trailingCharacterOffsets, double* maxHorizontalCellIncrement, double* trailingInterCharacterSpacing) const;
    private:                size_t          FindIndexOfFittableArrayOfChars         (ProcessContextCR, LineR);

    protected:  virtual     double          _GetMaxDisplacementBelowOrigin          () const override;
    protected:  virtual     double          _GetMaxHorizontalCellIncrement          () const override;
    protected:  virtual     WString         _ToString                               (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;
    protected:  virtual     size_t          _GetCharacterCount                      () const override;
    protected:  virtual     WChar           _GetCharacter                           (size_t) const override;
    protected:  virtual     void            _Splice                                 (RunP& firstRun, RunP& secondRun, size_t index) override;
    protected:  virtual     size_t          _GetNextWordBreakIndex                  (size_t) const override;
    protected:  virtual     AppendStatus    _AppendNextRunToLine                    (LineR, RunP& nextSplitRun, ProcessContextCR) override;
    protected:  virtual     AppendStatus    _AppendToLine                           (LineR, RunP& nextRun, ProcessContextCR) override;
    protected:  virtual     bool            _CanFit                                 (LineR, ProcessContextCR) override;
    protected:  virtual     void            _ComputeRange                           () override;
    protected:  virtual     void            _Preprocess                             () const override;
    protected:  virtual     RunP            _Clone                                  () const override;
    protected:  virtual     void            _GetRangesForSelection                  (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;
    protected:  virtual     void            _GetTransformedRangesForSelection       (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;
    protected:  virtual     bool            _IsLastCharSpace                        () const override;
    protected:  virtual     double          _ComputeLeftEdgeAlignDistance           () const override;
    protected:  virtual     void            _Draw                                   (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR options) const override;
    protected:  virtual     bool            _AllowTrailingWordBreak                 () const override;
    protected:  virtual     void            _SetProperties                          (RunPropertiesCR) override;
    protected:  virtual     void            _SetIsLastRunInLine                     (bool) override;
    protected:  virtual     bool            _ContainsOnlyWhitespace                 () const override;
    protected:  virtual     bool            _Equals                                 (RunCR, TextBlockCompareOptionsCR) const override;
    protected:  virtual     BentleyStatus   _ComputeCaretAtLocation                 (CaretR, DPoint3dCR, bool isStrict) const override;
    protected:  virtual     void            _ComputeOffsetToChar                    (DVec3dR, double& scale, CaretCR) const override;
    protected:  virtual     bool            _IsSpace                                (size_t) const;
    protected:  virtual     void            _SetString                              (WStringCR);
    protected:  virtual     void            _GetEdfMaskForLayout                    (size_t offset, size_t length, DgnGlyphLayoutContext::T_EdfMask&) const;
    protected:  virtual     void            _ComputeRangeOfCharsWithoutSpaces       (DRange3dR nominalRange, DRange3dR exactRange, size_t offset, size_t length, bool trimLeft, bool trimRight) const;
    protected:  virtual     void            _Merge                                  (CharStreamP);
    protected:  virtual     void            _SetRunLayoutFlags                      (DgnGlyphRunLayoutFlags) override;
    protected:  virtual     void            _ComputeTransformedHitTestRange         (DRange3dR) const override;
    protected:  virtual     void            _SetOverrideTrailingCharacterOffset     (double) override;
    protected:  virtual     void            _ClearTrailingCharacterOffsetOverride   () override;
    protected:  virtual     void            _GetElementRange                        (DRange3dR) const;

    protected:              bool            FitsInLine                              (LineCR, ProcessContextCR, size_t offset, size_t length, bool isFirstFitAttempt);
    private:                void            ComputeWhiteSpaceSymbolOrigin           (DPoint3dR, double& effectiveFontSize, size_t iChar) const;
    protected:              void            DrawWhiteSpace                          (ViewContextR) const;

    public:                 bool            IsSpace                                 (size_t) const;
    public:                 BentleyStatus   GetFraction                             (UInt8& numerator, UInt8& denominator, size_t i) const;
    public:                 void            SetString                               (WStringCR);
    public:                 void            Draw                                    (ViewContextR, bool isViewIndependent, StackedFractionType, StackedFractionSection, TextBlockDrawOptionsCR) const;
    public: DGNPLATFORM_EXPORT TextFieldDataP  GetFieldData                            () const;
    public:                 void            SetFieldData                            (TextFieldDataP);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Returns this instance's internal string. It is returned as Unicode, regardless of the fonts' encoding.
    //! @note   This is more efficient than Run::ToString, becuse it returns a reference to this instance's internal string, as opposed to creating a new one.
    public: DGNPLATFORM_EXPORT WStringCR GetString () const;

    }; // CharStream

//=======================================================================================
//! Enter Data Fields (EDFs) are atomic placeholder runs of pre-defined length (number of characters).
//! EDFs also have a justification. Since they are a fixed length, their display is padded with space characters to ensure a consistent length. The justification controls how space characters are inserted for padding. It is invalid to splice an EDF; it must be replaced or removed in entirety.
// @bsiclass                                                    Jeff.Marker     07/2010
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EdfCharStream : public CharStream
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(CharStream)
    private: EdfJustification m_edfJustification;

    public:     DGNPLATFORM_EXPORT                          EdfCharStream                       (WCharCP, size_t totalLength, EdfJustification, RunPropertiesCR, DgnGlyphRunLayoutFlags);

    protected:                      virtual void            _Splice                             (RunP& firstRun, RunP& secondRun, size_t index) override;
    protected:                      virtual size_t          _GetNextWordBreakIndex              (size_t) const override;
    protected:                      virtual AppendStatus    _AppendNextRunToLine                (LineR, RunP& nextSplitRun, ProcessContextCR) override;
    protected:                      virtual AppendStatus    _AppendToLine                       (LineR, RunP& nextRun, ProcessContextCR) override;
    protected:                      virtual bool            _CanFit                             (LineR, ProcessContextCR) override;
    protected:                      virtual RunP            _Clone                              () const override;
    protected:                      virtual void            _Draw                               (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const override;
    protected:                      virtual bool            _AllowTrailingWordBreak             () const override;
    protected:                      virtual void            _Merge                              (CharStreamP) override;
    protected:                      virtual void            _SetString                          (WStringCR) override;
    protected:                      virtual bool            _Equals                             (RunCR, TextBlockCompareOptionsCR) const override;
    protected:                      virtual void            _GetEdfMaskForLayout                (size_t offset, size_t length, DgnGlyphLayoutContext::T_EdfMask&) const override;
    protected:                      virtual bool            _IsAtomic                           () const override;
    protected:                      virtual void            _ComputeRangeOfCharsWithoutSpaces   (DRange3dR nominalRange, DRange3dR exactRange, size_t offset, size_t length, bool trimLeft, bool trimRight) const override;
    protected:                      virtual WString         _ToString                           (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Returns the justification of this EDF. This justification controls how space characters are distributed to reach the total length.
    public: DGNPLATFORM_EXPORT EdfJustification GetEdfJustification () const;

    }; // EdfCharStream

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
