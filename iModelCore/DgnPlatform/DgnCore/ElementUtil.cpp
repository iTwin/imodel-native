/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Tools/MdlCnv.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendRotMatrix (Byte*& buffer, RotMatrixCR value, bool is3d)
    {
    double quat[4];

    if (is3d)
        value.GetQuaternion(quat, true);
    else
        value.GetRowValuesXY(quat);

    memcpy (buffer, quat, sizeof (quat));
    buffer += sizeof (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractRotMatrix (RotMatrixR value, Byte const *& buffer, bool is3d)
    {
    double quat[4];

    memcpy (quat, buffer, sizeof (quat));
    buffer += sizeof (quat);

    if (is3d)
        value.InitTransposedFromQuaternionWXYZ (quat);
    else
        value.InitFromRowValuesXY (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDPoint3d (Byte*& buffer, DPoint3dCR value)
    {
    double rBuf[3];

    rBuf[0] = value.x;
    rBuf[1] = value.y;
    rBuf[2] = value.z;

    memcpy (buffer, rBuf, sizeof (rBuf));
    buffer += sizeof (rBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractDPoint3d (DPoint3dR value, Byte const *& buffer)
    {
    double rBuf[3];

    memcpy (rBuf, buffer, sizeof (rBuf));
    buffer += sizeof (rBuf);

    value.x = rBuf[0];
    value.y = rBuf[1];
    value.z = rBuf[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDouble (Byte*& buffer, double const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractDouble (double& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendLong (Byte*& buffer, long const & value)       { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractLong (long& value, Byte const *& buffer)      { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendShort (Byte*& buffer, short const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractShort (short& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt64 (Byte*& buffer, int64_t const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt64 (int64_t& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt32 (Byte*& buffer, uint32_t const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt32 (uint32_t& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt (Byte*& buffer, int const & value)         { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt (int& value, Byte const *& buffer)        { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt16 (Byte*& buffer, uint16_t const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt16 (uint16_t& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt32 (Byte*& buffer, int32_t const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt32 (int32_t& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUShort (Byte*& buffer, unsigned short const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUShort (unsigned short& value, Byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendULong (Byte*& buffer, unsigned long const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractULong (unsigned long& value, Byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Init ()
    {
    rMatrix.InitIdentity ();
    offset.Zero ();
    space1 = 0.0;
    angle1 = 0.0;
    space2 = 0.0;
    angle2 = 0.0;
    scale = 1.0;
    tolerance = 0.0;
    annotationscale = 1.0;
    memset (cellName, 0, sizeof (cellName));
    cellId = 0;
    modifiers = PatternParamsModifierFlags::None;
    minLine = -1;
    maxLine = -1;
    color = ColorDef::Black();
    weight = 0;
    style = 0;
    holeStyle = static_cast<int16_t>(PatternParamsHoleStyleType::Normal);
    dwgHatchDef.pixelSize = 0.0;
    dwgHatchDef.islandStyle = 0;
    origin.Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParams::PatternParams () {Init ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::Create () {return new PatternParams ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::CreateFromExisting (PatternParamsCR params) 
    {
    PatternParamsP newParams = new PatternParams ();
    newParams->Copy (params);
    return  newParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Copy (PatternParamsCR params) 
    {
    rMatrix.copy (&params.rMatrix);
    offset = params.offset;
    space1 = params.space1;
    angle1 = params.angle1;
    space2 = params.space2;
    angle2 = params.angle2;
    scale = params.scale;
    tolerance = params.tolerance;
    annotationscale = params.annotationscale;
    memcpy (cellName, params.cellName, sizeof (cellName));
    cellId = params.cellId;
    modifiers = params.modifiers;
    minLine = params.minLine;
    maxLine = params.maxLine;
    color = params.color;
    weight = params.weight;
    style = params.style;
    holeStyle = params.holeStyle;
    dwgHatchDef.pixelSize = params.dwgHatchDef.pixelSize;
    dwgHatchDef.islandStyle = params.dwgHatchDef.islandStyle;
    dwgHatchDef.hatchLines = params.dwgHatchDef.hatchLines;
    origin = params.origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PatternParams::IsEqual (PatternParamsCR params, PatternParamsCompareFlags compareFlags) 
    {
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_RMatrix)
        {
        if ((params.modifiers & PatternParamsModifierFlags::RotMatrix) != (modifiers & PatternParamsModifierFlags::RotMatrix))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::RotMatrix))
            {
            if (!rMatrix.isEqual (&params.rMatrix))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Offset)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Offset) != (modifiers & PatternParamsModifierFlags::Offset))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Offset))
            {
            if (!offset.IsEqual(params.offset))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Default)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Space1) != (modifiers & PatternParamsModifierFlags::Space1))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space1))
            {
            if (space1 != params.space1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Space2) != (modifiers & PatternParamsModifierFlags::Space2))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space2))
            {
            if (space2 != params.space2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle1) != (modifiers & PatternParamsModifierFlags::Angle1))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle1))
            {
            if (angle1 != params.angle1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle2) != (modifiers & PatternParamsModifierFlags::Angle2))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle2))
            {
            if (angle2 != params.angle2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Scale) != (modifiers & PatternParamsModifierFlags::Scale))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Scale))
            {
            if (scale != params.scale)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Cell) != (modifiers & PatternParamsModifierFlags::Cell))
            return false;
        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Cell))
            {
            if (0 != wcscmp (cellName, params.cellName))
                return false;

            if (cellId != params.cellId)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Symbology)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Color) != (modifiers & PatternParamsModifierFlags::Color))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Color))
            {
            if (color != params.color)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Weight) != (modifiers & PatternParamsModifierFlags::Weight))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Weight))
            {
            if (weight != params.weight)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Style) != (modifiers & PatternParamsModifierFlags::Style))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Style))
            {
            if (style != params.style)
                return false;
            }
        }
  
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Mline)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Multiline) != (modifiers & PatternParamsModifierFlags::Multiline))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Multiline))
            {
            if (minLine != params.minLine)
                return false;

            if (maxLine != params.maxLine)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Tolerance)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Tolerance) != (modifiers & PatternParamsModifierFlags::Tolerance))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Tolerance))
            {
            if (tolerance != params.tolerance)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_AnnotationScale)
        {
        if ((params.modifiers & PatternParamsModifierFlags::AnnotationScale) != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            {
            if (annotationscale != params.annotationscale)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_HoleStyle)
        {
        if ((params.modifiers & PatternParamsModifierFlags::HoleStyle) != (modifiers & PatternParamsModifierFlags::HoleStyle))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::HoleStyle))
            {
            if (holeStyle != params.holeStyle)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_DwgHatch)
        {
        if ((params.modifiers & PatternParamsModifierFlags::DwgHatchDef) != (modifiers & PatternParamsModifierFlags::DwgHatchDef))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::DwgHatchDef))
            {
            if (dwgHatchDef.pixelSize != params.dwgHatchDef.pixelSize)
                return false;

            if (dwgHatchDef.islandStyle != params.dwgHatchDef.islandStyle)
                return false;

            if (dwgHatchDef.hatchLines.size() != params.dwgHatchDef.hatchLines.size())
                return false;

            size_t  nHatchLines = dwgHatchDef.hatchLines.size();

            for (size_t i=0; i<nHatchLines; ++i)
                {
                DwgHatchDefLine const* h1 = &dwgHatchDef.hatchLines.at(i);
                DwgHatchDefLine const* h2 = &params.dwgHatchDef.hatchLines.at(i);

                if (0 != memcmp (h1, h2, sizeof (*h1)))
                    return false;
                }
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Origin)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Origin) != (modifiers & PatternParamsModifierFlags::Origin))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Origin))
            {
            if (!origin.IsEqual(params.origin))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesGetter::ElementPropertiesGetter (ElementHandleCR eh)
    {
#if defined (V10_WIP_ELEMENTHANDLER)
    // NEEDSWORK: Is there still a need to get "element" symbology. What tools/operation might need this?
    ElementHandlerR handler = eh.GetHandler ();

    handler.GetElemDisplayParams (eh, m_displayParams);
#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesGetterPtr ElementPropertiesGetter::Create (ElementHandleCR eh)
    {
    return new ElementPropertiesGetter (eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ElementPropertiesGetter::GetLineStyle (LineStyleParamsP lsParams) const
    {
#if defined (WIP_LINESTYLE)
    if (lsParams)
        {
        LineStyleParamsCP  params = m_displayParams.GetLineStyleParams ();

        if (params)
            *lsParams = *params;
        else
            lsParams->Init ();
        }
    
    return m_displayParams.GetLineStyle ();
#else
    return 0;
#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ElementPropertiesGetter::GetColor () const {return m_displayParams.GetLineColor();}
uint32_t ElementPropertiesGetter::GetWeight () const {return m_displayParams.GetWeight ();}
DgnCategoryId ElementPropertiesGetter::GetCategory () const {return m_displayParams.GetCategoryId ();}
int32_t ElementPropertiesGetter::GetDisplayPriority () const {return m_displayParams.GetDisplayPriority ();}
double ElementPropertiesGetter::GetTransparency () const {return m_displayParams.GetTransparency ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesSetter::ElementPropertiesSetter ()
    {
    m_propMask          = ELEMENT_PROPERTY_None;
    m_changeAll         = false;

    m_setElemColor      = false;
    m_setFillColor      = false;

    m_color             = ColorDef::Black();
    m_fillColor         = ColorDef::Black();
    m_weight            = 0;
    m_priority          = 0;
    m_transparency      = 0.0;

    m_style             = 0;
    m_lsParams          = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetColor (ColorDef color)
    {
    m_propMask      = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Color);
    m_color         = color;
    m_setElemColor  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetFillColor (ColorDef fillColor)
    {
    m_propMask      = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Color);
    m_fillColor     = fillColor;
    m_setFillColor  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetLinestyle (int32_t style, LineStyleParamsCP lsParams)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Linestyle);
    m_style    = style;
    m_lsParams = lsParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetWeight (uint32_t weight)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Weight);
    m_weight = weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetCategory (DgnCategoryId category)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Category);
    m_category = category;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetDisplayPriority (int32_t priority)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_DisplayPriority);
    m_priority = priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetTransparency (double transparency)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Transparency);
    m_transparency = transparency;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void ElementPropertiesSetter::SetFont (DgnFontCR font)
    {
    m_propMask  = (ElementProperties)(m_propMask | ELEMENT_PROPERTY_Font);
    m_font      = &font;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::SetChangeEntireElement (bool changeAll)
    {
    m_changeAll = changeAll;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementPropertiesSetter::IsValidBaseID (EachPropertyBaseArg& arg)
    {
    if (!m_changeAll && 0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return false;

    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return false;

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachColorCallback (EachColorArg& arg)
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_setElemColor && (m_changeAll || 0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID)))
        {
        arg.SetStoredValue (m_color.GetValue());

        if (!m_setFillColor)
            arg.SetPropertyCallerFlags ((PropsCallerFlags) (PROPSCALLER_FLAGS_PreserveOpaqueFill | PROPSCALLER_FLAGS_PreserveMatchingDecorationColor));
        else
            arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_PreserveMatchingDecorationColor);
        }

    if (m_setFillColor && (m_changeAll || 0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBackgroundID)))
        arg.SetStoredValue (m_fillColor.GetValue());
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachLineStyleCallback (EachLineStyleArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_style);

    if (m_lsParams)
        arg.SetParams (m_lsParams);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachWeightCallback (EachWeightArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_weight);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachCategoryCallback (EachCategoryArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_category);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachDisplayPriorityCallback (EachDisplayPriorityArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_priority);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachTransparencyCallback (EachTransparencyArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_transparency);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void ElementPropertiesSetter::_EachFontCallback (EachFontArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;
    
    DgnFontId fontNumber = arg.GetPropertyContext().GetDestinationDgnModel()->GetDgnDb().Fonts().AcquireId(*m_font);
    if (!fontNumber.IsValid())
        { BeAssert (false); return; }
    
    arg.SetStoredValue (fontNumber);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ElementPropertiesSetter::Apply (EditElementHandleR eeh)
    {
    if (ELEMENT_PROPERTY_None == m_propMask)
        return false; // Nothing to do, no properties set...
    return PropertyContext::EditElementProperties (eeh, this);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesSetterPtr ElementPropertiesSetter::Create ()
    {
    return new ElementPropertiesSetter ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DML             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void     DataConvert::Points3dTo2d (DPoint2dP outP, DPoint3dCP inP, int numPts)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/86
+---------------+---------------+---------------+---------------+---------------+------*/
void     DataConvert::Points2dTo3d (DPoint3dP outP, DPoint2dCP inP, int numPts, double zElev)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        outP->z = zElev;
        }
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          DataConvert::ReverseUInt64                                    |
|                                                                       |
| author        RayBentley                             04/02k           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     DataConvert::ReverseUInt64
(
uint64_t&        output,
uint64_t       input
)
    {
    uint64_t    tmp = input;
    Byte *pTmpBytes = (Byte *) &tmp, *pOutputBytes = (Byte *) &output;

    pOutputBytes[0] = pTmpBytes[7];
    pOutputBytes[1] = pTmpBytes[6];
    pOutputBytes[2] = pTmpBytes[5];
    pOutputBytes[3] = pTmpBytes[4];
    pOutputBytes[4] = pTmpBytes[3];
    pOutputBytes[5] = pTmpBytes[2];
    pOutputBytes[6] = pTmpBytes[1];
    pOutputBytes[7] = pTmpBytes[0];
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveVectorUtil::PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,
DPoint3dCP          pointL,
DPoint3dCP          pointD,
DgnViewportP        vp
)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return partialDeleteElement (outEeh1, outEeh2, eh, pointF, pointL, pointD, NULL, false, vp);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveVectorUtil::PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,
DPoint3dCP          pointL,
DVec3dP             dirVec,             // <=> Output when computeDirection
bool                computeDirection,
DgnViewportP        vp
)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return partialDeleteElement (outEeh1, outEeh2, eh, pointF, pointL, NULL, dirVec, computeDirection, vp);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
bool BentleyApi::in_span (double theta, double start, double sweep)
    {
    theta -= start;

    if (fabs (theta) > 62.8) return (0);

    while (theta < 0.0) theta += msGeomConst_2pi;
    while (theta > msGeomConst_2pi) theta -= msGeomConst_2pi;
    if (sweep > 0.0)
        {
        if (sweep >= theta)
            return (true);
        return (false);
        }
    else
        {
        if (theta >= (msGeomConst_2pi + sweep))
            return (true);
        return (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementUtil::InitScanRangeForUnion (DRange3dR range, bool is3d)
    {
    range.Init();
    if (!is3d)
        {
        range.low.z = range.high.z = 0.0;
        }
    }
#if defined (NEEDS_WORK_CONVERTER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementUtil::GetIntersectionPointByIndex
(
DPoint3dR       isPnt,
ElementHandleCR eh1,
ElementHandleCR eh2,
TransformCP     pathTrans1,
TransformCP     pathTrans2,
int             index
)
    {
    if (index < 0)
        {
        BeAssert (false);

        return ERROR;
        }

    bvector<DPoint3d> isPnts1, isPnts2;

    if (SUCCESS != ElementUtil::GetIntersections (&isPnts1, &isPnts2, eh1, eh2, pathTrans1, pathTrans2, true))
        return ERROR;

    int     nIntersect = (int) isPnts1.size ();

    if (index > nIntersect-1)
        return ERROR;

    // Make sure this is a real intersection (not apparent one)
    if (!LegacyMath::RpntEqual (&isPnts1[index], &isPnts2[index]))
        return ERROR;

    isPnt = isPnts1[index];

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ElementUtil::GetIgnoreScaleDisplayTransforms
(
TransformP      newTopTransP,
TransformP      newScTransP,
ViewContextR    context
)
    {
    Transform   fwdTopLocalToFrustum, invTopLocalToFrustum, fwdRefLocalToFrustum, invRefLocalToFrustum;

    context.GetCurrLocalToFrustumTrans (fwdTopLocalToFrustum);
    context.GetLocalToFrustumTrans (fwdRefLocalToFrustum, context.GetRefTransClipDepth ());
    bsiTransform_invertTransform (&invTopLocalToFrustum, &fwdTopLocalToFrustum);
    bsiTransform_invertTransform (&invRefLocalToFrustum, &fwdRefLocalToFrustum);

    bsiTransform_multiplyTransformTransform (newTopTransP, &fwdRefLocalToFrustum, &invTopLocalToFrustum);
    bsiTransform_multiplyTransformTransform (newScTransP, &fwdTopLocalToFrustum, &invRefLocalToFrustum);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum TEXTSTYLE_PROP
    {
    TEXTSTYLE_PROP_FontNo               = 0,
    TEXTSTYLE_PROP_ShxBigFont           = 1,
    TEXTSTYLE_PROP_Width                = 2,
    TEXTSTYLE_PROP_Height               = 3,
    TEXTSTYLE_PROP_Slant                = 4,
    TEXTSTYLE_PROP_LineSpacing          = 5,
    TEXTSTYLE_PROP_InterCharSpacing     = 6,
    TEXTSTYLE_PROP_UnderlineOffset      = 7,
    TEXTSTYLE_PROP_OverlineOffset       = 8,
    TEXTSTYLE_PROP_WidthFactor          = 9,
    TEXTSTYLE_PROP_LineOffset           = 10,
    TEXTSTYLE_PROP_Just                 = 11,
    TEXTSTYLE_PROP_NodeJust             = 12,
    TEXTSTYLE_PROP_LineLength           = 13,
    TEXTSTYLE_PROP_TextDirection        = 14,
    TEXTSTYLE_PROP_BackgroundStyle      = 15,
    TEXTSTYLE_PROP_BackgroundWeight     = 16,
    TEXTSTYLE_PROP_BackgroundColor      = 17,
    TEXTSTYLE_PROP_BackgroundFillColor  = 18,
    TEXTSTYLE_PROP_BackgroundBorder     = 19,
    TEXTSTYLE_PROP_UnderlineStyle       = 20,
    TEXTSTYLE_PROP_UnderlineWeight      = 21,
    TEXTSTYLE_PROP_UnderlineColor       = 22,
    TEXTSTYLE_PROP_OverlineStyle        = 23,
    TEXTSTYLE_PROP_OverlineWeight       = 24,
    TEXTSTYLE_PROP_OverlineColor        = 25,
    TEXTSTYLE_PROP_ParentId             = 26,
    TEXTSTYLE_PROP_Flags                = 27,
    TEXTSTYLE_PROP_OverrideFlags        = 28,
    TEXTSTYLE_PROP_Color                = 29
    
    }; // TEXTSTYLE_PROP


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void setAreaFillFromDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    IAreaFillPropertiesEdit* areaObj = dynamic_cast <IAreaFillPropertiesEdit*> (&eeh.GetElementHandler());

    if (areaObj)
        {
        areaObj->RemoveAreaFill (eeh);

        if (NULL != params.GetGradient ())
            {
            areaObj->AddGradientFill (eeh, *params.GetGradient ());
            }
        else if (FillDisplay::Never != params.GetFillDisplay ())
            {
            bool    alwaysFill = (FillDisplay::Always == params.GetFillDisplay ());
            ColorDef fillColor = params.GetFillColor();
            areaObj->AddSolidFill (eeh, &fillColor, &alwaysFill);
            }

        return;
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void setMaterialFromDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
#ifdef WIP_VANCOUVER_MERGE // material
    IMaterialPropertiesExtension* materialExt = IMaterialPropertiesExtension::Cast (eeh.GetHandler ());

    if (materialExt)
        {
        materialExt->DeleteMaterialAttachment (eeh);

        if (params.IsAttachedMaterial () && params.GetMaterial ())
            {
            MaterialCP  material = params.GetMaterial ();
            MaterialId  materialId (material->GetElementId (), material->GetName ().c_str ());

            materialExt->AddMaterialAttachment (eeh, materialId);
            }

        return;
        }

    // Apply to public children...
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        setMaterialFromDisplayParams (childElm, params);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyElemDisplayParamsRestricted (EditElementHandleR eeh, ElemDisplayParamsCR params, TemplateIgnores options)
    {
    ElementPropertiesSetter remapper;

    if (0 == (options & TEMPLATE_IGNORE_Category))
        remapper.SetCategory (params.GetCategoryId ());

    if (0 == (options & TEMPLATE_IGNORE_Color))
        remapper.SetColor (params.GetLineColor());

#if defined (WIP_LINESTYLE)
    if (0 == (options & TEMPLATE_IGNORE_Style))
        {
        int32_t             style = params.GetLineStyle ();
        LineStyleParamsCP   styleParams = params.GetLineStyleParams ();

        remapper.SetLinestyle (style, IS_LINECODE (style) || 0 != (options & TEMPLATE_IGNORE_StyleModifiers) ? NULL : styleParams);
        }
#endif

    if (0 == (options & TEMPLATE_IGNORE_Weight))
        remapper.SetWeight (params.GetWeight ());

    if (0 == (options & TEMPLATE_IGNORE_Transparency))
        remapper.SetTransparency (params.GetTransparency ());

    if (0 == (options & TEMPLATE_IGNORE_Priority))
        remapper.SetDisplayPriority (params.GetDisplayPriority ());

    remapper.Apply (eeh);

    if (0 == (options & TEMPLATE_IGNORE_Fill))
        setAreaFillFromDisplayParams (eeh, params);

    if (0 == (options & TEMPLATE_IGNORE_Material))
        setMaterialFromDisplayParams (eeh, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyElemDisplayParams (EditElementHandleR eeh, ElemDisplayParamsCR params)
    {
    ApplyElemDisplayParamsRestricted (eeh, params, TEMPLATE_IGNORE_None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyTemplateRestricted (EditElementHandleR eeh, ElementHandleCR templateEh, TemplateIgnores options)
    {
#if defined (V10_WIP_ELEMENTHANDLER)
    ElementHandlerR handler = templateEh.GetHandler ();

    ElemDisplayParams params;

    handler.GetElemDisplayParams (templateEh, params, true); // Get material for apply...

    ApplyElemDisplayParamsRestricted (eeh, params, options);

    if (0 == (options & TEMPLATE_IGNORE_Pattern))
        applyPatternFromTemplate (eeh, templateEh); // Pattern isn't part of ElemDisplayParams...
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::ApplyTemplate (EditElementHandleR eeh, ElementHandleCR templateEh)
    {
    ApplyTemplateRestricted (eeh, templateEh, TEMPLATE_IGNORE_None);
    }
