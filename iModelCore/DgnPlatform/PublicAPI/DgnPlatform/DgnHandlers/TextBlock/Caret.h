/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/Caret.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "TextAPICommon.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     12/2009
//=======================================================================================
enum CaretBias
    {
    CARET_BIAS_Leading,
    CARET_BIAS_Trailing

    }; // CaretBias

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     06/2010
//=======================================================================================
enum CaretMotionDirection
    {
    CARET_MOTION_DIRECTION_Forward,
    CARET_MOTION_DIRECTION_Backward,
    CARET_MOTION_DIRECTION_VisualLeading,
    CARET_MOTION_DIRECTION_VisualTrailing

    }; // CaretMotionDirection

//__PUBLISH_SECTION_START__

//=======================================================================================
//! Represents an insertable location within a TextBlock. Caret is most technically a character iterator, with facilities for advancing by other DOM elements.
//!
//! Carets are used to indicate a location within a TextBlock; pairs of Carets are used to indicated ranges. Just like in a visual editor, a Caret is positioned between two characters; thus, it has a current Paragraph / Run, but a previous / next character. Note that if you think of the DOM visually, multiple Carets can resolve to the same location; the difference can be which Run you're in (e.g. you can be just past the last character of one Run, or just before the first character of the next Run). While using either of these Carets will be equivalent for content operations, they may be different for formatting operations.<br>
//! <br>
//! Carets are associated with a TextBlock, and should not be shared between multiple instances. In this light, you cannot directly create a Caret, but must ask an instance of a TextBlock to make one for you. Carets can only be moved in increments of Paragraphs, Runs, and characters, and you are not intended to access the internal indices directly.
//!
//! @note A Caret at the beginning will be in the first Paragraph/Run, and just before the first character. A Caret at the end will be in the last Paragraph/Run, and just after the last character.
//! @note To iterate over Paragraphs and Runs, the easiest way is to use ParagraphIterator / ParagraphRange and RunIterator / RunRange instead. As noted above, while a Caret can be positioned just past the last character, it <i>cannot</i> be positioned just past the last Run or Paragraph (the end Caret still sits in a Run). This makes it impossible to write for... loops as you would with other collections. ParagraphIterator and RunIterator can be used to simulate this normal pattern.
//! @note It should be noted that many TextBlock modification operations can modify the internal DOM of the TextBlock, and can thus invalidate Carets. These functions either take Carets by-reference, or return updated Carets so that you can stay in sync.
//
// @bsiclass                                                    Venkat.Kalyan   05/04
//=======================================================================================
struct Caret : public RefCountedBase
    {
//__PUBLISH_SECTION_END__

    private:    friend struct  TextBlock;
    private:    friend struct  Paragraph;
    private:    friend struct  Line;
    private:    friend struct  Run;

    private:    TextBlockCP             m_textBlock;
    private:    size_t                  m_paragraphIndex;
    private:    size_t                  m_lineIndex;
    private:    size_t                  m_runIndex;
    private:    size_t                  m_characterIndex;
    private:    CaretMotionDirection    m_approachDirection;

    private:    DGNPLATFORM_EXPORT  explicit                            Caret                   (TextBlockCR);
    public:     DGNPLATFORM_EXPORT                                      Caret                   (CaretCR);
    public:     DGNPLATFORM_EXPORT              CaretR                  operator=               (CaretCR);

    public:     DGNPLATFORM_EXPORT              TextBlockCR             GetTextBlock            () const;
    private:                                    void                    SetTextBlock            (TextBlockCP);
    public:     DGNPLATFORM_EXPORT              size_t                  GetParagraphIndex       () const;
    public:                                     void                    SetParagraphIndex       (size_t);
    public:     DGNPLATFORM_EXPORT              size_t                  GetLineIndex            () const;
    public:                                     void                    SetLineIndex            (size_t);
    public:     DGNPLATFORM_EXPORT              size_t                  GetRunIndex             () const;
    public:                                     void                    SetRunIndex             (size_t);
    public:     DGNPLATFORM_EXPORT              size_t                  GetCharacterIndex       () const;
    public:                                     void                    SetCharacterIndex       (size_t);
    public:     DGNPLATFORM_EXPORT              CaretMotionDirection    GetApproachDirection    () const;
    public:                                     void                    SetApproachDirection    (CaretMotionDirection);

    public:     DGNPLATFORM_EXPORT              ParagraphP              GetCurrentParagraphP    () const;
    public:     DGNPLATFORM_EXPORT              LineCP                  GetCurrentLineCP        () const;
    public:     DGNPLATFORM_EXPORT              LineP                   GetCurrentLineP         () const;
    public:     DGNPLATFORM_EXPORT              RunP                    GetCurrentRunP          () const;

    public:     DGNPLATFORM_EXPORT              BentleyStatus           MoveToNextLine          ();
    public:     DGNPLATFORM_EXPORT              BentleyStatus           MoveToPreviousLine      ();

    //! Similar to MoveToNextCharacter, but advances to the next user-insertable location; this includes a trailing position on lines, and skipping internal whitespace.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToNextInsertPosition (CaretBias);

    //! Similar to MoveToPreviousCharacter, but retreats to the previous user-insertable location; this includes a trailing position on lines, and skipping internal whitespace.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToPreviousInsertPosition (CaretBias);

    //! Goes to the front of the line (based on settings indices to 0).
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToFrontOfLine ();

    //! Goes to the back of the line (based on being after the last character of the last content run).
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToBackOfLine ();

    //! Goes to the front of the paragraph (based on settings indices to 0).
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToFrontOfParagraph ();

    //! Goes to the back of the paragraph (based on being after the last character of the last content run of the paragraph).
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToBackOfParagraph ();

    //! Determines if this caret points to a valid location in the TextBlock; this includes a trailing position on lines.
    public: DGNPLATFORM_EXPORT bool IsValid () const;

    //! Creates a LineIterator at this location's line. The resulting iterator is meant to be inclusive of the current line.
    public: DGNPLATFORM_EXPORT LineIterator CreateLineBegin () const;

    //! Creates a LineIterator just beyond this location's line. The resulting iterator is meant to be exclusive of the current line.
    public: DGNPLATFORM_EXPORT LineIterator CreateLineEnd () const;

    //! Creates a DgnTextStyle object that represents the style at this instance's current location in the TextBlock.
    public: DGNPLATFORM_EXPORT DgnTextStylePtr CreateEffectiveTextStyle () const;

    //! Virtual lines are special locations in a TextBlock where an editor would show the caret on a new line that doesn't exist yet in the document (but will on next insert).
    public: DGNPLATFORM_EXPORT bool IsOnVirtualLine () const;

    //! Forces 'first' to be before or at 'second'; if this is not the case, the values are swapped.
    public: DGNPLATFORM_EXPORT static void EnforceCaretOrder (CaretR first, CaretR second);

    //! Purposefully un-documented; you should generally never call this. See header file if you think you need further explanation.
    // This method is not very robust, and should only be used under controlled circumstances. For example, assuming that this instance is valid, if you just cloned this instance's TextBlock and have done nothing else to it, you can use this to simply re-assign this instance's TextBlock.
    // This will set this instance's internal TextBlock reference to the provided TextBlock, and will ensure that this instance is then valid (e.g. resolves to a non-NULL run and valid character position). If this basic validity check passes, nothing further is done. Otherwise, it attempts to advance the same number of characters from the source TextBlock into the new one. This means that the position will likely be visually different, except in very specific cases.
    public: DGNPLATFORM_EXPORT void Remap (TextBlockCR);

    //! Compute the number of characters from the beginning of the TextBlock to this instance.
    public: DGNPLATFORM_EXPORT size_t ComputeGlobalCharacterOffset () const;

    //! Advances this instance 'offset' characters from the beginning of the TextBlock.
    public: DGNPLATFORM_EXPORT void GoToGlobalCharacterIndex (size_t offset);

    //! Returns the ParagraphProperties for the current paragraph if valid, or the ParagraphProperties for add of the TextBlock.
    public: DGNPLATFORM_EXPORT ParagraphPropertiesCR GetEffectiveParagraphProperties () const;

    //! Returns the RunProperties for the current run if valid, or the RunProperties for add of the TextBlock.
    public: DGNPLATFORM_EXPORT RunPropertiesCR GetEffectiveRunProperties () const;

    //! Goes to the front of the current "word". A "word" is defined by where Ctrl+Left/Right should go. This will also respect atomic runs (e.g. EDFs), and treat their entire content as a single word.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToFrontOfCurrentWord ();

    //! Goes to the front of the next "word". A "word" is defined by where Ctrl+Left/Right should go. This will also respect atomic runs (e.g. EDFs), and treat their entire content as a single word.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToFrontOfNextWord ();

    //! Goes to the front of the current whole word. A whole word is a coarser unit than MoveToFrontOfCurrentWord, and is suitable for things like spell check or whole word search.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToFrontOfCurrentWholeWord ();

    //! Goes to the back of the current whole word. A whole word is a coarser unit than MoveToFrontOfCurrentWord, and is suitable for things like spell check or whole word search.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToBackOfCurrentWholeWord ();

    //! Deterimes if the caret is between whole words. A whole word is a coarser unit than MoveToFrontOfCurrentWord, and is suitable for things like spell check or whole word search.
    public: DGNPLATFORM_EXPORT bool IsAtWholeWordBoundary ();

    //! Invalid insert positions include being in the middle of atomic runs (e.g. EDFs). These positions are still valid for query operations.
    public: DGNPLATFORM_EXPORT bool IsAtValidInsertPosition () const;

    //! Advances this instance past any in-line whitespace.
    public: DGNPLATFORM_EXPORT void AdvancePastWhiteSpace ();

    //! Retreats this instance past any in-line whitespace.
    public: DGNPLATFORM_EXPORT void RetreatPastWhiteSpace ();

    //! Determines if this caret is at the same location as another (equivalent to ==).
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsAtVisually (CaretCR) const;

    //! Advances or retreats this caret to the nearest insert position. Visual positioning is used to determine whether to go the left-hand side or right-hand side boundary.
    public: DGNPLATFORM_EXPORT void RoundToInsertPosition ();

    //! Attempts to ensure that this caret resides within a run (e.g. it is not dangling). If this caret is not at the end, and it is at the tail of one run, it will be advanced to the head of the next run.
    public: DGNPLATFORM_EXPORT void RoundToRun ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Creates a deep copy of this instance.
    public: DGNPLATFORM_EXPORT CaretPtr Clone () const;

    //! Determines if this caret is at the same location as another.
    //! @note   This only takes into account indices; two carets may have the same indices (i.e. this returns true), but appear differently when rendered in bi-directional text.
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsAt (CaretCR) const;

    //! Determines if this caret is strictly before another (equivalent to <).
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsBefore (CaretCR) const;

    //! Determines if this caret is before or at another (equivalent to <=).
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsBeforeOrAt (CaretCR) const;

    //! Determines if this caret is strictly after another (equivalent to >).
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsAfter (CaretCR) const;

    //! Determines if this caret is after or at another (equivalent to >=).
    //! @note   Assumes the same TextBlock (otherwise the answer is meaningless).
    public: DGNPLATFORM_EXPORT bool IsAfterOrAt (CaretCR) const;

    //! True if this instance is at the beginning of its associated TextBlock.
    //! @note   If it is at the beginning, you cannot move or look previous to the caret.
    public: DGNPLATFORM_EXPORT bool IsAtBeginning () const;

    //! True if this instance is at or beyond the end of its associated TextBlock.
    //! @note   The end is one past the last run, similar to an STL iterator.
    //! @note   If it is at the end, you cannot move or look next to the caret.
    public: DGNPLATFORM_EXPORT bool IsAtEnd () const;

    //! Gets the paragraph that this instance currently sits in.
    public: DGNPLATFORM_EXPORT ParagraphCP GetCurrentParagraphCP () const;

    //! Gets the run that this instance currently sits in.
    public: DGNPLATFORM_EXPORT RunCP GetCurrentRunCP () const;

    //! Gets the character that sits to the left of this caret; can be 0 if the caret is at the beginning.
    public: DGNPLATFORM_EXPORT WChar GetPreviousCharacter () const;

    //! Gets the character that sits to the left of this caret; can be 0 if the caret is at the end.
    public: DGNPLATFORM_EXPORT WChar GetNextCharacter () const;

    //! Advances the caret to the next paragraph; resets the other sub-paragraph indices to zero.
    //! @note   Returns ERROR if this instance is already at the end of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToNextParagraph ();

    //! Advances the caret to the next run, advancing to the next paragraph if necessary; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the end of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToNextRun ();

    //! Advances the caret to the next run in the current paragraph; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the last run of the current paragraph.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToNextRunInParagraph ();

    //! Advances the caret to the next character, advancing to the next run if necessary; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the end of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToNextCharacter ();

    //! Rewinds the caret to the previous paragraph; resets the other sub-paragraph indices to zero.
    //! @note   Returns ERROR if this instance is already at the beginning of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToPreviousParagraph ();

    //! Rewinds the caret to the previous run; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the beginning of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToPreviousRun ();

    //! Rewinds the caret to the previous run in the current paragraph; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the first run of the current paragraph.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToPreviousRunInParagraph ();

    //! Rewinds the caret to the previous character; resets the other sub-run indices to zero.
    //! @note   Returns ERROR if this instance is already at the beginning of the text block.
    public: DGNPLATFORM_EXPORT BentleyStatus MoveToPreviousCharacter ();

    //! Creates a ParagraphIterator at this location's paragraph. The resulting iterator is meant to be inclusive of the current paragraph.
    public: DGNPLATFORM_EXPORT ParagraphIterator CreateParagraphBegin () const;

    //! Creates a ParagraphIterator just beyond this location's paragraph. The resulting iterator is meant to be exclusive of the current paragraph.
    public: DGNPLATFORM_EXPORT ParagraphIterator CreateParagraphEnd () const;

    //! Creates a RunIterator at this location's run. The resulting iterator is meant to be inclusive of the current run.
    public: DGNPLATFORM_EXPORT RunIterator CreateRunBegin () const;

    //! Creates a RunIterator just beyond this location's run. The resulting iterator is meant to be exclusive of the current run.
    public: DGNPLATFORM_EXPORT RunIterator CreateRunEnd () const;

    }; // Caret

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
