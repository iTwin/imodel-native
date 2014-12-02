/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Run.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"
#include "RunProperties.h"

//__PUBLISH_SECTION_END__

#include "TextBlockNode.h"

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! In the TextBlock DOM, a base class for all other runs.
//!
//! This base class has functionality and data this is common to all kinds of runs, a run being the most atomic piece of a DOM.
//!
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::Append... to effectively create and append runs to a TextBlock.
//!
// @bsiclass                                                    Venkat.Kalyan   01/2006
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Run
//__PUBLISH_SECTION_END__
    : public TextBlockNode
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(TextBlockNode)
    protected:  RotMatrix               m_orientation;
    protected:  DVec3d                  m_lineOffset;
    protected:  RunProperties           m_properties;
    protected:  bool                    m_isLastRunInLine;
    protected:  DgnGlyphRunLayoutFlags  m_layoutFlags;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                                                          Run                                     (TextParamWideCR, DPoint2dCR scale, DgnModelR);
    protected:                                                          Run                                     (RunPropertiesCR, DgnGlyphRunLayoutFlags);
    public:                         explicit                            Run                                     (DgnModelR);
    public:     DGNPLATFORM_EXPORT                                      Run                                     (RunCR);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual     TextBlockNodeLevel      _GetUnitLevel                           () const override;
    protected:                      virtual     DRange3d                _GetNominalRange                        () const override;
    protected:                      virtual     DRange3d                _GetExactRange                          () const override;
    protected:                      virtual     Transform               _GetTransform                           () const override;

    protected:                      virtual     double                  _GetMaxDisplacementAboveOrigin          () const;
    protected:                      virtual     double                  _GetMaxDisplacementBelowOrigin          () const;
    protected:                      virtual     double                  _GetMaxExactHeightAboveOrigin           () const;
    protected:                      virtual     double                  _GetMaxExactDepthBelowOrigin            () const;
    protected:                      virtual     double                  _GetMaxHorizontalCellIncrement          () const;
    protected:                      virtual     WString                 _ToString                               (size_t offset, size_t length, TextBlockToStringOptionsCR) const = 0;
    protected:                      virtual     size_t                  _GetCharacterCount                      () const = 0;
    protected:                      virtual     WChar                   _GetCharacter                           (size_t) const = 0;
    protected:                      virtual     void                    _Splice                                 (RunP& firstRun, RunP& secondRun, size_t index) = 0;
    protected:                      virtual     size_t                  _GetNextWordBreakIndex                  (size_t offset) const = 0;
    protected:                      virtual     AppendStatus            _AppendNextRunToLine                    (LineR, RunP& nextSplitRun, ProcessContextCR) = 0;
    protected:                      virtual     AppendStatus            _AppendToLine                           (LineR, RunP& nextRun, ProcessContextCR) = 0;
    protected:                      virtual     bool                    _CanFit                                 (LineR, ProcessContextCR) = 0;
    protected:                      virtual     void                    _ComputeRange                           () = 0;
    protected:                      virtual     void                    _Preprocess                             () const = 0;
    protected:                      virtual     RunP                    _Clone                                  () const = 0;
    protected:                      virtual     void                    _GetRangesForSelection                  (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const;
    protected:                      virtual     void                    _GetTransformedRangesForSelection       (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const;
    protected:                      virtual     bool                    _IsLastCharSpace                        () const;
    protected:                      virtual     double                  _ComputeLeftEdgeAlignDistance           () const;
    protected:                      virtual     void                    _Draw                                   (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const = 0;
    protected:                      virtual     bool                    _AllowTrailingWordBreak                 () const;
    protected:                      virtual     void                    _SetProperties                          (RunPropertiesCR);
    protected:                      virtual     bool                    _IsLastRunInLine                        () const;
    protected:                      virtual     void                    _SetIsLastRunInLine                     (bool);
    protected:                      virtual     bool                    _ContainsOnlyWhitespace                 () const;
    protected:                      virtual     bool                    _Equals                                 (RunCR, TextBlockCompareOptionsCR) const;
    protected:                      virtual     bool                    _IsContentRun                           () const;
    protected:                      virtual     bool                    _IsAtomic                               () const;
    protected:                      virtual     BentleyStatus           _ComputeCaretAtLocation                 (CaretR, DPoint3dCR, bool isStrict) const;
    protected:                      virtual     void                    _ComputeOffsetToChar                    (DVec3dR, double& scale, CaretCR) const = 0;
    protected:                      virtual     void                    _SetRunLayoutFlags                      (DgnGlyphRunLayoutFlags);
    protected:                      virtual     void                    _ComputeTransformedHitTestRange         (DRange3dR) const;
    protected:                      virtual     void                    _SetOverrideTrailingCharacterOffset     (double);
    protected:                      virtual     void                    _ClearTrailingCharacterOffsetOverride   ();
    protected:                      virtual     void                    _GetElementRange                        (DRange3dR) const = 0;

    protected:                                  bool                    IsForEditing                            () const;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT              double                  GetMaxDisplacementAboveOrigin           () const;
    public:                                     double                  GetMaxDisplacementBelowOrigin           () const;
    public:     DGNPLATFORM_EXPORT              double                  GetMaxExactHeightAboveOrigin            () const;
    public:                                     double                  GetMaxExactDepthBelowOrigin             () const;
    public:                                     double                  GetMaxHorizontalCellIncrement           () const;
    public:     DGNPLATFORM_EXPORT              size_t                  GetCharacterCount                       () const;
    public:                                     WChar                   GetCharacter                            (size_t) const;
    public:                                     void                    Splice                                  (RunP& firstRun, RunP& secondRun, size_t index);
    public:                                     size_t                  GetNextWordBreakIndex                   (size_t offset) const;
    public:                                     AppendStatus            AppendNextRunToLine                     (LineR, RunP& nextSplitRun, ProcessContextCR);
    public:                                     AppendStatus            AppendToLine                            (LineR, RunP& nextRun, ProcessContextCR);
    public:                                     bool                    CanFit                                  (LineR, ProcessContextCR);
    public:                                     void                    ComputeRange                            ();
    public:                                     void                    Preprocess                              () const;
    public:                                     RunP                    Clone                                   () const;
    public:                                     void                    GetRangesForSelection                   (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const;
    public:                                     void                    GetTransformedRangesForSelection        (bvector<DRange3d>&, size_t fromIdx, size_t toIdx) const;
    public:                                     bool                    IsLastCharSpace                         () const;
    public:                                     double                  ComputeLeftEdgeAlignDistance            () const;
    public:                                     void                    Draw                                    (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const;
    public:                                     bool                    AllowTrailingWordBreak                  () const;
    public:                                     void                    SetProperties                           (RunPropertiesCR);
    public:                                     bool                    IsLastRunInLine                         () const;
    public:                                     void                    SetIsLastRunInLine                      (bool val);
    public:     DGNPLATFORM_EXPORT              bool                    ContainsOnlyWhitespace                  () const;
    public:                                     DVec3d                  GetRunSpacing                           () const;
    public:                                     RotMatrixCR             GetOrientation                          () const;
    public:                                     void                    SetOrientation                          (RotMatrixCR);
    public:                                     BentleyStatus           ComputeCaretAtLocation                  (CaretR, DPoint3dCR, bool isStrict) const;
    public:                                     void                    ComputeOffsetToChar                     (DVec3dR, double& scale, CaretCR) const;
    public:                                     void                    SetLineOffset                           (DVec3dCR offset);
    public:     DGNPLATFORM_EXPORT              DVec3d                  GetLineOffset                           () const;
    public:                                     DPoint3d                GetLineOffsetAdjustedOrigin             () const;
    public:                                     void                    ComputeCaretParameters                  (DPoint3dR location, DVec3dR direction, CaretCR pointer) const;
    public:                                     DgnGlyphRunLayoutFlags  GetRunLayoutFlags                       () const;
    public:                                     void                    SetRunLayoutFlags                       (DgnGlyphRunLayoutFlags);
    public:                                     bool                    IsVertical                              () const;
    public:                                     void                    SetIsVertical                           (bool);
    public:                                     bool                    IsBackwards                             () const;
    public:                                     void                    SetIsBackwards                          (bool);
    public:                                     bool                    IsUpsideDown                            () const;
    public:                                     void                    SetIsUpsideDown                         (bool);
    public:                                     bool                    Equals                                  (RunCR, TextBlockCompareOptionsCR) const;
    public:                                     void                    ComputeTransformedHitTestRange          (DRange3dR) const;
    public:                                     void                    SetOverrideTrailingCharacterOffset      (double);
    public:                                     void                    ClearTrailingCharacterOffsetOverride    ();
    public:                                     void                    GetElementRange                         (DRange3dR) const;

    //! A content run is a run that has meaning in terms of caret positioning. For example, LineBreak and CarriageReturn are <i>not</i> content runs.
    public: bool IsContentRun () const;

    //! An atomic run is a run that you cannot insert in the middle of. You may still be able to read from a range.
    public: DGNPLATFORM_EXPORT bool IsAtomic () const;

    //! Creates a simple string representation of this run.
    public: DGNPLATFORM_EXPORT WString ToString (size_t offset, size_t length, TextBlockToStringOptionsCR) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Gets the properties for this run.
    public: DGNPLATFORM_EXPORT RunPropertiesCR GetProperties () const;

    //! Creates a simple string representation of this run.
    //! @note   Non CharStream runs will encode themselves as a string, which will not necessarily convey enough information for a 1:1 correlation (e.g. both a date in a CharStream and a Fraction may appear the same: "1/2"; you must dynamic_cast the run object to determine full information).
    public: DGNPLATFORM_EXPORT WString ToString () const;

    }; // Run

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
