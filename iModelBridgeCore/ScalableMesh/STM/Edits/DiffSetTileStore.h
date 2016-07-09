/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/DiffSetTileStore.h $
|    $RCSfile: DiffSetTileStore.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/08 13:06:11 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#pragma once


#include <ScalableMesh/IScalableMeshDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/ISMStore.h>
#include <TerrainModel/TerrainModel.h>
#include "../SMPointTileStore.h"
#include "DifferenceSet.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class DiffSetTileStore : public IScalableMeshDataStore < DifferenceSet, Byte, Byte >
    {
    public:
        static const ISMStore::FeatureType MASS_POINT_FEATURE_TYPE = 0;
        static ISMStore::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<ISMStore::NodeID>(blockID.m_integerID);
            }

        DiffSetTileStore(ISMStore::File::Ptr openedDTMFile, size_t layerID)
            :
            m_layerID(layerID),
            m_DTMFile(openedDTMFile),
            m_receivedOpenedFile(true)
            {
            /*if (m_DTMFile == NULL)
                throw;*/assert(m_DTMFile != NULL);
            }

        DiffSetTileStore(WString& path, size_t layerID, bool createFile=false)
            :
            m_layerID(layerID),
            m_receivedOpenedFile(false),
            m_path(path),
            m_DTMFile(NULL),
            m_needsCreate(createFile)
            {

            }

        virtual ~DiffSetTileStore()
            {
            if (!m_receivedOpenedFile)
                if (m_DTMFile != NULL)
                    m_DTMFile->Close();

            m_DTMFile = NULL;
            }

        virtual void Close()
            {
            if (!m_receivedOpenedFile)
                if (m_DTMFile != NULL)
                    m_DTMFile->Close();

            m_DTMFile = NULL;
            }
        void Open()
            {
            if (m_needsCreate) m_DTMFile = ISMStore::File::Create(m_path.c_str());
            else m_DTMFile = ISMStore::File::Open(m_path.c_str());
            if (m_DTMFile == NULL) m_needsCreate = true;
            }

        virtual bool StoreMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return false;//Open();

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (NULL == m_tileHandler)
                {
                ISMStore::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);

                if (NULL == layerDir)
                    {
                    layerDir = m_DTMFile->GetRootDir()->AddLayerDir();
                    HASSERT(NULL != layerDir);
                    if (NULL == layerDir)
                        return false;

                    m_layerID = layerDir->GetIndex();
                    HASSERT(0 == m_layerID);
                    }

                ISMStore::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
                if (NULL == featureDir)
                    {

                    featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(MASS_POINT_FEATURE_TYPE,
                                                                             ISMStore::PointTypeIDTrait<int32_t>::value,
                                                                             HTGFF::Compression::None::Create());

                    HASSERT(NULL != featureDir);
                    if (NULL == featureDir)
                        return false;
                    }


                m_tileHandler = ISMStore::PointTileHandler<int32_t>::CreateFrom(featureDir->GetPointDir());
                }


            return true;
            }

        virtual size_t LoadMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                if (!m_needsCreate)
                Open();
                else return 0;
            if (m_DTMFile == NULL) return 0;
            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (NULL == m_tileHandler)
                {
                ISMStore::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);
                if (NULL == layerDir)
                    return 0;

                ISMStore::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
                if (NULL == featureDir)
                    return 0;

                m_tileHandler = ISMStore::PointTileHandler<int32_t>::CreateFrom(featureDir->GetPointDir());
                }
            return 1;
            }

        // New interface

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
                }
            countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
            size_t nOfInts = (size_t)(ceil(((float)sizeof(size_t) / sizeof(int32_t))));
            int32_t* ptArray = new int32_t[countAsPts + countData + nOfInts];
            memcpy(ptArray,&countData, sizeof(size_t));
            size_t offset = sizeof(size_t);
            for (size_t i = 0; i < countData; i++)
                {
                ptArray[(size_t)(ceil(((float)offset / sizeof(int32_t))))] = (int32_t)ct[i];
                offset += sizeof(int32_t);
                memcpy((char*)ptArray + offset, serializedSet[i], ct[i]);
                offset += ct[i];
                free(serializedSet[i]);
                }
            delete[] serializedSet;
            delete[] ct;
            return ptArray;
            }

        virtual HPMBlockID StoreNewBlock(DifferenceSet* DataTypeArray, size_t countData)
            {
            if (m_DTMFile == NULL)
                Open();
            HPRECONDITION(m_tileHandler != NULL);
            HPRECONDITION(m_DTMFile != NULL);
            size_t countAsPts;
            int32_t * ptArray = Serialize(countAsPts, DataTypeArray, countData);
            ISMStore::PointTileHandler<int32_t>::PointArray arrayOfPoints(ptArray, countAsPts);

            ISMStore::NodeID newNodeID;

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (!m_tileHandler->AddPoints(newNodeID, arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... ISMStore::File API should be modified.");
                }
            delete[] ptArray;
            return HPMBlockID(newNodeID);
            }

        virtual HPMBlockID StoreBlock(DifferenceSet* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            if (m_DTMFile == NULL)
                Open();
            //HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            if (!blockID.IsValid() || blockID.m_integerID == ISMStore::SubNodesTable::GetNoSubNodeID())
                return StoreNewBlock(DataTypeArray, countData);
            size_t countAsPts;
            int32_t * ptArray = Serialize(countAsPts, DataTypeArray, countData);
            ISMStore::PointTileHandler<int32_t>::PointArray arrayOfPoints(ptArray, countAsPts);
            // PointArray arrayOfPoints(DataTypeArray, countData);

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (!m_tileHandler->SetPoints(ConvertBlockID(blockID), arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... ISMStore::File API should be modified.");
                }
            delete[] ptArray;
            return blockID;
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            if (m_DTMFile == NULL)
                const_cast<DiffSetTileStore*>(this)->Open();
            HPRECONDITION(m_tileHandler != NULL);

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            ISMStore::PointTileHandler<int32_t>::PointArray arrayOfPoints;
            //get size of packet in number of ints
            //size_t nOfInts = (size_t)ceil((float)sizeof(size_t) / sizeof(int32_t));
            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            int32_t* results = new int32_t[packetSize];
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw;
            size_t dataCount = 0;
            memcpy(&dataCount, results, sizeof(size_t));
            delete[] results;
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
            if (m_DTMFile == NULL)
                Open();
            HPRECONDITION(m_tileHandler != NULL);

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());
            std::string s;
            ISMStore::PointTileHandler<int32_t>::PointArray arrayOfPoints;
            //get size of packet in number of ints
            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            int32_t* results = new int32_t[packetSize];
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw; //TDORAY: Do something else*/
            size_t dataCount = 0;
            memcpy(&dataCount, results, sizeof(size_t));
            s += " results packet size " + std::to_string(packetSize) + " data count " + std::to_string(dataCount)+"\n";
            if (packetSize > 0 && maxCountData > 0)
                {
                size_t offset = (size_t)ceil(sizeof(size_t) / sizeof(int32_t));
                size_t ct = 0;
                s += "offset " + std::to_string(offset);
                while (results[offset] > 0 && ct < dataCount && offset + 1 < packetSize)
                    {
                    //The pooled vectors don't initialize the memory they allocate. For complex datatypes with some logic in the constructor (like bvector),
                    //this leads to undefined behavior when using the object. So we call the constructor on the allocated memory from the pool right here using placement new.
                    DifferenceSet * diffSet = new(DataTypeArray+ct)DifferenceSet();
                    diffSet->LoadFromBinaryStream(results + offset + 1, (size_t)results[offset]);
                    offset++;
                    offset += ceil(((float)results[offset])/sizeof(int32_t));
                    ++ct;
                    s += "offset " + std::to_string(offset) + " ct "+std::to_string(ct);
                    }
                }
            delete[] results;
            return packetSize;
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            if (m_DTMFile == NULL)
                Open();
            HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            return m_tileHandler->RemovePoints(ConvertBlockID(blockID));
            }

        static const ISMStore::FeatureType GRAPH_FEATURE_TYPE = 1;

    protected:
        const ISMStore::File::Ptr& GetFileP() const
            {
            return m_DTMFile;
            }

    private:
        ISMStore::File::Ptr m_DTMFile;
        HFCPtr<ISMStore::PointTileHandler<int32_t>> m_tileHandler;
        bool m_receivedOpenedFile;
        size_t m_layerID;
        WString m_path;
        bool m_needsCreate;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE