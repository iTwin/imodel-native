/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextAPICommon.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/RefCounted.h>
#include <Bentley/CodePages.h>
#include <DgnPlatform/DgnCore/TextParam.h>

#include <algorithm>
#include <iomanip>
#include <queue>
#include <set>
#include <sstream>
#include <utility>

DGNPLATFORM_TYPEDEFS (Caret)
DGNPLATFORM_TYPEDEFS (CharStream)
DGNPLATFORM_TYPEDEFS (DiagonalBarFraction)
DGNPLATFORM_TYPEDEFS (EdfCharStream)
DGNPLATFORM_TYPEDEFS (Fraction)
DGNPLATFORM_TYPEDEFS (IndentationData)
DGNPLATFORM_TYPEDEFS (LineBreak)
DGNPLATFORM_TYPEDEFS (HorizontalBarFraction)
DGNPLATFORM_TYPEDEFS (NoBarFraction)
DGNPLATFORM_TYPEDEFS (Paragraph)
DGNPLATFORM_TYPEDEFS (ParagraphBreak)
DGNPLATFORM_TYPEDEFS (ParagraphIterator)
DGNPLATFORM_TYPEDEFS (ParagraphProperties)
DGNPLATFORM_TYPEDEFS (ParagraphRange)
DGNPLATFORM_TYPEDEFS (Run)
DGNPLATFORM_TYPEDEFS (RunIterator)
DGNPLATFORM_TYPEDEFS (RunProperties)
DGNPLATFORM_TYPEDEFS (RunRange)
DGNPLATFORM_TYPEDEFS (Tab)
DGNPLATFORM_TYPEDEFS (TextBlock)
DGNPLATFORM_TYPEDEFS (TextBlockProperties)
DGNPLATFORM_TYPEDEFS (TextBlockNode)
DGNPLATFORM_TYPEDEFS (WhiteSpace)
DGNPLATFORM_TYPEDEFS (TextField);

//__PUBLISH_SECTION_END__

DGNPLATFORM_TYPEDEFS (AlongTextDependency)
DGNPLATFORM_TYPEDEFS (CharacterArray)
DGNPLATFORM_TYPEDEFS (CharStreamArray)
DGNPLATFORM_TYPEDEFS (EdfData)
DGNPLATFORM_TYPEDEFS (EdfDataArray)
DGNPLATFORM_TYPEDEFS (IDwgContext)
DGNPLATFORM_TYPEDEFS (ITextListener)
DGNPLATFORM_TYPEDEFS (Line)
DGNPLATFORM_TYPEDEFS (LineIterator)
DGNPLATFORM_TYPEDEFS (LineRange)
DGNPLATFORM_TYPEDEFS (ParagraphArray)
DGNPLATFORM_TYPEDEFS (ProcessContext)
DGNPLATFORM_TYPEDEFS (RunArray)
DGNPLATFORM_TYPEDEFS (RunPropertiesStyleOverrides)
DGNPLATFORM_TYPEDEFS (SpanSet)
DGNPLATFORM_TYPEDEFS (TextFieldData)
DGNPLATFORM_TYPEDEFS (TextParamAndScale)
DGNPLATFORM_TYPEDEFS (TextBlockCompareOptions)
DGNPLATFORM_TYPEDEFS (TextBlockDrawOptions)
DGNPLATFORM_TYPEDEFS (TextBlockNodeArray)
DGNPLATFORM_TYPEDEFS (TextBlockToStringOptions)

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//! Smart pointer wrapper for TextBlock.
typedef RefCountedPtr<TextBlock> TextBlockPtr;

//! Smart pointer wrapper for TextBlockProperties.
typedef RefCountedPtr<TextBlockProperties> TextBlockPropertiesPtr;

//! Smart pointer wrapper for ParagraphProperties.
typedef RefCountedPtr<ParagraphProperties> ParagraphPropertiesPtr;

//! Smart pointer wrapper for RunProperties.
typedef RefCountedPtr<RunProperties> RunPropertiesPtr;

//! Smart pointer wrapper for IndentationData.
typedef RefCountedPtr<IndentationData> IndentationDataPtr;

//! Smart pointer wrapper for Caret.
typedef RefCountedPtr<Caret> CaretPtr;

//! Smart pointer wrapper for TextField.
typedef RefCountedPtr<TextField> TextFieldPtr;

//__PUBLISH_SECTION_END__

