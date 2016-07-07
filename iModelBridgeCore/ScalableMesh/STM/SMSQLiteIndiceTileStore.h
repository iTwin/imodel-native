#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include "SMPointTileStore.h"
#include "SMSQLiteFile.h"


template <class EXTENT> class SMSQLiteIndiceTileStore : public SMPointTileStore<int32_t, EXTENT>
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
            /*pi_uncompressedPacket.SetBufferOwnership(true);
            pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));*/
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

        SMSQLiteIndiceTileStore(SMSQLiteFilePtr file)
            {
            m_smSQLiteFile = file;
            }

        virtual ~SMSQLiteIndiceTileStore()
            {
            //m_smSQLiteFile->Close();
            }

        virtual bool HasSpatialReferenceSystem()
            {
            // if spatialreference for scm return true
            return false;
            }

        virtual std::string GetSpatialReferenceSystem()
            {
            // return string for GCS
            return string();
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

        virtual HPMBlockID StoreNewBlock(int32_t* DataTypeArray, size_t countData)
            {
            std::string s;
            s = " DATA HAS " + std::to_string(countData) + " INDICES ";
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(int32_t));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(int32_t));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            m_smSQLiteFile->StoreIndices(id, indexData, countData*sizeof(int32_t));
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(int32_t* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(int32_t));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(int32_t));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = blockID.m_integerID;
            m_smSQLiteFile->StoreIndices(id, indexData, countData*sizeof(int32_t));
            return HPMBlockID(id);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return m_smSQLiteFile->GetNumberOfIndices(blockID.m_integerID) / sizeof(int32_t);
            }

        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadBlock(int32_t* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            bvector<uint8_t> ptData;
            size_t uncompressedSize = 0;
            m_smSQLiteFile->GetIndices(blockID.m_integerID, ptData, uncompressedSize);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
            pi_compressedPacket.SetDataSize(ptData.size());
            pi_uncompressedPacket.SetBuffer(DataTypeArray, maxCountData*sizeof(int32_t));
            pi_uncompressedPacket.SetBufferOwnership(false);
            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
            //memcpy(DataTypeArray, pi_uncompressedPacket.GetBufferAddress(), std::min(uncompressedSize, maxCountData*sizeof(int32_t)));
            return std::min(uncompressedSize, maxCountData*sizeof(int32_t));
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

    private:
        SMSQLiteFilePtr m_smSQLiteFile;
    };