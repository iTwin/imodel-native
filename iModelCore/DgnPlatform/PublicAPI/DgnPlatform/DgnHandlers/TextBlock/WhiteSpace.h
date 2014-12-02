/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/WhiteSpace.h $
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
//! In the TextBlock DOM, an abstract base class representing all whitespace types, and is not to be used directly.
// @bsiclass                                                    Venkat.Kalyan   08/04
//=======================================================================================
struct WhiteSpace : public Run
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(Run)
    private:    mutable double  m_maxDisplacementBelowOrigin;

    protected:                      WhiteSpace                      (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);
    protected:                      WhiteSpace                      (RunPropertiesCR, DgnGlyphRunLayoutFlags);

    protected:  virtual double      _GetMaxDisplacementBelowOrigin  () const override;
    protected:  virtual size_t      _GetCharacterCount              () const override;
    protected:  virtual void        _ComputeOffsetToChar            (DVec3dR, double& scale, CaretCR) const override;
    protected:  virtual void        _Splice                         (RunP& firstRun, RunP& secondRun, size_t index) override;
    protected:  virtual size_t      _GetNextWordBreakIndex          (size_t) const override;
    protected:  virtual bool        _CanFit                         (LineR, ProcessContextCR) override;
    protected:  virtual void        _ComputeRange                   () override;
    protected:  virtual void        _Preprocess                     () const override;
    protected:  virtual bool        _Equals                         (RunCR, TextBlockCompareOptionsCR) const override;
    protected:  virtual void        _GetElementRange                (DRange3dR) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // WhiteSpace

//=======================================================================================
//! In the TextBlock DOM, a carriage return, or paragraph break. This kind of run is used to denote a hard carriage return, forcing the creation of a new paragraph (and hence a new line).
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct ParagraphBreak : public WhiteSpace
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(WhiteSpace)

    public:                                                 ParagraphBreak                      (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);
    public:     DGNPLATFORM_EXPORT                          ParagraphBreak                      (RunPropertiesCR, DgnGlyphRunLayoutFlags);

    protected:                      virtual DRange3d        _GetNominalRange                    () const override;
    protected:                      virtual DRange3d        _GetExactRange                      () const override;

    protected:                      virtual double          _GetMaxDisplacementAboveOrigin      () const override;
    protected:                      virtual WString         _ToString                           (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;
    protected:                      virtual WChar           _GetCharacter                       (size_t) const override;
    protected:                      virtual AppendStatus    _AppendNextRunToLine                (LineR, RunP& nextSplitRun, ProcessContextCR) override;
    protected:                      virtual AppendStatus    _AppendToLine                       (LineR, RunP& nextRun, ProcessContextCR) override;
    protected:                      virtual RunP            _Clone                              () const override;
    protected:                      virtual bool            _IsContentRun                       () const override;
    protected:                      virtual void            _Draw                               (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const override;
    protected:                      virtual void            _GetRangesForSelection              (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;
    protected:                      virtual void            _GetTransformedRangesForSelection   (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // ParagraphBreak

//=======================================================================================
//! In the TextBlock DOM, a line feed, or line break. This kind of run is used to denote a hard line feed, forcing the creation of a new line in the same paragraph. This is different than transparent soft line feeds, which denote new lines based on word-wrapping.
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct LineBreak : public WhiteSpace
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(WhiteSpace)

    public:                                                 LineBreak                           (TextParamWideCR textParams, DPoint2dCR fontSize, DgnModelR dgnCache);
    public:     DGNPLATFORM_EXPORT                          LineBreak                           (RunPropertiesCR, DgnGlyphRunLayoutFlags);

    protected:                      virtual DRange3d        _GetNominalRange                    () const override;
    protected:                      virtual DRange3d        _GetExactRange                      () const override;

    protected:                      virtual double          _GetMaxDisplacementAboveOrigin      () const override;
    protected:                      virtual WString         _ToString                           (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;
    protected:                      virtual WChar           _GetCharacter                       (size_t index)  const override;
    protected:                      virtual AppendStatus    _AppendNextRunToLine                (LineR line, RunP& nextSplitRun, ProcessContextCR processContext) override;
    protected:                      virtual AppendStatus    _AppendToLine                       (LineR line, RunP& nextRun, ProcessContextCR processContext) override;
    protected:                      virtual RunP            _Clone                              () const override;
    protected:                      virtual bool            _IsContentRun                       () const override;
    protected:                      virtual void            _Draw                               (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const override;
    protected:                      virtual void            _GetRangesForSelection              (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;
    protected:                      virtual void            _GetTransformedRangesForSelection   (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // LineBreak

//=======================================================================================
//! In the TextBlock DOM, a tab. This kind of run is used to advance the following run to the next tab stop, as stored/computed by the current paragraph.
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct Tab : public WhiteSpace
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(WhiteSpace)

    private:    DPoint2d    m_tabExtents;

    public:                                                 Tab                     (TextParamWideCR, DPoint2dCR fontSize, DgnModelR);
    public:     DGNPLATFORM_EXPORT                          Tab                     (RunPropertiesCR, DgnGlyphRunLayoutFlags);

    protected:                      virtual DRange3d        _GetNominalRange        () const override;
    protected:                      virtual double          _GetNominalHeight       () const override;
    protected:                      virtual DRange3d        _GetExactRange          () const override;

    protected:                      virtual WString         _ToString               (size_t offset, size_t length, TextBlockToStringOptionsCR) const override;
    protected:                      virtual WChar           _GetCharacter           (size_t) const override;
    protected:                      virtual AppendStatus    _AppendNextRunToLine    (LineR, RunP& nextSplitRun, ProcessContextCR) override;
    protected:                      virtual AppendStatus    _AppendToLine           (LineR, RunP& nextRun, ProcessContextCR) override;
    protected:                      virtual bool            _CanFit                 (LineR, ProcessContextCR) override;
    protected:                      virtual RunP            _Clone                  () const override;
    protected:                      virtual bool            _Equals                 (RunCR, TextBlockCompareOptionsCR) const override;
    protected:                      virtual void            _Draw                   (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const override;

    public:                                 DPoint2dCR      GetTabExtents           () const;
    public:                                 void            SetTabExtents           (DPoint2dCR);
    public:                                 bool            SetTabWidth             (ProcessContextCR);
    public:                                 void            GetLastRunOrigin        (DPoint3dR, LineCR, ProcessContextCR) const;
    public:                                 double          ComputeNextTabStop      (LineCR, ProcessContextCR, DPoint3dCR) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // Tab

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
