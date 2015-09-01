//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMBufferedImageIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HIMBufferedImageIterator
//-----------------------------------------------------------------------------
// This class describes an image buffer iterator.
//-----------------------------------------------------------------------------

#pragma once


#include "HRARasterIterator.h"
#include "HRAReferenceToRaster.h"
#include "HIMBufferedImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HIMBufferedImageIterator : public HRARasterIterator
    {
public:

    // Primary methods

    HIMBufferedImageIterator(const HFCPtr<HIMBufferedImage>& pi_pBufImg,
                             const HRAIteratorOptions&       pi_rOptions);

    HIMBufferedImageIterator(const HIMBufferedImageIterator& pi_rObj);

    HIMBufferedImageIterator&
    operator=(const HIMBufferedImageIterator& pi_rObj);

    virtual         ~HIMBufferedImageIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next();

    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();

    // Debug function
#ifdef __HMR_PRINTSTATE
    virtual void    PrintState(ostream& po_rOutput) const;
#endif

protected:

private:

    // Advance to the next useful tile
    void            FindNextUsefulID();
    void            Initialize();

    // Info. for useful tiles.
    uint64_t    m_TileCount;
    HIMBufImgTileIDSet*
    m_pIDSet;
    HIMBufImgTileIDSet::iterator
    m_IDSetIterator;

    uint64_t    m_CurrentID;

    HFCPtr<HRARaster>
    m_pCurrentRaster;


    bool           m_Valid;
    };


END_IMAGEPP_NAMESPACE