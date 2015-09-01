//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRADrawOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRADrawOptions
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRATransaction.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions()
    :m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_ApplyAlphaBlend       =   false;
    //m_Attributes
    m_OverviewMode          =   false;
    m_pShape                =   NULL;
    m_ApplyGridShape        =   false;
    m_pReplacingCoordSys    =   NULL;
    m_pReplacingPixelType   =   NULL;
    m_pTransaction          =   NULL;
    m_DataDimensionFix      =   false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HRADrawOptions& pi_rOptions)
    :m_Resampling(pi_rOptions.m_Resampling)
    {
    m_ApplyAlphaBlend       =   pi_rOptions.m_ApplyAlphaBlend;
    m_Attributes            =   pi_rOptions.m_Attributes;
    m_OverviewMode          =   pi_rOptions.m_OverviewMode;
    m_pShape                =   pi_rOptions.m_pShape;
    m_ApplyGridShape        =   pi_rOptions.m_ApplyGridShape;
    m_pReplacingCoordSys    =   pi_rOptions.m_pReplacingCoordSys;
    m_pReplacingPixelType   =   pi_rOptions.m_pReplacingPixelType;
    m_pTransaction          =   pi_rOptions.m_pTransaction;
    m_DataDimensionFix      =   pi_rOptions.m_DataDimensionFix;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor on CopyFrom options
//-----------------------------------------------------------------------------
HRADrawOptions::HRADrawOptions(const HRACopyFromLegacyOptions& pi_rCFOptions)
    :m_Resampling(pi_rCFOptions.GetResamplingMode())
    {
    m_ApplyAlphaBlend       =   pi_rCFOptions.ApplyAlphaBlend();
    //m_Attributes            =   NULL;
    m_OverviewMode          =   pi_rCFOptions.GetOverviewMode();
    m_ApplyGridShape        =   pi_rCFOptions.GetGridShapeMode();
    m_pReplacingCoordSys    =   NULL;
    m_pReplacingPixelType   =   NULL;
    m_pTransaction          =   NULL;
    m_DataDimensionFix      =   false; //TR 300554 : Temporary fix activation.

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
HRADrawOptions& HRADrawOptions::operator=(const HRADrawOptions& pi_rOptions)
    {
    if(this != &pi_rOptions)
        {
        m_Resampling            =   pi_rOptions.m_Resampling;
        m_ApplyAlphaBlend       =   pi_rOptions.m_ApplyAlphaBlend;
        m_Attributes            =   pi_rOptions.m_Attributes;
        m_OverviewMode          =   pi_rOptions.m_OverviewMode;
        m_pShape                =   pi_rOptions.m_pShape;
        m_ApplyGridShape        =   pi_rOptions.m_ApplyGridShape;
        m_pReplacingCoordSys    =   pi_rOptions.m_pReplacingCoordSys;
        m_pReplacingPixelType   =   pi_rOptions.m_pReplacingPixelType;
        m_pTransaction          =   pi_rOptions.m_pTransaction;
        m_DataDimensionFix      =   pi_rOptions.m_DataDimensionFix;
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


//-----------------------------------------------------------------------------
// public
// SetTransaction
//-----------------------------------------------------------------------------
void HRADrawOptions::SetTransaction(const HFCPtr<HRATransaction>& pi_rpTransaction)
    {
    m_pTransaction = pi_rpTransaction;
    }

//-----------------------------------------------------------------------------
// public
// GetTransaction
//-----------------------------------------------------------------------------
HFCPtr<HRATransaction> HRADrawOptions::GetTransaction() const
    {
    return m_pTransaction;
    }

//End Temporary Functions