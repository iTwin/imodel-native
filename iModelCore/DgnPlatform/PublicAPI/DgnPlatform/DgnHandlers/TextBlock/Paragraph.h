/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Paragraph.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"
#include <DgnPlatform/DgnHandlers/TextBlock/IDgnTextStyleApplyable.h>

//__PUBLISH_SECTION_END__

#include "TextBlockNode.h"

struct DgnImporter;

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! Represents the properties that can be controlled on a per-Paragraph basis.
//!
//! To create a new instance, use the static Create methods, or Clone an existing instance.<br>
//! <br>
//! This class stores property override flags and an optional text style. If a text style is present, override flags will be enabled when you call a Set... method. This means that if changes are made to the underlying text style, said modified properties will not propogate, but remain as-is on a given element because they were overridden. You can clear an override by calling the appropriate Clear...Override method (does nothing if there is no text style); you can also test to see if an override was set by calling the appropriate Is...Overridden method (always returns false if there is no text style). ApplyTextStyle provides the option to maintain and respect overrides, or zero them out and ignore them.<br>
//! <br>
//! If created from a text style, this object is a snapshot; it will not automatically update if you change the text style via other means.<br>
//! <br>
//! Since ParagraphProperties can be used to round trip and clone elements among files, and stores things that are at most per-cache (e.g. sizes in UORs), this class stores (and requires at all times) an associated cache, so effective values can be computed. This DgnModel is expected to always match that of the TextBlock it is used by.
//! <br>
//! This class also effectively implements the IDgnTextStyleApplyable interface; use AsIDgnTextStyleApplyable and AsIDgnTextStyleApplyableR to get direct access.
//!
// @bsiclass                                                    Venkat.Kalyan   08/07
//=======================================================================================
struct ParagraphProperties :    public RefCountedBase
//__PUBLISH_SECTION_END__
                                ,public IDgnTextStyleApplyable
