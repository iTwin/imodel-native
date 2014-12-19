//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAStripedRaster.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAStripedRaster
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRAStripedRaster.h>
#include <Imagepp/all/h/HGF2DGrid.h>


HPM_REGISTER_CLASS(HRAStripedRaster, HRATiledRaster)


// Constants
//

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HRAStripedRaster::HRAStripedRaster ()
    : HRATiledRaster ()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAStripedRaster::HRAStripedRaster  (const HFCPtr<HRAStoredRaster>& pi_pRasterModel,
                                     uint64_t        pi_StripHeight,
                                     uint64_t        pi_WidthPixels,
                                     uint64_t        pi_HeightPixels,
                                     HPMObjectStore* pi_pStore,
                                     HPMPool*        pi_pLog,
                                     bool           pi_DisableTileStatus)

    : HRATiledRaster  (pi_pRasterModel,
                       pi_WidthPixels, pi_StripHeight,
                       pi_WidthPixels, pi_HeightPixels,
                       pi_pStore, pi_pLog, pi_DisableTileStatus)
    {
    }




//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRAStripedRaster::HRAStripedRaster(const HRAStripedRaster& pi_rObj)
    : HRATiledRaster (pi_rObj)
    {
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAStripedRaster::~HRAStripedRaster()
    {
    }




//-----------------------------------------------------------------------------
// public
// Assignment operation
//-----------------------------------------------------------------------------
HRAStripedRaster& HRAStripedRaster::operator=(const HRAStripedRaster& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        HRATiledRaster::operator=(pi_rObj);
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// InitPhysicalShape - Init the physical shape, set a new shape and flush
//                     the previous Data.
//-----------------------------------------------------------------------------
void HRAStripedRaster::InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels)
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Marc.Bedard                     03/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    HFCPtr<HRATiledRaster> pTmpRaster = new HRATiledRaster (m_pRasterModel,
                                                            pi_WidthPixels,
                                                            GetTileSizeY(),
                                                            pi_WidthPixels,
                                                            pi_HeightPixels,
                                                            GetStore(),
                                                            m_pPool);
    // Keep the ID
    pTmpRaster->SetID (GetID());

    // Replace the object
    ReplaceObject (pTmpRaster);
    }





//----------------------------------------- Statics





