/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextBlock.h $
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

#include <DgnPlatform/DgnHandlers/MdlTextInternal.h>

struct DgnImporter;

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup TextModule
/// @beginGroup

//=======================================================================================
//! Represents the properties that can be controlled on a per-TextBlock basis.
//!
//! To create a new instance, use the static Create methods, or Clone an existing instance.<br>
//! <br>
//! This class stores property override flags and an optional text style. If a text style is present, override flags will be enabled when you call a Set... method. This means that if changes are made to the underlying text style, said modified properties will not propogate, but remain as-is on a given element because they were overridden. You can clear an override by calling the appropriate Clear...Override method (does nothing if there is no text style); you can also test to see if an override was set by calling the appropriate Is...Overridden method (always returns false if there is no text style). ApplyTextStyle provides the option to maintain and respect overrides, or zero them out and ignore them.<br>
//! <br>
//! If created from a text style, this object is a snapshot; it will not automatically update if you change the text style via other means.<br>
//! <br>
//! Since TextBlockProperties can be used to round trip and clone elements among files, and stores things that are at most per-cache (e.g. sizes in UORs), this class stores (and requires at all times) an associated cache, so effective values can be computed. This DgnModel is expected to always match that of the TextBlock it is used by. Attempting to mix DgnModel objects can lead to undefined behavior.
//! <br>
//! This class also effectively implements the IDgnTextStyleApplyable interface; use AsIDgnTextStyleApplyable and AsIDgnTextStyleApplyableR to get direct access.
//!
// @bsiclass                                                    Venkat.Kalyan   08/07
//=======================================================================================
struct TextBlockProperties :    public RefCountedBase
//__PUBLISH_SECTION_END__
                                ,public IDgnTextStyleApplyable
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    friend struct TextBlock;
    friend struct TextParamAndScale;

    //=======================================================================================
    // @bsiclass                                                    Venkat.Kalyan   10/07
    //=======================================================================================
    private: struct Overrides
        {
        public: bool    m_isBackwards;
        public: bool    m_isUpsideDown;
        public: bool    m_isVertical;
        public: bool    m_maxCharactersPerLine;

        public:         Overrides   ();
        public: void    Clear       ();
        public: bool    Equals      (Overrides const &) const;

        }; // Overrides

    private:    DgnModelP                   m_dgnModel;
    private:    Overrides                   m_overrides;
    private:    bool                        m_hasAnnotationScale;
    private:    double                      m_annotationScale;
    private:    bool                        m_isBackwards;
    private:    bool                        m_isUpsideDown;
    private:    bool                        m_isViewIndependent;
    private:    bool                        m_isVertical;
    private:    bool                        m_isFitted;
    private:    UInt32                      m_maxCharactersPerLine;
    private:    double                      m_documentWidth;
    private:    UInt32                      m_textStyleId;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT  explicit                                TextBlockProperties             (DgnModelR);
    public:     DGNPLATFORM_EXPORT                                          TextBlockProperties             (TextParamWideCR, DPoint2dCR fontSize, UInt32 maxCharactersPerLine, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                          TextBlockProperties             (DgnTextStyleCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                          TextBlockProperties             (TextBlockPropertiesCR);

    private:                                    void                        InitDefaults                    (DgnModelR);

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    protected:                      virtual     bool                        _HasTextStyle                   () const override;
    protected:                      virtual     UInt32                      _GetTextStyleId                 () const override;
    protected:                      virtual     DgnTextStylePtr             _GetTextStyleInFile             () const override;
    protected:                      virtual     void                        _ToStyle                        (DgnTextStyleR) const override;

    protected:                      virtual     void                        _ApplyTextStyle                 (DgnTextStyleCR, bool respectOverrides) override;
    protected:                      virtual     void                        _RemoveTextStyle                () override;
    protected:                      virtual     void                        _SetProperties                  (DgnTextStyleCR, DgnTextStylePropertyMaskCR) override;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT              void                        ToElementData                   (TextParamWideR, UInt32* maxCharactersPerLine) const;
    public:     DGNPLATFORM_EXPORT              void                        FromElementData                 (TextParamWideCR, DPoint2dCR fontSize, UInt32 maxCharactersPerLine);
    public:     DGNPLATFORM_EXPORT              bool                        Equals                          (TextBlockPropertiesCR) const;
    public:     DGNPLATFORM_EXPORT              bool                        Equals                          (TextBlockPropertiesCR, double tolerance) const;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT              bool                        IsFitted                        () const;
    public:     DGNPLATFORM_EXPORT              void                        SetIsFitted                     (bool);

    private:                                    void                        SetPropertiesFromStyle          (DgnTextStylePropertyMaskCR applyMask, DgnTextStyleCR, DgnTextStylePropertyMaskCR overridesMask);

    // Do NOT publish this variant; it is here to maintain the reference counted pattern internally.
    public: DGNPLATFORM_EXPORT static TextBlockPropertiesPtr Create (TextParamWideCR, DPoint2dCR fontSize, UInt32 maxCharactersPerLine, DgnModelR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable const & AsIDgnTextStyleApplyable () const;

    //! Provides access to the IDgnTextStyleApplyable interface that this object effectively implements.
    public: DGNPLATFORM_EXPORT IDgnTextStyleApplyable& AsIDgnTextStyleApplyableR ();

    //! Creates a new instance of TextBlockProperties with default (e.g. zero'ed) values.
    public: DGNPLATFORM_EXPORT static TextBlockPropertiesPtr Create (DgnModelR);

    //! Creates a new instance of TextBlockProperties with applicable properties from the provided style.
    public: DGNPLATFORM_EXPORT static TextBlockPropertiesPtr Create (DgnTextStyleCR, DgnModelR);

    //! Creates a new instance as a deep copy of this instance.
    public: DGNPLATFORM_EXPORT TextBlockPropertiesPtr Clone () const;

    //! Gets the DGN cache that this instance is associated with.
    public: DGNPLATFORM_EXPORT DgnModelR GetDgnModelR () const;

    //! True if an annotation scale has been specified.
    public: DGNPLATFORM_EXPORT bool HasAnnotationScale () const;

    //! Removes annotation scale.
    public: DGNPLATFORM_EXPORT void ClearAnnotationScale ();

    //! Gets the annotation scale.
    //! @note   This value is only valid if HasAnnotationScale.
    public: DGNPLATFORM_EXPORT double GetAnnotationScale () const;

    //! Sets the annotation scale.
    //! Calling this sets the value, and makes this instance recognize it (e.g. via HasAnnotationScale).
    public: DGNPLATFORM_EXPORT void SetAnnotationScale (double);

    //! Gets the is backwards value.
    public: DGNPLATFORM_EXPORT bool IsBackwards () const;

    //! Sets the is backwards value.
    public: DGNPLATFORM_EXPORT void SetIsBackwards (bool);

    //! True if the is backwards property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsBackwardsOverridden () const;

    //! Clears the is backwards override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsBackwardsOverride ();

    //! Gets the is upside down value.
    public: DGNPLATFORM_EXPORT bool IsUpsideDown () const;

    //! True if the is upside down property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsUpsideDownOverridden () const;

    //! Clears the is upside down property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsUpsideDownOverride ();

    //! Sets the is upside down value.
    public: DGNPLATFORM_EXPORT void SetIsUpsideDown (bool);

    //! Gets the is view independent value.
    public: DGNPLATFORM_EXPORT bool IsViewIndependent () const;

    //! Sets the is view independent value.
    public: DGNPLATFORM_EXPORT void SetIsViewIndependent (bool);

    //! Gets this is vertical value.
    //! This vertical flag means that individual glyphs are layed out in a top-down fashing (e.g. as if a line feed existing after every glyph), and is more of a decoration than a proper way to represent vertically flowing languages. In these cases, you will want to use a vertical font (e.g. a TrueType font with the '@' indicator), and rotate the text 90 degrees.
    public: DGNPLATFORM_EXPORT bool IsVertical () const;

    //! Sets the is vertical value.
    //! @see    IsVertical for additional notes
    public: DGNPLATFORM_EXPORT void SetIsVertical (bool);

    //! True if the is vertical property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsVerticalOverridden () const;

    //! Clears the is vertical property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearIsVerticalOverride ();

    //! Gets the maximum number of characters to place on a line.
    public: DGNPLATFORM_EXPORT UInt32 GetMaxCharactersPerLine () const;

    //! Gets the maximum number of characters to place on a line.
    //! @see GetMaxCharactersPerLine for additional notes
    public: DGNPLATFORM_EXPORT void SetMaxCharactersPerLine (UInt32);

    //! True if the max characters per line property is overridden from the underlying style.
    public: DGNPLATFORM_EXPORT bool IsMaxCharactersPerLineOverridden () const;

    //! Clears the max characters per line property override flag and reverts its value from the underlying style.
    public: DGNPLATFORM_EXPORT void ClearMaxCharactersPerLineOverride ();

    //! Gets the maximum width of the document (in UORs) at which lines will wrap.
    //! Wrapping only occurs at word-break characters, which include whitespace.
    public: DGNPLATFORM_EXPORT double GetDocumentWidth () const;

    //! Sets the maximum width of the document (in UORs) at which lines will wrap.
    //! @see    GetDocumentWidth for additional notes
    public: DGNPLATFORM_EXPORT void SetDocumentWidth (double);

    //! Scales all scalar values in this instace.
    //! This includes the document width value.
    public: DGNPLATFORM_EXPORT void ApplyScale (DPoint2dCR);

    }; // TextBlockProperties

//=======================================================================================
//! In the TextBlock DOM, this is the master object that represents a piece of text as a whole, and is the primary high-level object used to deal with multi-line, formatted text (and is also generally recommended for any text, regardless of complexity).
//!
//! As described in the Text module documentation, TextBlock consists of a DOM (Document Object Model). Elements of the DOM include Paragraph and Run objects. Internally, lines are also computed, but only affect layout, and are not directly exposed.<br>
//! <br>
//! @attention TextBlock objects are meant to have a short lifetime, and store information that can potentially become invalidated when retained for a long period of time (e.g. text style indices, or even DGN caches).
//!
//! You can navigate and identify specific parts of the DOM via the Caret. To acquire a Caret, use CreateStartCaret or CreateEndCaret. You can then ask the Caret for specific information, such as its current paragraph and run, as well as navigate it in either direction. It is important to note that Caret objects merely store indices into the DOM; modifying a TextBlock potentially invalidates any existing Caret objects.<br>
//! <br>
//! Any DOM objects you obtain from TextBlock are meant to be read-only, and are for reference only. You cannot instantiate DOM objects directly, but must instead ask TextBlock to create and append an object for you. See the Append... methods.<br>
//! <br>
//! Enter Data Fields (EDFs; represented by EdfCharStream) are atomic runs. While you are free to have a Caret step into the individual characters of these runs for query purposes, you cannot inject characters into them (via Insert* methods), or delete portions of them (via Remove). If you attempt to do so, the API will round to one side or the other to preserve the atomic run.<br>
//! <br>
//! TextBlock maintains 3 property structures: for itself (TextBlockProperties), and to use for any paragraphs and runs that are added (ParagraphProperties and RunProperties). You do not need to provide property structures each time you append a DOM node; these 'seed' properties are used until they are changed (see SetParagraphPropertiesForAdd and SetRunPropertiesForAdd).<br>
//! <br>
//! TextBlock I/O:
//! <ul>
//!   <li>To acquire a TextBlock from an element, use the ITextQuery handler interface</li>
//!   <li>To modify the text of an element, use the ITextQuery and ITextEdit interfaces to get/set the text</li>
//!   <li>To create a free-standing text element from a TextBlock, use TextHandlerBase::CreateElement</li>
//! </ul>
//!
// @bsiclass                                                    Venkat.Kalyan   07/2007
//=======================================================================================
struct TextBlock : public RefCountedBase
{
    //=======================================================================================
    //! Describes the possible results from generating an element from a TextBlock. @see TextHandlerBase::CreateElement.
    // @bsiclass                                                    Jeff.Marker     06/2009
    //=======================================================================================
    public: enum ToElementResult
        {
        TO_ELEMENT_RESULT_Success,   //!< The operation completed successfully.
        TO_ELEMENT_RESULT_Error,     //!< An unknown failure occured; no element was created.
        TO_ELEMENT_RESULT_Empty      //!< An element was not created because an empty TextBlock was provided (and it did not have the force text node flag).

        }; // ToElementResult

//__PUBLISH_SECTION_END__

    friend struct  Caret;
    friend struct  ::DgnImporter;

    private:            DgnModelP               m_dgnModel;
    private:            TextBlockProperties     m_properties;
    private:            ParagraphArray          m_paragraphArray;
    private:            RotMatrix               m_orientation;
    private:    mutable DPoint3d                m_origin;
    private:            OriginType              m_primaryOriginType;
    private:            DPoint3d                m_primaryOrigin;
    private:            TextBlockType           m_type;
    private:            bool                    m_isForEditing;

    private:            UInt32                  m_nodeNumber;
    private:            double                  m_fittedLength;
    private:            bool                    m_V7Compatible;
    private:            bool                    m_forceTextNode;
    private:            RunPropertiesP          m_nodeProperties;
    private:            Caret                   m_dirty;
    private:            ProcessLevel            m_processLevel;
    private:    mutable DRange3d                m_nominalRange;
    private:    mutable DRange3d                m_exactRange;

    private:            UInt64                  m_alongRootId;
    private:            DPoint3d                m_descrStartPoint;
    private:            double                  m_descrOffset;
    private:            double                  m_alongDist;
    private:            DescriptorSide          m_descrSide;
    private:            bool                    m_useAlongDist;
    private:            DgnModelP            m_alongDgnModel;

    private:            ParagraphProperties     m_paragraphPropertiesForAdd;
    private:            RunProperties           m_runPropertiesForAdd;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    private:                                    DVec3d                          GetJustificationOffset                          () const;
    private:                                    DVec3d                          GetParagraphSpacing                             (bool isVertical, double logicalNodeNumberHeight, double annotationScale, ParagraphCR existingParagraph, ParagraphCR newParagraph) const;
    private:                                    void                            ComputeRange                                    (bool recomputeComponentRanges);
    private:                                    void                            ApplyAnnotationScale                            (double annotationScale, bool applyAnnotationScaleFlag);
    private:                                    void                            CreateTextElement                               (EditElementHandleR, ElementHandleCP templateEl, DgnModelP) const;
    private:                                    void                            CreateTextNode                                  (EditElementHandleR, ElementHandleCP templateEl, DgnModelP) const;
    private:                                    void                            CreateAlongText                                 (EditElementHandleR, ElementHandleCP templateEl, DgnModelP) const;
    private:                                    void                            AppendAlongTextDependency                       (EditElementHandleR) const;
    private:                                    void                            AppendComponentTextElementsToNode               (RunIteratorCR, EditElementHandleR, ElementHandleCP textElemTemplate, ElementHandleCP textNodeTemplate, DgnModelP) const;
    private:                                    Transform                       GetLineTransformAtCaret                         (CaretCR) const;
    private:                                    Transform                       GetRunTransformAtCaret                          (CaretCR) const;
    private:                                    bool                            IsSingleLineFontTrueType                        () const;
    private:                                    BentleyStatus                   GetAlongTextDependency                          (AlongTextDependencyR atDep) const;
    private:                                    void                            FromTextElement                                 (ElementHandleCR elemHandle);
    private:                                    void                            FromTextNodeElement                             (ElementHandleCR elemHandle);
    private:                                    bool                            ExtractAlongTextParameters                      (ElementHandleCR elemHandle);
    private:                                    void                            MirrorGetNewParameters                          (DPoint3d& origin, RotMatrix& rMatrix, TextElementJustification& just, TransformCP  pTransform);
    private:                                    void                            InitDefaults                                    (DgnModelR);
    private:                                    template <typename T>
                                                void                            SetParagraphProperty                            (void (Paragraph::*method) (T), T val, ParagraphRangeCR);
    private:                                    bool                            JustifyVerticalLine                             (LineR line, ParagraphCR paragraph, double maxHeight, double indent, bool wordWrappedText);
    private:                                    bool                            JustifyHorizontalLine                           (LineR line, ParagraphCR paragraph, double maxWidth, double indent, bool wordWrappedText);
    private:                                    void                            JustifyLines                                    (double minimumReferenceDistance);
    private:                                    double                          GetTextBlockReferenceDistance                   () const;
    private:                                    void                            GetLineReferenceDistances                       (double& referenceDistance, double& alignDistance, LineCR line, HorizontalJustification hJust) const;
    private:                                    bool                            GetRunPropertiesForInsertion                    (RunPropertiesR prop, CaretCR caret) const;
    private:                                    RunPropertiesCP                 GetRunProperty                                  (TextBlockNodeCP) const;
    private:                                    void                            ComputeLocalCaretParameters                     (DPoint3dR location, DVec3dR direction, CaretCR caret) const;
    private:                                    void                            ProcessRangeForSelectionBox                     (bvector<DRange3d>&, CaretCR, DRange3dR, size_t lineStartIdx) const;
    private:                                    void                            IsolateRange                                    (CaretR from, CaretR to);
    private:                                    DRange3d                        ComputeJustificationRange                       () const;
    private:                                    void                            RecomputeTextBlockOrigin                        () const;
    private:                                    UInt32                          ReplaceStaticFractionsWithStackedFractions      (CaretCR start, CaretCR end);
    private:                                    void                            Drop                                            (TextBlockNodeArrayR);
    private:                                    void                            SetOriginForNextParagraph                       (ParagraphP nextParagraph, ParagraphCP currentParagraph) const;
    private:                                    void                            AppendNodesNormal                               (TextBlockNodeArrayR);
    private:                                    void                            InsertNodes                                     (CaretR insertLocation, TextBlockNodeArrayR);
    private:                                    void                            ExtractNodes                                    (CaretR caret, TextBlockNodeArrayR);
    private:                                    void                            RemoveNodes                                     (CaretCR caret);
    private:                                    BentleyStatus                   AppendNodesAlongElement                         (TextBlockNodeArrayCR);
    private:                                    bool                            OffsetCaret                                     (CaretR pointerToOffset, CaretCP pointerToOffsetFrom, int numUnits) const;
    private:                                    Transform                       GetFlipTransform                                () const;
    private:                                    void                            Preprocess                                      (DgnModelP modelRef);
    private:                                    void                            Scale                                           (DPoint2dCR scale);
    private:                                    TextElementJustification        MapJustificationsForMirroring                   (TextElementJustification justification, bool xFlip, bool yFlip) const;
    private:                                    double                          ComputeVerticalDescenderJustificationOffset     () const;
    private:                                    OriginType                      GetOriginType                                   () const;
    private:                                    void                            SetProcessLevel                                 (ProcessLevel processLevel);
    private:                                    size_t                          GetRunCount                                     (CaretCR start, CaretCR end) const;
    private:                                    size_t                          GetCharacterCount                               (CaretCR start, CaretCR end) const;
    private:                         static     void                            ComputePlanarRangeVertices                      (DPoint3d vertices[], DRange3dCR range);
    private:                                    void                            ChangeCaseInternal                              (CaretCR from, CaretCR to, WChar (*changeCaseCallback)(WChar, WChar));

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT  explicit                                    TextBlock                                       (ElementHandleCR);
    public:     DGNPLATFORM_EXPORT  explicit                                    TextBlock                                       (DgnModelR);
    public:     DGNPLATFORM_EXPORT                                              TextBlock                                       (TextBlockPropertiesCR, ParagraphPropertiesCR, RunPropertiesCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                              TextBlock                                       (DgnTextStyleCR, DgnModelR);
    public:     DGNPLATFORM_EXPORT                                              TextBlock                                       (TextBlockCR);
    public:     DGNPLATFORM_EXPORT  virtual                                     ~TextBlock                                      ();

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:                                     double                          GetNominalWidth                                 () const;
    public:                                     double                          GetExactWidth                                   () const;
    public:                                     double                          GetNominalHeight                                () const;
    public:                                     double                          GetExactHeight                                  () const;
    public:                                     DPoint3d                        GetOrigin                                       () const;
    public:     DGNPLATFORM_EXPORT              Transform                       GetTransform                                    () const;
    public:                                     void                            AppendParagraph                                 (ParagraphR paragraph);
    public:                                     void                            AppendNodes                                     (TextBlockNodeArrayR);
    public:                                     void                            ReplaceNodes                                    (CaretR start, CaretR end, TextBlockNodeArrayR);
    public:                                     bool                            IsInField                                       (CaretCR caret) const;
    public:                                     double                          ComputeHorizontalDescenderJustificationOffset   () const;
    public:                                     double                          GetPaperWidth                                   () const;
    public:                                     void                            ReplaceFieldsWithNormalString                   ();
    public:                                     LineP                           GetLine                                         (size_t index) const;
    public:                                     void                            AppendFixedSpace                                (double width);
    public:                                     DPoint3d                        GetTextElementOrigin                            () const;
    public:                                     void                            GetLineExactLocalRange                          (DRange3dR range, size_t lineNo) const;
    public:    DGNPLATFORM_EXPORT               void                            ApplyTransform                                  (TransformCR transform);
    public:                                     void                            SetLineSpacingType                              (DgnLineSpacingType type, ParagraphRangeCR range);
    public:                                     void                            SetLineSpacingTypeOverrideFlag                  (bool overrideFromStyle, ParagraphRangeCR range);
    public:                                     void                            SetLineSpacingValueOverrideFlag                 (bool overrideFromStyle, ParagraphRangeCR range);
    public:                                     void                            SetLineSpacingValue                             (double exactLineSpacing, ParagraphRangeCR range);
    public:                                     void                            SetCharacterSpacing                             (double spacing, CharacterSpacingType characterSpacingType, CaretCR start, CaretCR end);
    public:                                     void                            AppendStackedFraction                           (WCharCP numeratorText, WCharCP denominatorText, StackedFractionType, StackedFractionAlignment, DPoint2dCP fractionScale);
    public:                                     DPoint3d                        GetTextOrigin                                   () const;
    public:                                     bool                            GetForceTextNodeFlag                            () const;
    public:                                     TextBlockType                   GetType                                         () const;
    public:                                     bool                            IsDGNType                                       () const;
    public:                                     bool                            IsDTextType                                     () const;
    public:                                     ToElementResult                 ToElement                                       (EditElementHandleR editElemHandle, DgnModelP targetDgnModel, ElementHandleCP templateElemHandle) const;
    public:                                     DgnGlyphRunLayoutFlags          ComputeRunLayoutFlags                           () const;

    //-----------------------------------------------------------------------------------------------------------------------------------------------
    public:     DGNPLATFORM_EXPORT              DRange3d                        ComputeTransformedNominalRange                  () const;
    public:     DGNPLATFORM_EXPORT              DRange3d                        GetExactRange                                   () const;
    public:     DGNPLATFORM_EXPORT              DRange3d                        ComputeEditorRange                              (DPadding* decorationPadding, ViewportCR) const;
    public:     DGNPLATFORM_EXPORT              UInt32                          GetParagraphCount                               () const;
    public:     DGNPLATFORM_EXPORT              ParagraphP                      GetParagraph                                    (size_t index) const;
    public:     DGNPLATFORM_EXPORT              DgnTextStylePtr                 GetTextStyle                                    () const;
    public:     DGNPLATFORM_EXPORT  static      void                            GetHorizontalVerticalJustifications             (HorizontalJustification* horizontalJustification, VerticalJustification* verticalJustification, TextElementJustification justification);
    public:     DGNPLATFORM_EXPORT  static      TextElementJustification        GetJustificationFromAlignments                  (HorizontalJustification horizontalJustification, VerticalJustification verticalJustification);
    public:     DGNPLATFORM_EXPORT              RunPropertiesP                  GetNodeProperties                               () const;
    public:     DGNPLATFORM_EXPORT              void                            PlayTo                                          (ITextListenerP listener, DgnModelP modelRef) const;
    public:     DGNPLATFORM_EXPORT              ITextListenerPtr                CreateListener                                  ();
    public:     DGNPLATFORM_EXPORT              RunPropertiesCP                 GetFirstRunProperties                           () const;
    public:     DGNPLATFORM_EXPORT              RunPropertiesCP                 GetLastRunProperties                            () const;
    public:     DGNPLATFORM_EXPORT              RunPropertiesCP                 GetTextNodeRunProperties                        () const;
    public:     DGNPLATFORM_EXPORT              void                            SetTextPrimaryOriginType                        (OriginType primaryOriginType);
    public:     DGNPLATFORM_EXPORT              void                            SetTextElementOrigin                            (DPoint3dCR origin);
    public:     DGNPLATFORM_EXPORT              void                            SetTextAutoCADOrigin                            (DPoint3dCR origin);
    public:     DGNPLATFORM_EXPORT              void                            SetV7Compatible                                 (bool V7Compatible);
    public:     DGNPLATFORM_EXPORT              bool                            IsV7Compatible                                  () const;
    public:     DGNPLATFORM_EXPORT              size_t                          ReevaluateFields                                (EvaluationReason);
    public:     DGNPLATFORM_EXPORT              void                            ReplaceText                                     (WCharCP text, CaretCR start, CaretR end);
    public:     DGNPLATFORM_EXPORT              BentleyStatus                   FindText                                        (bool& changeable, CaretR foundStart, CaretR foundEnd, WCharCP searchText, bool regularExpression, bool wholeWords, bool matchCase, CaretCR offsetStart, CaretCR offsetEnd) const;
    public:     DGNPLATFORM_EXPORT              void                            ComputeCaretParameters                          (DPoint3dR location, DVec3dR direction, CaretCR caret) const;
    public:     DGNPLATFORM_EXPORT              void                            Clear                                           ();
    public:     DGNPLATFORM_EXPORT              bool                            IsComplexText                                   () const;
    public:     DGNPLATFORM_EXPORT              DPoint3d                        GetTextAutoCADOrigin                            () const;
    public:     DGNPLATFORM_EXPORT              ParagraphPropertiesCP           GetParagraphProperties                          (size_t index) const;
    public:     DGNPLATFORM_EXPORT              size_t                          GetLineCount                                    (CaretCR start, CaretCR end) const;
    public:     DGNPLATFORM_EXPORT              void                            FromDText                                       (WStringCR markup, bool isShapeCode = false);
    public:     DGNPLATFORM_EXPORT              void                            FromMText                                       (WCharCP markupString, IDwgContextCR dwgContext, double scaleToDGN);
    public:     DGNPLATFORM_EXPORT              void                            ToDText                                         (WStringR markupString, DgnModelP modelRef, bool shouldExportEdfsAsUnderscores) const;
    public:     DGNPLATFORM_EXPORT              void                            ToMText                                         (WStringR markupString, DgnModelP modelRef, IDwgContextCP dwgContext, double scaleToDGN, bool expandFields, bool shouldExportEdfsAsUnderscores) const;
    public:     DGNPLATFORM_EXPORT              void                            ConvertToMTextCompatibleTextBlock               ();
    public:     DGNPLATFORM_EXPORT              bool                            IsDTextCompatible                               () const;
    public:     DGNPLATFORM_EXPORT              bool                            IsMTextCompatible                               (DgnModelP modelRef) const;
    public:     DGNPLATFORM_EXPORT              bool                            IsValidUserOriginForDText                       () const;
    public:     DGNPLATFORM_EXPORT              void                            ComputeUserOriginForDText                       (DPoint3dR userOrigin) const;
    public:     DGNPLATFORM_EXPORT              void                            Refit                                           (double fitLength, bool insertSpacesBetweenCharacters);
    public:     DGNPLATFORM_EXPORT              BentleyStatus                   ComputeCaretAtLocation                          (CaretR, DPoint3dCR, bool isStrict) const;
    public:     DGNPLATFORM_EXPORT              void                            MoveChars                                       (CaretR caret, int nChars) const;
    public:     DGNPLATFORM_EXPORT              bool                            ContainsLeadingIndents                          () const;
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    public:     DGNPLATFORM_EXPORT              FieldCollection                 GetFields                                       () const;
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    public:     DGNPLATFORM_EXPORT              void                            SetAlongDisplayPath                             (DisplayPathCP pAlongDisplayPath);
    public:     DGNPLATFORM_EXPORT              void                            SetDescrOffset                                  (double offset);
    public:     DGNPLATFORM_EXPORT              void                            SetDescrSide                                    (DescriptorSide side);
    public:     DGNPLATFORM_EXPORT              void                            SetDescrStartPoint                              (DPoint3dCR start);
    public:     DGNPLATFORM_EXPORT              void                            SetDescrDistance                                (double distance);
    public:     DGNPLATFORM_EXPORT              void                            RemoveAlongElementDependency                    ();
    public:     DGNPLATFORM_EXPORT              void                            SetIsVertical                                   (bool isVertical);
    public:     DGNPLATFORM_EXPORT              void                            SetVerticalOverrideFlag                         (bool overrideFromStyle);
    public:     DGNPLATFORM_EXPORT              void                            SetMaxCharactersPerLine                         (UInt16);
    public:     DGNPLATFORM_EXPORT              void                            SetMaxCharactersPerLineOverrideFlag             (bool overrideFromStyle);
    public:     DGNPLATFORM_EXPORT              void                            SetLineBreakLength                              (double lineBreakLength);
    public:     DGNPLATFORM_EXPORT              void                            SetTextOrigin                                   (DPoint3dCR origin);
    public:     DGNPLATFORM_EXPORT              void                            SetViewIndependent                              (bool viewIndependent);
    public:     DGNPLATFORM_EXPORT              void                            SetNodeNumber                                   (UInt32 nodeNUmber);
    public:     DGNPLATFORM_EXPORT              void                            SetIsUpsideDown                                 (bool upsideDown);
    public:     DGNPLATFORM_EXPORT              void                            SetUpsideDownOverrideFlag                       (bool overrideFromStyle);
    public:     DGNPLATFORM_EXPORT              void                            SetIsBackwards                                  (bool backwards);
    public:     DGNPLATFORM_EXPORT              void                            SetBackwardsOverrideFlag                        (bool overrideFromStyle);
    public:     DGNPLATFORM_EXPORT              void                            SetForceTextNodeFlag                            (bool forceTextNode);
    public:     DGNPLATFORM_EXPORT              void                            SetType                                         (TextBlockType type);
    public:     DGNPLATFORM_EXPORT              bool                            SetAnnotationScale                              (double annotationScale);
    public:     DGNPLATFORM_EXPORT              bool                            SetApplyAnnotationScaleFlag                     (bool applyAnnotationScale);
    public:     DGNPLATFORM_EXPORT              void                            SetJustification                                (TextElementJustification justification, ParagraphRangeCR range);
    public:     DGNPLATFORM_EXPORT              void                            SetJustificationOverrideFlag                    (bool overrideFromStyle, ParagraphRangeCR range);
    public:     DGNPLATFORM_EXPORT              BentleyStatus                   ReplaceRunProperties                            (RunPropertiesCR updatedRunProperties, size_t& index);
    public:     DGNPLATFORM_EXPORT              void                            SetTextNodeProperties                           (RunPropertiesCP runProperties);
    public:     DGNPLATFORM_EXPORT              void                            AppendField                                     (WCharCP fieldString, DgnModelP modelRef, EvaluationReason* evaluationReason = NULL);
    public:     DGNPLATFORM_EXPORT              void                            GetTextBlockBox                                 (DPoint3d boxPts[], bool exact = true) const;
    public:     DGNPLATFORM_EXPORT              size_t                          GetRunPropertiesCount                           () const;
    public:     DGNPLATFORM_EXPORT              RunPropertiesCP                 GetRunProperties                                (size_t index) const;
    public:     DGNPLATFORM_EXPORT              UInt32                          GetNodeNumber                                   () const;
    public:     DGNPLATFORM_EXPORT              bool                            IsMTextType                                     () const;
    public:     DGNPLATFORM_EXPORT              Caret                           Begin                                           () const;
    public:     DGNPLATFORM_EXPORT              Caret                           End                                             () const;
    public:     DGNPLATFORM_EXPORT              void                            Reprocess                                       ();
    public:     DGNPLATFORM_EXPORT              double                          GetNodeHeight                                   () const;
    public:     DGNPLATFORM_EXPORT              double                          GetNodeOrFirstRunHeight                         () const;
    public:     DGNPLATFORM_EXPORT              void                            SetParagraphPropertiesForRange                  (ParagraphPropertiesCR, ParagraphRangeCR);
    public:     DGNPLATFORM_EXPORT              void                            ApplyTextStyleToRunsInRange                     (DgnTextStyleCR, bool respectOverrides, CaretCR from, CaretCR to);
    public:     DGNPLATFORM_EXPORT              void                            RemoveTextStyleFromRunsInRange                  (CaretCR from, CaretCR to);
    public:     DGNPLATFORM_EXPORT              void                            RevertTextStylesOnRunsInRange                   (CaretCR from, CaretCR to);
    public:     DGNPLATFORM_EXPORT              void                            SetPropertiesOnRunsInRange                      (DgnTextStyleCR, DgnTextStylePropertyMaskCR, CaretCR from, CaretCR to);
    public:     DGNPLATFORM_EXPORT              void                            SetPropertiesOnParagraphsInRange                (DgnTextStyleCR, DgnTextStylePropertyMaskCR, CaretCR from, CaretCR to);
    public:     DGNPLATFORM_EXPORT              void                            ComputeSelectionBoxes                           (bvector<DRange3d>&, CaretCR from, CaretCR to) const;
    public:     DGNPLATFORM_EXPORT              WString                         ToString                                        (CaretCR from, CaretCR to) const;
    public:     DGNPLATFORM_EXPORT              WString                         ToString                                        (CaretCR from, CaretCR to, TextBlockToStringOptionsCR) const;
    public:     DGNPLATFORM_EXPORT              void                            InsertFromTextBlock                             (CaretCR from, CaretCR to, CaretCR at);
    public:     DGNPLATFORM_EXPORT              void                            Draw                                            (ViewContextR, TextBlockDrawOptionsCR) const;
    public:     DGNPLATFORM_EXPORT              void                            ChangeCase                                      (CaretCR from, CaretCR to, ChangeCaseOperation);
    public:     DGNPLATFORM_EXPORT              void                            ComputeElementRange                             (DRange3dR) const;
    public:     DGNPLATFORM_EXPORT              void                            SetIsForEditing                                 (bool);

    //! Compares this instance to another, ignoring aspects in the mask provided.
    //! @note   By default, this is not far from a binary comparison; you most likely want to provide some level of mask, such as TextBlockCompareOptions::CreateForCompareContentAndLocation, to ignore internal noise that doesn't represent any real content that would be persisted to the file.
    public: DGNPLATFORM_EXPORT bool Equals (TextBlockCR, TextBlockCompareOptionsCR) const;

    // Do NOT publish these variants; they are here to maintain the reference counted pattern internally.
    public: DGNPLATFORM_EXPORT static TextBlockPtr Create (ElementHandleCR);
    public: DGNPLATFORM_EXPORT static TextBlockPtr Create (TextBlockCR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Creates a new, blank instance.
    //! @note   The property structures are all effectively zero'ed, and may not result in readable text.
    public: DGNPLATFORM_EXPORT static TextBlockPtr Create (DgnModelR);

    //! Creates a new, blank instance, with the provided seed property structures; as DOM nodes are added, these properties will be used.
    public: DGNPLATFORM_EXPORT static TextBlockPtr Create (TextBlockPropertiesCR, ParagraphPropertiesCR, RunPropertiesCR, DgnModelR);

    //! Creates a new, blank instance, and creates its internal property structures based on the provided text style.
    public: DGNPLATFORM_EXPORT static TextBlockPtr Create (DgnTextStyleCR, DgnModelR);

    //! Create a new Caret that designates the beginning of this instance.
    public: DGNPLATFORM_EXPORT CaretPtr CreateStartCaret () const;

    //! Create a new Caret that designates the end of this instance (one run past the last run).
    public: DGNPLATFORM_EXPORT CaretPtr CreateEndCaret () const;

    //! Gets the associated DgnModel.
    public: DGNPLATFORM_EXPORT DgnModelR GetDgnModelR () const;

    //! Gets the formatting properties.
    //! @note   Non-formatting properties, such as origin and rotation, are stored directly on the TextBlock.
    //! @note   Individual properties cannot be modified directly; to modify a single property, you must clone the existing properties, mutate them, and push them back.
    public: DGNPLATFORM_EXPORT TextBlockPropertiesCR GetProperties () const;

    //! Sets the formatting properties.
    //! @note   You should also call PerformLayout to ensure that the internal structure of TextBlock is up-to-date.
    //! @see    GetProperties for additional notes
    public: DGNPLATFORM_EXPORT void SetProperties (TextBlockPropertiesCR);

    //! Gets the paragraph properties that will be used when any future paragraph DOM nodes are created.
    public: DGNPLATFORM_EXPORT ParagraphPropertiesCR GetParagraphPropertiesForAdd () const;

    //! Gets the paragraph properties that will be used when any future paragraph DOM nodes are created.
    public: DGNPLATFORM_EXPORT ParagraphPropertiesR GetParagraphPropertiesForAddR ();

    //! Sets the paragraph properties that will be used when any future paragraph DOM nodes are created.
    public: DGNPLATFORM_EXPORT void SetParagraphPropertiesForAdd (ParagraphPropertiesCR);

    //! Gets the run properties that will be used when any future run DOM nodes are created.
    public: DGNPLATFORM_EXPORT RunPropertiesCR GetRunPropertiesForAdd () const;

    //! Gets the run properties that will be used when any future run DOM nodes are created.
    public: DGNPLATFORM_EXPORT RunPropertiesR GetRunPropertiesForAddR ();

    //! Sets the run properties that will be used when any future run DOM nodes are created.
    public: DGNPLATFORM_EXPORT void SetRunPropertiesForAdd (RunPropertiesCR);

    //! Gets the user origin.
    //! The user origin is the data point that the user selected when placing the text. Text flows around the user origin based on justification (e.g. top-left text flows down and to the right of the user origin, whereas bottom-right text start above and to the right of the user origin, and flows towards it).
    public: DGNPLATFORM_EXPORT DPoint3d GetUserOrigin () const;

    //! Sets the user origin.
    //! @see    GetUserOrigin for additional notes
    public: DGNPLATFORM_EXPORT void SetUserOrigin (DPoint3dCR);

    //! Gets the rotation.
    public: DGNPLATFORM_EXPORT RotMatrix GetOrientation () const;

    //! Sets the rotation.
    public: DGNPLATFORM_EXPORT void SetOrientation (RotMatrixCR);

    //! Creates a deep copy of this instance.
    public: DGNPLATFORM_EXPORT TextBlockPtr Clone () const;

    //! Determines if this instance is empty.
    public: DGNPLATFORM_EXPORT bool IsEmpty () const;

    //! Appends a string of like-formatted, single-line characters. This is analagous to appending a CharStream.
    //! @note   The string will be processed for control characters such as carriage returns, line feeds, and tabs, and appropriate runs will be added on your behalf.
    public: DGNPLATFORM_EXPORT void AppendText (WCharCP);

    //! Appends a carriage return, beginning a new paragraph. This is analagous to appending a ParagraphBreak.
    //! @note   This does not actually append another paragraph; it merely marks the existing paragraph as complete, and if another run is added, a new paragraph is then created to contain it. Thus, it doesn't matter if you call SetParagraphPropertiesForAdd before or after this method, as long as it is before adding the first run to the new paragraph.
    public: DGNPLATFORM_EXPORT void AppendParagraphBreak ();

    //! Appends a line break, beginning a new line in the same paragraph. This is analagous to appending a LineBreak.
    public: DGNPLATFORM_EXPORT void AppendLineBreak ();

    //! Appends a tab. This is analagous to appending a Tab.
    public: DGNPLATFORM_EXPORT void AppendTab ();

    //! Appends a stacked fraction. Depending on type, this is analagous to appending a NoBarFraction, a HorizontalBarFraction, or a DiagonalBarFraction.
    public: DGNPLATFORM_EXPORT void AppendStackedFraction (WCharCP numeratorText, WCharCP denominatorText, StackedFractionType, StackedFractionAlignment);

    //! Appends an enter data field. This is a fixed-length atomic node that can be used as a placeholder for data.
    //! @note   The maximum length of an enter data field is currently 128.
    //! @note   The value parameter's length should be less than or equal to totalLength (otherwise it will be truncated). If it is less, the resulting run will be padded with space characters (U+0020) according to the EDF justification.
    public: DGNPLATFORM_EXPORT void AppendEnterDataField (WCharCP value, size_t totalLength, EdfJustification);

    //! Inserts a string of like-formatted, single-line characters. This is analagous to inserting a CharStream.
    //! @note   The string will be processed for control characters such as carriage returns, line feeds, and tabs, and appropriate runs will be added on your behalf.
    //! @return The new position of the insert caret. Note that this is not necessarily N insert positions from the original insert position (where N is the length of the string) (e.g. if you append a character to a word that causes it to word-wrap to the next line).
    public: DGNPLATFORM_EXPORT CaretPtr InsertText (CaretCR, WCharCP);

    //! Inserts a carriage return, beginning a new paragraph. This is analagous to inserting a ParagraphBreak.
    public: DGNPLATFORM_EXPORT void InsertParagraphBreak (CaretCR);

    //! Inserts a line break, beginning a new line in the same paragraph. This is analagous to inserting a LineBreak.
    public: DGNPLATFORM_EXPORT void InsertLineBreak (CaretCR);

    //! Inserts a tab. This is analagous to inserting a Tab.
    public: DGNPLATFORM_EXPORT void InsertTab (CaretCR);

    //! Inserts a stacked fraction. Depending on type, this is analagous to inserting a NoBarFraction, a HorizontalBarFraction, or a DiagonalBarFraction.
    //! @return The new position of the insert caret. Note that this is not necessarily N insert positions from the original insert position (where N is the length of the string) (e.g. if you append a character to a word that causes it to word-wrap to the next line).
    public: DGNPLATFORM_EXPORT CaretPtr InsertStackedFraction (CaretCR, WCharCP numeratorText, WCharCP denominatorText, StackedFractionType, StackedFractionAlignment);

    //! Inserts an enter data field. This is a fixed-length atomic node that can be used as a placeholder for data.
    //! @note   The value parameter's length should be less than or equal to totalLength (otherwise it will be truncated). If it is less, the resulting run will be padded with space characters (U+0020) according to the EDF justification.
    //! @note   The maximum length of an enter data field is currently 128.
    //! @return The new position of the insert caret. Note that this is not necessarily N insert positions from the original insert position (where N is the length of the string) (e.g. if you append a character to a word that causes it to word-wrap to the next line).
    public: DGNPLATFORM_EXPORT CaretPtr InsertEnterDataField (CaretCR, WCharCP value, size_t totalLength, EdfJustification);

    //! Removes all content between two carets (from inclusive, to exclusive).
    //! @return The new position of the 'from' caret parameter. Note that the original 'from' caret can become invalid (e.g. if you remove the last run of a paragraph, the resulting paragraph is removed, and 'from' is no longer pointing to a valid insert position).
    public: DGNPLATFORM_EXPORT CaretPtr Remove (CaretCR from, CaretCR to);

    //! Creates a plain-text string representing the entire TextBlock. Note that special runs such as fractions can no longer be uniquely identified as such.
    public: DGNPLATFORM_EXPORT WString ToString () const;

    //! Forces TextBlock to re-layout all runs after the dirty caret.
    //! @note   You only need to call this method after modifying properties; appending or inserting into the TextBlock automatically forces layout.
    public: DGNPLATFORM_EXPORT void PerformLayout ();

    //! Gets the nominal range of the entire TextBlock. This is a union of all nominal ranges, which is a union of every character's cell box.
    //! @note   The nominal range is good to use if you want to generally know the "size" of a TextBlock.
    public: DGNPLATFORM_EXPORT DRange3d GetNominalRange () const;

    //! Renders this TextBlock (all lines, all formatting) to the given context.
    public: DGNPLATFORM_EXPORT void Draw (ViewContextR) const;

    //! Indicates that this TextBlock represents text that is associated to another element; character layout, positioning, and orientation is based on the target element.
    //! @note   The details of "along element" text are not currently available in the published API; if you encounter a TextBlock that is along element, and you require graphical information about it, you can use ElementGraphicsOutput to collect the underlying _ProcessTextString calls.
    public: DGNPLATFORM_EXPORT bool IsAlongElement () const;

    }; // TextBlock

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
