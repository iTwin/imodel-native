//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAStripedRaster.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAStripedRaster
//-----------------------------------------------------------------------------
// This class describes an image buffer.
//-----------------------------------------------------------------------------

#pragma once

#include "HRATiledRaster.h"

class HRAStripedRaster : public HRATiledRaster
    {
    HPM_DECLARE_CLASS_DLL(_HDLLg,  1238)

public:

    // Primary methods

    HRAStripedRaster ();
    HRAStripedRaster (const HFCPtr<HRAStoredRaster>& pi_pRasterModel,
                      uint64_t        pi_StripHeight,
                      uint64_t        pi_WidthPixels,
                      uint64_t        pi_HeightPixels,
                      HPMObjectStore* pi_pStore=0,
                      HPMPool*        pi_pLog=0,
                      bool           pi_DisableTileStatus = false);

    HRAStripedRaster(const HRAStripedRaster& pi_rObj);

    virtual ~HRAStripedRaster();

    HRAStripedRaster& operator=(const HRAStripedRaster& pi_rObj);


    // Overriden from HRATiledRaster

    virtual void    InitSize(uint64_t pi_WidthPixels, uint64_t pi_HeightPixels);

    virtual HPMPersistentObject* Clone () const;

    virtual HRARaster*
    Clone (HPMObjectStore* pi_pStore,
           HPMPool*        pi_pLog=0) const;


protected:

private:

    // Members

    // Methods

    };

#include "HRAStripedRaster.hpp"

