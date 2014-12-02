/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlockPlayer.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*=================================================================================**//**
* @bsiclass                                                     Jyoti.Swarup    07/06
+===============+===============+===============+===============+===============+======*/
class TextBlockPlayer
{
private:

    TextBlockCP         m_textBlock;
    ITextListenerP      m_listener;
    bvector<WChar>    m_text;
    RunPropertiesCP     m_curProperties;
    DgnModelP        m_modelRef;

public:

                TextBlockPlayer         (TextBlockCP textBlock, ITextListenerP listener, DgnModelP modelRef);

    DgnProjectP GetDgnProject() const  { return m_modelRef ? &m_modelRef->GetDgnProject() : NULL; }

    void        InsertSpace             ();
    void        FlushText               (RunPropertiesCP prop, bool force);

    void        ExportParagraph         (CaretR caret);
    void        ExportRun               (RunCP run, CaretP caret);
    void        ExportRunElements       (RunCP run, CaretP caret);
    void        ExportPlainCharStream   (CharStreamCP charStream);
    void        ExportEDF               (size_t startIndex, size_t endIndex, EdfCharStreamCR edf);
    bool        ExportStaticFraction    (CharStreamCR charStream, size_t index);
    void        ExportFontChars         (size_t startIndex, size_t endIndex, CharStreamCR charStream);
    void        ExportCharStream        (CharStreamCP charStream, CaretP caret);
    void        ExportTab               (TabCP tab, CaretP caret);
    void        ExportLineBreak         (LineBreakCP lineFeed);

    void        ExportFraction          (FractionCP fraction, CaretP caret);

    void        ExportParagraphBreak    (ParagraphBreakCP lineFeed);

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    void        ExportBeginField        (FieldP pField);
    void        ExportEndField          (FieldP pField);
#endif // BEIJING_DGNPLATFORM_WIP_Fields

    void        ExportBeginFormat       ();
    void        ExportEndFormat         ();

