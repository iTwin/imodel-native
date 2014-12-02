/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/DgnTextElementProcessor.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

GLOBAL_TYPEDEF (ExportContext, ExportContext)

typedef bvector<int>            IntArray;
typedef bvector<WhiteSpaceCP>   WhiteSpaceArray;
typedef std::queue<ExportContext> ExportContextQueue;

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   06/2007
//=======================================================================================
struct ExportContext : public TextContext
    {
    private:    RunCP           m_run;
    private:    WhiteSpaceArray m_whiteSpaceArray;
    private:    Transform       m_transform;
    private:    TextParamWide   m_textParams;
    private:    DPoint2d        m_textSize;
    private:    DgnModelP    m_modelRef;
    private:    bool            m_needsIndentation;
    private:    EDFieldVector   m_edFields;
    private:    bool            m_isNextRunPartOfSameField;
    private:    bool            m_isFirstRunOfField;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan
    //---------------------------------------------------------------------------------------
    public: void Init ()
        {
        m_textParams.Initialize ();
        memset (&m_textSize, 0, sizeof (m_textSize));
        
        m_transform.initIdentity ();

        m_needsIndentation          = false;
        m_isNextRunPartOfSameField  = false;
        m_isFirstRunOfField         = false;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan
    //---------------------------------------------------------------------------------------
    public: ExportContext ()
        {
        Init ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan
    //---------------------------------------------------------------------------------------
    public: ExportContext (TextBlockCP textBlock, ParagraphCP paragraph, LineCP line, DgnModelP modelRef) :
        TextContext (textBlock, paragraph, line),
        m_modelRef  (modelRef)
        {
        Init ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     08/2010
    //---------------------------------------------------------------------------------------
    public: void SetRun (RunCP run)
        {
        m_run = run;
        
        if (NULL == m_run)
            return;
        
        m_run->GetProperties ().ToElementData (m_textParams, m_modelRef->GetDgnProject ());
        m_textSize = m_run->GetProperties ().GetFontSize ();
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan
    //---------------------------------------------------------------------------------------
    public: RunCP               GetRun                      () const                { return m_run; }
    public: WhiteSpaceArray&    GetWhiteSpaceArrayR         ()                      { return m_whiteSpaceArray; }
    public: TextParamWideR      GetTextParamsR              ()                      { return m_textParams; }
    public: DPoint2dR           GetTextSizeR                ()                      { return m_textSize; }
    public: TransformR          GetTransformR               ()                      { return m_transform; }
    public: DgnModelR        GetDgnModelR                () const                { return *m_modelRef; }
    public: DgnProjectR         GetDgnProject               () const                { return m_modelRef->GetDgnProject(); }
    public: EDFieldVectorR      GetEDFieldsR                ()                      { return m_edFields; }
    public: void                SetTransform                (TransformCR transform) { m_transform = transform; }
    public: void                SetDgnModel                 (DgnModelP modelRef) { m_modelRef = modelRef; }
    public: bool                NeedsIndentation            () const                { return m_needsIndentation; }
    public: void                SetNeedsIndentation         (bool needsIndentation) { m_needsIndentation = needsIndentation; }
    public: bool                IsNextRunPartOfSameField    () const                { return m_isNextRunPartOfSameField; }
    public: void                SetIsNextRunPartOfSameField (bool value)            { m_isNextRunPartOfSameField = value; }
    public: bool                IsFirstRunOfField           () const                { return m_isFirstRunOfField; }
    public: void                SetIsFirstRunOfField        (bool value)            { m_isFirstRunOfField = value; }

    }; // ExportContext

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
static void synchParametersWithParagraphParameters (TextParamWideR textParams, ParagraphPropertiesCR paraProps, bool isTextNode)
    {
    paraProps.ToElementData (textParams);
    
    if (isTextNode)
        return;

    textParams.lineSpacing                              = 0.0;
    textParams.overridesFromStyle.acadLineSpacingType   = false;
    textParams.overridesFromStyle.fullJustification     = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/05
//---------------------------------------------------------------------------------------
static void synchParametersWithTextBlockParameters (TextParamWideR textParams, DPoint2dR textScale, TextBlockCR textBlock, bool isTextNode)
    {
    textBlock.GetProperties ().ToElementData (textParams, NULL);

    textParams.exFlags.renderPercentsAsMText = (textBlock.IsMTextType () ? true : false);

    if (isTextNode)
        {
        if (NULL != textBlock.GetTextNodeRunProperties ())
            textScale = textBlock.GetTextNodeRunProperties ()->GetFontSize ();
        
        if (0.0 != textBlock.GetNodeHeight ())
            textScale.y = fabs (textBlock.GetNodeHeight ());
        
        textParams.nodeNumber                   = textBlock.GetNodeNumber ();
        textParams.textnodeWordWrapLength       = textBlock.GetProperties ().GetDocumentWidth ();
        textParams.exFlags.wordWrapTextNode     = (0.0 != textBlock.GetProperties ().GetDocumentWidth ());
        textParams.renderingFlags.documentType  = textBlock.GetType ();
        }
    else
        {
        textParams.nodeNumber               = 0;
        textParams.textnodeWordWrapLength   = 0.0;
        textParams.exFlags.wordWrapTextNode = false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
static void adjustRotMatrixForBackwardsUpsideDown (RotMatrixR rotation, bool backwards, bool upsideDown)
    {
    if (backwards)
        {
        RotMatrix   flipMatrix;
        DVec3d      yVec;
        
        yVec.init (0.0, 1.0, 0.0);
        
        flipMatrix.initFromVectorAndRotationAngle (&yVec, msGeomConst_radiansPerDegree * 180.0);
        rotation.productOf (&rotation, &flipMatrix);
        }

    if (upsideDown)
        {
        RotMatrix   flipMatrix;
        DVec3d      xVec;
        
        xVec.init (1.0, 0.0, 0.0);
        
        flipMatrix.initFromVectorAndRotationAngle (&xVec, msGeomConst_radiansPerDegree * 180.0);
        rotation.productOf (&rotation, &flipMatrix);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
static void adjustWidthAndHeightForBackwardsUpsideDown (double& width, double& height, bool backwards, bool upsideDown)
    {
    if (backwards && width > 0.0)
        width *= -1.0;

    if (upsideDown && height > 0.0)
        height *= -1.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/06
//---------------------------------------------------------------------------------------
static bool shouldCreate3dElement (DgnModelR modelRef, ElementHandleCP templateEh)
    {
    if (NULL == templateEh)
        return modelRef.Is3d ();
    
    return DisplayHandler::Is3dElem (templateEh->GetElementCP ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   SamWilson       05/01
//---------------------------------------------------------------------------------------
static bool isRightLogicKBEnabled ()
    {
    static int s_rightLogicKB = -1;
    
    if (-1 == s_rightLogicKB)
        {
        WString rtlMode;
        
#if defined (WIP_CFGVAR) // MS_RIGHTLOGICKB
        if (SUCCESS != ConfigurationManager::GetVariable (rtlMode, L"MS_RIGHTLOGICKB"))
            s_rightLogicKB = 0;
        else
            s_rightLogicKB = (0 == BeStringUtilities::Wcsicmp (rtlMode.c_str (), L"ON"));
#else
        s_rightLogicKB = 0;
#endif
        }

    return (0 != s_rightLogicKB);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   HirooJumonji    02/94
//---------------------------------------------------------------------------------------
static bool detectLegacyRtlFlag (WCharCP uniString)
    {
    // The original version of this function took an MSWideChar, assumed it was 1255-encoded (Hebrew), and checked for Hebrew letters [E0..FA].
    //  These letters are [U+05D0..U+05EA] in Unicode.
    for (WCharCP wcP = uniString; (NULL != wcP); ++wcP)
        {
        if ((*wcP >= 0x05D0) && (*wcP <= 0x05EA))
            return true;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
static BentleyStatus createTextElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR origin, RotMatrixCR rotation, ExportContextR exportContext)
    {
    CharStreamCP charStream = dynamic_cast<CharStreamCP>(exportContext.GetRun ());
    if (NULL == charStream)
        { BeAssert (false); return ERROR; }
    
    size_t numChars = charStream->GetCharacterCount ();
    
    if (0 == numChars)
        return SUCCESS;

    DPoint2d                textSize            = exportContext.GetTextSizeR ();
    TextParamWideR          textParams          = exportContext.GetTextParamsR ();
    TextBlockCR             textBlock           = *exportContext.GetTextBlock ();
    ParagraphPropertiesCR   effectiveParaProps  = ((0 == textBlock.GetParagraphCount ()) ? textBlock.GetParagraphPropertiesForAdd () : textBlock.GetParagraph (0)->GetProperties ());

    synchParametersWithTextBlockParameters (textParams, textSize, *exportContext.GetTextBlock (), false);
    synchParametersWithParagraphParameters (textParams, effectiveParaProps, false);
    
    if (*((Int64*)&textParams.overridesFromStyle))
        textParams.exFlags.styleOverrides = true;

    WStringCR   unicodeString   = charStream->GetString ();
    DPoint3d    textOrigin      = origin;
    RotMatrix   textRotation    = rotation;
    bool        is3d            = shouldCreate3dElement (exportContext.GetDgnModelR (), templateEh);
    
    if (!is3d)
        {
        adjustWidthAndHeightForBackwardsUpsideDown (textSize.x, textSize.y, exportContext.GetTextBlock ()->GetProperties ().IsBackwards (), exportContext.GetTextBlock ()->GetProperties ().IsUpsideDown ());
        adjustRotMatrixForBackwardsUpsideDown (textRotation, exportContext.GetTextBlock ()->GetProperties ().IsBackwards (), exportContext.GetTextBlock ()->GetProperties ().IsUpsideDown ());
        }

    // Make the justification for margin text the same as v7 - not sure what this is for.
    switch ((TextElementJustification)textParams.just)
        {
        case TextElementJustification::LeftMarginBaseline:     textParams.just = (int)TextElementJustification::LeftBaseline;  break;
        case TextElementJustification::LeftMarginMiddle:       textParams.just = (int)TextElementJustification::LeftMiddle;    break;
        case TextElementJustification::LeftMarginTop:          textParams.just = (int)TextElementJustification::LeftTop;       break;
        case TextElementJustification::RightMarginBaseline:    textParams.just = (int)TextElementJustification::RightBaseline; break;
        case TextElementJustification::RightMarginMiddle:      textParams.just = (int)TextElementJustification::RightMiddle;   break;
        case TextElementJustification::RightMarginTop:         textParams.just = (int)TextElementJustification::RightTop;      break;
        }

    // Set up the legacy RTL flag; I am not confident enough that old versions are not using it in order to always leave it unset.
    if (isRightLogicKBEnabled ())
        {
        bool legacyRtlFlag = detectLegacyRtlFlag (unicodeString.c_str ());
        
        if (textParams.flags.rightToLeft_deprecated != legacyRtlFlag)
            {
            textParams.flags.rightToLeft_deprecated = legacyRtlFlag;

            // Technically it is bogus that this is considered part of the text style, since it is derived by the value of the string, and there is no UI to control it to begin with.
            textParams.overridesFromStyle.direction = (0 != textParams.textStyleId);
            }
        }
    
    BeAssert ((*(Int64*)&textParams.overridesFromStyle && textParams.exFlags.styleOverrides) || (!*(Int64*)&textParams.overridesFromStyle && !textParams.exFlags.styleOverrides));
    
    if (SUCCESS != TextElemHandler::CreateElement (eeh, templateEh, textOrigin, textRotation, textSize, textParams, unicodeString.c_str (), is3d, exportContext.GetDgnModelR ()))
        return ERROR;
    
    if (!exportContext.GetEDFieldsR ().empty ())
        TextElemHandler::SetEDFields (eeh, &exportContext.GetEDFieldsR ());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
static void appendEdfStruct (EDFieldVectorR edFields, byte start, byte len, byte just)
    {
    TextEDField edf;
            
    edf.start   = start;
    edf.len     = len;
    edf.just    = just;

    edFields.push_back (edf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2010
//---------------------------------------------------------------------------------------
static CharStreamP combineLikeFormattedAndEdfRuns (RunIteratorR runIter, RunIteratorCR end, EDFieldVectorR edFields)
    {
    // Need unique run (e.g. text element) positioning for full justification.
    if (runIter.GetCurrentParagraphCP ()->GetProperties ().IsFullJustified ())
        return NULL;

// BEIJING_DGNPLATFORM_WIP_Fields
    CharStreamCP firstCharStream = dynamic_cast<CharStreamCP>(&*runIter);
    if ((NULL == firstCharStream) || (NULL != firstCharStream->GetFieldData ()))
        return NULL;

    EdfCharStreamCP firstEdf = dynamic_cast<EdfCharStreamCP>(firstCharStream);
    if (NULL != firstEdf)
        appendEdfStruct (edFields, 1, (byte)firstEdf->GetString ().length (), (byte)firstEdf->GetEdfJustification ());
    
    LineCR      line                = *runIter.GetCurrentLineCP ();
    CharStreamP combinedCharStream  = NULL;
    RunIterator peekRunIter         = runIter;
    
    ++peekRunIter;
    for (; ((end != peekRunIter) && (&line == peekRunIter.GetCurrentLineCP ())); ++peekRunIter)
        {
        CharStreamCP currCharStream = dynamic_cast<CharStreamCP>(&*peekRunIter);
        if (NULL == currCharStream)
            return combinedCharStream;
        
        if (!firstCharStream->GetProperties ().Equals (currCharStream->GetProperties ()))
            return combinedCharStream;
        
// BEIJING_DGNPLATFORM_WIP_Fields
        if (NULL != currCharStream->GetFieldData ())
            return combinedCharStream;
        
        if (NULL == combinedCharStream)
            combinedCharStream = new CharStream (*firstCharStream);
        
        EdfCharStreamCP currEdf = dynamic_cast<EdfCharStreamCP>(currCharStream);
        if (NULL != currEdf)
            appendEdfStruct (edFields, (byte)combinedCharStream->GetString ().length () + 1, (byte)currEdf->GetString ().length (), (byte)currEdf->GetEdfJustification ());

        combinedCharStream->SetString (combinedCharStream->GetString () + currCharStream->GetString ());

        runIter = peekRunIter;
        }

    return combinedCharStream;
    }

// WIP_NONPORT - don't pass non-const reference to temporary object
static CharStreamP combineLikeFormattedAndEdfRuns (RunIteratorCR runIter, RunIteratorCR end, EDFieldVectorR edFields) {return combineLikeFormattedAndEdfRuns (const_cast<RunIteratorR>(runIter), const_cast<RunIteratorR>(end), edFields);}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
void TextBlock::CreateTextElement (EditElementHandleR eeh, ElementHandleCP templateEh, DgnModelP modelRef) const
    {
    Transform transform = this->GetLineTransformAtCaret (this->Begin ());

    ParagraphP  paragraph   = GetParagraph (0);
    LineP       line        = paragraph->GetLine (0);
    CharStreamP charStream  = dynamic_cast<CharStreamP>(line->GetRun (0));

    BeAssert (NULL != charStream);
    
    EditElementHandle processedTemplate;

    if (NULL != templateEh && TEXT_NODE_ELM == templateEh->GetLegacyType())
        {
        EditElementHandleP processedTemplate = (EditElementHandleP)_alloca (sizeof (EditElementHandle));
        TextHandlerBase::CreateTemplateTextElemFromNode (*processedTemplate, *templateEh);
        templateEh = processedTemplate;
        }

    RotMatrix rMatrix;
    transform.getMatrix (&rMatrix);

    DPoint3d origin = charStream->GetOrigin ();

    // We want a single text element. SHX aligns vertical glyphs to be horizontally centered,
    //  so adjust element origin so that it matches placement with other font types.
    
    if (m_properties.IsVertical () && IsDGNType () && DgnFontType::Shx == charStream->GetProperties ().GetFont ().GetType ())
        origin.x += charStream->GetExactWidth () / 2.0;

    transform.multiply (&origin);

    ExportContext exportContext (this, paragraph, line, modelRef);
    
    CharStreamP combinedCharStream = combineLikeFormattedAndEdfRuns (this->Begin ().CreateRunBegin (), this->End ().CreateRunEnd (), exportContext.GetEDFieldsR ());
    if (NULL != combinedCharStream)
        exportContext.SetRun (combinedCharStream);
    else
        exportContext.SetRun (charStream);
    
    exportContext.GetTextParamsR ().renderingFlags.documentType = m_type;

    createTextElement (eeh, templateEh, origin, rMatrix, exportContext);

    if (NULL != combinedCharStream)
        delete combinedCharStream;
    
    // 08.11 didn't do this, but also didn't force a text node... seems to me this should be allowed.
    if (!exportContext.GetParagraph ()->GetProperties ().GetIndentation ().IsDefault ())
        TextElemHandler::SetIndentationData (eeh, &exportContext.GetParagraph ()->GetProperties ().GetIndentation ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendAlongTextDependency (EditElementHandleR eeh) const
    {
    if (!eeh.IsValid () || NULL == eeh.GetElementDescrP ())
        return;

    if (0 == m_alongRootId)
        return;

    // Create the along text dependency. That will give us everything we need after that.
    AlongTextDependency atDep;
    if (SUCCESS != this->GetAlongTextDependency (atDep))
        { BeAssert (false); return; }

    if (atDep.IsValid (*eeh.GetDgnModelP ()))
        TextNodeHandler::SetAlongTextData (eeh, &atDep);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   07/05
//---------------------------------------------------------------------------------------
void getTextTemplates (EditElementHandleR textElemTemplateEeh, EditElementHandleR textNodeTemplateEeh, ElementHandleCP templateEh, UInt32 nodeNumber)
    {
    if (NULL == templateEh || !templateEh->IsValid ())
        return;

    // If a template has been supplied.
    if (TEXT_NODE_ELM == templateEh->GetLegacyType())
        {
        // Multi-line text node.
        TextHandlerBase::CreateTemplateTextElemFromNode (textElemTemplateEeh, *templateEh);
        
        textNodeTemplateEeh.SetElementDescr(new MSElementDescr(*(templateEh->GetElementCP()),*templateEh->GetDgnModelP ()), false);
        }
    else if (TEXT_ELM == templateEh->GetLegacyType())
        {
        TextHandlerBase::CreateTemplateNodeFromTextElem (textNodeTemplateEeh, *templateEh);
        TextNodeHandler::SetNodeNumber (textNodeTemplateEeh, nodeNumber);
        
        textElemTemplateEeh.SetElementDescr(new MSElementDescr(*(templateEh->GetElementCP()),*templateEh->GetDgnModelP ()), false);
        }
    else
        {
        BeAssert (false);
        return;
        }

    // We want all ID's to be zero. Don't rely on any auxilliary functions to come back later and
    //  zero ID's out. By setting the template ID's to 0, it will be mem-copied into the new elements.
    //  As a side note, there also currently exists a function very later on specific to text elements
    //  during a replace in the cache that will attempt to match up ID's.
    textNodeTemplateEeh.GetElementP()->InvalidateElementId();
    textElemTemplateEeh.GetElementP()->InvalidateElementId();

    // We don't want linkages on the parent text node to be copied to child text elements.
    ElementLinkageIterator lIter;
    while (textElemTemplateEeh.EndElementLinkages () != (lIter = textElemTemplateEeh.BeginElementLinkages (0)))
        textElemTemplateEeh.RemoveElementLinkage (lIter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/04
//---------------------------------------------------------------------------------------
void TextBlock::CreateAlongText (EditElementHandleR eeh, ElementHandleCP templateEh, DgnModelP modelRef) const
    {
    RunCP run = GetParagraph (0)->GetLine (0)->GetRun (0);
    if (NULL == run)
        return;

    EditElementHandle  textElemTemplateEh;
    EditElementHandle  textNodeTemplateEh;
    
    getTextTemplates (textElemTemplateEh, textNodeTemplateEh, templateEh, m_nodeNumber);

    RotMatrix       rMatrix             = m_orientation;
    DPoint3d        origin              = run->GetOrigin ();
    UShort          lineLength          = 0;
    TextParamWide   textNodeParams;
    DPoint2d        textNodeSize;
    RunPropertiesCP runProperties       = ((NULL == m_nodeProperties) ? GetFirstRunProperties () : m_nodeProperties);

    textNodeSize = runProperties->GetFontSize ();
    
    runProperties->ToElementData (textNodeParams, modelRef->GetDgnProject ());

    synchParametersWithTextBlockParameters (textNodeParams, textNodeSize, *this, true);
    synchParametersWithParagraphParameters (textNodeParams, this->GetParagraph (0)->GetProperties (), true);
    
    if (*((Int64*) &textNodeParams.overridesFromStyle))
        textNodeParams.exFlags.styleOverrides = true;

    if (SUCCESS != TextNodeHandler::CreateElement (eeh, templateEh ? &textNodeTemplateEh : NULL, origin, rMatrix, textNodeSize, textNodeParams, lineLength, shouldCreate3dElement (*modelRef, templateEh), *modelRef))
        return;
    
    AppendComponentTextElementsToNode (this->Begin ().CreateRunBegin (), eeh, (templateEh ? &textElemTemplateEh : NULL), (templateEh ? &textNodeTemplateEh : NULL), modelRef);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
static void getWhiteSpacesUntilNextCharStream (RunIteratorR runIter, RunIteratorCR end, WhiteSpaceArray& whiteSpaceArray)
    {
    WhiteSpaceCP whiteSpace;
    
    for (; ((end != runIter) && (NULL != (whiteSpace = dynamic_cast<WhiteSpaceCP>(&*runIter)))); ++runIter)
        whiteSpaceArray.push_back (whiteSpace);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
static void createBitMaskOrGetCRCountForWhiteSpaceArray (WhiteSpaceBitmaskValueVectorR wsVector, int* crCountIn, WhiteSpaceArray const & whiteSpaceArray, bool v7Compatible)
    {
    wsVector.clear ();
    
    if (NULL != crCountIn)
        *crCountIn = 0;

    size_t nWhiteSpaces    = whiteSpaceArray.size ();
    int crCount         = 0;
    
    if (v7Compatible && NULL != crCountIn)
        {
        for (size_t i = 0; i < nWhiteSpaces; i++)
            {
            if (NULL != dynamic_cast <ParagraphBreakCP> (whiteSpaceArray[i]))
                crCount++;
            }
        
        *crCountIn = crCount;
        return;
        }

    if (!v7Compatible)
        {
        for (size_t j = 0; j < nWhiteSpaces; j++)
            {
            if (NULL != dynamic_cast<ParagraphBreakCP>(whiteSpaceArray[j]))
                {
                wsVector.push_back (WhiteSpaceBitmaskValue_ParagraphBreak);
                crCount++;
                }
            else if (NULL != dynamic_cast <TabCP> (whiteSpaceArray[j]))
                {
                wsVector.push_back (WhiteSpaceBitmaskValue_Tab);
                }
            else if (NULL != dynamic_cast <LineBreakCP> (whiteSpaceArray[j]))
                {
                wsVector.push_back (WhiteSpaceBitmaskValue_LineBreak);
                }
            }

        if (crCount == nWhiteSpaces && NULL != crCountIn)
            {
            *crCountIn = crCount;
            wsVector.clear ();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/05
//---------------------------------------------------------------------------------------
static void computeAdjustmentForLeftAlignment (DVec3dR adjustment, LineCP line, TextElementJustification justification)
    {
    adjustment.zero ();

    RunP run = line->GetRun (0);
    if (NULL == run)
        return;

    DRange3d runExactRange = run->GetExactRange ();
    if (!runExactRange.isNull ())
        adjustment.x = runExactRange.low.x;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   11/05
//---------------------------------------------------------------------------------------
static bool needsAlignEdgeFlag (bool firstUnitInLine, LineCR line, TextBlockCR textBlock)
    {
    if (textBlock.GetProperties ().IsVertical ())
        return false;

    if (!firstUnitInLine)
        return false;

    if (textBlock.IsDTextType () || textBlock.IsAlongElement ())
        return false;

    if (textBlock.IsMTextType ())
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
static void appendTextElementFromCharStream (EditElementHandleR eeh, ElementHandleCP textElemTemplate, ElementHandleCP textNodeTemplate, ExportContext& exportContext)
    {
    BeAssert (NULL != exportContext.GetRun ());
    BeAssert (0 != exportContext.GetRun ()->GetCharacterCount ());

    RotMatrix textRotation;
    exportContext.GetTransformR ().getMatrix (&textRotation);

    RotMatrix runRotation = exportContext.GetRun ()->GetOrientation ();
    textRotation.productOf (&textRotation, &runRotation);

    DPoint3d textElOrigin = exportContext.GetRun ()->GetLineOffsetAdjustedOrigin ();

    // SHX aligns vertical glyphs to be horizontally centered, so adjust element origin so that it matches placement with other font types.
    TextBlockCR     textBlock       = *exportContext.GetTextBlock ();
    CharStreamCP    currCharStream  = dynamic_cast<CharStreamCP>(exportContext.GetRun ());

    if (textBlock.GetProperties ().IsVertical () && textBlock.IsDGNType () && NULL != currCharStream && DgnFontType::Shx == currCharStream->GetProperties ().GetFont ().GetType ())
        textElOrigin.x += currCharStream->GetExactWidth () / 2.0;

    // Vertical MText aligns runs within lines to the left as their widths vary.
    if (textBlock.GetProperties ().IsVertical () && textBlock.IsMTextType () && NULL != exportContext.GetLine () && NULL != currCharStream)
        {
        LineCR myLine = *exportContext.GetLine ();
        
        if (myLine.GetRunCount () > 0)
            {
            double  myWidth     = currCharStream->GetExactWidth ();
            double  maxWidth    = 0.0;

            for (size_t runIndex = 0; runIndex < myLine.GetRunCount (); runIndex++)
                {
                CharStreamCP charStream = dynamic_cast<CharStreamCP>(myLine.GetRun (runIndex));
                if (NULL == charStream)
                    continue;

                double charStreamUnitWidth = charStream->GetExactWidth ();
                
                if (charStreamUnitWidth > maxWidth)
                    maxWidth = charStreamUnitWidth;
                }

            if (myWidth < maxWidth)
                textElOrigin.x -= (maxWidth - myWidth) / 2.0;
            }
        }

    bool            firstUnitLine   = exportContext.GetLine ()->GetRun (0) == exportContext.GetRun ();
    TextParamWideR  textParams      = exportContext.GetTextParamsR ();

    if (needsAlignEdgeFlag (firstUnitLine, *exportContext.GetLine (), *exportContext.GetTextBlock ()))
        {
        textParams.renderingFlags.alignEdge = true;

        DVec3d adjustment;
        computeAdjustmentForLeftAlignment (adjustment, exportContext.GetLine (), exportContext.GetParagraph ()->GetJustification ());

        textElOrigin.sumOf (&textElOrigin, &adjustment);
        }

    exportContext.GetTransformR ().multiply (&textElOrigin);
    
    // In-memory, text data and computations are normalized; V8 elements have annotation scale baked into them.
    //double annotationScale = textBlock.GetProperties ().GetAnnotationScale (); removed in graphite
    //Transform annotationTransform = Transform::FromFixedPointAndScaleFactors(textBlock.GetUserOrigin (), annotationScale, annotationScale, annotationScale);
    
    //annotationTransform.Multiply (textElOrigin);

    WhiteSpaceBitmaskValueVector    wsVector;
    int                             crCount     = 0;
    createBitMaskOrGetCRCountForWhiteSpaceArray (wsVector, &crCount, exportContext.GetWhiteSpaceArrayR (), exportContext.GetTextBlock ()->IsV7Compatible ());

    textParams.exFlags.bitMaskContainsTabCRLF = (wsVector.size () > 0) ? true : false;
    textParams.exFlags.crCount = crCount;

    if (NULL != currCharStream->GetFieldData ())
        textParams.exFlags.isField = true;

    EditElementHandle newElemEEH;
    
    if (SUCCESS == createTextElement (newElemEEH, textElemTemplate, textElOrigin, textRotation, exportContext))
        {
        if (exportContext.NeedsIndentation () && !exportContext.GetParagraph ()->GetProperties().GetIndentation().IsDefault ())
            TextElemHandler::SetIndentationData (newElemEEH, &exportContext.GetParagraph ()->GetProperties().GetIndentation());
        else
            TextElemHandler::SetIndentationData (newElemEEH, NULL);

        // We normally don't want any arbitrary linkages from the node template, but we need things like transparency to carry over, which happens to be a linkage, so take care of displayable things this way.
        if (NULL != textNodeTemplate)
            ElementPropertiesSetter::ApplyTemplateRestricted (newElemEEH, *textNodeTemplate, ElementPropertiesSetter::TEMPLATE_IGNORE_Color);

// BEIJING_DGNPLATFORM_WIP_Fields
        if (NULL != currCharStream->GetFieldData ())
            {
            if (exportContext.IsFirstRunOfField ())
                currCharStream->GetFieldData ()->AppendToElement (newElemEEH);
                
            if (!exportContext.IsNextRunPartOfSameField ())
                {
                bvector<UInt8>      endOfFieldLinkage;  LinkageUtil::CreateStringLinkage (endOfFieldLinkage, STRING_LINKAGE_KEY_EndField, L"EF");
                MSStringLinkage*    stringLinkage       = (MSStringLinkage*)&endOfFieldLinkage[0];
            
                newElemEEH.AppendElementLinkage (NULL, stringLinkage->header, &stringLinkage->data);
                }
            }
        
        if (wsVector.size () > 0)
            TextElemHandler::SetWhiteSpaceBitmaskValues (newElemEEH, &wsVector);

        MSElementDescrPtr eehEdP = eeh.ExtractElementDescr();
        eehEdP->AddComponent(*newElemEEH.ExtractElementDescr().get());
        
        eeh.SetElementDescr(eehEdP.get(), false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
static void createTextElementDescrFromFraction (EditElementHandleR eeh, ElementHandleCP textElemTemplate, ElementHandleCP textNodeTemplate, ExportContext& exportContext)
    {
    FractionCP fraction = dynamic_cast<FractionCP>(exportContext.GetRun ());
    if (NULL == fraction)
        { BeAssert (false && L"Expected a Fraction object."); return; }

    WhiteSpaceArray whiteSpaceArray (exportContext.GetWhiteSpaceArrayR ());
    exportContext.GetWhiteSpaceArrayR ().clear ();

    DPoint3d    runOrigin       = fraction->GetLineOffsetAdjustedOrigin ();
    Transform   runTransform;   runTransform.initFrom (&runOrigin);
    
    exportContext.GetTransformR ().productOf (&exportContext.GetTransformR (), &runTransform);

    for (size_t iFractionSection = 0; iFractionSection < 2; ++iFractionSection)
        {
        CharStreamCP charStream = fraction->GetCharStream (iFractionSection);
        if (NULL == charStream)
            continue;

        exportContext.SetRun (charStream);
        fraction->GenerateElementParameters (exportContext.GetTextParamsR (), exportContext.GetTextSizeR (), iFractionSection, exportContext.GetDgnProject());

        if (1 == iFractionSection)
            exportContext.GetWhiteSpaceArrayR ().assign (whiteSpaceArray.begin (), whiteSpaceArray.end ());

        appendTextElementFromCharStream (eeh, textElemTemplate, textNodeTemplate, exportContext);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   09/04
//---------------------------------------------------------------------------------------
void TextBlock::AppendComponentTextElementsToNode (RunIteratorCR beginningOfContent, EditElementHandleR eeh, ElementHandleCP textElemTemplate, ElementHandleCP textNodeTemplate, DgnModelP modelRef) const
    {
    ExportContextQueue      exportContextQueue;
    ParagraphCP             lastKnownParagraph      = NULL;
    RunRange                range                   (*this);
    bvector<CharStreamP>    charStreamsToDelete;
    
    for (RunIterator runIter = beginningOfContent; range.end () != runIter; )
        {
        ParagraphCP     currParagraph   = runIter.GetCurrentParagraphCP ();
        RunCP           run             = &*runIter;
        ExportContext   exportContext;

        exportContext.GetTransformR () = this->GetLineTransformAtCaret (*runIter.ToCaret ());
        exportContext.SetDgnModel (modelRef);
        exportContext.SetTextBlock (this);
        exportContext.SetParagraph (currParagraph);
        exportContext.SetLine (runIter.GetCurrentLineCP ());
        
        CharStreamP combinedCharStream = NULL;
        
        if (!this->IsAlongElement ())
            combinedCharStream = combineLikeFormattedAndEdfRuns (runIter, range.end (), exportContext.GetEDFieldsR ());
        
        if (NULL != combinedCharStream)
            {
            exportContext.SetRun (combinedCharStream);
            charStreamsToDelete.push_back (combinedCharStream);
            }
        else
            {
            exportContext.SetRun (run);
            }

        if (lastKnownParagraph != currParagraph)
            {
            exportContext.SetNeedsIndentation (true);
            lastKnownParagraph = currParagraph;
            }

        ++runIter;

// BEIJING_DGNPLATFORM_WIP_Fields
        CharStreamCP    currRunAsCharStream     = dynamic_cast<CharStreamCP>(exportContext.GetRun ());
        TextFieldDataCP currRunFieldData        = ((NULL == currRunAsCharStream) ? NULL : currRunAsCharStream->GetFieldData ());
        TextFieldDataCP previousRunFieldData    = NULL;
        
        if ((NULL != currRunFieldData) && !exportContextQueue.empty ())
            {
            CharStreamCP previousRunAsCharStream = dynamic_cast<CharStreamCP>(exportContextQueue.back ().GetRun ());
            previousRunFieldData = ((NULL == previousRunAsCharStream) ? NULL : previousRunAsCharStream->GetFieldData ());
            }
        
        exportContext.SetIsFirstRunOfField ((NULL != currRunFieldData) && (currRunFieldData != previousRunFieldData));
        
        if (range.end () != runIter)
            {
            if (NULL != currRunFieldData)
                {
                CharStreamCP nextRunAsCharStream = dynamic_cast<CharStreamCP>(&*runIter);
                exportContext.SetIsNextRunPartOfSameField ((NULL != nextRunAsCharStream) && (nextRunAsCharStream->GetFieldData () == currRunFieldData));
                }
            }

        getWhiteSpacesUntilNextCharStream (runIter, range.end (), exportContext.GetWhiteSpaceArrayR ());

        exportContextQueue.push (exportContext);
        }

    while (!exportContextQueue.empty ())
        {
        ExportContext& exportContext = exportContextQueue.front ();

        if (NULL != dynamic_cast <FractionCP> (exportContext.GetRun ()))
            createTextElementDescrFromFraction (eeh, textElemTemplate, textNodeTemplate, exportContext);
        else
            appendTextElementFromCharStream (eeh, textElemTemplate, textNodeTemplate, exportContext);

        exportContextQueue.pop ();
        }
    
    for (CharStreamP const & charStreamToDelete : charStreamsToDelete)
        delete charStreamToDelete;
    
    eeh.GetElementDescrP()->Validate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
void TextBlock::CreateTextNode (EditElementHandleR eeh, ElementHandleCP templateEh, DgnModelP modelRef) const
    {
    EditElementHandle  textElemTemplateEeh;
    EditElementHandle  textNodeTemplateEeh;
    getTextTemplates (textElemTemplateEeh, textNodeTemplateEeh, templateEh, m_nodeNumber);

    WhiteSpaceArray whiteSpaceArray;
    RunIterator     beginningOfContent  = this->Begin ().CreateRunBegin ();
    
    getWhiteSpacesUntilNextCharStream (beginningOfContent, this->End ().CreateRunEnd (), whiteSpaceArray);

    TextParamWide   textNodeParams;
    DPoint2d        textNodeSize;
    
    RunPropertiesCP runProperties = ((NULL == m_nodeProperties) ? GetFirstRunProperties () : m_nodeProperties);

    // We can end up with NULL runProperties when being asked to create an empty text node.
    //  If you want a valid empty text node, you must provide node properties.
    if (NULL == runProperties)
        return;

    textNodeSize = runProperties->GetFontSize ();
    runProperties->ToElementData (textNodeParams, modelRef->GetDgnProject ());

    ParagraphPropertiesCR effectiveParaProps = (m_paragraphArray.empty () ? m_paragraphPropertiesForAdd : m_paragraphArray[0]->GetProperties ());

    synchParametersWithTextBlockParameters (textNodeParams, textNodeSize, *this, true);
    synchParametersWithParagraphParameters (textNodeParams, effectiveParaProps, true);
    
    textNodeParams.exFlags.bitMaskContainsTabCRLF = (0 != whiteSpaceArray.size ());
    
    if (*((Int64*) &textNodeParams.overridesFromStyle))
        textNodeParams.exFlags.styleOverrides = true;

    if (!IsV7Compatible ())
        {
        DPoint2d nodeSize = textNodeSize;
        
        adjustWidthAndHeightForBackwardsUpsideDown (nodeSize.x, nodeSize.y, m_properties.IsBackwards (), m_properties.IsUpsideDown ());

        RotMatrix textNodeRotation;
        GetTransform ().getMatrix (&textNodeRotation);

        if (!shouldCreate3dElement (*modelRef, templateEh))
            adjustRotMatrixForBackwardsUpsideDown (textNodeRotation, m_properties.IsBackwards (), m_properties.IsUpsideDown ());

        if (SUCCESS != TextNodeHandler::CreateElement (
                eeh, templateEh ? &textNodeTemplateEeh : NULL,
                GetUserOrigin (), textNodeRotation, nodeSize, textNodeParams, static_cast<UInt16>(m_properties.GetMaxCharactersPerLine ()),
                shouldCreate3dElement (*modelRef, templateEh), *modelRef))
            return;
        
        TextNodeHandler::SetNodeNumber (eeh, m_nodeNumber);
        
        if (!effectiveParaProps.GetIndentation ().IsDefault ())
            TextNodeHandler::SetIndentationData (eeh, &effectiveParaProps.GetIndentation ());
        else
            TextNodeHandler::SetIndentationData (eeh, NULL);
        
        if (whiteSpaceArray.size () > 0)
            {
            WhiteSpaceBitmaskValueVector wsVector;
            createBitMaskOrGetCRCountForWhiteSpaceArray (wsVector, NULL, whiteSpaceArray, IsV7Compatible ());

            TextNodeHandler::SetWhiteSpaceBitmaskValues (eeh, &wsVector);
            }
        }

    AppendComponentTextElementsToNode (beginningOfContent, eeh, (templateEh ? &textElemTemplateEeh : NULL), (templateEh ? &textNodeTemplateEeh : NULL), modelRef);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   03/07
//---------------------------------------------------------------------------------------
static bool shouldCreateTextNode (TextBlockCR textBlock)
    {
    if (textBlock.GetForceTextNodeFlag () && !textBlock.IsV7Compatible ())
        return true;

    if (textBlock.IsComplexText () || textBlock.IsV7Compatible ())
        return true;

    // This is primarily here for TR#266694, but will affect all full-justified text.
    //  In the TR, old code simply checked if the first paragraph was full-justified,
    //  which could result in promoting child text elements to text nodes, resulting in a bad parent text node.
    //  However, generally speaking, there's no requirement for a text node in this scenario if there's only one character.
    //  Therefore, we can do this check to help the TR, and also make a leaner file.
    bool hasMoreThanOneCharacter =
        (textBlock.GetParagraphCount () > 1
            || textBlock.GetParagraph (0)->GetLineCount () > 1
            || textBlock.GetParagraph (0)->GetLine (0)->GetRunCount () > 1
            || (textBlock.GetParagraph (0)->GetLine (0)->GetRunCount () > 0 && textBlock.GetParagraph (0)->GetLine (0)->GetRun (0)->GetCharacterCount () > 1));

    if (!hasMoreThanOneCharacter)
        return false;

    return (textBlock.GetParagraph(0)->GetIsFullJustification ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/04
//---------------------------------------------------------------------------------------
TextBlock::ToElementResult TextBlock::ToElement (EditElementHandleR eeh, DgnModelP targetDgnModel, ElementHandleCP templateElemHandle) const
    {
    if (NULL == targetDgnModel)
        { BeAssert (false); return TO_ELEMENT_RESULT_Error; }
    
    if (eeh.IsValid ())
        { BeAssert (false); return TO_ELEMENT_RESULT_Error; }
    
    if (IsEmpty () && !GetForceTextNodeFlag ())
        return TO_ELEMENT_RESULT_Empty;

    if ((NULL != templateElemHandle) && !templateElemHandle->IsValid ())
        templateElemHandle = NULL;
    
    if (!IsEmpty () && IsAlongElement ())
        {
        CreateAlongText (eeh, templateElemHandle, targetDgnModel);
        }
    else
        {
        bool isTemplateElementTextNode = ((NULL != templateElemHandle) && (TEXT_NODE_ELM == templateElemHandle->GetLegacyType()));
        
        if (isTemplateElementTextNode || shouldCreateTextNode (*this))
            {
            CreateTextNode (eeh, templateElemHandle, targetDgnModel);
            }
        else
            {
            CreateTextElement (eeh, templateElemHandle, targetDgnModel);
            
            // The above paths that make text nodes do element descriptor validation, which as a side effect validates range; do it manually for text elements.
            eeh.GetDisplayHandler ()->ValidateElementRange(eeh);
            }
        }

    // Append this even if dependency is bad and we've reverted to non-along-element text above.
    if (0 != m_alongRootId)
        AppendAlongTextDependency (eeh);

    return TO_ELEMENT_RESULT_Success;
    }

/*=================================================================================**//**
* @bsiclass                                                     Venkat.Kalyan   12/07
+===============+===============+===============+===============+===============+======*/
struct DgnImporter
    {
    private:    TextBlockR          m_textBlock;
    private:    ElementHandleCR     m_elemHandle;
    private:    TextFieldDataPtr    m_currentFieldData;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/05
    //---------------------------------------------------------------------------------------
    private: void ExtractWhiteSpaceArray (TextBlockNodeArrayR unitArray, ElementHandleCR elemHandle)
        {
        TextParamWide   textParams;
        DPoint2d        fontSize;
        
        memset (&textParams, 0, sizeof (textParams));
        fontSize.zero ();
        
        bool isNode = TEXT_NODE_ELM == elemHandle.GetLegacyType();
        
        if (isNode)
            {
            TextNodeHandler::GetFontSize (elemHandle, fontSize);
            TextNodeHandler::GetTextParams (elemHandle, textParams);
            }
        else
            {
            TextElemHandler::GetFontSize (elemHandle, fontSize);
            TextElemHandler::GetTextParams (elemHandle, textParams);
            }

        WhiteSpaceBitmaskValueVector wsVector;
        if (isNode ? (SUCCESS == TextNodeHandler::GetWhiteSpaceBitmaskValues (elemHandle, wsVector))
                : (SUCCESS == TextElemHandler::GetWhiteSpaceBitmaskValues (elemHandle, wsVector)))
            {
            for (WhiteSpaceBitmaskValueVector::size_type i = 0; i < wsVector.size (); ++i)
                {
                int value = wsVector[i];
                
                switch (value)
                    {
                    case WhiteSpaceBitmaskValue_Tab:
                        {
                        RunP run = new Tab (textParams, fontSize, m_textBlock.GetDgnModelR ());
                        unitArray.push_back (run);

                        break;
                        }
                    
                    case WhiteSpaceBitmaskValue_ParagraphBreak:
                        {
                        RunP run = new ParagraphBreak (textParams, fontSize, m_textBlock.GetDgnModelR ());
                        unitArray.push_back (run);

                        break;
                        }
                    
                    case WhiteSpaceBitmaskValue_LineBreak:
                        {
                        RunP run = new LineBreak (textParams, fontSize, m_textBlock.GetDgnModelR ());
                        unitArray.push_back (run);

                        break;
                        }
                    }
                }
            }

        if (0 != wsVector.size () || 0 == textParams.exFlags.crCount)
            return;

        for (unsigned int i = 0; i < textParams.exFlags.crCount; ++i)
            {
            RunP run = new ParagraphBreak (textParams, fontSize, m_textBlock.GetDgnModelR ());
            unitArray.push_back (run);
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/04
    //---------------------------------------------------------------------------------------
    private: void AddWhiteSpaceAndIndentation (ElementHandleCR elemHandle)
        {
        TextBlockNodeArray unitArray;
        ExtractWhiteSpaceArray (unitArray, elemHandle);
        
        m_textBlock.AppendNodes (unitArray);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/04
    //---------------------------------------------------------------------------------------
    private: BentleyStatus ExtractIndentationLinkageData (ParagraphPropertiesR paraProps, ElementHandleCR eh)
        {
        IndentationData indentation;
        
        if (TEXT_NODE_ELM == eh.GetLegacyType())
            {
            if (SUCCESS != TextNodeHandler::GetIndentationData (eh, indentation))
                return ERROR;
            }
        else
            {
            if (SUCCESS != TextElemHandler::GetIndentationData (eh, indentation))
                return ERROR;
            }
        
        paraProps.SetIndentation (indentation);
        
        return SUCCESS;
        }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   06/07
    //---------------------------------------------------------------------------------------
    private: bvector<CharStreamP> GetCharStreamsFromElement (ElementHandleCR eh)
        {
        bvector<CharStreamP>    charStreams;
        WString                 unicodeString;

        if ((SUCCESS != TextElemHandler::GetString (eh, unicodeString)) || unicodeString.empty ())
            return charStreams;

        TextParamWide   textParams; memset (&textParams, 0, sizeof(textParams));
        DPoint2d        fontSize;   fontSize.Zero ();
        EDFieldVector   edFields;
        
        if ((SUCCESS != TextElemHandler::GetFontSize (eh, fontSize))
            || (SUCCESS != TextElemHandler::GetTextParams (eh, textParams))
            || (SUCCESS != TextElemHandler::GetEDFields (eh, edFields)))
            {
            return charStreams;
            }

// BEIJING_DGNPLATFORM_WIP_Fields
#ifdef DGN_IMPORTER_REORG_WIP
        if (m_currentFieldData.IsValid () || (textParams.exFlags.isField && (m_currentFieldData = TextFieldData::Create (eh)).IsValid ()))
            {
            RunPropertiesPtr    runProps    = RunProperties::Create (textParams, fontSize, *eh.GetDgnModelP ());
            CharStreamP         charStream  = new CharStream (unicodeString.c_str (), *runProps, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize));
            
            charStream->SetFieldData (m_currentFieldData.get ());

            charStreams.push_back (charStream);
            
            WChar endFieldLinkageBuffer[4];   memset (endFieldLinkageBuffer, 0, sizeof (endFieldLinkageBuffer));
            UShort  endFieldLinkageId           = STRING_LINKAGE_KEY_EndField;
            
            if ((SUCCESS == LinkageUtil::ExtractStringLinkageByIndex (&endFieldLinkageId, endFieldLinkageBuffer, _countof (endFieldLinkageBuffer), 0, eh.GetElementCP ())) && (0 == wcscmp (L"EF", endFieldLinkageBuffer)))
                m_currentFieldData = NULL;

            return charStreams;
            }
#endif
        
        TextString::SortAndValidateEdfs (edFields, unicodeString.length ());
        
        if (edFields.empty ())
            {
            charStreams.push_back (new CharStream (unicodeString.c_str (), textParams, fontSize, *eh.GetDgnModelP ()));
            }
        else
            {
            RunPropertiesPtr runProps = RunProperties::Create (textParams, fontSize, *eh.GetDgnModelP ());
            
            if (edFields.front ().start > 0)
                charStreams.push_back (new CharStream (unicodeString.substr (0, edFields.front ().start).c_str (), *runProps, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize)));

            for (size_t iEDField = 0; iEDField < edFields.size (); ++iEDField)
                {
                TextEDFieldCR currEdf = edFields[iEDField];
                
                charStreams.push_back (new EdfCharStream (unicodeString.substr (currEdf.start, currEdf.len).c_str (), currEdf.len, (EdfJustification)currEdf.just, *runProps, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize)));
                
                size_t nextStart = (((edFields.size () - 1) == iEDField) ? unicodeString.size () : (edFields[iEDField + 1].start));

                if ((size_t)(currEdf.start + currEdf.len) < nextStart)
                    charStreams.push_back (new CharStream (unicodeString.substr ((currEdf.start + currEdf.len), nextStart - (currEdf.start + currEdf.len)).c_str (), *runProps, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize)));
                }
            }
        
        return charStreams;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   09/07
    //---------------------------------------------------------------------------------------
    private: CharStreamP GetCharStreamForFractionSection (ChildElemIter const & childElemIter)
        {
        bvector<CharStreamP> charStreams = GetCharStreamsFromElement (childElemIter);
        
        BeDataAssert ((1 == charStreams.size ()) && L"Fraction processing only expects a single CharStream from an element (no EDFs!).");
        
        for (size_t iCharStream = 1; iCharStream < charStreams.size (); ++iCharStream)
            delete charStreams[iCharStream];
        
        return charStreams.front ();
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/05
    //---------------------------------------------------------------------------------------
    private: void ExtractTextBlockNodeArrayFromFractionChain (TextBlockNodeArrayR unitArray, ChildElemIter& childElemIter)
        {
        TextParamWide   textParams;
        DPoint2d        fontSize;
        
        memset (&textParams, 0, sizeof (textParams));
        fontSize.zero ();
        
        TextElemHandler::GetFontSize (childElemIter, fontSize);
        TextElemHandler::GetTextParams (childElemIter, textParams);

        DPoint2d convertedFontSize;
        
        convertedFontSize.x = (fabs (fontSize.x) * (1.0 / Fraction::GetDefaultFractionScaleFactor ().x));
        convertedFontSize.y = (fabs (fontSize.y) * (1.0 / Fraction::GetDefaultFractionScaleFactor ().y));

        RunProperties   runProperties   (textParams, convertedFontSize, m_textBlock.GetDgnModelR ());
        FractionP       fraction        = NULL;
        DPoint2d        zeroRunOffset;  zeroRunOffset.Zero ();

        // We're a fraction; we know how to re-compute our run offsets; don't artificially shove them into the new fraction object.
        runProperties.SetRunOffset (zeroRunOffset);
        runProperties.ClearRunOffsetOverride ();

        switch ((StackedFractionType)textParams.exFlags.stackedFractionType)
            {
            case StackedFractionType::NoBar:           fraction    = new NoBarFraction (runProperties, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize));         break;
            case StackedFractionType::HorizontalBar:   fraction    = new HorizontalBarFraction (runProperties, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize)); break;
            case StackedFractionType::DiagonalBar:     fraction    = new DiagonalBarFraction (runProperties, TextBlockUtilities::ComputeRunLayoutFlags (textParams, fontSize));   break;

            default:
                // ElementIsPartOfFractionChain should now prevent us from ever getting here... prefer to drop the run vs. crashing as a last resort.
                BeAssert (false);
                return;
            }

        fraction->SetTextScale (Fraction::GetDefaultFractionScaleFactor ());
        fraction->SetAlignment ((StackedFractionAlignment)textParams.exFlags.stackedFractionAlign);

        // Do NOT condense the following 2 lines into a single declaration statement. The copy constructor currently acts as ToNext(), and will thus not create an exact copy.
        //  You must declare a blank ChildElemIter first, then use the = operator as an assignment (not a declaration) to get an exact copy.
        ChildElemIter denomChildElemIter;
        denomChildElemIter = childElemIter;

        if (StackedFractionSection::Numerator == (StackedFractionSection)textParams.exFlags.stackedFractionSection)
            {
            WString numeratorString;
            if (SUCCESS == TextElemHandler::GetString (childElemIter, numeratorString))
                fraction->SetNumeratorText (numeratorString.c_str ());
            else
                BeDataAssert (false);
            
            denomChildElemIter = childElemIter.ToNext ();
            }
        
        if (denomChildElemIter.IsValid ())
            {
            TextElemHandler::GetTextParams (denomChildElemIter, textParams);
            if (StackedFractionSection::Denominator == (StackedFractionSection)textParams.exFlags.stackedFractionSection)
                {
                WString denominatorString;
                if (SUCCESS == TextElemHandler::GetString (denomChildElemIter, denominatorString))
                    fraction->SetDenominatorText (denominatorString.c_str ());
                else
                    BeDataAssert (false);
            
                childElemIter = denomChildElemIter;
                }
            }

        unitArray.push_back (fraction);
        ExtractWhiteSpaceArray (unitArray, childElemIter);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   06/07
    //---------------------------------------------------------------------------------------
    private: bool ElementIsPartOfFractionChain (ChildElemIter& childElemIter)
        {
        TextParamWide textParams;
        memset (&textParams, 0, sizeof (textParams));

        TextElemHandler::GetTextParams (childElemIter, textParams);

        return (StackedFractionSection::Numerator == (StackedFractionSection)textParams.exFlags.stackedFractionSection || StackedFractionSection::Denominator == (StackedFractionSection)textParams.exFlags.stackedFractionSection);

        // Unreachable code
        // TR#317235 has customer data with non-zero stackedFractionSection, but zero stackedFractionType; we used to only check stackedFractionSection, which let this pass through ExtractUnitArrayFromFractionChain, which will fail (used to crash) since there is no stackedFractionType. While this is a one-off corrupt data case, the check is trivial.
        //return (((StackedFractionSection::Numerator == textParams.exFlags.stackedFractionSection) || (StackedFractionSection::Denominator == textParams.exFlags.stackedFractionSection))
                //&& (textParams.exFlags.stackedFractionType >= StackedFractionType::NoBar)
                //&& (textParams.exFlags.stackedFractionType <= StackedFractionType::HorizontalBar));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/05
    //---------------------------------------------------------------------------------------
    private: void ExtractTextBlockNodeArrayFromElementChain (TextBlockNodeArrayR unitArray, ChildElemIter& childElemIter)
        {
        ParagraphProperties props = m_textBlock.GetParagraphPropertiesForAdd ();
        if (SUCCESS == this->ExtractIndentationLinkageData (props, childElemIter))
            m_textBlock.SetParagraphPropertiesForAdd (props);
    
        if (ElementIsPartOfFractionChain (childElemIter))
            {
            ExtractTextBlockNodeArrayFromFractionChain (unitArray, childElemIter);
            return;
            }
            
        bvector<CharStreamP> charStreams = GetCharStreamsFromElement (childElemIter);
        unitArray.insert (unitArray.end (), charStreams.begin (), charStreams.end ());

        ExtractWhiteSpaceArray (unitArray, childElemIter);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   06/07
    //---------------------------------------------------------------------------------------
    private: void SetFromOrignAndRotation (RotMatrixR textRotation, DPoint3dR origin, bool userOrigin, bool is3d)
        {
        if (is3d)
            adjustRotMatrixForBackwardsUpsideDown (textRotation, m_textBlock.GetProperties ().IsBackwards (), m_textBlock.GetProperties ().IsUpsideDown ());

        if (userOrigin)
            m_textBlock.SetUserOrigin (origin);
        else
            m_textBlock.SetTextElementOrigin (origin);
        
        m_textBlock.SetOrientation (textRotation);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/04
    //---------------------------------------------------------------------------------------
    private: void FromTextElement ()
        {
        DPoint3d        textElementOrigin;
        RotMatrix       textRotation;
        TextParamWide   textParams;
        DPoint2d        fontSize;

        memset (&textParams, 0, sizeof(textParams));
        fontSize.zero ();

        TextElemHandler::GetElementOrigin (m_elemHandle, textElementOrigin);
        TextElemHandler::GetOrientation (m_elemHandle, textRotation);
        TextElemHandler::GetFontSize (m_elemHandle, fontSize);
        TextElemHandler::GetTextParams (m_elemHandle, textParams);

        m_textBlock.m_properties.FromElementData (textParams, fontSize, 0);
        
        ParagraphProperties paraProps (textParams, m_textBlock.GetDgnModelR ());
        ExtractIndentationLinkageData (paraProps, m_elemHandle);
        
        // Text elements cannot store a line spacing value; if they have a text style, however, take its value instead. This is for cases where you start with a single-line text element, and promote it to multi-line through editting. You don't want to assume the uninitialized value of 0.0 and get an override on there!
        //  Also note that clearing an override has both the effect of clearing the bit (no-op here), and resetting the value back to the style's. You might be tempted to SetLineSpacingValue, but this will set the override flag because it thinks you are forcing a new value in there (even if it matches), and that'd be wrong.
        if (paraProps.HasTextStyle ())
            paraProps.ClearLineSpacingValueOverride ();

        // Similar for line length.
        if (m_textBlock.GetProperties ().HasTextStyle ())
            m_textBlock.m_properties.ClearMaxCharactersPerLineOverride ();
        
        m_textBlock.SetParagraphPropertiesForAdd (paraProps);

        SetFromOrignAndRotation (textRotation, textElementOrigin, false, m_elemHandle.GetElementCP()->ToText_2d().Is3d() ? true : false);

        if (textParams.renderingFlags.documentType)
            m_textBlock.SetType ((TextBlockType)textParams.renderingFlags.documentType);
        else if (TextHandlerBase::HasMTextRenderingLinkage (m_elemHandle))
            m_textBlock.SetType (TEXTBLOCK_TYPE_DwgMText);

        bvector<CharStreamP> charStreams = GetCharStreamsFromElement (m_elemHandle);
        if (charStreams.empty ())
            return;

#if !defined (NDEBUG)
        size_t          numCharStreams          = charStreams.size ();
#endif
        CharStreamCP    savedFirstCharStream    = charStreams.front ();
        
        TextBlockNodeArray unitArray;
        unitArray.insert (unitArray.end (), charStreams.begin (), charStreams.end ());
        
        m_textBlock.AppendNodes (unitArray);

        DgnFontCP font = DgnFontManager::ResolveFont (textParams.font, m_elemHandle.GetDgnModelP ()->GetDgnProject (), DGNFONTVARIANT_DontCare);
        
        // We are a single text element. SHX aligns vertical glyphs to be horizontally centered,
        //  so adjust element origin so that it matches placement with other font types.
        if (textParams.flags.vertical && m_textBlock.IsDGNType () && DgnFontType::Shx == font->GetType ())
            {
            DPoint3d textBlockOrigin = m_textBlock.GetOrigin ();
            
            BeDataAssert ((1 == numCharStreams) && L"DText should not have EDFs and should always have resulted in a single CharStream.");
            textBlockOrigin.x -= savedFirstCharStream->GetExactWidth () / 2.0;
            
            m_textBlock.SetTextElementOrigin (textBlockOrigin);
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   03/05
    //---------------------------------------------------------------------------------------
    private: bool ExtractAlongTextParameters (ElementHandleCR elemHandle)
        {
        AlongTextDependency workingTextDep;
        if (SUCCESS != TextNodeHandler::GetAlongTextData (elemHandle, workingTextDep))
            return false;
        
        m_textBlock.m_alongRootId   = workingTextDep.m_rootId;

        if (workingTextDep.m_customDependencyData.m_parameters.m_areParametersUsed)
            m_textBlock.SetDescrSide (workingTextDep.m_customDependencyData.m_parameters.m_isBelowText ? DESCRIPTOR_SIDE_Below : DESCRIPTOR_SIDE_Above);
        else
            m_textBlock.SetDescrSide (workingTextDep.m_customDependencyData.m_distanceFromElement < 0.0 ? DESCRIPTOR_SIDE_Below : DESCRIPTOR_SIDE_Above);

        m_textBlock.SetDescrOffset (fabs (workingTextDep.m_customDependencyData.m_distanceFromElement));

        DPoint3d descrStartPoint = workingTextDep.m_customDependencyData.m_startPoint;

        if (workingTextDep.m_customDependencyData.m_parameters.m_useStartPoint)
            m_textBlock.SetDescrStartPoint (descrStartPoint);
        else
            m_textBlock.SetDescrDistance (workingTextDep.m_customDependencyData.m_startOffsetAlongElement);

        m_textBlock.m_alongDgnModel = elemHandle.GetDgnModelP ();

        if (!workingTextDep.IsValid (*elemHandle.GetDgnModelP ()))
            return false;

        return true;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   09/07
    //---------------------------------------------------------------------------------------
    private: void AppendDataFromNormalElementChain (ElementHandleCR elemHandle)
        {
        // NEEDS_THOUGHT: I want to make this more efficient if possible.
        for (ChildElemIter childElm (elemHandle, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
            {
            TextBlockNodeArray unitArray;
            if (TEXT_ELM != childElm.GetLegacyType())
                {
                BeAssert (false);
                continue;
                }

            ExtractTextBlockNodeArrayFromElementChain (unitArray, childElm);
            m_textBlock.AppendNodes (unitArray);
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   09/07
    //---------------------------------------------------------------------------------------
    private: void AppendDataFromAlongElementChain (ElementHandleCR elemHandle)
        {
        // NEEDS_THOUGHT: I want to make this more efficient if possible.
        TextBlockNodeArray unitArray;
        for (ChildElemIter childElm (elemHandle, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
            {
            if (TEXT_ELM != childElm.GetLegacyType())
                {
                BeAssert (false);
                continue;
                }

            ExtractTextBlockNodeArrayFromElementChain (unitArray, childElm);
            }
        
        m_textBlock.AppendNodes (unitArray);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Jeff.Marker     06/08
    //---------------------------------------------------------------------------------------
    private: bool ComputeDerivedRotation (ElementHandleCR textNodeElemHandle, RotMatrixR derivedRotMatrix)
        {
        RotMatrixP firstRotation = NULL;

        for (ChildElemIter childElm (textNodeElemHandle, ExposeChildrenReason::Count); childElm.IsValid (); childElm = childElm.ToNext ())
            {
            if (TEXT_ELM != childElm.GetLegacyType())
                {
                BeAssert (false);
                continue;
                }

            DgnElementCP textElement         = childElm.GetElementCP ();
            RotMatrix   textElementRotation;

            if (textElement->Is3d())
                textElementRotation.InitTransposedFromQuaternionWXYZ (textElement->ToText_3d().quat);
            else
                textElementRotation.InitFromAxisAndRotationAngle(2, textElement->ToText_2d().rotationAngle);

            if (NULL == firstRotation)
                {
                firstRotation   = (RotMatrixP)_alloca (sizeof (RotMatrix));
                *firstRotation  = textElementRotation;
                }
            else
                {
                if (!firstRotation->isEqual (&textElementRotation))
                    return false;
                }
            }

        if (NULL == firstRotation)
            return false;

        derivedRotMatrix = *firstRotation;
        
        return true;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   10/04
    //---------------------------------------------------------------------------------------
    private: void FromTextNodeElement ()
        {
        TextParamWide   textNodeParams;
        DPoint2d        textNodeSize;
        DPoint3d        textNodeUserOrigin;
        RotMatrix       textNodeRotation;
        UInt16          maxUnitsPerLine = 0;

        memset (&textNodeParams, 0, sizeof (textNodeParams));
        textNodeSize.zero ();
        
        TextNodeHandler::GetUserOrigin (m_elemHandle, textNodeUserOrigin);
        TextNodeHandler::GetOrientation (m_elemHandle, textNodeRotation);
        TextNodeHandler::GetFontSize (m_elemHandle, textNodeSize);
        TextNodeHandler::GetTextParams (m_elemHandle, textNodeParams);
        TextNodeHandler::GetMaxCharsPerLine (m_elemHandle, maxUnitsPerLine);

        RunProperties runProperties (textNodeParams, textNodeSize, m_textBlock.GetDgnModelR ());

        m_textBlock.SetTextNodeProperties (&runProperties);

        DgnElementCP element = m_elemHandle.GetElementCP ();

        m_textBlock.m_properties.FromElementData (textNodeParams, textNodeSize, maxUnitsPerLine);
        
        ParagraphProperties paraProps (textNodeParams, m_textBlock.GetDgnModelR ());
        ExtractIndentationLinkageData (paraProps, m_elemHandle);
        m_textBlock.SetParagraphPropertiesForAdd (paraProps);

        bool alongElement = ExtractAlongTextParameters (m_elemHandle);

        if (!alongElement)
            this->ComputeDerivedRotation (m_elemHandle, textNodeRotation);

        SetFromOrignAndRotation (textNodeRotation, textNodeUserOrigin, true, element->ToText_2d().Is3d() ? true : false);

        if (textNodeParams.renderingFlags.documentType)
            m_textBlock.SetType ((TextBlockType)textNodeParams.renderingFlags.documentType);
        else if (TextHandlerBase::HasMTextRenderingLinkage (m_elemHandle))
            m_textBlock.SetType (TEXTBLOCK_TYPE_DwgMText);

        m_textBlock.SetNodeNumber (element->ToText_node_2d().nodenumber);
        m_textBlock.SetLineBreakLength (textNodeParams.textnodeWordWrapLength);
        
        AddWhiteSpaceAndIndentation (m_elemHandle);

        if (alongElement)
            AppendDataFromAlongElementChain (m_elemHandle);
        else
            AppendDataFromNormalElementChain (m_elemHandle);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   12/07
    //---------------------------------------------------------------------------------------
    private: DgnImporter (TextBlockR textBlock, ElementHandleCR eh) :
        m_textBlock (textBlock),
        m_elemHandle (eh)
        {
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Venkat.Kalyan   12/07
    //---------------------------------------------------------------------------------------
    public: static void Import (TextBlockR textBlock, ElementHandleCR eh)
        {
        DgnImporter importer (textBlock, eh);
        
        if (TEXT_ELM == eh.GetLegacyType())
            importer.FromTextElement ();
        else if (TEXT_NODE_ELM == eh.GetLegacyType())
            importer.FromTextNodeElement ();

        textBlock.m_processLevel = PROCESS_LEVEL_TextBlock;

        // Although this is somewhat ambiguous, people who load up a TextBlock from an element and then want to append (or do edits in general)
        //  will not really think about setting up a style (the element had one, right?), and we can do something predictable, so why not make lives easier...
        //  you can always override before you append/insert if you want something special.
        DgnTextStylePtr endStyle = textBlock.End ().CreateEffectiveTextStyle ();
        if (endStyle.IsValid ())
            {
            textBlock.SetParagraphPropertiesForAdd (*ParagraphProperties::Create (*endStyle, *eh.GetDgnModelP ()));
            textBlock.SetRunPropertiesForAdd (*RunProperties::Create (*endStyle, *eh.GetDgnModelP ()));
            }
        }
    
    }; // DgnImporter

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   10/04
//---------------------------------------------------------------------------------------
TextBlockPtr TextBlock::Create (ElementHandleCR eh) { return new TextBlock (eh); }

#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning (disable:4355)
#endif // defined (_MSC_VER)
TextBlock::TextBlock (ElementHandleCR eh) :
    RefCountedBase (),
    m_properties                (*eh.GetDgnModelP ()),
    m_paragraphPropertiesForAdd (*eh.GetDgnModelP ()),
    m_runPropertiesForAdd       (*eh.GetDgnModelP ()),
    m_dirty                     (*this)
    {
    this->InitDefaults (*eh.GetDgnModelP ());
    DgnImporter::Import (*this, eh);
    }
#if defined (_MSC_VER)
    #pragma warning(pop)
#endif // defined (_MSC_VER)
