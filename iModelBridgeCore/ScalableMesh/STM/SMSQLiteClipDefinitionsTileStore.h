#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include "SMPointTileStore.h"
#include "SMSQLiteFile.h"


template <class EXTENT> class SMSQLiteClipDefinitionsTileStore : public SMPointTileStore<DPoint3d, EXTENT>
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
            pi_uncompressedPacket.SetBufferOwnership(true);
            pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
            const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
            pi_uncompressedPacket.SetDataSize(compressedDataSize);

            return true;
            }

    public:
        // Don't need this
        /*SMSQLiteIndiceTileStore(BENTLEY_NAMESPACE_NAME::WString filename, const ISMStore::AccessMode& accessMode)
        {
        m_smSQLiteFile = SMSQLiteFile::Create();
        Utf8String filenameA;
        BeStringUtilities::WCharToUtf8(filenameA, filename.c_str());
        if (accessMode.m_HasCreateAccess)
        m_smSQLiteFile->Create(filenameA.c_str());
        else
        m_smSQLiteFile->Open(filenameA.c_str());
        }*/

        SMSQLiteClipDefinitionsTileStore(SMSQLiteFilePtr file)
            {
            m_smSQLiteFile = file;
            }

        virtual ~SMSQLiteClipDefinitionsTileStore()
            {
            //m_smSQLiteFile->Close();
            }
     
        virtual void Close()
            {}

        virtual bool StoreMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {
            return 0;
            }

        virtual size_t LoadMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {
            return 0;
            }

        virtual HPMBlockID StoreNewBlock(DPoint3d* DataTypeArray, size_t countData)
            {
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DPoint3d));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(DPoint3d));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            m_smSQLiteFile->StoreClipPolygon(id, indexData, countData*sizeof(DPoint3d));
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(DPoint3d* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DPoint3d));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(DPoint3d));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = blockID.m_integerID;
            m_smSQLiteFile->StoreClipPolygon(id, indexData, countData*sizeof(DPoint3d));
            return HPMBlockID(id);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return m_smSQLiteFile->GetClipPolygonByteCount(blockID.m_integerID) / sizeof(DPoint3d);
            }

        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadBlock(DPoint3d* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            bvector<uint8_t> ptData;
            size_t uncompressedSize = 0;
            m_smSQLiteFile->GetClipPolygon(blockID.m_integerID, ptData, uncompressedSize);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
            pi_compressedPacket.SetDataSize(ptData.size());
            pi_uncompressedPacket.SetDataSize(uncompressedSize);
            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
            memcpy(DataTypeArray, pi_uncompressedPacket.GetBufferAddress(), std::min(uncompressedSize, maxCountData*sizeof(DPoint3d)));
            return std::min(uncompressedSize, maxCountData*sizeof(DPoint3d));
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

        void SetMetadata(uint64_t id, double importance, int nDimensions)
            {
            m_smSQLiteFile->SetClipPolygonMetadata(id, importance, nDimensions);
            }

        void GetMetadata(uint64_t id, double& importance, int& nDimensions)
            {
            m_smSQLiteFile->GetClipPolygonMetadata(id, importance, nDimensions);
            }

        void GetAllIDs(bvector<uint64_t>& allIds)
            {
            m_smSQLiteFile->GetAllClipIDs(allIds);
            }

        SMSQLiteFile* GetFile() { return m_smSQLiteFile.get(); }

    private:
        SMSQLiteFilePtr m_smSQLiteFile;
    };