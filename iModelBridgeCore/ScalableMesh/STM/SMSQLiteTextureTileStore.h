#pragma once

#include <ScalableMesh/IScalableMeshDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include "SMSQLiteFile.h"

class SMSQLiteTextureTileStore : public IScalableMeshDataStore<Byte, float, float> // JPEGData (Byte*), size
{
public:

    static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
    {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
    }
   /* SMSQLiteTextureTileStore(Bentley::WString filename, const IDTMFile::AccessMode& accessMode)
        {
        m_smSQLiteFile = SMSQLiteFile::Create();
        Utf8String filenameA;
        BeStringUtilities::WCharToUtf8(filenameA, filename.c_str());
        if (accessMode.m_HasCreateAccess)
            m_smSQLiteFile->Create(filenameA.c_str());
        else
            m_smSQLiteFile->Open(filenameA.c_str());
        }*/

    SMSQLiteTextureTileStore(SMSQLiteFilePtr file)
        {
        m_smSQLiteFile = file;
        }

    virtual ~SMSQLiteTextureTileStore()
        {
        //m_smSQLiteFile->Close();
        }
    virtual void Close()
        {}

    virtual bool StoreMasterHeader(float* indexHeader, size_t headerSize)
    {
        return false;
    }

    virtual size_t LoadMasterHeader(float* indexHeader, size_t headerSize)
    {
        return 0;
    }

    bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
        HCDPacket& pi_compressedPacket, int width, int height, int nOfChannels = 3)
    {
        HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<UInt32>::max) ());

        // initialize codec
        auto codec = new HCDCodecIJG(width, height, 8 * nOfChannels);
        codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
        codec->SetQuality(70);
        codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
        HFCPtr<HCDCodec> pCodec = codec;
        pi_compressedPacket.SetBufferOwnership(true);
        size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
        pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
        const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
        pi_compressedPacket.SetDataSize(compressedDataSize);

        return true;
    }

    // New interface

    virtual HPMBlockID StoreNewBlock(Byte* DataTypeArray, size_t countData)
        {
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
        pi_uncompressedPacket.SetDataSize(countData);
        size_t w, h;
        w = h = (size_t)sqrt(countData / 4);
        WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, (int)w, (int)h, 4);
        bvector<uint8_t> texData(pi_compressedPacket.GetDataSize());
        memcpy(&texData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
        int64_t id = SQLiteNodeHeader::NO_NODEID;
        m_smSQLiteFile->StoreIndices(id, texData, countData);
        return HPMBlockID(id);
        }

    virtual HPMBlockID StoreBlock(Byte* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_uncompressedPacket.SetBuffer(DataTypeArray, countData);
        pi_uncompressedPacket.SetDataSize(countData);
        size_t w, h;
        w = h = (size_t)sqrt(countData / 4);
        WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, (int)w, (int)h, 4);
        bvector<uint8_t> texData(pi_compressedPacket.GetDataSize());
        memcpy(&texData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
        int64_t id = blockID.m_integerID;
        m_smSQLiteFile->StoreTexture(id, texData, countData);
        return HPMBlockID(id);
        }

    virtual size_t GetBlockDataCount(HPMBlockID blockID) const
    {
    return m_smSQLiteFile->GetTextureByteCount(blockID.m_integerID) + 3 * sizeof(int);
    }


    virtual size_t StoreHeader(float* header, HPMBlockID blockID)
    {
        return 0;
    }

    virtual size_t LoadHeader(float* header, HPMBlockID blockID)
    {

        return 0;
    }

    bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket, HCDPacket& pi_uncompressedPacket, size_t width, size_t height, size_t nOfChannels = 3)
    {
    auto codec = new HCDCodecIJG(width, height, nOfChannels * 8);// (pi_compressedPacket.GetDataSize()); // 24 bits per pixels
    codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
    codec->SetQuality(70);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec = codec;
    pi_uncompressedPacket.SetBufferOwnership(true);
    pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
    const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
    pi_uncompressedPacket.SetDataSize(compressedDataSize);
    return true;
    }

    virtual size_t LoadBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetTexture(blockID.m_integerID, ptData, uncompressedSize);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
    pi_compressedPacket.SetDataSize(ptData.size());
    pi_uncompressedPacket.SetDataSize(uncompressedSize);
    size_t w, h;
    w = h = (size_t)sqrt(uncompressedSize / 4);
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket,(int)w,(int)h,4);
    ((int*)DataTypeArray)[0] = (int)w;
    ((int*)DataTypeArray)[1] = (int)h;
    ((int*)DataTypeArray)[2] = 4;
    memcpy(DataTypeArray+3*sizeof(int), pi_uncompressedPacket.GetBufferAddress(), std::min(uncompressedSize, maxCountData));
    return std::min(uncompressedSize + 3 * sizeof(int), maxCountData);
    }

    size_t LoadCompressedBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        bvector<uint8_t> ptData;
        size_t uncompressedSize = 0;
        m_smSQLiteFile->GetTexture(blockID.m_integerID, ptData, uncompressedSize);
        assert(ptData.size() < maxCountData*sizeof(uint8_t));
        memcpy(DataTypeArray, &uncompressedSize, sizeof(uncompressedSize));
        memcpy(DataTypeArray + sizeof(uncompressedSize), ptData.data(), ptData.size());
        return ptData.size() + sizeof(uncompressedSize);
        }

    virtual bool DestroyBlock(HPMBlockID blockID)
    {
        return false;
    }

protected:

private:
    SMSQLiteFilePtr m_smSQLiteFile;
};