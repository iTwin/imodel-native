/*--------------------------------------------------------------------------------------+
|     $Source: PublicAPI/DgnPlatform/DgnCore/TextString.h $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "DgnPlatform/DgnPlatform.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
struct TextStringStyle : public RefCountedBase
{
//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase)
    friend struct TextStyleInterop;

    ColorDef m_color;
    DgnFontCP m_font;
    bool m_isBold;
    bool m_isItalic;
    bool m_isUnderlined;
    DPoint2d m_size;

    // *************************************************************
    // **** ADDING MEMBERS? Consider updating: TextStyleInterop ****
    // *************************************************************

    void CopyFrom(TextStringStyleCR);

public:
    DGNPLATFORM_EXPORT TextStringStyle();
    DGNPLATFORM_EXPORT TextStringStyle(TextStringStyleCR);
    DGNPLATFORM_EXPORT TextStringStyleR operator=(TextStringStyleCR);
    
//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
    DGNPLATFORM_EXPORT static TextStringStylePtr Create();
    DGNPLATFORM_EXPORT TextStringStylePtr Clone() const;

    DGNPLATFORM_EXPORT ColorDefCR GetColor() const;
    DGNPLATFORM_EXPORT void SetColor(ColorDefCR);
    DGNPLATFORM_EXPORT DgnFontCR GetFont() const;
    DGNPLATFORM_EXPORT void SetFont(DgnFontCR);
    DGNPLATFORM_EXPORT bool IsBold() const;
    DGNPLATFORM_EXPORT void SetIsBold(bool);
    DGNPLATFORM_EXPORT bool IsItalic() const;
    DGNPLATFORM_EXPORT void SetIsItalic(bool);
    DGNPLATFORM_EXPORT bool IsUnderlined() const;
    DGNPLATFORM_EXPORT void SetIsUnderlined(bool);
    DGNPLATFORM_EXPORT DPoint2dCR GetSize() const;
    DGNPLATFORM_EXPORT void SetSize(DPoint2dCR);

    DGNPLATFORM_EXPORT double GetHeight() const;
    DGNPLATFORM_EXPORT void SetHeight(double);
    DGNPLATFORM_EXPORT double GetWidth() const;
    DGNPLATFORM_EXPORT void SetWidth(double);
    DGNPLATFORM_EXPORT void SetSize(double widthAndHeight);
    DGNPLATFORM_EXPORT void SetSize(double width, double height);

}; // TextStringStyle

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
struct TextString : public RefCountedBase
    {
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     01/2015
    //=======================================================================================
    enum struct HorizontalJustification
        {
        Left,
        Center,
        Right

        }; // HorizontalJustification
    
    //=======================================================================================
    // @bsiclass                                                    Jeff.Marker     01/2015
    //=======================================================================================
    enum struct VerticalJustification
        {
        Top,
        Middle,
        Bottom

        }; // VerticalJustification

//__PUBLISH_SECTION_END__
private:
    DEFINE_T_SUPER(RefCountedBase);
    
    Utf8String m_text;
    TextStringStyle m_style;
    DPoint3d m_origin;
    RotMatrix m_orientation;
    
    bool m_isValid;
    DRange2d m_range;
    bvector<DgnGlyphCP> m_glyphs;
    bvector<GlyphCode> m_glyphCodes;
    bvector<DPoint3d> m_glyphOrigins;

    void CopyFrom(TextStringCR);
    void Update() const;
    void Update();
    void ComputeAndLayoutGlyphs();
    DVec2d ComputeOffsetToJustification(HorizontalJustification, VerticalJustification) const;

public:
    DGNPLATFORM_EXPORT TextString();
    DGNPLATFORM_EXPORT TextString(TextStringCR);
    DGNPLATFORM_EXPORT TextStringR operator=(TextStringCR);

    DGNPLATFORM_EXPORT void ComputeBoundingShape(DPoint3dP) const;
    DGNPLATFORM_EXPORT void ComputeBoundingShape(DPoint3dP, double uniformPadding) const;
    DGNPLATFORM_EXPORT void ComputeBoundingShape(DPoint3dP, double horizontalPadding, double verticalPadding) const;
    DGNPLATFORM_EXPORT void ComputeGlyphAxes(DVec3dR, DVec3dR) const;
    DGNPLATFORM_EXPORT Transform ComputeTransform() const;
    DGNPLATFORM_EXPORT void DrawTextAdornments(ViewContextR) const;
    DGNPLATFORM_EXPORT void GetGlyphSymbology(ElemDisplayParamsR) const;
    DGNPLATFORM_EXPORT static void TransformOrientationAndExtractScale(DPoint2dR scaleFactor, RotMatrixR orientation, TransformCR transform);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    DGNPLATFORM_EXPORT static TextStringPtr Create();
    DGNPLATFORM_EXPORT TextStringPtr Clone() const;
    
    DGNPLATFORM_EXPORT Utf8StringCR GetText() const;
    DGNPLATFORM_EXPORT void SetText(Utf8CP);
    DGNPLATFORM_EXPORT TextStringStyleCR GetStyle() const;
    DGNPLATFORM_EXPORT TextStringStyleR GetStyleR();
    DGNPLATFORM_EXPORT void SetStyle(TextStringStyleCR);
    DGNPLATFORM_EXPORT DPoint3dCR GetOrigin() const;
    DGNPLATFORM_EXPORT void SetOrigin(DPoint3dCR);
    DGNPLATFORM_EXPORT RotMatrixCR GetOrientation() const;
    DGNPLATFORM_EXPORT void SetOrientation(RotMatrixCR);
    
    DGNPLATFORM_EXPORT void Invalidate();
    DGNPLATFORM_EXPORT DRange2dCR GetRange() const;
    DGNPLATFORM_EXPORT size_t GetNumGlyphs() const;
    DGNPLATFORM_EXPORT DgnGlyphCP const* GetGlyphs() const;
    DGNPLATFORM_EXPORT GlyphCodeCP GetGlyphCodes() const;
    DGNPLATFORM_EXPORT DPoint3dCP GetGlyphOrigins() const;

    DGNPLATFORM_EXPORT DPoint3d ComputeJustificationOrigin(HorizontalJustification, VerticalJustification) const;
    DGNPLATFORM_EXPORT void SetOriginFromJustificationOrigin(DPoint3dCR, HorizontalJustification, VerticalJustification);

}; // TextString

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
