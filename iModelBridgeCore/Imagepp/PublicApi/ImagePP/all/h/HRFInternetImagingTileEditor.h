//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFInternetImagingTileEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetImagingTileEditor
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFTile.h"
#include "HGFTileIDDescriptor.h"
#include "HCDCodec.h"

class HRFInternetImagingFile;

class HRFInternetImagingTileEditor : public HRFResolutionEditor
    {
    //--------------------------------------
    // Friends & Macros
    //--------------------------------------

    friend class HRFInternetImagingFile;


public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    virtual         ~HRFInternetImagingTileEditor();


    //--------------------------------------
    // Edition by Tile
    //--------------------------------------

    virtual HSTATUS ReadBlock       (uint32_t                   pi_PosTileX,
                                     uint32_t                   pi_PosTileY,
                                     Byte*                     po_pData,
                                     HFCLockMonitor const*      pi_pSisterFileLock = 0);

    virtual HSTATUS ReadBlock       (uint32_t                   pi_PosTileX,
                                     uint32_t                   pi_PosTileY,
                                     HFCPtr<HCDPacket>&         po_rpPacket,
                                     HFCLockMonitor const*      pi_pSisterFileLock = 0);

protected:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    HRFInternetImagingTileEditor(const HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                 uint32_t                  pi_Page,
                                 unsigned short             pi_ResIndex,
                                 HFCAccessMode              pi_AccessMode,
                                 bool                       pi_WaitTileOnRead);

    HRFInternetImagingTileEditor(const HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                 uint32_t                  pi_Page,
                                 double                     pi_Resolution,
                                 HFCAccessMode              pi_AccessMode,
                                 bool                       pi_WaitTileOnRead);

    //--------------------------------------
    // Methods
    //--------------------------------------

    // Gets the tile from the tile pool or request a new one if none available
    HFCPtr<HRFTile> GetTile     (uint64_t    pi_TileID);

    // Uncompresses the tile
    HSTATUS         DecodeTile  (Byte*       po_pBuffer,
                                 const Byte* pi_pData,
                                 size_t       pi_DataSize);

    // Get the codec for the compression info of a tile
    HFCPtr<HCDCodec>
    GetCodec    (const Byte* pi_pData);

    // Wait and format a tile to a byte buffer or a packet
    HSTATUS         ReadTile    (uint64_t pi_TileID,
                                 Byte*  po_pData);
    HSTATUS         ReadTile    (uint64_t          pi_TileID,
                                 HFCPtr<HCDPacket>& po_rpPacket);



private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Keep a copy of the raster file for convenience (there is one
    // in the super-class but we have to cast it each access
    HFCPtr<HRFInternetImagingFile>  m_pPageFile;

    // A tile descriptor to convert from positions to IDs
    HGFTileIDDescriptor m_TileDescriptor;

    // Size of an uncompressed tile in bytes
    size_t              m_TileSize;

    string              m_StrResFactor;

    // Codecs for tile decompression
    HFCPtr<HCDCodec>    m_pCodecIdentity;
    HFCPtr<HCDCodec>    m_pCodecSingleColor;
    HFCPtr<HCDCodec>    m_pCodecJpeg;
    HFCPtr<HCDCodec>    m_pCodecWavelet;
    HFCPtr<HCDCodec>    m_pCodecDeflate;
    HFCPtr<HCDCodec>    m_pCodecPackBits;
    HFCPtr<HCDCodec>    m_pCodecCCITT3;
    HFCPtr<HCDCodec>    m_pCodecCCITT4;
    HFCPtr<HCDCodec>    m_pCodecRLE;

    // use by VPR
    // when is set to false, ReadBlock method return H_NOT_FOUND error if the tile
    // data is not downloaded from the server
    bool               m_WaitTileOnRead;

    //--------------------------------------
    // Methods Disabled
    //--------------------------------------

    HRFInternetImagingTileEditor(const HRFInternetImagingTileEditor& pi_rObj);
    HRFInternetImagingTileEditor& operator=(const HRFInternetImagingTileEditor& pi_rObj);
    };




