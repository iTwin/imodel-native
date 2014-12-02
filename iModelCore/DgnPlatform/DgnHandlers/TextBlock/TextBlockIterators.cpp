/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlockIterators.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- ParagraphIterator --------------------------------------------------------------------------------------------------------- ParagraphIterator --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphIterator::ParagraphIterator (TextBlockCR textBlock, size_t paragraphIndex) :
    m_textBlock         (textBlock),
    m_paragraphIndex    (paragraphIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphCR ParagraphIterator::operator*    () const                        { return *m_textBlock.GetParagraph (m_paragraphIndex); }
bool        ParagraphIterator::operator==   (ParagraphIteratorCR rhs) const { return (&m_textBlock == &rhs.m_textBlock) && (m_paragraphIndex == rhs.m_paragraphIndex); }
bool        ParagraphIterator::operator!=   (ParagraphIteratorCR rhs) const { return !(*this == rhs); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphIteratorR ParagraphIterator::operator++ ()
    {
    ++m_paragraphIndex;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphIteratorR ParagraphIterator::operator-- ()
    {
    --m_paragraphIndex;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIterator ParagraphIterator::CreateRunBegin () const
    {
    Caret caret = m_textBlock.Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    return caret.CreateRunBegin ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIterator ParagraphIterator::CreateRunEnd () const
    {
    Caret caret = m_textBlock.Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    
    while (SUCCESS == caret.MoveToNextRunInParagraph ())
        ;
    
    return caret.CreateRunEnd ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
CaretPtr ParagraphIterator::ToCaret () const
    {
    CaretPtr caret = m_textBlock.CreateStartCaret ();
    caret->SetParagraphIndex (m_paragraphIndex);
    
    if (!caret->IsValid ())
        return NULL;
    
    return caret;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- ParagraphRange --------------------------------------------------------------------------------------------------------------- ParagraphRange --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
                    ParagraphRange::ParagraphRange  (TextBlockCR textBlock)         : m_begin (textBlock.CreateStartCaret ()), m_end (textBlock.CreateEndCaret ()) { }
CaretCR             ParagraphRange::GetStartCaret   () const                        { return *m_begin; }
CaretCR             ParagraphRange::GetEndCaret     () const                        { return *m_end; }
ParagraphIterator   ParagraphRange::begin           () const                        { return m_begin->CreateParagraphBegin (); }
ParagraphIterator   ParagraphRange::end             () const                        { return m_end->CreateParagraphEnd (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
ParagraphRange::ParagraphRange (CaretCR begin, CaretCR end)
    {
    if (begin.IsBefore (end))
        {
        m_begin = begin.Clone ();
        m_end   = end.Clone ();
        return;
        }
    
    m_begin = end.Clone ();
    m_end   = begin.Clone ();
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- LineIterator ------------------------------------------------------------------------------------------------------------------- LineIterator --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineIterator::LineIterator (TextBlockCR textBlock, size_t paragraphIndex, size_t lineIndex) :
    m_textBlock         (textBlock),
    m_paragraphIndex    (paragraphIndex),
    m_lineIndex         (lineIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineCR      LineIterator::operator*             () const                    { return *m_textBlock.GetParagraph (m_paragraphIndex)->GetLine (m_lineIndex); }
bool        LineIterator::operator==            (LineIteratorCR rhs) const  { return (&m_textBlock == &rhs.m_textBlock) && (m_paragraphIndex == rhs.m_paragraphIndex) && (m_lineIndex == rhs.m_lineIndex); }
bool        LineIterator::operator!=            (LineIteratorCR rhs) const  { return !(*this == rhs); }
ParagraphCP LineIterator::GetCurrentParagraphCP () const                    { return m_textBlock.GetParagraph (m_paragraphIndex); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineIteratorR LineIterator::operator++ ()
    {
    Caret caret = m_textBlock.Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    caret.SetLineIndex (m_lineIndex);
    
    if (ERROR == caret.MoveToNextLine ())
        {
        ++m_paragraphIndex;
        m_lineIndex = 0;
        }
    else
        {
        m_paragraphIndex    = caret.GetParagraphIndex ();
        m_lineIndex         = caret.GetLineIndex ();
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineIteratorR LineIterator::operator-- ()
    {
    Caret caret = m_textBlock.Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    caret.SetLineIndex (m_lineIndex);
    
    if (ERROR == caret.MoveToPreviousRun ())
        {
        m_paragraphIndex    = 0;
        m_lineIndex         = 0;
        }
    else
        {
        m_paragraphIndex    = caret.GetParagraphIndex ();
        m_lineIndex         = caret.GetLineIndex ();
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
CaretPtr LineIterator::ToCaret () const
    {
    CaretPtr caret = m_textBlock.CreateStartCaret ();
    caret->SetParagraphIndex (m_paragraphIndex);
    caret->SetLineIndex (m_lineIndex);
    
    if (!caret->IsValid ())
        return NULL;
    
    return caret;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- LineRange ------------------------------------------------------------------------------------------------------------------------- LineRange --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
                LineRange::LineRange        (TextBlockCR textBlock)         : m_begin (textBlock.CreateStartCaret ()), m_end (textBlock.CreateEndCaret ()) { }
CaretCR         LineRange::GetStartCaret    () const                        { return *m_begin; }
CaretCR         LineRange::GetEndCaret      () const                        { return *m_end; }
LineIterator    LineRange::begin            () const                        { return m_begin->CreateLineBegin (); }
LineIterator    LineRange::end              () const                        { return m_end->CreateLineEnd (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
LineRange::LineRange (CaretCR begin, CaretCR end)
    {
    if (begin.IsBefore (end))
        {
        m_begin = begin.Clone ();
        m_end   = end.Clone ();
        return;
        }
    
    m_begin = end.Clone ();
    m_end   = begin.Clone ();
    }


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- RunIterator --------------------------------------------------------------------------------------------------------------------- RunIterator --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIterator::RunIterator (TextBlockCR textBlock, size_t paragraphIndex, size_t lineIndex, size_t runIndex) :
    m_textBlock         (&textBlock),
    m_paragraphIndex    (paragraphIndex),
    m_lineIndex         (lineIndex),
    m_runIndex          (runIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
RunIterator::RunIterator (RunIteratorCR rhs)
    {
    *this = rhs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunCR       RunIterator::operator*              () const                    { return *m_textBlock->GetParagraph (m_paragraphIndex)->GetLine (m_lineIndex)->GetRun (m_runIndex); }
bool        RunIterator::operator==             (RunIteratorCR rhs) const   { return (m_textBlock == rhs.m_textBlock) && (m_paragraphIndex == rhs.m_paragraphIndex) && (m_lineIndex == rhs.m_lineIndex) && (m_runIndex == rhs.m_runIndex); }
bool        RunIterator::operator!=             (RunIteratorCR rhs) const   { return !(*this == rhs); }
ParagraphCP RunIterator::GetCurrentParagraphCP  () const                    { return m_textBlock->GetParagraph (m_paragraphIndex); }
LineCP      RunIterator::GetCurrentLineCP       () const                    { return m_textBlock->GetParagraph (m_paragraphIndex)->GetLine (m_lineIndex); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
RunIteratorR RunIterator::operator= (RunIteratorCR rhs)
    {
    if (this != &rhs)
        {
        m_textBlock         = rhs.m_textBlock;
        m_paragraphIndex    = rhs.m_paragraphIndex;
        m_lineIndex         = rhs.m_lineIndex;
        m_runIndex          = rhs.m_runIndex;
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIteratorR RunIterator::operator++ ()
    {
    Caret caret = m_textBlock->Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    caret.SetLineIndex (m_lineIndex);
    caret.SetRunIndex (m_runIndex);
    
    if (ERROR == caret.MoveToNextRun ())
        {
        ++m_paragraphIndex;
        m_lineIndex = 0;
        m_runIndex  = 0;
        }
    else
        {
        m_paragraphIndex    = caret.GetParagraphIndex ();
        m_lineIndex         = caret.GetLineIndex ();
        m_runIndex          = caret.GetRunIndex ();
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunIteratorR RunIterator::operator-- ()
    {
    Caret caret = m_textBlock->Begin ();
    caret.SetParagraphIndex (m_paragraphIndex);
    caret.SetLineIndex (m_lineIndex);
    caret.SetRunIndex (m_runIndex);
    
    if (ERROR == caret.MoveToPreviousRun ())
        {
        m_paragraphIndex    = 0;
        m_lineIndex         = 0;
        m_runIndex          = 0;
        }
    else
        {
        m_paragraphIndex    = caret.GetParagraphIndex ();
        m_lineIndex         = caret.GetLineIndex ();
        m_runIndex          = caret.GetRunIndex ();
        }
    
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
CaretPtr RunIterator::ToCaret () const
    {
    CaretPtr caret = m_textBlock->CreateStartCaret ();
    caret->SetParagraphIndex (m_paragraphIndex);
    caret->SetLineIndex (m_lineIndex);
    caret->SetRunIndex (m_runIndex);
    
    if (!caret->IsValid ())
        return NULL;
    
    return caret;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- RunRange --------------------------------------------------------------------------------------------------------------------------- RunRange --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
            RunRange::RunRange      (TextBlockCR textBlock)         : m_begin (textBlock.CreateStartCaret ()), m_end (textBlock.CreateEndCaret ()) { }
CaretCR     RunRange::GetStartCaret () const                        { return *m_begin; }
CaretCR     RunRange::GetEndCaret   () const                        { return *m_end; }
RunIterator RunRange::begin         () const                        { return m_begin->CreateRunBegin (); }
RunIterator RunRange::end           () const                        { return m_end->CreateRunEnd (); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
RunRange::RunRange (CaretCR begin, CaretCR end)
    {
    if (begin.IsBefore (end))
        {
        m_begin = begin.Clone ();
        m_end   = end.Clone ();
        return;
        }
    
    m_begin = end.Clone ();
    m_end   = begin.Clone ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
RunRange::RunRange (TextBlockCR textBlock, ParagraphCR paragraph)
    {
    m_begin = textBlock.CreateStartCaret ();

    while (m_begin->GetCurrentParagraphCP () != &paragraph)
        {
        if (SUCCESS != m_begin->MoveToNextParagraph ())
            {
            BeAssert (false && L"Requested paragrah was not in the provided TextBlock.");
            
            m_begin = textBlock.CreateEndCaret ();
            m_end   = textBlock.CreateEndCaret ();
            
            return;
            }
        }
    
    m_end = m_begin->Clone ();
    m_end->MoveToBackOfParagraph ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
RunRange::RunRange (TextBlockCR textBlock, LineCR line)
    {
    m_begin = textBlock.CreateStartCaret ();

    while (m_begin->GetCurrentLineCP () != &line)
        {
        if (SUCCESS != m_begin->MoveToNextLine ())
            {
            BeAssert (false && L"Requested line was not in the provided TextBlock.");
            
            m_begin = textBlock.CreateEndCaret ();
            m_end   = textBlock.CreateEndCaret ();
            
            return;
            }
        }
    
    m_end = m_begin->Clone ();
    m_end->MoveToBackOfLine ();
    }
