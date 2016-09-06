//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFMapboxTileEditor.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

BEGIN_IMAGEPP_NAMESPACE




struct MapBoxTileQuery 
{
    MapBoxTileQuery(uint64_t tileId, Utf8StringCR tileUri, HRFMapBoxFile& rasterFile) 
    :m_tileId(tileId), m_tileUri(tileUri), m_rasterFile(rasterFile) 
        {}

    virtual ~MapBoxTileQuery(){};
    
    virtual void _Run();

    uint64_t                    m_tileId;
    Utf8String                  m_tileUri;
    bvector<Byte>               m_tileData;
    HRFMapBoxFile&        m_rasterFile;  // Do not hold a HRFMapBoxEditor since it might be destroyed while query are still running. 
};


class HRFMapBoxFile;

class HRFMapBoxTileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFMapBoxFile;

    virtual ~HRFMapBoxTileEditor  ();

    // Edition by block
    virtual HSTATUS ReadBlock(uint64_t               pi_PosBlockX,
                              uint64_t               pi_PosBlockY,
                              Byte*                  po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t       pi_PosBlockX,
                               uint64_t       pi_PosBlockY,
                               const Byte*    pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFMapBoxTileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                        uint32_t              pi_Page,
                        unsigned short       pi_Resolution,
                        HFCAccessMode         pi_AccessMode);
private:

    HGFTileIDDescriptor m_TileIDDescriptor;

    // Methods Disabled
    HRFMapBoxTileEditor(const HRFMapBoxTileEditor& pi_rObj);
    HRFMapBoxTileEditor& operator=(const HRFMapBoxTileEditor& pi_rObj);
    };

END_IMAGEPP_NAMESPACE

