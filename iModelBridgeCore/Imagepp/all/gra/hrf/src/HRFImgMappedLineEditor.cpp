//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFImgMappedLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
                //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFImgMappedFile.h>
#include <Imagepp/all/h/HRFImgMappedLineEditor.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>


/** ---------------------------------------------------------------------------
    Constructor.
    ImgMapped format line editor.

    @param pi_rpRasterFile  Raster file.
    @param pi_Page          Page descriptor.
    @param pi_Resolution    Resolution descriptor.
    @param pi_AccessMode    Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFImgMappedLineEditor::HRFImgMappedLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                               uint32_t              pi_Page,
                                               unsigned short       pi_Resolution,
                                               HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    }

/** ---------------------------------------------------------------------------
    Destructor.
    ---------------------------------------------------------------------------
 */
HRFImgMappedLineEditor::~HRFImgMappedLineEditor()
    {
    }

/** ---------------------------------------------------------------------------
    Read a specific block (line) of pixels beginning at @i{pi_PosBlockX}, @i{pi_PosBlockY}.

    @param pi_PosBlockX  Line number.
    @param pi_PosBlockY  Column number.
    @param po_pData      Buffer to contain readen data.

    @return HSTATUS H_SUCCESS.
    ---------------------------------------------------------------------------
 */
HSTATUS HRFImgMappedLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                          uint64_t pi_PosBlockY,
                                          Byte*   po_pData,
                                          HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        HPRECONDITION (po_pData != 0);
        HPRECONDITION (m_AccessMode.m_HasReadAccess);

        uint32_t Offset;

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        Offset = static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_OffsetToPixelData +
                 ((uint32_t)pi_PosBlockY * (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth());

        //:> Place file ptr
        if (static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->GetCurrentPos() != Offset)
            static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->SeekToPos(Offset);

        //:> Read data
        uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
        if(static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->Read(po_pData, DataSize) != DataSize)
            Status = H_ERROR;

        //:> Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

/** ---------------------------------------------------------------------------
    Write a specific block (line) of pixels beginning at @i{pi_PosBlockX}, @i{pi_PosBlockY}.

    @param pi_PosBlockX  Line number.
    @param pi_PosBlockY  Column number.
    @param po_pData      Buffer that contains data to be written.

    @return HSTATUS H_SUCCESS.
    ---------------------------------------------------------------------------
 */
HSTATUS HRFImgMappedLineEditor::WriteBlock(uint64_t       pi_PosBlockX,
                                           uint64_t       pi_PosBlockY,
                                           const Byte*    pi_pData,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS     Status = H_SUCCESS;

    uint32_t Offset;

    Offset = static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_OffsetToPixelData +
             ((uint32_t)pi_PosBlockY * (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth());

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    //:> Place file ptr
    if (static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->GetCurrentPos() != Offset)
        static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->SeekToPos(Offset);

    //:> Write data
    uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
    if(static_cast<HRFImgMappedFile*>(GetRasterFile().GetPtr())->m_pImgMappedFile->Write(pi_pData, DataSize) != DataSize)
        Status = H_ERROR;

    //:> Increment the counter after the edition
    GetRasterFile()->SharingControlIncrementCount();

    //:> Unlock the sister file
    SisterFileLock.ReleaseKey();


    return Status;
    }
