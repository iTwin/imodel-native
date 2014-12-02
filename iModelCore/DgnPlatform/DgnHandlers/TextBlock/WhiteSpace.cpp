/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/WhiteSpace.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- WhiteSpace ----------------------------------------------------------------------------------------------------------------------- WhiteSpace --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
WhiteSpace::WhiteSpace (TextParamWideCR textParams, DPoint2dCR scale, DgnModelR dgnCache) :
    Run (textParams, scale, dgnCache),
    m_maxDisplacementBelowOrigin (0.0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
WhiteSpace::WhiteSpace (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags)
    : Run (runProperties, layoutFlags),
    m_maxDisplacementBelowOrigin (0.0)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
size_t  WhiteSpace::_GetCharacterCount      () const                                                { return 1; }
size_t  WhiteSpace::_GetNextWordBreakIndex  (size_t offset) const                                   { return (0 == offset) ? 0 : 1; }
bool    WhiteSpace::_CanFit                 (LineR line, ProcessContextCR processContext)           { return true; }
void    WhiteSpace::_ComputeRange           ()                                                      { }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
double WhiteSpace::_GetMaxDisplacementBelowOrigin () const
    {
    this->Preprocess ();
    return m_maxDisplacementBelowOrigin * m_properties.GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void WhiteSpace::_ComputeOffsetToChar (DVec3dR offset, double& scale, CaretCR caret) const
    {
    offset.Zero ();
    
    if (caret.GetCharacterIndex () > 0)
        offset.x = this->GetNominalWidth ();
    
    offset.x += m_properties.GetRunOffset ().x;
    offset.y += m_properties.GetRunOffset ().y;

    scale = this->GetProperties ().GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
void WhiteSpace::_Splice (RunP& firstRun, RunP& secondRun, size_t index)
    {
    BeAssert (index <= 1);

    if (0 == index)
        {
        firstRun    = NULL;
        secondRun   = this;
        
        return;
        }

    firstRun    = this;
    secondRun   = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
void WhiteSpace::_Preprocess () const
    {
    m_maxDisplacementBelowOrigin = m_properties.ResolveFont ().GetDescenderRatio ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool WhiteSpace::_Equals (RunCR rhsRun, TextBlockCompareOptionsCR compareOptions) const
    {
    WhiteSpaceCP rhs = dynamic_cast<WhiteSpaceCP>(&rhsRun);
    if (NULL == rhs)
        return false;
    
    if (!T_Super::_Equals (rhsRun, compareOptions))
        return false;
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (!compareOptions.AreDoublesEqual (m_maxDisplacementBelowOrigin, rhs->m_maxDisplacementBelowOrigin)) return false;
        }
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void WhiteSpace::_GetElementRange (DRange3dR value) const
    {
    value = this->GetNominalRange ();
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- ParagraphBreak --------------------------------------------------------------------------------------------------------------- ParagraphBreak --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
ParagraphBreak::ParagraphBreak (TextParamWideCR textParams, DPoint2dCR fontSize, DgnModelR dgnCache) :
    WhiteSpace (textParams, fontSize, dgnCache)
    {
    m_origin.zero ();
    m_orientation.initIdentity ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
ParagraphBreak::ParagraphBreak (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    WhiteSpace (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DRange3d        ParagraphBreak::_GetExactRange          () const                                { return this->GetNominalRange (); }
WString         ParagraphBreak::_ToString               (size_t offset,
                                                            size_t length,
                                                            TextBlockToStringOptionsCR) const   { return WString (L"\r").substr (offset, length); }
WChar           ParagraphBreak::_GetCharacter           (size_t index) const                    { return (0 == index) ? L'\r' : 0; }
AppendStatus    ParagraphBreak::_AppendNextRunToLine    (LineR, RunP&, ProcessContextCR)        { return APPEND_STATUS_RunsNotMergeable; }
RunP            ParagraphBreak::_Clone                  () const                                { return new ParagraphBreak (*this); }
bool            ParagraphBreak::_IsContentRun           () const                                { return false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
DRange3d ParagraphBreak::_GetNominalRange () const
    {
    DRange3d nominalRange;
    nominalRange.low.zero ();
    nominalRange.high.zero ();

    if (this->IsVertical ())
        nominalRange.high.x = this->GetProperties ().GetFontSize().x;
    else
        nominalRange.high.y = this->GetProperties ().GetFontSize().y;

    return nominalRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
double ParagraphBreak::_GetMaxDisplacementAboveOrigin () const
    {
    if (this->IsVertical ())
        return m_properties.GetFontSize ().x;
    
    return m_properties.GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus ParagraphBreak::_AppendToLine (LineR line, RunP& nextRun, ProcessContextCR processContext)
    {
    line.AddRun (this);
    return APPEND_STATUS_Appended_ParagraphBreak;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void ParagraphBreak::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    if (!options.ShouldDrawWhiteSpace ())
        return;
    
    DgnFontCP arialFont = DgnFontManager::FindFont ("Arial", DgnFontType::TrueType, NULL);
    if (NULL == arialFont)
        return;

    Transform tform;
    tform.InitFrom (m_orientation, this->GetLineOffsetAdjustedOrigin ());
    
    context.PushTransform (tform);
        {
        TextStringProperties tsProps (m_properties, m_layoutFlags, false, isViewIndependent, true, TextElementJustification::LeftTop, StackedFractionType::None, StackedFractionSection::None);
        
        tsProps.SetFont (*arialFont);
        
        WChar           glyphStr[]  = { 0x00b6, 0x0000 };
        TextStringPtr   glyph       = TextString::Create (glyphStr, NULL, NULL, tsProps);
        
        context.DrawTextString (*glyph);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void ParagraphBreak::_GetRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void ParagraphBreak::_GetTransformedRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- LineBreak --------------------------------------------------------------------------------------------------------------------------- LineBreak --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
LineBreak::LineBreak (TextParamWideCR textParams, DPoint2dCR fontSize, DgnModelR dgnCache) :
    WhiteSpace (textParams, fontSize, dgnCache)
    {
    m_origin.zero ();
    m_orientation.initIdentity ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
LineBreak::LineBreak (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags)
    : WhiteSpace (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DRange3d        LineBreak::_GetExactRange       () const                                { return GetNominalRange (); }
WString         LineBreak::_ToString            (size_t offset,
                                                    size_t length,
                                                    TextBlockToStringOptionsCR) const   { return WString (L"\n").substr (offset, length); }
WChar         LineBreak::_GetCharacter        (size_t index)  const                   { return (0 == index) ? L'\n' : 0; }
AppendStatus    LineBreak::_AppendNextRunToLine (LineR, RunP&, ProcessContextCR)        { return APPEND_STATUS_RunsNotMergeable; }
RunP            LineBreak::_Clone               () const                                { return new LineBreak (*this); }
bool            LineBreak::_IsContentRun        () const                                { return false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
DRange3d LineBreak::_GetNominalRange () const
    {
    DRange3d nominalRange;
    nominalRange.low.zero ();
    nominalRange.high.zero ();

    if (this->IsVertical ())
        nominalRange.high.x = this->GetProperties ().GetFontSize().x;
    else
        nominalRange.high.y = this->GetProperties ().GetFontSize().y;

    return nominalRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
double LineBreak::_GetMaxDisplacementAboveOrigin () const
    {
    if (this->IsVertical ())
        return m_properties.GetFontSize ().x;
    
    return m_properties.GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus LineBreak::_AppendToLine (LineR line, RunP& nextRun, ProcessContextCR processContext)
    {
    line.AddRun (this);
    return APPEND_STATUS_Appended_Linefeed;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void LineBreak::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    if (!options.ShouldDrawWhiteSpace ())
        return;
    
    DgnFontCP symbolFont = DgnFontManager::FindFont ("Symbol", DgnFontType::TrueType, NULL);
    if (NULL == symbolFont)
        return;
    
    Transform tform;
    tform.InitFrom (m_orientation, this->GetLineOffsetAdjustedOrigin ());
    
    context.PushTransform (tform);
        {
        TextStringProperties tsProps (m_properties, m_layoutFlags, false, isViewIndependent, true, TextElementJustification::LeftTop, StackedFractionType::None, StackedFractionSection::None);
        
        tsProps.SetFont (*symbolFont);
        
        WChar           glyphStr[]  = { 0x00bf, 0x0000 };
        TextStringPtr   glyph       = TextString::Create (glyphStr, NULL, NULL, tsProps);
        
        context.DrawTextString (*glyph);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void LineBreak::_GetRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void LineBreak::_GetTransformedRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Tab ------------------------------------------------------------------------------------------------------------------------------------- Tab --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Tab::Tab (TextParamWideCR textParams, DPoint2dCR fontSize, DgnModelR dgnCache)
    : WhiteSpace (textParams, fontSize, dgnCache)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Tab::Tab (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags)
    : WhiteSpace (runProperties, layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
DRange3d        Tab::_GetExactRange         () const                                { return this->GetNominalRange (); }
WString         Tab::_ToString              (size_t offset,
                                                size_t length,
                                                TextBlockToStringOptionsCR) const   { return WString (L"\t").substr (offset, length); }
WChar         Tab::_GetCharacter          (size_t index) const                    { return (0 == index) ? L'\t' : 0; }
AppendStatus    Tab::_AppendNextRunToLine   (LineR, RunP&, ProcessContextCR)        { return APPEND_STATUS_RunsNotMergeable; }
RunP            Tab::_Clone                 () const                                { return new Tab (*this); }

DPoint2dCR      Tab::GetTabExtents          () const                                { return m_tabExtents; }
void            Tab::SetTabExtents          (DPoint2dCR value)                      { m_tabExtents = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
DRange3d Tab::_GetNominalRange () const
    {
    DRange3d    nominalRange;
    DPoint2d    tabExtents      = this->GetTabExtents ();

    nominalRange.low.zero ();
    nominalRange.high.zero ();
    
    if (IsVertical())
        {
        nominalRange.low.y  = tabExtents.y;
        nominalRange.high.x = m_properties.GetFontSize ().x;
        }
    else
        {
        nominalRange.high.y = m_properties.GetFontSize ().y;
        nominalRange.high.x = tabExtents.x;
        }

    return nominalRange;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/05
//---------------------------------------------------------------------------------------
double Tab::_GetNominalHeight () const
    {
    DRange3d range = this->GetNominalRange ();
    if (range.isNull ())
        return 0.0;
    
    // We allow negative? That's how it differed from base.
    return (range.high.y - range.low.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus Tab::_AppendToLine (LineR line, RunP& nextRun, ProcessContextCR processContext)
    {
    if (!this->SetTabWidth (processContext))
        return APPEND_STATUS_Overflow;

    line.AddRun (this);
    
    return APPEND_STATUS_Appended;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
bool Tab::_CanFit (LineR line, ProcessContextCR processContext)
    {
    if (0.0 == processContext.GetTextBlock ()->GetProperties ().GetDocumentWidth ())
        return true;
    
    DPoint3d lastRunOrigin;
    this->GetLastRunOrigin (lastRunOrigin, line, processContext);
    
    double nextTabStop = this->ComputeNextTabStop (line, processContext, lastRunOrigin);
    
    return (nextTabStop <= processContext.GetTextBlock ()->GetProperties ().GetDocumentWidth ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void Tab::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    if (!options.ShouldDrawWhiteSpace ())
        return;
    
    DgnFontCP arialFont = DgnFontManager::FindFont ("Arial", DgnFontType::TrueType, NULL);
    if (NULL == arialFont)
        return;

    Transform tform;
    tform.InitFrom (m_orientation, this->GetLineOffsetAdjustedOrigin ());
    
    context.PushTransform (tform);
        {
        TextStringProperties    tsProps         (m_properties, m_layoutFlags, false, isViewIndependent, true, TextElementJustification::LeftTop, StackedFractionType::None, StackedFractionSection::None);
        DPoint2d                scaledFontSize  = m_properties.GetFontSize ();
        
        scaledFontSize.x = (m_tabExtents.x * 0.7);
        
        tsProps.SetFont (*arialFont);
        tsProps.SetFontSize (scaledFontSize);
        
        WChar           glyphStr[]  = { 0x2192, 0x0000 };
        TextStringPtr   glyph       = TextString::Create (glyphStr, NULL, NULL, tsProps);
        
        context.DrawTextString (*glyph);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
bool Tab::SetTabWidth (ProcessContextCR processContext)
    {
    DPoint3d origin;
    this->GetLastRunOrigin (origin, *processContext.GetLine (), processContext);

    double nextTabStop = this->ComputeNextTabStop (*processContext.GetLine (), processContext, origin);
    
    if (0 != processContext.GetTextBlock ()->GetProperties ().GetDocumentWidth ()
            && processContext.GetLine ()->GetRunCount () > 0
            && (nextTabStop > processContext.GetTextBlock ()->GetProperties ().GetDocumentWidth ()))
        {
        return false;
        }
    
    DPoint2d    tabExtents;
    double      indent                  = processContext.GetParagraph ()->GetIndentForLine (processContext.GetLine ());
    double      leftAlignEdgeDistance   = (TEXTBLOCK_TYPE_DwgMText == processContext.GetTextBlock ()->GetType ())
                                                ? processContext.GetLine ()->ComputeLeftEdgeAlignDistance ()
                                                : 0.0;
    
    if (processContext.GetTextBlock ()->GetProperties ().IsVertical ())
        {
        tabExtents.x = 0.0;
        tabExtents.y = -(nextTabStop + origin.y - indent);
        }
    else
        {
        tabExtents.x = nextTabStop - origin.x - indent + leftAlignEdgeDistance;
        tabExtents.y = 0.0;
        }
    
    this->SetTabExtents (tabExtents);
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Tab::GetLastRunOrigin (DPoint3dR runOrigin, LineCR line, ProcessContextCR processContext) const
    {
    if (0 == line.GetRunCount ())
        {
        runOrigin.zero ();
        return;
        }

    RunCP lastRun = line.GetRun (line.GetRunCount () - 1);
    runOrigin = lastRun->GetOrigin ();

    DVec3d displacement = lastRun->GetRunSpacing ();
    
    runOrigin.add (&displacement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
double Tab::ComputeNextTabStop (LineCR line, ProcessContextCR processContext, DPoint3dCR origin) const
    {
    TextBlockCR textBlock   = *processContext.GetTextBlock ();
    ParagraphCR paragraph   = *processContext.GetParagraph ();

    // AutoCAD does not honor leading tabs when the word-wrap length is 0.
    if (textBlock.IsMTextType () && 0.0 == textBlock.GetProperties ().GetDocumentWidth () && 0.0 == origin.x)
        return 0.0;
    
    RunPropertiesCP firstStyle      = textBlock.GetFirstRunProperties ();
    double          referenceHeight = (NULL != firstStyle && !textBlock.IsMTextType ()) ? firstStyle->GetFontSize().y : textBlock.GetNodeOrFirstRunHeight ();

    // A zero reference height means we're at the beginning of the text block.
    //  The only sensible thing I can think to use is this tab's height.
    if (0.0 == referenceHeight)
        referenceHeight = this->GetProperties ().GetFontSize ().y;

    double  leftAlignEdgeDistance   = textBlock.IsMTextType () ? line.ComputeLeftEdgeAlignDistance () : 0.0;
    bool    vertical                = textBlock.GetProperties ().IsVertical ();
    double  indent                  = paragraph.GetIndentForLine (&line);
    double  offset                  = vertical ? (-origin.y + indent) : (origin.x + indent - leftAlignEdgeDistance);

    return paragraph.GetProperties().GetIndentation ().GetNextTabStop (offset, referenceHeight, textBlock.GetProperties ().GetDocumentWidth ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool Tab::_Equals (RunCR rhsRun, TextBlockCompareOptionsCR compareOptions) const
    {
    TabCP rhs = dynamic_cast<TabCP>(&rhsRun);
    if (NULL == rhs)
        return false;
    
    if (!T_Super::_Equals (rhsRun, compareOptions))
        return false;
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (!m_tabExtents.IsEqual (rhs->m_tabExtents, compareOptions.GetTolerance ())) return false;
        }
    
    return true;
    }
