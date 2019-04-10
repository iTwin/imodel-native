//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTiffTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTiffTileEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFTiffTileEditor.h>
#include <ImagePP/all/h/HRFTiffFile.h>
#include <ImagePP/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HCDPacket.h>

#define RASTER_FILE     static_cast<HRFTiffFile*>(GetRasterFile().GetPtr())
#define TIFF_FILE       (RASTER_FILE->GetFilePtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFTiffTileEditor::HRFTiffTileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)

    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_IndexOfPage  = RASTER_FILE->GetIndexOfPage(m_Page);

    HTIFFFile::DirectoryID CurDir = TIFF_FILE->CurrentDirectory();
    TIFF_FILE->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, m_IndexOfPage+pi_Resolution));

    TIFF_FILE->SetDirectory(CurDir);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFTiffTileEditor::~HRFTiffTileEditor()
    {
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffTileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*  po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);
    
    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // read the tile
    Status = TIFF_FILE->TileRead(po_pData, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffTileEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                     uint64_t           pi_PosBlockY,
                                     HFCPtr<HCDPacket>& po_rpPacket)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);
    
    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // Set ownership to true is the buffer size is zero
    if (po_rpPacket->GetBufferSize() == 0)
        {
        po_rpPacket->SetBufferOwnership(true);
        }

    // Read the compressed tile
    Status = TIFF_FILE->TileRead(po_rpPacket, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffTileEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                      uint64_t     pi_PosBlockY,
                                      const Byte*  pi_pData)
    {
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);

    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // write the tile
    Status =  TIFF_FILE->TileWrite(pi_pData, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    // Update the directories.
    RASTER_FILE->SaveDescriptors();

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffTileEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                      uint64_t                 pi_PosBlockY,
                                      const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);

    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    Status = TIFF_FILE->TileWriteCompress(pi_rpPacket->GetBufferAddress(),
             (uint32_t)pi_rpPacket->GetDataSize(), (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    // Update the directories;
    RASTER_FILE->SaveDescriptors();

    return Status;
    }
