// SMMeshIndex.cpp

#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"
USING_NAMESPACE_IMAGEPP
#include "Edits/ClipUtilities.hpp"
#include "ScalableMesh/ScalableMeshGraph.h"
#include "ScalableMesh.h"
#include "SMPointIndex.h"
#include "SMMeshIndex.h"
#include "SMMeshIndex.hpp"
#include "ScalableMeshQuery.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

//template class SMPointIndex<DPoint3d, ISMStore::Extent3d64f>;

//template void SMMeshIndexNode<DPoint3d, ISMStore::Extent3d64f>::SplitMeshForChildNodes();

//template void BENTLEY_NAMESPACE_NAME::ScalableMesh::ClipMeshToNodeRange<DPoint3d, ISMStore::Extent3d64f>(vector<int>& faceIndexes, vector<DPoint3d>& nodePts, bvector<DPoint3d>& pts, ISMStore::Extent3d64f& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP);

template class SMMeshIndex<DPoint3d, DRange3d>;

template class SMMeshIndexNode<DPoint3d, DRange3d>;

template class ISMPointIndexMesher<DPoint3d, DRange3d>;

template<typename T> size_t GetSizeInMemory(T* item)
    {
    return sizeof(T);
    }


template<> size_t GetSizeInMemory<MTGGraph>(MTGGraph* item)
    {
    size_t count = 0;
    count += sizeof(*item);
    count += (sizeof(MTGLabelMask) + 2 * sizeof(int))*item->GetLabelCount();
    count += sizeof(MTG_Node)*item->GetNodeIdCount();
    count += sizeof(int)*item->GetNodeIdCount()* item->GetLabelCount();
    return count;
    }

template<> size_t GetSizeInMemory<DifferenceSet>(DifferenceSet* item)
    {
    size_t count = sizeof(item) + item->addedFaces.size()*sizeof(DPoint3d) + item->addedVertices.size() * sizeof(int32_t) +
        item->removedFaces.size() * sizeof(int32_t) + item->removedVertices.size() * sizeof(int32_t) + item->addedUvIndices.size() * sizeof(int32_t) +
        item->addedUvs.size() * sizeof(DPoint2d);
    return count;
    }

template<> size_t GetSizeInMemory<BcDTMPtr>(BcDTMPtr* item)
    {
    size_t count = (item->get() == nullptr ? 0 : ((*item)->GetTinHandle() == nullptr ? 0 : sizeof(BC_DTM_OBJ) + (*item)->GetPointCount() *(sizeof(DPoint3d) + sizeof(DTM_TIN_NODE)+ sizeof(DTM_CIR_LIST)*6)));
    return count;
    }

bool TopologyIsDifferent(const int32_t* indicesA, const size_t nIndicesA, const int32_t* indicesB, const size_t nIndicesB)
    {
    MTGGraph graphA, graphB;
    bvector<int> temp;
    CreateGraphFromIndexBuffer(&graphA, (const long*)indicesA, (int)nIndicesA, (int)nIndicesA, temp, nullptr);
    CreateGraphFromIndexBuffer(&graphB, (const long*)indicesB, (int)nIndicesB, (int)nIndicesB, temp, nullptr);

    size_t nFacesA = CountExteriorFaces(&graphA);
    size_t nFacesB = CountExteriorFaces(&graphB);
    return nFacesA != nFacesB;
    }

bool FindEmptyTex(int&x, int&y, int width, int height, const uint8_t* texP)
    {
    return false;
    }

bool IsMatch(int x, int y, Point2d dimensions, const uint8_t* texP, const uint8_t* pattern, int width, int height)
    {
    for (int i = 0; i < width; ++i)
        for (int j = 0; j < height; ++j)
            {
            if ((y + j) >= dimensions.y || (x + i) >= dimensions.x) return false;
            const uint8_t* origPixel = texP+(y+j)*dimensions.x * 3 + (x+i) * 3;
            const uint8_t* targetPixel = pattern+j*width * 3 + i * 3;
            if (*origPixel != *targetPixel) return false;
            if (*(origPixel + 1) != *(targetPixel + 1)) return false;
            if (*(origPixel + 2) != *(targetPixel + 2)) return false;
            }
    return true;
    }

bool FindMatchingTex(int&x, int&y, int width, int height, Point2d dimensions, const uint8_t* texP, const uint8_t* pattern)
    {
    for (int i = 0; i < dimensions.x; ++i)
        for (int j = 0; j < dimensions.y; ++j)
            {
            if (IsMatch(i, j, dimensions, texP, pattern, width, height))
                {
                x = i;
                y = j;
                return true;
                }
            }
    return false;
    }


