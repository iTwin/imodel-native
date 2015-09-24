/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMTextDrawer.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h" 
#include "DTMTextDrawer.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

bool AddDTMTextStyle (EditElementHandle& elem, uint32_t textStyleId, uint32_t key)
    {
    DgnTextStylePtr textStyle;

    // ToDo Vancouver
    if (textStyleId == 0)
        {
        DTMElementHandlerManager::IDTMElementPowerPlatformExtension* extension = DTMElementHandlerManager::GetDTMElementPowerPlatformExtension ();
        if (extension)
            textStyle = extension->GetActiveStyle (*elem.GetDgnFileP());
        else
            textStyle = DgnTextStyle::Create ((WCharCP)nullptr, *elem.GetDgnFileP());
        }
    else
        textStyle = DgnTextStyle::GetByID (textStyleId, *elem.GetDgnFileP());

    DgnTextStylePtr currentTextStyle = DgnTextStyle::ExtractFromElement (elem, key);

    if (!textStyle.IsValid() && !currentTextStyle.IsValid())
        return false;

    if (textStyle.IsValid())
        {
        if (currentTextStyle.IsValid() && textStyle->Compare (*currentTextStyle.get())->GetFlagsCount() == 0) //   IsFlagsEmpty() )
            return false;

        textStyle->AddToElement (elem, key);
        }
    else
        DgnTextStyle::DeleteFromElement (elem, key);
    return true;
    }

int GetDTMTextParamId (ElementHandleCR elem, uint32_t key)
    {
    DgnTextStylePtr textStyle = DgnTextStyle::ExtractFromElement (elem, key);
    
    if (!textStyle.IsNull())
        return (int) textStyle->GetID();    // ToDo Vancouver - ElementId (uint64_t) to (int) - possible loss of data
    return 0;
    }

