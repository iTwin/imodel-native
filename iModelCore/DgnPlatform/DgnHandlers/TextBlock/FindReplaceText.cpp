/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/FindReplaceText.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define CHAR_SPACE              0x0020
#define CHAR_LINEFEED           0x000A
#define CHAR_CARRIAGERETURN     0x000D
#define CHAR_TAB                0x0009
#define IS_WHITESPACE_CHAR(x)   (CHAR_SPACE == x || CHAR_LINEFEED == x || CHAR_CARRIAGERETURN == x)

#ifdef UNUSED_FUNCTION
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
static bool processFraction (WStringR unicodeString, WString::size_type fractionStart, WString::size_type fractionSeparatorIndex, WString::size_type fractionEnd, DgnFontCR rscFont)
    {
    if (0 == fractionSeparatorIndex || fractionSeparatorIndex == fractionStart || fractionSeparatorIndex == fractionEnd)
        return false;
    
    int numerator;
    if (1 != BE_STRING_UTILITIES_SWSCANF (unicodeString.c_str () + fractionStart, L"%d", &numerator))
        { BeAssert (false); return false; }
    
    int denominator;
    if (1 != BE_STRING_UTILITIES_SWSCANF (unicodeString.c_str () + (fractionSeparatorIndex + 1), L"%d", &denominator))
        { BeAssert (false); return false; }
    
    UInt16 fractionCharacterCode = rscFont.GetCharCodeFromFraction (numerator, denominator);
    if (0 == fractionCharacterCode)
        return false;
    
    unicodeString.erase ((fractionStart + 1), (fractionEnd - fractionStart));
    
    unicodeString[fractionStart] = rscFont.RemapFontCharToUnicodeChar (fractionCharacterCode);
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/09
//---------------------------------------------------------------------------------------
static void convertFractionsIntoRSCGlyphs (WStringR unicodeString, DgnFontCR rscFont)
    {
    bool                inPotentialFraction = false;
    bool                isCurrCharDigit;
    WString::size_type  fractionStart;          // index of first character in the fraction sequence
    WString::size_type  fractionSeparatorIndex; // index of the forward slash
    WString::size_type  fractionEnd;            // index of the final character in the fraction sequence (NOT 1 past)
    
    for (WString::size_type i = 0; i < unicodeString.size (); ++i)
        {
        WString::value_type c = unicodeString[i];
        
        isCurrCharDigit = (0 != iswdigit (c));
        
        if (!inPotentialFraction)
            {
            if (!isCurrCharDigit)
                continue;
            
            inPotentialFraction     = true;
            fractionStart           = i;
            fractionSeparatorIndex  = 0;
            fractionEnd             = 0;
            
            continue;
            }
        
        if (L'/' == c && 0 == fractionSeparatorIndex)
            {
            fractionSeparatorIndex = i;
            continue;
            }
        
        if (isCurrCharDigit)
            continue;
        
        inPotentialFraction = false;
        
        fractionEnd = (isCurrCharDigit ? i : (i - 1));
        
        if (processFraction (unicodeString, fractionStart, fractionSeparatorIndex, fractionEnd, rscFont))
            i = fractionStart;
        }
    
    if (inPotentialFraction)
        processFraction (unicodeString, fractionStart, fractionSeparatorIndex, (unicodeString.size () - 1), rscFont);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Abeesh.Basheer  07/08
//---------------------------------------------------------------------------------------
bool TextBlock::GetRunPropertiesForInsertion (RunPropertiesR prop, CaretCR caretIn) const
    {
    if (this->IsEmpty ())
        {
        RunPropertiesCP nodeProp = this->GetTextNodeRunProperties ();
        if (NULL == nodeProp)
            return false;

        prop = *nodeProp;
        return true;
        }

    if (!caretIn.IsAtBeginning ())
        {
        Caret previousRunCaret = caretIn;
        previousRunCaret.MoveToPreviousRun ();

        RunCP previousRun = previousRunCaret.GetCurrentRunCP ();
        if (NULL == previousRun)
            return false;

        prop = previousRun->GetProperties ();
        return true;
        }

    Caret firstUsableRunCaret = caretIn;
    firstUsableRunCaret.MoveToNextRun ();

    RunCP firstUsableRun = firstUsableRunCaret.GetCurrentRunCP ();
    if (NULL == firstUsableRun)
        return false;

    prop = firstUsableRun->GetProperties ();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Abeesh.Basheer  07/08
//---------------------------------------------------------------------------------------
RunPropertiesCP TextBlock::GetRunProperty (TextBlockNodeCP domNode) const
    {
    if (NULL == domNode)
        return NULL;

    RunCP domNodeAsRun = dynamic_cast<RunCP>(domNode);
    if (NULL != domNodeAsRun)
        return &domNodeAsRun->GetProperties ();

    LineCP domNodeAsLine = dynamic_cast<LineCP>(domNode);
    if (NULL != domNodeAsLine)
        {
        if (0 == domNodeAsLine->GetRunCount ())
            return NULL;
        
        return &domNodeAsLine->GetRun (0)->GetProperties ();
        }
    
    ParagraphCP domNodeAsParagraph = dynamic_cast<ParagraphCP>(domNode);
    if (NULL != domNodeAsParagraph)
        {
        if (0 == domNodeAsParagraph->GetLineCount ())
            return NULL;
        
        LineCP firstLine = domNodeAsParagraph->GetLine (0);
        if (0 == firstLine->GetRunCount ())
            return NULL;
        
        return &firstLine->GetRun (0)->GetProperties ();
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
static bool doesEndCaretCrossEDFBoundary (TextBlockCR textBlock, CaretCR start, CaretCR end, bool* startsInEDF)
    {
    bool _startsInEDF;
    if (NULL == startsInEDF)
        startsInEDF = &_startsInEDF;

    EdfCharStreamCP firstRunAsEdf   = dynamic_cast<EdfCharStreamCP>(start.GetCurrentRunP ());
                    *startsInEDF    = (NULL != firstRunAsEdf);
    Caret           runIter         = start;

    // No more runs? Didn't possible cross a boundary.
    if (SUCCESS != runIter.MoveToNextRun ())
        return false;

    RunRange runRange (runIter, end);
    FOR_EACH (RunCR run, runRange)
        {
        // If we started in an EDF, by-rule the following run will be a non-EDF or a different EDF>
        if (*startsInEDF)
            return true;

        // Otherwise we only cross if we encounter a new EDF.
        if (NULL != dynamic_cast<EdfCharStreamCP>(&run))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
void TextBlock::ReplaceText (WCharCP string, CaretCR start, CaretR end)
    {
    if (IsInField (start) || IsInField (end))
        return;

    // NEEDS_WORK: Search and replace special characters.

    BeAssert (!start.IsAfter (end));

    /*
        Since EDF's aren't going to be around for long, I'm not going to spend too much time working them out here.
        The basic scenario of replacing N characters with N characters in a single EDF needs to be supported, but I'm okay with being cheap in other scenarios.
        The general principle is to treat EDF's as atomic units; do not find/replace both inside and outside an EDF. The full operation should occur completely
        inside an EDF, or completely outside an EDF.

        If find-start is in an EDF, and find-end is outside of that EDF
            Then do nothing
        Else
            If replace size equals find size
                Then simple replace within EDF
            Else if replace size is less than find size
                Then replace characters and pad with spaces to not change EDF size
            Else if replace size is greater than find size
                Then replace characters up to the EDF size, and clip any additional
    */

    CharStreamP     firstRunAsCharStream    = dynamic_cast<CharStreamP>(start.GetCurrentRunP ());
    bool            startsInEDF;
    bool            endCrossesEDF           = doesEndCaretCrossEDFBoundary (*this, start, end, &startsInEDF);

    if (endCrossesEDF)
        return;

    if (startsInEDF)
        {
        // end will be one unit past the last character to find (e.g. potentially not in the same CharStream);
        //  since we want the character index one past the end within the run, need to adjust.
        Caret endIndexCaret = end;
        endIndexCaret.MoveToPreviousCharacter ();

        EdfCharStreamP runAsEdf = dynamic_cast<EdfCharStreamP>(firstRunAsCharStream);
        if (NULL == runAsEdf)
            { BeAssert (false && L"Detected that we started in an EDF, yet the first run is not an EDF."); return; }

        WString edfText     = runAsEdf->GetString ();
        size_t  findStart   = start.GetCharacterIndex ();
        size_t  findEnd     = endIndexCaret.GetCharacterIndex () + 1;

        WString newEdfText = edfText.replace (findStart, findEnd - findStart, string);
        newEdfText.Trim ();
        
        runAsEdf->SetString (newEdfText);

        m_processLevel = PROCESS_LEVEL_Run;
        
        this->Reprocess ();

        return;
        }

    TextBlockNodeArray   endPieces;
    Caret       caret           (end);
    TextBlockNodeArray   replacedPieces;
    
    ExtractNodes (caret, endPieces);

    if (!end.IsAt (start))
        {
        caret = start;
        ExtractNodes (caret, replacedPieces);
        }

    TextBlockNodeArray unitArray;
    
    for (TextBlockNodeArray::iterator iter = replacedPieces.begin (); iter != replacedPieces.end (); iter++)
        (*iter)->Drop (unitArray);

    TextBlockNodeArray  newTextBlockNodeArray;
    size_t              nChars                  = wcslen (string);
    
    if (nChars > 0)
        {
        size_t  nReplaceChars   = nChars;
        
        for (TextBlockNodeArray::iterator iter = unitArray.begin (); iter != unitArray.end (); iter++)
            {
            RunP        run             = static_cast<RunP>(*iter);
            size_t      nFittable       = run->GetCharacterCount ();
            size_t      nFit            = ((nFittable < nReplaceChars) ? nFittable : nReplaceChars);
            CharStreamP oldCharStream   = dynamic_cast<CharStreamP>(run);
            
            if (NULL == oldCharStream)
                {
                BeAssert (false);
                continue;
                }
            
            CharStreamP newCharStream   = new CharStream (*oldCharStream);
            
            newTextBlockNodeArray.push_back (newCharStream);
            newCharStream->SetString (WString (string + (nChars - nReplaceChars)).substr (0, nFit) + newCharStream->GetString ().substr (nFittable));
            nReplaceChars -= nFit;

            if (0 == nReplaceChars)
                break;
            }

        // nReplaceChars could be different from above. Hence, dont combine the two sections.

        if (nReplaceChars > 0)
            {
            RunProperties prop (GetDgnModelR ());

            if (!newTextBlockNodeArray.empty())
                {
                RunPropertiesCP runPropP = GetRunProperty (newTextBlockNodeArray.back ());
                if (NULL == runPropP)
                    return;

                prop = *runPropP;
                }
            else if (IsEmpty ())
                {
                if (!endPieces.empty())
                    {
                    RunPropertiesCP runPropP = GetRunProperty (endPieces.front ());
                    if (NULL == runPropP)
                        return;

                    prop = *runPropP;
                    }
                else if (!GetRunPropertiesForInsertion (prop, start))
                    {
                    return;
                    }
                }
            else if (start.IsAtEnd() && (!GetRunPropertiesForInsertion (prop, start)))
                {
                return;
                }
            else if (!GetRunPropertiesForInsertion (prop, start))
                {
                return;
                }

            newTextBlockNodeArray.push_back (new CharStream ((string + nChars - nReplaceChars), prop, this->ComputeRunLayoutFlags ()));
            }
        }

    // This is a compromise for SS3 to be able to replace with CR/LF/Tab, while minimizing complexity and overall TextBlock stability.
    //  This keeps the changes local to this function, does not require any new utilities on TextBlock, and does not disrupt the existing processing above.
    //  It should be re-visited, and worked into the loop(s) above for performance (and cleanliness) considerations.
    for (size_t iUnit = 0; iUnit < newTextBlockNodeArray.size (); ++iUnit)
        {
        // Based on the loop above, the only Units in newUnitArray should be CharStreams.
        CharStreamP currCharStream = dynamic_cast<CharStreamP>(newTextBlockNodeArray[iUnit]);
        if (NULL == currCharStream)
            { BeAssert (false); break; }
        
        size_t  firstCROffset       = currCharStream->GetString ().find (0x000D);
        size_t  firstLFOffset       = currCharStream->GetString ().find (0x000A);
        size_t  firstTabOffset      = currCharStream->GetString ().find (0x0009);
        size_t  controlCharOffset   = 0;
        RunP    controlCharRun      = NULL;
        
        if ((firstCROffset < firstLFOffset) && (firstCROffset < firstTabOffset))
            {
            controlCharRun      = new ParagraphBreak (currCharStream->GetProperties (), this->ComputeRunLayoutFlags ());
            controlCharOffset   = firstCROffset;
            }
        else if (firstLFOffset < firstTabOffset)
            {
            controlCharRun      = new LineBreak (currCharStream->GetProperties (), this->ComputeRunLayoutFlags ());
            controlCharOffset   = firstLFOffset;
            }
        else
            {
            controlCharRun      = new Tab (currCharStream->GetProperties (), this->ComputeRunLayoutFlags ());
            controlCharOffset   = firstTabOffset;
            }
        
        if ((size_t)-1 == controlCharOffset)
            continue;
        
        WString charStreamStr = currCharStream->GetString ();
        charStreamStr.erase (controlCharOffset, 1);
        
        if (charStreamStr.empty ())
            {
            newTextBlockNodeArray[iUnit] = controlCharRun;
            delete currCharStream;
            continue;
            }
        
        currCharStream->SetString (charStreamStr);
        
        RunP    lhs = NULL;
        RunP    rhs = NULL;
        
        currCharStream->Splice (lhs, rhs, controlCharOffset);
        
        if (NULL == lhs)
            {
            newTextBlockNodeArray.insert (newTextBlockNodeArray.begin () + iUnit, controlCharRun);
            ++iUnit;
            }
        else
            {
            newTextBlockNodeArray.insert (newTextBlockNodeArray.begin () + iUnit + 1, controlCharRun);
            ++iUnit;
            
            if (NULL != rhs)
                newTextBlockNodeArray.insert (newTextBlockNodeArray.begin () + iUnit + 1, rhs);
            }
        }
    
    for (TextBlockNodeArray::iterator iter = unitArray.begin (); iter != unitArray.end (); ++iter)
        delete *iter;

    AppendNodes (newTextBlockNodeArray);
    
    end = End ();
    end.MoveToPreviousCharacter ();
    AppendNodes (endPieces);
    
    end.MoveToNextCharacter ();

    // NEEDS_WORK: Repair any damage done by fractions. This is needed when expanding compressed
    // fractions in RSC fonts.  See old code. This is used primarily in spell checker. I need to
    // think of a way to do this.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
bool TextBlock::OffsetCaret (CaretR pointerToOffset, CaretCP pointerToOffsetFrom, int nUnits) const
    {
    if (NULL == pointerToOffsetFrom)
        pointerToOffset = this->Begin ();
    else
        pointerToOffset = *pointerToOffsetFrom;

    while (nUnits--)
        {
        pointerToOffset.MoveToNextCharacter ();
        
        if (pointerToOffset.IsAtEnd ())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
static bool isRangeChangeable (TextBlockCR textBlock, CaretCR start, CaretCR end)
    {
    // Not changeable if in a field.
    RunRange range (start, end);
    for (RunIterator runIter = range.begin (); range.end () != runIter; ++runIter)
        if (textBlock.IsInField (*runIter.ToCaret ()))
            return false;

    // Not changeable if crossing an EDF boundary.
    if (doesEndCaretCrossEDFBoundary (textBlock, start, end, NULL))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2009
//---------------------------------------------------------------------------------------
    // Not changeable if in a field.

    // Not changeable if crossing an EDF boundary.


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
BentleyStatus TextBlock::FindText
(
bool&           changeable,
CaretR          foundStart,
CaretR          foundEnd,
WCharCP       searchText,
bool            regularExpression,
bool            wholeWords,
bool            matchCase,
CaretCR         offsetStart,
CaretCR         offsetEnd
) const
    {
    // Eventaully, I would like to implement a proper regular expression parsers and
    //  proper string search algorithms to search for strings in a textblock.  As of
    //  now, I am going ahead with the current algorithms which convert the textblock
    //  into a string and then search for a pattern in the string and then map back
    //  the locations to the textblock.  This should change.  NEEDS_WORK:
    WString charArray;
    Caret   stPtr (*this);

    // NEEDS_WORK: For now, this is ok.  However, this is going to find the string
    // till the end of the last run.  We will probably get more than what we wanted
    // Need to think about this.
//    RunP run = NULL;
    stPtr = offsetStart;

    TextBlockToStringOptionsPtr tsOptions = TextBlockToStringOptions::CreateDefault ();
    tsOptions->SetShouldExpandRscFractions (false);
    
    WString str = this->ToString (offsetStart, offsetEnd, *tsOptions);
    
    if (0 == str.size ())
        return ERROR;

    WCharP newSearch  = const_cast<WCharP>(searchText);
    WCharP chars      = (WCharP)_alloca ((str.size () + 1) * sizeof (WChar));
    
    wcsncpy (chars, str.c_str (), str.size ());
    *(chars + str.size ()) = 0;

    WCharCP   bufferStart         = chars;
    WCharCP   foundBufferStart    = NULL;
    WCharCP   foundBufferEnd      = NULL;

    if (regularExpression)
        {
        int stop = 0;
        
        if (mdlString_matchREExtended2 (chars, bufferStart, newSearch, &foundBufferStart, &foundBufferEnd, &stop))
            {
            return ERROR;
            }
        else
            {
            int startOffset = static_cast<int>(foundBufferStart - chars);
            int endOffset   = static_cast<int>(foundBufferEnd - chars);

            if (!OffsetCaret (foundStart, &offsetStart, startOffset))
                {
                foundStart.MoveToPreviousCharacter ();
                foundStart.SetCharacterIndex (foundStart.GetCharacterIndex () + 1);
                }

            if (!OffsetCaret (foundEnd, &offsetStart, endOffset))
                {
                foundEnd.MoveToPreviousCharacter ();
                foundEnd.SetCharacterIndex (foundEnd.GetCharacterIndex () + 1);
                }

            // if we are in a field, do not do anything
            changeable = isRangeChangeable (*this, foundStart, foundEnd);
            return SUCCESS;
            }
        }

    if (!matchCase)
        {
        newSearch = (WChar *) _alloca ((wcslen (searchText) + 1) * sizeof (WChar));
        
        wcscpy (newSearch, searchText);
        
        BeStringUtilities::Wcsupr (newSearch);
        BeStringUtilities::Wcsupr (chars);
        }

    if (NULL != (foundBufferStart = ::wcsstr (bufferStart, newSearch)))
        {
        if (wholeWords)
            {
            do
                {
                WCharCP   pPrevChar   = foundBufferStart - 1;
                WCharCP   pEndChar    = foundBufferStart + wcslen (newSearch);

                if ((foundBufferStart == bufferStart || IS_WHITESPACE_CHAR (*pPrevChar)) && (0 == *pEndChar || IS_WHITESPACE_CHAR (*pEndChar)))
                    {
                    int startOffset = static_cast<int>(foundBufferStart - chars);
                    int endOffset   = static_cast<int>(foundBufferStart + wcslen (newSearch) - chars);
                    
                    OffsetCaret (foundStart, &offsetStart, startOffset);
                    OffsetCaret (foundEnd, &offsetStart, endOffset);
                    changeable = isRangeChangeable (*this, foundStart, foundEnd);
                    return SUCCESS;
                    }
                else
                    {
                    WCharCP pNextChar = foundBufferStart + 1;
                    while (*pNextChar)
                        {
                        if (IS_WHITESPACE_CHAR (*pNextChar))
                            break;

                        pNextChar++;
                        }

                    if (0 == *pNextChar)
                        return ERROR;

                    bufferStart = pNextChar + 1;
                    }
                } while (NULL != (foundBufferStart = ::wcsstr (bufferStart, newSearch)));
            }
        else
            {
            int startOffset = static_cast<int>(foundBufferStart - chars);
            int endOffset   = static_cast<int>(foundBufferStart + wcslen (newSearch) - chars);
            
            OffsetCaret (foundStart, &offsetStart, startOffset);
            OffsetCaret (foundEnd, &offsetStart, endOffset);
            changeable = isRangeChangeable (*this, foundStart, foundEnd);
            return SUCCESS;
            }
        }

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/05
//---------------------------------------------------------------------------------------
void TextBlock::GetTextBlockBox (DPoint3d boxPoints[5], bool exact) const
    {
    DRange3d textblockRange = (exact ? GetExactRange () : GetNominalRange ());

    TextBlock::ComputePlanarRangeVertices (boxPoints, textblockRange);

    Transform transform = GetTransform ();
    transform.multiply (boxPoints, 5);
    }
