/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Line.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/DgnRscFont.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

typedef bvector<size_t>     sizetVector;
typedef bvector<RunArray*>  WordArray;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Line Utility Functions ----------------------------------------------------------------------------------------------- Line Utility Functions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
static double  computeXOffsetForLeadingRSCSpaces (TextBlockCR textBlock, ParagraphCR paragraph, LineCR line, RunCR run)
    {
    // The goal of this function is make multiple RSC-based runs in a line have equivalent spacing
    //  as if they were a single run. This is unique to RSC because if the RSC font is missing a
    //  space glyph, it should use the width of the preceding character. When splitting a line into
    //  multiple runs, the rendering system will treat each in isolation, thus giving leading spaces
    //  in runs a width of 1.0. We can counteract this as a pre-process by shifting run origins.
    //  This algorithm started in 8.9.4. While it is unfortunate the amount of special casing needed,
    //  we have determined this is generally desirable, and would be a regression if removed. It is
    //  important to aggressively find scenarios where we can't/shouldn't do this, including when
    //  previous runs are of different font type, or are not normal CharStream's.

    // If run being added isn't a CharStream or is not using an RSC font, do nothing.
    CharStreamCP runAsCharStream = dynamic_cast<CharStreamCP>(&run);
    if (NULL == runAsCharStream || DgnFontType::Rsc != run.GetProperties ().GetFont ().GetType ())
        return 0.0;

    // If run being added is (sub|super)script, do nothing.
    if (run.GetProperties ().IsSubScript () || run.GetProperties ().IsSuperScript ())
        return 0.0;

    // If the run being added's font specifies a space character, do nothing.
    DgnFontCR runFont = run.GetProperties ().ResolveFont ();
    if (runFont.ContainsCharacter (0x20))
        return 0.0;

    // Determine the number of leading spaces.
    size_t nSpacesAtStart = 0;
    for (; nSpacesAtStart < run.GetCharacterCount (); nSpacesAtStart++)
        {
        if (!runAsCharStream->IsSpace (nSpacesAtStart))
            break;
        }

    // If no leading spaces, nothing to do.
    if (0 == nSpacesAtStart)
        return 0.0;

    // We need to iterate backwards over the text block to find a preceding character;
    //  since we are in the process of adding a new run to an existing line, we can
    //  simply use the end of the text block (minus 1 character of course).
    Caret caret = textBlock.End ();
    caret.MoveToPreviousCharacter ();

    // If we're at the beginning of the text block, there are no possible previous
    //  characters to base our calculation off of.
    if (caret.IsAt (textBlock.Begin ()))
        return 0.0;
    
    // We only want to operate on one line at a time; store the line and paragraph index.
    size_t  runParagraphIndex   = caret.GetParagraphIndex ();
    size_t  runLineIndex        = caret.GetLineIndex ();
    bool        foundValidChar      = false;

    // Stop at the beginning of the line.
    Caret stopCaret = caret;
    stopCaret.SetParagraphIndex (runParagraphIndex);
    stopCaret.SetLineIndex (runLineIndex);
    stopCaret.SetRunIndex (0);
    stopCaret.SetCharacterIndex (0);

    // Keep moving backwards one character until we find a valid non-space character.
    for (; caret.IsAfter (stopCaret) || caret.IsAt (stopCaret); caret.MoveToPreviousCharacter ())
        {
        // If previous run is not a CharStream or does not use an RSC-based font, do nothing.
        CharStreamCP charStream = dynamic_cast<CharStreamCP> (caret.GetCurrentRunCP ());
        if (NULL == charStream || DgnFontType::Rsc != caret.GetCurrentRunCP ()->GetProperties ().GetFont ().GetType ())
            return 0.0;

        // If previous run is (sub|super)script, do nothing.
        if (charStream->GetProperties ().IsSubScript () || charStream->GetProperties ().IsSuperScript ())
            return 0.0;

        // If we found a non-space, break.
        if (!charStream->IsSpace (caret.GetCharacterIndex ()))
            {
            foundValidChar = true;
            break;
            }

        // If we didn't find a valid character and we're at the beginning, no valid characters left to find.
        if (caret.IsAt (textBlock.Begin ()))
            return 0.0;
        }

    // If we couldn't find a valid previous character to base width off of, do nothing.
    if (!foundValidChar)
        return 0.0;

    // Get the glyph corresponding to the previous non-space character.
    DgnGlyphCP lastNonSpaceGlyph = static_cast<DgnRscFontCR>(caret.GetCurrentRunCP ()->GetProperties ().ResolveFont ()).FindGlyphCP (caret.GetNextCharacter ());
    if (NULL == lastNonSpaceGlyph)
        return 0.0;

    // The rendering system will use 1.0 as the width, but we know what the width should be;
    //  compute the offset and use it for each leading space character.
    double  xScaleFactor    = run.GetProperties ().GetFontSize ().x;
    double  offset          = nSpacesAtStart * ((lastNonSpaceGlyph->GetBlackBoxRight () * xScaleFactor) - xScaleFactor);

    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
static void getWordBreakIndices (sizetVector& wordBreakIndices, RunArray& runArray)
    {
    size_t cumulativeRunIndex = 0;
    
    for (RunArray::iterator iter = runArray.begin (); iter != runArray.end (); ++iter)
        {
        size_t nextWordBreakIndex = 0;
        
        for (;;)
            {
            nextWordBreakIndex = (*iter)->GetNextWordBreakIndex (nextWordBreakIndex);
            
            if (nextWordBreakIndex >= (*iter)->GetCharacterCount ())
                break;

            wordBreakIndices.push_back (nextWordBreakIndex + cumulativeRunIndex);
            nextWordBreakIndex++;
            }

        cumulativeRunIndex += (*iter)->GetCharacterCount ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/07
//---------------------------------------------------------------------------------------
static void getWordArray (WordArray& wordArray, sizetVector const& wordBreakIndices, RunArray& runArray)
    {
    wordArray.push_back (new RunArray ());
    
    size_t cumulativeRunIndex = 0;
    
    for (sizetVector::const_iterator ciiter = wordBreakIndices.begin (); ciiter != wordBreakIndices.end (); ciiter++)
        {
        RunP currentRun = runArray.front ();
        runArray.pop_front ();

        size_t localIndex = *ciiter - cumulativeRunIndex + 1;
        if (localIndex > currentRun->GetCharacterCount ())
            {
            wordArray.back ()->push_back (currentRun);
            cumulativeRunIndex += currentRun->GetCharacterCount ();
            
            continue;
            }

        RunP first = NULL;
        currentRun->Splice (first, currentRun, localIndex);
        
        if (NULL != currentRun)
            {
            DVec3d      displacement    = first->GetRunSpacing ();
            DPoint3d    origin          = first->GetOrigin ();
            
            origin.add (&displacement);
            
            currentRun->SetOrigin (origin);
            }

        cumulativeRunIndex += first->GetCharacterCount ();

        wordArray.back ()->push_back (first);
        wordArray.push_back (new RunArray ());
        
        if (NULL != currentRun)
            runArray.push_front (currentRun);
        }

    if (0 != runArray.size ())
        {
        BeAssert (runArray.size () == 1);
        wordArray.back ()->push_back (runArray.back ());
        }
    else if (0 == wordArray.back ()->size ())
        {
        wordArray.pop_back ();
        }
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Line ----------------------------------------------------------------------------------------------------------------------------------- Line --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
Line::Line () :
    TextBlockNode (),
    m_maxUnitHeight                 (0.0),
    m_maxUnitWidth                  (0.0),
    m_maxAscender                   (0.0),
    m_maxDescender                  (0.0),
    m_lowestUnitY                   (0.0),
    m_maxDistanceAboveBaseline      (0.0),
    m_maxExactDistanceAboveOrigin   (0.0),
    m_maxUnitOffset                 (0.0),
    m_maxExactDepthBelowOrigin      (0.0),
    m_isFullLine                    (false),
    m_maxHorizontalCellIncrement    (0.0)
    {
    m_baselineDisplacement.zero ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
Line::Line (LineCR rhs) :
    TextBlockNode (rhs),
    m_maxUnitHeight                 (rhs.m_maxUnitHeight),
    m_maxUnitWidth                  (rhs.m_maxUnitWidth),
    m_maxAscender                   (rhs.m_maxAscender),
    m_maxDescender                  (rhs.m_maxDescender),
    m_lowestUnitY                   (rhs.m_lowestUnitY),
    m_maxDistanceAboveBaseline      (rhs.m_maxDistanceAboveBaseline),
    m_maxExactDistanceAboveOrigin   (rhs.m_maxExactDistanceAboveOrigin),
    m_maxUnitOffset                 (rhs.m_maxUnitOffset),
    m_maxExactDepthBelowOrigin      (rhs.m_maxExactDepthBelowOrigin),
    m_baselineDisplacement          (rhs.m_baselineDisplacement),
    m_isFullLine                    (rhs.m_isFullLine),
    m_maxHorizontalCellIncrement    (rhs.m_maxHorizontalCellIncrement)
    {
    for (size_t i = 0; i < rhs.m_runArray.size (); ++i)
        m_runArray.push_back (rhs.m_runArray[i]->Clone ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/06
//---------------------------------------------------------------------------------------
Line::~Line ()
    {
    for (size_t i = 0; i < m_runArray.size (); ++i)
        delete m_runArray[i];
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextBlockNodeLevel  Line::_GetUnitLevel                 () const        { return TEXTBLOCK_NODE_LEVEL_Line; }
void                Line::SetFullLine                   (bool value)    { m_isFullLine = value; }
size_t              Line::GetRunCount                   () const        { return m_runArray.size (); }
double              Line::GetMaxUnitWidth               () const        { return m_maxUnitWidth; }
double              Line::GetMaxUnitHeight              () const        { return m_maxUnitHeight; }
double              Line::GetMaxAscender                () const        { return m_maxAscender; }
double              Line::GetMaxDescender               () const        { return m_maxDescender; }
double              Line::GetLowestUnitY                () const        { return m_lowestUnitY; }
double              Line::GetMaxDistanceAboveBaseline   () const        { return m_maxDistanceAboveBaseline; }
double              Line::GetMaxExactHeightAboveOrigin  () const        { return m_maxExactDistanceAboveOrigin; }
double              Line::GetMaxExactDepthBelowOrigin   () const        { return m_maxExactDepthBelowOrigin; }
double              Line::GetMaxUnitOffset              () const        { return m_maxUnitOffset; }
double              Line::GetMaxHorizontalCellIncrement () const        { return m_maxHorizontalCellIncrement; }
bool                Line::IsWrappedLine                 () const        { return !this->EndsInParagraphBreak () && !this->EndsInLineBreak (); }
bool                Line::IsEmpty                       () const        { return (0 == this->GetRunCount ()); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void Line::ComputeLineInformation (TextBlockCR textBlock)
    {
    m_maxUnitWidth                  = 0.0;
    m_maxUnitHeight                 = 0.0;
    m_maxAscender                   = 0.0;
    m_maxDescender                  = 0.0;
    m_lowestUnitY                   = 0.0;
    m_maxDistanceAboveBaseline      = 0.0;
    m_maxUnitOffset                 = 0.0;
    m_maxExactDepthBelowOrigin      = 0.0;
    m_maxHorizontalCellIncrement    = 0.0;

    // NEEDS_WORK: Need to clean this up and also remove guess work in the cae of at least lien spacing as of now this should do. (same as v85)
    DRange3d    range;
    
    for (size_t i = 0; i < GetRunCount (); i++)
        {
        RunP run = GetRun (i);
        if (run->GetNominalWidth () > m_maxUnitWidth)
            m_maxUnitWidth = run->GetNominalWidth ();

        if (run->GetNominalHeight () > m_maxUnitHeight)
            m_maxUnitHeight = run->GetNominalHeight ();

        range = run->ComputeTransformedNominalRange ();

        double ascender = range.high.y;

        if (run->GetMaxDisplacementAboveOrigin () > m_maxDistanceAboveBaseline)
            m_maxDistanceAboveBaseline = run->GetMaxDisplacementAboveOrigin ();

        if (ascender > m_maxAscender)
            m_maxAscender = ascender;

        // NEEDS_WORK: This includes carriage return heights that cause problems. See how we can deal with that.

        if (run->GetMaxExactHeightAboveOrigin () > m_maxExactDistanceAboveOrigin)
            m_maxExactDistanceAboveOrigin = run->GetMaxExactHeightAboveOrigin ();

        double descender = run->GetMaxDisplacementBelowOrigin ();
        if (descender > m_maxDescender)
            m_maxDescender = descender;

        double lowestUnitY = m_maxExactDistanceAboveOrigin; // as far as I can see this is what it works out to in Paul's code; in effect this gets the baseline location
        if (lowestUnitY > m_lowestUnitY)
            m_lowestUnitY = lowestUnitY;

        if (m_maxExactDepthBelowOrigin > run->GetMaxExactDepthBelowOrigin ())
            m_maxExactDepthBelowOrigin = run->GetMaxExactDepthBelowOrigin ();

        if (run->GetMaxHorizontalCellIncrement () > m_maxHorizontalCellIncrement)
            m_maxHorizontalCellIncrement = run->GetMaxHorizontalCellIncrement ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
void Line::GetFractionAlignment (size_t startIndex, StackedFractionAlignment& fractionAlignment, bool& fractionsPresent) const
    {
    // The alignment of stacked fractions with flowin text does not work very well
    // in V8.5. It is not a very well defined behavior.  There are two ways to approach
    // this.  1. Say there can be only one alignment per line.  2. Allow alignment with
    // the previous character.  In 1, we can move all the characters in the line to align
    // at the same line.  In 2, we can have different alignments within the line and we
    // only move the fractions with respect to the pther characters.  1 seems to be more
    // consistent with the way ACAD works as well as the way 8.5 works in parts.  So,
    // using that approach the algorthm is as follows:
    // 1. Find the alignment of the first fraction.
    // 2. Find the tallest unit (fractions and otherwise)
    // 3. Move all units in the line upwards by a distance based on the alignment

    // Find the fraction alignment to be applied and the tallest unit
    for (size_t i = startIndex; i < GetRunCount (); i++)
        {
        FractionP fraction = dynamic_cast <FractionP> (GetRun (i));
        if (NULL == fraction)
            continue;

        fractionAlignment = fraction->GetAlignment ();
        if (StackedFractionAlignment::None != fractionAlignment)
            {
            fractionsPresent = true;
            return;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
bool Line::AlignUnits (double nodeNumberHeight, DgnLineSpacingType lineSpacingType)
    {
    double  maxNominalHeight    = 0.0;
    double  maxNominalDepth     = 0.0;
    
    for (RunCP run : m_runArray)
        {
        double runNominalHeight = run->GetNominalHeight ();
        if (runNominalHeight > maxNominalHeight)
            maxNominalHeight = runNominalHeight;

        double runNominalDepth = -run->GetNominalRange ().low.y;
        if (runNominalDepth > maxNominalDepth)
            maxNominalDepth = runNominalDepth;
        }
    

    StackedFractionAlignment    fractionAlignment   = StackedFractionAlignment::None;
    bool                        fractionsPresent    = false;
    GetFractionAlignment (0, fractionAlignment, fractionsPresent);

    bool changed = false;
    
    for (size_t iRun = 0; iRun < m_runArray.size (); ++iRun)
        {
        RunP                run                 = m_runArray[iRun];
        RunPropertiesCR     runProperties       = run->GetProperties ();
        double              runNominalHeight    = run->GetNominalHeight ();
        double              runNominalDepth     = std::max (-run->GetNominalRange ().low.y, 0.0);

        double lineOffsetY = runNominalDepth - maxNominalDepth;

        // *** TRICKY: The values 0,1,2 seem to mean the same thing in LineAlignment and StackedFractionAlignment
        switch ((LineAlignment)(fractionsPresent ? (LineAlignment)fractionAlignment : runProperties.GetLineAlignment ()))
            {
            case LINE_ALIGNMENT_Center: lineOffsetY += ((maxNominalHeight - runNominalHeight) / 2.0);   break;
            case LINE_ALIGNMENT_Top:    lineOffsetY += (maxNominalHeight - runNominalHeight);           break;
            }

        DVec3d lineOffset = run->GetLineOffset ();
        if (lineOffset.y != lineOffsetY)
            {
            lineOffset.y = lineOffsetY;
            run->SetLineOffset (lineOffset);
            changed = true;
            }

        if (m_maxUnitOffset < fabs (lineOffsetY))
            m_maxUnitOffset = lineOffsetY;

        if (NULL != dynamic_cast <FractionP> (run))
            GetFractionAlignment (iRun + 1, fractionAlignment, fractionsPresent);
        }

    return changed;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus Line::AppendToLine (RunP currentRun, RunP nextRun, TextBlockNodeArrayR unitArray, ProcessContextCR processContext)
    {
    AppendStatus appendStatus;

    if (NULL != currentRun)
        {
        appendStatus = currentRun->AppendNextRunToLine (*this, nextRun, processContext);

        if (APPEND_STATUS_Appended & appendStatus)
            {
            unitArray.pop_front ();

            if (NULL != nextRun)
                unitArray.push_front (nextRun);

            return appendStatus;
            }
        }

    RunP nextSplitRun = NULL;
    appendStatus = nextRun->AppendToLine (*this, nextSplitRun, processContext);

    if (APPEND_STATUS_Appended & appendStatus)
        unitArray.pop_front ();

    // Things like EDFs won't actually split; don't put the same run in the array, or else chaos will ensue.
    if ((NULL != nextSplitRun) && (nextSplitRun != unitArray.front ()))
        unitArray.push_front (nextSplitRun);

    return appendStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
void Line::UpdateRunsForEndOfLineFlag ()
    {
    size_t runArraySize = m_runArray.size ();

    for (size_t i = 0; i < runArraySize; i++)
        {
        RunP    currRun             = m_runArray[i];
        bool    shouldBeEndOfLine   = (i == (runArraySize - 1));

        if (currRun->IsLastRunInLine () != shouldBeEndOfLine)
            currRun->SetIsLastRunInLine (shouldBeEndOfLine);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/05
//---------------------------------------------------------------------------------------
double Line::_GetExactWidth () const
    {
    DRange3d localRange = GetExactRange ();
    if (!localRange.isNull ())
        return localRange.high.x - localRange.low.x;
    
    return 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/05
//---------------------------------------------------------------------------------------
double Line::_GetExactHeight () const
    {
    DRange3d localRange = GetExactRange ();
    if (!localRange.isNull ())
        return localRange.high.y - localRange.low.y;
    
    return 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void Line::_Drop (TextBlockNodeArrayR unitArray)
    {
    for (size_t i = 0; i < GetRunCount (); i++)
        {
        TextBlockNodeArray droppedArray;
        GetRun (i)->Drop (droppedArray);
        
        unitArray.insert (unitArray.end (), droppedArray.begin (), droppedArray.end ());
        }
    
    m_runArray.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
size_t Line::GetNumberOfChars () const
    {
    size_t nUnits = 0;
    for (size_t i = 0; i < GetRunCount (); i++)
        nUnits += GetRun (i)->GetCharacterCount ();
    
    return nUnits;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
bool Line::EndsInParagraphBreak () const
    {
    if (IsEmpty ())
        return false;

    return NULL != dynamic_cast <ParagraphBreakCP> (m_runArray.back ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
bool Line::EndsInLineBreak () const
    {
    if (IsEmpty ())
        return false;

    return NULL != dynamic_cast <LineBreakP> (m_runArray.back ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
bool Line::IsComplete (RunR nextRun, ProcessContextCR processContext)
    {
    if (0 == GetRunCount ())
        return false;

    RunCP pLastRun = GetRun (GetRunCount () - 1);

    if (NULL != dynamic_cast <ParagraphBreakCP> (pLastRun))
        return true;

    if (NULL != dynamic_cast <LineBreakCP> (pLastRun))
        return true;

    return !nextRun.CanFit (*this, processContext);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/06
//---------------------------------------------------------------------------------------
bool Line::IsBlankLine () const
    {
    if (IsEmpty ())
        return true;

    size_t nRuns = GetRunCount ();
    if ((nRuns > 1) || (GetRun (0)->GetCharacterCount () > 1))
        return false;

    RunCP run = GetRun (0);
    if (NULL != dynamic_cast <ParagraphBreakCP> (run) || NULL != dynamic_cast <LineBreakCP> (run))
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/05
//---------------------------------------------------------------------------------------
bool Line::IsSpaceBeforeParagraphBreak () const
    {
    if (GetRunCount () < 2)
        return false;

    RunP run = GetRun (GetRunCount () - 2);
    BeAssert (NULL != run);
    
    return run->IsLastCharSpace ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/08
//---------------------------------------------------------------------------------------
bool Line::AllowTrailingWordBreak () const
    {
    if (0 == GetRunCount ())
        return true;

    return GetRun (GetRunCount () - 1)->AllowTrailingWordBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
void Line::AddRun (RunP run)
    {
    m_runArray.push_back (run);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
void Line::ExtractNodes (CaretCR caret, TextBlockNodeArrayR unitArray)
    {
    // Nothing to extract?
    if (0 == GetRunCount ())
        return;

    RunP frontPiece = GetRun (caret.GetRunIndex ());
    if (NULL == frontPiece)
        { BeAssert (false && L"Invalid caret."); return; }
    
    RunP    tailPiece;
    size_t  startIndex;

    frontPiece->Splice (frontPiece, tailPiece, caret.GetCharacterIndex ());

    if (NULL != frontPiece && NULL != tailPiece)
        {
        unitArray.push_back (tailPiece);
        startIndex = caret.GetRunIndex () + 1;
        }
    else
        {
        startIndex = caret.GetRunIndex ();

        if (NULL == tailPiece)
            startIndex++;
        }

    for (size_t i = startIndex; i < GetRunCount (); i++)
        unitArray.push_back (GetRun (i));

    m_runArray.erase (m_runArray.begin () + startIndex, m_runArray.end ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/04
//---------------------------------------------------------------------------------------
AppendStatus Line::AppendNodes (TextBlockNodeArrayR unitArray, ProcessContextR processContext)
    {
    processContext.SetLine (this);

    TextBlockCP         textBlock       = processContext.GetTextBlock ();
    ParagraphCP         paragraph       = processContext.GetParagraph ();
    AppendStatus        appendStatus    = APPEND_STATUS_Appended;
    DPoint3d            origin;
    
    while (0 != unitArray.size ())
        {
        if (unitArray[0]->GetUnitLevel () < TEXTBLOCK_NODE_LEVEL_Run)
            break;

        // ensure last run in line is not a CR or LF which means we should have created a new line
        RunP currentRun = 0 == GetRunCount () ? NULL : m_runArray.back ();
        BeAssert (NULL == currentRun || (NULL == dynamic_cast <ParagraphBreakP> (currentRun) && NULL == dynamic_cast <LineBreakP> (currentRun)));

        RunP nextRun = static_cast <RunP> (unitArray[0]);
        BeAssert (NULL != nextRun);

        if (NULL == currentRun)
            {
            origin.zero ();
            }
        else
            {
            currentRun->SetIsLastRunInLine (false);

            DVec3d displacement = currentRun->GetRunSpacing ();

            origin = currentRun->GetOrigin ();
            origin.add (&displacement);
            origin.x += computeXOffsetForLeadingRSCSpaces (*processContext.GetTextBlock (), *processContext.GetParagraph (), *this, *nextRun);
            }

        nextRun->SetOrigin (origin);

        appendStatus = AppendToLine (currentRun, nextRun, unitArray, processContext);

        this->UpdateRunsForEndOfLineFlag ();

        ComputeRange ((processContext.GetProcessLevel () >= PROCESS_LEVEL_Character), processContext.GetParagraph ()->GetLineSpacingType (), textBlock->GetNodeOrFirstRunHeight ());

        if (APPEND_STATUS_LineBreak & appendStatus || APPEND_STATUS_ParagraphBreak & appendStatus || APPEND_STATUS_Overflow & appendStatus)
            break;
        }

    // We also want to compute such information extraneous information used in line and paragraph spacing calculations
    ComputeLineInformation (*textBlock);
    
    if (AlignUnits (textBlock->GetNodeOrFirstRunHeight (), paragraph->GetLineSpacingType ()))
        ComputeRange (true, paragraph->GetLineSpacingType (), textBlock->GetNodeOrFirstRunHeight ());
    
    processContext.SetLine (NULL);
    
    return appendStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
static void drawRunHighlight (ViewContextR context, RunCR run)
    {
    IViewDrawR  draw        = context.GetIViewDraw ();
    UInt32      strokeColor = context.GetViewport ()->GetHiliteColor ();
    UInt32      fillColor   = 0;
    
    draw.SetSymbology (strokeColor, fillColor, 0, 0);
                
    DRange3d    runRange    = run.ComputeTransformedNominalRange ();
    double      fontHeight  = run.GetProperties ().GetFontSize ().y;
    double      padding     = fontHeight * 0.1;
            
    runRange.low.x  -= padding;
    runRange.low.y  -= padding;
    runRange.high.x += padding;
    runRange.high.y += padding;

    if (NULL != dynamic_cast<CharStreamCP>(&run))
        runRange.low.y -= (fontHeight * run.GetProperties ().ResolveFont ().GetDescenderRatio ());

    DPoint3d shapePts[5] =
        {
        {   runRange.low.x,     runRange.low.y,     runRange.low.z  },
        {   runRange.low.x,     runRange.high.y,    runRange.low.z  },
        {   runRange.high.x,    runRange.high.y,    runRange.low.z  },
        {   runRange.high.x,    runRange.low.y,     runRange.low.z  },
        {   runRange.low.x,     runRange.low.y,     runRange.low.z  }
        };
            
    draw.DrawShape3d (5, shapePts, false, &runRange.low);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void Line::Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    Transform transform = this->GetTransform ();
    context.PushTransform (transform);
        {
        for (RunCP run : m_runArray)
            {
            if (options.GetHighlightRunCP () == run)
                drawRunHighlight (context, *run);
            
            run->Draw (context, isViewIndependent, options);
            }
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/04
//---------------------------------------------------------------------------------------
RunP Line::GetRun (size_t index) const
    {
    if (index >= m_runArray.size())
        return NULL;
    
    return m_runArray[index];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/07
//---------------------------------------------------------------------------------------
BentleyStatus Line::ComputeCaretAtLocation (CaretR caret, DPoint3dCR locationIn, bool isVertical, bool isStrict) const
    {
    Transform transform = this->GetTransform ();
    transform.inverseOf (&transform);

    DPoint3d location = locationIn;
    transform.multiply (&location);
    
    size_t runIndex = 0;

    for (; runIndex < this->GetRunCount (); ++runIndex)
        {
        RunP run = this->GetRun (runIndex);
        
        if (!run->IsContentRun ())
            break;
        
        DRange3d range;
        range.Init ();
        run->ComputeTransformedHitTestRange (range);
        
        // Vertical glyphs are centered based on font size. Thus, their range does not necessarily include the origin (in X).
        //  We need to be able to round-trip ComputeCaretParameters with this method, and that method will utilize origins.
        if (isVertical)
            range.Extend (run->GetOrigin ());
        
        bool    containsPoint       = isVertical ? location.y > range.low.y : location.x < range.high.x;
        bool    dontConsiderNextRun = false;
        
        if (containsPoint && isStrict)
            {
            containsPoint &= isVertical ? location.y > range.high.y : location.x > range.low.x;
            dontConsiderNextRun = true;
            }

        if (containsPoint)
            {
            caret.SetRunIndex (runIndex);
            return run->ComputeCaretAtLocation (caret, location, isStrict);
            }
        
        if (dontConsiderNextRun)
            return ERROR;
        }
    
    if (isStrict)
        return ERROR;

    caret.SetRunIndex (((m_runArray.size () < 2) || m_runArray.back ()->IsContentRun ()) ? m_runArray.size () - 1 : runIndex - 1);
    
    RunCR caretRun = *caret.GetCurrentRunCP ();
    
    caret.SetCharacterIndex (caretRun.IsContentRun () ? caretRun.GetCharacterCount () : 0);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void Line::ComputeCaretParameters (DPoint3dR location, DVec3dR direction, CaretCR caret) const
    {
    RunCP run = this->GetRun (caret.GetRunIndex ());
    if (NULL == run)
        return;

    run->ComputeCaretParameters (location, direction, caret);

    DPoint3d lineOffset = run->GetLineOffset ();
    location.SumOf (location, lineOffset);
    
    DPoint3d origin = this->_GetOrigin ();
    location.SumOf (location, origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
double Line::ComputeLeftEdgeAlignDistance () const
    {
    return ((0 == m_runArray.size ()) ? 0.0 : m_runArray.front ()->ComputeLeftEdgeAlignDistance ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void Line::ComputeBaselineDisplacement (DVec3d& baselineDisplacement, DgnLineSpacingType lineSpacingType, TextBlockCR textBlock) const
    {
    baselineDisplacement.zero ();
    if (textBlock.GetProperties ().IsVertical ())
        return;
    
    if (DgnLineSpacingType::AtLeast == lineSpacingType && !this->ContainsOnlyWhitespace ())
        baselineDisplacement.y = -m_maxExactDistanceAboveOrigin;
    else
        baselineDisplacement.y = -GetMaxDistanceAboveBaseline ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void Line::SetBaselineAdjustedOrigin (DPoint3dCR origin, TextBlockCR textBlock, DgnLineSpacingType lineSpacingType)
    {
    DVec3d baselineDisplacement;
    ComputeBaselineDisplacement (baselineDisplacement, lineSpacingType, textBlock);

    DPoint3d lineOrigin;
    lineOrigin.sumOf (&origin, &baselineDisplacement);
    
    SetOrigin (lineOrigin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void Line::GetBaselineAdjustedOrigin (DPoint3dR origin, TextBlockCR textBlock, DgnLineSpacingType lineSpacingType) const
    {
    origin = GetOrigin ();

    DVec3d baselineDisplacement;
    ComputeBaselineDisplacement (baselineDisplacement, lineSpacingType, textBlock);
    
    origin.sumOf (&origin, &baselineDisplacement, -1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void Line::ComputeRange (bool recomputeComponentRanges, DgnLineSpacingType lineSpacingType, double nodeNumberHeight)
    {
    if (recomputeComponentRanges)
        {
        for (size_t i = 0; i < GetRunCount (); i++)
            m_runArray[i]->Preprocess ();
        }

    // If there is more than one word in the line and the last word contains just a single line feed or carriage return, then do not
    //  include its range in the line range.
    
    size_t nRuns = GetRunCount ();
    
    if (nRuns > 1 && (NULL != dynamic_cast <ParagraphBreakCP> (m_runArray.back ()) || NULL != dynamic_cast <LineBreakCP> (m_runArray.back ())))
        nRuns--;

    m_nominalRange.init ();
    for (size_t i = 0; i < nRuns; ++i)
        {
        DRange3d runRange = GetRun (i)->ComputeTransformedNominalRange ();
        ExtendNominalRange (runRange);
        }

    m_exactRange.init ();
    for (size_t i = 0; i < nRuns; ++i)
        {
        DRange3d runRange = GetRun (i)->ComputeTransformedExactRange ();
        ExtendExactRange (runRange);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void Line::FullJustify (double unusedSpace, bool vertical)
    {
    if (unusedSpace < mgds_fc_epsilon)
        return;

    // Move to the last run that is not a tab
    RunArray::iterator iter;
    for (iter = m_runArray.end (); iter != m_runArray.begin (); --iter)
        {
        if (NULL != dynamic_cast<TabP>(*(iter - 1)))
            break;
        }

    if (m_runArray.end () == iter)
        return;

    // Break up into words
    RunArray    runArray            (iter, m_runArray.end ());
    
    m_runArray.erase (iter, m_runArray.end ());

    RunP crOrLF = NULL;
    if (NULL != dynamic_cast <ParagraphBreakP> (runArray.back ()) || NULL != dynamic_cast <LineBreakP> (runArray.back ()))
        {
        crOrLF = runArray.back ();
        runArray.pop_back ();
        }

    sizetVector wordBreakIndices;
    getWordBreakIndices (wordBreakIndices, runArray);

    if (0 == wordBreakIndices.size ())
        {
        if (runArray.size () > 0)
            m_runArray.insert (m_runArray.begin (), runArray.begin (), runArray.end ());

        if (NULL != crOrLF)
            m_runArray.push_back (crOrLF);

        for (RunP run: m_runArray)
            run->ClearTrailingCharacterOffsetOverride ();
        
        return;
        }

    WordArray wordArray;
    getWordArray (wordArray, wordBreakIndices, runArray);

    // Now, add spacePerWord for every word
    double  spacePerWord    = (wordArray.size () <= 1) ? 0.0 : unusedSpace / (wordArray.size () - 1);
    double  spaceToBeAdded  = 0.0;
    
    for (WordArray::iterator witer = wordArray.begin (); witer != wordArray.end (); ++witer)
        {
        for (iter = (*witer)->begin (); iter != (*witer)->end (); ++iter)
            {
            DPoint3d origin = (*iter)->GetOrigin ();
            origin.x += spaceToBeAdded;
            (*iter)->SetOrigin (origin);
            (*iter)->SetOverrideTrailingCharacterOffset ((*iter)->GetNominalWidth () + spacePerWord);
            m_runArray.push_back (*iter);
            }

        spaceToBeAdded += spacePerWord;
        
        delete (*witer);
        }

    m_runArray.back ()->ClearTrailingCharacterOffsetOverride ();
    
    if (NULL != crOrLF)
        m_runArray.push_back (crOrLF);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
bool Line::ContainsOnlyWhitespace () const
    {
    size_t runArraySize = m_runArray.size ();
    for (size_t i = 0; i < runArraySize; i++)
        {
        if (!m_runArray[i]->ContainsOnlyWhitespace ())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool Line::Equals (LineCR rhs, TextBlockCompareOptionsCR compareOptions) const
    {
    if (!T_Super::Equals (rhs, compareOptions))
        return false;
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (m_isFullLine != rhs.m_isFullLine) return false;
        
        if (!compareOptions.AreDoublesEqual (m_maxUnitHeight,               rhs.m_maxUnitHeight))               return false;
        if (!compareOptions.AreDoublesEqual (m_maxUnitWidth,                rhs.m_maxUnitWidth))                return false;
        if (!compareOptions.AreDoublesEqual (m_maxAscender,                 rhs.m_maxAscender))                 return false;
        if (!compareOptions.AreDoublesEqual (m_maxDescender,                rhs.m_maxDescender))                return false;
        if (!compareOptions.AreDoublesEqual (m_lowestUnitY,                 rhs.m_lowestUnitY))                 return false;
        if (!compareOptions.AreDoublesEqual (m_maxDistanceAboveBaseline,    rhs.m_maxDistanceAboveBaseline))    return false;
        if (!compareOptions.AreDoublesEqual (m_maxExactDistanceAboveOrigin, rhs.m_maxExactDistanceAboveOrigin)) return false;
        if (!compareOptions.AreDoublesEqual (m_maxUnitOffset,               rhs.m_maxUnitOffset))               return false;
        if (!compareOptions.AreDoublesEqual (m_maxExactDepthBelowOrigin,    rhs.m_maxExactDepthBelowOrigin))    return false;
        if (!compareOptions.AreDoublesEqual (m_maxHorizontalCellIncrement,  rhs.m_maxHorizontalCellIncrement))  return false;
        
        if (!m_baselineDisplacement.IsEqual (rhs.m_baselineDisplacement, compareOptions.GetTolerance ())) return false;
        }
    
    if (m_runArray.size () != rhs.m_runArray.size ())
        return false;
    
    for (RunArray::const_iterator lhsIter = m_runArray.begin (), rhsIter = rhs.m_runArray.begin (); m_runArray.end () != lhsIter; ++lhsIter, ++rhsIter)
        if (!(*lhsIter)->Equals (**rhsIter, compareOptions))
            return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void Line::SplitRunInPlace (CaretCR at)
    {
    // In theory, splitting in-place should not require additional layout.
    
    RunP    lhs;
    RunP    rhs;
    
    size_t atCharacterIndex = at.GetCharacterIndex ();
    if (0 == atCharacterIndex || atCharacterIndex >= at.GetCurrentRunCP ()->GetCharacterCount ())
        return;
    
    at.GetCurrentRunP ()->Splice (lhs, rhs, at.GetCharacterIndex ());
    
    m_runArray.insert (m_runArray.begin () + at.GetRunIndex () + 1, rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Line::ComputeTransformedHitTestRange (DRange3dR hitTestRange) const
    {
    for (RunCP run: m_runArray)
        {
        DRange3d runRange;
        runRange.Init ();

        run->ComputeTransformedHitTestRange (runRange);

        hitTestRange.Extend (&runRange.low, 2);
        }
    
    this->GetTransform ().Multiply (hitTestRange, hitTestRange);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void Line::ComputeElementRange (DRange3dR value) const
    {
    value.Init ();

    for (RunCP run: m_runArray)
        {
        DRange3d runElementRange;
        run->GetElementRange (runElementRange);

        run->GetTransform ().Multiply (runElementRange, runElementRange);

        value.Extend (&runElementRange.low, 2);
        }
    }
