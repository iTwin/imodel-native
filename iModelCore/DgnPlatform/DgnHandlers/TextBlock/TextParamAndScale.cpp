/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/TextBlock/TextParamAndScale.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   02/05
//---------------------------------------------------------------------------------------
TextParamAndScale::TextParamAndScale (TextParamWideCP pTextParams, DPoint2dCP pScale)
    {
    SetTextParamWide (pTextParams);

    if (m_textParams.exFlags.annotationScale && 0.0 == m_textParams.annotationScale)
        {
        BeAssert (false);
        GetTextParamWide ()->annotationScale = 1.0;
        }

    m_scale = *pScale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   04/05
//---------------------------------------------------------------------------------------
TextParamAndScaleP TextParamAndScale::Clone ()
    {
    return new TextParamAndScale (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
bool TextParamAndScale::IsEqual (TextParamAndScaleP pTextParamAndScale)
    {
    TextParamWide   textParams;
    DPoint2d        scale;
    
    pTextParamAndScale->GetTextParamWide (&textParams);
    pTextParamAndScale->GetScale (&scale);
    
    return IsEqual (&textParams, &scale);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   12/05
//---------------------------------------------------------------------------------------
void TextParamAndScale::Validate ()
    {
    GetTextParamWide ()->exFlags.crCount = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
bool TextParamAndScale::IsEqual (TextParamWideCP pTextParams, DPoint2dCP pScale)
    {
    if (!pScale->isEqual (&m_scale))
        return false;

    // NEEDS_THOUGHT: For floating point values, memcmp in general may not be a good
    //  idea but then in most cases, even if there is a small difference,
    //  we can treat it as styled differently and that should be ok.
    //  So, a memcmp is ok.  However, in cases were we want more compactness,
    //  I could rewrite this logic.
    //  problem is that we have data along with parameters.  Examples are tab/LF/CR linkages
    //  crCount etc.  I want to ignore those.  So, let me force equalize those parameters
    
    TextParamWide textParams = m_textParams;
    
    textParams.exFlags.crCount                  = pTextParams->exFlags.crCount;
    textParams.exFlags.bitMaskContainsTabCRLF   = pTextParams->exFlags.bitMaskContainsTabCRLF;
    
    if (0 != memcmp (pTextParams, &textParams, sizeof (textParams)))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   01/06
//---------------------------------------------------------------------------------------
void TextParamAndScale::GetScale (DPoint2dP pScale) const
    {
    if (NULL != pScale)
        *pScale = m_scale;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Venkat.Kalyan   08/07
//---------------------------------------------------------------------------------------
TextParamAndScale::TextParamAndScale
(
RunPropertiesCR         runProperties,
ParagraphPropertiesCR   paragraphProperties,
TextBlockPropertiesCR   textBlockProperties,
RunPropertiesCP         textNodeProperties
)
    {
    m_scale = runProperties.GetFontSize ();
    m_textParams.Initialize ();
    runProperties.ToElementData (m_textParams, textBlockProperties.GetDgnModelR ().GetDgnProject ());

    textBlockProperties.ToElementData (m_textParams, NULL);

    if (NULL != textNodeProperties && textNodeProperties->GetFontSize ().y > mgds_fc_epsilon)
        m_scale = textNodeProperties->GetFontSize ();

    paragraphProperties.ToElementData (m_textParams);

    if (m_textParams.exFlags.annotationScale && 0.0 == m_textParams.annotationScale)
        {
        BeAssert (false);
        m_textParams.annotationScale = 1.0;
        }
    }
