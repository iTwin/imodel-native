/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/CharStream.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>
#include <DgnPlatform/DgnCore/DgnRscFont.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace std;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- TextFieldData ----------------------------------------------------------------------------------------------------------------- TextFieldData --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2010
//---------------------------------------------------------------------------------------
static void readByteArray (bvector<byte>& contents, DataInternalizer& internalizer)
    {
    UInt16 count;
    internalizer.get (&count);
    
    contents.resize (count);
    
    UInt8* firstElement = contents.data ();
    
    internalizer.get (firstElement, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2010
//---------------------------------------------------------------------------------------
static WString readString (DataInternalizer& internalizer)
    {
    UInt8 stringType;
    internalizer.get (&stringType);

    UInt16 stringLength;
    internalizer.get (&stringLength);

    if (0 == stringLength)
        return WString (L"");

    if (0 == stringType)
        {
        // 2 byte chars
        size_t      bufferLen   = (stringLength + 1) * 2;
        wchar_t*    buffer      = (wchar_t*)_alloca (bufferLen);
        
        memset (buffer, 0, bufferLen);
        internalizer.get ((UInt16*)buffer, (int)stringLength);
        
        return WString (buffer);
        }

    // 1 byte chars
    size_t  bufferLen   = stringLength + 1;
    Int8*   buffer      = (Int8*)_alloca (bufferLen);
    
    memset (buffer, 0, bufferLen);
    internalizer.get (buffer, (int)stringLength);
    
    return WString ((char*)buffer, false);
    }

#ifdef DGN_IMPORTER_REORG_WIP
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2010
//---------------------------------------------------------------------------------------
static void writeString (WCharCP string, DataExternalizer& externalizer)
    {
    UInt8   stringType  = 1; // Default to 1-byte characters
    UInt16  count       = (UInt16)wcslen (string);

    if (0 == count)
        {
        externalizer.put (stringType);
        externalizer.put (count);
        
        return;
        }

    UInt8* buffer = (UInt8*)_alloca (count);
    
    for (unsigned i = 0; i < count; ++i)
        {
        buffer [i] = (UInt8)(string [i] & 0xFF);
        if (buffer[i] != string[i])
            stringType = 0;
        }

    externalizer.put (stringType);
    externalizer.put (count);
    
    if (0 == stringType)
        {
        UInt16 const* cString = (Utf16CP)string;
        externalizer.put (cString, count);
        
        return;
        }
    
    externalizer.put (buffer, count);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2010
//---------------------------------------------------------------------------------------
WCharCP           TextFieldData::GetEvaluatorId  () const { return m_evaluatorId.c_str (); }
WCharCP           TextFieldData::GetExpression   () const { return m_expression.c_str (); }
bvector<byte> const&   TextFieldData::GetFormatterBytes    () const { return m_formatterBytes; }
WCharCP             TextFieldData::GetFormatterName() const { return !m_formatterName.empty() ? m_formatterName.c_str() : NULL; }
ElementId           TextFieldData::GetSourceElementId() const       { return m_sourceElementId; }
ElementId           TextFieldData::GetDependentElementId() const    { return m_dependentElementId; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2010
//---------------------------------------------------------------------------------------
TextFieldData::TextFieldData (WCharCP evaluatorId, WCharCP expression, WCharCP formatterName, bvector<byte> const& formatterBytes, ElementId srcElementId, ElementId depElementId) :
    m_evaluatorId   (evaluatorId),
    m_expression    (expression),
    m_formatterName (formatterName),
    m_formatterBytes(formatterBytes.begin(), formatterBytes.end()),
    m_sourceElementId (srcElementId),
    m_dependentElementId (depElementId)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2010
//---------------------------------------------------------------------------------------
TextFieldDataPtr TextFieldData::Create (WCharCP evaluatorId, WCharCP expression, WCharCP formatterName, bvector<byte> const& formatterBytes, ElementId srcElementId, ElementId depElementId)
    {
    return new TextFieldData (evaluatorId, expression, formatterName, formatterBytes, srcElementId, depElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2010
//---------------------------------------------------------------------------------------
TextFieldDataPtr TextFieldData::Create (ElementHandleCR eh)
    {
    XAttributeHandlerId             xAttrHandlerID  (XATTRIBUTEID_ECField, FIELD_XAttrId);
    ElementHandle::XAttributeIter   xAttrIter       (eh, xAttrHandlerID, 0); 

    if (!xAttrIter.IsValid ())
        return NULL;
    
    WString             evaluatorId;
    WString             expression;
    WString             formatterName;
    bvector<byte>       formatterBytes;
    DataInternalizer    internalizer    ((byte const*)xAttrIter.PeekData (), xAttrIter.GetSize ());

    Int32   size;                   internalizer.get (&size);
    UInt8   version;                internalizer.get (&version);
    UInt32  rawFlags;               internalizer.get (&rawFlags);
    UInt16  state;                  internalizer.get (&state);
    UInt16  rawEvaluationReason;    internalizer.get (&rawEvaluationReason);
    
    PropertyFieldContentsFlags      contentsFlags   = static_cast<PropertyFieldContentsFlags>(rawFlags);
//    PropertyFieldEvaluationReason   evalReason      = static_cast<PropertyFieldEvaluationReason>(rawEvaluationReason);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::EvaluatorIdFlag))
        evaluatorId = readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::ExpressionFlag))
        expression = readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::ErrorMessageFlag))
        readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::CachedDisplayFlag))
        readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::CachedValueFlag))
        {
        // We are not doing anything with it yet but we still need to step over it.
        UInt8 valueType;
        internalizer.get (&valueType);
        
        switch (valueType)
            {
            case TextFieldData::StringValue:
                {
                readString (internalizer);
                break;
                }

            case TextFieldData::Int32Value:
                {
                Int32 temp;
                internalizer.get (&temp);
                break;
                }
            
            case TextFieldData::Int64Value:
                {
                Int64 temp;
                internalizer.get (&temp);
                break;
                }

            case TextFieldData::Double:
            case TextFieldData::DateTimeValue:
                {
                double temp;
                internalizer.get (&temp);
                break;
                }
            }
        }

    // Throw away the obsolete format string.
    if (static_cast<int>(contentsFlags) & (1 << 7))
        readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::HandlerPersistenceKeyFlag))
        formatterName = readString (internalizer);

    if (TextFieldData::NoneFlag != (contentsFlags & TextFieldData::HandlerPersistentDataFlag))
        readByteArray (formatterBytes, internalizer);

    // See if dependency linkage exists
    UInt64 depElementId = INVALID_ELEMENTID;
#if defined (NEEDS_WORK_DGNITEM)
    DependencyLinkageAccessor depLinkage;
    if (SUCCESS == DependencyManagerLinkage::GetLinkage (&depLinkage, eh, DEPENCENCYAPPID_TextField, 0))
        {
        DependencyRoot roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
        BeAssert (NULL != eh.GetDgnModelP());
        UInt16 nRoots = DependencyManagerLinkage::GetRoots (roots, eh.GetDgnModelP(), *depLinkage, 0);
        if (1 == nRoots)
            depElementId = roots[0].elemid;
        }
