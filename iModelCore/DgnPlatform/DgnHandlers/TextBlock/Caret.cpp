/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/Caret.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- Caret --------------------------------------------------------------------------------------------------------------------------------- Caret --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
Caret::Caret (TextBlockCR textBlock) :
    RefCountedBase      (),
    m_textBlock         (&textBlock),
    m_paragraphIndex    (0),
    m_lineIndex         (0),
    m_runIndex          (0),
    m_characterIndex    (0),
    m_approachDirection (CARET_MOTION_DIRECTION_Forward)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
Caret::Caret (CaretCR rhs) :
    RefCountedBase      (),
    m_textBlock         (rhs.m_textBlock),
    m_paragraphIndex    (rhs.m_paragraphIndex),
    m_lineIndex         (rhs.m_lineIndex),
    m_runIndex          (rhs.m_runIndex),
    m_characterIndex    (rhs.m_characterIndex),
    m_approachDirection (rhs.m_approachDirection)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
CaretR Caret::operator= (CaretCR rhs)
    {
    if (this != &rhs)
        {
        m_textBlock         = rhs.m_textBlock;
        m_paragraphIndex    = rhs.m_paragraphIndex;
        m_lineIndex         = rhs.m_lineIndex;
        m_runIndex          = rhs.m_runIndex;
        m_characterIndex    = rhs.m_characterIndex;
        m_approachDirection = rhs.m_approachDirection;
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2009
//---------------------------------------------------------------------------------------
CaretPtr Caret::Clone () const
    {
    return new Caret (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/04
//---------------------------------------------------------------------------------------
TextBlockCR             Caret::GetTextBlock         () const                        { return *m_textBlock; }
void                    Caret::SetTextBlock         (TextBlockCP textBlock)         { m_textBlock = textBlock; }
size_t                  Caret::GetParagraphIndex    () const                        { return m_paragraphIndex; }
void                    Caret::SetParagraphIndex    (size_t paragraphIndex)         { m_paragraphIndex = paragraphIndex; }
size_t                  Caret::GetLineIndex         () const                        { return m_lineIndex; }
void                    Caret::SetLineIndex         (size_t lineIndex)              { m_lineIndex = lineIndex; }
size_t                  Caret::GetRunIndex          () const                        { return m_runIndex; }
void                    Caret::SetRunIndex          (size_t runIndex)               { m_runIndex = runIndex; }
size_t                  Caret::GetCharacterIndex    () const                        { return m_characterIndex; }
void                    Caret::SetCharacterIndex    (size_t charIndex)              { m_characterIndex = charIndex; }
CaretMotionDirection    Caret::GetApproachDirection () const                        { return m_approachDirection; }
void                    Caret::SetApproachDirection (CaretMotionDirection value)    { m_approachDirection = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
bool Caret::IsBefore (CaretCR rhs) const
    {
    if (m_paragraphIndex < rhs.m_paragraphIndex)
        return true;
    
    if (m_paragraphIndex > rhs.m_paragraphIndex)
        return false;
    
    if (m_lineIndex < rhs.m_lineIndex)
        return true;

    if (m_lineIndex > rhs.m_lineIndex)
        return false;
        
    if (m_runIndex < rhs.m_runIndex)
        return true;

    if (m_runIndex > rhs.m_runIndex)
        return false;
        
    return (m_characterIndex < rhs.m_characterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
bool Caret::IsAfter (CaretCR rhs) const
    {
    if (m_paragraphIndex > rhs.m_paragraphIndex)
        return true;
    
    if (m_paragraphIndex < rhs.m_paragraphIndex)
        return false;
    
    if (m_lineIndex > rhs.m_lineIndex)
        return true;

    if (m_lineIndex < rhs.m_lineIndex)
        return false;
    
    if (m_runIndex > rhs.m_runIndex)
        return true;

    if (m_runIndex < rhs.m_runIndex)
        return false;
        
    return (m_characterIndex > rhs.m_characterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
bool Caret::IsAt (CaretCR rhs) const
    {
    return  (
            m_paragraphIndex    == rhs.m_paragraphIndex     &&
            m_lineIndex         == rhs.m_lineIndex          &&
            m_runIndex          == rhs.m_runIndex           &&
            m_characterIndex    == rhs.m_characterIndex
            );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
bool Caret::IsAtVisually (CaretCR rhs) const
    {
    return  (
            m_paragraphIndex    == rhs.m_paragraphIndex     &&
            m_lineIndex         == rhs.m_lineIndex          &&
            m_runIndex          == rhs.m_runIndex           &&
            m_characterIndex    == rhs.m_characterIndex     &&
            m_approachDirection == rhs.m_approachDirection
            );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
bool Caret::IsAfterOrAt (CaretCR rhs) const
    {
    return (this->IsAfter (rhs) || this->IsAt (rhs));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
bool Caret::IsBeforeOrAt (CaretCR rhs) const
    {
    return (this->IsBefore (rhs) || this->IsAt (rhs));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
bool Caret::IsAtBeginning () const
    {
    return ((0 == m_paragraphIndex) && (0 == m_lineIndex) && (0 == m_runIndex) && (0 == m_characterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
bool Caret::IsAtEnd () const
    {
    if (m_textBlock->IsEmpty ())
        return ((0 == m_paragraphIndex) && (0 == m_lineIndex) && (0 == m_runIndex) && (0 == m_characterIndex));
    
    if ((m_textBlock->GetParagraphCount () - 1) != m_paragraphIndex)
        return false;
    
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if ((currParagraph->GetLineCount () - 1) != m_lineIndex)
        return false;
    
    LineCP currLine = currParagraph->GetLine (m_lineIndex);
    if ((currLine->GetRunCount () - 1) != m_runIndex)
        return false;
    
    return (currLine->GetRun (m_runIndex)->GetCharacterCount () == m_characterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextParagraph ()
    {
    if (m_paragraphIndex == (m_textBlock->GetParagraphCount () - 1))
        return ERROR;
    
    ++m_paragraphIndex;
    m_lineIndex = m_runIndex = m_characterIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Forward;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousParagraph ()
    {
    if (0 == m_paragraphIndex)
        return ERROR;

    --m_paragraphIndex;
    m_lineIndex = m_runIndex = m_characterIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Backward;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextLine ()
    {
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL == currParagraph)
        return ERROR;
    
    if (m_lineIndex == (currParagraph->GetLineCount () - 1))
        return this->MoveToNextParagraph ();
    
    ++m_lineIndex;
    m_runIndex = m_characterIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Forward;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousLine ()
    {
    if (0 != m_lineIndex)
        {
        --m_lineIndex;
        m_runIndex = m_characterIndex = 0;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Backward;
        
        return SUCCESS;
        }

    if (SUCCESS != this->MoveToPreviousParagraph ())
        return ERROR;
    
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL == currParagraph)
        return ERROR;
    
    m_lineIndex = (this->GetCurrentParagraphCP ()->GetLineCount () - 1);
    m_runIndex = m_characterIndex = 0;
    
    // MoveToPreviousParagraph already set m_approachDirection

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextRun ()
    {
    LineCP currLine = this->GetCurrentLineCP ();
    if (NULL == currLine)
        return ERROR;
    
    if (m_runIndex == (currLine->GetRunCount () - 1))
        return this->MoveToNextLine ();
    
    ++m_runIndex;
    m_characterIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Forward;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextRunInParagraph ()
    {
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL == currParagraph)
        return ERROR;
    
    LineCP currLine = this->GetCurrentLineCP ();
    if (NULL == currLine)
        return ERROR;

    if (m_lineIndex >= (currParagraph->GetLineCount () - 1) && m_runIndex == (currLine->GetRunCount () - 1))
        return ERROR;
    
    return this->MoveToNextRun ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousRun ()
    {
    if (0 != m_runIndex)
        {
        --m_runIndex;
        m_characterIndex = 0;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Backward;

        return SUCCESS;
        }
    
    if (SUCCESS != this->MoveToPreviousLine ())
        return ERROR;
    
    LineCP currLine = this->GetCurrentLineCP ();
    if (NULL == currLine)
        return ERROR;

    m_runIndex = (currLine->GetRunCount () - 1);
    m_characterIndex = 0;
    
    // MoveToPreviousLine already set m_approachDirection

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousRunInParagraph ()
    {
    if (0 == m_lineIndex && 0 == m_runIndex)
        return ERROR;
    
    return this->MoveToPreviousRun ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextCharacter ()
    {
    RunCP currRun = this->GetCurrentRunCP ();
    if (NULL == currRun)
        return ERROR;
    
    if (m_characterIndex == currRun->GetCharacterCount ())
        {
        // If we're at the tail of the current run, for all intensive purposes, the next character is the second character of the next run.
        if (SUCCESS != this->MoveToNextRun ())
            return ERROR;
        
        return this->MoveToNextCharacter ();
        }
    
    m_approachDirection = CARET_MOTION_DIRECTION_Forward;

    if ((m_characterIndex == (currRun->GetCharacterCount () - 1)) && (SUCCESS == this->MoveToNextRun ()))
        return SUCCESS;
    
    ++m_characterIndex;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/05
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousCharacter ()
    {
    if (0 != m_characterIndex)
        {
        --m_characterIndex;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Backward;
        
        return SUCCESS;
        }
    
    if (SUCCESS != this->MoveToPreviousRun ())
        return ERROR;

    RunCP currRun = this->GetCurrentRunCP ();
    if (NULL == currRun)
        return ERROR;
    
    m_characterIndex = currRun->GetCharacterCount () - 1;
    
    // MoveToPreviousRun already set m_approachDirection

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToNextInsertPosition (CaretBias bias)
    {
    if (this->IsAtEnd ())
        return ERROR;
    
    RunCP currentRun = this->GetCurrentRunCP ();

    if (currentRun->IsAtomic () && (currentRun->GetCharacterCount () != m_characterIndex))
        {
        m_characterIndex    = currentRun->GetCharacterCount ();
        m_approachDirection = CARET_MOTION_DIRECTION_Forward;
            
        return SUCCESS;
        }
    
    // If inside a run, just like normal; additionally, if bias is trailing, allow it to be on the end of the run.
    if (currentRun->IsContentRun () && m_characterIndex < ((CARET_BIAS_Trailing == bias) ? currentRun->GetCharacterCount () : currentRun->GetCharacterCount () - 1))
        {
        ++m_characterIndex;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Forward;
        
        return SUCCESS;
        }
    
    Caret   nextRunCaret    (*this);
    RunCP   nextRun         = NULL;
    
    // This means there's another content run adjacent; simply advance to it.
    if (SUCCESS == nextRunCaret.MoveToNextRun () && (nextRun = nextRunCaret.GetCurrentRunCP ())->IsContentRun ())
        {
        size_t currentCharacterIndex = m_characterIndex;
        
        this->MoveToNextRun ();

        RunCP newRun = this->GetCurrentRunCP ();
        
        // If we were at the absolute tail of the previous run, simply advancing to the next run is an equivalent position; need to advance one more.
        //  I believe we also need to do this in the general case if were at the right edge of non-content runs (e.g. the right edge of a line break at the end of a TextBlock).
        if ((CARET_BIAS_Trailing == bias) && (currentRun->IsContentRun () || currentCharacterIndex > 0))
            {
            if (newRun->IsAtomic ())
                m_characterIndex = newRun->GetCharacterCount ();
            else
                ++m_characterIndex;
            }
        
        // MoveToNextRun already set m_approachDirection

        return SUCCESS;
        }
    
    // There are some boundary cases we need to now account for:
    //  - Don't position on a non-content run, unless it is the only thing in the line
    //  - Allow on a trailing line that will exist due to a trailing line break, even if index is currently out of bounds
    //  - Allow on a trailing line that will exist due to a trailing paragraph break, even if index is currently out of bounds
    
    // If there is a run, but it's not a content run, advance to the next line. If the current line ends in a line break, stay in the same paragraph.
    if (NULL != dynamic_cast<LineBreakCP>(nextRun) || NULL != dynamic_cast<LineBreakCP>(currentRun))
        {
        if (SUCCESS == this->MoveToNextLine ())
            return SUCCESS;
        
        m_runIndex          = this->GetCurrentLineCP ()->GetRunCount () - 1;
        m_characterIndex    = this->GetCurrentRunCP ()->GetCharacterCount ();
        
        // MoveToNextLine already set m_approachDirection
        
        return SUCCESS;
        }
    else if (NULL != dynamic_cast<ParagraphBreakCP>(nextRun) || NULL != dynamic_cast<ParagraphBreakCP>(currentRun))
        {
        size_t currentCharacterIndex = m_characterIndex;
        
        if (SUCCESS == this->MoveToNextParagraph ())
            {
            // For successive paragraph breaks -- put on a new virtual line.
            if (!currentRun->IsContentRun () && (currentCharacterIndex > 0) && (NULL != nextRun) && !nextRun->IsContentRun ())
                ++m_characterIndex;
            
            // MoveToNextParagraph already set m_approachDirection

            return SUCCESS;
            }
        
        m_runIndex          = this->GetCurrentLineCP ()->GetRunCount () - 1;
        m_characterIndex    = this->GetCurrentRunCP ()->GetCharacterCount ();
        m_approachDirection = CARET_MOTION_DIRECTION_Forward;

        return SUCCESS;
        }
    
    // Even if the bias didn't match above, if we move to the tail, we should.
    if (currentRun->IsContentRun () && (m_characterIndex < currentRun->GetCharacterCount ()))
        {
        ++m_characterIndex;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Forward;
        
        return SUCCESS;
        }
    
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToPreviousInsertPosition (CaretBias bias)
    {
    if (this->IsAtBeginning ())
        return ERROR;
    
    RunCP currentRun = this->GetCurrentRunCP ();
    
    if (currentRun->IsAtomic () && (0 != m_characterIndex))
        {
        m_characterIndex    = 0;
        m_approachDirection = CARET_MOTION_DIRECTION_Backward;
            
        return SUCCESS;
        }
    
    // If inside a run, just like normal; additionally, if bias is trailing, do not allow it to be at the head of the run, unless we are the first run in a line.
    //  Also, need to handle the off-the-end case with the virtual line.
    if ((currentRun->IsContentRun () && m_characterIndex > (((CARET_BIAS_Leading == bias) || (0 == m_runIndex)) ? (size_t)0 : (size_t)1))
        || (!currentRun->IsContentRun () && (CARET_BIAS_Leading == bias) && this->IsOnVirtualLine () && (m_characterIndex > 0)))
        {
        --m_characterIndex;
        
        m_approachDirection = CARET_MOTION_DIRECTION_Backward;

        return SUCCESS;
        }
    
    Caret   previousRunCaret    (*this);
    RunCP   previousRun         = NULL;
    
    // This means there's another content run adjacent; simply back up to it.
    if (SUCCESS == previousRunCaret.MoveToPreviousRun () && (previousRun = previousRunCaret.GetCurrentRunCP ())->IsContentRun ())
        {
        size_t currentCharacterIndex = m_characterIndex;
        
        this->MoveToPreviousRun ();
        
        RunCP newRun = this->GetCurrentRunCP ();
        
        // If we were at the absolute head of the current run, simply backing up to the previous run is an equivalent position; need to back up one more.
        //  I believe we also need to do this in the general case if were at the right edge of non-content runs (e.g. the right edge of a line break at the end of a TextBlock).
        if (((CARET_BIAS_Trailing == bias) && (NULL != currentRun) && (currentRun->IsContentRun () || currentCharacterIndex > 0)))
            {
            m_characterIndex = previousRun->GetCharacterCount ();
            }
        else
            {
            if (newRun->IsAtomic ())
                m_characterIndex = 0;
            else
                m_characterIndex = previousRun->GetCharacterCount () - 1;
            }
        
        // MoveToPreviousRun already set m_approachDirection

        return SUCCESS;
        }
    
    // There are some boundary cases we need to now account for:
    //  - Don't position on a non-content run, unless it is the only thing in the line
    //  - Allow on a trailing line that will exist due to a trailing line break, even if index is currently out of bounds
    //  - Allow on a trailing line that will exist due to a trailing paragraph break, even if index is currently out of bounds
    
    // If there is a run, but it's not a content run, back up to the next line.
    if (NULL != dynamic_cast<LineBreakCP>(previousRun) || NULL != dynamic_cast<ParagraphBreakCP>(previousRun))
        {
        // For successive paragraph breaks -- we were on a virtual line, so only need to move back to the end of the current line.
        if (this->IsOnVirtualLine ())
            {
            --m_characterIndex;
            
            m_approachDirection = CARET_MOTION_DIRECTION_Backward;

            return SUCCESS;
            }
        
        this->MoveToPreviousRun ();
        
        if ((CARET_BIAS_Trailing == bias) && (m_runIndex > 0))
            {
            this->MoveToPreviousRun ();
            
            RunCP currRun = this->GetCurrentRunCP ();
            if (NULL != currRun)
                m_characterIndex = currRun->GetCharacterCount ();
            }
        
        // MoveToPreviousRun already set m_approachDirection

        return SUCCESS;
        }
    
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToFrontOfLine ()
    {
    m_characterIndex = m_runIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Backward;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToBackOfLine ()
    {
    LineCP currLine = this->GetCurrentLineCP ();
    if (NULL == currLine)
        return ERROR;
    
    size_t lastMeaningfulRunIndex = currLine->GetRunCount () - 1;
    
    while (lastMeaningfulRunIndex > 0)
        {
        if (currLine->GetRun (lastMeaningfulRunIndex)->IsContentRun ())
            break;
        
        lastMeaningfulRunIndex--;
        }
    
    m_runIndex = lastMeaningfulRunIndex;
    
    RunCP newRun = currLine->GetRun (m_runIndex);
    
    m_characterIndex = newRun->IsContentRun () ? newRun->GetCharacterCount () : newRun->GetCharacterCount () - 1;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Forward;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToFrontOfParagraph ()
    {
    m_characterIndex = m_runIndex = m_lineIndex = 0;
    
    m_approachDirection = CARET_MOTION_DIRECTION_Backward;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToBackOfParagraph ()
    {
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL == currParagraph)
        return ERROR;
    
    m_lineIndex = (currParagraph->GetLineCount () - 1);
    this->MoveToBackOfLine ();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
ParagraphP Caret::GetCurrentParagraphP () const
    {
    return m_textBlock->GetParagraph (m_paragraphIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
ParagraphCP Caret::GetCurrentParagraphCP () const
    {
    return this->GetCurrentParagraphP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
LineP Caret::GetCurrentLineP () const
    {
    ParagraphCP paragraph = this->GetCurrentParagraphCP ();
    return (NULL == paragraph) ? NULL : paragraph->GetLine (m_lineIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
LineCP Caret::GetCurrentLineCP () const
    {
    return this->GetCurrentLineP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
RunP Caret::GetCurrentRunP () const
    {
    LineCP line = this->GetCurrentLineCP ();
    return (NULL == line) ? NULL : line->GetRun (m_runIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
RunCP Caret::GetCurrentRunCP () const
    {
    return this->GetCurrentRunP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
WChar Caret::GetPreviousCharacter () const
    {
    if (this->IsAtBeginning ())
        return 0;

    if (0 == m_characterIndex)
        {
        Caret hintedCaret = *this;
        
        if (SUCCESS != hintedCaret.MoveToPreviousCharacter ())
            return 0;
        
        return hintedCaret.GetNextCharacter ();
        }

    return this->GetCurrentRunCP ()->GetCharacter (m_characterIndex - 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
WChar Caret::GetNextCharacter () const
    {
    if (this->IsAtEnd ())
        return 0;

    RunCP run = this->GetCurrentRunCP ();

    if (m_characterIndex >= run->GetCharacterCount ())
        {
        Caret hintedCaret = *this;
        
        if (SUCCESS != hintedCaret.MoveToNextCharacter ())
            return 0;
        
        return hintedCaret.GetNextCharacter ();
        }

    return run->GetCharacter (m_characterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2009
//---------------------------------------------------------------------------------------
bool Caret::IsValid () const
    {
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL == currParagraph)
        return false;
    
    LineCP currLine = currParagraph->GetLine (m_lineIndex);
    if (NULL == currLine)
        return false;
    
    RunCP currRun = currLine->GetRun (m_runIndex);
    if (NULL == currRun)
        return false;
    
    return m_characterIndex <= currRun->GetCharacterCount ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphIterator Caret::CreateParagraphBegin () const
    {
    return ParagraphIterator(*m_textBlock, m_paragraphIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphIterator Caret::CreateParagraphEnd () const
    {
    if (m_textBlock->IsEmpty ())
        return ParagraphIterator (*m_textBlock, 0);
    
    return ParagraphIterator(*m_textBlock, m_paragraphIndex + 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineIterator Caret::CreateLineBegin () const
    {
    return LineIterator (*m_textBlock, m_paragraphIndex, m_lineIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineIterator Caret::CreateLineEnd () const
    {
    if (m_textBlock->IsEmpty ())
        return LineIterator (*m_textBlock, 0, 0);
    
    Caret caret (*this);
    
    if (this->IsAtEnd () || SUCCESS != caret.MoveToNextLine ())
        return LineIterator (*m_textBlock, m_paragraphIndex + 1, 0);
    
    return LineIterator (*m_textBlock, caret.m_paragraphIndex, caret.m_lineIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIterator Caret::CreateRunBegin () const
    {
    return RunIterator (*m_textBlock, m_paragraphIndex, m_lineIndex, m_runIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIterator Caret::CreateRunEnd () const
    {
    if (m_textBlock->IsEmpty ())
        return RunIterator (*m_textBlock, 0, 0, 0);
    
    Caret caret (*this);
    
    if (this->IsAtEnd () || SUCCESS != caret.MoveToNextRun ())
        return RunIterator (*m_textBlock, m_paragraphIndex + 1, 0, 0);
    
    return RunIterator (*m_textBlock, caret.m_paragraphIndex, caret.m_lineIndex, caret.m_runIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2010
//---------------------------------------------------------------------------------------
DgnTextStylePtr Caret::CreateEffectiveTextStyle () const
    {
    RunCP                   run         = this->GetCurrentRunCP ();
    RunPropertiesCR         runProps    = (NULL != run) ? run->GetProperties () : m_textBlock->GetRunPropertiesForAdd ();
    ParagraphCP             paragraph   = this->GetCurrentParagraphCP ();
    ParagraphPropertiesCR   paraProps   = (NULL != paragraph) ? paragraph->GetProperties () : m_textBlock->GetParagraphPropertiesForAdd ();
    TextBlockPropertiesCR   tbProps     = m_textBlock->GetProperties ();
    
    DgnTextStylePtr style;
    if (runProps.HasTextStyle ())
        style = runProps.GetTextStyleInFile ();
    else
        style = DgnTextStyle::Create (m_textBlock->GetDgnModelR ().GetDgnProject());
    
    runProps.ToStyle (*style);
    paraProps.ToStyle (*style);
    tbProps.ToStyle (*style);
    
    return style;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2010
//---------------------------------------------------------------------------------------
bool Caret::IsOnVirtualLine () const
    {
    RunCP caretRun = this->GetCurrentRunCP ();
    
    return ((NULL != caretRun) && !caretRun->IsContentRun () && (m_characterIndex > 0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2010
//---------------------------------------------------------------------------------------
void Caret::EnforceCaretOrder (CaretR first, CaretR second)
    {
    if (first.IsBeforeOrAt (second))
        return;
    
    std::swap (first, second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
void Caret::Remap (TextBlockCR rhsTextBlock)
    {
    if (m_textBlock == &rhsTextBlock)
        return;
    
    Caret oldInstanceCopy = *this;
    m_textBlock = &rhsTextBlock;
    
    if (this->IsValid ())
        return;
    
    *this = m_textBlock->Begin ();

    for (Caret charIter = oldInstanceCopy.m_textBlock->Begin (); charIter.IsBefore (oldInstanceCopy);)
        {
        if (SUCCESS != this->MoveToNextCharacter () || SUCCESS != charIter.MoveToNextCharacter ())
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
size_t Caret::ComputeGlobalCharacterOffset () const
    {
    size_t charOffset = 0;
    
    for (Caret charIter = m_textBlock->Begin (); charIter.IsBefore (*this); charIter.MoveToNextCharacter ())
        ++charOffset;

    return charOffset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
void Caret::GoToGlobalCharacterIndex (size_t charOffset)
    {
    m_paragraphIndex    = 0;
    m_lineIndex         = 0;
    m_runIndex          = 0;
    m_characterIndex    = 0;
    
    for (size_t i = 0; i < charOffset; ++i)
        this->MoveToNextCharacter ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
ParagraphPropertiesCR Caret::GetEffectiveParagraphProperties () const
    {
    ParagraphCP currParagraph = this->GetCurrentParagraphCP ();
    if (NULL != currParagraph)
        return currParagraph->GetProperties ();

    return m_textBlock->GetParagraphPropertiesForAdd ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2010
//---------------------------------------------------------------------------------------
RunPropertiesCR Caret::GetEffectiveRunProperties () const
    {
    RunCP currRun = this->GetCurrentRunCP ();
    if (NULL != currRun)
        return currRun->GetProperties ();

    return m_textBlock->GetRunPropertiesForAdd ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
static WString lineToStringNoFractions (LineCR line)
    {
    TextBlockToStringOptionsPtr options = TextBlockToStringOptions::CreateDefault ();

    // Wants to be a script-agnostic printable character... '_' seems like a decent choice...
    options->SetAtomicRunContentSubstituteChar (L'_');

    // Also need to maintain a 1:1 relationship with characters, so don't expand fractions.
    options->SetShouldExpandRscFractions (false);

    WString str;

    for (size_t iRun = 0; iRun < line.GetRunCount (); ++iRun)
        str.append (line.GetRun (iRun)->ToString (0, WString::npos, *options).c_str ());
    
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
static BentleyStatus moveToPreviousWord (CaretR caret, WordBoundaryReason boundaryType)
    {
    if (caret.IsAtBeginning ())
        return ERROR;

    bool    isAtFrontOfLine = ((0 == caret.GetRunIndex ()) && (0 == caret.GetCharacterIndex ()));
    LineCP  currLine        = NULL;
    
    if (isAtFrontOfLine || (NULL == (currLine = caret.GetCurrentLineCP ())))
        {
        if (SUCCESS != caret.MoveToPreviousInsertPosition (CARET_BIAS_Trailing))
            return ERROR;

        caret.RetreatPastWhiteSpace ();
        return SUCCESS;
        }
    
    WString lineAsString    = lineToStringNoFractions (*currLine);
    size_t  charOffset      = 0;

    Caret charIter (caret);
    while ((SUCCESS == charIter.MoveToPreviousCharacter ()) && (charIter.GetCurrentLineCP () == currLine))
        ++charOffset;

    size_t previousWordOffset = WordBoundaryServices::FindPreviousWordBoundary (boundaryType, lineAsString.c_str (), lineAsString.length (), charOffset);
    
    for (size_t i = charOffset; i > previousWordOffset; --i)
        {
        if (SUCCESS != caret.MoveToPreviousCharacter ())
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
static BentleyStatus moveToNextWord (CaretR caret, WordBoundaryReason boundaryType)
    {
    if (caret.IsAtEnd ())
        return ERROR;
    
    RunCP   currRun     = caret.GetCurrentRunCP ();
    LineCP  currLine    = caret.GetCurrentLineCP ();

    // Detect errors and end-of-line.
    if ((NULL == currLine) || (NULL == currRun))
        return caret.MoveToNextInsertPosition (CARET_BIAS_Leading);

    // If we are on a paragraph- or line-break, never allow it to advance to the end of the current run like logic below will allow; simply move beyond it.
    if ((NULL != dynamic_cast<ParagraphBreakCP>(caret.GetCurrentRunCP ())) || (NULL != dynamic_cast<LineBreakCP>(caret.GetCurrentRunCP ())))
        return caret.MoveToNextInsertPosition (CARET_BIAS_Leading);
    
    size_t  currLineRunCount    = currLine->GetRunCount ();
    bool    isLastRun           = (caret.GetRunIndex () >= (currLineRunCount - 1));
    bool    isNextRunCrLf       = ((caret.GetRunIndex () == (currLineRunCount - 2)) && ((NULL != dynamic_cast<ParagraphBreakCP>(currLine->GetRun (currLineRunCount - 1))) || (NULL != dynamic_cast<LineBreakCP>(currLine->GetRun (currLineRunCount - 1)))));
        
    if ((caret.GetCharacterIndex () >= currRun->GetCharacterCount ()) && (isLastRun || isNextRunCrLf))
        return caret.MoveToNextInsertPosition (CARET_BIAS_Leading);
    
    WString lineAsString    = lineToStringNoFractions (*currLine);
    size_t  charOffset      = 0;

    Caret charIter (caret);
    while ((SUCCESS == charIter.MoveToPreviousCharacter ()) && (charIter.GetCurrentLineCP () == currLine))
        ++charOffset;

    size_t nextWordOffset = WordBoundaryServices::FindNextWordBoundary (boundaryType, lineAsString.c_str (), lineAsString.length (), charOffset);
    
    for (size_t i = charOffset; i < nextWordOffset; ++i)
        {
        if (SUCCESS != caret.MoveToNextCharacter ())
            return ERROR;
        }

    if (caret.GetCurrentRunCP ()->IsAtomic ())
        caret.SetCharacterIndex (0);

    if (caret.GetCurrentLineCP () != currLine)
        {
        caret.MoveToPreviousLine ();
        caret.MoveToBackOfLine ();
        }
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToFrontOfCurrentWord ()
    {
    return moveToPreviousWord (*this, WordBoundaryReason::CaretPositioning);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToFrontOfNextWord ()
    {
    return moveToNextWord (*this, WordBoundaryReason::CaretPositioning);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToFrontOfCurrentWholeWord ()
    {
    return moveToPreviousWord (*this, WordBoundaryReason::FindingWords);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
BentleyStatus Caret::MoveToBackOfCurrentWholeWord ()
    {
    return moveToNextWord (*this, WordBoundaryReason::FindingWords);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
bool Caret::IsAtWholeWordBoundary ()
    {
    if (this->IsAtBeginning () || this->IsAtEnd ())
        return true;
    
    LineCP currLine = this->GetCurrentLineCP ();
    if (NULL == currLine)
        return true;
    
    WString lineAsString    = lineToStringNoFractions (*currLine);
    size_t  charOffset      = 0;
    Caret   charIter        (*this);
    
    while ((SUCCESS == charIter.MoveToPreviousCharacter ()) && (charIter.GetCurrentLineCP () == currLine))
        ++charOffset;

    if ((0 == charOffset) || (lineAsString.size () == charOffset))
        return true;
    
    return WordBoundaryServices::IsAtWordBoundary (lineAsString.c_str (), lineAsString.length (), charOffset);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool Caret::IsAtValidInsertPosition () const
    {
    RunCP currRun = this->GetCurrentRunCP ();
    if (NULL == currRun)
        return false;

    if (!currRun->IsAtomic ())
        return true;
    
    return ((0 == m_characterIndex) || (currRun->GetCharacterCount () == m_characterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void Caret::AdvancePastWhiteSpace ()
    {
    while (0x0020 == this->GetNextCharacter () && !this->GetCurrentRunCP ()->IsAtomic ())
        if (SUCCESS != this->MoveToNextInsertPosition (CARET_BIAS_Trailing))
            break;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void Caret::RetreatPastWhiteSpace ()
    {
    while (0x0020 == this->GetPreviousCharacter ())
        {
        Caret peek (*this);
        if ((SUCCESS != peek.MoveToPreviousInsertPosition (CARET_BIAS_Leading)) || peek.GetCurrentRunCP ()->IsAtomic ())
            break;
        
        *this = peek;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
void Caret::RoundToInsertPosition ()
    {
    if (this->IsAtValidInsertPosition ())
        return;
        
    DPoint3d    proposedCaretLocation;
    DVec3d      proposedCaretDirection;
    this->GetTextBlock ().ComputeCaretParameters (proposedCaretLocation, proposedCaretDirection, *this);
        
    Caret lhsBoundary = *this;
        
    lhsBoundary.MoveToPreviousInsertPosition (CARET_BIAS_Leading);
        
    DPoint3d    lhsBoundaryLocation;
    DVec3d      lhsBoundaryDirection;
    this->GetTextBlock ().ComputeCaretParameters (lhsBoundaryLocation, lhsBoundaryDirection, lhsBoundary);
        
    Caret rhsBoundary = *this;
        
    rhsBoundary.MoveToNextInsertPosition (CARET_BIAS_Trailing);
        
    DPoint3d    rhsBoundaryLocation;
    DVec3d      rhsBoundaryDirection;
    this->GetTextBlock ().ComputeCaretParameters (rhsBoundaryLocation, rhsBoundaryDirection, rhsBoundary);
        
    double  distToLhsBoundary   = proposedCaretLocation.Distance (lhsBoundaryLocation);
    double  distToRhsBoundary   = proposedCaretLocation.Distance (rhsBoundaryLocation);
        
    if (distToLhsBoundary <= distToRhsBoundary)
        {
        *this = lhsBoundary;
        return;
        }
        
    *this = rhsBoundary;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/2011
//---------------------------------------------------------------------------------------
void Caret::RoundToRun ()
    {
    if (this->IsAtEnd ())
        return;
    
    RunCP currentRun = this->GetCurrentRunCP ();
    if (NULL == currentRun)
        return;
    
    if (m_characterIndex < currentRun->GetCharacterCount ())
        return;
    
    this->MoveToNextRun ();
    }