typedef RefCountedPtr<TextBlockCompareOptions>  TextBlockCompareOptionsPtr;
typedef RefCountedPtr<TextBlockDrawOptions>     TextBlockDrawOptionsPtr;
typedef RefCountedPtr<TextBlockToStringOptions> TextBlockToStringOptionsPtr;
typedef RefCountedPtr<ITextListener>            ITextListenerPtr;
typedef RefCountedPtr<TextFieldData>            TextFieldDataPtr;

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Describes how a fraction is aligned to neighboring runs.
//! Since fractions are typically taller than surrounding runs, this describes how smaller runs are arranged around the fraction. Although it's technically how other runs align to the fraction, this property is on the fraction itself.
// @bsiclass                                                    Venkat.Kalyan   07/07
//=======================================================================================
enum class StackedFractionAlignment
    {
    Bottom              = 0,    //!< Surrounding text's baseline matches the baseline of the denominator
    Middle              = 1,    //!< Surrounding text is centered with the fraction (based on its cell box)
    Top                 = 2,    //!< Surrounding text's cell box top matches the top of the fraction numerator's cell box
    None                = 3     //!< DO NOT USE; it is used an internal unitialized state; you could use it for similar, but don't pass around expecting any predictable to happen

    }; // StackedFractionAlignment

//=======================================================================================
//! If an EDF (Enter Data Field) does not have its entire length used, this describes how to align the text within the EDF.
//! For compliance with old data, you should typically check for center and right, and assume any other value is effectively left.
// @bsiclass                                                    Venkat.Kalyan   07/07
//=======================================================================================
enum class EdfJustification : byte
    {
    Left                = 0xff, //!< The unoccupied length is padded to the right with spaces
    Center              = 0x00, //!< The unoccupied length is padded to the left and right with spaces (with excess on the right if required)
    Right               = 0x01  //!< The unoccupied length is padded to the left with spaces

    };

//=======================================================================================
//! Describes the possible results from generating an element from a TextBlock. @see TextHandlerBase::CreateElement.
// @bsiclass                                                    Jeff.Marker     06/2009
//=======================================================================================
enum TextBlockToElementResult
    {
    TEXTBLOCK_TO_ELEMENT_RESULT_Success,    //!< The operation completed successfully.
    TEXTBLOCK_TO_ELEMENT_RESULT_Error,      //!< An unknown failure occured; no element was created.
    TEXTBLOCK_TO_ELEMENT_RESULT_Empty       //!< An element was not created because an empty TextBlock was provided (and it did not have the force text node flag).
    
    }; // TextBlockToElementResult

//__PUBLISH_SECTION_END__

typedef UInt16 CharCode;

static const UInt16 FIELD_XAttrId = 0;

//=======================================================================================
//! Line alignment (inter-line vertical justification) types for MicroStation text elements. This affects how mixed-height text is placed within a line compared to other text.
// @bsiclass                                                    BentleySystems   07/2007
//=======================================================================================
enum LineAlignment
    {
    LINE_ALIGNMENT_Bottom   = 0,    //!< Aligns the baselines
    LINE_ALIGNMENT_Center   = 1,    //!< Aligns the centers of the cell boxes
    LINE_ALIGNMENT_Top      = 2     //!< Aligns the tops of the cell boxes

    }; // LineAlignment

//=======================================================================================
//! Describes the horizontal justifications of TextElementJustification.
//! @note Margin justification is deprecated and support will be removed at some point; use word wrap length instead to achieve the same effect
// @bsiclass                                                    Venkat.Kalyan   10/05
//=======================================================================================
enum HorizontalJustification
    {
    HORIZONTAL_JUSTIFICATION_Left           = 0,    //!< Left
    HORIZONTAL_JUSTIFICATION_Center         = 1,    //!< Center
    HORIZONTAL_JUSTIFICATION_Right          = 2,    //!< Right
    HORIZONTAL_JUSTIFICATION_LeftMargin     = 3,    //!< (deprecated) Left margin
    HORIZONTAL_JUSTIFICATION_RightMargin    = 4     //!< (deprecated) Right margin

    }; // HorizontalJustification

//=======================================================================================
//! Describes the vertical justifications of TextElementJustification.
//! @note Cap justification is no longer supported, and will act like its non-cap equivalent; kept so old data can be interpretted
// @bsiclass                                                    Venkat.Kalyan   10/05
//=======================================================================================
enum VerticalJustification
    {
    VERTICAL_JUSTIFICATION_Top          = 0,    //!< Top
    VERTICAL_JUSTIFICATION_Middle       = 1,    //!< Middle
    VERTICAL_JUSTIFICATION_Baseline     = 2,    //!< Bottom (baseline)
    VERTICAL_JUSTIFICATION_Descender    = 3,    //!< Descender
    VERTICAL_JUSTIFICATION_Cap          = 4     //!< (not supported) Cap

    }; // VerticalJustification

