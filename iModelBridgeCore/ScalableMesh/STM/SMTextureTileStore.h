#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
//#include <Mtg/MtgStructs.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>

class TextureTileStore : public IHPMPermanentStore<Byte, float, float> // JPEGData (Byte*), size
    {
    public:

        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        TextureTileStore(WCharCP filename, const IDTMFile::AccessMode& accessMode, size_t layerID)
            {
            m_receivedOpenedFile = false;

            m_DTMFile = IDTMFile::File::Open(filename, accessMode);

            if (m_DTMFile == NULL)
                throw;

            m_layerID = layerID;

            }


        TextureTileStore(IDTMFile::File::Ptr openedDTMFile, size_t layerID)
            :
            m_layerID(layerID),
            m_DTMFile(openedDTMFile),
            m_receivedOpenedFile(true)
            {
            if (m_DTMFile == NULL)
                throw;
            }

        virtual ~TextureTileStore()
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

        virtual bool StoreMasterHeader(float* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return false;

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());
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
                    //HASSERT(0 == m_layerID);
                    }

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(TEXTURE_FEATURE_TYPE);
                if (NULL == featureDir)
                    {

                    // No Point dir ... we create one
                    //NEEDS_WORK_SM: Why is this a feature dir (and is it necessary for storing packets at all?)
                    // Texture already compressed => arg 3 need to be changed
                    featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(TEXTURE_FEATURE_TYPE,
                                                                             IDTMFile::PointTypeIDTrait<Byte>::value,
                                                                             HTGFF::Compression::Deflate::Create());

                    HASSERT(NULL != featureDir);
                    if (NULL == featureDir)
                        return false;
                    }


                m_tileHandler = IDTMFile::PointTileHandler<Byte>::CreateFrom(featureDir->GetPointDir());
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

        virtual size_t LoadMasterHeader(float* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return 0;

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (NULL == m_tileHandler)
                {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);
                if (NULL == layerDir)
                    return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(TEXTURE_FEATURE_TYPE);
                if (NULL == featureDir)
                    return 0;

                m_tileHandler = IDTMFile::PointTileHandler<Byte>::CreateFrom(featureDir->GetPointDir());
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

        bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
            HCDPacket& pi_compressedPacket, int width, int height, int nOfChannels = 3)
        {
            HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<UInt32>::max) ());

            // initialize codec
            auto codec = new HCDCodecIJG(width, height, 8*nOfChannels);
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
            HPRECONDITION(m_tileHandler != NULL);
            
            Byte* ptArray = new Byte[countData];
            
            int width, height, nOfChannels;
            memcpy(ptArray, DataTypeArray, countData);

            memcpy(&width, ptArray, sizeof(int));
            memcpy(&height, (int*)ptArray + 1, sizeof(int));
            memcpy(&nOfChannels, (int*)ptArray + 2, sizeof(int));

            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            size_t sizeTmp = countData - 3 * sizeof(int);
            Byte* dataArrayTmp = new Byte[sizeTmp];
            memcpy(dataArrayTmp, (int*)ptArray+3, sizeTmp);
            pi_uncompressedPacket.SetBuffer(dataArrayTmp, sizeTmp);
            pi_uncompressedPacket.SetDataSize(sizeTmp);
            WriteCompressedPacket(pi_uncompressedPacket, pi_compressedPacket, width, height, nOfChannels);
            size_t countAsPts = pi_compressedPacket.GetDataSize()+3*sizeof(int);
            //countAsPts = (size_t(ceil((float)countData / sizeof(Byte))));
            Byte* result = new Byte[countAsPts];
            memcpy(result, &width, sizeof(int));
            memcpy((int*)result+1, &height, sizeof(int));
            memcpy((int*)result + 2, &nOfChannels, sizeof(int));
            memcpy((int*)result + 3, pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize());
            /*
            void** serializedTexture = new void*[countData];
            size_t countAsPts = 0;
            size_t countAsBytes = 0;
            size_t* ct = new size_t[countData];

            

            m_compressedData.resize (dataSize);
            memcpy (&m_compressedData.front(), pData, dataSize);
            readRGBFromJPEGData (m_data, m_size, pData, dataSize);

            for (size_t i = 0; i < countData; i++)
                {
                ct[i] = DataTypeArray[i].WriteToBinaryStream(serializedTexture[i]);
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
                memcpy((char*)ptArray + offset, serializedTexture[i], ct[i]);
                offset += ct[i];
                free(serializedTexture[i]);
                }
            ptArray[1] = (int32_t)offset;
            delete[] serializedTexture;
            delete[] ct;*/
            //IDTMFile::PointTileHandler<Byte>::PointArray arrayOfPoints(ptArray, countAsPts);
            IDTMFile::PointTileHandler<Byte>::PointArray arrayOfPoints(result, countAsPts);
            
            IDTMFile::NodeID newNodeID;

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

            if (!m_tileHandler->AddPoints(newNodeID, arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
                }

#ifdef ACTIVATE_TEXTURE_DUMP
         /*   WString fileName = L"file://";
            fileName.append(L"e:\\output\\scmesh\\2015-11-19\\texture_store_");
            fileName.append(std::to_wstring(newNodeID).c_str());
            fileName.append(L".bmp");
            HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
            HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
            byte* pixelBuffer = new byte[1024 * 1024 * 3];
            size_t t = 0;
            for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
                {
                pixelBuffer[t] = *(pi_uncompressedPacket.GetBufferAddress() + i);
                pixelBuffer[t + 1] = *(pi_uncompressedPacket.GetBufferAddress()  + i + 1);
                pixelBuffer[t + 2] = *(pi_uncompressedPacket.GetBufferAddress()  + i + 2);
                t += 3;
                }
            HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                      1024,
                                                      1024,
                                                      pImageDataPixelType,
                                                      pixelBuffer);
            delete[] pixelBuffer;*/
#endif
            delete[] ptArray;
            delete[] result;
            delete[] dataArrayTmp;
            return HPMBlockID(newNodeID);
            }

        virtual HPMBlockID StoreBlock(Byte* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            //HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
                return StoreNewBlock(DataTypeArray, countData);


            Byte* ptArray = new Byte[countData];

            memcpy(ptArray, DataTypeArray, countData);
            size_t countAsPts = 0;
            countAsPts = (size_t(ceil((float)countData / sizeof(Byte))));
            /*void** serializedGraph = new void*[countData];
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
            delete[] ct;*/
            IDTMFile::PointTileHandler<Byte>::PointArray arrayOfPoints(ptArray, countAsPts);
           // PointArray arrayOfPoints(DataTypeArray, countData);
           
            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

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
            HPRECONDITION(m_tileHandler != NULL);
            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());
            //return 1;

            size_t size;

            IDTMFile::PointTileHandler<Byte>::PointArray arrayOfPoints;
            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            //size_t packetSize = 2 * sizeof(int);
            Byte* results = new Byte[packetSize];
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw;

            int height, width, nOfChannels;
            memcpy(&width, results, sizeof(int));
            memcpy(&height, (int*)results + 1, sizeof(int));
            memcpy(&nOfChannels, (int*)results + 2, sizeof(int));
            size = height*width * nOfChannels;

            delete[] results;
            return size;
            }


        virtual size_t StoreHeader(float* header, HPMBlockID blockID)
            {
            return 1;
            }

        virtual size_t LoadHeader(float* header, HPMBlockID blockID)
            {

            return 1;
            }

        bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket, HCDPacket& pi_uncompressedPacket, size_t width, size_t height, size_t nOfChannels=3)
        {
            HPRECONDITION(pi_compressedPacket.GetDataSize() <= (numeric_limits<UInt32>::max)());

            // initialize codec
            //HFCPtr<HCDCodec> pCodec = new HCDCodecImage("CodecName", piWidth, piHeight, piBitPerPixel);// (pi_compressedPacket.GetDataSize());
            //HFCPtr<HCDCodec> pCodec = new HCDCodecJPEG();
            /*
            union MyUnion
            {
                UInt32 m_IntValue;
                Byte m_ByteValue[4];
            } SubType;
            UInt32 Type;

            Type = ((UINT32*) pi_compressedPacket)*/

            //pi_compressedPacket.GetCodec();
            /*
            HFCPtr<HCDCodec> pCodec = (HCDCodec*)pi_compressedPacket.GetCodec()->Clone();

            HCDCodecImage::SetCodecForImage(pCodec,
                blockWidth,
                blockHeight,
                bitsPerPixel);

            size_t MaxSubsetCompressed = pCodec->GetSubsetMaxCompressedSize();
            */
            auto codec = new HCDCodecIJG(width, height, nOfChannels * 8);// (pi_compressedPacket.GetDataSize()); // 24 bits per pixels
            codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
            codec->SetQuality(70);
            codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
            HFCPtr<HCDCodec> pCodec = codec;
            pi_uncompressedPacket.SetBufferOwnership(true);
            pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
            const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
            pi_uncompressedPacket.SetDataSize(compressedDataSize);

            // We decompress the specified line from the image buffer
            //HCDPacket uncompress(po_pData, m_WidthInByteToRead);
            /*HCDPacket compressSubset((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                m_CompressPacket.GetBufferAddress() + m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex());*/

//            HFCPtr<HCDCodec> pCodec;// = new HCDCodecImage();
//            HFCPtr<HCDPacket> packet = new HCDPacket(new HCDCodecIdentity(), 0, 0, 0);

            /*const size_t compressedDataSize = pi_compressedPacket.GetCodec()->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
            pi_uncompressedPacket.SetDataSize(compressedDataSize);*/
            //pi_compressedPacket.Decompress(&pi_uncompressedPacket);

            return true;
        }

        virtual size_t LoadBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HPRECONDITION(m_tileHandler != NULL);

         
            //Byte*dataArrayTmp = nullptr;

            //m_tileHandler->GetPoints(ConvertBlockID(blockID), dataArrayTmp
            
            IDTMFile::PointTileHandler<Byte>::PointArray arrayOfPoints;
            //get size of packet in number of ints

            m_DTMFile->GetFileAccessMutex().lock();

            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            Byte* results = new Byte[packetSize];
         //   std::string s;
           // s += "COMPRESSED SIZE LOAD FOR BLOCK " + std::to_string(blockID.m_integerID) + " : " + std::to_string(packetSize);

            
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw; //TDORAY: Do something else*/

            m_DTMFile->GetFileAccessMutex().unlock();

            int width;
            memcpy(&width, results, sizeof(int));
            int height;
            memcpy(&height, (int*)results+1, sizeof(int));
            int nOfChannels;
            memcpy(&nOfChannels, (int*)results + 2, sizeof(int));
            HCDPacket pi_uncompressedPacket, pi_compressedPacket;
            pi_compressedPacket.SetBuffer(results + 3 * sizeof(int), packetSize*sizeof(Byte) - 3 * sizeof(int));
            pi_compressedPacket.SetDataSize(packetSize*sizeof(Byte) - 3 * sizeof(int));
            pi_uncompressedPacket.SetDataSize(height * width * nOfChannels * sizeof(Byte)); // number of pixel image : height*width*pixelDepth
            //size_t width = 1, height = 1;

            LoadCompressedPacket(pi_compressedPacket, pi_uncompressedPacket, width, height, nOfChannels);
