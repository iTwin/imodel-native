//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAdaptStripToTile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptStripToTile
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
class HRFAdaptStripToTileCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptStripToTileCapabilities)

public:
    HRFAdaptStripToTileCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptStripToTileCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptStripToTileCreator)

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
class HRFAdaptStripToTile : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    // friend class HRFRasterFile;
    HRFAdaptStripToTile(
        HRFBlockAdapterCapabilities*   pi_pCapabilities,
        HFCPtr<HRFRasterFile>          pi_rpRasterFile,
        uint32_t                       pi_Page,
        unsigned short                pi_Resolution,
        HFCAccessMode                  pi_AccessMode);

    virtual                ~HRFAdaptStripToTile();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t               pi_PosBlockX,
                               uint64_t               pi_PosBlockY,
                               const Byte*            pi_pData,
                               HFCLockMonitor const*  pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }

    void                   NoMoreRead    () override;

protected:
    HSTATUS        LoadBlocks     (uint32_t                pi_PosBlockX,
                                   uint32_t                pi_PosBlockY,
                                   HFCLockMonitor const*   pi_pSisterFileLock);
    HSTATUS        SaveBlocks     (HFCLockMonitor const* pi_pSisterFileLock);

    // Resolution dimension
    uint32_t                                m_Height;
    uint32_t                                m_Width;

    // Tile dimension
    uint32_t                                m_BlockHeight;
    uint32_t                                m_BlockWidth;

    uint32_t                                m_StripHeight;

    // Calc the number of bytes per Image Width
    double                                 m_DBytesByPixel;
    uint32_t                                m_ExactBytesPerWidth;

    // Calc the number of blocks per width
    uint32_t                                m_BlocksPerWidth;

    // Calc the number of bytes per Tile Width
    uint32_t                                m_ExactBytesPerBlockWidth;

    // Blocks information
    uint32_t                                m_PosTileY;
    bool                                   m_IsBlocksEmpty;
    bool                                   m_IsBlocksOverwritten;

    // Blocks data
    HArrayAutoPtr<Byte>                    m_pStripBuffer;
    HArrayAutoPtr<HArrayAutoPtr<Byte> >    m_ppBlocks;

private:

    void Alloc_m_ppBlocks               ();

    // Methods Disabled
    HRFAdaptStripToTile(const HRFAdaptStripToTile& pi_rObj);
    HRFAdaptStripToTile& operator=(const HRFAdaptStripToTile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
