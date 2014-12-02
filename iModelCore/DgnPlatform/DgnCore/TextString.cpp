/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TextString.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define LOG (*LoggingManager::GetLogger("Text"))

static const double FIELD_BGMARGIN_FACTOR = 0.05;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     01/09
+---------------+---------------+---------------+---------------+---------------+------*/
TextString::TextString () :
    m_bufferCount (0)
    {
    DPoint3d                origin;         origin.Zero ();
    RotMatrix               orientation;    orientation.InitIdentity ();
    TextStringProperties    props;

    Init ((FontCharCP)NULL, 0, origin, orientation, props, 0, NULL, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Venkat.Kalyan                   11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
TextString::TextString (WCharCP uniString, DPoint3dCP origin, RotMatrixCP orientation, TextStringPropertiesCR props) :
    m_bufferCount (0)
    {
    DPoint3d    defaultOrigin;      defaultOrigin.Zero ();
    RotMatrix   defaultOrientation; defaultOrientation.InitIdentity ();
    
    if (NULL == origin)
        origin = &defaultOrigin;

    if (NULL == orientation)
        orientation = &defaultOrientation;

    Init (uniString, *origin, *orientation, props, 0, NULL, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
TextString::~TextString ()
    {
    FREE_AND_CLEAR (m_alloced);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void TextString::Init (WCharCP uniString, DPoint3dCR origin, RotMatrixCR orientation, TextStringPropertiesCR props, size_t numEDFields, TextEDFieldCP edFields, int lineWeight)
    {
    T_FontChars fontChars = props.GetFontForCodePage ().ResolveToRenderFont()->UnicodeStringToFontChars (uniString);
    Init (&fontChars[0], (fontChars.size () - 1), origin, orientation, props, numEDFields, edFields, lineWeight);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void TextString::Init (FontCharCP fontChars, size_t numFontChars, DgnModelR model, DPoint3dCR origin, RotMatrixCR orientation, DPoint2dCR fontSize, TextParamWideCR textParams, size_t numEDFields, TextEDFieldCP edFields, int lineWeight)
    {
    TextStringProperties props (textParams, fontSize, model.Is3d (), model.GetDgnProject ());
    Init (fontChars, numFontChars, origin, orientation, props, numEDFields, edFields, lineWeight);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2012
//---------------------------------------------------------------------------------------
void TextString::Init (FontCharCP fontChars, size_t numFontChars, DPoint3dCR origin, RotMatrixCR orientation, TextStringPropertiesCR props, size_t numEDFields, TextEDFieldCP edFieldData, int lineWeight)
    {
    m_alloced                       = NULL;
    m_fontChars                     = NULL;
    m_edFields                      = NULL;
    m_enterDataFlags                = NULL;
    m_glyphCodes                    = NULL;
    m_glyphOrigins                  = NULL;
    m_numEdFields                   = 0;
    m_numGlyphCodes                 = 0;
    m_loadedFontNumber              = 0;
    m_extentsValid                  = false;
    m_shouldIgnorePercentEscapes    = false;
    
    EnsureInternalBuffer (numFontChars, false);
    
    m_numFontChars  = numFontChars;
    m_lineWeight    = lineWeight;
    m_props         = props;
    m_lowerLeft     = origin;
    m_rMatrix       = orientation;
    
    m_extents.Init ();
    memcpy (m_fontChars, fontChars, (sizeof (FontChar) * numFontChars));

    if (0 == numEDFields)
        return;

    // Need to deal with corrupt text elements that have junk in the EDField data to avoid crashes. Several customer files seem to have this.
    bvector<TextEDField> edFields;
    for (size_t iEdf = 0; iEdf < numEDFields; ++iEdf)
        edFields.push_back (edFieldData[iEdf]);
    
    TextString::SortAndValidateEdfs (edFields, numFontChars);
    
    if (edFields.size () > numFontChars)
        {
        LOG.warning("Encountered a corrupt text element in TextString::Init: Too many EDFields.");
        return;
        }

    m_numEdFields = edFields.size ();
    
    memcpy (m_edFields, &edFields[0], sizeof (TextEDField) * m_numEdFields);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void TextString::Clear ()
    {
    memset (m_preAlloc, 0, sizeof (m_preAlloc));
    FREE_AND_CLEAR (m_alloced);
    
    m_lowerLeft.zero ();
    m_extents.init ();
    m_rMatrix.initIdentity ();
    
    m_fontChars                     = NULL;
    m_edFields                      = NULL;
    m_enterDataFlags                = NULL;
    m_glyphCodes                    = NULL;
    m_glyphOrigins                  = NULL;
    m_props                         = TextStringProperties ();
    m_bufferCount                   = 0;
    m_numFontChars                  = 0;
    m_numEdFields                   = 0;
    m_numGlyphCodes                 = 0;
    m_lineWeight                    = 0;
    m_extentsValid                  = false;
    m_shouldIgnorePercentEscapes    = false;
    m_loadedFontNumber              = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static bool edfStartPositionComparer (TextEDFieldCR lhs, TextEDFieldCR rhs)
    {
    return (lhs.start < rhs.start);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
void TextString::SortAndValidateEdfs (bvector<TextEDField>& edFields, size_t elementStringLength)
    {
    // Sorting based on start allows us to write nice iterative loops...
    std::sort (edFields.begin (), edFields.end (), edfStartPositionComparer);

    // Let's sanitize in a first pass... also lets us adjust the start field to be 0-based.
    // There's a lot of work here because I've seen many customer files with junk EDF data, so we need to aggressively validate it.
    for (size_t iEDField = 0; iEDField < edFields.size (); ++iEDField)
        {
        TextEDFieldR currEdf = edFields[iEDField];
                
        if ((0 == currEdf.start) || (currEdf.start > elementStringLength))
            {
            LOG.warning("Encountered corrupt EDF in TextString::SortAndValidateEdfs: start is too small or exceeds element's string length.");
            
            // Since we sort on start position, any remaining EDFs are also invalid.
            edFields.erase (edFields.begin () + iEDField, edFields.end ());
                    
            break;
            }
                
        --currEdf.start;

        if ((size_t)(currEdf.start + currEdf.len) > elementStringLength)
            {
            LOG.warning("Encountered corrupt EDF in TextString::SortAndValidateEdfs: length exceeds element's string length.");
                    
            currEdf.len = (byte)(elementStringLength - currEdf.start);
                    
            if (iEDField < (edFields.size () - 1))
                {
                // EDFs should not overlap; if any more exist, remove them.
                edFields.erase (edFields.begin () + iEDField + 1, edFields.end ());

                break;
                }
            }

        EdfJustification    currJust = static_cast<EdfJustification> (currEdf.just);
        if ((EdfJustification::Left != currJust) && (EdfJustification::Center != currJust) && (EdfJustification::Right != currJust))
            {
            LOG.warning("Encountered corrupt EDF in TextString::SortAndValidateEdfs: unknown justification value.");
            currEdf.just = static_cast<byte>(EdfJustification::Left);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/05
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR TextString::GetOrigin () const {return m_lowerLeft;}
RotMatrixCR TextString::GetRotMatrix () const {return m_rMatrix;}
UInt32 TextString::GetLineWeight () const {return m_lineWeight;}
void  TextString::SetLineWeight (UInt32 value) {m_lineWeight = value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/05
+---------------+---------------+---------------+---------------+---------------+------*/
WString       TextString::GetString() const
    {
    T_FontChars fontChars;
    fontChars.resize (m_numFontChars + 1);
    memcpy (&fontChars[0], m_fontChars, (m_numFontChars * sizeof (FontChar)));
    fontChars[fontChars.size () - 1] = 0;
    
    return m_props.GetFontForCodePage ().FontCharsToUnicodeString (&fontChars[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::SetEDFlags() const
    {
    if (0 == m_numFontChars)
        return;

    memset (m_enterDataFlags, 0, m_numFontChars);

    for (size_t i = 0; i < m_numEdFields; ++i)
        {
        // can't use index because sizeof(TextEDField) != 3 due to aligment padding
        TextEDFieldCP edf = (TextEDFieldCP)(((byte*)m_edFields) + 3 * i);

        for (byte j = 0; j < edf->len; ++j)
            {
            int edChar = (j + edf->start - 1);
            
            if ((edChar >= 0) && ((size_t)edChar < m_numFontChars))
                m_enterDataFlags[edChar] = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/06
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TextString::GetFontDescription (WStringR descr) const
    {
    // IMPORTANT! If any changes are made to this font naming convention, corresponding
    // changes must be made to the printing system. See PlotOutputExport::textCB.

    descr.assign (L"");
    if (NULL == m_props.m_font)
        return ERROR;

    descr = WString (m_props.m_font->GetName ().c_str (), BentleyCharEncoding::Utf8);
    
    if (m_props.m_shxBigFont && DgnFontType::Shx == m_props.m_shxBigFont->GetType ())
        {
        descr.append (L"-");
        descr.append (WString (m_props.m_shxBigFont->GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        }

    if (m_props.IsBold ())
        descr.append (L"*B");

    if (m_props.ShouldUseItalicTypeface ())
        descr.append (L"*I");

    switch (m_props.m_font->GetType ())
        {
        case DgnFontType::Rsc:
            descr.append (L",R");
            break;

        case DgnFontType::Shx:
            descr.append (L",S");
            break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TextString::LoadGlyphs (IDgnGlyphLayoutListenerP glyphListener) const
    {
    if (m_extentsValid && (NULL == glyphListener))
        return SUCCESS;
    
    m_extentsValid = true;
    
    // Any characters present?
    if (0 == m_numFontChars)
        return SUCCESS;

    if (NULL != glyphListener)
        m_loadedFontNumber = glyphListener->_OnFontAnnounced(*this);
    else
        m_loadedFontNumber = (UInt32)-1;

    DgnGlyphLayoutContext::T_EdfMask edfMask (m_numFontChars, false);
    for (size_t i = 0; i < m_numEdFields; ++i)
        {
        TextEDFieldCR   edf         = m_edFields[i];
        byte            edfStart    = edf.start;
        byte            edfStop     = edfStart + edf.len;

        if (edfStart >= m_numFontChars)
            break;

        for (int j = edfStart; (j < edfStop && j < (int)edfMask.size ()); ++j)
            edfMask[j] = true;
        }

    // 2D backward/up-side down text stores flags and negative font sizes; 3D stores flags, positive font sizes, and a mirrored matrix.
    //  At runtime, font size is always positive.
    //  For purposes of glyph layout, we want it to reverse the flow of text only for 2D with backwards/up-side down; it should not for 3D, since the transform will effectively do it.
    DgnGlyphRunLayoutFlags effectiveLayoutFlags = m_props.GetRunLayoutFlags ();
    if (m_props.Is3d ())
        effectiveLayoutFlags = (DgnGlyphRunLayoutFlags)(effectiveLayoutFlags & ~(GLYPH_RUN_LAYOUT_FLAG_Backwards | GLYPH_RUN_LAYOUT_FLAG_UpsideDown));
    
    DgnGlyphLayoutContext layoutContext(m_props.GetFont(), m_props.GetShxBigFontCP());
    layoutContext.SetFontChars(m_fontChars, &edfMask[0], m_numFontChars);
    layoutContext.SetIDgnGlyphLayoutListenerP(glyphListener);
    layoutContext.SetPropertiesFromRunPropertiesBase(m_props);
    layoutContext.SetRunLayoutFlags(effectiveLayoutFlags);
    layoutContext.SetShouldIgnorePercentEscapes(m_shouldIgnorePercentEscapes);

    DgnGlyphLayoutResult layoutResult;
    if (SUCCESS != m_props.ResolveFont().LayoutGlyphs(layoutContext, layoutResult))
        return ERROR;
    
    const_cast<TextStringP>(this)->EnsureInternalBuffer (layoutResult.GetGlyphCodesR ().size (), true);
    
    m_numGlyphCodes = layoutResult.GetGlyphCodesR ().size ();

    memcpy (m_glyphCodes, &layoutResult.GetGlyphCodesR ().front (), sizeof (UInt16) * layoutResult.GetGlyphCodesR ().size ());
    memcpy (m_glyphOrigins, &layoutResult.GetGlyphOriginsR ().front (), sizeof (DPoint3d) * layoutResult.GetGlyphOriginsR ().size ());
    m_extents = layoutResult.ComputeElementRange (layoutContext);

    SetEDFlags ();
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TextString::MakeExtentsValid () const
    {
    return m_extentsValid ? true : (SUCCESS == LoadGlyphs (NULL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TextString::GenerateBoundingShape (DPoint3dP boundingShape, DPoint2dCP expandLL, DPoint2dCP expandUR) const
    {
    return this->GenerateBoundingShape (boundingShape, 0, m_numFontChars, expandLL, expandUR);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
BentleyStatus TextString::GenerateBoundingShape (DPoint3dP boundingShape, size_t offset, size_t length, DPoint2dCP expandLL, DPoint2dCP expandUR) const
    {
    if (!MakeExtentsValid () || m_extents.IsEmpty ())
        return ERROR;

    DPoint2d shape[5];
    shape[0]    = m_extents.low;
    shape[2]    = m_extents.high;

    if (0 != offset)
        {
        if (m_props.IsVertical ())
            shape[2].y = m_glyphOrigins[offset - 1].y;
        else
            shape[0].x = m_glyphOrigins[offset].x;
        }

    if (m_numFontChars != (offset + length))
        {
        if (m_props.IsVertical ())
            shape[0].y = m_glyphOrigins[offset + length - 1].y;
        else
            shape[2].x = m_glyphOrigins[offset + length].x;
        }
    
    if (expandLL)
        {
        if (NULL == expandUR)
            expandUR = expandLL;

        shape[0].subtract (expandLL);
        shape[2].add (expandUR);
        }

    shape[1].x  = shape[2].x;
    shape[1].y  = shape[0].y;
    shape[3].x  = shape[0].x;
    shape[3].y  = shape[2].y;
    shape[4]    = shape[0];

    Transform trans;
    GetDrawTransform (trans, true);
    trans.multiply (boundingShape, shape, 5);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetDrawTransform (TransformR trans, bool includeShear) const
    {
    RotMatrix rMatrix = m_rMatrix;
    if (includeShear && !m_props.IsVertical() && m_props.IsItalic())
        {
        DVec3d axes[2];
        axes[0].init (1.0, 0.0, 0.0);
        axes[1].init (tan(m_props.GetCustomSlantAngle()), 1.0, 0.0);
        RotMatrix shearMtx;
        shearMtx.initFrom2Vectors (axes, axes+1);
        rMatrix.productOf (&rMatrix, &shearMtx);
        }

    trans.initFrom (&rMatrix);
    DPoint3d offset = {m_lowerLeft.x, m_lowerLeft.y, m_lowerLeft.z};
    trans.setTranslation (&offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetGlyphSymbology (ElemDisplayParamsR elParams) const
    {
    elParams.SetLineColor (m_props.GetColor ());
    elParams.SetWeight(m_props.ResolveFont().ShouldDrawWithLineWeight() ? m_lineWeight : 0);
    elParams.SetLineStyle (0);
    elParams.SetFillDisplay (FillDisplay::Never);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetUnderlineSymbology (ElemDisplayParamsR elParams) const
    {
    if (!m_props.m_shouldUseUnderlineStyle)
        {
        GetGlyphSymbology (elParams);

        // We don't draw outline fonts with a line weight (see GetGlyphSymbology), but still want under-/over-line symbology to use it like 8.5.
        if (m_props.ResolveFont().ShouldDrawWithLineWeight())
            elParams.SetWeight (m_lineWeight);

        return;
        }

    elParams.SetLineColor (m_props.m_underlineColor);
    elParams.SetWeight (m_props.m_underlineWeight);
    elParams.SetLineStyle (m_props.m_underlineLineStyle);
    elParams.SetFillDisplay (FillDisplay::Never);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetOverlineSymbology (ElemDisplayParamsR elParams) const
    {
    if (!m_props.m_shouldUseOverlineStyle)
        {
        GetGlyphSymbology (elParams);

        // We don't draw outline fonts with a line weight (see GetGlyphSymbology), but still want under-/over-line symbology to use it like 8.5.
        if (m_props.ResolveFont().ShouldDrawWithLineWeight())
            elParams.SetWeight (m_lineWeight);

        return;
        }

    elParams.SetLineColor (m_props.m_overlineColor);
    elParams.SetWeight (m_props.m_overlineWeight);
    elParams.SetLineStyle (m_props.m_overlineLineStyle);
    elParams.SetFillDisplay (FillDisplay::Never);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetFractionSymbology (ElemDisplayParamsR elParams) const
    {
    elParams.SetLineColor (m_props.m_color);

    // NOTE: Leave current weight (from dhdr) for rsc fonts...
    if (!IsRscFont())
        elParams.SetWeight (1);

    elParams.SetLineStyle (0);
    elParams.SetFillDisplay (FillDisplay::Never);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetOutlineSymbology (ElemDisplayParamsR elParams) const
    {
    elParams.SetLineColor (m_props.m_backgroundBorderColor);
    elParams.SetWeight (m_props.m_backgroundBorderWeight);
    elParams.SetLineStyle (m_props.m_backgroundBorderLineStyle);
    elParams.SetFillColor (m_props.m_backgroundFillColor);
    elParams.SetFillDisplay (FillDisplay::Never);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPropertiesCR TextString::GetProperties () const 
    {
    return m_props;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::GetBackgroundSymbology (ElemDisplayParamsR elParams) const
    {
    GetOutlineSymbology (elParams);
    elParams.SetFillDisplay (FillDisplay::Blanking);
    }

/*---------------------------------------------------------------------------------**//**
* draw the underline, overline, and fraction line for a textstring
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextString::DrawTextAdornments (ViewContextR context) const
    {
    if (DrawPurpose::RegionFlood == context.GetDrawPurpose ())
        return;

    bool haveUnderline = m_props.ShouldShowUnderline ();
    bool haveOverline  = m_props.ShouldShowOverline ();
    bool haveFraction  = HasFraction();

    if (!haveUnderline && !haveOverline && !haveFraction)
        return;

    if (!MakeExtentsValid())
        return;

    IDrawGeomR  output = context.GetIDrawGeom();
    Transform   textTrans;

    GetDrawTransform (textTrans, true);
    context.PushTransform (textTrans);

    DPoint3d    pts[2];

    pts[0].z = pts[1].z = 0.0;

    if (Is3d ())
        pts[0].z = pts[1].z = 0.0;
    else
        pts[0].z = pts[1].z = context.GetDisplayPriority ();

    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams();

    if (haveUnderline)
        {
        GetUnderlineSymbology (*elParams);
        context.CookDisplayParams ();

        pts[0].y = pts[1].y = m_extents.low.y - m_props.m_underlineOffset;
        pts[0].x = m_extents.low.x;
        pts[1].x = m_extents.high.x;

        output.DrawLineString3d (2, pts, NULL);
        }

    if (haveOverline)
        {
        GetOverlineSymbology (*elParams);
        context.CookDisplayParams ();

        pts[0].y = pts[1].y = m_extents.high.y + m_props.m_overlineOffset;
        pts[0].x = m_extents.low.x;
        pts[1].x = m_extents.high.x;

        output.DrawLineString3d (2, pts, NULL);
        }

    // if this is the top of a diagonal stacked fraction then we need to draw the divsor '/'
    if (haveFraction)
        {
        GetFractionSymbology (*elParams);
        context.CookDisplayParams ();

        if (IsRscFont())
            {
            // rsc glphys frequently don't have any left-side-bearing space, which causes the divisor to run through the
            // lower glyph. Therefore, we fudge the slope of the line a bit for RSC fonts, adjusting it if there actually is a LSB.
            // This comes from TR#243923.
            DgnGlyphCP firstChar = static_cast<DgnRscFontCR>(m_props.ResolveFont ()).FindGlyphCP (m_fontChars[0]);

            double leadingSpace = firstChar ? firstChar->GetBlackBoxLeft() * 2 : 0;
            if (leadingSpace < 0)
                leadingSpace = 0;

            pts[0].x = -m_props.m_fontSize.x * (0.6 - leadingSpace);
            pts[0].y = -m_props.m_fontSize.y * 0.1;
            pts[1].x =  m_props.m_fontSize.x * (0.3 + leadingSpace);
            pts[1].y =  m_props.m_fontSize.y * 1.4;
            }
        else
            {
            pts[0].x = -m_props.m_fontSize.x * 0.5;
            pts[0].y = -m_props.m_fontSize.y * 0.0;
            pts[1].x =  m_props.m_fontSize.x * 0.5;
            pts[1].y =  m_props.m_fontSize.y * 1.5;
            }
        output.DrawLineString3d (2, pts, NULL);
        }

    context.PopTransformClip ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static void setupContextForFieldBackground (ViewContextR context, TextStringCR textString, ElemDisplayParamsR elParams)
    {
    ViewportP viewport = context.GetViewport ();

    if (NULL == viewport)
        {
        textString.GetBackgroundSymbology (elParams);
        context.CookDisplayParams ();

        return;
        }
        
    // Need to get current line color w/overrides...
    context.CookDisplayParams ();

    UInt32  textColor   = context.GetCurrLineColor ();
    UInt32  bgColor     = viewport->GetBackgroundColor ();
    UInt32  fieldColorL = viewport->AdjustColorForContrast (viewport->MakeTrgbColor (0xa0, 0xa0, 0xa0, 0), bgColor);
    UInt32  fieldColorD = viewport->AdjustColorForContrast (viewport->MakeTrgbColor (0x5f, 0x5f, 0x5f, 0), bgColor);
    UInt32  useColor    = (fieldColorL == viewport->AdjustColorForContrast (fieldColorL, textColor) ? fieldColorL : fieldColorD);

    elParams.SetLineColorTBGR (useColor);
    elParams.SetFillColorTBGR (useColor);
    elParams.SetFillDisplay (FillDisplay::Blanking);
    elParams.SetWeight (0);
    elParams.SetLineStyle (0);

    context.CookDisplayParams ();

    OvrMatSymbP ovrMatSymb = context.GetOverrideMatSymb ();
    if (NULL != ovrMatSymb)
        {
        ovrMatSymb->SetFlags (MATSYMB_OVERRIDE_None);
        context.ActivateOverrideMatSymb ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2010
//---------------------------------------------------------------------------------------
static void drawBackgroundShape (ViewContextR context, TextStringCR textString, ElemDisplayParamsR elParams, DPoint3dP pts)
    {
    // If we're creating a qvElem, then the blanking shape should be drawn at the same z=0 as the text.
    //  But, if we're NOT creating a qvElem, and if we're in a 2d element, then it needs to be at the same display priority as the text.
    //  Hopefully, we can get rid of this when we always create qvElems for text.
    if (!context.CheckICachedDraw () && !textString.Is3d ())
        {
        double priority = context.GetDisplayPriority ();

        for (size_t iPoint = 0; iPoint < 5; ++iPoint)
            pts[iPoint].z = priority;
        }
        
    IDrawGeomR output = context.GetIDrawGeom ();
    
    output.DrawShape3d (5, pts, true, NULL);

    ElemMatSymbP matSymb = context.GetElemMatSymb ();

    // If outline is same color as fill, don't draw it.
    //  Additionally, if not plotting, always draw the outline if it has greater than single-pixel width [TR 148867].
    if ((matSymb->GetLineColorTBGR () != matSymb->GetFillColorTBGR ()) || ((DrawPurpose::Plot != context.GetDrawPurpose ()) && (matSymb->GetWidth () > 1)))
        {
        textString.GetOutlineSymbology (elParams);
        context.CookDisplayParams ();

        output.DrawLineString3d (5, pts, NULL);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void TextString::DrawTextBackground (ViewContextR context) const
    {
    if (DrawPurpose::RegionFlood == context.GetDrawPurpose ())
        return;

    if ((NULL != context.GetViewport ()) && m_props.IsPartOfField () && context.WantShowDefaultFieldBackground ())
        {
        ElemDisplayParamsP elParams = context.GetCurrentDisplayParams ();
        setupContextForFieldBackground (context, *this, *elParams);
    
        DPoint2d    boxPaddingLL;
        DPoint2d    boxPaddingUR;
        double      height = fabs (m_props.m_fontSize.y);

        boxPaddingLL.x = boxPaddingUR.x = boxPaddingUR.y    = (height * FIELD_BGMARGIN_FACTOR);
        boxPaddingLL.y                                      = (height * (m_props.ResolveFont().GetDescenderRatio () + FIELD_BGMARGIN_FACTOR));

        DPoint3d pts[5];
        this->GenerateBoundingShape (pts, &boxPaddingLL, &boxPaddingUR);
    
        drawBackgroundShape (context, *this, *elParams, pts);
        return;
        }
    
    if (!m_props.ShouldUseBackground ())
        return;
        
    ElemDisplayParamsP elParams = context.GetCurrentDisplayParams ();

    GetBackgroundSymbology (*elParams);
    context.CookDisplayParams ();

    DPoint3d pts[5];
    this->GenerateBoundingShape (pts, &m_props.m_backgroundBorderPadding, &m_props.m_backgroundBorderPadding);
    
    drawBackgroundShape (context, *this, *elParams, pts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TextString::ComputeUserOrigin (DPoint3dR userOrg) const
    {
    if (!MakeExtentsValid())
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    DPoint3d offset;
    TextString::ComputeUserOriginOffset (offset, m_extents.high, 0.0, m_props.m_justification, m_props.IsVertical (), &m_props.ResolveFont());
    m_rMatrix.multiply (&offset, &offset, 1);

    userOrg = m_lowerLeft;
    userOrg.Add (offset);
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   09/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   TextString::SetOriginFromUserOrigin (DPoint3dCR userOrg)
    {
    if (!MakeExtentsValid())
        return ERROR;

    DPoint3d offset;
    TextString::ComputeUserOriginOffset (offset, m_extents.high, 0.0, m_props.m_justification, m_props.IsVertical(), &m_props.ResolveFont());
    m_rMatrix.multiply (&offset, &offset, 1);

    m_lowerLeft = userOrg;
    m_lowerLeft.subtract (&offset);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TextString::IsBlankString () const
    {
    for (size_t i = 0; i < m_numFontChars; ++i)
        {
        if (0x20 /* space */ != m_fontChars[i])
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TextString::HasFraction()  const
    {
    if (m_props.m_stackedFractionType != StackedFractionType::DiagonalBar || m_props.IsVertical ())
        return false;

    return m_props.m_stackedFractionSection == StackedFractionSection::Denominator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             01/89
+---------------+---------------+---------------+---------------+---------------+------*/
void TextString::ComputeUserOriginOffset (DPoint3dR computedOffset, DPoint2dCR textExtents, double wordWrapLength, TextElementJustification justification, bool isVertical, DgnFontCP font)
    {
    computedOffset.zero ();

    int     justificationInt = static_cast<int>(justification);

#if defined (NEEDS_WORK_DGNITEM)
    // The justifications LBTTM, CBTTM, RBTTM mean nothing for text nodes or vertical text.
    if ((TEXT_NODE_ELM == elementType && justificationInt >= static_cast<int>(TextElementJustification::LeftDescender)) || isVertical)
        {
        switch (justification)
            {
            case TextElementJustification::LeftDescender:
                justification = TextElementJustification::LeftBaseline;
                break;
            
            case TextElementJustification::CenterDescender:
                justification = TextElementJustification::CenterBaseline;
                break;
            
            case TextElementJustification::RightDescender:
                justification = TextElementJustification::RightBaseline;
                break;
            }
        }
#endif
    
    if (isVertical)
        {
        switch (justificationInt % 3)
            {
            case 1: // Center
                computedOffset.y = textExtents.y/2.0;
                break;

            case 2: // Lower
                computedOffset.y = textExtents.y;
                break;
            }

        // Left Margin and Right Margin not supported for vertical text they become Right and Left justification, respectively.
        switch (justificationInt/3)
            {
            case 2: // Center
                computedOffset.x = textExtents.x/2.0;
                break;

            case 3: // Right Margin
            case 4: // Right
                computedOffset.x = textExtents.x;
                break;
            }
        }
    else
        {
        if (justificationInt >= static_cast<int>(TextElementJustification::LeftDescender))
            {
            double descender = font ? font->GetDescenderRatio() : 0.0;
            switch (justification)
                {
                case TextElementJustification::LeftDescender:
                    computedOffset.y = - textExtents.y * descender;
                    break;
                
                case TextElementJustification::CenterDescender:
                    computedOffset.x = textExtents.x/2.0;
                    computedOffset.y = - textExtents.y * descender;
                    break;
                
                case TextElementJustification::RightDescender:
                    computedOffset.x = textExtents.x;
                    computedOffset.y = - textExtents.y * descender;
                    break;
                }
            }
        else
            {
            switch (justificationInt % 3)
                {
                case 0: // Upper
                    computedOffset.y = textExtents.y;
                    break;

                case 1: // Center
                    computedOffset.y = textExtents.y/2.0;
                    break;
                }

            switch (justificationInt/3)
                {
                case 1: // Left Margin
                    computedOffset.x = textExtents.x - wordWrapLength;
                    break;

                case 2: // Center
                    computedOffset.x = textExtents.x/2.0;
                    break;

                case 3: // Right Margin
                    computedOffset.x = wordWrapLength;
                    break;

                case 4: // Right
                    computedOffset.x = textExtents.x;
                    break;
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2dCR TextString::GetExtents () const
    {
    MakeExtentsValid ();
    return m_extents;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPtr TextString::Create ()
    {
    return new TextString ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextStringPtr TextString::Create (WCharCP string, DPoint3dCP lowerLeft, RotMatrixCP rMatrix, TextStringPropertiesCR props)
    {
    return new TextString (string, lowerLeft, rMatrix, props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/86
+---------------+---------------+---------------+---------------+---------------+------*/
void TextString::TransformOrientationAndGetScale (DPoint2dR scaleFactor, RotMatrixR orientation, double* rotationAngle, TransformCR transform, bool is3d)
    {
    orientation.productOf (&transform, &orientation);
    
    DVec3d xcol;
    orientation.getColumn (&xcol, 0);
    
    DVec3d ycol;
    orientation.getColumn (&ycol, 1);
    
    scaleFactor.x = xcol.Magnitude();
    scaleFactor.y = ycol.Magnitude();

    if (is3d)
        {
        if (fabs (scaleFactor.x) < 1e-10)
            {
            if (fabs (scaleFactor.y) < 1e-10)
                {
                orientation.initIdentity ();
                }
            else
                {
                LegacyMath::RMatrix::FromYVector (&orientation, &ycol);
                orientation.invert ();
                }
            }
        else if (fabs (scaleFactor.y) < 1e-10)
            {
            LegacyMath::RMatrix::FromXVector (&orientation, &xcol);
            orientation.invert ();
            }
        else
            {
            double scale = 1.0 / scaleFactor.x;
            
            if (scale > 1.0e-15)
                xcol.scale (scale);
            else
                xcol.init (1.0, 0.0, 0.0);
            
            DVec3d zcol;
            
            zcol.CrossProduct (xcol, ycol);
            zcol.normalize ();

            // To get y-axis, cross z with x.
            ycol.CrossProduct (zcol, xcol);
            ycol.normalize ();
            
            orientation.initFromColumnVectors (&xcol, &ycol, &zcol);
            }
        }
    else
        {
        double tempRotationAngle;
        if (NULL == rotationAngle)
            rotationAngle = &tempRotationAngle;

        *rotationAngle = Angle::Atan2 (xcol.y, xcol.x);

        // See if text is mirrored.
        if ((xcol.x * ycol.y - xcol.y * ycol.x) < 0.0)
            {
            if (xcol.x < 0.0)
                {
                scaleFactor.x *= -1.0;
                *rotationAngle += msGeomConst_pi;
                }
            else
                {
                scaleFactor.y *= -1.0;
                }
            }

        orientation.InitFromAxisAndRotationAngle(2,  *rotationAngle);
        }
    }

#define ALLOC_FROM_BUFFER(var,type,n) { var = reinterpret_cast<type*>(buffer); size_t segmentSize = ((n) * sizeof (type)); memset (buffer, 0, segmentSize); buffer += segmentSize; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void TextString::EnsureInternalBuffer (size_t newCount, bool shouldCopyData)
    {
    if (newCount <= m_bufferCount)
        return;
    
    bvector<Byte> oldData;
    if (shouldCopyData)
        {
        size_t dataSize = (m_bufferCount * PER_CHAR_SIZE);
        oldData.resize (dataSize);
        memcpy (&oldData[0], m_fontChars, dataSize);
        }
    
    FREE_AND_CLEAR (m_alloced);
    
    Byte* buffer = NULL;

    if (newCount <= PREALLOC_TEXTLEN)
        buffer = m_preAlloc;
    else
        buffer = m_alloced = static_cast<Byte*>(malloc (newCount * PER_CHAR_SIZE));
    
    ALLOC_FROM_BUFFER (m_fontChars,         FontChar,       newCount);
    ALLOC_FROM_BUFFER (m_edFields,          TextEDField,    newCount);
    ALLOC_FROM_BUFFER (m_enterDataFlags,    Byte,           newCount);
    ALLOC_FROM_BUFFER (m_glyphCodes,        UInt16,         newCount);
    ALLOC_FROM_BUFFER (m_glyphOrigins,      DPoint3d,       newCount);
    
    if (shouldCopyData)
        {
        Byte*   srcData     = &oldData[0];
        size_t  chunkSize;
        
        chunkSize = (sizeof (FontChar) * m_bufferCount);    memcpy (m_fontChars, srcData, chunkSize);       srcData += chunkSize;
        chunkSize = (sizeof (TextEDField) * m_bufferCount); memcpy (m_edFields, srcData, chunkSize);        srcData += chunkSize;
        chunkSize = (sizeof (Byte) * m_bufferCount);        memcpy (m_enterDataFlags, srcData, chunkSize);  srcData += chunkSize;
        chunkSize = (sizeof (UInt16) * m_bufferCount);      memcpy (m_glyphCodes, srcData, chunkSize);      srcData += chunkSize;
        chunkSize = (sizeof (DPoint3d) * m_bufferCount);    memcpy (m_glyphOrigins, srcData, chunkSize);
        }
    
    m_bufferCount = newCount;
    }

#undef ALLOC_FROM_BUFFER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TextString::IsEmptyString () const
    {
    if (0 == m_numFontChars)
        return true;

    return m_props.ResolveFont ().DoGlyphsHaveBlankGeometry (m_fontChars, m_numFontChars);
    }
        
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void TextString::SetShouldIgnorePercentEscapes (bool value)
    {
    m_shouldIgnorePercentEscapes = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawTextString (TextStringCR text)
    {
    text.DrawTextBackground (*this);

    text.GetGlyphSymbology (*GetCurrentDisplayParams ());
    CookDisplayParams ();

    double priority = GetDisplayPriority ();
    
    GetIDrawGeom ().DrawTextString (text, text.Is3d () ? NULL : &priority);

    text.DrawTextAdornments (*this);
    }
