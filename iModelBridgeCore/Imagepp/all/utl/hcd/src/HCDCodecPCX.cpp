//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecPCX.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecPCX
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecPCX.h>


#define HCD_CODEC_NAME L"PCX"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecPCX::HCDCodecPCX()
    : HCDCodecVector(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecPCX::HCDCodecPCX(size_t pi_DataSize)
    : HCDCodecVector(HCD_CODEC_NAME,
                     pi_DataSize)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecPCX::HCDCodecPCX(const HCDCodecPCX& pi_rObj)
    : HCDCodecVector(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecPCX::~HCDCodecPCX()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecPCX::Clone() const
    {
    return new HCDCodecPCX(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecPCX::CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t po_OutBufferSize)
    {
    size_t  BytesCount = pi_InDataSize;
    Byte* pIn = (Byte*)pi_pInData;
    Byte* pOut = (Byte*)po_pOutBuffer;
    Byte DataByte;
    uint32_t Len;

    while(BytesCount != 0)
        {
        DataByte = (*pIn);

        // test the length of the run

        Len = 1;

        pIn++;

        while((*pIn) == DataByte && Len < 63)
            {
            Len++;
            pIn++;
            }

        BytesCount -= Len;

        // if the byte value is greater or equal to 0xc0
        if(DataByte >= 0xC0)
            {
            *pOut = (Byte)(0xC0 | Len);
            pOut++;
            *pOut = DataByte;
            pOut++;
            }
        else
            {
            if(Len <= 2)
                {
                *pOut = DataByte;
                pOut++;
                }
            else
                {
                *pOut = (Byte)(0xC0 | Len);
                pOut++;
                *pOut = DataByte;
                pOut++;
                }
            }
        }

    return(pOut - ((Byte*)po_pOutBuffer));
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecPCX::DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize)
    {
    size_t  BytesCount = GetSubsetSize();
    Byte* pIn = (Byte*)pi_pInData;
    Byte* pOut = (Byte*)po_pOutBuffer;
    Byte DataByte;
    uint32_t LenCount;

    while(BytesCount != 0)
        {
        // test if the two high bits are '1'
        if(((*pIn) & 0xC0) == 0xC0)
            {
            // if yes, extract the count
            LenCount = (*pIn & 0x3F);
            pIn++;

            // get the data byte
            DataByte = *pIn;
            pIn++;

            BytesCount -= LenCount;

            while(LenCount != 0)
                {
                *pOut = DataByte;

                pOut++;

                LenCount--;
                }
            }
        else
            {
            // simply copy the byte
            *pOut = *pIn;
            pIn++;
            pOut++;

            BytesCount--;
            }
        }

    return(GetSubsetSize());
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecPCX::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetSize() * 2);
    }

