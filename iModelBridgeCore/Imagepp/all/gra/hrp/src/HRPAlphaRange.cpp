//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPAlphaRange.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for HRPAlphaRange
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPAlphaRange.h>
#include <Imagepp/all/h/HGFRGBCube.h>

//-----------------------------------------------------------------------------
// public
// default constructor.
//-----------------------------------------------------------------------------
HRPAlphaRange::HRPAlphaRange()
    {
    m_pSet = new HGFRGBCube(0,255,0,255,0,255);
    m_AlphaValue = 255;
    }

//-----------------------------------------------------------------------------
// public
// constructor.
//-----------------------------------------------------------------------------
HRPAlphaRange::HRPAlphaRange(   Byte pi_RedMin,
                                Byte pi_RedMax,
                                Byte pi_GreenMin,
                                Byte pi_GreenMax,
                                Byte pi_BlueMin,
                                Byte pi_BlueMax,
                                Byte pi_AlphaValue)
    {
    m_pSet = new HGFRGBCube(pi_RedMin, pi_RedMax,
                            pi_GreenMin, pi_GreenMax,
                            pi_BlueMin, pi_BlueMax);
    m_AlphaValue = pi_AlphaValue;
    }

//-----------------------------------------------------------------------------
// public
// constructor.
//-----------------------------------------------------------------------------
HRPAlphaRange::HRPAlphaRange(HFCPtr<HGFColorSet>& pi_pSet, Byte pi_AlphaValue)
    {
    m_pSet = pi_pSet;
    m_AlphaValue = pi_AlphaValue;
    }

//-----------------------------------------------------------------------------
// public
// copy constructor.
//-----------------------------------------------------------------------------
HRPAlphaRange::HRPAlphaRange(const HRPAlphaRange& pi_rObj)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// public
// operator=.
//-----------------------------------------------------------------------------
HRPAlphaRange& HRPAlphaRange::operator=(const HRPAlphaRange& pi_rObj)
    {
    if(&pi_rObj != this)
        {
        DeepCopy(pi_rObj);
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// clone
//-----------------------------------------------------------------------------
HRPAlphaRange* HRPAlphaRange::Clone() const
    {
    return new HRPAlphaRange(*this);
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy.
//-----------------------------------------------------------------------------
void HRPAlphaRange::DeepCopy(const HRPAlphaRange& pi_rObj)
    {
    m_pSet = pi_rObj.m_pSet;
    m_AlphaValue = pi_rObj.m_AlphaValue;
    }


