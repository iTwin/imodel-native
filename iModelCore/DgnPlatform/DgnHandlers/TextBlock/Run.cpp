/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Run.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
double                  Run::GetMaxDisplacementAboveOrigin          () const                                                        { return _GetMaxDisplacementAboveOrigin (); }
double                  Run::GetMaxDisplacementBelowOrigin          () const                                                        { return _GetMaxDisplacementBelowOrigin (); }
double                  Run::GetMaxExactHeightAboveOrigin           () const                                                        { return _GetMaxExactHeightAboveOrigin (); }
double                  Run::GetMaxExactDepthBelowOrigin            () const                                                        { return _GetMaxExactDepthBelowOrigin (); }
WString                 Run::ToString                               () const                                                        { return _ToString (0, WString::npos, *TextBlockToStringOptions::CreateDefault ()); }
WString                 Run::ToString                               (size_t offset,
                                                                        size_t length,
                                                                        TextBlockToStringOptionsCR options) const                   { return _ToString (offset, length, options); }
size_t                  Run::GetCharacterCount                      () const                                                        { return _GetCharacterCount (); }
WChar                   Run::GetCharacter                           (size_t index) const                                            { return _GetCharacter (index); }
void                    Run::Splice                                 (RunP& firstRun, RunP& secondRun, size_t index)                 { _Splice (firstRun, secondRun, index); }
size_t                  Run::GetNextWordBreakIndex                  (size_t offset) const                                           { return _GetNextWordBreakIndex (offset); }
AppendStatus            Run::AppendNextRunToLine                    (LineR line, RunP& nextSplitRun, ProcessContextCR context)      { return _AppendNextRunToLine (line, nextSplitRun, context); }
AppendStatus            Run::AppendToLine                           (LineR line, RunP& nextRun, ProcessContextCR context)           { return _AppendToLine (line, nextRun, context); }
bool                    Run::CanFit                                 (LineR line, ProcessContextCR context)                          { return _CanFit (line, context); }
void                    Run::ComputeRange                           ()                                                              { _ComputeRange (); }
void                    Run::Preprocess                             () const                                                        { _Preprocess (); }
RunP                    Run::Clone                                  () const                                                        { return _Clone (); }
void                    Run::GetRangesForSelection                  (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const { _GetRangesForSelection (ranges, fromIdx, toIdx); }
void                    Run::GetTransformedRangesForSelection       (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const { _GetTransformedRangesForSelection (ranges, fromIdx, toIdx); }
bool                    Run::IsLastCharSpace                        () const                                                        { return _IsLastCharSpace (); }
double                  Run::ComputeLeftEdgeAlignDistance           () const                                                        { return _ComputeLeftEdgeAlignDistance (); }
void                    Run::Draw                                   (ViewContextR context,
                                                                        bool isViewIndependent,
                                                                        TextBlockDrawOptionsCR options) const                       { _Draw (context, isViewIndependent, options); }
bool                    Run::AllowTrailingWordBreak                 () const                                                        { return _AllowTrailingWordBreak (); }
void                    Run::SetProperties                          (RunPropertiesCR value)                                         { _SetProperties (value); }
bool                    Run::IsLastRunInLine                        () const                                                        { return _IsLastRunInLine (); }
void                    Run::SetIsLastRunInLine                     (bool value)                                                    { _SetIsLastRunInLine (value); }
bool                    Run::ContainsOnlyWhitespace                 () const                                                        { return _ContainsOnlyWhitespace (); }
DgnGlyphRunLayoutFlags  Run::GetRunLayoutFlags                      () const                                                        { return m_layoutFlags; }
void                    Run::SetRunLayoutFlags                      (DgnGlyphRunLayoutFlags value)                                  { _SetRunLayoutFlags (value); }
void                    Run::_SetRunLayoutFlags                     (DgnGlyphRunLayoutFlags value)                                  { m_layoutFlags = value; }
bool                    Run::IsVertical                             () const                                                        { return (GLYPH_RUN_LAYOUT_FLAG_Vertical == (GLYPH_RUN_LAYOUT_FLAG_Vertical & m_layoutFlags)); }
void                    Run::SetIsVertical                          (bool value)                                                    { this->_SetRunLayoutFlags (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_Vertical) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_Vertical)); }
bool                    Run::IsBackwards                            () const                                                        { return (GLYPH_RUN_LAYOUT_FLAG_Backwards == (GLYPH_RUN_LAYOUT_FLAG_Backwards & m_layoutFlags)); }
void                    Run::SetIsBackwards                         (bool value)                                                    { this->_SetRunLayoutFlags (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_Backwards) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_Backwards)); }
bool                    Run::IsUpsideDown                           () const                                                        { return (GLYPH_RUN_LAYOUT_FLAG_UpsideDown == (GLYPH_RUN_LAYOUT_FLAG_UpsideDown & m_layoutFlags)); }
void                    Run::SetIsUpsideDown                        (bool value)                                                    { this->_SetRunLayoutFlags (value ? (DgnGlyphRunLayoutFlags)(m_layoutFlags | GLYPH_RUN_LAYOUT_FLAG_UpsideDown) : (DgnGlyphRunLayoutFlags)(m_layoutFlags & ~GLYPH_RUN_LAYOUT_FLAG_UpsideDown)); }
bool                    Run::IsForEditing                           () const                                                        { return (GLYPH_RUN_LAYOUT_FLAG_IsForEditing == (GLYPH_RUN_LAYOUT_FLAG_IsForEditing & m_layoutFlags)); }
bool                    Run::Equals                                 (RunCR rhs, TextBlockCompareOptionsCR compareOptions) const     { return _Equals (rhs, compareOptions); }
bool                    Run::IsContentRun                           () const                                                        { return _IsContentRun (); }
bool                    Run::IsAtomic                               () const                                                        { return _IsAtomic (); }
BentleyStatus           Run::ComputeCaretAtLocation                 (CaretR caret, DPoint3dCR locationIn, bool isStrict) const      { return _ComputeCaretAtLocation (caret, locationIn, isStrict); }
void                    Run::ComputeOffsetToChar                    (DVec3dR offset, double& scale, CaretCR caret) const            { _ComputeOffsetToChar (offset, scale, caret); }
void                    Run::ComputeTransformedHitTestRange         (DRange3dR value) const                                         { _ComputeTransformedHitTestRange (value); }
void                    Run::SetOverrideTrailingCharacterOffset     (double value)                                                  { _SetOverrideTrailingCharacterOffset (value); }
void                    Run::ClearTrailingCharacterOffsetOverride   ()                                                              { _ClearTrailingCharacterOffsetOverride (); }
void                    Run::GetElementRange                        (DRange3dR value) const                                         { _GetElementRange (value); }
    
