//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRASurface.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRASurface
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HRAEditor.h>

#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>

// Codec
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>

// macro
#define BytesFromBits(NbBits) ((NbBits + 7) / 8)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRASurface::HRASurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor)
    : m_pSurfaceDescriptor(pi_rpDescriptor)
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

            m_pSurfaceDescriptor=new HGSMemoryRLESurfaceDescriptor(rpDescriptor->GetWidth(),
                                                                   rpDescriptor->GetHeight(),
                                                                   rpDescriptor->GetPixelType(),
                                                                   pPacketRLE,
                                                                   HGF_UPPER_LEFT_HORIZONTAL);
            // create editor to initialize the buffer
            HRAEditor SurfaceEditor(*this);
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

            m_pSurfaceDescriptor=new HGSMemorySurfaceDescriptor(rpDescriptor->GetWidth(),
                                                                rpDescriptor->GetHeight(),
                                                                rpDescriptor->GetPixelType(),
                                                                pPacket,
                                                                HGF_UPPER_LEFT_HORIZONTAL,
                                                                BytesPerRow);
            // create editor to initialize the buffer
            HRAEditor SurfaceEditor(*this);
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
const HFCPtr<HGSRegion>& HRASurface::GetRegion() const {return m_pRegion;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRASurface::SetRegion(HFCPtr<HGSRegion> const& pRegion){m_pRegion=pRegion;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
const HFCPtr<HGSSurfaceDescriptor>& HRASurface::GetSurfaceDescriptor() const {return m_pSurfaceDescriptor;}