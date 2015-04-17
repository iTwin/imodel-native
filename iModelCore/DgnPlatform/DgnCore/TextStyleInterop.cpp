/*--------------------------------------------------------------------------------------+
|     $Source: DgnCore/TextStyleInterop.cpp $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/TextStyleInterop.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStyleInterop::AnnotationToTextString(TextStringStyleR tss, AnnotationTextStyleCR ats)
    {
    tss.m_color = ats.GetColor();
    tss.m_font = &DgnFontManager::ResolveFont(ats.GetFontId(), ats.GetDgnProjectR(), DGNFONTVARIANT_DontCare);
    tss.m_isBold = ats.IsBold();
    tss.m_isItalic = ats.IsItalic();
    tss.m_isUnderlined = ats.IsUnderlined();
    tss.m_size.x = (ats.GetWidthFactor() * ats.GetHeight());
    tss.m_size.y = ats.GetHeight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     01/2015
//---------------------------------------------------------------------------------------
BentleyStatus TextStyleInterop::TextStringToAnnotation(DgnDbR db, AnnotationTextStyleR ats, TextStringStyleR tss)
    {
    uint32_t fontID;
    db.Fonts().AcquireFontNumber(fontID, *tss.m_font);
    
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::ColorId, tss.m_color.GetValue());
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::FontId, fontID);
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsBold, tss.m_isBold ? 1 : 0);
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, tss.m_isItalic? 1 : 0);
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, tss.m_isUnderlined? 1 : 0);
    ats.m_data.SetRealProperty(AnnotationTextStyleProperty::Height, tss.m_size.y);
    ats.m_data.SetRealProperty(AnnotationTextStyleProperty::WidthFactor, (tss.m_size.x / tss.m_size.y));

    return SUCCESS;
    }