    void        ExportTextBlock         ();

}; // TextBlockPlayer

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
TextBlockPlayer::TextBlockPlayer (TextBlockCP textBlock, ITextListenerP listener, DgnModelP modelRef) :
    m_listener (listener),
    m_textBlock (textBlock),
    m_modelRef (modelRef)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Abeesh.Basheer  08/07
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportRunElements (RunCP run, CaretP caret)
    {
    if (NULL != dynamic_cast <FractionCP> (run))
        ExportFraction (static_cast <FractionCP> (run), caret);
    else if (NULL != dynamic_cast <TabCP> (run))
        ExportTab ( static_cast <TabCP> (run), caret);
    else if (NULL != dynamic_cast <LineBreakCP> (run))
        ExportLineBreak ( static_cast <LineBreakCP> (run));
    else if (NULL != dynamic_cast <ParagraphBreakCP> (run))
        ExportParagraphBreak ( static_cast <ParagraphBreakCP> (run));
    else if (NULL != dynamic_cast <CharStreamCP> (run))
        ExportCharStream (static_cast <CharStreamCP> (run), caret);
    else
        BeAssert (false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportRun (RunCP run, CaretP caret)
    {
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    ExportBeginAndEndSpans (run);
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    ExportRunElements (run, caret);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportLineBreak (LineBreakCP lineFeed)
    {
    FlushText (m_curProperties, true);
    m_curProperties = &lineFeed->GetProperties ();
    m_listener->PlayLinefeed ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportParagraphBreak (ParagraphBreakCP lineFeed)
    {
    FlushText (m_curProperties, true);
    m_curProperties = &lineFeed->GetProperties ();
    m_listener->PlayParagraphBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportTextBlock ()
    {
    RunPropertiesCP runProps = m_textBlock->GetNodeProperties ();
    if (NULL == runProps)
        runProps = m_textBlock->GetFirstRunProperties ();

    BeAssert (NULL != runProps);
    if (NULL == runProps)
        return;

    RunProperties   runProperties   = *runProps;
    DVec2d          lineOffset;
    
    lineOffset.zero ();
    runProperties.SetRunOffset (lineOffset);

    TextParamAndScaleP bodyParamAndScale = new TextParamAndScale (runProperties, *m_textBlock->GetParagraphProperties (0), m_textBlock->GetProperties (), m_textBlock->GetNodeProperties ());

    m_listener->PlayDocBegin (bodyParamAndScale, m_textBlock->GetProperties ().GetMaxCharactersPerLine ());

    Caret caret = m_textBlock->Begin ();
    
    while (!caret.IsAtEnd ())
        ExportParagraph (caret);

    m_listener->PlayDocEnd ();
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportBeginField (FieldP field)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportEndField (FieldP field)
    {
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportFraction (FractionCP fraction, CaretP caret)
    {
    FlushText (m_curProperties, true);

    m_curProperties = &fraction->GetProperties ();

    WString         numContents;
    CharStreamCP    numerator   = fraction->GetNumerator ();
    
    if (NULL != numerator)
        numContents = numerator->ToString ();

    WString         denomContents;
    CharStreamCP    denominator     = fraction->GetDenominator ();
    
    if (NULL != denominator)
        denomContents = denominator->ToString ();

    TextParamWide textParams;
    m_curProperties->ToElementData (textParams, *GetDgnProject());
    textParams.flags.underline = false;
    
    if (StackedFractionType::DiagonalBar == fraction->GetFractionType ())
        {
        textParams.lineOffset.x = 0.0;
        textParams.lineOffset.y = 0.0;
        }
    
    TextParamAndScale textParamAndScale (&textParams, &m_curProperties->GetFontSize ());
    m_listener->PlayFraction (&textParamAndScale, fraction->GetFractionType (), fraction->GetAlignment (), &fraction->GetTextScale (), numContents, denomContents);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportTab (TabCP tab, CaretP caret)
    {
    FlushText (m_curProperties, true);
    m_curProperties = &tab->GetProperties ();

    DPoint2d    tabExtents  = tab->GetTabExtents ();
    double      tabWidth    = (m_textBlock->GetProperties ().IsVertical () ? tabExtents.y : tabExtents.x);

    m_listener->PlayTab (m_curProperties->GetFontSize ().y, tabWidth);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    02/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::InsertSpace ()
    {
    size_t sz = m_text.size();
    if (0 == sz)
        {
        m_text.push_back (WChar (0x20));
        return;
        }

    WChar lc  = m_text [sz -1];
    WChar sp  = ((lc == 0x20) || (lc == '>')) ? 0xa0 : 0x20;
    
    m_text.push_back (sp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::FlushText (RunPropertiesCP properties, bool force)
    {
    size_t sz = m_text.size ();
    if (0 == sz)
        return;

    if (m_curProperties->Equals (*properties) && !force)
        return;

    BeAssert (NULL != properties);

    TextParamWide textParams;
    m_curProperties->ToElementData (textParams, *GetDgnProject());

    TextParamAndScale textParamAndScale (&textParams, &m_curProperties->GetFontSize());
    
    m_text.push_back ('\0');
    m_listener->PlayText (&textParamAndScale, m_text);
    m_text.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
bool TextBlockPlayer::ExportStaticFraction (CharStreamCR charStream, size_t index)
    {
    UInt8   numerator;
    UInt8   denominator;
    
    if (!charStream.GetFraction (numerator, denominator, static_cast<size_t>(index)))
        return false;

    FlushText (m_curProperties, true);
    m_curProperties = &charStream.GetProperties ();
    
    TextParamWide textParams;
    m_curProperties->ToElementData (textParams, *GetDgnProject());

    TextParamAndScale textParamAndScale (&textParams, &m_curProperties->GetFontSize());

    m_listener->PlayStaticFraction (&textParamAndScale, numerator, denominator);
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportFontChars (size_t startIndex, size_t endIndex, CharStreamCR charStream)
    {
    // endIndex must point to 1 after end character index
    for (size_t index = startIndex; index < endIndex; index++)
        {
        if (ExportStaticFraction (charStream, index))
            continue;

        WChar unicodeChar = charStream.GetCharacter (index);
        switch (unicodeChar)
            {
            case 0x0020:
            case 0x00A0:
                InsertSpace ();
                break;

            default:
                m_text.push_back (unicodeChar);
                break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportEDF (size_t startIndex, size_t endIndex, EdfCharStreamCR edf)
    {
    TextParamWide textParams;
    m_curProperties->ToElementData (textParams, *GetDgnProject());
    
    TextParamAndScale       textParamAndScale (&textParams, &m_curProperties->GetFontSize());
    bvector<WChar>    contents;
    
    for (size_t index = startIndex; index < endIndex; index++)
        contents.push_back (edf.GetCharacter (index));

    contents.push_back ('\0');
    m_listener->PlayEnterDataField (&textParamAndScale, edf.GetEdfJustification (), contents);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportCharStream (CharStreamCP charStream, CaretP caret)
    {
    FlushText (&charStream->GetProperties (), false);

    m_curProperties = &charStream->GetProperties ();

    EdfCharStreamCP edf = dynamic_cast<EdfCharStreamCP>(charStream);

    if (NULL == edf)
        {
        ExportFontChars (0, charStream->GetCharacterCount (), *charStream);
        return;
        }

    ExportEDF (0, edf->GetString ().length (), *edf);
        
    FlushText (m_curProperties, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlockPlayer::ExportParagraph (CaretR caret)
    {
    ParagraphCP         paragraph   = caret.GetCurrentParagraphCP ();
    IndentationDataCR   indentation = paragraph->GetProperties().GetIndentation ();
    T_DoubleVector      tabStops    = indentation.GetTabStops ();
    
    RunCP   run         = caret.GetCurrentRunCP ();
    bool    paraEmpty   = (run == NULL) || (NULL != dynamic_cast <ParagraphBreakCP> (run));
    
    m_listener->PlayParagraphBegin (indentation.GetHangingIndent (), indentation.GetFirstLineIndent (), tabStops, paraEmpty);

    while (NULL != run)
        {
        ExportRun (run, &caret);
        
        if (SUCCESS != caret.MoveToNextRun () || caret.GetCurrentParagraphCP () != paragraph)
            break;

        run = caret.GetCurrentRunCP ();
        }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    ExportEndingSpans (m_spanSet);
    m_spanSet.clear ();
#endif // BEIJING_DGNPLATFORM_WIP_Fields

    FlushText (m_curProperties, true);
    m_listener->PlayParagraphEnd ();
    
    if (NULL == run) // For an empty para we do not do a MoveNext Run. Hence
        caret.MoveToNextParagraph ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
void TextBlock::PlayTo (ITextListenerP listener, DgnModelP modelRef) const
    {
    TextBlockPlayer player (this, listener, modelRef);
    player.ExportTextBlock();
    }
