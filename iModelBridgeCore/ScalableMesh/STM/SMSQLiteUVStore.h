
#include <ImagePP/all/h/HPMDataStore.h>
#include "SMPointTileStore.h"
#include "SMSQLiteFile.h"


template <class EXTENT> class SMSQLiteUVTileStore : public SMPointTileStore<DPoint2d, EXTENT>
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
        /*SMSQLiteUVTileStore(BENTLEY_NAMESPACE_NAME::WString filename, const ISMStore::AccessMode& accessMode)
            {
            m_smSQLiteFile = SMSQLiteFile::Create();
            Utf8String filenameA;
            BeStringUtilities::WCharToUtf8(filenameA, filename.c_str());
            if (accessMode.m_HasCreateAccess)
                m_smSQLiteFile->Create(filenameA.c_str());
            else
                m_smSQLiteFile->Open(filenameA.c_str());
            }*/

        SMSQLiteUVTileStore(SMSQLiteFilePtr file)
            {
            m_smSQLiteFile = file;
            }

        virtual ~SMSQLiteUVTileStore()
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

        virtual HPMBlockID StoreNewBlock(DPoint2d* DataTypeArray, size_t countData)
            {
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DPoint2d));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(DPoint2d));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            m_smSQLiteFile->StoreUVs(id, indexData, countData*sizeof(DPoint2d));
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(DPoint2d* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_uncompressedPacket.SetBuffer(DataTypeArray, countData*sizeof(DPoint2d));
            pi_uncompressedPacket.SetDataSize(countData*sizeof(DPoint2d));
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket);
            bvector<uint8_t> indexData(pi_compressedPacket.GetDataSize());
            memcpy(&indexData[0], pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            int64_t id = blockID.m_integerID;
            m_smSQLiteFile->StoreUVs(id, indexData, countData*sizeof(DPoint2d));
            return HPMBlockID(id);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return m_smSQLiteFile->GetNumberOfUVs(blockID.m_integerID) / sizeof(DPoint2d);
            }

        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            return 0;
            }

        virtual size_t LoadBlock(DPoint2d* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            bvector<uint8_t> ptData;
            size_t uncompressedSize = 0;
            m_smSQLiteFile->GetUVs(blockID.m_integerID, ptData, uncompressedSize);
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_compressedPacket.SetBuffer(&ptData[0], ptData.size());
            pi_compressedPacket.SetDataSize(ptData.size());
            pi_uncompressedPacket.SetDataSize(uncompressedSize);
            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket);
            memcpy(DataTypeArray, pi_uncompressedPacket.GetBufferAddress(), std::min(uncompressedSize, maxCountData*sizeof(DPoint2d)));
            return std::min(uncompressedSize, maxCountData*sizeof(DPoint2d));
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

    private:
        SMSQLiteFilePtr m_smSQLiteFile;
    };