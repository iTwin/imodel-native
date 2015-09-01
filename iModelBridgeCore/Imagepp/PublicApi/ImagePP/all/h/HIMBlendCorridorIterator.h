//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMBlendCorridorIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


#include "HRARasterIterator.h"
#include "HRAReferenceToRaster.h"
#include "HIMBlendCorridor.h"
#include "HVETileIDIterator.h"

BEGIN_IMAGEPP_NAMESPACE''

/** -----------------------------------------------------------------------------
    Iterator class for blend corridors.
    -----------------------------------------------------------------------------
*/
class HIMBlendCorridorIterator : public HRARasterIterator
    {
public:

    // Primary methods

    HIMBlendCorridorIterator(const HFCPtr<HIMBlendCorridor>& pi_rpBlendCorridor,
                             const HRAIteratorOptions&       pi_rOptions);

    HIMBlendCorridorIterator(const HIMBlendCorridorIterator& pi_rObj);

    HIMBlendCorridorIterator&
    operator=(const HIMBlendCorridorIterator& pi_rObj);

    virtual         ~HIMBlendCorridorIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next();

    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();


private:

    void                PrepareCurrentTile();
    void                SearchNextIndex(bool pi_FirstCall = false);


    // Info. for useful tiles.
    uint64_t           m_MaxIndex;

    HVETileIDIterator   m_IDIterator;
    uint64_t           m_Index;

    HFCPtr<HRARaster>   m_pCurrentTile;

    /** The scale down limit below which we won't
        return any part of the blend. Below that, the
        blend wouldn't be visible anyways.
    */
    static double  s_BlendUtilityTreshold;
    };

END_IMAGEPP_NAMESPACE