#ifdef ACTIVATE_TEXTURE_DUMP
          /*  WString fileName2;
            fileName2.append(L"e:\\output\\scmesh\\2015-11-19\\texture_compressed_load_");
            fileName2.append(std::to_wstring(blockID.m_integerID).c_str());
            fileName2.append(L".bin");
            FILE* binCompressed = _wfopen(fileName2.c_str(), L"wb");
            fwrite(results + 3 * sizeof(int),sizeof(byte),packetSize*sizeof(Byte) - 3 * sizeof(int), binCompressed);
            fclose(binCompressed);*/
           /* WString fileName = L"file://";
            fileName.append(L"e:\\output\\scmesh\\2015-11-19\\texture_load_");
            fileName.append(std::to_wstring(blockID.m_integerID).c_str());
            fileName.append(L".bmp");
            HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
            HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
            byte* pixelBuffer = new byte[1024 * 1024 * 3];
            size_t t = 0;
            for (size_t i = 0; i < 1024 * 1024 * 4; i += 4)
                {
                pixelBuffer[t] = *(pi_uncompressedPacket.GetBufferAddress() + i);
                pixelBuffer[t + 1] = *(pi_uncompressedPacket.GetBufferAddress() + i + 1);
                pixelBuffer[t + 2] = *(pi_uncompressedPacket.GetBufferAddress() + i + 2);
                t += 3;
                }
            HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                      1024,
                                                      1024,
                                                      pImageDataPixelType,
                                                      pixelBuffer);
            delete[] pixelBuffer;*/
