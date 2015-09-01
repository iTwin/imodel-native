//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVETileIDIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVETileIDIterator
//-----------------------------------------------------------------------------

#pragma once

#include "HGFTileIDDescriptor.h"
#include "HVE2DRectangle.h"


BEGIN_IMAGEPP_NAMESPACE
class HVEShape;

class HVETileIDIterator
    {
    HDECLARE_SEALEDCLASS_ID(HVETileIDIteratorId_Base)

public:

    // Primary methods

    HVETileIDIterator();
    HVETileIDIterator(HGFTileIDDescriptor*    pi_pDescriptor,
                      const HFCPtr<HVEShape>& pi_rpRegion);
    ~HVETileIDIterator();

    void            SetParameters(HGFTileIDDescriptor*    pi_pDescriptor,
                                  const HFCPtr<HVEShape>& pi_rpRegion);

    // Query methods
    uint64_t       GetFirstTileIndex ();
    uint64_t       GetNextTileIndex ();
    uint64_t       GetCurrentTileIndex() const;

    HGFTileIDDescriptor*
    GetDescriptor() const;
    HFCPtr<HVEShape>
    GetRegion() const;

protected:
private:


    // Disabled because we only keep a pointer to the
    // TileIDDescriptor, which keeps state information inside...
    HVETileIDIterator (const HVETileIDIterator& pi_rObj);
    HVETileIDIterator&
    operator=(const HVETileIDIterator& pi_rObj);


    void            FindNextValidIndex();

    // Members

    // The region to iterate on
    HFCPtr<HVEShape>
    m_pRegion;

    // The descriptor to use
    HGFTileIDDescriptor*
    m_pDescriptor;

    // Index on the iteration in the raster.
    uint64_t       m_Index;
    };
END_IMAGEPP_NAMESPACE

#include "HVETileIDIterator.hpp"

