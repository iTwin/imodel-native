/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GradientSettings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Holds all the values to be persisted for gradient fill.
// @bsiclass                                                     RayBentley      10/02
struct          GradientSettings : public GradientSymb
{
private:


static int      s_currentVersion;

public:

virtual              ~GradientSettings() {}

DGNPLATFORM_EXPORT       GradientSettings ();

                void Init () {;}

void        GetKeyRGBFactor    (RgbFactor *pFactor, double *pValue, int index) const
    {
    // mdlColor_rgbColorDefToFactor (pFactor, const_cast <RgbColorDef *> (&m_colors[index])), *pValue = m_values[index];
    pFactor->red   = m_colors[index].GetRed()   / 255.0;
    pFactor->green = m_colors[index].GetGreen() / 255.0;
    pFactor->blue  = m_colors[index].GetBlue()  / 255.0;

    *pValue = m_values[index];
    }

StatusInt   SetKeys (const ColorDef* pColors, const double* pValues, uint16_t nKeys)
    {
    if (nKeys > 0 && nKeys <= MAX_GRADIENT_KEYS)
        {
        m_nKeys = nKeys;

        for (int i=0; i<nKeys; i++)
            {
            m_values[i] = pValues[i];
            m_colors[i] = pColors[i];
            }

        return SUCCESS;
        }

    return ERROR;
    }

GradientMode*   GetModeP ()  { return &m_mode; }
uint16_t*         GetFlagsP () { return &m_flags; }
double*         GetAngleP () { return &m_angle; }
double*         GetTintP ()  { return &m_tint; }
double*         GetShiftP () { return &m_shift; }

}; // GradientSettings

END_BENTLEY_DGN_NAMESPACE
