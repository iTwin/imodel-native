//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDPacket.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDPacket
//-----------------------------------------------------------------------------
// Class used by the codec.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodec;

class HCDPacket : public HFCShareableObject<HCDPacket>
    {
    HDECLARE_BASECLASS_ID(HCDPacketId_Base)

public:

    // constructors

    IMAGEPP_EXPORT                    HCDPacket();

    IMAGEPP_EXPORT                 HCDPacket(  Byte* pi_pBuffer,
                                       size_t pi_BufferSize,
                                       size_t pi_DataSize = 0);

    IMAGEPP_EXPORT                 HCDPacket(  const HFCPtr<HCDCodec>& pi_pCodec,
                                       Byte* pi_pBuffer,
                                       size_t pi_BufferSize,
                                       size_t pi_DataSize = 0);

    IMAGEPP_EXPORT                 HCDPacket(const HCDPacket& pi_rPacket);
    IMAGEPP_EXPORT HCDPacket&      operator=(const HCDPacket& pi_rPacket);



    // destructor

    IMAGEPP_EXPORT virtual         ~HCDPacket();


    // settings

    size_t          GetBufferSize() const;

    size_t          GetDataSize() const;

    IMAGEPP_EXPORT void            SetDataSize(size_t pi_DataSize);

    Byte*         GetBufferAddress() const;

    IMAGEPP_EXPORT void            SetBuffer(void* pi_pBuffer, size_t pi_BufferSize);

    IMAGEPP_EXPORT void            SetCodec(const HFCPtr<HCDCodec>& pi_pCodec);

    IMAGEPP_EXPORT HFCPtr<HCDCodec>& GetCodec() const;


    // compression

    IMAGEPP_EXPORT bool           Compress(HCDPacket* pio_pOutPacket) const;

    IMAGEPP_EXPORT bool           Decompress(HCDPacket* pio_pOutPacket);


    // memory management

    bool            HasBufferOwnership() const;
    IMAGEPP_EXPORT void            SetBufferOwnership(bool pi_Owner);
    IMAGEPP_EXPORT void            ShrinkBufferToDataSize();
    IMAGEPP_EXPORT void            ChangeBufferSize(size_t pi_NewBufferSize);


    virtual HCDPacket* Clone() const;

protected:


private:


    // attributes

    Byte*         m_pBuffer;
    size_t          m_DataSize;
    size_t          m_BufferSize;
    bool           m_BufferOwner;
    HFCPtr<HCDCodec>
    m_pCodec;
    };

END_IMAGEPP_NAMESPACE

#include "HCDPacket.hpp"