void MergeTextures(bvector<uint8_t>& outTex, DPoint2d& uvBotLeft, DPoint2d&  uvTopRight, const ScalableMeshTexture* textureP, const uint8_t* texData, size_t texSize)
    {
    if (textureP == nullptr || textureP->GetSize() == 0)
        {
        uvBotLeft = DPoint2d::From(0.0, 0.0);
        uvTopRight = DPoint2d::From(1.0, 1.0);
        outTex.resize(texSize);
        memcpy(outTex.data(), texData, texSize);

         /*   {
            int w = ((uint32_t*)outTex.data())[0];
            int h = ((uint32_t*)outTex.data())[1];
        WString fileName = L"file://";
        fileName.append(L"e:\\output\\scmesh\\2016-07-29\\texture_");
        fileName.append(std::to_wstring((uint64_t)textureP).c_str());
        fileName.append(L".bmp");
        HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
        HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
        byte* pixelBuffer = new byte[w * h * 3];
        size_t t = 0;
        for (size_t i = 0; i < w * h * 3; i += 3)
            {
            pixelBuffer[t] = *(outTex.data() + 3 * sizeof(int) + i);
            pixelBuffer[t + 1] = *(outTex.data() + 3 * sizeof(int) + i + 1);
            pixelBuffer[t + 2] = *(outTex.data() + 3 * sizeof(int) + i + 2);
            t += 3;
            }
        HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                  w,
                                                  h,
                                                  pImageDataPixelType,
                                                  pixelBuffer);
        delete[] pixelBuffer;
        }*/
        return;
        }

    int width, height;
    width = ((const uint32_t*)texData)[0];
    height = ((const uint32_t*)texData)[1];

    Point2d origDim = textureP->GetDimension();
    int x, y;
    int newWidth, newHeight;
    if (origDim.x >= width && origDim.y >= height && (FindEmptyTex(x, y, width, height, textureP->GetData()) || FindMatchingTex(x,y,width,height,origDim, textureP->GetData(), texData+3*sizeof(int))))
        {
        newWidth = origDim.x;
        newHeight = origDim.y;
        uvBotLeft.x = (double)x / newWidth;
        uvBotLeft.y = (double)(y + height) / newHeight;
        uvTopRight.x = (double)(x + width) / newWidth;
        uvTopRight.y = (double)y / newHeight;
        return;
        }
    else
        {
        if (origDim.x + width <= 8192)
            {
            x = origDim.x;
            y = 0;
            newWidth = origDim.x + width;
            newHeight = std::max(height, origDim.y);
            }
        else
            {
            newWidth = std::max(width, origDim.x);
            newHeight = origDim.y + height;
            x = 0;
            y = origDim.y;
            }
        outTex.resize(3 * sizeof(uint32_t) + newWidth*newHeight * 3, 0xFF);
        }
    ((uint32_t*)outTex.data())[0] = newWidth;
    ((uint32_t*)outTex.data())[1] = newHeight;
    ((uint32_t*)outTex.data())[2] = 3;
    uint8_t* buffer = outTex.data() + 3 * sizeof(uint32_t);

    //copy old texture data in outTex starting at 0,0
    for (size_t i = 0; i < origDim.x; ++i)
        {
        for (size_t j = 0; j < origDim.y; ++j)
            {
            const uint8_t* origPixel = textureP->GetData() + j*origDim.x*3 + i*3;
            uint8_t* targetPixel = buffer + j*newWidth*3 + i*3;
            memcpy(targetPixel, origPixel, 3);
            }
        }

    //copy new texture data in outTex starting at x, y
    for (size_t i = 0; i < width; ++i)
        {
        for (size_t j = 0; j < height; ++j)
            {
            const uint8_t* origPixel = texData + 3 * sizeof(uint32_t) + j*width * 3 + i * 3;
            uint8_t* targetPixel = buffer + (y+j)*newWidth * 3 + (i+x) * 3;
            memcpy(targetPixel, origPixel, 3);
            }
        }

   /* bool dbg = false;
    if (dbg)
                  {
                    WString fileName = L"file://";
                    fileName.append(L"e:\\output\\scmesh\\2016-07-29\\texture2_");
                    fileName.append(std::to_wstring((uint64_t)textureP).c_str());
                    fileName.append(L".bmp");
                    HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
                    HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
                    byte* pixelBuffer = new byte[newWidth * newHeight * 3];
                    size_t t = 0;
                    for (size_t i = 0; i < newWidth * newHeight * 3; i += 3)
                        {
                        pixelBuffer[t] = *(outTex.data() + 3 * sizeof(int) + i);
                        pixelBuffer[t + 1] = *(outTex.data() + 3 * sizeof(int) + i + 1);
                        pixelBuffer[t + 2] = *(outTex.data() + 3 * sizeof(int) + i + 2);
                        t += 3;
                        }
                    HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                              newWidth,
                                                              newHeight,
                                                              pImageDataPixelType,
                                                              pixelBuffer);
                    delete[] pixelBuffer;
                        }*/

    //compute uvs
    uvBotLeft.x = (double)x / newWidth;
    uvBotLeft.y = (double)(y + height) / newHeight;
    uvTopRight.x = (double)(x + width) / newWidth;
    uvTopRight.y = (double)y / newHeight;
    }