#endif

    return new TextFieldData (evaluatorId.c_str (), expression.c_str (), formatterName.c_str (), formatterBytes, eh.GetElementId(), ElementId(depElementId));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2010
//---------------------------------------------------------------------------------------
void TextFieldData::AppendToElement (EditElementHandleR eeh) const
    {
#ifdef DGN_IMPORTER_REORG_WIP
    XAttributeHandlerId xAttrHandlerID  (XATTRIBUTEID_ECField, FIELD_XAttrId);
    DataExternalizer    externalizer;

    UInt32 blockSize = 0;
    externalizer.put (blockSize);
    
    UInt8 fieldFormatVersion = 1;
    externalizer.put (fieldFormatVersion);

    UInt32 contentsFlags = 0;
    
    if (m_evaluatorId.length () > 0)
        contentsFlags = (contentsFlags | TextFieldData::EvaluatorIdFlag);

    if (m_expression.length () > 0)
        contentsFlags = (contentsFlags | TextFieldData::ExpressionFlag);

    if (m_formatterName.length () > 0)
        contentsFlags = (contentsFlags | TextFieldData::HandlerPersistenceKeyFlag);

    if (m_formatterBytes.size() > 0)
        contentsFlags = (contentsFlags | TextFieldData::HandlerPersistentDataFlag);

    externalizer.put (contentsFlags);

    UInt16 stateFlags = 0; // Need a good default value for this.
    externalizer.put (stateFlags);
    
    UInt16 evaluationFlags = 0; // Need a good default value for this.
    externalizer.put (evaluationFlags);

    // May want to give the field handler a chance to turn off some of the bits in the contents flag to stop use from unnecessarily saving stuff.
    if (TextFieldData::NoValue != (contentsFlags & TextFieldData::EvaluatorIdFlag))
        writeString (m_evaluatorId.c_str (), externalizer);

    if ((contentsFlags & TextFieldData::ExpressionFlag) != TextFieldData::NoValue)
        writeString (m_expression.c_str (), externalizer);

    if ((contentsFlags & TextFieldData::HandlerPersistenceKeyFlag) != TextFieldData::NoValue)
        writeString (m_formatterName.c_str (), externalizer);

    if ((contentsFlags & TextFieldData::HandlerPersistentDataFlag) != TextFieldData::NoValue)
        {
        UInt16 count16 = (UInt16)m_formatterBytes.size();
        externalizer.put (count16);
        externalizer.put (&m_formatterBytes[0], m_formatterBytes.size());
        }

    UInt32* sizeEntry = (UInt32*)externalizer.getBufRW ();
    blockSize = (UInt32)externalizer.getBytesWritten ();
    *sizeEntry = blockSize;

    // Add dependency linkage if required
    TextFieldHandlerCP handler = TextFieldHandler::GetById (m_evaluatorId.c_str());
    BeAssert (NULL != handler);
    if (INVALID_ELEMENTID != m_dependentElementId)
        {
        // Is either a element property field, or a placeholder field that has been placed in a cell
        BeAssert (handler->RequiresElementSource() || handler->IsPlaceHolderType());
        DependencyLinkage   depLinkage;
        DependencyManagerLinkage::DefineElementIDDependency (depLinkage, DEPENCENCYAPPID_TextField, 0, DEPENDENCY_ON_COPY_RemapRootsWithinSelection, m_dependentElementId);
        DependencyManagerLinkage::AppendLinkage (eeh, depLinkage, 0);
        }
    else if (handler->IsPlaceHolderType())
        {
        // placeholder field which has not been placed in a cell yet. Add a self-dependency to trigger callback when it is placed into a cell.
        DependencyLinkage   depLinkage;
        DependencyManagerLinkage::InitLinkage (depLinkage, DEPENCENCYAPPID_TextField, 0, DEPENDENCY_ON_COPY_RemapRootsWithinSelection);
        DependencyManagerLinkage::AppendLinkage (eeh, depLinkage, 0);
        }
    else
        { BeAssert (!handler->RequiresElementSource()); }

    // Schedule the XAttribute to be saved with the element.
    eeh.ScheduleWriteXAttribute (xAttrHandlerID, FIELD_XAttrId, externalizer.getBytesWritten (), externalizer.getBuf ());   
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
TextFieldDataPtr TextFieldData::Clone () const
    {
    return new TextFieldData (m_evaluatorId.c_str (), m_expression.c_str (), m_formatterName.c_str (), m_formatterBytes, m_sourceElementId, m_dependentElementId);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- CharStream Helper Functions ------------------------------------------------------------------------------------- CharStream Helper Functions --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- CharStream ----------------------------------------------------------------------------------------------------------------------- CharStream --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
CharStream::CharStream (DgnModelR dgnCache) :
    Run (dgnCache)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
CharStream::CharStream (WCharCP chars, TextParamWideCR textParams, DPoint2dCR textScale, DgnModelR dgnCache) :
    Run (textParams, textScale, dgnCache)
    {
    Init (chars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
CharStream::CharStream (WCharCP chars, RunPropertiesCR runProperties, DgnGlyphRunLayoutFlags layoutFlags) :
    Run (runProperties, layoutFlags)
    {
    Init (chars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
CharStream::CharStream (CharStreamCR rhs) :
    Run (rhs),
    m_string                            (rhs.m_string),
    m_isDirty                           (rhs.m_isDirty),
    m_maxDisplacementBelowOrigin        (rhs.m_maxDisplacementBelowOrigin),
    m_maxHorizontalCellIncrement        (rhs.m_maxHorizontalCellIncrement),
    m_trailingInterCharacterSpacing     (rhs.m_trailingInterCharacterSpacing),
    m_leadingCharacterOffsets           (rhs.m_leadingCharacterOffsets),
    m_trailingCharacterOffsets          (rhs.m_trailingCharacterOffsets),
    m_computedTrailingCharacterOffset   (rhs.m_computedTrailingCharacterOffset),
    m_containsOnlyWhitespace            (rhs.m_containsOnlyWhitespace)
    {
    if (rhs.m_fieldData.IsValid ())
        m_fieldData = rhs.m_fieldData->Clone ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void CharStream::Init (WCharCP chars)
    {
    if (WString::IsNullOrEmpty (chars))
        BeAssert (false);
    else
        m_string = chars;
    
    m_isDirty                           = true;
    m_maxHorizontalCellIncrement        = 0.0;
    m_trailingInterCharacterSpacing     = 0.0;
    m_computedTrailingCharacterOffset   = 0.0;

    this->UpdateContainsOnlyWhitespace ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
WStringCR       CharStream::GetString                       () const                                                { return m_string; }

bool            CharStream::_IsSpace                        (size_t index) const                                    { return ((0x0020 == m_string[index]) || (0x00A0 == m_string[index])); }
size_t          CharStream::_GetCharacterCount              () const                                                { return m_string.length (); }
RunP            CharStream::_Clone                          () const                                                { return new CharStream (*this); }
bool            CharStream::_IsLastCharSpace                () const                                                { return false; }
bool            CharStream::_ContainsOnlyWhitespace         () const                                                { return m_containsOnlyWhitespace; }
void            CharStream::_GetEdfMaskForLayout            (size_t offset,
                                                                size_t length,
                                                                DgnGlyphLayoutContext::T_EdfMask& edfMask) const    { edfMask.resize (length, false); }
void            CharStream::_SetRunLayoutFlags              (DgnGlyphRunLayoutFlags value)                          { T_Super::_SetRunLayoutFlags (value); m_isDirty = true; }

bool            CharStream::IsSpace                         (size_t iChar) const                                    { return this->_IsSpace (iChar); }
void            CharStream::SetString                       (WStringCR value)                                       { this->_SetString (value); }
TextFieldDataP  CharStream::GetFieldData                    () const                                                { return m_fieldData.get (); }
void            CharStream::SetFieldData                    (TextFieldDataP value)                                  { m_fieldData = value; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
void CharStream::UpdateContainsOnlyWhitespace ()
    {
    m_containsOnlyWhitespace = true;

    for (WString::iterator iter = m_string.begin (); iter != m_string.end (); ++iter)
        {
        // Don't allow to break in the middle of an EDF. An EDF must reside entirely in a single text element, and thus cannot be split.
        if (0x20 != *iter)
            {
            m_containsOnlyWhitespace = false;
            return;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void CharStream::GetNextWordChars (WStringR nextWordChars, size_t offset) const
    {
    size_t nextWordIndex = GetNextWordBreakIndex (offset);
    nextWordChars = m_string.substr (0, ((nextWordIndex == m_string.length ()) ? m_string.length () : nextWordIndex + 1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void CharStream::LayoutGlyphs (size_t offset, size_t length, DRange3dR nominalRange, DRange3dR exactRange, DRange3dP elementRange) const
    {
    this->LayoutGlyphs (offset, length, nominalRange, exactRange, elementRange, NULL, NULL, NULL, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void CharStream::LayoutGlyphs
(
size_t          offset,
size_t          length,
DRange3dR       cellBoxRange,
DRange3dR       blackBoxRange,
DRange3dP       elementRange,
T_DoubleVectorP leadingCharacterOffsets,
T_DoubleVectorP trailingCharacterOffsets,
double*         maxHorizontalCellIncrement,
double*         trailingInterCharacterSpacing
) const
    {
    cellBoxRange.low.Zero ();
    cellBoxRange.high.Zero ();
    blackBoxRange.low.Zero ();
    blackBoxRange.high.Zero ();
    
    if (NULL != maxHorizontalCellIncrement)
        *maxHorizontalCellIncrement = 0.0;
    
    if (NULL != trailingInterCharacterSpacing)
        *trailingInterCharacterSpacing = 0.0;

    if (0 == length)
        return;
    
    DgnFontCP   font        = &m_properties.GetFont ();
    DgnFontCP   shxBigFont  = m_properties.ResolveShxBigFontCP ();
    
    font = font->ResolveToRenderFont ();
    
    if ((NULL != shxBigFont) && ((DGNFONTVARIANT_ShxBig != shxBigFont->GetVariant ()) || (DgnFontType::Shx != font->GetType ())))
        shxBigFont = NULL;
    
    if (NULL != shxBigFont)
        shxBigFont = shxBigFont->ResolveToRenderFont ();
    
    DgnGlyphLayoutContext::T_EdfMask edfMask;
    this->_GetEdfMaskForLayout (offset, length, edfMask);
    
    auto effectiveString = m_string.substr(offset, length);
    
    DgnGlyphLayoutContext layoutContext(m_properties.GetFont(), m_properties.GetShxBigFontCP());
    layoutContext.SetFontChars(effectiveString.c_str(), &edfMask[0], effectiveString.size());
    layoutContext.SetPropertiesFromRunPropertiesBase(m_properties);
    layoutContext.SetRunLayoutFlags(m_layoutFlags);

    // TextBlock layout needs to occur before display shifts are done... this is the definition of a display-time shift.
    layoutContext.SetShouldIgnoreDisplayShifts (true);

    // When the TextBlock is used for editing, we need to show the underlying characters; the percents are normally escaped at display time.
    layoutContext.SetShouldIgnorePercentEscapes (this->IsForEditing ());

    DgnGlyphLayoutResult layoutResult;
    if (SUCCESS != font->LayoutGlyphs(layoutContext, layoutResult))
        return;
    
    cellBoxRange.low.Init (layoutResult.GetCellBoxRangeR ().low);
    cellBoxRange.high.Init (layoutResult.GetCellBoxRangeR ().high);
    blackBoxRange.low.Init (layoutResult.GetJustificationBlackBoxRangeR ().low);
    blackBoxRange.high.Init (layoutResult.GetJustificationBlackBoxRangeR ().high);
    
    if (NULL != elementRange)
        {
        DRange2d elementRange2D = layoutResult.ComputeElementRange (layoutContext);
        elementRange->low.Init (elementRange2D.low);
        elementRange->high.Init (elementRange2D.high);
        }

    if (this->GetProperties ().IsSubScript () || this->GetProperties ().IsSuperScript ())
        {
        cellBoxRange.low.y      /= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
        cellBoxRange.high.y     /= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
        blackBoxRange.low.y     /= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
        blackBoxRange.high.y    /= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
        }

    if (NULL != leadingCharacterOffsets)
        {
        leadingCharacterOffsets->clear ();
        leadingCharacterOffsets->insert (leadingCharacterOffsets->end (), layoutResult.GetLeadingCaretOffsetsR ().begin (), layoutResult.GetLeadingCaretOffsetsR ().end ());
        }

    if (NULL != trailingCharacterOffsets)
        {
        trailingCharacterOffsets->clear ();
        trailingCharacterOffsets->insert (trailingCharacterOffsets->end (), layoutResult.GetTrailingCaretOffsetsR ().begin (), layoutResult.GetTrailingCaretOffsetsR ().end ());
        }
    
    if (NULL != maxHorizontalCellIncrement)
        *maxHorizontalCellIncrement = layoutResult.GetMaxHorizontalCellIncrementR ();
    
    if (NULL != trailingInterCharacterSpacing)
        *trailingInterCharacterSpacing = layoutResult.GetTrailingInterCharacterSpacingR ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void CharStream::_ComputeRangeOfCharsWithoutSpaces (DRange3dR nominalRange, DRange3dR exactRange, size_t offset, size_t length, bool trimLeft, bool trimRight) const
    {
    size_t lastCharIndex = (offset + length - 1);
    
    if (trimLeft)
        while (iswspace (m_string[offset]) && offset < lastCharIndex)
            ++offset;
    
    size_t lastNonSpaceIdx = lastCharIndex;
    if (trimRight)
        while (iswspace (m_string[lastNonSpaceIdx]) && lastNonSpaceIdx > offset)
            --lastNonSpaceIdx;
    
    length = ((lastNonSpaceIdx - offset) + 1);
    
    this->LayoutGlyphs (offset, length, nominalRange, exactRange, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
bool CharStream::FitsInLine (LineCR line, ProcessContextCR processContext, size_t offset, size_t length, bool isFirstFitAttempt)
    {
    TextBlockCP textBlock   = processContext.GetTextBlock ();
    ParagraphCP paragraph   = processContext.GetParagraph ();

    if ((textBlock->GetProperties ().GetMaxCharactersPerLine () > 0) && ((line.GetNumberOfChars () + length) > textBlock->GetProperties ().GetMaxCharactersPerLine ()) && (!isFirstFitAttempt || line.AllowTrailingWordBreak ()))
        return false;

    if (0.0 == textBlock->GetProperties ().GetDocumentWidth ())
        return true;
    
    TextElementJustification    paragraphJustification  = paragraph->GetProperties ().GetJustification ();
    HorizontalJustification     hJust;
    
    TextBlock::GetHorizontalVerticalJustifications (&hJust, NULL, paragraphJustification);

    // NEEDS_WORK: Check the differences between the vertical and horizontal algorithms.
    DRange3d    nextWordNominalRange;
    DRange3d    nextWordExactRange;
    
    _ComputeRangeOfCharsWithoutSpaces (nextWordNominalRange, nextWordExactRange, offset, length, (HORIZONTAL_JUSTIFICATION_Right == hJust), (HORIZONTAL_JUSTIFICATION_Left == hJust));

    if (nextWordExactRange.isNull ())
        return true;

    DRange3d    lineRange       = line.GetNominalRange ();
    DRange3d    nextWordRange   = TextBlockUtilities::ComputeJustificationRange (nextWordNominalRange, nextWordExactRange);
    double      indentation     = ((paragraph->GetLine (0) == &line) ? paragraph->GetProperties ().GetIndentation().GetFirstLineIndent () : paragraph->GetProperties ().GetIndentation().GetHangingIndent ());

    if (lineRange.IsNull ())
        memset (&lineRange, 0, sizeof (lineRange));

    if (textBlock->GetProperties ().IsVertical ())
        {
        // Remember, vertical text starts at y=0.0 and goes negative, so low.y is negative (but still indicates effective line length).
        double  lineHeight          = -lineRange.low.y;
        double  nextWordFitHeight   = -nextWordRange.low.y;

        return ((indentation + lineHeight + nextWordFitHeight) <= textBlock->GetProperties ().GetDocumentWidth ());
        }
    
    // Horizontal case.
    double  lineWidth           = lineRange.high.x;
    double  nextWordFitWidth    = nextWordRange.high.x;

    return ((indentation + lineWidth + nextWordFitWidth) <= textBlock->GetProperties ().GetDocumentWidth ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
double CharStream::_GetMaxDisplacementBelowOrigin () const
    {
    this->Preprocess ();
    return m_maxDisplacementBelowOrigin * m_properties.GetFontSize ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/08
//---------------------------------------------------------------------------------------
double CharStream::_GetMaxHorizontalCellIncrement () const
    {
    Preprocess ();
    
    return m_maxHorizontalCellIncrement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/05
//---------------------------------------------------------------------------------------
WString CharStream::_ToString (size_t offset, size_t length, TextBlockToStringOptionsCR options) const
    {
    DgnFontCR myFont = m_properties.GetFont ();
    
    if (!options.ShouldExpandRscFractions () || (DgnFontType::Rsc != myFont.GetType ()))
        return m_string.substr (offset, length);
    
    WString expandedStr;
    static_cast<DgnRscFontCR>(myFont).ExpandFractions (expandedStr, m_string.substr (offset, length).c_str ());
    
    return expandedStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
WChar CharStream::_GetCharacter (size_t index) const
    {
    if (index >= m_string.length ())
        return 0;

    return m_string[index];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void CharStream::_Splice (RunP& firstRun, RunP& secondRun, size_t index)
    {
    BeAssert (index <= m_string.length ());

    if (0 == index)
        {
        firstRun    = NULL;
        secondRun   = this;
        return;
        }

    if (m_string.length () == index)
        {
        firstRun    = this;
        secondRun   = NULL;
        return;
        }

    CharStreamP nextCharStream = new CharStream (m_properties.GetDgnModelR ());
    nextCharStream->m_properties = m_properties;
    nextCharStream->m_layoutFlags = m_layoutFlags;
    nextCharStream->m_fieldData = m_fieldData;
    nextCharStream->m_string.assign (m_string.begin () + index, m_string.end ());
    nextCharStream->UpdateContainsOnlyWhitespace ();
    nextCharStream->m_isDirty = true;

    m_string.erase (m_string.begin () + index, m_string.end ());
    this->UpdateContainsOnlyWhitespace ();
    
    m_isDirty   = true;
    firstRun    = this;
    secondRun   = nextCharStream;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
size_t CharStream::_GetNextWordBreakIndex (size_t offset) const
    {
    // This method used to return the index of the next breaking character (e.g. a space). The Uniscribe function will find the index of the next actual word. To convert, we must substract from the index, unless we are at zero, or the very end (where we technically don't know if we have to subtract or not).
    
    size_t nextWordBoundary = WordBoundaryServices::FindNextWordBoundary (WordBoundaryReason::WordWrapping, m_string.c_str (), m_string.length (), offset);

    if ((nextWordBoundary > 0) && (nextWordBoundary < m_string.length ()))
        --nextWordBoundary;

    return nextWordBoundary;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
AppendStatus CharStream::_AppendNextRunToLine (LineR line, RunP& nextRunIn, ProcessContextCR processContext)
    {
    // We only support literal CharStreams, and not sub-classes, so actually compare type_infos.
    
    if (typeid (CharStream) != typeid (*nextRunIn))
        return APPEND_STATUS_RunsNotMergeable;
    
    CharStreamP nextStream = static_cast <CharStreamP> (nextRunIn);

    if (!nextStream->GetProperties ().Equals (GetProperties ()))
        return APPEND_STATUS_RunsNotMergeable;

    if (m_fieldData.get () != nextStream->m_fieldData.get ())
        return APPEND_STATUS_RunsNotMergeable;
    
    TextBlockCP textBlock = processContext.GetTextBlock ();
    if (0 == textBlock->GetProperties ().GetMaxCharactersPerLine () && 0.0 == textBlock->GetProperties ().GetDocumentWidth ())
        {
        this->_Merge (nextStream);
        nextRunIn = NULL;
        return APPEND_STATUS_Appended;
        }
    
    size_t spliceIndex = nextStream->FindIndexOfFittableArrayOfChars (processContext, line);
    if (0 == spliceIndex)
        return APPEND_STATUS_Overflow;

    if (spliceIndex == nextStream->m_string.length ())
        {
        this->_Merge (nextStream);
        nextRunIn = NULL;
        return APPEND_STATUS_Appended;
        }

    RunP    firstRun    = NULL;
    RunP    secondRun   = NULL;
    
    nextStream->Splice (firstRun, secondRun, spliceIndex);
    
    this->_Merge (static_cast<CharStream*>(firstRun));
    
    nextRunIn = secondRun;

    return ((NULL != secondRun) ? APPEND_STATUS_Appended_Overflow : APPEND_STATUS_Appended);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
AppendStatus CharStream::_AppendToLine (LineR line, RunP& nextSplitRun, ProcessContextCR processContext)
    {
    BeAssert (m_string.length () != 0);
    
    size_t index = this->FindIndexOfFittableArrayOfChars (processContext, line);

    if (0 == index && !line.IsEmpty ())
        return APPEND_STATUS_Overflow;

    // NEEDS_WORK: TO TAKE CARE OF EMPTY Line case correctly
    if (0 == index)
        index = GetNextWordBreakIndex (0);

    RunP    firstRun    = NULL;
    RunP    secondRun   = NULL;
    
    Splice (firstRun, secondRun, index);

    line.AddRun (this);
    
    if (NULL != (nextSplitRun = secondRun))
        return APPEND_STATUS_Appended_Overflow;

    return APPEND_STATUS_Appended;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
bool CharStream::_CanFit (LineR line, ProcessContextCR processContext)
    {
    WString nextWordChars;
    GetNextWordChars (nextWordChars, 0);

    return FitsInLine (line, processContext, 0, nextWordChars.length (), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void CharStream::_ComputeRange ()
    {
    this->Preprocess ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   05/07
//---------------------------------------------------------------------------------------
void CharStream::_Preprocess () const
    {
    if (!m_isDirty)
        return;

    m_maxDisplacementBelowOrigin = GetProperties ().ResolveFont ().GetDescenderRatio ();
    
    this->LayoutGlyphs (0, m_string.length (), m_nominalRange, m_exactRange, &m_elementRange, &m_leadingCharacterOffsets, &m_trailingCharacterOffsets, &m_maxHorizontalCellIncrement, &m_trailingInterCharacterSpacing);

    if (m_trailingCharacterOffsets.empty())
        return;

    m_computedTrailingCharacterOffset = m_trailingCharacterOffsets.back ();

    if (this->IsLastRunInLine ())
        {
        if (this->IsVertical ())
            m_nominalRange.low.y += m_trailingInterCharacterSpacing;
        else
            m_nominalRange.high.x -= m_trailingInterCharacterSpacing;
        }

    m_isDirty = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
void CharStream::_GetRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    this->Preprocess ();

    if (m_string.empty () || (fromIdx >= m_string.length ()) || (toIdx > m_string.length ()) || (fromIdx >= toIdx))
        return;
    
    DPoint2d runOffset = this->GetProperties ().GetRunOffset ();

    for (size_t iChar = fromIdx; iChar < toIdx; ++iChar)
        {
        // Start a new range if we need the initial one, or we've changed directions. Ranges start on leading offsets.
        if (ranges.empty () || (((m_trailingCharacterOffsets[iChar] - m_leadingCharacterOffsets[iChar]) * (m_trailingCharacterOffsets[iChar - 1] - m_leadingCharacterOffsets[iChar - 1])) < 0.0))
            {
            ranges.push_back (this->GetNominalRange ());

            ranges.back ().low.x    = (m_leadingCharacterOffsets[iChar] + runOffset.x);
            ranges.back ().low.y    += runOffset.y;
            ranges.back ().high.x   = (m_leadingCharacterOffsets[iChar] + runOffset.x);
            ranges.back ().high.y   += runOffset.y;
            }

        // And iteratively expand to the trailing offsets.
        double newPos = (m_trailingCharacterOffsets[iChar] + this->GetProperties ().GetRunOffset ().x);
        
        if (newPos > ranges.back ().high.x)
            ranges.back ().high.x = newPos;
        else if (newPos < ranges.back ().low.x)
            ranges.back ().low.x = newPos;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/07
//---------------------------------------------------------------------------------------
void CharStream::_GetTransformedRangesForSelection (bvector<DRange3d>& ranges, size_t fromIdx, size_t toIdx) const
    {
    this->GetRangesForSelection (ranges, fromIdx, toIdx);
    
    for (bvector<DRange3d>::iterator rangesIter = ranges.begin (); ranges.end () != rangesIter; ++rangesIter)
        this->GetTransform ().multiply (&(*rangesIter).low, 2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
double CharStream::_ComputeLeftEdgeAlignDistance () const
    {
    return (m_exactRange.isNull () ? 0.0 : m_exactRange.low.x);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void CharStream::ComputeWhiteSpaceSymbolOrigin (DPoint3dR symbolOrigin, double& effectiveFontSize, size_t iChar) const
    {
    effectiveFontSize = this->GetProperties ().GetFontSize ().y;

    if (this->GetProperties ().IsSubScript () || this->GetProperties ().IsSuperScript ())
        effectiveFontSize *= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
        
    symbolOrigin.Zero ();
        
    symbolOrigin.y = (effectiveFontSize / 2.0);

    if (this->GetProperties ().IsSubScript ())
        symbolOrigin.y -= (this->GetProperties ().GetFontSize ().y * RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT);
    if (this->GetProperties ().IsSuperScript ())
        symbolOrigin.y += (this->GetProperties ().GetFontSize ().y * (1.0 - RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT));

    symbolOrigin.x = ((m_leadingCharacterOffsets[iChar] + m_trailingCharacterOffsets[iChar]) / 2.0);
        
    symbolOrigin.x += this->GetProperties ().GetRunOffset ().x;
    symbolOrigin.y += this->GetProperties ().GetRunOffset ().y;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void CharStream::DrawWhiteSpace (ViewContextR context) const
    {
    for (size_t iChar = 0; iChar < m_string.length (); ++iChar)
        {
        WChar currChar = m_string[iChar];
        
        // Space characters get a dot.
        if (0x0020 == currChar)
            {
            DPoint3d    symbolOrigin;
            double      effectiveFontSize;
            this->ComputeWhiteSpaceSymbolOrigin (symbolOrigin, effectiveFontSize, iChar);
            
            ElemDisplayParamsP elParams = context.GetCurrentDisplayParams ();
            elParams->SetWeight (1);
            context.CookDisplayParams ();
    
            context.GetIDrawGeom().DrawPointString3d (1, &symbolOrigin, NULL);

            continue;
            }
        
        // Other whitespace is not typically in a CharStream; draw slightly differently. This should also catch some Middle Eastern control characters.
        //  iswspace is only documented as handling (0x0009 – 0x000D or 0x0020)... sigh... so throwing more in on an as-needed basis.
        if (iswspace (currChar) || ((currChar >= 0x2000) && (currChar < 0x2010)) || ((currChar >= 0x2028) && (currChar < 0x2030)))
            {
            DPoint3d    symbolOrigin;
            double      effectiveFontSize;
            this->ComputeWhiteSpaceSymbolOrigin (symbolOrigin, effectiveFontSize, iChar);
            
            double      boxHalfWidth    = (effectiveFontSize / 12.0);
            double      boxHalfHeight   = (effectiveFontSize / 8.0);
            DPoint3d    boxPts[]        =
                {
                { symbolOrigin.x - boxHalfWidth, symbolOrigin.y - boxHalfHeight, symbolOrigin.z },
                { symbolOrigin.x - boxHalfWidth, symbolOrigin.y + boxHalfHeight, symbolOrigin.z },
                { symbolOrigin.x + boxHalfWidth, symbolOrigin.y + boxHalfHeight, symbolOrigin.z },
                { symbolOrigin.x + boxHalfWidth, symbolOrigin.y - boxHalfHeight, symbolOrigin.z },
                { symbolOrigin.x - boxHalfWidth, symbolOrigin.y - boxHalfHeight, symbolOrigin.z }
                };
            
            ElemDisplayParamsP elParams = context.GetCurrentDisplayParams ();
            elParams->SetWeight (0);
            context.CookDisplayParams ();
    
            context.GetIDrawGeom().DrawShape3d (5, boxPts, false, NULL);

            continue;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/07
//---------------------------------------------------------------------------------------
void CharStream::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    this->Draw (context, isViewIndependent, StackedFractionType::None, StackedFractionSection::None, options);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
void CharStream::Draw (ViewContextR context, bool isViewIndependent, StackedFractionType fractionType, StackedFractionSection fractionSection, TextBlockDrawOptionsCR options) const
    {
    Transform tform;
    tform.InitFrom (m_orientation, this->GetLineOffsetAdjustedOrigin ());
    
    context.PushTransform (tform);
        {
        TextStringProperties textStringProperties (m_properties, m_layoutFlags, false, isViewIndependent, true, TextElementJustification::LeftTop, fractionType, fractionSection);
    
        textStringProperties.SetIsPartOfField (textStringProperties.IsPartOfField () || m_fieldData.IsValid ());
        TextString textString (m_string.c_str (), NULL, NULL, textStringProperties);
        textString.SetShouldIgnorePercentEscapes (this->IsForEditing ());
        context.DrawTextString (textString);
    
        if (options.ShouldDrawWhiteSpace ())
            this->DrawWhiteSpace (context);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/08
//---------------------------------------------------------------------------------------
bool CharStream::_AllowTrailingWordBreak () const
    {
    // The point of this method is to determine if a following run can break. This resolves the issue of two adjacent CharStreams forming a single word (i.e. they have different formatting).
    if (0 == m_string.length ())
        return true;

    WString fakeString = m_string + L"A";
    return (WordBoundaryServices::FindNextWordBoundary (WordBoundaryReason::WordWrapping, fakeString.c_str (), fakeString.length (), (fakeString.length () - 2)) == (fakeString.length () - 1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static bool doesCharMatch (DgnFontCR font, wchar_t const& uniChar, UInt16 charCode)
    {
    if (LangCodePage::Unicode != font.GetCodePage ())
        return font.RemapUnicodeCharToFontChar (uniChar) == charCode;
    
    return ((UInt16)uniChar == charCode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
static wchar_t getCharForUnicode (DgnFontCR font, UInt16 charCode)
    {
    if (LangCodePage::Unicode != font.GetCodePage ())
        return font.RemapFontCharToUnicodeChar (charCode);
    
    return charCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/07
//---------------------------------------------------------------------------------------
void CharStream::_SetProperties (RunPropertiesCR updatedRunProperties)
    {
    DgnFontCR   lhsFont = this->GetProperties ().GetFontForCodePage ();
    DgnFontCR   rhsFont = updatedRunProperties.GetFontForCodePage ();
    
    if (&lhsFont != &rhsFont)
        {
        for (wchar_t& uniChar: m_string)
            {
            if (doesCharMatch (lhsFont, uniChar, lhsFont.RemapFontCharToUnicodeChar (lhsFont.GetDegreeCharCode ())))    uniChar = getCharForUnicode (rhsFont, rhsFont.RemapFontCharToUnicodeChar (rhsFont.GetDegreeCharCode ()));
            if (doesCharMatch (lhsFont, uniChar, lhsFont.RemapFontCharToUnicodeChar (lhsFont.GetDiameterCharCode ())))  uniChar = getCharForUnicode (rhsFont, rhsFont.RemapFontCharToUnicodeChar (rhsFont.GetDiameterCharCode ()));
            if (doesCharMatch (lhsFont, uniChar, lhsFont.RemapFontCharToUnicodeChar (lhsFont.GetPlusMinusCharCode ()))) uniChar = getCharForUnicode (rhsFont, rhsFont.RemapFontCharToUnicodeChar (rhsFont.GetPlusMinusCharCode ()));
            }
        }
    
    m_properties    = updatedRunProperties;
    m_isDirty       = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/08
//---------------------------------------------------------------------------------------
void CharStream::_SetIsLastRunInLine (bool val)
    {
    bool previousValue = this->IsLastRunInLine ();

    T_Super::_SetIsLastRunInLine (val);

    if (val == previousValue)
        return;

    if (this->IsVertical ())
        m_nominalRange.low.y += val ? m_trailingInterCharacterSpacing : -m_trailingInterCharacterSpacing;
    else
        m_nominalRange.high.x -= val ? m_trailingInterCharacterSpacing : -m_trailingInterCharacterSpacing;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
size_t CharStream::FindIndexOfFittableArrayOfChars (ProcessContextCR processContext, LineR line)
    {
    // Short-circuit if no word-wrapping; this stuff is expensive!
    if ((0.0 == processContext.GetTextBlock ()->GetProperties ().GetDocumentWidth ()) && (0 == processContext.GetTextBlock ()->GetProperties ().GetMaxCharactersPerLine ()))
        return m_string.length ();
    
    if (this->FitsInLine (line, processContext, 0, m_string.length (), true))
        return m_string.length ();

    size_t index = 0;

    // Force a fit if the line is empty or only contains whitespace.
    bool forceFit = true;
    
    for (size_t i = 0; i < line.GetRunCount (); ++i)
        {
        RunCR run = *line.GetRun (i);
        
        if (NULL != dynamic_cast<FractionCP>(&run))
            {
            forceFit = false;
            break;
            }

        for (size_t charIndex = 0; charIndex < run.GetCharacterCount (); charIndex++)
            {
            if (0x20 != run.GetCharacter (charIndex))
                {
                forceFit = false;
                break;
                }
            }
        
        if (!forceFit)
            break;
        }
    
    bool isFirstFitAttempt = true;

    for (;;)
        {
        WString nextWordChars;
        GetNextWordChars (nextWordChars, index);

        // The first word always fits on a line.
        if (forceFit)
            {
            forceFit = false;
            }
        else
            {
            if (!FitsInLine (line, processContext, 0, nextWordChars.length (), isFirstFitAttempt))
                return index;
            }

        index               = nextWordChars.length ();
        isFirstFitAttempt   = false;
        
        if (nextWordChars.length () >= GetCharacterCount ())
            break;
        }

    return index;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/07
//---------------------------------------------------------------------------------------
void CharStream::_Merge (CharStreamP nextCharStream)
    {
    m_string.insert (m_string.end (), nextCharStream->m_string.begin (), nextCharStream->m_string.end ());
    
    this->UpdateContainsOnlyWhitespace ();

    m_isDirty = true;

    delete nextCharStream;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
BentleyStatus CharStream::GetFraction (UInt8& numerator, UInt8& denominator, size_t i) const
    {
    DgnFontCR font = this->GetProperties ().GetFont ();
    if (DgnFontType::Rsc != font.GetType ())
        return ERROR;
    
    return static_cast<DgnRscFontCR>(font).FontCharToFraction (numerator, denominator, font.RemapUnicodeCharToFontChar (m_string[i]));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     03/08
//---------------------------------------------------------------------------------------
void CharStream::_SetString (WStringCR newChars)
    {
    m_string = newChars;
    this->UpdateContainsOnlyWhitespace ();
    m_isDirty = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2009
//---------------------------------------------------------------------------------------
bool CharStream::_Equals (RunCR rhsRun, TextBlockCompareOptionsCR compareOptions) const
    {
    CharStreamCP rhs = dynamic_cast<CharStreamCP>(&rhsRun);
    if (NULL == rhs)
        return false;
    
    if (!T_Super::_Equals (rhsRun, compareOptions))
        return false;
    
    if (!compareOptions.ShouldIgnoreInternalState ())
        {
        if (m_isDirty != rhs->m_isDirty) return false;
        }
    
    if (!compareOptions.ShouldIgnoreCachedValues ())
        {
        if (m_containsOnlyWhitespace != rhs->m_containsOnlyWhitespace) return false;
        
        if (!compareOptions.AreDoublesEqual (m_maxDisplacementBelowOrigin,      rhs->m_maxDisplacementBelowOrigin))     return false;
        if (!compareOptions.AreDoublesEqual (m_maxHorizontalCellIncrement,      rhs->m_maxHorizontalCellIncrement))     return false;
        if (!compareOptions.AreDoublesEqual (m_trailingInterCharacterSpacing,   rhs->m_trailingInterCharacterSpacing))  return false;
        }
    
    if (m_string != rhs->m_string) return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2010
//---------------------------------------------------------------------------------------
BentleyStatus CharStream::_ComputeCaretAtLocation (CaretR caret, DPoint3dCR locationIn, bool isStrict) const
    {
    Transform transform = this->GetTransform ();
    transform.inverseOf (&transform);

    DPoint3d location = locationIn;
    transform.multiply (&location);

    location.x -= this->GetProperties ().GetRunOffset ().x;
    location.y -= this->GetProperties ().GetRunOffset ().y;

    size_t                          iCharIndex              = 0;
    T_DoubleVector::const_iterator  minLeadingOffsetIter    = min_element (m_leadingCharacterOffsets.begin (), m_leadingCharacterOffsets.end ());

    if (location.x < *minLeadingOffsetIter)
        {
        if (isStrict)
            return ERROR;

        iCharIndex = (size_t)(minLeadingOffsetIter - m_leadingCharacterOffsets.begin ());
        }
    else
        {
        bool foundChar = false;

        for (; iCharIndex < m_string.length (); ++iCharIndex)
            {
            if (this->IsVertical ())
                {
                if ((location.y >= std::min (m_leadingCharacterOffsets[iCharIndex], m_trailingCharacterOffsets[iCharIndex]))
                    && (location.y < std::max (m_leadingCharacterOffsets[iCharIndex], m_trailingCharacterOffsets[iCharIndex])))
                    {
                    foundChar = true;
                    break;
                    }
                }
            else
                {
                if ((location.x >= std::min (m_leadingCharacterOffsets[iCharIndex], m_trailingCharacterOffsets[iCharIndex]))
                    && (location.x < std::max (m_leadingCharacterOffsets[iCharIndex], m_trailingCharacterOffsets[iCharIndex])))
                    {
                    foundChar = true;
                    break;
                    }
                }
            }

        if (isStrict && !foundChar)
            return ERROR;
        }

    caret.SetApproachDirection (CARET_MOTION_DIRECTION_VisualLeading);

    if (this->IsVertical ())
        {
        if (fabs (location.y - m_trailingCharacterOffsets[iCharIndex]) < fabs (location.y - m_leadingCharacterOffsets[iCharIndex]))
            ++iCharIndex;
        }
    else
        {
        if (fabs (location.x - m_trailingCharacterOffsets[iCharIndex]) < fabs (location.x - m_leadingCharacterOffsets[iCharIndex]))
            {
            if (m_trailingCharacterOffsets[iCharIndex] != m_leadingCharacterOffsets[iCharIndex + 1])
                caret.SetApproachDirection (CARET_MOTION_DIRECTION_VisualTrailing);
            else                
                ++iCharIndex;
            }
        }

    caret.SetCharacterIndex (iCharIndex);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// See http://msdn.microsoft.com/en-us/library/dd317793%28v=VS.85%29.aspx
// @bsimethod                                                   Venkat.Kalyan   06/07
//---------------------------------------------------------------------------------------
void CharStream::_ComputeOffsetToChar (DVec3dR offset, double& scale, CaretCR caret) const
    {
    offset.zero ();

    size_t charIndex = caret.GetCharacterIndex ();

    if (charIndex > m_string.length ())
        charIndex = m_string.length ();
    
    T_DoubleVectorCP offsets = &m_leadingCharacterOffsets;

    switch (caret.GetApproachDirection ())
        {
        case CARET_MOTION_DIRECTION_Forward:
        case CARET_MOTION_DIRECTION_VisualTrailing:
            {
            // Use leading edge for first character (trailing edge of character -1 is the first character's leading edge).
            if (0 == charIndex)
                break;
            
            // Otherwise use the trailing edge of the character we just passed over.
            offsets = &m_trailingCharacterOffsets;
            
            if (CARET_MOTION_DIRECTION_Forward == caret.GetApproachDirection ())
                --charIndex;

            break;
            }
        
        case CARET_MOTION_DIRECTION_Backward:
        case CARET_MOTION_DIRECTION_VisualLeading:
            {
            // Always use leading edge unless off the end.
            if (m_string.length () == charIndex)
                {
                offsets = &m_trailingCharacterOffsets;
                --charIndex;
                }

            break;
            }
        
        default:
            {
            BeAssert (false);
            break;
            }
        }

    if (charIndex >= offsets->size ())
        {
        BeAssert (false && L"Tried to go off the end of the caret offset array.");
        charIndex = (offsets->size () - 1);
        }
    
    if (this->IsVertical ())
        offset.y = (*offsets)[charIndex];
    else
        offset.x = (*offsets)[charIndex];
    
    // Sub-/super-script and run offsets are display-time shifts; account for them manually here.
    if (this->GetProperties ().IsSubScript ())
        offset.y -= this->GetProperties ().GetFontSize ().y * RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT;
    else if (this->GetProperties ().IsSuperScript ())
        offset.y += this->GetProperties ().GetFontSize ().y * (1.0 - RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT);
    
    offset.x += m_properties.GetRunOffset ().x;
    offset.y += m_properties.GetRunOffset ().y;

    scale = this->GetProperties ().GetFontSize ().y;
    
    if (this->GetProperties ().IsSubScript () || this->GetProperties ().IsSuperScript ())
        scale *= RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SCALE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void CharStream::_ComputeTransformedHitTestRange (DRange3dR hitTestRange) const
    {
    hitTestRange = this->GetNominalRange ();

    if (this->GetProperties ().IsSubScript ())
        hitTestRange.low.y -= (this->GetProperties ().GetFontSize ().y * RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT);
    else if (this->GetProperties ().IsSuperScript ())
        hitTestRange.high.y += (this->GetProperties ().GetFontSize ().y * (1.0 - RunPropertiesBase::SUPERSCRIPT_SUBSCRIPT_SHIFT));
    
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
void CharStream::_SetOverrideTrailingCharacterOffset (double value)
    {
    m_trailingCharacterOffsets.back () = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2011
//---------------------------------------------------------------------------------------
void CharStream::_ClearTrailingCharacterOffsetOverride ()
    {
    m_trailingCharacterOffsets.back () = m_computedTrailingCharacterOffset;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void CharStream::_GetElementRange (DRange3dR value) const
    {
    value = m_elementRange;
    }


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- EdfCharStream ----------------------------------------------------------------------------------------------------------------- EdfCharStream --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static WString createPaddedEdfString (WCharCP value, size_t totalLength, EdfJustification edfJustification)
    {
    WString edfString (value);
    
    if (edfString.length () == totalLength)
        return edfString;
    
    if (edfString.length () > totalLength)
        {
        BeDataAssert (false && L"Attempted to create an enter data field with a value whose length exceeds the specific maximum.");
        return edfString.substr (0, totalLength);
        }

    switch (edfJustification)
        {
        case EdfJustification::Left:
            edfString.PadRight (totalLength, L' ');
            return edfString;

        case EdfJustification::Center:
            edfString.PadRight (totalLength - ((totalLength - edfString.length ()) / 2), L' ');
            edfString.PadLeft (totalLength, L' ');
            return edfString;
            
        case EdfJustification::Right:
            edfString.PadLeft (totalLength, L' ');
            return edfString;

        default:
            BeAssert (false && L"Unknown EDF justification value.");
            edfString.PadRight (totalLength, L' ');
            return edfString;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
EdfCharStream::EdfCharStream (WCharCP value, size_t totalLength, EdfJustification edfJustification, RunPropertiesCR props, DgnGlyphRunLayoutFlags layoutFlags) :
    CharStream          (createPaddedEdfString (value, totalLength, edfJustification).c_str (), props, layoutFlags),
    m_edfJustification  (edfJustification)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_Splice (RunP& firstRun, RunP& secondRun, size_t iChar)
    {
    if (0 == iChar)
        {
        firstRun    = NULL;
        secondRun   = this;
        return;
        }
    
    BeAssert ((m_string.length () == iChar) && L"Should not attempt to splice an EDF run.");
    
    firstRun    = this;
    secondRun   = NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
size_t EdfCharStream::_GetNextWordBreakIndex (size_t iChar) const
    {
    if (0 == iChar)
        return 0;

    return m_string.length ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
AppendStatus EdfCharStream::_AppendNextRunToLine (LineR, RunP& nextSplitRun, ProcessContextCR)
    {
    return APPEND_STATUS_RunsNotMergeable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
AppendStatus EdfCharStream::_AppendToLine (LineR line, RunP& nextRun, ProcessContextCR context)
    {
    if (!line.IsEmpty () && !this->FitsInLine (line, context, 0, m_string.size (), true))
        {
        nextRun = this;
        return APPEND_STATUS_Overflow;
        }
    
    line.AddRun (this);
    
    return APPEND_STATUS_Appended;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool EdfCharStream::_CanFit (LineR line, ProcessContextCR context)
    {
    return this->FitsInLine (line, context, 0, m_string.size (), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
RunP EdfCharStream::_Clone () const
    {
    return new EdfCharStream (m_string.c_str (), m_string.length (), m_edfJustification, m_properties, m_layoutFlags);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_Draw (ViewContextR context, bool isViewIndependent, TextBlockDrawOptionsCR options) const
    {
    Transform tform;
    tform.InitFrom (m_orientation, this->GetLineOffsetAdjustedOrigin ());
    
    context.PushTransform (tform);
        {
        TextEDField             edField;
        TextString              textString;
        TextStringProperties    props           (m_properties, m_layoutFlags, false, isViewIndependent, true, TextElementJustification::LeftTop, StackedFractionType::None, StackedFractionSection::None);
        DPoint3d                origin;         origin.Zero ();
        RotMatrix               orientation;    orientation.InitIdentity ();

        edField.start   = 1;
        edField.len     = (byte)m_string.length ();
        edField.just    = (byte)m_edfJustification;
    
        textString.Init (m_string.c_str (), origin, orientation, props, 1, &edField, 0);
        context.DrawTextString (textString);
    
        if (options.ShouldDrawWhiteSpace ())
            this->DrawWhiteSpace (context);
        }
    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool EdfCharStream::_AllowTrailingWordBreak () const
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_Merge (CharStreamP)
    {
    BeAssert (false && L"Should not attempt to merge an EDF run.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_SetString (WStringCR value)
    {
    m_string    = createPaddedEdfString (value.c_str (), m_string.length (), m_edfJustification);
    m_isDirty   = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool EdfCharStream::_Equals (RunCR rhsRun, TextBlockCompareOptionsCR options) const
    {
    EdfCharStreamCP rhs = dynamic_cast<EdfCharStreamCP>(&rhsRun);
    if (NULL == rhs)
        return false;
    
    if (!T_Super::_Equals (*rhs, options))
        return false;

    if (m_edfJustification != rhs->m_edfJustification)  return false;
    
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_GetEdfMaskForLayout (size_t offset, size_t length, DgnGlyphLayoutContext::T_EdfMask& edfMask) const
    {
    edfMask.resize (length, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
bool EdfCharStream::_IsAtomic () const
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
void EdfCharStream::_ComputeRangeOfCharsWithoutSpaces (DRange3dR nominalRange, DRange3dR exactRange, size_t offset, size_t length, bool trimLeft, bool trimRight) const
    {
    // Never allowed to trim either side of an EDF.
    T_Super::_ComputeRangeOfCharsWithoutSpaces (nominalRange, exactRange, offset, length, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2010
//---------------------------------------------------------------------------------------
WString EdfCharStream::_ToString (size_t offset, size_t length, TextBlockToStringOptionsCR options) const
    {
    if (!options.ShouldSubstitueAtomicRunContent ())
        return T_Super::_ToString (offset, length, options);
    
    return WString (min (m_string.length (), length), options.GetAtomicRunContentSubstituteChar ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
EdfJustification EdfCharStream::GetEdfJustification () const
    {
    return m_edfJustification;
    }
