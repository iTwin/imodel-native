//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAStripedRaster.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRAStripedRaster
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Clone -
//-----------------------------------------------------------------------------
inline HRARaster* HRAStripedRaster::Clone (HPMObjectStore* pi_pStore,
                                           HPMPool*        pi_pLog) const
    {
    // Store Specify ?
    if (pi_pStore)
        {
        // Copy the Tile Raster
        //
        HRAStripedRaster* pTmpRaster = new HRAStripedRaster ();

        pTmpRaster->HRAStoredRaster::operator=(*this);

        pTmpRaster->DeepDelete();

        pTmpRaster->CopyMembers(*this);

        pTmpRaster->DeepCopy (*this, pi_pStore, pi_pLog);

        return (pTmpRaster);
        }
    else
        return new HRAStripedRaster (*this);
    }

//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HRAStripedRaster::Clone () const
    {
    return new HRAStripedRaster(*this);
    }