void RemapAllUVs(bvector<DPoint2d>& inoutUvs, DPoint2d uvBotLeft, DPoint2d uvTopRight)
    {
    bool repeat = false;
    double maxX = 1.0, maxY = 1.0;
    for (auto& uv : inoutUvs)
        {
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            {
            repeat = true;
            }
        if (uv.x > maxX) maxX = ceil(uv.x);
        if (uv.y > maxY) maxY = ceil(uv.y);
        }

    for (auto& uv : inoutUvs)
        {
       /* if (uv.x < 0.0 || uv.x > 1.0)
            {
            double integral;
            uv.x = std::modf(uv.x, &integral);
            uv.x += (int)integral % 2;
            }*/
        if (repeat && maxX > 0)uv.x /= maxX;
        uv.x = uv.x* (uvTopRight.x - uvBotLeft.x) + uvBotLeft.x;
        if (fabs(uv.x) < 1e-5) uv.x = 0.0;
        if (fabs(uv.x-1.0) < 1e-5) uv.x = 1.0;
        uv.x = fabs(uv.x);
       /* if (uv.y < 0.0 || uv.y > 1.0)
            {
            double integral;
            uv.y = std::modf(uv.y, &integral);
            uv.y += (int)integral % 2;
            }*/
        if (repeat && maxY > 0)uv.y /= maxY;
        uv.y = uv.y* (uvBotLeft.y - uvTopRight.y) + uvTopRight.y;
        if (fabs(uv.y) < 1e-5) uv.y = 0.0;
        if (fabs(uv.y - 1.0) < 1e-5) uv.y = 1.0;
        uv.y = fabs(uv.y);
        }
    }


void ComputeTexPart(bvector<uint8_t>&texPart, DPoint2d* uvPart, size_t nUvs, bvector<uint8_t>& texDataUnified)
    {
    DRange2d range = DRange2d::From(uvPart, (int)nUvs);
     int width, height;
    width = ((const uint32_t*)texDataUnified.data())[0];
    height = ((const uint32_t*)texDataUnified.data())[1];

    if (range.XLength() == 0 || range.YLength() == 0) return;
     int newWidth, newHeight;
    newWidth = ceil(range.XLength()*width);
    newHeight = ceil(range.YLength()*height);
    texPart.resize(3 * sizeof(uint32_t) + 3 * newWidth*newHeight);
    ((uint32_t*)texPart.data())[0] = newWidth;
    ((uint32_t*)texPart.data())[1] = newHeight;
    ((uint32_t*)texPart.data())[2] = 3;
    uint8_t* buffer = texPart.data() + 3 * sizeof(uint32_t);

     int x = range.low.x*width, y = range.low.y*height;
    for (size_t i = 0; i < newWidth; ++i)
        {
        for (size_t j = 0; j < newHeight; ++j)
            {
            const uint8_t* origPixel = &texDataUnified[0] + 3 * sizeof(uint32_t) + (j+y)*width * 3 + (i+x) * 3;
            uint8_t* targetPixel = buffer + j*newWidth * 3 + i * 3;
            memcpy(targetPixel, origPixel, 3);
            }
        }


         /*  {
           WString fileName = L"file://";
           fileName.append(L"e:\\output\\scmesh\\2016-07-29\\texturepart_");
           fileName.append(std::to_wstring((uint64_t)texPart.data()).c_str());
           fileName.append(L".bmp");
           HFCPtr<HFCURL> fileUrl(HFCURL::Instanciate(fileName));
           HFCPtr<HRPPixelType> pImageDataPixelType(new HRPPixelTypeV24B8G8R8());
           byte* pixelBuffer = new byte[newWidth * newHeight * 3];
           size_t t = 0;
           for (size_t i = 0; i < newWidth * newHeight * 3; i += 3)
               {
               pixelBuffer[t] = *(texPart.data() + 3 * sizeof(int) + i);
               pixelBuffer[t + 1] = *(texPart.data() + 3 * sizeof(int) + i + 1);
               pixelBuffer[t + 2] = *(texPart.data() + 3 * sizeof(int) + i + 2);
               t += 3;
               }
           HRFBmpCreator::CreateBmpFileFromImageData(fileUrl,
                                                     newWidth,
                                                     newHeight,
                                                     pImageDataPixelType,
                                                     pixelBuffer);
           delete[] pixelBuffer;
               }*/
    for (size_t i = 0; i < nUvs; ++i)
        {
        uvPart[i].x = (uvPart[i].x - range.low.x) / range.XLength();
        uvPart[i].y = (uvPart[i].y - range.low.y) / range.YLength();
        }
    }