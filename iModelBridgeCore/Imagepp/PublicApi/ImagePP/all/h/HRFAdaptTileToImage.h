//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAdaptTileToImage.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptTileToImage
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HFCMacros.h"
#include "HRFBlockAdapter.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRFRasterFile;

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
class HRFAdaptTileToImageCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptTileToImageCapabilities)

public:
    HRFAdaptTileToImageCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptTileToImageCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptTileToImageCreator)

public:
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const;

    // Creation of implementator
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                unsigned short       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const;
    };

//-----------------------------------------------------------------------------
// This specific implementation adapt the storage type to another
//-----------------------------------------------------------------------------
class HRFAdaptTileToImage : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    // friend class HRFRasterFile;
    HRFAdaptTileToImage(
        HRFBlockAdapterCapabilities*   pi_pCapabilities,
        HFCPtr<HRFRasterFile>          pi_rpRasterFile,
        uint32_t                       pi_Page,
        unsigned short                pi_Resolution,
        HFCAccessMode                  pi_AccessMode);

    virtual                ~HRFAdaptTileToImage();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket,
                              HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

protected:

    // Pixel information
    uint32_t                                m_ExactBytesPerWidth;

    // tile Information
    uint32_t                                m_TileWidth;
    uint32_t                                m_TileHeight;
    uint32_t                                m_TilePerWidth;
    uint32_t                                m_TileSize;
    uint32_t                                m_TilePerHeight;
    uint32_t                                m_Height;
    uint32_t                                m_ExactBytesPerTileWidth;
    uint32_t                                m_ExactBytesToCopyForTheLastRightTile;
    HArrayAutoPtr<Byte>                    m_pTile;

private:
    // Methods Disabled
    HRFAdaptTileToImage(const HRFAdaptTileToImage& pi_rObj);
    HRFAdaptTileToImage& operator=(const HRFAdaptTileToImage& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

