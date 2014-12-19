//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFPreDrawOptions.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFPreDrawOptions
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGFPreDrawOptions.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HGFPreDrawOptions::HGFPreDrawOptions(uint32_t pi_DrawSurfaceMaxWidth,
                                     uint32_t pi_DrawSurfaceMaxHeight)
    {
    HPRECONDITION(pi_DrawSurfaceMaxWidth > 0);
    HPRECONDITION(pi_DrawSurfaceMaxHeight > 0);
    /*
        m_DrawSurfaceMaxWidth = pi_DrawSurfaceMaxWidth;
        m_DrawSurfaceMaxHeight = pi_DrawSurfaceMaxHeight;   */
    m_NbOfDrawCalls = 0;
    m_pUpSubResProgressListener = 0;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HGFPreDrawOptions::HGFPreDrawOptions(const HGFPreDrawOptions& pi_rOptions)
    {
    *this = pi_rOptions;
    }


//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HGFPreDrawOptions::~HGFPreDrawOptions()
    {
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGFPreDrawOptions& HGFPreDrawOptions::operator=(const HGFPreDrawOptions& pi_rObj)
    {
    if(this != &pi_rObj)
        {   /*
            m_DrawSurfaceMaxWidth = pi_rObj.m_DrawSurfaceMaxWidth;
            m_DrawSurfaceMaxHeight = pi_rObj.m_DrawSurfaceMaxHeight;

            m_NbOfDrawCalls
            m_ConsumerId =
            m_Async = ;
            m_pUpSubResProgressListener =
            m_PrecomputedData =         */
        }

    return(*this);
    }

/*
//-----------------------------------------------------------------------------
// public
// GetDrawSurfaceMaxDimension
//-----------------------------------------------------------------------------
void HGFPreDrawOptions::GetDrawSurfaceMaxDimension(UInt32& po_rDrawSurfaceMaxWidth,
                                                          UInt32& po_rDrawSurfaceMaxHeight)
{
    po_rDrawSurfaceMaxWidth = m_DrawSurfaceMaxWidth;
    po_rDrawSurfaceMaxHeight = m_DrawSurfaceMaxHeight;
}

//-----------------------------------------------------------------------------
// public
// SetNbTilesToUpdate
//-----------------------------------------------------------------------------
void    HGFPreDrawOptions::SetDrawSurfaceMaxDimension(UInt32 pi_DrawSurfaceMaxWidth,
                                                          UInt32 pi_DrawSurfaceMaxHeight)
{
    m_DrawSurfaceMaxWidth = pi_DrawSurfaceMaxWidth;
    m_DrawSurfaceMaxHeight = pi_DrawSurfaceMaxHeight;
}


//-----------------------------------------------------------------------------
// public
// SetNbTilesToUpdate
//-----------------------------------------------------------------------------
void    HGFPreDrawOptions::SetNbTilesToUpdate(UInt64 pi_NbTilesToUpdate)
{
    m_NbTilesToUpdate = pi_NbTilesToUpdate;
}

//-----------------------------------------------------------------------------
// public
// GetNbTilesToUpdate
//-----------------------------------------------------------------------------
UInt64 HGFPreDrawOptions::GetNbTilesToUpdate()
{
    return m_NbTilesToUpdate;
}
*/
//-----------------------------------------------------------------------------
// public
// SetLookAheadOptions
//-----------------------------------------------------------------------------
void    HGFPreDrawOptions::SetLookAheadOptions(uint32_t pi_ConsumerId,
                                                      bool  pi_Async)
    {
    m_ConsumerId = pi_ConsumerId;
    m_Async         = pi_Async;
    }

//-----------------------------------------------------------------------------
// public
// GetLookAheadOptions
//-----------------------------------------------------------------------------
void    HGFPreDrawOptions::GetLookAheadOptions(uint32_t& po_ConsumerId,
                                                      bool&  po_Async)
    {
    po_ConsumerId = m_ConsumerId;
    po_Async      = m_Async;
    }

//-----------------------------------------------------------------------------
// public
// GetDataWarehouse
//-----------------------------------------------------------------------------
HFCDataWarehouse& HGFPreDrawOptions::GetPrecomputedDataStorage()
    {
    return m_PrecomputedData;
    }

//-----------------------------------------------------------------------------
// public
// GetLookAheadOptions
//-----------------------------------------------------------------------------
HFCProgressListener*    HGFPreDrawOptions::GetUpdateSubResProgressListener()
    {
    return m_pUpSubResProgressListener;
    }

//-----------------------------------------------------------------------------
// public
// GetLookAheadOptions
//-----------------------------------------------------------------------------
void    HGFPreDrawOptions::SetUpdateSubResProgressListener(HFCProgressListener* pi_pUpSubResProgressListener)
    {
    m_pUpSubResProgressListener = pi_pUpSubResProgressListener;
    }

