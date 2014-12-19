//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDPacket.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDPacket
//-----------------------------------------------------------------------------
// Class used by the codec.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

class HCDCodec;

class HCDPacket : public HFCShareableObject<HCDPacket>
    {
    HDECLARE_BASECLASS_ID(1156)

public:

    // constructors

    _HDLLu                    HCDPacket();

    _HDLLu                 HCDPacket(  Byte* pi_pBuffer,
                                       size_t pi_BufferSize,
                                       size_t pi_DataSize = 0);

    _HDLLu                 HCDPacket(  const HFCPtr<HCDCodec>& pi_pCodec,
                                       Byte* pi_pBuffer,
                                       size_t pi_BufferSize,
                                       size_t pi_DataSize = 0);

    _HDLLu                 HCDPacket(const HCDPacket& pi_rPacket);
    _HDLLu HCDPacket&      operator=(const HCDPacket& pi_rPacket);



    // destructor

    _HDLLu virtual         ~HCDPacket();


    // settings

    size_t          GetBufferSize() const;

    size_t          GetDataSize() const;

    _HDLLu void            SetDataSize(size_t pi_DataSize);

    Byte*         GetBufferAddress() const;

    _HDLLu void            SetBuffer(void* pi_pBuffer, size_t pi_BufferSize);

    _HDLLu void            SetCodec(const HFCPtr<HCDCodec>& pi_pCodec);

    _HDLLu HFCPtr<HCDCodec>& GetCodec() const;


    // compression

    _HDLLu bool           Compress(HCDPacket* pio_pOutPacket) const;

    _HDLLu bool           Decompress(HCDPacket* pio_pOutPacket);


    // memory management

    bool            HasBufferOwnership() const;
    _HDLLu void            SetBufferOwnership(bool pi_Owner);
    _HDLLu void            ShrinkBufferToDataSize();
    _HDLLu void            ChangeBufferSize(size_t pi_NewBufferSize);


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

#include "HCDPacket.hpp"

