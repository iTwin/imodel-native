//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFIntergraphTileEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"
#include "HRFIntergraphFile.h"
#include "HFCBinStream.h"

BEGIN_IMAGEPP_NAMESPACE
//class HRFIntergraphFile;
class HGFTileIDDescriptor;

class HRFIntergraphTileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFIntergraphFile;

    HRFIntergraphTileEditor
    (HFCPtr<HRFRasterFile> pi_rpRasterFile,
     uint32_t              pi_Page,
     unsigned short       pi_Resolution,
     HFCAccessMode         pi_AccessMode,
     HRFIntergraphFile::IntergraphResolutionDescriptor&
     pi_rIntergraphResolutionDescriptor,
     HRFIntergraphFile::ListOfFreeBlock&
     pio_rListOfFreeBlock);

    virtual        ~HRFIntergraphTileEditor  ();


    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t            pi_PosBlockX,
                              uint64_t            pi_PosBlockY,
                              Byte*               po_pData,
                              HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS ReadBlock(uint64_t             pi_PosBlockX,
                              uint64_t             pi_PosBlockY,
                              HFCPtr<HCDPacket>&   po_rpPacket,
                              HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint64_t           pi_PosBlockX,
                               uint64_t           pi_PosBlockY,
                               const Byte*        pi_pData,
                               HFCLockMonitor const* pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

private:
    // Methods Disabled
    HRFIntergraphTileEditor (const HRFIntergraphTileEditor& pi_rObj);
    HRFIntergraphTileEditor& operator= (const HRFIntergraphTileEditor& pi_rObj);

    // Methods
    bool IsUnInstanciateTile      (uint32_t pi_PosTileX, uint32_t pi_PosTileY, const Byte* pi_pData);

    void  FindFileFreeSpace        (uint32_t& pio_rOldBlockOffset, uint32_t& pio_rOldBlockSize, uint32_t pi_RequestedSize);

    void  InitializeJpegDecompTable(double pi_QualityFactor, Byte* po_pTileBuffer, unsigned int pi_DataSize);
    void  BuildJpegLumiChromaTable (double pi_QualityFactor, Byte* po_pLuminance,  Byte* po_pChroma);

    void  ApplyLUTColorCorrection  (Byte* pio_pData, uint32_t pi_pixelCount);

    void  AddInitialFreeBlock     (const uint32_t* pi_pOffset,  const uint32_t*  pi_pSize,   uint32_t pi_Count);
    void  CheckAlloc              (      uint32_t* pio_pOffset,       uint32_t pi_CurSize, uint32_t pi_NewSize);
    void  CheckAllocWithoutUpdate (      uint32_t* pio_pOffset,       uint32_t pi_CurSize, uint32_t pi_NewSize);

    bool GetListFreeBlock        (      uint32_t** pi_ppOffset,      uint32_t** pi_ppSize,  uint32_t* pi_pCount);
    bool OverlapsFreeBlocks      (      uint32_t pi_Offset,        uint32_t pi_Size) const;

    uint32_t GetCurrentTileSizeInByte( const uint32_t pi_PosBlockX, const uint32_t pi_PosBlockY) const;



    bool VerrifyBlockOverlap();

    void MapStreamHole();

    // The List is sorted by Offset;
    HRFIntergraphFile::ListOfFreeBlock& m_ListOfFreeBlock;
    bool                               m_ListDirty;

    // Members
    unsigned short m_BitPerPixel;

    uint32_t        m_RasterOffset;

    uint32_t        m_TileSizeInByte;
    uint32_t        m_TileWidthInByte;
    uint32_t        m_TileHeight;
    uint32_t         m_PageIndex;

    Byte*         m_pCompBuffer;

    HRFIntergraphFile::IntergraphResolutionDescriptor&
    m_IntergraphResolutionDescriptor;

    HGFTileIDDescriptor*
    m_pTileIdDescriptor;
    HFCBinStream*   m_pIntergraphFile;
    };
END_IMAGEPP_NAMESPACE