//=======================================================================================
//! Describes the different origins that text can have.
// @bsiclass                                                    Venkat.Kalyan   10/05
//=======================================================================================
enum OriginType
    {
    ORIGIN_TYPE_TextBlock   = 0,    //!< Left-top
    ORIGIN_TYPE_Element     = 1,    //!< Left-bottom
    ORIGIN_TYPE_User        = 2,    //!< Justification occurs around this point; it is not fixed
    ORIGIN_TYPE_AutoCAD     = 3     //!< Left-top for horizontal text, left-center for vertical text

    }; // OriginType

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   10/07
//=======================================================================================
enum DescriptorSide
    {
    DESCRIPTOR_SIDE_Below   = 0,
    DESCRIPTOR_SIDE_Above   = 1,
    DESCRIPTOR_SIDE_On      = 2

    }; // DescriptorSide

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan
//=======================================================================================
enum ProcessLevel
    {
    PROCESS_LEVEL_TextBlock = 0,
    PROCESS_LEVEL_Paragraph = 1,
    PROCESS_LEVEL_Line      = 2,
    PROCESS_LEVEL_Run       = 3,
    PROCESS_LEVEL_Character = 4

    }; // ProcessLevel

//=======================================================================================
// @bsiclass
//=======================================================================================
enum AppendStatus
    {
    APPEND_STATUS_Appended                  = 1 << 0,
    APPEND_STATUS_RunsNotMergeable          = 1 << 1,
    APPEND_STATUS_ParagraphBreak            = 1 << 2,
    APPEND_STATUS_LineBreak                 = 1 << 3,
    APPEND_STATUS_Overflow                  = 1 << 4,

    APPEND_STATUS_Appended_ParagraphBreak   = APPEND_STATUS_ParagraphBreak  | APPEND_STATUS_Appended,
    APPEND_STATUS_Appended_Linefeed         = APPEND_STATUS_LineBreak       | APPEND_STATUS_Appended,
    APPEND_STATUS_Appended_Overflow         = APPEND_STATUS_Appended        | APPEND_STATUS_Overflow

    }; // AppendStatus

//=======================================================================================
// @bsiclass
//=======================================================================================
enum TextBlockNodeLevel
    {
    TEXTBLOCK_NODE_LEVEL_TextBlock  = 0,
    TEXTBLOCK_NODE_LEVEL_Paragraph  = 1,
    TEXTBLOCK_NODE_LEVEL_Line       = 2,
    TEXTBLOCK_NODE_LEVEL_Run        = 3,
    TEXTBLOCK_NODE_LEVEL_Character  = 4

    }; // TextBlockNodeLevel

