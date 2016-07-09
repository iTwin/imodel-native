#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include "SMPointTileStore.h"
#include "SMSQLiteFile.h"


template <class POINT, class EXTENT> class SMSQLitePointTileStore : public SMPointTileStore<POINT, EXTENT>
{
private:

    bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
        HCDPacket& pi_compressedPacket)
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

    bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket,
        HCDPacket& pi_uncompressedPacket)
    {
        HPRECONDITION(pi_compressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_compressedPacket.GetDataSize());
       /* pi_uncompressedPacket.SetBufferOwnership(true);
        pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));*/
        const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
        pi_uncompressedPacket.SetDataSize(compressedDataSize);

        return true;
    }

public:
    /*SMSQLitePointTileStore(BENTLEY_NAMESPACE_NAME::WString filename, const ISMStore::AccessMode& accessMode)
    {
    m_smSQLiteFile = SMSQLiteFile::Create();
    Utf8String filenameA;
    BeStringUtilities::WCharToUtf8(filenameA, filename.c_str());
        if (accessMode.m_HasCreateAccess)
            m_smSQLiteFile->Create(filenameA.c_str());
        else
            m_smSQLiteFile->Open(filenameA.c_str());
    }*/
    // Because SMSQLiteFile is created before by ScalableMesh, we didn't to opened or created it in TileStore.
    SMSQLitePointTileStore(SMSQLiteFilePtr database)
    {
        m_smSQLiteFile = database;
    }

    virtual ~SMSQLitePointTileStore()
    {
        // We didn't want to close it. It's ScalableMesh which did this stuff now (and if SMSQLiteFilePtr is destroy, close is automatically called)
    //m_smSQLiteFile->Close();
    }
   
    virtual uint64_t GetNextID() const override
        {
        //return m_smSQLiteFile->GetLastInsertRowId(); // This only works if last insert was performed on the same database connection
        return m_smSQLiteFile->GetLastNodeId();
        }

    virtual void Close()
    {
    }

    virtual bool StoreMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader == nullptr) return 0;
    SQLiteIndexHeader header = *indexHeader;
    m_smSQLiteFile->SetMasterHeader(header);
    return true;
    }

    virtual size_t LoadMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
    {
    if (indexHeader == nullptr) return 0;
    SQLiteIndexHeader header;
    if (!m_smSQLiteFile->GetMasterHeader(header)) return 0;
    if (header.m_rootNodeBlockID == SQLiteNodeHeader::NO_NODEID) return 0;
    *indexHeader = header;
    return sizeof(*indexHeader);
    }

    virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData)
    {
    std::string s;
    s = " DATA HAS " + std::to_string(countData) + " POINTS ";
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(POINT));
    pi_uncompressedPacket.SetDataSize(countData*sizeof(POINT));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> ptData(pi_compressedPacket.GetDataSize());
    memcpy(&ptData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = SQLiteNodeHeader::NO_NODEID;
    m_smSQLiteFile->StorePoints(id, ptData, countData*sizeof(POINT));
    return HPMBlockID(id);
    }

    virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(POINT));
    pi_uncompressedPacket.SetDataSize(countData*sizeof(POINT));
    WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
    bvector<uint8_t> ptData(pi_compressedPacket.GetDataSize());
    memcpy(&ptData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
    int64_t id = blockID.m_integerID;
    m_smSQLiteFile->StorePoints(id, ptData, countData*sizeof(POINT));
    return HPMBlockID(id);
    }

    virtual size_t GetBlockDataCount(HPMBlockID blockID) const
    {
    if(!blockID.IsValid()) return 0;
    return m_smSQLiteFile->GetNumberOfPoints(blockID.m_integerID) / sizeof(POINT);
    }

    virtual size_t StoreNodeHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
    {
    if (header == nullptr) return 0;
    if (header->m_ptsIndiceID.size() > 0)header->m_ptsIndiceID[0] = blockID;
    SQLiteNodeHeader nodeHeader = *header;
    if (header->m_SubNodeNoSplitID.IsValid() && !header->m_apSubNodeID[0].IsValid())
        nodeHeader.m_apSubNodeID[0] = nodeHeader.m_SubNodeNoSplitID;
    nodeHeader.m_nodeID = blockID.m_integerID;
    nodeHeader.m_graphID = nodeHeader.m_nodeID;
    m_smSQLiteFile->SetNodeHeader(nodeHeader);
    return sizeof(header);
    }

    virtual size_t LoadNodeHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
    {
    if (header == nullptr) return 0;
    SQLiteNodeHeader nodeHeader;
    if (header->m_meshComponents != nullptr) delete[] header->m_meshComponents;
    nodeHeader.m_nodeID = blockID.m_integerID;
    if (!m_smSQLiteFile->GetNodeHeader(nodeHeader)) return 1;
    //nodeHeader.m_nbPoints = GetBlockDataCount(blockID);
    *header = nodeHeader;
    header->m_IsLeaf = header->m_apSubNodeID.size() == 0 || (!header->m_apSubNodeID[0].IsValid());
    header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());
    if (!header->m_IsLeaf && !header->m_IsBranched)
        {
        header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];
        header->m_apSubNodeID[0] = HPMBlockID();
        }
    //if (header->m_ptsIndiceID.size() > 0)header->m_ptsIndiceID[0] = blockID;
    if (header->m_isTextured)
        {
        header->m_uvID = blockID;
        header->m_ptsIndiceID[0] = HPMBlockID();
        }
    header->m_graphID = blockID;
    return sizeof(*header);
    }

    virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
    {
    if (!blockID.IsValid()) return 0;
    bvector<uint8_t> ptData;
    size_t uncompressedSize = 0;
    m_smSQLiteFile->GetPoints(blockID.m_integerID, ptData, uncompressedSize);
    HCDPacket pi_uncompressedPacket, pi_compressedPacket;
    pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
    pi_compressedPacket.SetDataSize(ptData.size());
    pi_uncompressedPacket.SetBuffer(DataTypeArray, maxCountData*sizeof(POINT));
    pi_uncompressedPacket.SetBufferOwnership(false);
    LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
    //memcpy(DataTypeArray, pi_uncompressedPacket.GetBufferAddress(), std::min(uncompressedSize, maxCountData*sizeof(POINT)));
    return std::min(uncompressedSize, maxCountData*sizeof(POINT));
    }

    virtual bool DestroyBlock(HPMBlockID blockID)
    {
        return false;
    }

    SMSQLiteFile* GetDbConnection() { return m_smSQLiteFile.get(); }

private:
    SMSQLiteFilePtr m_smSQLiteFile;
};