//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRADrawOptions.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRADrawOptions
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRADrawOptions.h>

class HGFPreDrawOptions;

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions()
    : HGFDrawOptions()
    {
    m_ApplyMosaicSupersampling = false;
    m_ApplyGridShape = false;
    m_DataDimensionFix = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HRADrawOptions& pi_rOptions)
    : HGFDrawOptions(pi_rOptions)
    {
    m_pShape                    = pi_rOptions.m_pShape;
    m_ApplyGridShape            = pi_rOptions.m_ApplyGridShape;
    m_pReplacingCoordSys        = pi_rOptions.m_pReplacingCoordSys;
    m_pReplacingPixelType       = pi_rOptions.m_pReplacingPixelType;
    m_ApplyMosaicSupersampling  = pi_rOptions.m_ApplyMosaicSupersampling;
    m_DataDimensionFix          = pi_rOptions.m_DataDimensionFix;
    m_pTransaction              = pi_rOptions.m_pTransaction;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor on ancestor object
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HGFDrawOptions& pi_rOptions)
    : HGFDrawOptions(pi_rOptions)
    {
    // The default for us

    m_ApplyMosaicSupersampling = false;
    m_ApplyGridShape = false;
    m_DataDimensionFix = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor on polymorphic pointer
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HGFDrawOptions* pi_pOptions)
    : HGFDrawOptions(pi_pOptions)
    {
    if (pi_pOptions != 0 && pi_pOptions->IsCompatibleWith(CLASS_ID))
        {
        m_pShape                    = ((HRADrawOptions*)pi_pOptions)->m_pShape;
        m_ApplyGridShape            = ((HRADrawOptions*)pi_pOptions)->m_ApplyGridShape;
        m_pReplacingCoordSys        = ((HRADrawOptions*)pi_pOptions)->m_pReplacingCoordSys;
        m_pReplacingPixelType       = ((HRADrawOptions*)pi_pOptions)->m_pReplacingPixelType;
        m_ApplyMosaicSupersampling  = ((HRADrawOptions*)pi_pOptions)->m_ApplyMosaicSupersampling;
        m_DataDimensionFix          = ((HRADrawOptions*)pi_pOptions)->m_DataDimensionFix;
        m_pTransaction              = ((HRADrawOptions*)pi_pOptions)->m_pTransaction;
        }
    else
        {
        // Default values
        m_ApplyGridShape = false;
        m_ApplyMosaicSupersampling = false;
        m_DataDimensionFix = false;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor on CopyFrom options
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HRACopyFromOptions& pi_rCFOptions)
    : HGFDrawOptions()
    {
    SetAlphaBlend(pi_rCFOptions.ApplyAlphaBlend());
    SetResamplingMode(pi_rCFOptions.GetResamplingMode());
    SetMosaicSupersampling(pi_rCFOptions.ApplyMosaicSupersampling());
    SetGridShape(pi_rCFOptions.GetGridShapeMode());
    SetOverviewMode(pi_rCFOptions.GetOverviewMode());
    SetPreDrawOptions(pi_rCFOptions.GetPreDrawOptions());

    //TR 300554 : Temporary fix activation.
    SetDataDimensionFix(false);

    // Take a copy, the shape in CF options is held by dumb pointer
    if (pi_rCFOptions.GetDestShape() != 0)
        SetShape(new HVEShape(*pi_rCFOptions.GetDestShape()));
    }


//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRADrawOptions::~HRADrawOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HRADrawOptions& HRADrawOptions::operator=(const HRADrawOptions& pi_rObj)
    {
    if(this != &pi_rObj)
        {
        HGFDrawOptions::operator=(pi_rObj);

        m_pShape                    = pi_rObj.m_pShape;
        m_ApplyGridShape            = pi_rObj.m_ApplyGridShape;
        m_pReplacingCoordSys        = pi_rObj.m_pReplacingCoordSys;
        m_pReplacingPixelType       = pi_rObj.m_pReplacingPixelType;
        m_ApplyMosaicSupersampling  = pi_rObj.m_ApplyMosaicSupersampling;
        m_DataDimensionFix          = pi_rObj.m_DataDimensionFix;        
        m_pTransaction              = pi_rObj.m_pTransaction;
        }

    return(*this);
    }


//-----------------------------------------------------------------------------
// public
// operator= on ancestor object
//-----------------------------------------------------------------------------
HRADrawOptions& HRADrawOptions::operator=(const HGFDrawOptions& pi_rObj)
    {
    HGFDrawOptions::operator=(pi_rObj);

    m_pShape                = 0;
    m_ApplyGridShape        = false;
    m_pReplacingCoordSys    = 0;
    m_pReplacingPixelType   = 0;
    m_ApplyMosaicSupersampling = false;
    m_DataDimensionFix      = false;
    m_pTransaction          = 0;

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// operator= on polymorphic pointer
//-----------------------------------------------------------------------------
HRADrawOptions& HRADrawOptions::operator=(const HGFDrawOptions* pi_pObj)
    {
    HGFDrawOptions::operator=(pi_pObj);

    if ((HGFDrawOptions*) this != pi_pObj)
        {
        if (pi_pObj != 0 &&
            pi_pObj->IsCompatibleWith(CLASS_ID))
            {
            m_pShape                    = ((HRADrawOptions*)pi_pObj)->m_pShape;
            m_ApplyGridShape            = ((HRADrawOptions*)pi_pObj)->m_ApplyGridShape;
            m_pReplacingCoordSys        = ((HRADrawOptions*)pi_pObj)->m_pReplacingCoordSys;
            m_pReplacingPixelType       = ((HRADrawOptions*)pi_pObj)->m_pReplacingPixelType;
            m_ApplyMosaicSupersampling  = ((HRADrawOptions*)pi_pObj)->m_ApplyMosaicSupersampling;
            m_DataDimensionFix          = ((HRADrawOptions*)pi_pObj)->m_DataDimensionFix;
            m_pTransaction              = ((HRADrawOptions*)pi_pObj)->m_pTransaction;
            }
        else
            {
            m_pShape                    = 0;
            m_ApplyGridShape            = false;
            m_pReplacingCoordSys        = 0;
            m_pReplacingPixelType       = 0;
            m_ApplyMosaicSupersampling  = false;
            m_DataDimensionFix          = false;
            m_pTransaction              = 0;
            }
        }

    return(*this);
    }

//Temporary functions for fixing the TR 300554 without breaking the CopyFrom for
//other Imagepp users.
//-----------------------------------------------------------------------------
// public
// GetDataDimensionFix
//-----------------------------------------------------------------------------
bool HRADrawOptions::GetDataDimensionFix() const
    {
    return m_DataDimensionFix;
    }

//-----------------------------------------------------------------------------
// public
// SetDataDimensionFix
//-----------------------------------------------------------------------------
void HRADrawOptions::SetDataDimensionFix(bool pi_fix)
    {
    m_DataDimensionFix = pi_fix;
    }

//End Temporary Functions