//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRATiledRasterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//
// Class: HRATiledRasterIterator
// ----------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"
#include "HGFTileIDDescriptor.h"
#include "HRABitmapBase.h"
#include "HVETileIDIterator.h"


BEGIN_IMAGEPP_NAMESPACE
class HRATiledRaster;

// ----------------------------------------------------------------------------
//  HRATiledRasterIterator
// ----------------------------------------------------------------------------
class HRATiledRasterIterator : public HRARasterIterator

    {
public:
    // Primary methods

    HRATiledRasterIterator   (const HFCPtr<HRATiledRaster>& pi_pTileRaster,
                              const HRAIteratorOptions&        pi_rOptions);

    HRATiledRasterIterator   (const HRATiledRasterIterator& pi_rObj);

    virtual         ~HRATiledRasterIterator(void);

    HRATiledRasterIterator&   operator=(const HRATiledRasterIterator& pi_rBitmap);

    // Inherited from HRARasterIterator

    virtual const HFCPtr<HRARaster>& Next();
    virtual const HFCPtr<HRARaster>& operator()();

    virtual void    Reset();

protected:

private:
    // Members

    // Keep a copy on the Raster pointer.
    HFCPtr<HRATiledRaster> m_pTileRaster;

    // Index maximun (Number of tiles)
    uint64_t           m_MaxIndex;

    // index iterator
    HVETileIDIterator   m_IDIterator;
    uint64_t           m_Index;

    // Copy of the tileDexription from the Raster
    HGFTileIDDescriptor m_TileDescription;

    HFCPtr<HRABitmapBase> m_pCurrentTile;

    // Methods
    void                PrepareCurrentTile  ();
    void                SearchNextIndex     (bool pi_FirstCall=false);
    };
END_IMAGEPP_NAMESPACE

#include "HRATiledRasterIterator.hpp"

