/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GradientSettings.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include <DgnPlatform/GradientSettings.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
int GradientSettings::s_currentVersion = 4;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
GradientSettings::GradientSettings ()
    {
    //SerInitFields();
    }

#ifdef WIP_DGNV8_SETTINGS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GradientSettings::FromGradientSettings (GradientSettingsR other)
    {
    Init();
    CopyFrom (other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void    GradientSettings::SerInitFields ()
    {
    m_angle     = 0.0;
    m_tint      = 1.0;
    m_shift     = 0.0;
    m_active    = false;
    m_mode      = GradientMode::Linear;
    m_flags     = 0;
    m_nKeys     = 2;
    m_values[0] = 0.0;
    m_values[1] = 1.0;
    m_colors[0] = ColorDef::Black();
    m_colors[1] = ColorDef::White();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GradientSettings::SerWriteFields (DataExternalizer& writer)
    {
    writer.put ((int8_t) m_active);
    writer.put (m_angle);
    writer.put (m_tint);
    writer.put (m_shift);
    writer.put ((int16_t) m_mode);
    writer.put (m_flags);
    writer.put (m_nKeys);
    for (int i=0; i<m_nKeys; i++)
        {
        writer.put (m_colors[i].GetRed());
        writer.put (m_colors[i].GetGreen());
        writer.put (m_colors[i].GetBlue());
        }

    for (int i=0; i<m_nKeys; i++)
        writer.put (m_values[i]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley    03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GradientSettings::SerReadFields (DataInternalizer& reader)
    {
    if (SerGetHighestVersionWritten() >= 4)     // ignore old gradients - they're hornswaggled.
        {
        int8_t          activeByte;
        reader.get (&activeByte);
        m_active = (0 != activeByte);

        reader.get (&m_angle);
        reader.get (&m_tint);
        reader.get (&m_shift);
        reader.get ((int16_t*) &m_mode);
        reader.get (&m_flags);
        reader.get (&m_nKeys);

        if (m_nKeys > MAX_GRADIENT_KEYS)
            m_nKeys = MAX_GRADIENT_KEYS;

        for (int i=0; i<m_nKeys; i++)
            {
            Byte red,green,blue;
            reader.get (&red);
            reader.get (&green);
            reader.get (&blue);
            m_colors[i] = ColorDef(red,green,blue);
            }

        for (int i=0; i<m_nKeys; i++)
            reader.get (&m_values[i]);
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            GradientSymb::CopyFrom (GradientSymb const& other)
    {
    m_mode  = other.m_mode;
    m_flags = other.m_flags;
    m_nKeys = other.m_nKeys;
    m_angle = other.m_angle;
    m_tint  = other.m_tint;
    m_shift = other.m_shift;

    memcpy (m_colors, other.m_colors, m_nKeys * sizeof (m_colors[0]));
    memcpy (m_values, other.m_values, m_nKeys * sizeof (m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GradientSymb::SetKeys (uint16_t nKeys, ColorDef const* pColors, double const* pValues)
    {
    m_nKeys = nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : nKeys;

    memcpy (m_colors, pColors, m_nKeys * sizeof (m_colors[0]));
    memcpy (m_values, pValues, m_nKeys * sizeof (m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/07
+---------------+---------------+---------------+---------------+---------------+------*/
GradientSymbPtr GradientSymb::Create () {return new GradientSymb ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GradientSymb::operator==(GradientSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_mode != m_mode)
        return false;

    if (rhs.m_flags != m_flags)
        return false;

    if (rhs.m_nKeys != m_nKeys)
        return false;

    if (rhs.m_angle != m_angle)
        return false;

    if (rhs.m_tint  != m_tint)
        return false;

    if (rhs.m_shift != m_shift)
        return false;

    int nKeys = m_nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : m_nKeys;

    for (int i=0; i<nKeys; i++)
        {
        if (rhs.m_values[i] != m_values[i])
            return false;

        if (rhs.m_colors[i] != m_colors[i])
            return false;
        }

    return true;
    }

