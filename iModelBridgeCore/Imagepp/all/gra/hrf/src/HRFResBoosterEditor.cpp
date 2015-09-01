//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFResBoosterEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFResBoosterEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFResBoosterEditor.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFResBoosterEditor::HRFResBoosterEditor(   HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                            uint32_t              pi_Page,
                                            unsigned short       pi_Resolution,
                                            HFCAccessMode         pi_AccessMode,
                                            HRFResolutionEditor*  pi_pBoosterResolutionEditor)
    : HRFResolutionEditor(  pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    // Keep the editor of booster file
    m_pBoosterResolutionEditor = pi_pBoosterResolutionEditor;
    }


//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFResBoosterEditor::~HRFResBoosterEditor()
    {
    if (GetResolutionDescriptor()->PaletteHasChanged())
        m_pBoosterResolutionEditor->SetPalette(GetResolutionDescriptor()->GetPixelType()->GetPalette());
    }


//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);

    // Copy the specified Block to the client buffer
    return m_pBoosterResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                       uint64_t           pi_PosBlockY,
                                       HFCPtr<HCDPacket>& po_rpPacket,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    // Copy the specified Block to the client buffer
    return m_pBoosterResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                          uint64_t                 pi_PosBlockY,
                                          HFCPtr<HCDPacketRLE>&    po_rpPacketRLE,
                                          HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    // Copy the specified Block to the client buffer
    return m_pBoosterResolutionEditor->ReadBlockRLE(pi_PosBlockX, pi_PosBlockY, po_rpPacketRLE);
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status;

    // Copy the specified tile to the booster file
    Status = m_pBoosterResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_pData);
    if (Status == H_SUCCESS)
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                        uint64_t                 pi_PosBlockY,
                                        const HFCPtr<HCDPacket>& pi_rpPacket,
                                        HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status;

    // Copy the specified tile to the client buffer
    if (H_SUCCESS ==
        (Status = m_pBoosterResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_rpPacket)))
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFResBoosterEditor::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                           uint64_t              pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status;

    // Copy the specified tile to the booster file
    Status = m_pBoosterResolutionEditor->WriteBlockRLE(pi_PosBlockX, pi_PosBlockY, pi_rpPacketRLE);
    if (Status == H_SUCCESS)
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }