//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFDrawOptions.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFDrawOptions
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGFPreDrawOptions.h>
#include <Imagepp/all/h/HGFDrawOptions.h>

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HGFDrawOptions::HGFDrawOptions(const HGFDrawOptions& pi_rOptions)
    : m_Resampling(pi_rOptions.m_Resampling)
    {
    m_ApplyAlphaBlend       = pi_rOptions.m_ApplyAlphaBlend;
    m_AbortRequest          = pi_rOptions.m_AbortRequest;
    m_Attributes            = pi_rOptions.m_Attributes;
    m_InTemporaryRenderMode = pi_rOptions.m_InTemporaryRenderMode;
    m_OverviewMode          = pi_rOptions.m_OverviewMode;
    m_pPreDrawOptions        = pi_rOptions.m_pPreDrawOptions;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor on a pointer (may be null)
//-----------------------------------------------------------------------------
HGFDrawOptions::HGFDrawOptions(const HGFDrawOptions* pi_pOptions)
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    if (pi_pOptions != 0)
        {
        m_ApplyAlphaBlend       = pi_pOptions->m_ApplyAlphaBlend;
        m_Resampling            = pi_pOptions->m_Resampling;
        m_AbortRequest          = pi_pOptions->m_AbortRequest;
        m_Attributes            = pi_pOptions->m_Attributes;
        m_InTemporaryRenderMode = pi_pOptions->m_InTemporaryRenderMode;
        m_OverviewMode          = pi_pOptions->m_OverviewMode;
        m_pPreDrawOptions        = pi_pOptions->m_pPreDrawOptions;
        }
    else
        {
        m_ApplyAlphaBlend       = false;
        m_AbortRequest          = false;
        m_InTemporaryRenderMode = false;
        m_OverviewMode          = false;
        m_pPreDrawOptions        = 0;
        }
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HGFDrawOptions::HGFDrawOptions()
    : m_Resampling(HGSResampling::NEAREST_NEIGHBOUR)
    {
    m_ApplyAlphaBlend       = false;
    m_AbortRequest          = false;
    m_InTemporaryRenderMode = false;
    m_OverviewMode          = false;
    m_pPreDrawOptions        = 0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HGFDrawOptions::~HGFDrawOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGFDrawOptions& HGFDrawOptions::operator=(const HGFDrawOptions& pi_rObj)
    {
    if(this != &pi_rObj)
        {
        m_ApplyAlphaBlend       = pi_rObj.m_ApplyAlphaBlend;
        m_Resampling            = pi_rObj.m_Resampling;
        m_AbortRequest          = pi_rObj.m_AbortRequest;
        m_Attributes            = pi_rObj.m_Attributes;
        m_InTemporaryRenderMode = pi_rObj.m_InTemporaryRenderMode;
        m_OverviewMode          = pi_rObj.m_OverviewMode;
        m_pPreDrawOptions        = pi_rObj.m_pPreDrawOptions;
        }

    return(*this);
    }


//-----------------------------------------------------------------------------
// public
// operator= on a pointer (may be null)
//-----------------------------------------------------------------------------
HGFDrawOptions& HGFDrawOptions::operator=(const HGFDrawOptions* pi_pObj)
    {
    if(this != pi_pObj)
        {
        if (pi_pObj != 0)
            {
            m_ApplyAlphaBlend       = pi_pObj->m_ApplyAlphaBlend;
            m_Resampling            = pi_pObj->m_Resampling;
            m_AbortRequest          = pi_pObj->m_AbortRequest;
            m_Attributes            = pi_pObj->m_Attributes;
            m_InTemporaryRenderMode = pi_pObj->m_InTemporaryRenderMode;
            m_OverviewMode          = pi_pObj->m_OverviewMode;
            m_pPreDrawOptions       = pi_pObj->m_pPreDrawOptions;
            }
        else
            {
            m_ApplyAlphaBlend       = false;
            m_Resampling            = HGSResampling(HGSResampling::NEAREST_NEIGHBOUR);
            m_AbortRequest          = false;
            m_InTemporaryRenderMode = false;
            m_OverviewMode          = false;
            m_pPreDrawOptions       = 0;
            m_Attributes.Clear();
            }
        }

    return(*this);
    }

