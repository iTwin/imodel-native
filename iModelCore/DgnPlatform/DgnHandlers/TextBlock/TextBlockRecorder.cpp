/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextBlockRecorder.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*=================================================================================**//**
* @bsiclass                                                     Jyoti.Swarup    07/06
+===============+===============+===============+===============+===============+======*/
class TextBlockRecorder : public ITextListener
{
private:
    friend  struct DgnPlatform::TextBlock;
    TextBlockR          m_textBlock;
    TextParamAndScale   m_lastformat;

public:

                            TextBlockRecorder   (TextBlockR textBlock);

    virtual void            PlayDocBegin        (TextParamAndScaleP, UInt32 linelength) override;
    virtual void            PlayDocEnd          () override;
    virtual void            PlayParagraphBegin  (double paraIndent, double firstLineIndent, T_DoubleVectorCR tabStops, bool paraEmpty) override;
    virtual void            PlayParagraphEnd    () override;
    virtual void            PlayTab             (double height, double width) override;
    virtual void            PlayLinefeed        () override;
    virtual void            PlayParagraphBreak  () override;
    virtual void            PlayText            (TextParamAndScaleP textParamAndScale, WCharArray& text) override;
#ifdef BEIJING_DGNPLATFORM_WIP_Fields
    virtual void            PlayField           (TextParamAndScaleP textParamAndScale, WCharArray const& spec, WCharArray const& fieldStr) override;
#endif // BEIJING_DGNPLATFORM_WIP_Fields
    virtual void            PlayEnterDataField  (TextParamAndScaleP textParamAndScale, EdfJustification just, WCharArray const& contents) override;
    virtual void            PlayStaticFraction  (TextParamAndScaleP textParamAndScale, int numerator, int denominator) override;
    virtual void            PlayFraction        (TextParamAndScaleP textParamAndScale, StackedFractionType type, StackedFractionAlignment algn, DPoint2dCP fractionScale, WStringR numContents, WStringR denomContents) override;

}; // TextBlockRecorder

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
TextBlockRecorder::TextBlockRecorder (TextBlockR textBlock) :
    m_textBlock (textBlock)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayDocBegin (TextParamAndScaleP tps, UInt32 linelength)
    {
    m_lastformat = *tps;

    TextParamWideP  fs      = tps->GetTextParamWide ();
    DPoint2dP       scale   = tps->GetScale ();

    // viewIndepenent flag is not being set in wordlib. It has to come from text block. Hence, preserver whatever is on the textblock.
    bool viewIndependent = m_textBlock.GetProperties ().IsViewIndependent ();

    TextBlockProperties textblockProperties (*fs, *scale, linelength, m_textBlock.GetDgnModelR ());
    m_textBlock.SetProperties (textblockProperties);

    m_textBlock.SetTextNodeProperties (NULL);

    ParagraphProperties paraProperties (*fs, m_textBlock.GetDgnModelR ());
    m_textBlock.SetParagraphPropertiesForRange (paraProperties, ParagraphRange (m_textBlock));

    m_textBlock.SetViewIndependent (viewIndependent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayDocEnd ()
    {
    m_textBlock.PerformLayout ();
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayParagraphBegin (double paraIndent, double firstLineIndent, T_DoubleVectorCR tabStops, bool paraEmpty)
    {
    IndentationData indentation = m_textBlock.GetParagraphPropertiesForAdd ().GetIndentation ();
    indentation.SetHangingIndent (paraIndent);
    indentation.SetFirstLineIndent (firstLineIndent);
    indentation.SetTabStops (tabStops);
    
    m_textBlock.GetParagraphPropertiesForAddR ().SetIndentation (indentation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayParagraphEnd ()
    {
    BeAssert (false && L"Not Implemented.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayTab (double height, double width)
    {
    DPoint2d        scale           = { width, height };
    RunProperties   runProperties   (*m_lastformat.GetTextParamWide (), scale, m_textBlock.GetDgnModelR ());
    
    m_textBlock.SetRunPropertiesForAdd (runProperties);
    m_textBlock.AppendTab ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayLinefeed ()
    {
    RunProperties runProperties (*m_lastformat.GetTextParamWide(), *m_lastformat.GetScale(), m_textBlock.GetDgnModelR ());
    m_textBlock.SetRunPropertiesForAdd (runProperties);
    m_textBlock.AppendLineBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayParagraphBreak ()
    {
    RunProperties runProperties (*m_lastformat.GetTextParamWide(), *m_lastformat.GetScale(), m_textBlock.GetDgnModelR ());
    m_textBlock.SetRunPropertiesForAdd (runProperties);
    m_textBlock.AppendParagraphBreak ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayText (TextParamAndScaleP pTextParamAndScale, bvector<WChar> &text)
    {
    m_lastformat = *pTextParamAndScale;

    WCharP        pText   = &text[0];
    TextParamWideP  fs      = pTextParamAndScale->GetTextParamWide ();

    fs->exFlags.crCount = 0;

    RunProperties runProperties (*m_lastformat.GetTextParamWide (), *m_lastformat.GetScale (), m_textBlock.GetDgnModelR ());
    
    m_textBlock.SetRunPropertiesForAdd (runProperties);
    m_textBlock.AppendText (pText);
    }

#ifdef BEIJING_DGNPLATFORM_WIP_Fields
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayField (TextParamAndScaleP pTextParamAndScale, const bvector<WChar> &spec, const bvector<WChar> &fieldStr)
    {
    BeAssert (false && L"Not Implemented.");
    }
#endif // BEIJING_DGNPLATFORM_WIP_Fields

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayEnterDataField (TextParamAndScaleP pTextParamAndScale, EdfJustification just, const bvector<WChar> &contents)
    {
    BeAssert (false && L"Not Implemented.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayStaticFraction (TextParamAndScaleP pTextParamAndScale, int numerator, int denominator)
    {
    BeAssert (false && L"Not Implemented.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    08/06
//---------------------------------------------------------------------------------------
void TextBlockRecorder::PlayFraction (TextParamAndScale* textParamAndScale, StackedFractionType type, StackedFractionAlignment algn, DPoint2dCP fractionScale, WStringR numContents, WStringR denomContents)
    {
    BeAssert (false && L"Not Implemented.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jyoti.Swarup    07/06
//---------------------------------------------------------------------------------------
ITextListenerPtr TextBlock::CreateListener ()
    {
    return new TextBlockRecorder (*this);
    }