#endif
            size_t size = pi_uncompressedPacket.GetDataSize() + 3 * sizeof(int);
            Byte* resultDecompressed = new Byte[size];
            memcpy(resultDecompressed, &width, sizeof(int));
            memcpy((int*)resultDecompressed + 1, &height, sizeof(int));
            memcpy((int*)resultDecompressed + 2, &nOfChannels, sizeof(int));
            memcpy(resultDecompressed + 3 * sizeof(int), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize());

            //memcpy(DataTypeArray, pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize());
            memcpy(DataTypeArray, resultDecompressed, size);

            // Get our internal copy for easier code reading and data manipulation.
            /*UInt32 widthInByteToRead = (UInt32)ceil((float)(m_pResolutionDescriptor->GetWidth()) * ((float)m_BitPerPixel / 8.0));

            HCDPacket uncompress(DataTypeArray, widthInByteToRead);

            HCDPacket compressSubset((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                m_CompressPacket.GetBufferAddress() + m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex());

            compressSubset.Decompress(&uncompress);*/

            delete[] results;
            delete[] resultDecompressed;
            //return packetSize;
            return size;
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            std::lock_guard<std::recursive_mutex> lck(m_DTMFile->GetFileAccessMutex());

             return m_tileHandler->RemovePoints(ConvertBlockID(blockID));
            }

        static const IDTMFile::FeatureType TEXTURE_FEATURE_TYPE = 1;

    protected:
        const IDTMFile::File::Ptr& GetFileP() const
            {
            return m_DTMFile;
            }

    private:
        IDTMFile::File::Ptr m_DTMFile;
        HFCPtr<IDTMFile::PointTileHandler<Byte>> m_tileHandler;
        bool m_receivedOpenedFile;
        size_t m_layerID;
    };


