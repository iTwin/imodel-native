//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoRasterEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFGeoRasterEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFGeoRasterEditor.h>
#include <Imagepp/all/h/HRFGeoRasterFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/SDOGeoRasterWrapper.h>

#define GEORASTER_RASTERFILE      (static_cast<HRFGeoRasterFile*>(GetRasterFile().GetPtr()))

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFGeoRasterEditor::HRFGeoRasterEditor(HFCPtr<HRFRasterFile>    pi_rpRasterFile,
                                       uint32_t                 pi_Page,
                                       unsigned short          pi_Resolution,
                                       HFCAccessMode            pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    HPRECONDITION(m_pResolutionDescriptor->GetPixelType()->GetChannelOrg().CountChannels() >= 1);
    HPRECONDITION((m_pResolutionDescriptor->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->
                   GetSize() % 8) == 0);

    unsigned short BitsPerChannel = m_pResolutionDescriptor->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize();

    if ((BitsPerChannel >= 16) && (GEORASTER_RASTERFILE->m_IsBigEndian == false))
        {
        //Currently we don't support the swapping of multibytes per channel, multichannels pixel.
        HASSERT(m_pResolutionDescriptor->GetPixelType()->GetChannelOrg().CountChannels() == 1);
        m_NbBytesToSwap = (Byte)BitsPerChannel / 8;
        }
    else
        {
        m_NbBytesToSwap = 0;
        }

    m_pSDOGeoRasterWrapper = GEORASTER_RASTERFILE->m_pSDOGeoRasterWrapper.get();

    m_pCodec = GetResolutionDescriptor()->GetCodec();
    m_BlockSizeInByte = GetResolutionDescriptor()->GetBlockSizeInBytes();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFGeoRasterEditor::~HRFGeoRasterEditor()
    {
    }




//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGeoRasterEditor::ReadBlock(uint64_t                pi_PosBlockX,
                                      uint64_t                pi_PosBlockY,
                                      Byte*                   po_pData,
                                      HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Will reload directories if necessary
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_pCodec != 0 && !m_pCodec->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        {
        Byte* pBuffer;
        size_t BufferSize;
        m_pSDOGeoRasterWrapper->GetBlock(m_Resolution,
                                         0, // band
                                         (uint32_t)pi_PosBlockX / m_pResolutionDescriptor->GetBlockWidth(),
                                         (uint32_t)pi_PosBlockY / m_pResolutionDescriptor->GetBlockHeight(),
                                         &pBuffer,
                                         &BufferSize);


        HCDPacket Compressed(m_pCodec,
                             pBuffer,
                             BufferSize,
                             BufferSize);
        Compressed.SetBufferOwnership(true);

        HCDPacket Uncompressed(po_pData, m_BlockSizeInByte);
        try
            {
            Compressed.Decompress(&Uncompressed);
            }
        catch (...)
            {
            pBuffer = pBuffer;
            }
        }
    else
        {
        m_pSDOGeoRasterWrapper->GetBlock(m_Resolution,
                                         0, // band
                                         (uint32_t)pi_PosBlockX / m_pResolutionDescriptor->GetBlockWidth(),
                                         (uint32_t)pi_PosBlockY / m_pResolutionDescriptor->GetBlockHeight(),
                                         po_pData,
                                         m_pResolutionDescriptor->GetBlockSizeInBytes());
        }

    switch (m_NbBytesToSwap)
        {
        case 2:
            SwabArrayOfShort((unsigned short*)po_pData, m_BlockSizeInByte / sizeof(unsigned short));
            break;
        case 4:
            SwabArrayOfLong((uint32_t*)po_pData, m_BlockSizeInByte / sizeof(uint32_t));
            break;
        default:
            HASSERT(m_NbBytesToSwap == 0);
            break;
        }

    return Status;
    }





//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGeoRasterEditor::WriteBlock(uint64_t               pi_PosBlockX,
                                       uint64_t               pi_PosBlockY,
                                       const Byte*            pi_pData,
                                       HFCLockMonitor const*  pi_pSisterFileLock)
    {
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS RetValue = H_ERROR;

    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Will reload directories if necessary
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_pCodec != 0 && !m_pCodec->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        {
        size_t MaxCompressedSize = m_pCodec->GetSubsetMaxCompressedSize();
        Byte* pRasterData = new Byte[MaxCompressedSize];

        // Create a compress packet were the data will be compress
        HCDPacket Compress(m_pCodec,
                           pRasterData,
                           MaxCompressedSize);
        Compress.SetBufferOwnership(true);

        // Compress the data and get it's new size.
        HCDPacket UnCompress(const_cast<Byte*>(pi_pData),
                             m_pResolutionDescriptor->GetBlockSizeInBytes(),
                             m_pResolutionDescriptor->GetBlockSizeInBytes());
        UnCompress.Compress(&Compress);

        if (m_pSDOGeoRasterWrapper->SetBlock(m_Resolution,
                                             0, // band
                                             (uint32_t)pi_PosBlockX / m_pResolutionDescriptor->GetBlockWidth(),
                                             (uint32_t)pi_PosBlockY / m_pResolutionDescriptor->GetBlockHeight(),
                                             Compress.GetBufferAddress(),
                                             Compress.GetBufferSize()))
            RetValue = H_SUCCESS;

        }
    else
        {
        if (m_pSDOGeoRasterWrapper->SetBlock(m_Resolution,
                                             0, // band
                                             (uint32_t)pi_PosBlockX / m_pResolutionDescriptor->GetBlockWidth(),
                                             (uint32_t)pi_PosBlockY / m_pResolutionDescriptor->GetBlockHeight(),
                                             pi_pData,
                                             m_pResolutionDescriptor->GetBlockSizeInBytes()))
            RetValue = H_SUCCESS;
        }



    return RetValue;
    }
