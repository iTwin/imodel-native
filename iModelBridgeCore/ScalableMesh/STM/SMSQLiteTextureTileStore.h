#pragma once

#include <../STM/IScalableMeshDataStore.h>
/*#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>*/
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>
#include "SMSQLiteFile.h"

class SMSQLiteTextureTileStore : public IScalableMeshDataStore<Byte, float, float> // JPEGData (Byte*), size
{
public:

    /*static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
    {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
    }*/
   /* SMSQLiteTextureTileStore(BENTLEY_NAMESPACE_NAME::WString filename, const IDTMFile::AccessMode& accessMode)
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
        HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

        // initialize codec
        auto codec = new HCDCodecIJG(width, height, 8 * nOfChannels);
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
        return StoreBlock(DataTypeArray, countData, HPMBlockID());
        }

    virtual HPMBlockID StoreBlock(Byte* DataTypeArray, size_t countData, HPMBlockID blockID)
        {
        HCDPacket pi_uncompressedPacket, pi_compressedPacket;
        pi_uncompressedPacket.SetBuffer(DataTypeArray + 3 * sizeof(int), countData - 3 * sizeof(int)); // The data block starts with 12 bytes of metadata, followed by pixel data
        pi_uncompressedPacket.SetDataSize(countData - 3 * sizeof(int));
        // Retrieve width, height and number of channels from the first 12 bytes of the data block
        int w = ((int*)DataTypeArray)[0];
        int h = ((int*)DataTypeArray)[1];
        int nOfChannels = ((int*)DataTypeArray)[2];
        int format = 0; // Keep an int to define the format and possible other options
        // Compress the image with JPEG
        WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, w, h, nOfChannels);
        // Create the compressed data block by storing width, height, number of channels and format before the compressed image block		
        bvector<uint8_t> texData(4 * sizeof(int) + pi_compressedPacket.GetDataSize());
        int *pHeader = (int*)(texData.data());
        pHeader[0] = w;
        pHeader[1] = h;
        pHeader[2] = nOfChannels;
        pHeader[3] = format;
        memcpy(texData.data() + 4 * sizeof(int), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
        int64_t id = blockID.IsValid() ? blockID.m_integerID : SQLiteNodeHeader::NO_NODEID;
        m_smSQLiteFile->StoreTexture(id, texData, pi_uncompressedPacket.GetDataSize()); // We store the number of bytes of the uncompressed image, ignoring the bytes used to store width, height, number of channels and format
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
    codec->SetQuality(70);
    codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
    HFCPtr<HCDCodec> pCodec = codec;    
    const size_t uncompressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
    assert(uncompressedDataSize == pi_uncompressedPacket.GetDataSize());    
    return true;
    }

    virtual size_t LoadBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetTexture(blockID.m_integerID, ptData, uncompressedSize);

    assert(uncompressedSize + sizeof(int) * 3 == maxCountData);
    HCDPacket pi_uncompressedPacket(DataTypeArray + sizeof(int) * 3, uncompressedSize, uncompressedSize);    
    HCDPacket pi_compressedPacket;
    pi_compressedPacket.SetBuffer(ptData.data() + 4 * sizeof(int), ptData.size() - 4 * sizeof(int));
    pi_compressedPacket.SetDataSize(ptData.size() - 4 * sizeof(int));
    
    int *pHeader = (int*)(ptData.data());
    int w = pHeader[0];
    int h = pHeader[1];
    int nOfChannels = pHeader[2];
    //int format = pHeader[3]; // The format is not used yet, but is may be useful in the future to support other compression than JPEG
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket, w, h, nOfChannels);    
    ((int*)DataTypeArray)[0] = w;
    ((int*)DataTypeArray)[1] = h;
    ((int*)DataTypeArray)[2] = nOfChannels;
    return maxCountData;    
    }

    size_t LoadCompressedBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
        {
        bvector<uint8_t> ptData;
        size_t uncompressedSize = 0;
        m_smSQLiteFile->GetTexture(blockID.m_integerID, ptData, uncompressedSize);
        assert(ptData.size() < maxCountData*sizeof(uint8_t));
        memcpy(DataTypeArray, ptData.data(), ptData.size());
        return ptData.size();
        }

    virtual bool DestroyBlock(HPMBlockID blockID)
    {
        return false;
    }

protected:

private:
    SMSQLiteFilePtr m_smSQLiteFile;
};