/*
template <class POINT> class UVTileStore : public IHPMPermanentStore<POINT, float, float> // POInt, size
    {
    public:

        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        UVTileStore(WCharCP filename, const IDTMFile::AccessMode& accessMode, size_t layerID)
            {
            m_receivedOpenedFile = false;

            m_DTMFile = IDTMFile::File::Open(filename, accessMode);

            if (m_DTMFile == NULL)
                throw;

            m_layerID = layerID;

            }


        UVTileStore(IDTMFile::File::Ptr openedDTMFile, size_t layerID)
            :
            m_layerID(layerID),
            m_DTMFile(openedDTMFile),
            m_receivedOpenedFile(true)
            {
            if (m_DTMFile == NULL)
                throw;
            }

        virtual ~UVTileStore()
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

        virtual bool StoreMasterHeader(float* indexHeader, size_t headerSize)
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

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(TEXTURE_FEATURE_TYPE);
                if (NULL == featureDir)
                    {

                    // No Point dir ... we create one
                    //NEEDS_WORK_SM: Why is this a feature dir (and is it necessary for storing packets at all?)
                    // Texture already compressed => arg 3 need to be changed
                    featureDir = layerDir->CreatePointsOnlyUniformFeatureDir(TEXTURE_FEATURE_TYPE,
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
/*

            return true;
            }

        virtual size_t LoadMasterHeader(float* indexHeader, size_t headerSize)
            {
            if (m_DTMFile == NULL)
                return 0;
            if (NULL == m_tileHandler)
                {
                IDTMFile::LayerDir* layerDir = m_DTMFile->GetRootDir()->GetLayerDir(m_layerID);
                if (NULL == layerDir)
                    return 0;

                IDTMFile::UniformFeatureDir* featureDir = layerDir->GetUniformFeatureDir(TEXTURE_FEATURE_TYPE);
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
 /*           }

        // New interface

        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData)
            {
            HPRECONDITION(m_tileHandler != NULL);
            /*void** serializedGraph = new void*[countData];
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
            */
/*            IDTMFile::NodeID newNodeID;
            newNodeID = 0;//
            /*if (!m_tileHandler->AddPoints(newNodeID, arrayOfPoints))
                {
                HASSERT(!"Write failed!");
                //&&MM TODO we cannot use custom exception string. AND the message seems wrong.
                throw HFCWriteFaultException(L"Unable to obtain file name ... IDTMFile::File API should be modified.");
                }
            delete[] ptArray;*/
 /*           return HPMBlockID(newNodeID);
            }

        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            //HPRECONDITION(m_tileHandler != NULL);
            //HPRECONDITION(!m_DTMFile->IsReadOnly()); //TDORAY: Reactivate

            if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
                return StoreNewBlock(DataTypeArray, countData);

            /*void** serializedGraph = new void*[countData];
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
            delete[] ptArray;*/
 /*           return blockID;
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
           //Currently only support the one graph. Obtaining the number of elements for non-POD types using only the byte count of the packet is not possible,
           //and we're not supporting storing an actual number of elements as packet metadata right now.
            return 1;
            }


        virtual size_t StoreHeader(float* header, HPMBlockID blockID)
            {
            return 1;
            }

        virtual size_t LoadHeader(float* header, HPMBlockID blockID)
            {

            return 1;
            }

        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            HPRECONDITION(m_tileHandler != NULL);

            IDTMFile::PointTileHandler<int32_t>::PointArray arrayOfPoints;
            //get size of packet in number of ints
            size_t packetSize = m_tileHandler->GetDir().CountPoints(ConvertBlockID(blockID));
            int32_t* results = new int32_t[packetSize];
            arrayOfPoints.WrapEditable(results, 0, packetSize);
            if (!m_tileHandler->GetPoints(ConvertBlockID(blockID), arrayOfPoints))
                throw; //TDORAY: Do something else*/
/*            if (results[0] > 0 && maxCountData > 0)
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

        static const IDTMFile::FeatureType TEXTURE_FEATURE_TYPE = 1;

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
    };*/