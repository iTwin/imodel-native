#pragma once



#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <Mtg/MtgStructs.h>

class DTMGraphTileStore : public IHPMPermanentStore<MTGGraph, Byte, Byte>
    {
    public:

        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        DTMGraphTileStore(WCharCP filename, const IDTMFile::AccessMode& accessMode, size_t layerID)
            {
            m_receivedOpenedFile = false;

            m_DTMFile = IDTMFile::File::Open(filename, accessMode);

            if (m_DTMFile == NULL)
                throw;

            m_layerID = layerID;

            }


        DTMGraphTileStore(IDTMFile::File::Ptr openedDTMFile, size_t layerID)
            :
            m_layerID(layerID),
            m_DTMFile(openedDTMFile),
            m_receivedOpenedFile(true)
            {
            if (m_DTMFile == NULL)
                throw;
            }

        virtual ~DTMGraphTileStore()
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

        virtual bool StoreMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return false;

            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            if (NULL == m_tileHandler)
                {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);

                if (NULL == layerDir)
                    {
                    layerDir = m_DTMFile->GetRootDir()->AddLayerDir();
                    HASSERT(NULL != layerDir);
                    if (NULL == layerDir)
                        return false;

                    m_layerID = layerDir->GetIndex();
                    HASSERT(0 == m_layerID);
                    }

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(GRAPH_FEATURE_TYPE);
                if (NULL == featureDir)
                    {

                    // No Point dir ... we create one
                    //NEEDS_WORK_SM: Why is this a feature dir (and is it necessary for storing packets at all?)
                    featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(GRAPH_FEATURE_TYPE,
                                                                             IDTMFile::PointTypeIDTrait<int32_t>::value,
                                                                             HTGFF::Compression::Deflate::Create());

                    HASSERT(NULL != featureDir);
                    if (NULL == featureDir)
                        return false;
                    }


                m_tileHandler = IDTMFile::PointTileHandler<int32_t>::CreateFrom(featureDir->GetPointDir());
                }
                /*

                m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
                if (NULL == m_indexHandler)
                    {
                    m_indexHandler = IndexHandler::CreateFrom(featureDir->CreateSpatialIndexDir(IndexHandler::Options(true, // TDORAY: Set correct value for is progressive
                        indexHeader->m_SplitTreshold)));
                    HASSERT(NULL != m_indexHandler);
                    if (NULL == m_indexHandler)
                        return false;
                    }

                m_filteringDir = featureDir->GetFilteringDir();
                if (NULL == m_filteringDir)
                    {
                    // TDORAY: Match real filter type. Ask alain where to find this information.
                    m_filteringDir = featureDir->CreateFilteringDir(IDTMFile::DumbFilteringHandler::Options());
                    HASSERT(NULL != m_filteringDir);
                    if (NULL == m_filteringDir)
                        return false;
                    }
                }
            HASSERT(NULL != m_tileHandler);
            HASSERT(NULL != m_indexHandler);
            HASSERT(NULL != m_filteringDir);

            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            m_indexHandler->SetBalanced(indexHeader->m_balanced);


            if (indexHeader->m_rootNodeBlockID.m_integerInitialized)
                m_indexHandler->SetTopNode(ConvertBlockID(indexHeader->m_rootNodeBlockID));
            else
                m_indexHandler->SetTopNode(IDTMFile::GetNullNodeID());*/


            return true;
            }

        virtual size_t LoadMasterHeader(Byte* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return 0;
            if (NULL == m_tileHandler)
                {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);
                if (NULL == layerDir)
                    return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(GRAPH_FEATURE_TYPE);
                if (NULL == featureDir)
                    return 0;

                m_tileHandler = IDTMFile::PointTileHandler<int32_t>::CreateFrom(featureDir->GetPointDir());
                }
            return 1;
           /* if (NULL == m_tileHandler)
                {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);
                if (NULL == layerDir)
                    return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(MASS_POINT_FEATURE_TYPE);
                if (NULL == featureDir)
                    return 0;

                m_tileHandler = TileHandler::CreateFrom(featureDir->GetPointDir());

                m_indexHandler = IndexHandler::CreateFrom(featureDir->GetSpatialIndexDir());
                if (NULL == m_indexHandler)
                    return 0;

                m_filteringDir = featureDir->GetFilteringDir();
                if (NULL == m_filteringDir)
                    return 0;

                }
            HASSERT(NULL != m_tileHandler);
            HASSERT(NULL != m_indexHandler);
            HASSERT(NULL != m_filteringDir);

            // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
            indexHeader->m_SplitTreshold = m_indexHandler->GetSplitTreshold();
            indexHeader->m_balanced = m_indexHandler->IsBalanced();




            if (m_indexHandler->GetTopNode() != IDTMFile::GetNullNodeID())
                indexHeader->m_rootNodeBlockID = m_indexHandler->GetTopNode();
            else
                indexHeader->m_rootNodeBlockID = HPMBlockID();

            return headerSize;*/
            }

        // New interface

        virtual HPMBlockID StoreNewBlock(MTGGraph* DataTypeArray, size_t countData)
            {
            HPRECONDITION(m_tileHandler != NULL);
            void** serializedGraph = new void*[countData];
            size_t countAsPts = 0;
            size_t countAsBytes = 0;
            size_t* ct = new size_t[countData];
            for (size_t i = 0; i < countData; i++)
                {
                ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedGraph[i]);
                countAsBytes += ct[i];
                }
            countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
            int32_t* ptArray = new int32_t[countAsPts + countData + 2];
            ptArray[0] = (int32_t)countData;
            size_t offset = 2 * sizeof(int32_t);
            for (size_t i = 0; i < countData; i++)
                {
                ptArray[(size_t)(ceil(((float)offset / sizeof(int32_t))))] = (int32_t)ct[i];
                offset += sizeof(int32_t);
                memcpy((char*)ptArray + offset, serializedGraph[i], ct[i]);
                offset += ct[i];
                free(serializedGraph[i]);
                }
            ptArray[1] = (int32_t)offset;
            delete[] serializedGraph;
            delete[] ct;
            IDTMFile::PointTileHandler<int32_t>::PointArray arrayOfPoints(ptArray, countAsPts);

            IDTMFile::NodeID newNodeID;
            if (!m_tileHandler->AddPoints(newNodeID, arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
                }
            delete[] ptArray;
            return HPMBlockID(newNodeID);
            }

        virtual HPMBlockID StoreBlock(MTGGraph* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            //HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::VariableSubNodesTable::GetNoSubNodeID())
                return StoreNewBlock(DataTypeArray, countData);

            void** serializedGraph = new void*[countData];
            size_t countAsPts =0;
            size_t countAsBytes = 0;
            size_t* ct = new size_t[countData];
            for (size_t i = 0; i < countData; i++)
                {
                ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedGraph[i]);
                countAsBytes += ct[i];
                }
            countAsPts = (size_t)(ceil((float)countAsBytes / sizeof(int32_t)));
            int32_t* ptArray = new int32_t[countAsPts+countData+2];
            ptArray[0] = (int32_t) countData;
            size_t offset = 2*sizeof(int32_t);
            for (size_t i = 0; i < countData; i++)
                {
                ptArray[(size_t)(ceil(((float)offset / sizeof(int32_t))))] = (int32_t) ct[i];
                offset += sizeof(int32_t);
                memcpy((char*)ptArray + offset, serializedGraph[i], ct[i]);
                offset += ct[i];
                free(serializedGraph[i]);
                }
            ptArray[1] = (int32_t) offset;
            delete[] serializedGraph;
            delete[] ct;
            IDTMFile::PointTileHandler<int32_t>::PointArray arrayOfPoints(ptArray, countAsPts);
           // PointArray arrayOfPoints(DataTypeArray, countData);

            if (!m_tileHandler->SetPoints(ConvertBlockID(blockID), arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
                }
            delete[] ptArray;
            return blockID;
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
           //Currently only support the one graph. Obtaining the number of elements for non-POD types using only the byte count of the packet is not possible,
           //and we're not supporting storing an actual number of elements as packet metadata right now.
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
            HPRECONDITION(m_tileHandler != NULL);

            IDTMFile::PointTileHandler<int32_t>::PointArray arrayOfPoints;
            //get size of packet in number of ints
            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            int32_t* results = new int32_t[packetSize];
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw; //TDORAY: Do something else*/
            if (results[0] > 0 && maxCountData > 0)
                {
                if (results[2] > 0)
                    {
                    //The pooled vectors don't initialize the memory they allocate. For complex datatypes with some logic in the constructor (like bvector),
                    //this leads to undefined behavior when using the object. So we call the constructor on the allocated memory from the pool right here using placement new.
                    MTGGraph * graph = new(DataTypeArray) MTGGraph();
                    graph->LoadFromBinaryStream(results + 3, (size_t)results[2]);
                    }
                }
            delete[] results;
            return packetSize;
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

             return m_tileHandler->RemovePoints(ConvertBlockID(blockID));
            }

        static const IDTMFile::FeatureType GRAPH_FEATURE_TYPE = 1;

    protected:
        const IDTMFile::File::Ptr& GetFileP() const
            {
            return m_DTMFile;
            }

    private:
        IDTMFile::File::Ptr m_DTMFile;
        HFCPtr<IDTMFile::PointTileHandler<int32_t>> m_tileHandler;
        bool m_receivedOpenedFile;
        size_t m_layerID;
    };