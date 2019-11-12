//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>
                //:> must be first for PreCompiledHeader Option

#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFImgRGBFile.h>
#include <ImagePP/all/h/HRFImgRGBLineEditor.h>

#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>


/** ---------------------------------------------------------------------------
    Constructor.
    ImgRGB format line editor.

    @param pi_rpRasterFile  Raster file.
    @param pi_Page          Page descriptor.
    @param pi_Resolution    Resolution descriptor.
    @param pi_AccessMode    Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFImgRGBLineEditor::HRFImgRGBLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                         uint32_t              pi_Page,
                                         uint16_t       pi_Resolution,
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
HRFImgRGBLineEditor::~HRFImgRGBLineEditor()
    {
    }

/** ---------------------------------------------------------------------------
    Read a specific block (line) of pixels beginning at @i{pi_PosBlockX}, @i{pi_PosBlockY}.

    @param pi_PosBlockX  Line number.
    @param pi_PosBlockY  Column number.
    @param po_pData      Buffer to contain readen data.

    @return HSTATUS.
    ---------------------------------------------------------------------------
 */
HSTATUS HRFImgRGBLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY, 
                                       Byte*  po_pData)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

        {
        uint32_t    channelLineLength = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth() / 3;
        uint64_t     Offset            = pi_PosBlockY * channelLineLength;

        HArrayAutoPtr<Byte> pBuffRed(new Byte[channelLineLength*3]);
        Byte* pBuffGreen = &(pBuffRed[channelLineLength]);
        Byte* pBuffBlue  = &(pBuffRed[channelLineLength*2]);

        //:> Place file ptrs
        if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->GetCurrentPos() != Offset)
            static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->SeekToPos(Offset);
        if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->GetCurrentPos() != Offset)
            static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->SeekToPos(Offset);
        if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->GetCurrentPos() != Offset)
            static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->SeekToPos(Offset);

        //:> Read each channel
        if(static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->Read(pBuffRed, channelLineLength) != channelLineLength ||
           static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->Read(pBuffGreen, channelLineLength) != channelLineLength ||
           static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->Read(pBuffBlue, channelLineLength) != channelLineLength)
            goto WRAPUP;

        //:> Fill client buffer
        for (uint32_t pos=0, bufPos=0; pos<channelLineLength; pos++, bufPos+=3)
            {
            po_pData[bufPos]   = pBuffRed[pos];
            po_pData[bufPos+1] = pBuffGreen[pos];
            po_pData[bufPos+2] = pBuffBlue[pos];
            }
        }

    Status = H_SUCCESS;

WRAPUP:
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
HSTATUS HRFImgRGBLineEditor::WriteBlock(uint64_t      pi_PosBlockX,
                                        uint64_t      pi_PosBlockY,
                                        const Byte*   pi_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS     Status = H_SUCCESS;

    uint32_t    channelLineLength = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth() / 3;
    uint64_t     Offset            = pi_PosBlockY * channelLineLength;

    HArrayAutoPtr<Byte> pBuffRed(new Byte[channelLineLength*3]);
    Byte* pBuffGreen = &(pBuffRed [channelLineLength]);
    Byte* pBuffBlue  = &(pBuffRed[channelLineLength*2]);

    //:> Fill temp buffer
    for (uint32_t pos=0, bufPos=0; pos<channelLineLength; pos++, bufPos+=3)
        {
        pBuffRed[pos]   = pi_pData[bufPos];
        pBuffGreen[pos] = pi_pData[bufPos+1];
        pBuffBlue[pos]  = pi_pData[bufPos+2];
        }

    //:> Place file ptrs
    if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->GetCurrentPos() != Offset)
        static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->SeekToPos(Offset);
    if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->GetCurrentPos() != Offset)
        static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->SeekToPos(Offset);
    if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->GetCurrentPos() != Offset)
        static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->SeekToPos(Offset);

    //:> Write each channel
    if (static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pRedFile->Write(pBuffRed, channelLineLength) != channelLineLength ||
        static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pGreenFile->Write(pBuffGreen, channelLineLength)  != channelLineLength ||
        static_cast<HRFImgRGBFile*>(GetRasterFile().GetPtr())->m_pBlueFile->Write(pBuffBlue, channelLineLength) != channelLineLength)
        Status = H_ERROR;

    return Status;
    }
