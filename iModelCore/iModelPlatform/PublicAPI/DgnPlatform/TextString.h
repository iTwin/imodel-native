/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnPlatform/DgnPlatform.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
//! Represents the visual style of a TextString.
// @bsiclass
//=======================================================================================
struct TextStringStyle : public RefCountedBase
{
private:
    DEFINE_T_SUPER(RefCountedBase);
    friend struct TextString;
    friend struct TextStyleInterop;
    friend struct TextStringPersistence;

    FontId m_font;
    bool m_isBold;
    bool m_isItalic;
    bool m_isUnderlined;
    DPoint2d m_size;

    void Reset();
    DGNPLATFORM_EXPORT void CopyFrom(TextStringStyleCR);

public:
    DGNPLATFORM_EXPORT TextStringStyle();
    TextStringStyle(TextStringStyleCR rhs) : T_Super(rhs) { CopyFrom(rhs); }
    TextStringStyleR operator=(TextStringStyleCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this; }
    static TextStringStylePtr Create() { return new TextStringStyle(); }
    TextStringStylePtr Clone() const { return new TextStringStyle(*this); }

    FontId GetFont() const { return m_font; }
    void SetFont(FontId value) { m_font = value; }
    bool IsBold() const { return m_isBold; }
    void SetIsBold(bool value) { m_isBold = value; }
    bool IsItalic() const { return m_isItalic; }
    void SetIsItalic(bool value) { m_isItalic = value; }
    bool IsUnderlined() const { return m_isUnderlined; }
    void SetIsUnderlined(bool value) { m_isUnderlined = value; }
    DPoint2dCR GetSize() const { return m_size; }
    void SetSize(DPoint2dCR value) { m_size = value; }
    double GetHeight() const { return m_size.y; }
    void SetHeight(double value) { m_size.y = value; }
    double GetWidth() const { return m_size.x; }
    void SetWidth(double value) { m_size.x = value; }
    void SetSize(double widthAndHeight) { SetSize(widthAndHeight, widthAndHeight); }
    void SetSize(double width, double height) { m_size.x = width; m_size.y = height; }
};

