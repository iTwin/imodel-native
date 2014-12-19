//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFPreDrawOptions.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFPreDrawOptions
//-----------------------------------------------------------------------------

#pragma once

#include "HFCDataWarehouse.h"

class HFCProgressListener;

class HGFPreDrawOptions : public HFCShareableObject<HGFPreDrawOptions>
    {
public:

    // Primary methods
    _HDLLg                    HGFPreDrawOptions(uint32_t pi_DrawSurfaceMaxWidth,
                                                uint32_t pi_DrawSurfaceMaxHeight);

    HGFPreDrawOptions(const HGFPreDrawOptions& pi_rOptions);

    _HDLLg    virtual         ~HGFPreDrawOptions();

    // Operators

    HGFPreDrawOptions&        operator=(const HGFPreDrawOptions& pi_rObj);
    /*
          void                    GetDrawSurfaceMaxDimension(UInt32& po_rDrawSurfaceMaxWidth,
                                                             UInt32& po_rDrawSurfaceMaxHeight);

          void                    SetDrawSurfaceMaxDimension(UInt32 pi_DrawSurfaceMaxWidth,
                                                             UInt32 pi_DrawSurfaceMaxHeight);
    */
    void                    SetNbTilesToUpdate(uint64_t pi_NbTilesToUpdate);
    uint64_t                  GetNbTilesToUpdate();


    HFCDataWarehouse&        GetPrecomputedDataStorage();

    void                    SetLookAheadOptions(uint32_t pi_ConsumerId,
                                                bool  pi_Async);

    void                    GetLookAheadOptions(uint32_t& po_ConsumerId,
                                                bool&  pio_Async);

    HFCProgressListener*    GetUpdateSubResProgressListener();

    _HDLLg    void            SetUpdateSubResProgressListener(HFCProgressListener* pi_pUpSubResProgressListener);

private:

    uint32_t            m_AccessTileWidth;
    uint32_t            m_AccessTileHeight;
    uint32_t            m_AccessTilePerRow;
    uint32_t            m_AccessTilePerColumn;

    uint32_t               m_NbOfDrawCalls;
    uint32_t                m_ConsumerId;
    bool                 m_Async;
    HFCProgressListener* m_pUpSubResProgressListener;
    HFCDataWarehouse     m_PrecomputedData;
    };

