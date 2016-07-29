#pragma once
#include <ImagePP/all/h/HPMDataStore.h>
#include "SMPointTileStore.h"

#include "SMSQLiteFile.h"
#include "Edits\DifferenceSet.h"

#undef min
#undef max

class SMSQLiteDiffsetTileStore : public IScalableMeshDataStore<DifferenceSet, Byte, Byte>
    {
    private:
        
    public:

        static ISMStore::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<ISMStore::NodeID>(blockID.m_integerID);
            }
        virtual void Close()
            {
                if (m_smSQLiteFile != NULL)
                    m_smSQLiteFile->Close();

                m_smSQLiteFile = NULL;
            }
        void Open()
            {
            std::lock_guard<std::mutex> lock(fileLock);
            if (m_smSQLiteFile != NULL && m_smSQLiteFile->IsOpen()) return;
            StatusInt status;
            m_smSQLiteFile = SMSQLiteFile::Open(m_path, false, status);
                if (m_needsCreate || !status || !m_smSQLiteFile->IsOpen())
                    {
                    m_needsCreate = true;
                    m_smSQLiteFile->Create(m_path);
                    }
            }


        SMSQLiteDiffsetTileStore(WString& path, bool createFile = false)
            {
            m_path = path;
            m_needsCreate = createFile;
            }

        virtual ~SMSQLiteDiffsetTileStore()
            {
            //m_smSQLiteFile->Close();
            }


        virtual bool StoreMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            return false;
            }

        virtual size_t LoadMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen())
                if (!m_needsCreate)
                    Open();
                else return 0;
                if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen()) return 0;
            return 0;
            }
        int32_t* Serialize(size_t& countAsPts, DifferenceSet* DataTypeArray, size_t countData)
            {
            void** serializedSet = new void*[countData];
            countAsPts = 0;
            size_t countAsBytes = 0;
            size_t* ct = new size_t[countData];

            for (size_t i = 0; i < countData; i++)
                {
                ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedSet[i]);
                countAsBytes += ct[i];
                countAsPts += (size_t)(ceil((float)ct[i] / sizeof(int32_t)));
                }
            //countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
            size_t nOfInts = (size_t)(ceil(((float)sizeof(size_t) / sizeof(int32_t))));
            int32_t* ptArray = new int32_t[countAsPts + countData + nOfInts];
            memcpy(ptArray, &countData, sizeof(size_t));
            size_t offset = sizeof(size_t);
            for (size_t i = 0; i < countData; i++)
                {
                ptArray[(size_t)(ceil(((float)offset / sizeof(int32_t))))] = (int32_t)ct[i];
                offset = (size_t)(ceil(((float)offset / sizeof(int32_t))))*sizeof(int32_t);
                offset += sizeof(int32_t);
                memcpy((char*)ptArray + offset, serializedSet[i], ct[i]);
                offset += ct[i];
                free(serializedSet[i]);
                }
            delete[] serializedSet;
            delete[] ct;
            return ptArray;
            }
        // New interface

        virtual HPMBlockID StoreNewBlock(DifferenceSet* DataTypeArray, size_t countData)
            {
            if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen())
                Open();
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            size_t countAsPts;
            int32_t * ptArray = Serialize(countAsPts, DataTypeArray, countData);
            bvector<uint8_t> diffsetData(countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t));
            memcpy(&diffsetData[0], ptArray, countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t));
            int64_t id = SQLiteNodeHeader::NO_NODEID;
            m_smSQLiteFile->StoreDiffSet(id, diffsetData, countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t));
            delete[] ptArray;
            return HPMBlockID(id);
            }

        virtual HPMBlockID StoreBlock(DifferenceSet* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen())
                Open();
            if (!blockID.IsValid()) return StoreNewBlock(DataTypeArray, countData);
            size_t countAsPts;
            int32_t * ptArray = Serialize(countAsPts, DataTypeArray, countData);
            bvector<uint8_t> diffsetData(countAsPts*sizeof(int)+countData*sizeof(int)+sizeof(size_t));
            memcpy(&diffsetData[0], ptArray, countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t));
            int64_t id = blockID.m_integerID;
            m_smSQLiteFile->StoreDiffSet(id, diffsetData, countAsPts*sizeof(int) + countData*sizeof(int) + sizeof(size_t));
            delete[] ptArray;
            return HPMBlockID(id);
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen())
                const_cast<SMSQLiteDiffsetTileStore*>(this)->Open();
            bvector<uint8_t> diffsetData;
            size_t uncompressedSize = 0;
            m_smSQLiteFile->GetDiffSet(blockID.m_integerID, diffsetData, uncompressedSize);
            if (uncompressedSize == 0) return 0;
            size_t dataCount = 0;
            memcpy(&dataCount, &diffsetData[0], sizeof(size_t));
            return dataCount;
            }


        virtual size_t StoreNodeHeader(Byte* header, HPMBlockID blockID)
            {
            return 1;
            }

        virtual size_t LoadNodeHeader(Byte* header, HPMBlockID blockID)
            {

            return 1;
            }

        virtual size_t LoadBlock(DifferenceSet* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            if (m_smSQLiteFile == NULL || !m_smSQLiteFile->IsOpen())
                Open();
            bvector<uint8_t> diffsetData;
            size_t uncompressedSize = 0;
            m_smSQLiteFile->GetDiffSet(blockID.m_integerID, diffsetData, uncompressedSize);
            if (diffsetData.size() == 0) return 0;
            size_t dataCount = 0;
            memcpy(&dataCount, &diffsetData[0], sizeof(size_t));
            if (dataCount > 0 && maxCountData > 0)
                {
                size_t offset = (size_t)ceil(sizeof(size_t));
                size_t ct = 0;
                while (offset + 1 < diffsetData.size() && *((int32_t*)&diffsetData[offset]) > 0 && ct < dataCount)
                    {
                    //The pooled vectors don't initialize the memory they allocate. For complex datatypes with some logic in the constructor (like bvector),
                    //this leads to undefined behavior when using the object. So we call the constructor on the allocated memory from the pool right here using placement new.
                    DifferenceSet * diffSet = new(DataTypeArray + ct)DifferenceSet();
                    size_t sizeOfCurrentSerializedSet = (size_t)*((int32_t*)&diffsetData[offset]);
                    diffSet->LoadFromBinaryStream(&diffsetData[0] + offset + sizeof(int32_t), sizeOfCurrentSerializedSet);
                    diffSet->upToDate = true;
                    offset += sizeof(int32_t);
                    offset += sizeOfCurrentSerializedSet;
                    offset = ceil(((float)offset / sizeof(int32_t)))*sizeof(int32_t);
                    ++ct;
                    }
                }
            return diffsetData.size();
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return false;
            }

    protected:

    private:
        SMSQLiteFilePtr m_smSQLiteFile;
        WString m_path;
        bool m_needsCreate;
        std::mutex fileLock;
    };