bool GetDTMTextParam
(
ElementHandleCR    elem,
TextParamWideR  textParamWide,
double&         fontSizeX,
double&         fontSizeY,
DgnModelRefP    model,
uint32_t          key
)
    {
    DgnTextStylePtr textStyle = DgnTextStyle::ExtractFromElement (elem, key);
    
    if (textStyle.IsNull())
        {
        DgnFileP file = elem.GetDgnFileP ();
        if (file == nullptr)
            {
            DgnModelRefP model2 = model;
            
            if (model2 == nullptr)
                model2 = BENTLEY_NAMESPACE_NAME::TerrainModel::Element::GetModelRef (elem);
            if (model2 == nullptr)
               model2 = GetActivatedModel (elem, nullptr); //TODO Is this needed? 
            file = model2->GetDgnFileP();
            if (file == nullptr)
                return false;
            }
//ToDo        textStyle = DgnTextStyle::GetActiveStyle (*file);
        }

    if (textStyle.IsNull())
        {
        return false;
        }

    textStyle->GetTextParamWide (textParamWide, &fontSizeX, &fontSizeY, NULL, elem.GetDgnModelP());
//ToDo    DgnModelP cache = model->GetDgnModelP();
//ToDo    double annotScale = dgnModel_getEffectiveAnnotationScale(cache);

//ToDo    textstyle_getTextParamWideFromTextStyleEx (&textParamWide, x, y, nullptr, &ts, -1, false, true, elem.GetModelRef(), model, annotScale, dgnModel_getModelFlag (cache, MODELFLAG_USE_ANNOTATION_SCALE));

    if (textParamWide.exFlags.annotationScale)
        {
        fontSizeX *= textParamWide.annotationScale;
        fontSizeY *= textParamWide.annotationScale;
        }
    else
        {
        double annotationScale = dgnModel_getEffectiveAnnotationScale (elem.GetDgnModelP());
        fontSizeX *= annotationScale;
        fontSizeY *= annotationScale;
        }
    if (elem.GetModelRef() && elem.GetModelRef() != model /* ToDo && !mdlModelRef_isActiveModel (elem.GetModelRef())*/)
        {
        double factor;

        factor = dgnModel_getUorPerStorage (elem.GetDgnModelP()) / dgnModel_getUorPerStorage (model->GetDgnModelP());
        fontSizeX *= factor;
        fontSizeY *= factor;
        }
    return true;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct TextSymbol : public IDisplaySymbol
{
private:

//mutable EditElementHandle  m_eeh;
TextStringPtr m_text;
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
TextSymbol (TextStringPtr text) : m_text(text)
    {
    }
virtual ~TextSymbol()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetRange (DRange3dR range) const override
    {
    DRange2d range2d = m_text->GetExtents ();

    range.low.x = range2d.low.x;
    range.low.y = range2d.low.y;
    range.low.z = 0;
    range.high.x = range2d.high.x;
    range.high.y = range2d.high.y;
    range.high.z = 0;
    //DisplayHandlerP dHandler = m_eeh.GetDisplayHandler ();

    //if (!dHandler)
    //    return ERROR;

    //return dHandler->CalcElementRange (m_eeh, range, NULL);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Draw (ViewContextR context) override
    {
    context.DrawTextString (*m_text);
    }

}; // TextSymbol

const double TextDrawer::LABEL_SIZE_THRESHOLD = 2;
const double TextDrawer::LABEL_LINE_SIZE_THRESHOLD = 2;// This is disabling the draw as line for now.

TextDrawer::TextDrawer (const DTMDrawingInfo& drawingInfo, ViewContextR context, int textStyleId) : m_context (context), m_textStringProperties (*context.GetCurrentModel ()->GetDgnModelP ())
    {
    m_hasTextStringProperties = false;
    memset (&m_textParamWide, 0, sizeof m_textParamWide);

    m_dgnFile = drawingInfo.GetSymbologyElement().GetDgnFileP();
    m_isValid = GetDTMTextParam (drawingInfo.GetSymbologyElement(), m_textParamWide, m_fontSizeUOR.x, m_fontSizeUOR.y, GetActivatedModel (drawingInfo.GetSymbologyElement(), &context), textStyleId);

    m_fontSize.x = drawingInfo.ScaleUorsToStorage (m_fontSizeUOR.x);
    m_fontSize.y = drawingInfo.ScaleUorsToStorage (m_fontSizeUOR.y);
    }

void TextDrawer::FixBG (bool bgFlag, uint32_t bgColor)
    {
    // Fix the BG, no need to restore it.
    m_textParamWide.flags.bgColor = bgFlag ? 1 : 0;
    m_textParamWide.backgroundColor = bgColor;
    }

void TextDrawer::SetJustification (TextElementJustification just)
    {
    // ToDo this sould use the Vertical justification.
    m_textParamWide.just = just;
    }

IDisplaySymbolP TextDrawer::CreateTextSymbol (WStringCR text, bool useUORs) const
    {
    if (!m_hasTextStringProperties)
        {
        m_hasTextStringProperties = true;
        m_textStringProperties.FromElementData (m_textParamWide, useUORs ? m_fontSizeUOR : m_fontSize);
        }
    TextStringPtr textString = TextString::Create (text.c_str(), NULL, NULL, m_textStringProperties);

    DPoint3d zeroPt;
    zeroPt.Zero();
    textString->SetOriginFromUserOrigin (zeroPt);

    return new TextSymbol (textString);
    }

void TextDrawer::DrawMultipleUsingCache (WStringCR text, const TextItems& items, bool useUORs) const
    {
    IDisplaySymbolP textSymbol = CreateTextSymbol (text, useUORs);
    for (TextDrawer::TextItems::const_iterator p = items.begin (); p != items.end(); ++p)
        {
        RotMatrix   rMatrix;
        Transform   tr;

        rMatrix.InitFromAxisAndRotationAngle (2, p->m_angle);

        tr.InitFrom (rMatrix, p->m_origin);
        m_context.DrawSymbol (textSymbol, &tr, NULL, false, false);
        }
    if (textSymbol)
        {
        m_context.DeleteSymbol (textSymbol); // Only needed if DrawSymbol has been called...i.e. early returns are ok!
        delete textSymbol;
        }
    }

void TextDrawer::DrawText (WStringCR text, DPoint3dR origin, double angle, bool useUORs) const
    {
    RotMatrix   rMatrix;

    rMatrix.InitFromAxisAndRotationAngle (2, angle);
    DrawText (text, origin, &rMatrix, useUORs);
    }

void TextDrawer::DrawText (WStringCR text, DPoint3dR origin, RotMatrixCP rMatrix, bool useUORs) const
    {
    if (!m_hasTextStringProperties)
        {
        m_hasTextStringProperties = true;
        m_textStringProperties.FromElementData (m_textParamWide, useUORs ? m_fontSizeUOR : m_fontSize);
        m_textStringProperties.SetIs3d (true);
        }

    if (!m_drawTextAsLine)
        {
        TextStringPtr textString = TextString::Create (text.c_str (), &origin, rMatrix, m_textStringProperties);
        textString->SetOriginFromUserOrigin (origin);
        m_context.DrawTextString (*textString);
        }
    else
        {
        DPoint3d pts[2];

        if (m_tempTextString.IsNull ())
            m_tempTextString = TextString::Create (text.c_str (), &origin, rMatrix, m_textStringProperties);

        m_tempTextString->GetGlyphSymbology (*m_context.GetCurrentDisplayParams ());
        m_context.GetCurrentDisplayParams ()->SetLineStyle (2);
        m_context.CookDisplayParams ();

        pts[0] = origin;
        double len = text.length () * m_textStringProperties.GetFontSize().x;
        pts[1] = DPoint3d::FromProduct (pts[0], rMatrix ? *rMatrix : RotMatrix::FromIdentity (), len, 0, 0);

        m_context.GetIDrawGeom().DrawLineString3d (2, pts, nullptr);
        }
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
