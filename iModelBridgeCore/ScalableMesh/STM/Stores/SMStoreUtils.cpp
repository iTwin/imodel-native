#pragma once

#include <ScalableMeshPCH.h>

#include "SMStoreUtils.h"
#include "SMStoreUtils.hpp"

#include <ImagePP\all\h\HCDCodecZlib.h>
#include <ImagePP\all\h\HFCAccessMode.h>


template class SMIndexMasterHeader<DRange3d>;

template class SMIndexNodeHeaderBase<DRange3d>;

template class SMIndexNodeHeader<DRange3d>;

bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket, HCDPacket& pi_compressedPacket)
    {
    HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

    // initialize codec
    HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_uncompressedPacket.GetDataSize());
    pi_compressedPacket.SetBufferOwnership(true);
    size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
    pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
    const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
    pi_compressedPacket.SetDataSize(compressedDataSize);

    return true;
    }

bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket, HCDPacket& pi_uncompressedPacket)
    {
    HPRECONDITION(pi_compressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

    // initialize codec
    HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_compressedPacket.GetDataSize());
    /*pi_uncompressedPacket.SetBufferOwnership(true);
    pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));*/
    const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
    pi_uncompressedPacket.SetDataSize(compressedDataSize);

    return true;
    }