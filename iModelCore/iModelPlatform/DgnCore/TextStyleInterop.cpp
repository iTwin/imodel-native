/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/TextStyleInterop.h>

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStyleInterop::AnnotationToTextString(TextStringStyleR tss, AnnotationTextStyleCR ats)
    {
    tss.m_font = ats.GetFontId();
    tss.m_isBold = ats.IsBold();
    tss.m_isItalic = ats.IsItalic();
    tss.m_isUnderlined = ats.IsUnderlined();
    tss.m_size.x = (ats.GetWidthFactor() * ats.GetHeight());
    tss.m_size.y = ats.GetHeight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus TextStyleInterop::TextStringToAnnotation(DgnDbR db, AnnotationTextStyleR ats, TextStringStyleR tss)
    {
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::FontId, tss.GetFont().GetValue());
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsBold, tss.m_isBold ? 1 : 0);
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsItalic, tss.m_isItalic? 1 : 0);
    ats.m_data.SetIntegerProperty(AnnotationTextStyleProperty::IsUnderlined, tss.m_isUnderlined? 1 : 0);
    ats.m_data.SetRealProperty(AnnotationTextStyleProperty::Height, tss.m_size.y);
    ats.m_data.SetRealProperty(AnnotationTextStyleProperty::WidthFactor, (tss.m_size.x / tss.m_size.y));

    return SUCCESS;
    }
