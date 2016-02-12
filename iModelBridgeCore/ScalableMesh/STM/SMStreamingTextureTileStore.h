#pragma once

#include <ImagePP/all/h/HPMDataStore.h>
#include <ImagePP/all/h/IDTMTypes.h>
#include <ImagePP/all/h/IDTMFile.h>
//#include <Mtg/MtgStructs.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRFBmpFile.h>
//#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>

class StreamingTextureTileStore : public IHPMPermanentStore<Byte, float, float> // JPEGData (Byte*), size
    {
    public:

        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        StreamingTextureTileStore(WCharCP filename, size_t layerID)
            {
            m_receivedOpenedFile = false;

            m_layerID = layerID;

            }

        virtual ~StreamingTextureTileStore()
            {
            }

        virtual void Close()
            {
            }

        virtual bool StoreMasterHeader(float* indexHeader, size_t headerSize)
            {
            return true;
            }

        virtual size_t LoadMasterHeader(float* indexHeader, size_t headerSize)
            {
            return headerSize;
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

            size_t newNodeID = 0;

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
            if (!blockID.IsValid() || blockID.m_integerID == IDTMFile::SubNodesTable::GetNoSubNodeID())
                return StoreNewBlock(DataTypeArray, countData);


            Byte* ptArray = new Byte[countData];

            memcpy(ptArray, DataTypeArray, countData);
            size_t countAsPts = 0;
            countAsPts = (size_t(ceil((float)countData / sizeof(Byte))));
            delete[] ptArray;
            return blockID;
            }

        virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            return 0;
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

            auto codec = new HCDCodecIJG(width, height, nOfChannels * 8);// (pi_compressedPacket.GetDataSize()); // 24 bits per pixels
            codec->SetSourceColorMode(HCDCodecIJG::ColorModes::RGB);
            codec->SetQuality(70);
            codec->SetSubsamplingMode(HCDCodecIJG::SubsamplingModes::SNONE);
            HFCPtr<HCDCodec> pCodec = codec;
            pi_uncompressedPacket.SetBufferOwnership(true);
            pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
            const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
            pi_uncompressedPacket.SetDataSize(compressedDataSize);

            return true;
        }

        virtual size_t LoadBlock(Byte* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            return 0;
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return true;
            }

    private:
        bool m_receivedOpenedFile;
        size_t m_layerID;
    };