//__PUBLISH_SECTION_START__
{
//__PUBLISH_SECTION_END__
    friend struct Paragraph;

    //=======================================================================================
    // @bsiclass                                                    Venkat.Kalyan   10/07
    //=======================================================================================
    private: struct Overrides
        {
        public: bool    m_justification;
        public: bool    m_isFullJustified;
        public: bool    m_lineSpacingType;
        public: bool    m_lineSpacingValue;

        public:         Overrides ();
        public: void    Clear ();
        public: bool    Equals (Overrides const &) const;

        }; // Overrides

    private:    DgnModelP                   m_dgnModel;
    private:    Overrides                   m_overrides;
    private:    TextElementJustification    m_justification;
    private:    bool                        m_isFullJustified;
    private:    DgnLineSpacingType             m_lineSpacingType;
    private:    double                      m_lineSpacingValue;
    private:    IndentationData             m_indentation;
    private:    UInt32                      m_textStyleId;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT  explicit                        ParagraphProperties     (DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  ParagraphProperties     (TextParamWideCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  ParagraphProperties     (DgnTextStyleCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                  ParagraphProperties     (ParagraphPropertiesCR);

    private:                                    void                InitDefaults            (DgnModelR);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual     bool                _HasTextStyle           () const override;
    protected:                      virtual     UInt32              _GetTextStyleId         () const override;
    protected:                      virtual     DgnTextStylePtr     _GetTextStyleInFile     () const override;
    protected:                      virtual     void                _ToStyle                (DgnTextStyleR) const override;

    protected:                      virtual     void                _ApplyTextStyle         (DgnTextStyleCR, bool respectOverrides) override;
    protected:                      virtual     void                _RemoveTextStyle        () override;
    protected:                      virtual     void                _SetProperties          (DgnTextStyleCR, DgnTextStylePropertyMaskCR) override;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT              void                ToElementData           (TextParamWideR) const;
    public:     DGNPLATFORM_EXPORT              void                FromElementData         (TextParamWideCR);
    public:     DGNPLATFORM_EXPORT              bool                Equals                  (ParagraphPropertiesCR) const;
    public:     DGNPLATFORM_EXPORT              bool                Equals                  (ParagraphPropertiesCR, double tolerance) const;

    private:                                    void                SetPropertiesFromStyle  (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR, DgnTextStylePropertyMaskCR overridesMask);

    // Do NOT publish this variant; it is here to maintain the reference counted pattern internally.
    public: DGNPLATFORM_EXPORT static ParagraphPropertiesPtr Create (TextParamWideCR, DgnModelR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable const & AsIDgnTextStyleApplyable () const;

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable& AsIDgnTextStyleApplyableR ();

    //! Creates a new instance of ParagraphProperties with default (e.g. zero'ed) values.
    public: DGNPLATFORM_EXPORT static ParagraphPropertiesPtr Create (DgnModelR);

    //! Creates a new instance of ParagraphProperties with applicable properties from the provided style.
    public: DGNPLATFORM_EXPORT static ParagraphPropertiesPtr Create (DgnTextStyleCR, DgnModelR);

    //! Creates a new instance as a deep copy of this instance.
    public: DGNPLATFORM_EXPORT ParagraphPropertiesPtr Clone () const;

    //! Gets the DGN cache that this instance is associated with.
    public: DGNPLATFORM_EXPORT DgnModelR GetDgnModelR () const;

    //! Gets the justification.
    //! @note   Mixing justifications within the same TextBlock is experimental at this time.
    public: DGNPLATFORM_EXPORT TextElementJustification GetJustification () const;

    //! Sets the justification.
    public: DGNPLATFORM_EXPORT void SetJustification (TextElementJustification);

    //! True if the  property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsJustificationOverridden () const;

    //! Clears the  property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearJustificationOverride ();

    //! Determines if lines should be full justified.
    //! Full justification means that the space between words is adjusted so that every line begins and ends in a uniform location. The longest measured line is the base width, and all other lines have additional space inserted between words to pad them to take the same width. The last line of a full-justified paragraph is never adjusted, and is layed out normally according to the value available from GetJustification.
    public: DGNPLATFORM_EXPORT bool IsFullJustified () const;

    //! Sets if lines should be full justified.
    //! @see    IsFullJustified for additional notes
    public: DGNPLATFORM_EXPORT void SetIsFullJustified (bool);

    //! True if the is full justified property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsFullJustifiedOverridden () const;

    //! Clears the is full justified property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsFullJustifiedOverride ();

    //! Gets the line spacing type.
    public: DGNPLATFORM_EXPORT DgnLineSpacingType GetLineSpacingType () const;

    //! Sets the line spacing type.
    public: DGNPLATFORM_EXPORT void SetLineSpacingType (DgnLineSpacingType);

    //! True if the line spacing type property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsLineSpacingTypeOverridden () const;

    //! Clears the line spacing type property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearLineSpacingTypeOverride ();

    //! Gets the line spacing value.
    //! This value has different meaning according to the value available from GetLineSpacingType (e.g. it could be a distance in UORs, or a factor).
    public: DGNPLATFORM_EXPORT double GetLineSpacingValue () const;

    //! Sets the line spacing value.
    //! @see    GetLineSpacingValue for additional notes
    public: DGNPLATFORM_EXPORT void SetLineSpacingValue (double);

    //! True if the line spacing value property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsLineSpacingValueOverridden () const;

    //! Clears the line spacing value property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearLineSpacingValueOverride ();

    //! Gets the indentation of this paragraph.
    public: DGNPLATFORM_EXPORT IndentationDataCR GetIndentation () const;

    //! Sets the indentation of this paragraph.
    public: DGNPLATFORM_EXPORT void SetIndentation (IndentationDataCR);

    //! Scales all scalar values in this instace.
    //! This includes the line spacing value.
    public: DGNPLATFORM_EXPORT void ApplyScale (DPoint2dCR, bool isVertical);

    }; // ParagraphProperties

//=======================================================================================
//! In the TextBlock DOM, a collection of lines and runs.
//!
//! Although a Paragraph technically contains lines (which contain runs), they existly solely for layout purposes, and are not directly exposed; therefore, you should consider a Paragraph to be a collection of runs.<br>
//! <br>
//! Aside from serving as a layout container, paragraphs store unique properties that apply to their runs, including indentation, line spacing, and justification. LineBreak runs create new lines in the same paragraph, whereas ParagraphBreak runs create new paragraphs. Currently, a TextBlock always has at least one, possibly empty, paragraph. As new paragraphs are added, they assume the properties of the last paragraph.
//!
//! @note As with other DOM objects, direct access to this object is intended to be read-only.
//! @see  TextBlock::AppendParagraphBreak to effectively create and append a Paragraph to a TextBlock.
//!
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct Paragraph
//__PUBLISH_SECTION_END__
    : public TextBlockNode
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    DEFINE_T_SUPER(TextBlockNode)
    friend struct  TextBlock;
    friend struct  DgnImporter;

    private:    ParagraphProperties m_properties;
    private:    LineArray           m_lineArray;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    private:                                    void                        ExtractNodes                        (CaretCR, TextBlockNodeArrayR);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:                         explicit                                Paragraph                           (DgnModelR);
    public:                                                                 Paragraph                           (ParagraphCR);
    public:                         virtual                                 ~Paragraph                          ();

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual     TextBlockNodeLevel          _GetUnitLevel                       () const override;

    public:                                     void                        Preprocess                          (DgnModelP);
    public:                                     void                        InitFrom                            (ParagraphCR);

    public:                                     void                        AddLine                             (LineR line);

    public:                                     AppendStatus                AppendNodes                         (TextBlockNodeArrayR, ProcessContextR);
    protected:                      virtual     void                        _Drop                               (TextBlockNodeArrayR) override;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:    DGNPLATFORM_EXPORT               UInt32                      GetLineCount                        () const;
    public:    DGNPLATFORM_EXPORT               LineP                       GetLine                             (size_t) const;
    public:                                     void                        Draw                                (ViewContextR, bool isViewIndependent, TextBlockDrawOptionsCR) const;

    public:                                     bool                        IsComplete                          () const;
    public:                                     bool                        IsEmpty                             () const;

    public:                                     void                        Scale                               (DPoint2dCR scale, bool vertical);

    public:                                     BentleyStatus               ComputeCaretAtLocation              (CaretR, DPoint3dCR, bool isVertical, bool isStrict) const;
    public:                                     void                        ComputeCaretParameters              (DPoint3dR location, DVec3dR direction, CaretCR pointer, TextBlockCR) const;

    public:                                     void                        ComputeRange                        (bool recomputeComponentRanges, double nodeNumberHeight);
    public:                                     DVec3d                      ComputeLineSpacing                  (bool isVertical, double logicalNodeNumberHeight, double annotationScale, LineCR existingLine, LineCR newLine) const;

    public:                                     void                        SetLineSpacingType                  (DgnLineSpacingType);
    public:                                     void                        SetLineSpacingTypeOverrideFlag      (bool);
    public:                                     void                        SetLineSpacing                      (double);
    public:                                     void                        SetLineSpacingOverrideFlag          (bool);
    public:    DGNPLATFORM_EXPORT               void                        SetJustification                    (TextElementJustification);
    public:                                     void                        SetJustificationOverrideFlag        (bool);
    public:    DGNPLATFORM_EXPORT               void                        SetProperties                       (ParagraphPropertiesCR);

    public:                                     double                      GetIndentForLine                    (LineCP) const;

    public:    DGNPLATFORM_EXPORT               DgnLineSpacingType             GetLineSpacingType                  () const;
    public:    DGNPLATFORM_EXPORT               double                      GetLineSpacing                      () const;
    public:    DGNPLATFORM_EXPORT               TextElementJustification    GetJustification                    () const;
    public:    DGNPLATFORM_EXPORT               bool                        IsFullJustification                 () const;
    public:    DGNPLATFORM_EXPORT               bool                        GetIsFullJustification              () const;
    public:                                     void                        GetParagraphStyle                   (TextParamWideR) const;
    public:                                     bool                        Equals                              (ParagraphCR, TextBlockCompareOptionsCR) const;
    public:                                     void                        ComputeTransformedHitTestRange      (DRange3dR) const;
    public:                                     void                        ComputeElementRange                 (DRange3dR) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Gets this paragraph's properties.
    public: DGNPLATFORM_EXPORT ParagraphPropertiesCR GetProperties () const;

    }; // Paragraph

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
