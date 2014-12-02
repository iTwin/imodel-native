/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/MdlTextInternal.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define ISNOTEQUAL(x, y) (fabs (x - y) > mgds_fc_epsilon)
#define ISEQUAL(x, y) (fabs (x - y) <= mgds_fc_epsilon)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Abeesh.Basheer  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
UShort BentleyApi::mdlText_getMarginTextJustification (UInt32 justification)
    {
    switch ((TextElementJustification)justification)
        {
        case TextElementJustification::LeftTop:        return (UShort)TextElementJustification::LeftMarginTop;
        case TextElementJustification::LeftMiddle:     return (UShort)TextElementJustification::LeftMarginMiddle;
        case TextElementJustification::LeftBaseline:   return (UShort)TextElementJustification::LeftMarginBaseline;
        case TextElementJustification::RightTop:       return (UShort)TextElementJustification::RightMarginTop;
        case TextElementJustification::RightMiddle:    return (UShort)TextElementJustification::RightMarginMiddle;
        case TextElementJustification::RightBaseline:  return (UShort)TextElementJustification::RightMarginBaseline;

        default:
            return static_cast<UShort>(justification);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JohnFerguson    05/02
+---------------+---------------+---------------+---------------+---------------+------*/
UShort BentleyApi::mdlText_constrainTextJustification (int justification)
    {
    switch ((TextElementJustification)justification)
        {
        case TextElementJustification::LeftMarginTop:          return (UShort)TextElementJustification::LeftTop;
        case TextElementJustification::LeftMarginMiddle:       return (UShort)TextElementJustification::LeftMiddle;
        case TextElementJustification::LeftMarginBaseline:     return (UShort)TextElementJustification::LeftBaseline;
        case TextElementJustification::RightMarginTop:         return (UShort)TextElementJustification::RightTop;
        case TextElementJustification::RightMarginMiddle:      return (UShort)TextElementJustification::RightMiddle;
        case TextElementJustification::RightMarginBaseline:    return (UShort)TextElementJustification::RightBaseline;

        default:
            return static_cast<UShort>(justification);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void LegacyTextStyleOverrideFlags::ComputeLogicalOr(LegacyTextStyleOverrideFlagsR result, LegacyTextStyleOverrideFlagsCR rhs) const
    {
    *(Int64*)&result = (*(Int64*)this) | (*(Int64*)&rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool LegacyTextStyleOverrideFlags::AreAnyFlagsSet () const
    {
    return (0 != *(Int64*)this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    HirooJumonji    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextDrawFlags::AreAnyFlagsSet () const
    {
    TextDrawFlags flagsForCompare = *this;
    
    flagsForCompare.unused = 0;
    
    return (0 != *(UInt16*)&flagsForCompare);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    HirooJumonji    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextExFlags::AreAnyFlagsSet () const
    {
    TextExFlags exFlagsForCompare = *this;
    
    // Pre-Beijing code didn't compare these... not sure why, but keeping behavior for now.
    exFlagsForCompare.annotationScale           = 0;
    exFlagsForCompare.bitMaskContainsTabCRLF    = 0;
    exFlagsForCompare.isField                   = 0;
    
    return (0 != *(UInt32*)&exFlagsForCompare);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextParamWide::ApplyScaleFactor (double scaleFactor, bool isTextNode, bool allowSizeChange)
    {
    return this->ApplyScaleFactor (DPoint2d::From (scaleFactor, scaleFactor), isTextNode, allowSizeChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextParamWide::ApplyScaleFactor (DPoint2dCR scaleFactor, bool isTextNode, bool allowSizeChange)
    {
    if (ISEQUAL (scaleFactor.x, 1.0) && ISEQUAL (scaleFactor.y, 1.0))
        return false;
    
    // Adding overrides (when none existed) will cause a size increase.
    if ((0 != textStyleId) && (allowSizeChange || this->exFlags.styleOverrides))
        {
        this->exFlags.styleOverrides = true;

        if (ISNOTEQUAL (scaleFactor.x, 1.0))
            this->overridesFromStyle.width = true;
        
        if (ISNOTEQUAL (scaleFactor.y, 1.0))
            this->overridesFromStyle.height = true;
        }

    if (this->flags.AreAnyFlagsSet () || this->exFlags.AreAnyFlagsSet ())
        {
        double characterSpacingScale = this->flags.vertical ? scaleFactor.y : scaleFactor.x;

        if (!this->exFlags.acadInterCharSpacing)
            this->characterSpacing *= characterSpacingScale;

        this->underlineSpacing      *= scaleFactor.y;
        this->overlineSpacing       *= scaleFactor.y;
        this->backgroundBorder.x    *= scaleFactor.x;
        this->backgroundBorder.y    *= scaleFactor.y;

        if (isTextNode)
            {
            this->textnodeWordWrapLength *= scaleFactor.x;

            if (DgnLineSpacingType::AtLeast != static_cast<DgnLineSpacingType>(this->exFlags.acadLineSpacingType))
                this->lineSpacing *= fabs (this->flags.vertical ? scaleFactor.x : scaleFactor.y);
            }
        else
            {
            if (scaleFactor.x * scaleFactor.y < 0)
                this->slant *= -1.0;

            if (this->flags.offset)
                {
                this->lineOffset.x *= scaleFactor.x;
                this->lineOffset.y *= scaleFactor.y;
                }
            }

        // Adding overrides (when none existed) will cause a size increase.
        if ((0 != textStyleId) && (allowSizeChange || this->exFlags.styleOverrides))
            {
            if (ISNOTEQUAL (this->characterSpacing, 0.0) && ISNOTEQUAL (characterSpacingScale, 1.0))
                {
                this->exFlags.styleOverrides                = true;
                this->overridesFromStyle.interCharSpacing   = true;
                }

            if (ISNOTEQUAL (this->underlineSpacing, 0.0) && ISNOTEQUAL (scaleFactor.y, 1.0))
                {
                this->exFlags.styleOverrides                = true;
                this->overridesFromStyle.underlineOffset    = true;
                }

            if (ISNOTEQUAL (this->overlineSpacing, 0.0) && ISNOTEQUAL (scaleFactor.y, 1.0))
                {
                this->exFlags.styleOverrides                = true;
                this->overridesFromStyle.overlineOffset     = true;
                }

            if (ISNOTEQUAL (this->backgroundBorder.x, 0.0) || ISNOTEQUAL (this->backgroundBorder.y, 0.0))
                {
                this->exFlags.styleOverrides                = true;
                this->overridesFromStyle.backgroundborder   = true;
                }
            }

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     05/08
+---------------+---------------+---------------+---------------+---------------+------*/
void TextParamWide::SetCodePage (int codePage)
    {
    // See comments in header file about why you should use this function.

    this->codePage_deprecated       = codePage;
    this->flags.codePage_deprecated = (0 != codePage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
DgnFontCR TextParamWide::GetFontForCodePage (DgnProjectR file)
    {
    return DgnFontManager::GetFontForCodePage (this->font, this->shxBigFont, file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void removeAllLinkagesWithIDFromEeh (EditElementHandleR eeh, UInt16 linkageId)
    {
    ElementLinkageIterator iter;
    
    while (eeh.EndElementLinkages () != (iter = eeh.BeginElementLinkages (linkageId)))
        eeh.RemoveElementLinkage (iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16 TextFormattingLinkage::GetLinkageID ()
    {
    return TEXTATTR_ID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextFormattingLinkage::DoesElementHaveThisLinkage (ElementHandleCR eh)
    {
    return (eh.EndElementLinkages () != eh.BeginElementLinkages (TextFormattingLinkage::GetLinkageID ()));
    }


#define VALIDATE_AND_READ(READER_VAR, DEST_VAR)\
    {\
    if (READER_VAR.AtOrBeyondEOS() || (READER_VAR.getRemainingSize() < sizeof(DEST_VAR)))\
        { BeDataAssert(false); return; }\
    READER_VAR.get(&DEST_VAR);\
    }

#define VALIDATE_AND_READ_WITH_SIZE(READER_VAR, STORAGE_VAR, DEST_VAR)\
    {\
    if (READER_VAR.AtOrBeyondEOS() || (READER_VAR.getRemainingSize() < sizeof(STORAGE_VAR)))\
        { BeDataAssert(false); return; }\
    READER_VAR.get(DEST_VAR, sizeof(STORAGE_VAR));\
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void fillTextParamsFromTextFormattingLinkageBuffer (DataInternalizer& reader, TextParamWideR textParams)
    {
    // In the old days, we had a linkage structure that went like this:
    //  
    //  {
    //  LinkageHeader   linkHeader;     // 2 words
    //  TextDrawFlags   flags;          // 1 word
    //  ** 1 Word Padding **
    //  TextExFlags     exFlags;        // 2 words
    //  byte            textData[300];  // *
    //  }
    //  
    //  And simply cast it in/out of the actual element linkage.
    //  Thus, there is 1 word of padding between flags and exflags.
    
    VALIDATE_AND_READ_WITH_SIZE(reader, textParams.flags, (byte*)&textParams.flags)
    
    UInt16 dummyWord;
    VALIDATE_AND_READ(reader, dummyWord)
    
    VALIDATE_AND_READ_WITH_SIZE(reader, textParams.exFlags, (byte*)&textParams.exFlags)
    
    if (textParams.flags.interCharSpacing || textParams.flags.fixedWidthSpacing || textParams.exFlags.acadInterCharSpacing)
        VALIDATE_AND_READ(reader, textParams.characterSpacing)

    if (textParams.flags.slant)
        {
        VALIDATE_AND_READ(reader, textParams.slant)

        if (textParams.slant > msGeomConst_pi)
            textParams.slant -= msGeomConst_2pi;
        }

    if (textParams.flags.underline)
        VALIDATE_AND_READ(reader, textParams.underlineSpacing)

    if (textParams.flags.offset)
        {
        VALIDATE_AND_READ(reader, textParams.lineOffset.x)
        VALIDATE_AND_READ(reader, textParams.lineOffset.y)
        }

    if (textParams.flags.codePage_deprecated)
        VALIDATE_AND_READ(reader, textParams.codePage_deprecated)
    
    if (textParams.flags.shxBigFont)
        VALIDATE_AND_READ(reader, textParams.shxBigFont)

    if (textParams.flags.bgColor)
        {
        VALIDATE_AND_READ(reader, textParams.backgroundColor)
        VALIDATE_AND_READ(reader, textParams.backgroundStyle)
        VALIDATE_AND_READ(reader, textParams.backgroundWeight)
        VALIDATE_AND_READ(reader, textParams.backgroundBorder.x)
        VALIDATE_AND_READ(reader, textParams.backgroundBorder.y)
        VALIDATE_AND_READ(reader, textParams.backgroundFillColor)
        }

    if (textParams.exFlags.overline)
        VALIDATE_AND_READ(reader, textParams.overlineSpacing)

    if (textParams.flags.textStyle)
        VALIDATE_AND_READ(reader, textParams.textStyleId)

    if (textParams.exFlags.underlineStyle)
        {
        VALIDATE_AND_READ(reader, textParams.underlineColor)
        VALIDATE_AND_READ(reader, textParams.underlineStyle)
        VALIDATE_AND_READ(reader, textParams.underlineWeight)
        }

    if (textParams.exFlags.overlineStyle)
        {
        VALIDATE_AND_READ(reader, textParams.overlineColor)
        VALIDATE_AND_READ(reader, textParams.overlineStyle)
        VALIDATE_AND_READ(reader, textParams.overlineWeight)
        }

    if (textParams.exFlags.wordWrapTextNode)
        VALIDATE_AND_READ(reader, textParams.textnodeWordWrapLength)

    if (textParams.exFlags.styleOverrides)
        VALIDATE_AND_READ_WITH_SIZE(reader, textParams.overridesFromStyle, (byte*)&textParams.overridesFromStyle)

    if (textParams.exFlags.color)
        VALIDATE_AND_READ(reader, textParams.color)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextFormattingLinkage::FillTextParamsFromLinkage (ElementHandleCR eh, TextParamWideR textParams)
    {
    ConstElementLinkageIterator iter = eh.BeginElementLinkages (TextFormattingLinkage::GetLinkageID ());
    if (eh.EndElementLinkages () == iter)
        return;
    
    size_t linkageDataSize = (size_t)LinkageUtil::GetWords (iter.GetLinkage ());
    linkageDataSize *= 2;
    linkageDataSize -= sizeof (LinkageHeader);
    
    if (0 == linkageDataSize)
        return;
    
    DataInternalizer reader ((byte const *)iter.GetData (), linkageDataSize);
    
    fillTextParamsFromTextFormattingLinkageBuffer (reader, textParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextFormattingLinkage::FillTextParams (TextParamWideR textParams) const
    {
    DataInternalizer reader ((byte const *)&this->flags, sizeof (*this) - sizeof (this->linkHeader));
    
    fillTextParamsFromTextFormattingLinkageBuffer (reader, textParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void encodeTextFormattingLinkageBufferFromTextParams (DataExternalizer& writer, TextParamWideCR textParams)
    {
    writer.put ((byte const *)&textParams.flags, sizeof (textParams.flags));
    
    // See fillTextParamsFromTextFormattingLinkageBuffer as to why we do this.
    UInt16 dummyWord = 0;
    writer.put (dummyWord);
    
    writer.put ((byte const *)&textParams.exFlags, sizeof (textParams.exFlags));
    
    if (textParams.flags.interCharSpacing || textParams.flags.fixedWidthSpacing || textParams.exFlags.acadInterCharSpacing)
        writer.put (textParams.characterSpacing);

    if (textParams.flags.slant)
        writer.put (textParams.slant);

    if (textParams.flags.underline)
        writer.put (textParams.underlineSpacing);

    if (textParams.flags.offset)
        {
        writer.put (textParams.lineOffset.x);
        writer.put (textParams.lineOffset.y);
        }

    if (textParams.flags.codePage_deprecated)
        writer.put (textParams.codePage_deprecated);

    if (textParams.flags.shxBigFont)
        writer.put (textParams.shxBigFont);

    if (textParams.flags.bgColor)
        {
        writer.put (textParams.backgroundColor);
        writer.put (textParams.backgroundStyle);
        writer.put (textParams.backgroundWeight);
        writer.put (textParams.backgroundBorder.x);
        writer.put (textParams.backgroundBorder.y);
        writer.put (textParams.backgroundFillColor);
        }

    if (textParams.exFlags.overline)
        writer.put (textParams.overlineSpacing);

    if (textParams.flags.textStyle)
        writer.put (textParams.textStyleId);

    if (textParams.exFlags.underlineStyle)
        {
        writer.put (textParams.underlineColor);
        writer.put (textParams.underlineStyle);
        writer.put (textParams.underlineWeight);
        }

    if (textParams.exFlags.overlineStyle)
        {
        writer.put (textParams.overlineColor);
        writer.put (textParams.overlineStyle);
        writer.put (textParams.overlineWeight);
        }

    if (textParams.exFlags.wordWrapTextNode)
        writer.put (textParams.textnodeWordWrapLength);

    if (textParams.exFlags.styleOverrides)
        {
        LegacyTextStyleOverrideFlags adjustedOverridesFromStyle = textParams.overridesFromStyle;

        if (textParams.exFlags.annotationScale)
            {
            adjustedOverridesFromStyle.width            = 1;
            adjustedOverridesFromStyle.height           = 1;
            adjustedOverridesFromStyle.linespacing      = 1;
            adjustedOverridesFromStyle.interCharSpacing = 1;
            adjustedOverridesFromStyle.underlineOffset  = 1;
            adjustedOverridesFromStyle.overlineOffset   = 1;
            }

        writer.put ((byte const *)&adjustedOverridesFromStyle, sizeof (adjustedOverridesFromStyle));
        }

    if (textParams.exFlags.color)
        writer.put (textParams.color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextFormattingLinkage::AppendLinkageFromTextParams (EditElementHandleR eeh, TextParamWideCR textParams)
    {
    if (!textParams.flags.AreAnyFlagsSet () && !textParams.exFlags.AreAnyFlagsSet ())
        return;
    
    DataExternalizer writer;
    encodeTextFormattingLinkageBufferFromTextParams (writer, textParams);
    
    ElementLinkageUtil::AddLinkage (eeh, TextFormattingLinkage::GetLinkageID (), writer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextFormattingLinkage::AppendLinkageFromTextParams (DgnElementR el, TextParamWideCR textParams)
    {
    if (!textParams.flags.AreAnyFlagsSet () && !textParams.exFlags.AreAnyFlagsSet ())
        return;
    
    DataExternalizer writer;
    encodeTextFormattingLinkageBufferFromTextParams (writer, textParams);
    
    LinkageHeader linkHdr;
    ElementLinkageUtil::InitLinkageHeader (linkHdr, TextFormattingLinkage::GetLinkageID (), writer.getBytesWritten ());
    
    if (SUCCESS != linkage_appendToElement (&el, &linkHdr, (void*)writer.getBuf (), NULL))
        BeAssert (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextFormattingLinkage::InitializeFromTextParams (TextParamWideCR textParams)
    {
    memset (this, 0, sizeof (*this));
    
    this->linkHeader.user       = 1;
    this->linkHeader.primaryID  = TextFormattingLinkage::GetLinkageID ();
    
    size_t bytesWritten = 0;
    
    if (textParams.flags.AreAnyFlagsSet () || !textParams.exFlags.AreAnyFlagsSet ())
        {
        DataExternalizer writer ((byte*)&this->flags, sizeof (*this) - sizeof (this->linkHeader));
        
        encodeTextFormattingLinkageBufferFromTextParams (writer, textParams);
        
        bytesWritten = writer.getBytesWritten ();
        }
    
    LinkageUtil::SetWords (&this->linkHeader, (((sizeof (this->linkHeader) + bytesWritten) + 7) & ~7) / 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextFormattingLinkage::RemoveLinkages (EditElementHandleR eeh)
    {
    removeAllLinkagesWithIDFromEeh (eeh, TextFormattingLinkage::GetLinkageID ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextFormattingLinkage::RemoveLinkages (DgnElementR el)
    {
    linkage_deleteFromElement (&el, TextFormattingLinkage::GetLinkageID (), NULL, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16 TextAnnotationLinkage::GetLinkageID ()
    {
    return LINKAGEID_TextAnnotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextAnnotationLinkage::DoesElementHaveThisLinkage (ElementHandleCR eh)
    {
    return (eh.EndElementLinkages () != eh.BeginElementLinkages (TextAnnotationLinkage::GetLinkageID ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationLinkage::FillTextParamsFromLinkage (ElementHandleCR eh, TextParamWideR textParams)
    {
    // In the old days, we had a linkage structure that went like this:
    //  
    //  {
    //  LinkageHeader               linkHeader;         // 2 words
    //  ** 2 Words Padding **
    //  double                      annotationScale;    // 4 words
    //  TextAnnotationOverrideFlags flags;              // 1 word
    //  }
    //  
    //  And simply cast it in/out of the actual element linkage.
    //  Thus, there are 2 words of padding between linkHeader and annotationScale.
    
    ConstElementLinkageIterator iter = eh.BeginElementLinkages (TextAnnotationLinkage::GetLinkageID ());
    if (eh.EndElementLinkages () == iter)
        return;
    
    size_t linkageDataSize = (size_t)LinkageUtil::GetWords (iter.GetLinkage ());
    linkageDataSize *= 2;
    linkageDataSize -= sizeof (LinkageHeader);
    
    if (0 == linkageDataSize)
        return;
    
    DataInternalizer reader ((byte const *)iter.GetData (), linkageDataSize);
    
    UInt16 dummyWord;
    reader.get (&dummyWord);
    reader.get (&dummyWord);
    
    reader.get (&textParams.annotationScale);
    
    textParams.exFlags.annotationScale = true;
    
    TextAnnotationOverrideFlags flags;
    reader.get ((byte*)&flags, sizeof (flags));
    
    textParams.overridesFromStyle.width             = flags.width;
    textParams.overridesFromStyle.height            = flags.height;
    textParams.overridesFromStyle.linespacing       = flags.linespacing;
    textParams.overridesFromStyle.interCharSpacing  = flags.interCharSpacing;
    textParams.overridesFromStyle.underlineOffset   = flags.underlineOffset;
    textParams.overridesFromStyle.overlineOffset    = flags.overlineOffset;
    textParams.overridesFromStyle.lineOffset        = flags.lineOffset;
    textParams.overridesFromStyle.backgroundborder  = flags.backgroundborder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static void encodeTextAnnotationLinkageBufferFromTextParams (DataExternalizer& writer, TextParamWideCR textParams)
    {
    // See TextAnnotationLinkage::FillTextParamsFromLinkage as to why we do this.
    UInt16 dummyWord = 0;
    writer.put (dummyWord);
    writer.put (dummyWord);
    
    writer.put (textParams.annotationScale);
    
    TextAnnotationOverrideFlags flags;
    memset (&flags, 0, sizeof (flags));
    
    flags.width             = textParams.overridesFromStyle.width;
    flags.height            = textParams.overridesFromStyle.height;
    flags.linespacing       = textParams.overridesFromStyle.linespacing;
    flags.interCharSpacing  = textParams.overridesFromStyle.interCharSpacing;
    flags.underlineOffset   = textParams.overridesFromStyle.underlineOffset;
    flags.overlineOffset    = textParams.overridesFromStyle.overlineOffset;
    flags.lineOffset        = textParams.overridesFromStyle.lineOffset;
    flags.backgroundborder  = textParams.overridesFromStyle.backgroundborder;
    
    writer.put ((byte*)&flags, sizeof (flags));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationLinkage::AppendLinkageFromTextParams (EditElementHandleR eeh, TextParamWideCR textParams)
    {
    if (!textParams.exFlags.annotationScale)
        return;
    
    DataExternalizer writer;
    encodeTextAnnotationLinkageBufferFromTextParams (writer, textParams);

    ElementLinkageUtil::AddLinkage (eeh, TextAnnotationLinkage::GetLinkageID (), writer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextAnnotationLinkage::AppendLinkageFromTextParams (DgnElementR el, TextParamWideCR textParams)
    {
    if (!textParams.exFlags.annotationScale)
        return;
    
    DataExternalizer writer;
    encodeTextAnnotationLinkageBufferFromTextParams (writer, textParams);

    LinkageHeader linkHdr;
    ElementLinkageUtil::InitLinkageHeader (linkHdr, TextAnnotationLinkage::GetLinkageID (), writer.getBytesWritten ());
    
    if (SUCCESS != linkage_appendToElement (&el, &linkHdr, (void*)writer.getBuf (), NULL))
        BeAssert (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextAnnotationLinkage::RemoveLinkages (EditElementHandleR eeh)
    {
    removeAllLinkagesWithIDFromEeh (eeh, TextAnnotationLinkage::GetLinkageID ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextAnnotationLinkage::RemoveLinkages (DgnElementR el)
    {
    linkage_deleteFromElement (&el, TextAnnotationLinkage::GetLinkageID (), NULL, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16 TextRenderingLinkage::GetLinkageID ()
    {
    return LINKAGEID_TextRendering;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextRenderingLinkage::DoesElementHaveThisLinkage (ElementHandleCR eh)
    {
    return (eh.EndElementLinkages () != eh.BeginElementLinkages (TextRenderingLinkage::GetLinkageID ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextRenderingLinkage::FillTextParamsFromLinkage (ElementHandleCR eh, TextParamWideR textParams)
    {
    // In the old days, this linkage was actually packed (unlike TextFormattingLinkage and TextAnnotationLinkage).
    
    ConstElementLinkageIterator iter = eh.BeginElementLinkages (TextRenderingLinkage::GetLinkageID ());
    if (eh.EndElementLinkages () == iter)
        return;
    
    size_t linkageDataSize = (size_t)LinkageUtil::GetWords (iter.GetLinkage ());
    linkageDataSize *= 2;
    linkageDataSize -= sizeof (LinkageHeader);
    
    if (0 == linkageDataSize)
        return;
    
    DataInternalizer reader ((byte const *)iter.GetData (), linkageDataSize);
    
    reader.get ((byte*)&textParams.renderingFlags, sizeof (textParams.renderingFlags));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static void encodeTextRenderingLinkageBufferFromTextParams (DataExternalizer& writer, TextParamWideCR textParams)
    {
    writer.put ((byte const *)&textParams.renderingFlags, sizeof (textParams.renderingFlags));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextRenderingLinkage::AppendLinkageFromTextParams (EditElementHandleR eeh, TextParamWideCR textParams)
    {
    // In the old days, this linkage was actually packed (unlike TextFormattingLinkage and TextAnnotationLinkage).
    
    if (!textParams.renderingFlags.alignEdge && !textParams.renderingFlags.lineAlignment && !textParams.renderingFlags.documentType)
        return;
    
    DataExternalizer writer;
    encodeTextRenderingLinkageBufferFromTextParams (writer, textParams);

    ElementLinkageUtil::AddLinkage (eeh, TextRenderingLinkage::GetLinkageID (), writer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextRenderingLinkage::AppendLinkageFromTextParams (DgnElementR el, TextParamWideCR textParams)
    {
    // In the old days, this linkage was actually packed (unlike TextFormattingLinkage and TextAnnotationLinkage).
    
    if (!textParams.renderingFlags.alignEdge && !textParams.renderingFlags.lineAlignment && !textParams.renderingFlags.documentType)
        return;
    
    DataExternalizer writer;
    encodeTextRenderingLinkageBufferFromTextParams (writer, textParams);

    LinkageHeader linkHdr;
    ElementLinkageUtil::InitLinkageHeader (linkHdr, TextRenderingLinkage::GetLinkageID (), writer.getBytesWritten ());
    
    if (SUCCESS != linkage_appendToElement (&el, &linkHdr, (void*)writer.getBuf (), NULL))
        BeAssert (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextRenderingLinkage::RemoveLinkages (EditElementHandleR eeh)
    {
    removeAllLinkagesWithIDFromEeh (eeh, TextRenderingLinkage::GetLinkageID ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextRenderingLinkage::RemoveLinkages (DgnElementR el)
    {
    linkage_deleteFromElement (&el, TextRenderingLinkage::GetLinkageID (), NULL, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16 TextIndentationLinkage::GetLinkageID ()
    {
    return TEXT_IndentationLinkage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextIndentationLinkage::DoesElementHaveThisLinkage (ElementHandleCR eh)
    {
    return (eh.EndElementLinkages () != eh.BeginElementLinkages (TextIndentationLinkage::GetLinkageID ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextIndentationLinkage::FillIndentationDataFromLinkage (ElementHandleCR eh, IndentationDataR indentationData)
    {
    // In the old days, we had a linkage structure that went like this:
    //  
    //  {
    //  LinkageHeader           linkHeader; // 2 words
    //      {
    //      ** 2 Words Padding **
    //      double      firstLineIndent;    // 4 words
    //      double      paragraphIndent;    // 4 words
    //      int         tabCount;           // 2 words
    //      ** 2 Words Padding **
    //      double      tabStops[1];        // 4 words
    //      }
    //  }
    //  
    //  And simply cast it in/out of the actual element linkage.
    //  Thus, there are 2 words of padding between linkHeader and firstLineIndent, as well as 2 words of padding between tabCount and TabStops.
    
    ConstElementLinkageIterator iter = eh.BeginElementLinkages (TextIndentationLinkage::GetLinkageID ());
    if (eh.EndElementLinkages () == iter)
        return;
    
    size_t linkageDataSize = (size_t)LinkageUtil::GetWords (iter.GetLinkage ());
    linkageDataSize *= 2;
    linkageDataSize -= sizeof (LinkageHeader);
    
    if (0 == linkageDataSize)
        return;
    
    DataInternalizer reader ((byte const *)iter.GetData (), linkageDataSize);
    
    UInt16 dummyWord;
    reader.get (&dummyWord);
    reader.get (&dummyWord);
    
    double d;
    
    reader.get (&d); indentationData.SetFirstLineIndent (d);
    reader.get (&d); indentationData.SetHangingIndent (d);
    
    int tabCount;
    reader.get (&tabCount);
    
    if (0 == tabCount)
        return;
    
    // Detect and ignore bad indentation linkages... I guess somebody managed to create a bad data set somehow...
    size_t correctLinkageSize = 4 * sizeof (UInt16) + (2 + tabCount) * sizeof (double) + sizeof (int);
    
    if (linkageDataSize < correctLinkageSize)
        return;
    
    reader.get (&dummyWord);
    reader.get (&dummyWord);
    
    T_DoubleVector tabStops;
    for (int i = 0; i < tabCount; ++i)
        {
        reader.get (&d);
        tabStops.push_back (d);
        }
    
    indentationData.SetTabStops (tabStops);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextIndentationLinkage::AppendLinkageFromIndentationData (EditElementHandleR eeh, IndentationDataCR indentationData)
    {
    if (indentationData.IsDefault ())
        return;
    
    DataExternalizer writer;
    
    // See TextIndentationLinkage::FillIndentationDataFromLinkage as to why we do this.
    UInt16 dummyWord = 0;
    writer.put (dummyWord);
    writer.put (dummyWord);
    
    writer.put (indentationData.GetFirstLineIndent ());
    writer.put (indentationData.GetHangingIndent ());
    
    T_DoubleVectorCR    tabStops    = indentationData.GetTabStops ();
    int                 numTabStops = (int)tabStops.size ();
    
    writer.put (numTabStops);
    
    writer.put (dummyWord);
    writer.put (dummyWord);
    
    for (int i = 0; i < numTabStops; ++i)
        writer.put (tabStops[i]);

    ElementLinkageUtil::AddLinkage (eeh, TextIndentationLinkage::GetLinkageID (), writer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void TextIndentationLinkage::RemoveLinkages (EditElementHandleR eeh)
    {
    removeAllLinkagesWithIDFromEeh (eeh, TextIndentationLinkage::GetLinkageID ());
    }
