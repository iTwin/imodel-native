/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextParamWideAndScale.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/TextBlock/TextBlockAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Venkat.Kalyan   02/2005
//=======================================================================================
struct TextParamAndScale
    {
    private:    TextParamWide   m_textParams;
    private:    DPoint2d        m_scale;
    private:    bool            m_changeable;

    public:                                                 TextParamAndScale       (TextParamWideCP pTextParams, DPoint2dCP pScale);
    public:                                                 TextParamAndScale       (RunPropertiesCR runProperties, ParagraphPropertiesCR paragraphProperties, TextBlockPropertiesCR textblockProperties, RunPropertiesCP textNodeProperties);
    public:                                                 TextParamAndScale       () { };
    public:                                                 ~TextParamAndScale      () { };

    public: DGNPLATFORM_EXPORT  TextParamWideP              GetTextParamWide        ()                                      { return &m_textParams; }
    public: DGNPLATFORM_EXPORT  DPoint2d*                   GetScale                ()                                      { return &m_scale; }

    public: DGNPLATFORM_EXPORT  void                        GetTextParamWide        (TextParamWideP pTextParams) const      { *pTextParams = m_textParams; }
    public: DGNPLATFORM_EXPORT  void                        GetScale                (DPoint2dP pScale) const;

    public:                     void                        SetTextParamWide        (TextParamWideCP pTextParams)           { m_textParams = *pTextParams; }
    public:                     void                        SetScale                (DPoint2dCP pScale)                     { m_scale = *pScale; }

    public:                     TextParamAndScaleP          Clone                   ();
    public:                     bool                        IsEqual                 (TextParamAndScaleP);
    public:                     bool                        IsEqual                 (TextParamWideCP, DPoint2dCP fontSize);

    public:                     bool                        IsChangeable            ()                                      { return m_changeable; }
    public:                     void                        SetChangeable           (bool changeable)                       { m_changeable = changeable; }

    public:                     void                        Validate                ();

    public:                     UInt32                      GetFontnumber           (void)                                  { return GetTextParamWide ()->font; }
    public:                     void                        SetFontnumber           (UInt32 fontNumber)                     { GetTextParamWide ()->font = fontNumber; }

    public:                     bool                        GetBold                 ()                                      { return GetTextParamWide ()->exFlags.bold ? true : false; }
    public:                     void                        SetBold                 (bool bold)                             { GetTextParamWide ()->exFlags.bold = bold ? true : false; }

    public:                     bool                        GetSlant                ()                                      { return GetTextParamWide ()->flags.slant ? true : false; }
    public:                     void                        SetSlant                (bool slant)                            { GetTextParamWide ()->flags.slant = slant ? true : false; }

    public:                     bool                        GetUnderline            ()                                      { return GetTextParamWide ()->flags.underline ? true : false; }
    public:                     void                        SetUnderline            (bool underline)                        { GetTextParamWide ()->flags.underline = underline ? true : false; }

    public:                     bool                        GetOverline             ()                                      { return GetTextParamWide ()->exFlags.overline ? true : false; }
    public:                     void                        SetOverline             (bool overline)                         { GetTextParamWide ()->exFlags.overline = overline ? true : false; }

    public:                     bool                        GetVertical             ()                                      { return GetTextParamWide ()->flags.vertical ? true : false; }
    public:                     void                        SetIsVertical           (bool vertical)                         { GetTextParamWide ()->flags.vertical = vertical ? true : false; }

    public:                     double                      GetSlantAngle           ()                                      { return GetSlant () ? GetTextParamWide ()->slant : false; }
    public:                     void                        SetSlantAngle           (double slantangle)                     { if (GetSlant ()) GetTextParamWide ()->slant = slantangle; }

    public:                     int                         GetColor                ()                                      { return GetTextParamWide ()->color; }
    public:                     void                        SetColor                (int colorIn)                           { GetTextParamWide ()->color = colorIn; GetTextParamWide ()->exFlags.color = true; }

    public:                     int                         GetTextStyleId          ()                                      { return GetTextParamWide ()->textStyleId; }
    public:                     TextElementJustification    GetJustification        ()                                      { return (TextElementJustification)GetTextParamWide ()->just; }
    public:                     void                        SetJustification        (TextElementJustification value)        { GetTextParamWide ()->just = (int)value; }

    }; // TextParamAndScale

END_BENTLEY_DGNPLATFORM_NAMESPACE