//=======================================================================================
//! A light-weight object that represents a run of single-line, single-format characters for display purposes.
//! Glyph-based computations can be expensive. If you repeatedly need glyph, glyph origin, or range results, you should compute them once and cache them. This object also defers glyph-based computations until actually requested. This allows you to create the object and configure it with no overhead; then, the next glyph-based computation requested will trigger the expensive glyph layout call, and the results are cached in this object until invalidated by a set method.
//! Note that range and glyph origin values are local to this object, and do NOT automatically apply the origin and orientation transforms. If you need range and glyph origins in world coordinates, you must manually apply ComputeTransform to them.
//! A TextString's origin is always its lower-left baseline, and it orients about this point. "Justification" origin can be translated via ComputeJustificationOrigin and SetOriginFromJustificationOrigin for orienting about a different anchor point.
// @bsiclass
//=======================================================================================
struct TextString : public RefCountedBase
    {
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    enum class HorizontalJustification { Left, Center, Right };

    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    enum class VerticalJustification { Top, Middle, Bottom };

private:
    DEFINE_T_SUPER(RefCountedBase);
    friend struct TextStringPersistence;

    DgnDbP m_db;
    Utf8String m_text;
    TextStringStyle m_style;
    DPoint3d m_origin;
    RotMatrix m_orientation;

    bool m_isValid = false;
    DRange2d m_range;
    bvector<DbGlyphCP> m_glyphs;
    bvector<uint32_t> m_glyphIds;
    bvector<DPoint3d> m_glyphOrigins;

    void Reset();
    DGNPLATFORM_EXPORT void CopyFrom(TextStringCR);
    void Update() const { const_cast<TextStringP>(this)->Update(); }
    DGNPLATFORM_EXPORT void Update();
    void ComputeAndLayoutGlyphs();
    DVec2d ComputeOffsetToJustification(HorizontalJustification, VerticalJustification) const;

public:
    DGNPLATFORM_EXPORT TextString(DgnDbR);
    TextString(TextStringCR rhs) : T_Super(rhs), m_db(rhs.m_db) { CopyFrom(rhs); }
    TextStringR operator=(TextStringCR rhs) { T_Super::operator=(rhs); if (&rhs != this) CopyFrom(rhs); return *this; }
    static TextStringPtr Create(DgnDbR db) { return new TextString(db); }
    TextStringPtr Clone() const { return new TextString(*this); }

    Utf8StringCR GetText() const { return m_text; }
    void SetText(Utf8CP value) { Invalidate(); m_text.AssignOrClear(value); }
    TextStringStyleCR GetStyle() const { return m_style; }
    TextStringStyleR GetStyleR() { Invalidate(); return m_style; }
    void SetStyle(TextStringStyleCR value) { Invalidate(); m_style = value; }
    DPoint3dCR GetOrigin() const { return m_origin; }
    void SetOrigin(DPoint3dCR value) { m_origin = value; }
    RotMatrixCR GetOrientation() const { return m_orientation; }
    void SetOrientation(RotMatrixCR value) { m_orientation = value; }

    DgnDbR GetDgnDb() { return *m_db; }
    void ChangeDgnDb(DgnDbP db) { m_db = db; }
    DGNPLATFORM_EXPORT DbFontR ResolveFont() const;

    //! Causes the next call to get glyph-based results (e.g. glyphs, glyph origins, range) to perform a new glyph layout pass. Glyph layout can be slow, so it is best to minimize the number of times this is called.
    void Invalidate() { m_isValid = false; }
    //! @note This is a glyph-based computation.
    DRange2dCR GetRange() const { Update(); return m_range; }
    //! @note This is a glyph-based computation.
    size_t GetNumGlyphs() const { Update(); return m_glyphIds.size(); }
    //! @note This is a glyph-based computation.
    DbGlyphCP const* GetGlyphs() const { Update(); return &m_glyphs[0]; }
    //! @note This is a glyph-based computation.
    uint32_t const* GetGlyphIds() const { Update(); return &m_glyphIds[0]; }
    //! @note This is a glyph-based computation.
    DPoint3dCP GetGlyphOrigins() const { Update(); return &m_glyphOrigins[0]; }
    //! @note This is a glyph-based computation.
    void ComputeBoundingShape(DPoint3dP boxPts) const { ComputeBoundingShape(boxPts, 0.0, 0.0); }
    //! @note This is a glyph-based computation.
    void ComputeBoundingShape(DPoint3dP boxPts, double uniformPadding) const { ComputeBoundingShape(boxPts, uniformPadding, uniformPadding); }
    //! @note This is a glyph-based computation.
    DGNPLATFORM_EXPORT void ComputeBoundingShape(DPoint3dP, double horizontalPadding, double verticalPadding) const;
    Transform ComputeTransform() const { return Transform::From(m_orientation, m_origin); }
    DGNPLATFORM_EXPORT DPoint3d ComputeJustificationOrigin(HorizontalJustification, VerticalJustification) const;
    DGNPLATFORM_EXPORT void SetOriginFromJustificationOrigin(DPoint3dCR, HorizontalJustification, VerticalJustification);
    //! This will attempt to intelligently transform this instance. This means scale is extracted an applied to the style's height and width, translation is applied to the origin, and the remainder is applied to the orientation.
    DGNPLATFORM_EXPORT void ApplyTransform(TransformCR);

    //! Computes scaled X and Y axes for providing to rendering systems. Non-italic text will have perpendicular axes; italic text will have a skewed Y axis.
    //! @private
    DGNPLATFORM_EXPORT void ComputeGlyphAxes(DVec3dR, DVec3dR) const;
    //! This is typically used, internally, to draw the underline of text, which can be distinct from drawing the actual glyphs in some APIs.
    //! @private
    DGNPLATFORM_EXPORT void AddUnderline(Render::GraphicBuilderR) const;
    DGNPLATFORM_EXPORT bool GetUnderline(DSegment3dR underline) const;
    //! Applies the relevant TextStringStyle properties to an GeometryParams. Most GeometryParams members are left untouched.
    //! @return true if changed.
    //! @private
    DGNPLATFORM_EXPORT bool GetGlyphSymbology(Render::GeometryParamsR) const;
    //! Decomposes a Transform into an orientation and a 2D scale factor. This is useful for text, because scale should be transferred to the style's height and width, but orientation should remain as RotMatrix on the TextString.
    //! @private
    DGNPLATFORM_EXPORT static void TransformOrientationAndExtractScale(DPoint2dR scaleFactor, RotMatrixR orientation, TransformCR transform);
};

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
