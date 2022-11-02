/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatformInternal/DgnCore/TextStringPersistence.h>

template<typename T> static bool isEnumFlagSet(T testBit, T options) { return 0 != ((int)options & (int)testBit); }

using namespace flatbuffers;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextStringStyle::TextStringStyle() :
    T_Super(),
    m_isBold(false),
    m_isItalic(false),
    m_isUnderlined(false),
    m_size({ 0.0, 0.0 })
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextStringStyle::CopyFrom(TextStringStyleCR rhs)
    {
    m_font = rhs.m_font;
    m_isBold = rhs.m_isBold;
    m_isItalic = rhs.m_isItalic;
    m_isUnderlined = rhs.m_isUnderlined;
    m_size = rhs.m_size;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextStringStyle::Reset()
    {
    m_isBold = false;
    m_isItalic = false;
    m_isUnderlined = false;
    m_size.Zero();
    }

static const double DEFAULT_ITALIC_ANGLE = Angle::DegreesToRadians(30.0);
static const uint32_t DEFAULT_BOLD_WEIGHT = 2;
static const double DEFAULT_UNDERLINE_OFFSET_FACTOR = 0.15;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TextString::TextString(DgnDbR db) : T_Super(), m_db(&db)
    {
    m_origin.Zero();
    m_orientation.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::CopyFrom(TextStringCR rhs)
    {
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
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::Reset()
    {
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
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::ComputeGlyphAxes(DVec3dR xAxis, DVec3dR yAxis) const
    {
    DPoint2d size = m_style.GetSize();

    xAxis.Init(size.x, 0.0, 0.0);
    yAxis.Init(0.0, size.y, 0.0);

    auto fontType = m_db->Fonts().GetFontType(m_style.GetFont());
    if (m_style.IsItalic() && (FontType::TrueType != fontType))
        yAxis.x = (tan(DEFAULT_ITALIC_ANGLE) * size.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::ComputeBoundingShape(DPoint3dP boxPts, double horizontalPadding, double verticalPadding) const
    {
    Update();
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::SetOriginFromJustificationOrigin(DPoint3dCR jorigin, HorizontalJustification hjust, VerticalJustification vjust)
    {
    DPoint3d offsetFromJustification = DPoint3d::From(ComputeOffsetToJustification(hjust, vjust));
    m_orientation.Multiply(offsetFromJustification);

    m_origin = jorigin;
    m_origin.Subtract(offsetFromJustification);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::Update()
    {
    if (m_isValid)
        return;

    ComputeAndLayoutGlyphs();
    m_isValid = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::ComputeAndLayoutGlyphs()
    {
    m_range.Init();
    m_glyphs.clear();
    m_glyphIds.clear();
    m_glyphOrigins.clear();

    if (m_text.empty())
        return;

    auto& font = ResolveFont();
    GlyphLayoutContext layoutContext;
    layoutContext.m_string = m_text;
    layoutContext.m_drawSize = m_style.GetSize();
    layoutContext.m_isBold = m_style.IsBold();
    layoutContext.m_isItalic = (m_style.IsItalic() && FontType::TrueType == font.GetType());

    GlyphLayoutResult layoutResult;
    if (SUCCESS != font.LayoutGlyphs(layoutResult, layoutContext))
        return;

    auto numGlyphs = layoutResult.m_glyphs.size();
    if (0 == numGlyphs)
        return;

    m_range = layoutResult.m_justificationRange;

    m_glyphs.resize(numGlyphs);
    memcpy(&m_glyphs[0], &layoutResult.m_glyphs[0], sizeof(decltype(m_glyphs)::value_type) * numGlyphs);

    m_glyphIds.clear();
    for (DbGlyphCP glyph : layoutResult.m_glyphs)
        {
        if (nullptr == glyph)
            m_glyphIds.push_back(0);
        else
            m_glyphIds.push_back(glyph->GetId());
        }

    m_glyphOrigins.resize(numGlyphs);
    memcpy(&m_glyphOrigins[0], &layoutResult.m_glyphOrigins[0], sizeof(decltype(m_glyphOrigins)::value_type) * numGlyphs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::TransformOrientationAndExtractScale(DPoint2dR scaleFactor, RotMatrixR orientation, TransformCR transform)
    {
    orientation.InitProduct(transform, orientation);

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
            xcol.Scale(scale);
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
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::ApplyTransform(TransformCR transform)
    {
    DPoint3d newOrigin = GetOrigin();
    transform.Multiply(newOrigin);

    RotMatrix newOrientation = GetOrientation();
    DPoint2d scaleFactor;
    TextString::TransformOrientationAndExtractScale(scaleFactor, newOrientation, transform);

    SetOrigin(newOrigin);
    SetOrientation(newOrientation);

    // We used to just call 'GetStyleR().SetSize(newSize)', which would invalidate the layout and cause a new layout pass with the new size when asked later.
    // (1) This is expensive for little good reason
    // (2) During DgnV8 convert, we go to great lengths to hack up a WYSIWYG DB TextString from a DgnV8 TextString, and cannot tolerate a DB-based layout pass due to this.
    // As such, I am going to try to adjust size directly without going through a layout pass.
    // I believe this is "good enough". Using the real size when doing text layout may result in slightly different results due to precision and layout intricacies.

    DPoint2d newSize = GetStyle().GetSize();
    newSize.x *= scaleFactor.x;
    newSize.y *= scaleFactor.y;

    m_style.SetSize(newSize); // Do NOT call GetStyleR, which calls Invalidate for you; set size directly.

    if (m_isValid)
        {
        m_range.low.x *= scaleFactor.x;
        m_range.low.y *= scaleFactor.y;
        m_range.high.x *= scaleFactor.x;
        m_range.high.y *= scaleFactor.y;

        for (DPoint3dR glyphOrigin : m_glyphOrigins)
            {
            glyphOrigin.x *= scaleFactor.x;
            glyphOrigin.y *= scaleFactor.y;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TextString::GetGlyphSymbology(GeometryParamsR params) const
    {
    // Generally speaking, use the GeometryParams that the user already set up...
    bool changed = false;

    auto& font = ResolveFont();
    // Though we may actually care about weight...
    uint32_t weight = ((m_style.IsBold() && font.CanDrawWithLineWeight()) ? DEFAULT_BOLD_WEIGHT : 0);

    if (params.IsWeightFromSubCategoryAppearance() || weight != params.GetWeight())
        {
        params.SetWeight(weight);
        changed = true;
        }

    // And we should force these values, lest they interfere under various scenarios. NOT SURE THIS IS STILL NECESSARY...
    if (params.IsLineStyleFromSubCategoryAppearance() || nullptr != params.GetLineStyle())
        {
        params.SetLineStyle(nullptr);
        changed = true;
        }

    if (FillDisplay::Never != params.GetFillDisplay())
        {
        params.SetFillDisplay(FillDisplay::Never);
        changed = true;
        }

    return changed;
    }

DbFontR TextString::ResolveFont() const {
    return m_db->Fonts().FindFont(m_style.GetFont());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TextString::GetUnderline(DSegment3dR underline) const
    {
    if (!m_style.IsUnderlined())
        return false;

    Update();

    underline.point[0].x = m_range.low.x;
    underline.point[1].x = m_range.high.x;
    underline.point[0].y = underline.point[1].y = (m_range.low.y - (m_style.GetHeight() * DEFAULT_UNDERLINE_OFFSET_FACTOR));
    underline.point[0].z = underline.point[1].z = 0.0;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TextString::AddUnderline(Render::GraphicBuilderR graphic) const
    {
    DSegment3d underline;
    if (GetUnderline(underline))
        graphic.AddLineString(2, underline.point);
    }

static const uint8_t CURRENT_STYLE_MAJOR_VERSION = 1;
static const uint8_t CURRENT_STYLE_MINOR_VERSION = 0;
static const uint8_t CURRENT_MAJOR_VERSION = 1;
static const uint8_t CURRENT_MINOR_VERSION = 0;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::EncodeAsFlatBuf(Offset<FB::TextString>& textStringOffset, FlatBufferBuilder& encoder, TextStringCR text, DgnDbR db, FlatBufEncodeOptions options)
    {
    // I prefer to ensure encoders write default values instead of it being unknown later if it's really a default value, or if the encoder missed it and it's bad data.
    TemporaryForceDefaults forceDefaults(encoder, true);

    FontId fontId = text.m_style.m_font;
    if (!fontId.IsValid())
        return ERROR;

    FB::TextStringStyleBuilder fbStyle(encoder);
    fbStyle.add_majorVersion(CURRENT_STYLE_MAJOR_VERSION);
    fbStyle.add_minorVersion(CURRENT_STYLE_MINOR_VERSION);

    // we're going to store the fontid as a 32 bit value, even though in memory we have a 64bit value. Make sure the high bits are 0.
    BeAssert(fontId.GetValue() == (int64_t)((uint32_t)fontId.GetValue()));
    fbStyle.add_fontId((uint32_t)fontId.GetValue());

    fbStyle.add_isBold(text.m_style.m_isBold);
    fbStyle.add_isItalic(text.m_style.m_isItalic);
    fbStyle.add_isUnderlined(text.m_style.m_isUnderlined);
    fbStyle.add_height(text.m_style.m_size.y);
    fbStyle.add_widthFactor(text.m_style.m_size.x / text.m_style.m_size.y);

    Offset<FB::TextStringStyle> fbStyleOffset = fbStyle.Finish();

    Offset<Vector<uint32_t>> glypIdsOffset;
    Offset<Vector<FB::TextStringGlyphOrigin const*>> glypOriginsOffset;
    if (isEnumFlagSet(FlatBufEncodeOptions::IncludeGlyphLayoutData, options))
        {
        bvector<uint32_t> glyphIds;
        bvector<FB::TextStringGlyphOrigin> glyphOrigins;
        for (size_t iGlyph = 0; iGlyph < text.GetNumGlyphs(); ++iGlyph)
            {
            glyphIds.push_back(text.GetGlyphIds()[iGlyph]);

            DPoint3dCR glyphOrigin = text.GetGlyphOrigins()[iGlyph];
            glyphOrigins.push_back(FB::TextStringGlyphOrigin(glyphOrigin.x, glyphOrigin.y));
            }

        glypIdsOffset = encoder.CreateVector(glyphIds);
        glypOriginsOffset = encoder.CreateVectorOfStructs(glyphOrigins);
        }

    Offset<String> textValueOffset = encoder.CreateString(text.m_text);
    Transform textTransform = Transform::From(text.m_orientation, text.m_origin);

    FB::TextStringBuilder fbText(encoder);
    fbText.add_majorVersion(CURRENT_MAJOR_VERSION);
    fbText.add_minorVersion(CURRENT_MINOR_VERSION);
    fbText.add_text(textValueOffset);
    fbText.add_style(fbStyleOffset);

    if (!textTransform.IsIdentity())
        fbText.add_transform(reinterpret_cast<FB::TextStringTransform*>(const_cast<TransformP>(&textTransform)));

    if (isEnumFlagSet(FlatBufEncodeOptions::IncludeGlyphLayoutData, options))
        {
        fbText.add_range(reinterpret_cast<FB::TextStringRange const*>(&text.GetRange()));
        fbText.add_glyphIds(glypIdsOffset);
        fbText.add_glyphOrigins(glypOriginsOffset);
        }

    textStringOffset = fbText.Finish();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::EncodeAsFlatBuf(bvector<Byte>& buffer, TextStringCR text, DgnDbR db, FlatBufEncodeOptions options)
    {
    FlatBufferBuilder encoder;

    Offset<FB::TextString> textOffset;
    if (SUCCESS != EncodeAsFlatBuf(textOffset, encoder, text, db, options))
        return ERROR;

    encoder.Finish(textOffset);
    buffer.resize(encoder.GetSize());
    memcpy(&buffer[0], encoder.GetBufferPointer(), encoder.GetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::DecodeFromFlatBuf(TextStringR text, FB::TextString const& fbText)
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

    Transform textTransform = Transform::FromIdentity();
    if (fbText.has_transform())
        textTransform = *(TransformCP)fbText.transform();

    text.SetText(fbText.text()->c_str());
    textTransform.GetTranslation(text.m_origin);
    textTransform.GetMatrix(text.m_orientation);

    FB::TextStringStyle const& fbStyle = *fbText.style();
    TextStringStyle& style = text.m_style;

    style.SetFont(FontId((uint64_t)fbStyle.fontId()));
    if (fbStyle.has_isBold()) style.SetIsBold(0 != fbStyle.isBold());
    if (fbStyle.has_isItalic()) style.SetIsItalic(0 != fbStyle.isItalic());
    if (fbStyle.has_isUnderlined()) style.SetIsUnderlined(0 != fbStyle.isUnderlined());
    style.m_size.y = fbStyle.height();

    if (fbStyle.has_widthFactor())
        style.m_size.x = style.m_size.y * fbStyle.widthFactor();
    else
        style.m_size.x = style.m_size.y;

    auto& font = text.ResolveFont();

    // some converters store both the textstring and the glyph layout information so that visual fidelity with a source application is guaranteed.
    // However, if a fallback font is being used, that information isn't valid.
    if (!font.IsFallback() && fbText.has_range() && fbText.has_glyphIds() && fbText.has_glyphOrigins() && fbText.glyphIds()->size()>0  && fbText.glyphOrigins()->size()>0) {
        text.m_range = *reinterpret_cast<DRange2dCP>(fbText.range());
        FaceStyle faceStyle = FontManager::FaceStyleFromBoldItalic(text.m_style.m_isBold, text.m_style.m_isItalic);

        // See comments in TextStringPersistence::EncodeAsFlatBuf as to why these transforms occur.
        for (uoffset_t iGlyph = 0; iGlyph < fbText.glyphIds()->size(); ++iGlyph) {
            uint32_t glyphId = fbText.glyphIds()->Get(iGlyph);
            text.m_glyphIds.push_back(glyphId);
            text.m_glyphOrigins.push_back(DPoint3d::From(*reinterpret_cast<DPoint2dCP>(fbText.glyphOrigins()->Get(iGlyph))));
            text.m_glyphs.push_back(font.FindGlyphCP(glyphId, faceStyle));
        }

        text.m_isValid = true;
    }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStringPersistence::DecodeFromFlatBuf(TextStringR text, Byte const* buffer, size_t numBytes)
    {
    FB::TextString const* fbText = GetRoot<FB::TextString>(buffer);
    return DecodeFromFlatBuf(text, *fbText);
    }
