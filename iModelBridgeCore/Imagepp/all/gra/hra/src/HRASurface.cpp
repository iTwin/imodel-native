//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRASurface.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRASurface
//---------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRAEditor.h>

#include <Imagepp/all/h/HGSSurfaceCapability.h>
#include <Imagepp/all/h/HGSSurfaceAttribute.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

// PixelType
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI2R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV32A8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8Mask.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

// Codec
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>



HGS_BEGIN_SURFACECAPABILITIES_REGISTRATION(HRASurface)
// Surface type
HGS_REGISTER_SURFACECAPABILITY(HGSSurfaceTypeAttribute(HGSSurfaceType::MEMORY))
// Pixel type
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI1R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI1R8G8B8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI2R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI4R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI4R8G8B8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI8R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI8R8G8B8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI8VA8R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI8Gray8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV1Gray1::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV1GrayWhite1::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV8Gray8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV8GrayWhite8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV16B5G5R5::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV16R5G6B5::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV16PRGray8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV24R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV24B8G8R8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV24PhotoYCC::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32CMYK::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32A8R8G8B8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32R8G8B8X8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32B8G8R8X8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV48R16G16B16::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV64R16G16B16A16::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV64R16G16B16X16::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV16Gray16::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV16Int16::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV32Float32::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeI8R8G8B8Mask::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSPixelTypeAttribute(HRPPixelTypeV96R32G32B32::CLASS_ID))
// Memory alignment
HGS_REGISTER_SURFACECAPABILITY(HGSMemoryAlignmentAttribute(HGSMemoryAlignment::DWORD))
HGS_REGISTER_SURFACECAPABILITY(HGSMemoryAlignmentAttribute(HGSMemoryAlignment::BYTE))
// Compression
HGS_REGISTER_SURFACECAPABILITY(HGSCompressionAttribute(HCDCodecIdentity::CLASS_ID))
HGS_REGISTER_SURFACECAPABILITY(HGSCompressionAttribute(HCDCodecHMRRLE1::CLASS_ID))
// SLO
HGS_REGISTER_SURFACECAPABILITY(HGSSLOAttribute(HGF_UPPER_LEFT_HORIZONTAL))
HGS_REGISTER_SURFACECAPABILITY(HGSSLOAttribute(HGF_LOWER_LEFT_HORIZONTAL))

HGS_END_SURFACECAPABILITIES_REGISTRATION()


// macro
#define BytesFromBits(NbBits) ((NbBits + 7) / 8)




//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRASurface::HRASurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor)
    : HGSSurfaceImplementation(pi_rpDescriptor)
    {
    HPRECONDITION(pi_rpDescriptor != 0);
    HPRECONDITION(pi_rpDescriptor->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID) ||
                  pi_rpDescriptor->IsCompatibleWith(HGSMemoryRLESurfaceDescriptor::CLASS_ID));

    if(pi_rpDescriptor->IsCompatibleWith(HGSMemoryRLESurfaceDescriptor::CLASS_ID))
        {
        // test if the descriptor has a pointer on data
        const HFCPtr<HGSMemoryRLESurfaceDescriptor>& rpDescriptor = (const HFCPtr<HGSMemoryRLESurfaceDescriptor>&)pi_rpDescriptor;

        if(rpDescriptor->GetRLEPacket() == 0)
            {
            // create a RLE buffer
            // calculate the buffer size, the size was for a black bitmap
            // The RLE must be start and finish with a black pixel (0)
            // The maximun run length was 32767 (SHRT_MAX)
            size_t LineSize = (((uint32_t)(rpDescriptor->GetWidth() / SHRT_MAX) * 2) + 1) * sizeof(uint32_t);

            HFCPtr<HCDPacketRLE> pPacketRLE = new HCDPacketRLE(pi_rpDescriptor->GetWidth(), pi_rpDescriptor->GetHeight());
            pPacketRLE->SetBufferOwnership(true);

            for(uint32_t LineNo=0; LineNo < rpDescriptor->GetHeight(); ++LineNo)
                pPacketRLE->SetLineBuffer(LineNo, new Byte[LineSize], LineSize, 0);

            SetSurfaceDescriptor(new HGSMemoryRLESurfaceDescriptor(rpDescriptor->GetWidth(),
                                                                   rpDescriptor->GetHeight(),
                                                                   rpDescriptor->GetPixelType(),
                                                                   pPacketRLE,
                                                                   HGF_UPPER_LEFT_HORIZONTAL));
            // create editor to initialize the buffer
            HRAEditor SurfaceEditor(0, this);
            uint32_t ClearValue(0);
            SurfaceEditor.Clear(&ClearValue); // clear in black
            }
        }
    else
        {
        // test if the descriptor has a pointer on data
        const HFCPtr<HGSMemorySurfaceDescriptor>& rpDescriptor = (const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_rpDescriptor;

        if (rpDescriptor->GetPacket() == 0)
            {
            // allocate a buffer and set other data

            HFCPtr<HCDPacket> pPacket;
            uint32_t BytesPerRow;

            HPRECONDITION(rpDescriptor->GetPixelType() != 0);
            if (rpDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID) ||
                rpDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID))
                {
                HPRECONDITION(((const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_rpDescriptor)->GetBytesPerRow() == 0); // cannot set the bytes per row with a compressed data

                // create a RLE buffer
                // calculate the buffer size, the size was for a black bitmap
                // The RLE must be start and finish with a black pixel (0)
                // The maximun run length was 32767 (SHRT_MAX)
                size_t LineSize = (((uint32_t)(rpDescriptor->GetWidth() / SHRT_MAX) * 2) + 1) * sizeof(uint32_t);
                size_t BufferSize = LineSize * rpDescriptor->GetHeight();

                pPacket = new HCDPacket(new HCDCodecHMRRLE1(pi_rpDescriptor->GetWidth(),
                                                            pi_rpDescriptor->GetHeight()),
                                        new Byte[BufferSize],
                                        BufferSize);
                pPacket->SetBufferOwnership(true);

                ((HFCPtr<HCDCodecHMRRLE1>&)pPacket->GetCodec())->EnableLineIndexesTable(true);

                // the editor will initialize the RLE data and the index
                BytesPerRow = 0; // cannot known the size of one row with compressed data
                }
            else
                {
                BytesPerRow = BytesFromBits(rpDescriptor->GetWidth() *
                                            rpDescriptor->GetPixelType()->CountPixelRawDataBits());
                HASSERT((((const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_rpDescriptor)->GetBytesPerRow() == 0) ||
                        (((const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_rpDescriptor)->GetBytesPerRow() == BytesPerRow));
                uint32_t BufferSize = BytesPerRow * rpDescriptor->GetHeight();
                pPacket = new HCDPacket(new HCDCodecIdentity(),
                                        new Byte[BufferSize],
                                        BufferSize);
                pPacket->SetBufferOwnership(true);
                }

            SetSurfaceDescriptor(new HGSMemorySurfaceDescriptor(rpDescriptor->GetWidth(),
                                                                rpDescriptor->GetHeight(),
                                                                rpDescriptor->GetPixelType(),
                                                                pPacket,
                                                                HGF_UPPER_LEFT_HORIZONTAL,
                                                                BytesPerRow));
            // create editor to initialize the buffer
            HRAEditor SurfaceEditor(0, this);
            uint32_t ClearValue(0);
            SurfaceEditor.Clear(&ClearValue); // clear in black
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRASurface::~HRASurface()
    {
    }

