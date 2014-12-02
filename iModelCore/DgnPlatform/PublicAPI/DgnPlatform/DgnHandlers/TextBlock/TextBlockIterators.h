/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextBlockIterators.h $
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

//=======================================================================================
//! An STL iterator-like wrapper around Caret, that allows you to easily iterate paragraphs.
//! See notes on Caret as to why this is helpful.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct ParagraphIterator : std::iterator<std::forward_iterator_tag, Paragraph const>
    {
    private: friend struct Caret;

    private:    TextBlockCR m_textBlock;
    private:    size_t      m_paragraphIndex;

    private:                                            ParagraphIterator   (TextBlockCR, size_t paragraphIndex);

    public:     DGNPLATFORM_EXPORT  ParagraphIteratorR  operator++          ();
    public:     DGNPLATFORM_EXPORT  ParagraphIteratorR  operator--          ();
    public:     DGNPLATFORM_EXPORT  ParagraphCR         operator*           () const;
    public:     DGNPLATFORM_EXPORT  bool                operator==          (ParagraphIteratorCR rhs) const;
    public:     DGNPLATFORM_EXPORT  bool                operator!=          (ParagraphIteratorCR rhs) const;

    //! Creates a RunIterator pointing at the first run in the current paragraph.
    public: DGNPLATFORM_EXPORT RunIterator CreateRunBegin () const;

    //! Creates a RunIterator pointing just past the last run in the current paragraph.
    public: DGNPLATFORM_EXPORT RunIterator CreateRunEnd () const;

    //! Attempts to create a Caret representing this iterator's paragraph.
    //! @return Can return NULL if this is an end iterator (one past the last valid).
    public: DGNPLATFORM_EXPORT CaretPtr ToCaret () const;

    }; // ParagraphIterator

//=======================================================================================
//! Facilitates writing for-each loops for ParagraphIterator.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct ParagraphRange : public NonCopyableClass
    {
    public: typedef ParagraphIterator   const_iterator;
    public: typedef const_iterator      iterator;       // Only const iteration is possible.

    private:    CaretPtr    m_begin;
    private:    CaretPtr    m_end;

    //! Creates a range of all paragraphs in the given TextBlock.
    public: DGNPLATFORM_EXPORT explicit ParagraphRange (TextBlockCR);

    //! Creates a range of all paragraphs between the two Carets, inclusive on both ends.
    public: DGNPLATFORM_EXPORT ParagraphRange (CaretCR begin, CaretCR end);

    //! Gets the start caret used to create this range.
    public: DGNPLATFORM_EXPORT CaretCR GetStartCaret () const;

    //! Gets the end caret used to create this range.
    public: DGNPLATFORM_EXPORT CaretCR GetEndCaret () const;

    //! Creates a ParagraphIterator at the start caret location's paragraph. The resulting iterator is meant to be inclusive of that paragraph.
    public: DGNPLATFORM_EXPORT ParagraphIterator begin () const;

    //! Creates a ParagraphIterator just beyond the end caret location's paragraph. The resulting iterator is meant to be exclusive of that paragraph.
    public: DGNPLATFORM_EXPORT ParagraphIterator end () const;

    }; // ParagraphRange

//__PUBLISH_SECTION_END__

//=======================================================================================
//! An STL iterator-like wrapper around Caret, that allows you to iterate lines.
//! See notes on Caret as to why this is helpful.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct LineIterator : std::iterator<std::forward_iterator_tag, Line const>
    {
    private: friend struct Caret;

    private:    TextBlockCR m_textBlock;
    private:    size_t      m_paragraphIndex;
    private:    size_t      m_lineIndex;

    private:                                        LineIterator            (TextBlockCR, size_t paragraphIndex, size_t lineIndex);

    public:     DGNPLATFORM_EXPORT  LineIteratorR   operator++              ();
    public:     DGNPLATFORM_EXPORT  LineIteratorR   operator--              ();
    public:     DGNPLATFORM_EXPORT  LineCR          operator*               () const;
    public:     DGNPLATFORM_EXPORT  bool            operator==              (LineIteratorCR rhs) const;
    public:     DGNPLATFORM_EXPORT  bool            operator!=              (LineIteratorCR rhs) const;
    public:     DGNPLATFORM_EXPORT  ParagraphCP     GetCurrentParagraphCP   () const;
    public:     DGNPLATFORM_EXPORT  CaretPtr        ToCaret                 () const;

    }; // LineIterator

