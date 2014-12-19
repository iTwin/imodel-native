//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRACopyFromOptions.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRACopyFromOptions
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRACopyFromOptions.h>


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRACopyFromOptions::HRACopyFromOptions(const HRACopyFromOptions& pi_rOptions)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    *this = pi_rOptions;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromOptions::HRACopyFromOptions()
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = 0;
    m_GridShapeMode = false;

    m_AlphaBlend = false;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplyMosaicSupersampling = false;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromOptions::HRACopyFromOptions(bool pi_AlphaBlend)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = 0;
    m_GridShapeMode = false;

    m_AlphaBlend = pi_AlphaBlend;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplyMosaicSupersampling = false;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }


//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRACopyFromOptions::HRACopyFromOptions(const HVEShape* pi_pDestShape,
                                       bool pi_AlphaBlend)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_pDestShape = pi_pDestShape;
    m_GridShapeMode = false;

    m_AlphaBlend = pi_AlphaBlend;

    m_MaxResolutionStretchingFactor = 0;

    m_ApplyMosaicSupersampling = false;

    m_ApplySourceClipping = true;
    m_OverviewMode        = false;
    m_pDestReplacingPixelType = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRACopyFromOptions::~HRACopyFromOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRACopyFromOptions& HRACopyFromOptions::operator=(const HRACopyFromOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pDestShape = pi_rObj.m_pDestShape;
        m_GridShapeMode = pi_rObj.m_GridShapeMode;

        m_AlphaBlend = pi_rObj.m_AlphaBlend;

        m_Resampling = pi_rObj.m_Resampling;

        m_MaxResolutionStretchingFactor = pi_rObj.m_MaxResolutionStretchingFactor;

        m_ApplyMosaicSupersampling = pi_rObj.m_ApplyMosaicSupersampling;

        m_ApplySourceClipping = pi_rObj.m_ApplySourceClipping;
        m_OverviewMode        = pi_rObj.m_OverviewMode;

        m_pPreDrawOptions      = pi_rObj.m_pPreDrawOptions;

        m_pDestReplacingPixelType = pi_rObj.m_pDestReplacingPixelType;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// SetDestShape
//-----------------------------------------------------------------------------
void HRACopyFromOptions::SetDestShape(const HVEShape* pi_pShape)
    {
    m_pDestShape = pi_pShape;
    }