TextBlockNodeLevel      Run::_GetUnitLevel                          () const                                                        { return TEXTBLOCK_NODE_LEVEL_Run; }
void                    Run::SetLineOffset                          (DVec3dCR offset)                                               { m_lineOffset = offset; }
DVec3d                  Run::GetLineOffset                          () const                                                        { return m_lineOffset; }
double                  Run::_GetMaxDisplacementBelowOrigin         () const                                                        { return 0.0; }
double                  Run::_GetMaxExactDepthBelowOrigin           () const                                                        { return 0.0; }
bool                    Run::_IsLastCharSpace                       () const                                                        { return false; }
double                  Run::_ComputeLeftEdgeAlignDistance          () const                                                        { return 0.0; }
bool                    Run::_AllowTrailingWordBreak                () const                                                        { return true; }
RotMatrixCR             Run::GetOrientation                         () const                                                        { return m_orientation; }
void                    Run::SetOrientation                         (RotMatrixCR orientation)                                       { m_orientation = orientation; }
RunPropertiesCR         Run::GetProperties                          () const                                                        { return m_properties; }
void                    Run::_SetProperties                         (RunPropertiesCR value)                                         { m_properties = value; }
bool                    Run::_IsLastRunInLine                       () const                                                        { return m_isLastRunInLine; }
void                    Run::_SetIsLastRunInLine                    (bool val)                                                      { m_isLastRunInLine = val; }
bool                    Run::_ContainsOnlyWhitespace                () const                                                        { return true; }
bool                    Run::_IsContentRun                          () const                                                        { return true; }
bool                    Run::_IsAtomic                              () const                                                        { return false; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
DRange3d Run::_GetNominalRange () const
    {
    this->Preprocess ();
    return T_Super::_GetNominalRange ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
DRange3d Run::_GetExactRange () const
    {
    this->Preprocess ();
    return T_Super::_GetExactRange ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
Transform Run::_GetTransform () const
    {
    DPoint3d origin = GetLineOffsetAdjustedOrigin ();

    Transform transform;
    transform.initFrom (&GetOrientation (), &origin);

    return transform;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/06
//---------------------------------------------------------------------------------------
Run::Run (DgnModelR dgnCache) :
    TextBlockNode (),
    m_properties        (dgnCache),
    m_isLastRunInLine   (false),
    m_layoutFlags       (GLYPH_RUN_LAYOUT_FLAG_None)
    {
    m_lineOffset.zero ();
    m_orientation.initIdentity ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Run::Run (TextParamWideCR textParams, DPoint2dCR textScale, DgnModelR dgnCache) :
    TextBlockNode (),
    m_properties        (textParams, textScale, dgnCache),
    m_isLastRunInLine   (false),
    m_layoutFlags       (TextBlockUtilities::ComputeRunLayoutFlags (textParams, textScale))
    {
    m_lineOffset.zero ();
    m_orientation.initIdentity ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
Run::Run (RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    TextBlockNode (),
    m_properties        (runProperties),
    m_isLastRunInLine   (false),
    m_layoutFlags       (layoutFlags)
    {
    m_lineOffset.zero ();
    m_orientation.initIdentity ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
Run::Run (RunCR rhs) :
    TextBlockNode (rhs),
    m_orientation       (rhs.m_orientation),
    m_lineOffset        (rhs.m_lineOffset),
    m_properties        (rhs.m_properties),
    m_isLastRunInLine   (rhs.m_isLastRunInLine),
    m_layoutFlags       (rhs.m_layoutFlags)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
double Run::_GetMaxDisplacementAboveOrigin () const
    {
    // By default, we get the range.high.y
    DRange3d range = GetNominalRange ();
    
    return range.isNull () ? - m_origin.y : range.high.y - m_origin.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/05
//---------------------------------------------------------------------------------------
double Run::_GetMaxExactHeightAboveOrigin () const
    {
    // By default, we get the range.high.y
    DRange3d range = GetExactRange ();
    
    return range.isNull () ? - m_origin.y : range.high.y - m_origin.y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
DPoint3d Run::GetLineOffsetAdjustedOrigin () const
    {
    DPoint3d origin = GetOrigin ();
    origin.add (&m_lineOffset);
    
    return origin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
DVec3d Run::GetRunSpacing () const
    {
    DVec3d displacement;
    displacement.zero ();

    DRange3d range = this->GetNominalRange ();
    
    if (!range.isNull ())
        {
        if (this->IsVertical ())
            displacement.y = range.low.y;
        else
            displacement.x = range.high.x;
        }
    
    return displacement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Run::ComputeCaretParameters (DPoint3dR location, DVec3dR direction, CaretCR caret) const
    {
    if (caret.GetCharacterIndex () > GetCharacterCount ())
        {
        location.zero ();
        direction.zero ();
        return;
        }

    DVec3d  offset;
    double  scale;
    this->ComputeOffsetToChar (offset, scale, caret);

    location = offset;
    location.Add (this->GetOrigin ());
    
    if (!this->IsVertical ())
        this->GetOrientation ().getColumn (&direction, 1);
    else
        this->GetOrientation ().getColumn (&direction, 0);
    
    direction.Scale (scale);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
BentleyStatus Run::_ComputeCaretAtLocation (CaretR caret, DPoint3dCR locationIn, bool isStrict) const
    {
    Transform transform = this->GetTransform ();
    transform.inverseOf (&transform);

    DPoint3d location = locationIn;
    transform.multiply (&location);

    // From subjective experience, clicking to position the caret is slightly weighted to the left.
    //  That is to say, that to position the caret to the left of a character, you should be able to click on the right edge of the preceding character.
    //  It might be nice to come up with something better than this; CharStream's already override and use individually computed glyph locations.
    if (this->IsVertical ())
        location.y -= m_properties.GetFontSize ().y * 0.2;
    else
        location.x += m_properties.GetFontSize ().y * 0.2;
    
    size_t i = 0;
    for (i = 0; i < this->GetCharacterCount (); ++i)
        {
        bvector<DRange3d> ranges;
        this->GetRangesForSelection (ranges, 0, i + 1);

        bool    containsPoint               = false;
        bool    dontConsiderNextCharacter   = false;
        
        FOR_EACH (DRange3dCR range , ranges)
            {
            containsPoint = (this->IsVertical () ? (location.y > range.low.y) : (location.x < range.high.x));
            
            if (containsPoint && isStrict)
                {
                containsPoint &= (this->IsVertical () ? (location.y < range.high.y) : (location.x > range.low.x));
                dontConsiderNextCharacter = true;
                }

            if (containsPoint || dontConsiderNextCharacter)
                break;
            }
        
        if (containsPoint)
            break;
        
        if (dontConsiderNextCharacter)
            return ERROR;
        }

    caret.SetCharacterIndex (i);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Cell increment is a glyph concept that determines how far to move the pen
// from the left edge of one glyph to the left edge of the following glyph.
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
double Run::_GetMaxHorizontalCellIncrement () const
    {
    return 0.0;
    }

//---------------------------------------------------------------------------------------
// Cell increment is a glyph concept that determines how far to move the pen
// from the left edge of one glyph to the left edge of the following glyph.
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
double Run::GetMaxHorizontalCellIncrement () const
    {
    return _GetMaxHorizontalCellIncrement ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool Run::_Equals (RunCR rhs, TextBlockCompareOptionsCR compareOptions) const
    {
    if (!T_Super::Equals (rhs, compareOptions))
        return false;
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (m_isLastRunInLine   != rhs.m_isLastRunInLine)   return false;
        if (m_layoutFlags       != rhs.m_layoutFlags)       return false;
        }

    if (!m_orientation.IsEqual  (rhs.m_orientation, compareOptions.GetTolerance ()))                                                return false;
    if (!m_lineOffset.IsEqual   (rhs.m_lineOffset,  compareOptions.GetTolerance ()))                                                return false;
    if (!m_properties.Equals    (rhs.m_properties,  compareOptions.GetTolerance (), compareOptions.ShouldIgnoreElementOverhead ())) return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
void Run::_GetRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    if (fromIdx >= toIdx)
        return;
    
    DRange3d    range       = this->_GetNominalRange ();
    DPoint2d    runOffset   = this->GetProperties ().GetRunOffset ();
    
    range.low.x     += runOffset.x;
    range.low.y     += runOffset.y;
    range.high.x    += runOffset.x;
    range.high.y    += runOffset.y;
    
    ranges.push_back (range);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
void Run::_GetTransformedRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    this->_GetRangesForSelection (ranges, fromIdx, toIdx);

    FOR_EACH (DRange3dR range, ranges)
        this->GetTransform ().Multiply (&range.low, &range.low, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Run::_ComputeTransformedHitTestRange (DRange3dR hitTestRange) const
    {
    hitTestRange = this->GetNominalRange ();

    DPoint2dCR runOffset = this->GetProperties ().GetRunOffset ();
    
    if (runOffset.x <= 0.0)
        hitTestRange.low.x += runOffset.x;
    else
        hitTestRange.high.x += runOffset.x;
    
    if (runOffset.y <= 0.0)
        hitTestRange.low.y += runOffset.y;
    else
        hitTestRange.high.y += runOffset.y;

    this->GetTransform ().Multiply (hitTestRange, hitTestRange);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Run::_SetOverrideTrailingCharacterOffset (double)
    {
    // Default implementation does not support this operation.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Run::_ClearTrailingCharacterOffsetOverride ()
    {
    // Default implementation does not support this operation.
    }
