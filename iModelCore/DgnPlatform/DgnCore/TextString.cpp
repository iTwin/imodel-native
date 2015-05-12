/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/TextString.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/TextStringPersistence.h>

using namespace flatbuffers;

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
TextStringStylePtr TextStringStyle::Create() { return new TextStringStyle(); }
TextStringStyle::TextStringStyle() :
    T_Super(),
    m_font(&DgnFontManager::GetLastResortTrueTypeFont()),
    m_isBold(false),
    m_isItalic(false),
    m_isUnderlined(false),
    m_size({ 0.0, 0.0 })
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
TextStringStylePtr TextStringStyle::Clone() const { return new TextStringStyle(*this); }
TextStringStyle::TextStringStyle(TextStringStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
TextStringStyleR TextStringStyle::operator=(TextStringStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this; }
void TextStringStyle::CopyFrom(TextStringStyleCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_color = rhs.m_color;
    m_font = rhs.m_font;
    m_isBold = rhs.m_isBold;
    m_isItalic = rhs.m_isItalic;
    m_isUnderlined = rhs.m_isUnderlined;
    m_size = rhs.m_size;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextStringStyle::Reset()
    {
    // Making additions or changes? Please check constructor and CopyFrom.
    m_color.SetAllColors(0);
    m_font = &DgnFontManager::GetLastResortTrueTypeFont();
    m_isBold = false;
    m_isItalic = false;
    m_isUnderlined = false;
    m_size.Zero();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
ColorDefCR TextStringStyle::GetColor() const { return m_color; }
void TextStringStyle::SetColor(ColorDefCR value) { m_color = value; }
DgnFontCR TextStringStyle::GetFont() const { return *m_font; }
void TextStringStyle::SetFont(DgnFontCR value) { m_font = &DgnFontManager::ResolveFont(&value); }
bool TextStringStyle::IsBold() const { return m_isBold; }
void TextStringStyle::SetIsBold(bool value) { m_isBold = value; }
bool TextStringStyle::IsItalic() const { return m_isItalic; }
void TextStringStyle::SetIsItalic(bool value) { m_isItalic = value; }
bool TextStringStyle::IsUnderlined() const { return m_isUnderlined; }
void TextStringStyle::SetIsUnderlined(bool value) { m_isUnderlined = value; }
DPoint2dCR TextStringStyle::GetSize() const { return m_size; }
void TextStringStyle::SetSize(DPoint2dCR value) { m_size = value; }

double TextStringStyle::GetHeight() const { return m_size.y; }
void TextStringStyle::SetHeight(double value) { m_size.y = value; }
double TextStringStyle::GetWidth() const { return m_size.x; }
void TextStringStyle::SetWidth(double value) { m_size.x = value; }
void TextStringStyle::SetSize(double widthAndHeight) { SetSize(widthAndHeight, widthAndHeight); }
void TextStringStyle::SetSize(double width, double height) { m_size.x = width; m_size.y = height; }

//*****************************************************************************************************************************************************************************************************
//*****************************************************************************************************************************************************************************************************

static const double DEFAULT_ITALIC_ANGLE = Angle::DegreesToRadians(30.0);
static const uint32_t DEFAULT_BOLD_WEIGHT = 2;
static const double DEFAULT_UNDERLINE_OFFSET_FACTOR = 0.15;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
TextStringPtr TextString::Create() { return new TextString(); }
TextString::TextString() :
    T_Super(),
    m_isValid(false)
    {
    // Making additions or changes? Please check CopyFrom and Reset.
    m_origin.Zero();
    m_orientation.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
TextStringPtr TextString::Clone() const { return new TextString(*this); }
TextString::TextString(TextStringCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
TextStringR TextString::operator=(TextStringCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this; }
void TextString::CopyFrom(TextStringCR rhs)
    {
    // Making additions or changes? Please check constructor and Reset.
    m_text = rhs.m_text;
    m_style = rhs.m_style;
    m_origin = rhs.m_origin;
    m_orientation = rhs.m_orientation;
    
    m_isValid = rhs.m_isValid;
    m_range = rhs.m_range;
    m_glyphs = rhs.m_glyphs;
    m_glyphIds = rhs.m_glyphIds;
    m_glyphOrigins = rhs.m_glyphOrigins;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::Reset()
    {
    // Making additions or changes? Please check constructor and CopyFrom.
    m_text.clear();
    m_style.Reset();
    m_origin.Zero();
    m_orientation.InitIdentity();

    m_isValid = false;
    m_range = DRange2d::NullRange();
    m_glyphs.clear();
    m_glyphIds.clear();
    m_glyphOrigins.clear();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
Utf8StringCR TextString::GetText() const { return m_text; }
void TextString::SetText(Utf8CP value) { Invalidate(); m_text.AssignOrClear(value); }
TextStringStyleCR TextString::GetStyle() const { return m_style; }
TextStringStyleR TextString::GetStyleR() { Invalidate(); return m_style; }
void TextString::SetStyle(TextStringStyleCR value) { Invalidate(); m_style = value; }
DPoint3dCR TextString::GetOrigin() const { return m_origin; }
void TextString::SetOrigin(DPoint3dCR value) { m_origin = value; }
RotMatrixCR TextString::GetOrientation() const { return m_orientation; }
void TextString::SetOrientation(RotMatrixCR value) { m_orientation = value; }

void TextString::Invalidate() { m_isValid = false; }
DRange2dCR TextString::GetRange() const { Update(); return m_range; }
size_t TextString::GetNumGlyphs() const { Update(); return m_glyphIds.size(); }
DgnGlyphCP const* TextString::GetGlyphs() const { Update(); return &m_glyphs[0]; }
DgnGlyph::T_Id const* TextString::GetGlyphIds() const { Update(); return &m_glyphIds[0]; }
DPoint3dCP TextString::GetGlyphOrigins() const { Update(); return &m_glyphOrigins[0]; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::ComputeGlyphAxes(DVec3dR xAxis, DVec3dR yAxis) const
    {
    DPoint2d size = m_style.GetSize();

    xAxis.init(size.x, 0.0, 0.0);
    yAxis.init(0.0, size.y, 0.0);

    if (m_style.IsItalic() && (DgnFontType::TrueType != m_style.GetFont().GetType()))
        yAxis.x = (tan(DEFAULT_ITALIC_ANGLE) * size.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::ComputeBoundingShape(DPoint3dP boxPts) const { ComputeBoundingShape(boxPts, 0.0, 0.0); }
void TextString::ComputeBoundingShape(DPoint3dP boxPts, double uniformPadding) const { ComputeBoundingShape(boxPts, uniformPadding, uniformPadding); }
void TextString::ComputeBoundingShape(DPoint3dP boxPts, double horizontalPadding, double verticalPadding) const
    {
    memset(boxPts, 0, sizeof(DPoint3d) * 5);

    boxPts[0].Init(m_range.low);
    boxPts[2].Init(m_range.high);

    if (0.0 != horizontalPadding)
        {
        boxPts[0].x -= horizontalPadding;
        boxPts[2].x += horizontalPadding;
        }

    if (0.0 != verticalPadding)
        {
        boxPts[0].y -= verticalPadding;
        boxPts[2].y += verticalPadding;
        }
    
    boxPts[1].x = boxPts[2].x;
    boxPts[1].y = boxPts[0].y;
    boxPts[3].x = boxPts[0].x;
    boxPts[3].y = boxPts[2].y;
    
    boxPts[4] = boxPts[0];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
DVec2d TextString::ComputeOffsetToJustification(HorizontalJustification hjust, VerticalJustification vjust) const
    {
    DVec2d offset;
    offset.Zero();
    
    Update();

    switch (hjust)
        {
        case HorizontalJustification::Left:     break;
        case HorizontalJustification::Center:   offset.x += (m_range.XLength() / 2.0); break;
        case HorizontalJustification::Right:    offset.x += m_range.XLength(); break;

        default:
            BeAssert(false); // Unknown/unexpected HorizontalJustification;
            break;
        }

    switch (vjust)
        {
        case VerticalJustification::Top:    offset.y += m_range.YLength(); break;
        case VerticalJustification::Middle: offset.y += (m_range.YLength() / 2.0); break;
        case VerticalJustification::Bottom: break;

        default:
            BeAssert(false); // Unknown/unexpected HorizontalJustification;
            break;
        }
    
    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
DPoint3d TextString::ComputeJustificationOrigin(HorizontalJustification hjust, VerticalJustification vjust) const
    {
    DPoint3d offsetToJustification = DPoint3d::From(ComputeOffsetToJustification(hjust, vjust));
    m_orientation.Multiply(offsetToJustification);

    DPoint3d jorigin = m_origin;
    jorigin.Add(offsetToJustification);

    return jorigin;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::SetOriginFromJustificationOrigin(DPoint3dCR jorigin, HorizontalJustification hjust, VerticalJustification vjust)
    {
    DPoint3d offsetFromJustification = DPoint3d::From(ComputeOffsetToJustification(hjust, vjust));
    offsetFromJustification.Negate();

    RotMatrix invertedOrientation = m_orientation;
    m_orientation.Invert();
    invertedOrientation.Multiply(offsetFromJustification);
    
    m_origin = jorigin;
    m_origin.Add(offsetFromJustification);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
Transform TextString::ComputeTransform() const
    {
    return Transform::From(m_orientation, m_origin);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::Update() const { const_cast<TextStringP>(this)->Update(); }
void TextString::Update()
    {
    if (m_isValid)
        return;

    ComputeAndLayoutGlyphs();
    
    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::ComputeAndLayoutGlyphs()
    {
    m_range.Init();
    m_glyphs.clear();
    m_glyphIds.clear();
    m_glyphOrigins.clear();

    if (m_text.empty())
        return;
    
    DgnGlyphLayoutContext layoutContext;
    layoutContext.m_string = m_text;
    layoutContext.m_drawSize = m_style.GetSize();
    layoutContext.m_isBold = m_style.IsBold();
    layoutContext.m_isItalic = (m_style.IsItalic() && DgnFontType::TrueType == m_style.GetFont().GetType());

    DgnGlyphLayoutResult layoutResult;
    if (SUCCESS != m_style.GetFont().LayoutGlyphs(layoutResult, layoutContext))
        return;

    auto numGlyphs = layoutResult.m_glyphs.size();
    if (0 == numGlyphs)
        return;
    
    m_range = layoutResult.m_justificationRange;

    m_glyphs.resize(numGlyphs);
    memcpy(&m_glyphs[0], &layoutResult.m_glyphs[0], sizeof(decltype(m_glyphs)::value_type) * numGlyphs);
    
    m_glyphIds.clear();
    for (DgnGlyphCP glyph : layoutResult.m_glyphs)
        m_glyphIds.push_back(glyph->GetId());

    m_glyphOrigins.resize(numGlyphs);
    memcpy(&m_glyphOrigins[0], &layoutResult.m_glyphOrigins[0], sizeof(decltype(m_glyphOrigins)::value_type) * numGlyphs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Ray.Bentley     07/1986
//---------------------------------------------------------------------------------------
void TextString::TransformOrientationAndExtractScale(DPoint2dR scaleFactor, RotMatrixR orientation, TransformCR transform)
    {
    orientation.productOf(&transform, &orientation);

    DVec3d xcol;
    orientation.GetColumn(xcol, 0);

    DVec3d ycol;
    orientation.GetColumn(ycol, 1);

    scaleFactor.x = xcol.Magnitude();
    scaleFactor.y = ycol.Magnitude();

    if (fabs(scaleFactor.x) < 1e-10)
        {
        if (fabs(scaleFactor.y) < 1e-10)
            {
            orientation.InitIdentity();
            }
        else
            {
            LegacyMath::RMatrix::FromYVector(&orientation, &ycol);
            orientation.Invert();
            }
        }
    else if (fabs(scaleFactor.y) < 1e-10)
        {
        LegacyMath::RMatrix::FromXVector(&orientation, &xcol);
        orientation.Invert();
        }
    else
        {
        double scale = (1.0 / scaleFactor.x);

        if (scale > 1.0e-15)
            xcol.scale(scale);
        else
            xcol.Init(1.0, 0.0, 0.0);

        DVec3d zcol;
        zcol.CrossProduct(xcol, ycol);
        zcol.Normalize();

        // To get y-axis, cross z with x.
        ycol.CrossProduct(zcol, xcol);
        ycol.Normalize();

        orientation.InitFromColumnVectors(xcol, ycol, zcol);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::GetGlyphSymbology(ElemDisplayParamsR params) const
    {
    params.SetLineColor(m_style.GetColor());
    params.SetWeight((m_style.IsBold() && m_style.GetFont().CanDrawWithLineWeight()) ? DEFAULT_BOLD_WEIGHT : 0);
    
    params.SetFillDisplay(FillDisplay::Never);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
void TextString::DrawTextAdornments(ViewContextR context) const
    {
    if (DrawPurpose::RegionFlood == context.GetDrawPurpose())
        return;

    // The only current possible adornment is underline.
    if (!m_style.IsUnderlined())
        return;

    IDrawGeomR output = context.GetIDrawGeom();

    ElemDisplayParamsR elParams = *context.GetCurrentDisplayParams();
    GetGlyphSymbology(elParams);
    elParams.SetWeight(0); // IsBold should not affect underline.

    Update(); 
    
    DPoint3d pts[2];
    
    pts[0].x = m_range.low.x;
    pts[0].y = (m_range.low.y - (m_style.GetHeight() * DEFAULT_UNDERLINE_OFFSET_FACTOR));
    pts[0].z = 0.0;
    
    pts[1].x = m_range.high.x;
    pts[1].y = pts[0].y;
    pts[0].z = 0.0;

    context.PushTransform(ComputeTransform());
    context.CookDisplayParams();
    output.DrawLineString3d(2, pts, NULL);
    context.PopTransformClip();
    }

static const uint8_t CURRENT_STYLE_MAJOR_VERSION = 1;
static const uint8_t CURRENT_STYLE_MINOR_VERSION = 0;
static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::EncodeAsFlatBuf(Offset<FB::TextString>& textStringOffset, FlatBufferBuilder& encoder, TextStringCR text, DgnDbR db)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    //.............................................................................................
    if (nullptr == text.m_style.m_font)
        return ERROR;

    DgnFontId fontId = db.Fonts().AcquireId(*text.m_style.m_font);
    if (!fontId.IsValid())
        return ERROR;

    FB::TextStringStyleBuilder fbStyle(encoder);
    fbStyle.add_majorVersion(CURRENT_STYLE_MAJOR_VERSION);
    fbStyle.add_minorVersion(CURRENT_STYLE_MINOR_VERSION);
    fbStyle.add_color(text.m_style.m_color.GetValue());
    fbStyle.add_fontId(fontId.GetValue());
    fbStyle.add_isBold(text.m_style.m_isBold);
    fbStyle.add_isItalic(text.m_style.m_isItalic);
    fbStyle.add_isUnderlined(text.m_style.m_isUnderlined);
    fbStyle.add_height(text.m_style.m_size.y);
    fbStyle.add_widthFactor(text.m_style.m_size.x / text.m_style.m_size.y);

    Offset<FB::TextStringStyle> fbStyleOffset = fbStyle.Finish();

    //.............................................................................................
    Offset<String> textValueOffset = encoder.CreateString(text.m_text);
    Transform textTransform = Transform::FromMatrixAndFixedPoint(text.m_orientation, text.m_origin);

    FB::TextStringBuilder fbText(encoder);
    fbText.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbText.add_minorVersion(CURRENT_STYLE_MINOR_VERSION);
    fbText.add_text(textValueOffset);
    fbText.add_style(fbStyleOffset);
    fbText.add_transform(reinterpret_cast<FB::TextStringTransform*>(const_cast<TransformP>(&textTransform)));

    textStringOffset = fbText.Finish();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextStringCR text, DgnDbR db)
    {
    FlatBufferBuilder encoder;

    //.............................................................................................
    Offset<FB::TextString> textOffset;
    if (SUCCESS != EncodeAsFlatBuf(textOffset, encoder, text, db))
        return ERROR;

    //.............................................................................................
    encoder.Finish(textOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::DecodeFromFlatBuf(TextStringR text, FB::TextString const& fbText, DgnDbCR db)
    {
    text.Reset();
    
    if (!fbText.has_majorVersion() || fbText.majorVersion() > CURRENT_MAJOR_VERSION)
        return ERROR;
    
    if (!fbText.has_style() || !fbText.has_text())
        return ERROR;

    if (!fbText.style()->has_majorVersion() || fbText.style()->majorVersion() > CURRENT_STYLE_MAJOR_VERSION)
        return ERROR;

    if (!fbText.style()->has_fontId() || !fbText.style()->has_height())
        return ERROR;

    //.............................................................................................
    Transform textTransform = Transform::FromIdentity();
    if (fbText.has_transform())
        textTransform = *(TransformCP)fbText.transform();

    text.SetText(fbText.text()->c_str());
    textTransform.GetTranslation(text.m_origin);
    textTransform.GetMatrix(text.m_orientation);

    //.............................................................................................
    FB::TextStringStyle const& fbStyle = *fbText.style();
    TextStringStyle& style = text.m_style;

    if (fbStyle.has_color()) style.SetColor(ColorDef(fbStyle.color()));
    style.SetFont(T_HOST.GetFontAdmin().ResolveFont(db.Fonts().FindFontById(DgnFontId(fbStyle.fontId()))));
    if (fbStyle.has_isBold()) style.SetIsBold(0 != fbStyle.isBold());
    if (fbStyle.has_isItalic()) style.SetIsItalic(0 != fbStyle.isItalic());
    if (fbStyle.has_isUnderlined()) style.SetIsUnderlined(0 != fbStyle.isUnderlined());
    style.m_size.y = fbStyle.height();
    
    if (fbStyle.has_widthFactor())
        style.m_size.x = style.m_size.y * fbStyle.widthFactor();
    else
        style.m_size.x = style.m_size.y;
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::DecodeFromFlatBuf(TextStringR text, ByteCP buffer, size_t numBytes, DgnDbCR db)
    {
    FB::TextString const* fbText = GetRoot<FB::TextString>(buffer);
    return DecodeFromFlatBuf(text, *fbText, db);
    }
