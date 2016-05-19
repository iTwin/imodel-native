//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFAdaptLineToTile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptLineToTile
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
class HRFAdaptLineToTileCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptLineToTileCapabilities)

public:
    HRFAdaptLineToTileCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptLineToTileCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptLineToTileCreator)

public:
    virtual ~HRFAdaptLineToTileCreator();
        
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const;

    // Creation of implementator
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                uint16_t       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const;
    };

//-----------------------------------------------------------------------------
// This specific implementation adapt the storage type to another
//-----------------------------------------------------------------------------
class HRFAdaptLineToTile : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    // friend class HRFRasterFile;
    HRFAdaptLineToTile
    (HRFBlockAdapterCapabilities*  pi_pCapabilities,
     HFCPtr<HRFRasterFile>         pi_rpRasterFile,
     uint32_t                      pi_Page,
     uint16_t               pi_Resolution,
     HFCAccessMode                 pi_AccessMode);

    virtual                ~HRFAdaptLineToTile();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                  pi_PosBlockX,
                              uint64_t                  pi_PosBlockY,
                              Byte*                     po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                  pi_PosBlockX,
                              uint64_t                  pi_PosBlockY,
                              HFCPtr<HCDPacket>&        po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }


    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }


    virtual void           NoMoreRead    () override;

protected:
    HSTATUS        LoadTiles     (uint32_t                 pi_PosBlockX,
                                  uint32_t                 pi_PosBlockY);
    HSTATUS        SaveTiles     ();

    // Resolution dimension
    uint32_t                                m_Height;
    uint32_t                                m_Width;

    // Tile dimension
    uint32_t                                m_BlockHeight;
    uint32_t                                m_BlockWidth;

    // Calc the number of bytes per Image Width
    double                                 m_DBytesByPixel;
    uint32_t                                m_ExactBytesPerWidth;

    // Calc the number of tiles per width
    uint32_t                                m_BlocksPerWidth;

    // Calc the number of bytes per Tile Width
    uint32_t                                m_ExactBytesPerBlockWidth;

    HArrayAutoPtr<Byte>                    m_LineBuffer;

    // Tiles information
    uint32_t                                m_PosTileY;
    bool                                   m_IsBlocksEmpty;
    bool                                   m_IsBlocksOverwritten;
    HSTATUS                                m_LoadTilesStatus; // Remember the readBlock status so we can return the appropriate status for each tile.

    // Tiles data
    HArrayAutoPtr<HArrayAutoPtr<Byte> >    m_ppBlocks;

private:
    void Alloc_m_ppBlocks               ();

    // Methods Disabled
    HRFAdaptLineToTile(const HRFAdaptLineToTile& pi_rObj);
    HRFAdaptLineToTile& operator=(const HRFAdaptLineToTile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
