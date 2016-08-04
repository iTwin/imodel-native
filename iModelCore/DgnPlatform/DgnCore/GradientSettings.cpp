/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GradientSettings.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void GradientSymb::CopyFrom(GradientSymb const& other)
    {
    m_mode  = other.m_mode;
    m_flags = other.m_flags;
    m_nKeys = other.m_nKeys;
    m_angle = other.m_angle;
    m_tint  = other.m_tint;
    m_shift = other.m_shift;

    memcpy(m_colors, other.m_colors, m_nKeys * sizeof (m_colors[0]));
    memcpy(m_values, other.m_values, m_nKeys * sizeof (m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void GradientSymb::SetKeys(uint16_t nKeys, ColorDef const* pColors, double const* pValues)
    {
    m_nKeys = nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : nKeys;

    memcpy(m_colors, pColors, m_nKeys * sizeof (m_colors[0]));
    memcpy(m_values, pValues, m_nKeys * sizeof (m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GradientSymb::operator==(GradientSymbCR rhs) const
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

