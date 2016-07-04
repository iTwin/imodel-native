#pragma once
#include "IScalableMeshDataStore.h"
#include "SMPointTileStore.h"
#include <Mtg/MtgStructs.h>
#include "SMSQLiteFile.h"
#undef min
#undef max

class SMSQLiteGraphTileStore : public IScalableMeshDataStore<MTGGraph, Byte, Byte>
{
private:

    bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
                               HCDPacket& pi_compressedPacket)
        {
        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_uncompressedPacket.GetDataSize());
        pi_compressedPacket.SetBufferOwnership(true);
        size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
        pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
        const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
        pi_compressedPacket.SetDataSize(compressedDataSize);

        return true;
        }

    bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket,
                              HCDPacket& pi_uncompressedPacket)
        {
        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_compressedPacket.GetDataSize());
        pi_uncompressedPacket.SetBufferOwnership(true);
        pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
        const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
        pi_uncompressedPacket.SetDataSize(compressedDataSize);

        return true;
        }
public:

    static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
    {
        return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
    }

    /*SMSQLiteGraphTileStore(BENTLEY_NAMESPACE_NAME::WString filename, const IDTMFile::AccessMode& accessMode)
        {
        m_smSQLiteFile = SMSQLiteFile::Create();
        Utf8String filenameA;
        BeStringUtilities::WCharToUtf8(filenameA, filename.c_str());
        if (accessMode.m_HasCreateAccess)
            m_smSQLiteFile->Create(filenameA.c_str());
        else
            m_smSQLiteFile->Open(filenameA.c_str());
        }*/

    SMSQLiteGraphTileStore(SMSQLiteFilePtr file)
        {
        m_smSQLiteFile = file;
        }

    virtual ~SMSQLiteGraphTileStore()
        {
        //m_smSQLiteFile->Close();
        }

    virtual void Close()
    {
    }

    virtual bool StoreMasterHeader(Byte* indexHeader, size_t headerSize)
    {
        return false;
    }

    virtual size_t LoadMasterHeader(Byte* indexHeader, size_t headerSize)
    {
        return 0;
    }

    // New interface

    virtual HPMBlockID StoreNewBlock(MTGGraph* DataTypeArray, size_t countData)
    {
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    void* serializedData;
    size_t serializedSize = DataTypeArray->WriteToBinaryStream(serializedData);
    pi_uncompressedPacket.SetBuffer(serializedData, serializedSize);
    pi_uncompressedPacket.SetDataSize(serializedSize);
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> graphData(pi_compressedPacket.GetDataSize());
    memcpy(&graphData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = SQLiteNodeHeader::NO_NODEID;
    m_smSQLiteFile->StoreIndices(id, graphData, serializedSize);
    delete[] serializedData;
    return HPMBlockID(id);
    }

    virtual HPMBlockID StoreBlock(MTGGraph* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
    void* serializedData;
    size_t serializedSize = DataTypeArray->WriteToBinaryStream(serializedData);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(serializedData, serializedSize);
    pi_uncompressedPacket.SetDataSize(serializedSize);
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> graphData(pi_compressedPacket.GetDataSize());
    memcpy(&graphData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = blockID.m_integerID;
    m_smSQLiteFile->StoreGraph(id, graphData, serializedSize);
    delete[] serializedData;
    return HPMBlockID(id);
    }

    virtual size_t GetBlockDataCount(HPMBlockID blockID) const
    {
        //Currently only support the one graph. 
        return 1;
    }


    virtual size_t StoreHeader(Byte* header, HPMBlockID blockID)
    {
        return 1;
    }

    virtual size_t LoadHeader(Byte* header, HPMBlockID blockID)
    {

        return 1;
    }

    virtual size_t LoadBlock(MTGGraph* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetGraph(blockID.m_integerID, ptData, uncompressedSize);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
    pi_compressedPacket.SetDataSize(ptData.size());
    pi_uncompressedPacket.SetDataSize(uncompressedSize);
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
    MTGGraph * graph = new(DataTypeArray)MTGGraph(); //some of the memory managers call malloc but not the constructor
    if(uncompressedSize>0)graph->LoadFromBinaryStream(pi_uncompressedPacket.GetBufferAddress(), uncompressedSize);
        return 1;
    }

    virtual bool DestroyBlock(HPMBlockID blockID)
    {
        return false;
    }

protected:

private:
    SMSQLiteFilePtr m_smSQLiteFile;
};