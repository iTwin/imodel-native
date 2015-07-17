/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMTextDrawer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

bool AddDTMTextStyle (EditElementHandle& elem, UInt32 textStyleId, UInt32 key);
bool GetDTMTextParam (ElementHandleCR elem, TextParamWideR textParamWide, double& fontSizeX, double& fontSizeY, DgnModelRefP model, UInt32 key);
int GetDTMTextParamId (ElementHandleCR elem, UInt32 key);

struct TextDrawer
    {
private:
    static const double LABEL_SIZE_THRESHOLD;
    static const double LABEL_LINE_SIZE_THRESHOLD;
    mutable bool m_drawTextAsLine;
    TextParamWide m_textParamWide;
    DPoint2d m_fontSizeUOR;
    DPoint2d m_fontSize;
    mutable TextStringProperties m_textStringProperties;
    mutable bool m_hasTextStringProperties;
    DgnFileP m_dgnFile;
    ViewContextR m_context;
    mutable TextStringPtr m_tempTextString;

    bool m_isValid;

    public: struct TextItem
        {
        DPoint3d    m_origin;
        double      m_angle;
        public: TextItem (DPoint3dCR origin, double angle) : m_origin (origin), m_angle (angle)
                {}
        }; // End ContourLabelItem struct
    typedef bvector <TextItem> TextItems;
    public:
        TextDrawer (const DTMDrawingInfo& drawingInfo, ViewContextR context, int textStyleId);
            
        void FixBG (bool bgFlag = false, UInt32 bgColor = 0);

        void SetJustification (TextElementJustification just);


        void DrawMultipleUsingCache (WStringCR text, const TextItems& items, bool useUORs = true) const;
        void DrawText (WStringCR text, DPoint3dR origin, double angle, bool useUORs = true) const;
        void DrawText (WStringCR text, DPoint3dR origin, RotMatrixCP rMatrix = nullptr, bool useUORs = true) const;

        IDisplaySymbol* CreateTextSymbol (WStringCR text, bool useUORs = true) const;

    bool IsTextVisible (double pixelSize) const
        {
        if (pixelSize > 0)
            {
            m_drawTextAsLine = !((m_fontSizeUOR.y / pixelSize) > LABEL_LINE_SIZE_THRESHOLD);
            return (m_fontSizeUOR.y / pixelSize) > LABEL_SIZE_THRESHOLD;
            }
        m_drawTextAsLine = false;
        return true;
        }
    const DPoint2d& FontSize() const
        {
        return m_fontSize;
        }

    const DPoint2d& FontSizeUOR() const
        {
        return m_fontSizeUOR;
        }
    bool IsValid () const
        {
        return m_isValid;
        }

    void Scale (double scale)
        {
        m_fontSizeUOR.Scale (scale);
        m_fontSize.Scale (scale);
        }
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