//=======================================================================================
//! Identifies the underlying kind of text element that a TextBlock represents.
//! These describe the different flavors of text elements; each kind has unique rules for
//! layout and measurement. These differences are hard to quantify and explain as properties,
//! so we simply encapsulate them in this type enumeration.
// @bsiclass                                                    Venkat.Kalyan
//=======================================================================================
enum TextBlockType
    {
    TEXTBLOCK_TYPE_Dgn      = 0,    //!< DGN (text element or text node element)
    TEXTBLOCK_TYPE_DwgMText = 1,    //!< DWG MText
    TEXTBLOCK_TYPE_DwgDText = 2     //!< DWG DText

    }; // TextBlockType

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     10/2010
//=======================================================================================
enum ChangeCaseOperation
    {
    CHANGE_CASE_OPERATION_AllUpper,
    CHANGE_CASE_OPERATION_AllLower,
    CHANGE_CASE_OPERATION_Title,
    CHANGE_CASE_OPERATION_FirstCapital

    }; // ChangeCaseOperation

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/08
//=======================================================================================
template<class T>
struct VectorOfTextBlockNodes : bvector<T>
    {
    private:    typedef bvector<T> Base;

    public:             VectorOfTextBlockNodes  () : Base ()                                                    { }
    public:             VectorOfTextBlockNodes  (typename Base::iterator begin, typename Base::iterator end) : Base (begin, end)  { }
    public:     void    pop_front               ()                                                              { this->erase (this->begin ()); }
    public:     void    push_front              (T const & val)                                                 { this->insert (this->begin (), val); }

    }; // VectorOfUnits

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/07
//=======================================================================================
struct TextBlockNodeArray   : public VectorOfTextBlockNodes<TextBlockNodeP> { };
struct ParagraphArray       : public VectorOfTextBlockNodes<ParagraphP>     { };
struct LineArray            : public VectorOfTextBlockNodes<LineP>          { };

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/07
//=======================================================================================
struct RunArray : public VectorOfTextBlockNodes<RunP>
    {
    private:    typedef VectorOfTextBlockNodes<RunP> Base;

    public:     RunArray    () : Base ()                                                    { }
    public:     RunArray    (Base::iterator begin, Base::iterator end) : Base (begin, end)  { }

    }; // RunArray

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     11/08
//=======================================================================================
struct CharStreamArray : bvector<CharStreamP> { };

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     10/2009
//=======================================================================================
struct TextBlockCompareOptions : public RefCountedBase, public NonCopyableClass
    {
    private:    bool    m_shouldIgnoreInternalState;
    private:    bool    m_shouldIgnoreCachedValues;
    private:    bool    m_shouldIgnoreElementOverhead;
    private:    double  m_tolerance;

    public:     DGNPLATFORM_EXPORT          bool                        ShouldIgnoreInternalState           ()  const;
    public:     DGNPLATFORM_EXPORT          void                        SetShouldIgnoreInternalState        (bool val);
    public:     DGNPLATFORM_EXPORT          bool                        ShouldIgnoreCachedValues            () const;
    public:     DGNPLATFORM_EXPORT          void                        SetShouldIgnoreCachedValues         (bool val);
    public:     DGNPLATFORM_EXPORT          bool                        ShouldIgnoreElementOverhead         () const;
    public:     DGNPLATFORM_EXPORT          void                        SetShouldIgnoreElementOverhead      (bool val);
    public:     DGNPLATFORM_EXPORT          double                      GetTolerance                        () const;
    public:     DGNPLATFORM_EXPORT          void                        SetTolerance                        (double val);

    private:                                                            TextBlockCompareOptions             (bool shouldIgnoreInternalState, bool shouldIgnoreCachedValues, bool shouldIgnoreElementOverhead, double tolerance);

    public:     DGNPLATFORM_EXPORT  static  TextBlockCompareOptionsPtr  CreateForCompareContentAndLocation  ();
    public:     DGNPLATFORM_EXPORT          bool                        AreDoublesEqual                     (double lhs, double rhs) const;

    }; // TextBlockCompareOptions

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2010
//=======================================================================================
struct TextBlockDrawOptions : public RefCountedBase, public NonCopyableClass
    {
    private:    RunCP   m_highlightRun;
    private:    bool    m_shouldDrawWhiteSpace;

    public:     DGNPLATFORM_EXPORT          RunCP                   GetHighlightRunCP       () const;
    public:     DGNPLATFORM_EXPORT          void                    SetHighlightRun         (RunCP value);
    public:     DGNPLATFORM_EXPORT          bool                    ShouldDrawWhiteSpace    () const;
    public:     DGNPLATFORM_EXPORT          void                    SetShouldDrawWhiteSpace (bool value);

    private:                                                        TextBlockDrawOptions    ();

    public:     DGNPLATFORM_EXPORT  static  TextBlockDrawOptionsPtr CreateDefault           ();
    public:     DGNPLATFORM_EXPORT          TextBlockDrawOptionsPtr Clone                   () const;

    }; // TextBlockDrawOptions

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     08/2010
//=======================================================================================
struct TextBlockToStringOptions : public RefCountedBase, public NonCopyableClass
    {
    private:    bool    m_shouldSubstitueAtomicRunContent;
    private:    WChar m_atomicRunContentSubstituteChar;
    private:    bool    m_shouldExpandRscFractions;

    public:     DGNPLATFORM_EXPORT          bool                        ShouldSubstitueAtomicRunContent         () const;
    public:     DGNPLATFORM_EXPORT          void                        ClearShouldSubstitueAtomicRunContent    ();
    public:     DGNPLATFORM_EXPORT          void                        SetAtomicRunContentSubstituteChar       (WChar value);
    public:     DGNPLATFORM_EXPORT          WChar                     GetAtomicRunContentSubstituteChar       () const;
    public:     DGNPLATFORM_EXPORT          bool                        ShouldExpandRscFractions                () const;
    public:     DGNPLATFORM_EXPORT          void                        SetShouldExpandRscFractions             (bool);

    private:                                                            TextBlockToStringOptions                ();

    public:     DGNPLATFORM_EXPORT  static  TextBlockToStringOptionsPtr CreateDefault                           ();
    public:     DGNPLATFORM_EXPORT          TextBlockToStringOptionsPtr Clone                                   () const;

    }; // TextBlockToStringOptions

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     03/2011
//=======================================================================================
struct DPadding
    {
    public: double  m_left;
    public: double  m_top;
    public: double  m_right;
    public: double  m_bottom;

    public: DPadding () : m_left (0.0), m_top (0.0), m_right (0.0), m_bottom (0.0) { }
    public: DPadding (double left, double top, double right, double bottom) : m_left (left), m_top (top), m_right (right), m_bottom (bottom) { }

    }; // DPadding

//__PUBLISH_SECTION_START__

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