//=======================================================================================
//! Facilitates writing for-each loops for LineIterator.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct LineRange : public NonCopyableClass
    {
    public: typedef LineIterator    const_iterator;
    public: typedef const_iterator  iterator;       // Only const iteration is possible.

    private:    CaretPtr    m_begin;
    private:    CaretPtr    m_end;

    public: DGNPLATFORM_EXPORT  explicit                    LineRange       (TextBlockCR);
    public: DGNPLATFORM_EXPORT                              LineRange       (CaretCR begin, CaretCR end);
    public: DGNPLATFORM_EXPORT              CaretCR         GetStartCaret   () const;
    public: DGNPLATFORM_EXPORT              CaretCR         GetEndCaret     () const;
    public: DGNPLATFORM_EXPORT              LineIterator    begin           () const;
    public: DGNPLATFORM_EXPORT              LineIterator    end             () const;

    }; // LineRange

//__PUBLISH_SECTION_START__

//=======================================================================================
//! An STL iterator-like wrapper around Caret, that allows you to iterate runs.
//! See notes on Caret as to why this is helpful.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct RunIterator : std::iterator<std::forward_iterator_tag, Run const>
    {
    private: friend struct Caret;

    private:    TextBlockCP m_textBlock;
    private:    size_t      m_paragraphIndex;
    private:    size_t      m_lineIndex;
    private:    size_t      m_runIndex;

    private:                                        RunIterator         (TextBlockCR, size_t paragraphIndex, size_t lineIndex, size_t runIndex);
    public:     DGNPLATFORM_EXPORT                  RunIterator         (RunIteratorCR);

    public:     DGNPLATFORM_EXPORT  RunIteratorR    operator=           (RunIteratorCR);
    public:     DGNPLATFORM_EXPORT  RunIteratorR    operator++          ();
    public:     DGNPLATFORM_EXPORT  RunIteratorR    operator--          ();
    public:     DGNPLATFORM_EXPORT  RunCR           operator*           () const;
    public:     DGNPLATFORM_EXPORT  bool            operator==          (RunIteratorCR rhs) const;
    public:     DGNPLATFORM_EXPORT  bool            operator!=          (RunIteratorCR rhs) const;

//__PUBLISH_SECTION_END__
    public:     DGNPLATFORM_EXPORT  LineCP          GetCurrentLineCP    () const;
//__PUBLISH_SECTION_START__

    //! Attempts to get the Paragraph that the current run sits in.
    //! @return Can return NULL if this is an end iterator (one past the last valid).
    public: DGNPLATFORM_EXPORT ParagraphCP GetCurrentParagraphCP () const;

    //! Attempts to create a Caret representing this iterator's paragraph.
    //! @return Can return NULL if this is an end iterator (one past the last valid).
    public: DGNPLATFORM_EXPORT CaretPtr ToCaret () const;

    }; // RunIterator

//=======================================================================================
//! Facilitates writing for-each loops for RunIterator.
// @bsiclass                                                    Jeff.Marker     02/2010
//=======================================================================================
struct RunRange : public NonCopyableClass
    {
    public: typedef RunIterator     const_iterator;
    public: typedef const_iterator  iterator;       // Only const iteration is possible.

    private:    CaretPtr    m_begin;
    private:    CaretPtr    m_end;

    //! Creates a range of all runs in the given TextBlock.
    public: DGNPLATFORM_EXPORT explicit RunRange (TextBlockCR);

    //! Creates a range of all runs between the two Carets, inclusive on both ends. If the carets belong to different text blocks, the result is undefined.
    public: DGNPLATFORM_EXPORT RunRange (CaretCR begin, CaretCR end);

    //! Creates a range of all runs in the provided paragraph. If the paragraph is not in the text block, the result is undefined.
    public: RunRange (TextBlockCR, ParagraphCR);

//__PUBLISH_SECTION_END__

    //! Creates a range of all runs in the provided line. If the line is not in the text block, the result is undefined.
    public: RunRange (TextBlockCR, LineCR);

//__PUBLISH_SECTION_START__

    //! Gets the start caret used to create this range.
    public: DGNPLATFORM_EXPORT CaretCR GetStartCaret () const;

    //! Gets the end caret used to create this range.
    public: DGNPLATFORM_EXPORT CaretCR GetEndCaret () const;

    //! Creates a RunIterator at the start caret location's run. The resulting iterator is meant to be inclusive of that run.
    public: DGNPLATFORM_EXPORT RunIterator begin () const;

    //! Creates a RunIterator just beyond the end caret location's run. The resulting iterator is meant to be exclusive of that run.
    public: DGNPLATFORM_EXPORT RunIterator end () const;

    }; // RunRange

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
