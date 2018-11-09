/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMRasterDraping.cpp $
|    $RCSfile: MrDTMRasterDraping.cpp,v $
|   $Revision: 1.7 $
|       $Date: 2010/11/29 13:15:51 $
|     $Author: Daryl.Holmwood $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableTerrainModelPCH.h>


#include "MrDTMRasterDraping.h"

/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/

/*----------------------------------------------+
| Private type definitions                      |
+----------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

class ImageppOpaqueData
{
public :

    friend class MrDTMRasterDraping;

    ImageppOpaqueData() {}
    virtual ~ImageppOpaqueData() {}

    static const HFCPtr<HGF2DWorldCluster> GetWorldCluster() 
        {
        if (s_pLocalWorldCluster == 0)
            {
            s_pLocalWorldCluster = new HGFHMRStdWorldCluster();        
            }

        return s_pLocalWorldCluster;
        }

    static HPMPool* GetMemPool() 
        {
        if (s_pLocalPool == 0)
            {
            s_pLocalPool = new HPMPool(300000, HPMPool::KeepLastBlock);  
            }

        return s_pLocalPool;
        }

    const HFCPtr<HIMMosaic> GetDrapingMosaic() const
        {
        return m_pRastersToDrape;
        }

protected :

    HFCPtr<HIMMosaic>                m_pRastersToDrape;
    vector<HFCPtr<HRABitmap>>        m_pLastTexturesRequested;
    
    //Imagepp required global data that are kept locally for now.
    static HFCPtr<HGF2DWorldCluster> s_pLocalWorldCluster; 
    static HAutoPtr<HPMPool>         s_pLocalPool; 
};

#define LOCALWORLDCLUSTER (ImageppOpaqueData::GetWorldCluster())
#define LOCALPOOL         (ImageppOpaqueData::GetMemPool())
#define MOSAIC            (m_pImageppOpaqueData->m_pRastersToDrape)

//Imagepp required global data that are kept locally for now.
HFCPtr<HGF2DWorldCluster> ImageppOpaqueData::s_pLocalWorldCluster; //HFCPtr<HGF2DWorldCluster> 
HAutoPtr<HPMPool> ImageppOpaqueData::s_pLocalPool; //HAutoPtr<HPMPool>         

void CreateRawFileFromImageData(HFCPtr<HFCURL>&       pi_rpFileName, 
                                ULong32                pi_ImageDataWidth, 
                                ULong32                pi_ImageDataHeight, 
                                HFCPtr<HRPPixelType>& pi_rpImageDataPixelType, 
                                Byte*               pi_ImageData);


/*----------------------------------------------------------------------------+
|MrDTMRasterDraping::MrDTMRasterDraping
+----------------------------------------------------------------------------*/
MrDTMRasterDraping* MrDTMRasterDraping::Create(const AString& pi_rFileNameDescriptors)
    {
    MrDTMRasterDraping* pMrDTMRasterDraping = 0;
  
    try
        {
        pMrDTMRasterDraping = new MrDTMRasterDraping(pi_rFileNameDescriptors);

        //The mosaic was not created, likely because no valid raster file 
        //was found.


        if (pMrDTMRasterDraping->GetImageppOpaqueData()->GetDrapingMosaic() == 0)
            {
            delete pMrDTMRasterDraping;
            pMrDTMRasterDraping = 0;
            }
        }
    catch (HFCException&)
        {
        }

    return pMrDTMRasterDraping;
    }

/*----------------------------------------------------------------------------+
|MrDTMRasterDraping::MrDTMRasterDraping
+----------------------------------------------------------------------------*/
MrDTMRasterDraping::MrDTMRasterDraping(const AString& pi_rFileNameDescriptors)
    {    
    m_pImageppOpaqueData = 0;
    
   // vector<WChar*> mosaicRasterFileNames;
    HIMMosaic::RasterList             rasterList;
    HFCPtr<HGF2DCoordSys>  pLogicalCoordSys;
    HFCPtr<HRSObjectStore> pObjectStore;
    HFCPtr<HFCURL>         pRasterFileURL;
    HFCPtr<HRFRasterFile>  pRasterFile;
    HFCPtr<HRARaster>      pRaster;
    char*                  pFileNameDescSep = ";";

    HAutoPtr<char> pFileNameDescriptors(_strdup(pi_rFileNameDescriptors.c_str()));

    // Establish AString and get the first token:
    char* fileNameDesc = strtok(pFileNameDescriptors.get(), pFileNameDescSep); 
    
    while (fileNameDesc != NULL)
        {
        bool            continueSearch;
        HANDLE          hFindFile;
        WIN32_FIND_DATA findFileData;               
        WChar          pDrive[256];        
        WChar          pDir[1024];        
        AString          folderPath;

        errno_t errCode = _tsplitpath_s(fileNameDesc, pDrive, 256, pDir, 1024, 0, 0, 0, 0);

        assert(errCode == 0);
   
        folderPath = tstring(pDrive) + tstring(pDir);
       
        hFindFile = ::FindFirstFile(fileNameDesc, &findFileData);

        continueSearch = (hFindFile != INVALID_HANDLE_VALUE);
        
        while (continueSearch)
            {                
            if(!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                { 
                tstring fileName = tstring(L"file://") + 
                                   folderPath + 
                                   findFileData.cFileName;                                                                           

                pRasterFileURL = HFCURL::Instanciate(fileName);
            
                pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(pRasterFileURL, TRUE);

                pLogicalCoordSys = LOCALWORLDCLUSTER->GetCoordSysReference(pRasterFile->GetPageWorldIdentificator(0));
                pObjectStore = new HRSObjectStore(LOCALPOOL, 
                                                  pRasterFile, 
                                                  0,
                                                  pLogicalCoordSys);
                
                // Get the raster from the store
                HPMClassFilter<HRARaster> RasterFilter(pObjectStore);
                pRaster = (HRARaster*)RasterFilter.Load();
                HASSERT(pRaster != NULL);       

                rasterList.push_back(pRaster);   
                }

            continueSearch = ::FindNextFile(hFindFile, &findFileData);
            }                 
        // Get next token: 
        fileNameDesc = strtok(NULL, pFileNameDescSep); // C4996
        }
      
       
       
/*
    if (IsBigQuebecDataset == true)
        {
        bool            continueSearch;
        HANDLE          hFindFile;
        WIN32_FIND_DATA findFileData;
        tstring         folderPath(L"F:\\Dataset\\Quebec City 3D\\Imagery\\");
        tstring         folderPathWithWildCard(L"*.tif");

        folderPathWithWildCard = folderPath + folderPathWithWildCard;
       
        hFindFile = ::FindFirstFile(folderPathWithWildCard.c_str(), &findFileData);

        continueSearch = (hFindFile != INVALID_HANDLE_VALUE);
        
        while (continueSearch)
            {                
            if(!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                { 
                tstring fileName = tstring(L"file://") + 
                                   folderPath + 
                                   findFileData.cFileName;                           
                
                mosaicRasterFileNames.push_back(_tcsdup(fileName.c_str()));
                }

            continueSearch = ::FindNextFile(hFindFile, &findFileData);
            }                 
        }
    else
    if (IsQuebecDataset == true)
    {        
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5085.tif"));
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5086.tif"));
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5087.tif"));
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5185.tif"));
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5186.tif"));
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Quebec City 3D\\Imagery\\5187.tif"));        
    }
    else
    {                
        mosaicRasterFileNames.push_back(_tcsdup("file://F:\\Dataset\\Saguenay 3D\\Mos_Saguenay_20090305_mtm7_nad83.ecw"));
    }            
*/     
                               
    m_pImageppOpaqueData = new ImageppOpaqueData;

    if (rasterList.size() > 0)
        {
        HFCPtr<HGF2DCoordSys> pCoordSys(LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD));

        m_pImageppOpaqueData->m_pRastersToDrape = new HIMMosaic(pCoordSys);
        
        MOSAIC->Add(rasterList);    
        }
    }

/*----------------------------------------------------------------------------+
|MrDTMRasterDraping::~MrDTMRasterDraping
+----------------------------------------------------------------------------*/
MrDTMRasterDraping::~MrDTMRasterDraping()
    {
    if (m_pImageppOpaqueData != 0)
        {
        delete m_pImageppOpaqueData;
        }
    }

/*----------------------------------------------------------------------------+
|PRIVATE
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|MrDTMRasterDraping::CreateDrapingMosaic
+----------------------------------------------------------------------------*/
#ifdef __HMR_UNICODE
    #error "Unicode is not yet supported."
#endif

static bool OutputBitmapBuffer = false;
static bool TestVisibleTerrainDistanceAlongVerticalViewAxis = false;
static double ViewAxisYInc = 2;
static bool TestNewMultiCopyFromMechanism = true;

void MrDTMRasterDraping::CreateTextureBitmapForCurrentView(vector<TextureDescription>&  rTextures,    
                                                           double&         textureMinXInUor, 
                                                           double&         textureMinYInUor, 
                                                           double&         textureMaxXInUor, 
                                                           double&         textureMaxYInUor, 
                                                           DMatrix4dCP     rootToViewMatrix, 
                                                           ViewContextP   icontext,
                                                           DPoint3d*            pFencePt,
                                                           int             nbFencePt)
    {   
    assert(m_pImageppOpaqueData->m_pLastTexturesRequested.size() == 0);
    
    double viewMinX = HDOUBLE_MAX;
    double viewMinY = HDOUBLE_MAX;
    double viewMaxX = -HDOUBLE_MAX;
    double viewMaxY = -HDOUBLE_MAX;

    double rootMinX = HDOUBLE_MAX;
    double rootMinY = HDOUBLE_MAX;
    double rootMaxX = -HDOUBLE_MAX;
    double rootMaxY = -HDOUBLE_MAX;
    double rootMaxZ = -HDOUBLE_MAX;

    HArrayAutoPtr<DPoint3d> pFencePtInView(new DPoint3d[nbFencePt]);
    DPoint3d rootPt;
    DPoint3d viewPt;

    for (int ptInd = 0; ptInd < nbFencePt; ptInd++)
        {       
        rootPt.x = pFencePt[ptInd].x; 
        rootPt.y = pFencePt[ptInd].y;
        rootPt.z = pFencePt[ptInd].z;

        rootMinX = min(rootMinX, rootPt.x);
        rootMinY = min(rootMinY, rootPt.y);
        rootMaxX = max(rootMaxX, rootPt.x);
        rootMaxY = max(rootMaxY, rootPt.y);    
        rootMaxY = max(rootMaxY, rootPt.y);    
        rootMaxZ = max(rootMaxZ, rootPt.z);    

        rootPt.z = pFencePt[0].z;

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(rootToViewMatrix, &viewPt, &rootPt, 1);

        pFencePtInView[ptInd].x = viewPt.x;
        pFencePtInView[ptInd].y = viewPt.y;
        pFencePtInView[ptInd].z = viewPt.z;
        
        viewMinX = min(viewMinX, viewPt.x);
        viewMaxX = max(viewMaxX, viewPt.x);
        viewMinY = min(viewMinY, viewPt.y);
        viewMaxY = max(viewMaxY, viewPt.y);
        }    

    double bitmapPhysicalSizeX = viewMaxX - viewMinX;
    double bitmapPhysicalSizeY = viewMaxY - viewMinY;            

    if (TestNewMultiCopyFromMechanism)
        {
        HFCPtr<HGF2DCoordSys> pViewCoordSys(new HGF2DCoordSys());
        HGF2DExtent visibleAreaViewRect(viewMinX, viewMinY, viewMaxX, viewMaxY, pViewCoordSys);
        
        ViewportP pViewport = icontext->GetViewport();

        BSIRect rect;

        pViewport->GetViewRect(&rect, DgnCoordSystem::View);    

        HGF2DExtent viewRect(rect.origin.x, rect.origin.y, rect.corner.x, rect.corner.y, pViewCoordSys);        

        visibleAreaViewRect.Intersect(viewRect);

        if (visibleAreaViewRect.IsDefined())
            {
            DVec3d   dtmPlaneNormalInView;
            DPoint3d dtmPlanePtInView = {pFencePtInView[0].x, pFencePtInView[0].y, pFencePtInView[0].z};
                
            ComputePlaneNormal(dtmPlaneNormalInView, pFencePtInView, nbFencePt);    
            
            ComputeDestinationExtentFromSourceExtent(textureMinXInUor,
                                                     textureMaxXInUor,
                                                     textureMinYInUor,
                                                     textureMaxYInUor,      
                                                     &pViewport->GetRootToViewMap()->M1, 
                                                     visibleAreaViewRect.GetXMin(),
                                                     visibleAreaViewRect.GetXMax(),
                                                     visibleAreaViewRect.GetYMin(),
                                                     visibleAreaViewRect.GetYMax(),
                                                     0,
                                                     &dtmPlanePtInView, 
                                                     &dtmPlaneNormalInView);

            vector<CopyFromAreaInfo> copyFromAreas;
                
            ComputeDifferentResAreas(copyFromAreas, 
                                     &pViewport->GetRootToViewMap()->M0,
                                     &pViewport->GetRootToViewMap()->M1,
                                     dtmPlanePtInView,
                                     dtmPlaneNormalInView,
                                     visibleAreaViewRect.GetXMin(),
                                     visibleAreaViewRect.GetXMax(),
                                     visibleAreaViewRect.GetYMin(),                                 
                                     visibleAreaViewRect.GetYMax());

            if (copyFromAreas.size() == 0)
                {
                CopyFromAreaInfo wholeAreaInfo;

                wholeAreaInfo.m_xMinInUors = textureMinXInUor;
                wholeAreaInfo.m_xMaxInUors = textureMaxXInUor;
                wholeAreaInfo.m_yMinInUors = textureMinYInUor;
                wholeAreaInfo.m_yMaxInUors = textureMaxYInUor;
                wholeAreaInfo.m_zMinInUors = pFencePt[0].z;
                wholeAreaInfo.m_zMaxInUors = pFencePt[0].z;
                wholeAreaInfo.m_surfaceWidthInPixels = visibleAreaViewRect.GetWidth();
                wholeAreaInfo.m_surfaceHeightInPixels = visibleAreaViewRect.GetHeight();

                copyFromAreas.push_back(wholeAreaInfo);                
                }
                                      
            if (TestVisibleTerrainDistanceAlongVerticalViewAxis)
                {
                double middleX = visibleAreaViewRect.GetXMin() + visibleAreaViewRect.GetWidth() / 2;
                
                DPoint3d srcPt;
                DPoint3d dstPt;                
                DPoint3d lastDstPt;    
                HArrayAutoPtr<double> pDistances(new double[(unsigned int)(visibleAreaViewRect.GetHeight() / ViewAxisYInc)]);
                short distanceInd = 0;

                srcPt.x = middleX;
               
                for (double y = visibleAreaViewRect.GetYMin(); y < visibleAreaViewRect.GetYMax(); y += ViewAxisYInc)
                    {            
                    srcPt.y = y;
                    
                    if (dtmPlaneNormalInView.z != 0)
                        {            
                        srcPt.z = -1 * (dtmPlaneNormalInView.x * (srcPt.x - dtmPlanePtInView.x) + 
                                         dtmPlaneNormalInView.y * (srcPt.y - dtmPlanePtInView.y)) / dtmPlaneNormalInView.z + 
                                         dtmPlanePtInView.z;
                        }
                    else
                        {
                        srcPt.z = dtmPlanePtInView.z;
                        }

                    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(&pViewport->GetRootToViewMap()->M1, &dstPt, &srcPt, 1);                            

                    if (y > visibleAreaViewRect.GetYMin())
                        {                     
                        pDistances[distanceInd] = bsiDPoint3d_distance(&dstPt, &lastDstPt);
                        distanceInd++;
                        }            

                    lastDstPt = dstPt;            
                    }
                }

                HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());
                HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
                                         
                

                vector<CopyFromAreaInfo>::iterator copyFromAreaIter = copyFromAreas.begin();
                vector<CopyFromAreaInfo>::iterator copyFromAreaIterEnd = copyFromAreas.end();                            

                while (copyFromAreaIter != copyFromAreaIterEnd)
                    {                                  
                    //size_t             bufferLength;                          
                    vector<double>    clipShapePoints;                
                    HRACopyFromOptions copyFromOptions;                 
                    HFCPtr<HVEShape>   pClipShape;         
                    HFCPtr<HRARaster>  pSourceRaster((HRARaster*)MOSAIC);
                    HFCPtr<HRABitmap>  pTextureBitmap;
                    short              textureInd = 0;
                    static int         BitmapInd = 0;
                    double             scaleX = HDOUBLE_MAX;
                    double             scaleY = HDOUBLE_MAX;
                
                    scaleX = min(scaleX, (copyFromAreaIter->m_xMaxInUors - copyFromAreaIter->m_xMinInUors) / copyFromAreaIter->m_surfaceWidthInPixels);
                    scaleY = min(scaleY, (copyFromAreaIter->m_yMaxInUors - copyFromAreaIter->m_yMinInUors) / copyFromAreaIter->m_surfaceHeightInPixels);
                                        
                    //2048 is the maximum texture resolution
                       
                    //Take the highest resolution possible
                    double scale = min(scaleX, scaleY);    
                    double correctedScale = scale;
                                        
                    bitmapPhysicalSizeX = (copyFromAreaIter->m_xMaxInUors - copyFromAreaIter->m_xMinInUors) / scale;
                    bitmapPhysicalSizeY = (copyFromAreaIter->m_yMaxInUors - copyFromAreaIter->m_yMinInUors) / scale;

                    if (bitmapPhysicalSizeX > bitmapPhysicalSizeY)
                        {
                        if (bitmapPhysicalSizeX > 2048)
                            {
                            correctedScale = (copyFromAreaIter->m_xMaxInUors - copyFromAreaIter->m_xMinInUors) / 2048;                       
                            }
                        }
                    else
                        {
                        if (bitmapPhysicalSizeY > 2048)
                            {
                            correctedScale = (copyFromAreaIter->m_yMaxInUors - copyFromAreaIter->m_yMinInUors) / 2048;                       
                            }
                        }

                    if (correctedScale != scale)
                        {
                        scale = correctedScale;
                        bitmapPhysicalSizeX = (copyFromAreaIter->m_xMaxInUors - copyFromAreaIter->m_xMinInUors) / scale;
                        bitmapPhysicalSizeY = (copyFromAreaIter->m_yMaxInUors - copyFromAreaIter->m_yMinInUors) / scale;
                        }                                        

                        //Take the highest resolution possible
                       // double scale = min(ScaleX, ScaleY);                                        
                                        
                        HFCMatrix<3,3> transfoMatrix;
                        transfoMatrix[0][0] = scale;
                        transfoMatrix[0][1] = 0;
                        transfoMatrix[0][2] = copyFromAreaIter->m_xMinInUors; 
                        transfoMatrix[1][0] = 0;
                        transfoMatrix[1][1] = -scale; //transfoMatrix[0][0];// (MosaicExtent.GetYMax() - MosaicExtent.GetYMin()) / bitmapPhysicalSizeY;
                        transfoMatrix[1][2] = copyFromAreaIter->m_yMaxInUors; 
                        transfoMatrix[2][0] = 0;
                        transfoMatrix[2][1] = 0;
                        transfoMatrix[2][2] = 1;     

                        HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

                        HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

                        if (pSimplifiedModel != 0)
                            {
                            pTransfoModel = pSimplifiedModel;
                            }       
                      /*
                        bitmapPhysicalSizeX = copyFromAreaIter->m_surfaceWidthInPixels;
                        bitmapPhysicalSizeY = copyFromAreaIter->m_surfaceHeightInPixels;
                                 */                  
                        pTextureBitmap = new HRABitmap((ULong32)bitmapPhysicalSizeX,
                                                       (ULong32)bitmapPhysicalSizeY,
                                                       pTransfoModel.GetPtr(),      
                                                       LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD),                                                                                        
                                                       pPixelType,
                                                       8, 
                                                       HRABitmap::UPPER_LEFT_HORIZONTAL, 
                                                       pCodec);

                 //       pTextureBitmap->Clear();
                        /*
                        //Set destination clip shape
                        if (copyFromAreaIter->m_clipPoints.size() > 0)
                            {
                            clipShapePoints.clear();

                            for (short indClipPoint = 0; indClipPoint < copyFromAreaIter->m_clipPoints.size(); indClipPoint++)
                                {                            
                                clipShapePoints.push_back(copyFromAreaIter->m_clipPoints[indClipPoint].x);
                                clipShapePoints.push_back(copyFromAreaIter->m_clipPoints[indClipPoint].y);
                                }
                            
                            bufferLength = clipShapePoints.size();

                            pClipShape =  new HVEShape(&bufferLength,
                                                       &clipShapePoints[0],
                                                       LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD));                                                            

                            copyFromOptions.SetDestShape(pClipShape);
                            }
                        else
                            {
                            copyFromOptions.SetDestShape(0);
                            }*/
                                              
                  //      pTextureBitmap->CopyFrom(pSourceRaster, copyFromOptions);

                        if (OutputBitmapBuffer == true)
                            {                               
                            char TempBuffer[300];
                                                    
                            sprintf(TempBuffer, "file://F:\\MyDoc\\SS2 - Iteration 7\\MrDTM\\Visualization - Analysis\\Draping\\Texture%i_%i.bmp", BitmapInd, textureInd++);
                                
                            HFCPtr<HFCURL> pFileName(new HFCURLFile(TempBuffer));
                            
                            HFCPtr<HRPPixelType> pBMPPixelType(new HRPPixelTypeV32B8G8R8X8());            

                            CreateRawFileFromImageData(pFileName, 
                                                       (ULong32)bitmapPhysicalSizeX,                                                   
                                                       (ULong32)bitmapPhysicalSizeY,                                
                                                       pBMPPixelType, 
                                                       pTextureBitmap->GetPacket()->GetBufferAddress());

                            if (BitmapInd % 10 == 0)
                                {
                                BitmapInd = 0;
                                }
                            }                             

                        TextureDescription textureDesc;                                                                   

                        textureDesc.m_pTextureBitmapData = pTextureBitmap->GetPacket()->GetBufferAddress();
                        textureDesc.m_textureBitmapWidth = (int)bitmapPhysicalSizeX;
                        textureDesc.m_textureBitmapHeight = (int)bitmapPhysicalSizeY;                                                             

                        textureDesc.m_xMinInUors = copyFromAreaIter->m_xMinInUors;
                        textureDesc.m_xMaxInUors = copyFromAreaIter->m_xMaxInUors;
                        textureDesc.m_yMinInUors = copyFromAreaIter->m_yMinInUors;
                        textureDesc.m_yMaxInUors = copyFromAreaIter->m_yMaxInUors;

                        m_pImageppOpaqueData->m_pLastTexturesRequested.push_back(pTextureBitmap);

                        rTextures.push_back(textureDesc);                                                                    

                        
                    copyFromAreaIter++;
                    }                         
                }
        }
    else
        {
    HFCPtr<HVEShape>  pVisibleDTMShapeExtent(new HVEShape(rootMinX, 
                                                          rootMinY, 
                                                          rootMaxX, 
                                                          rootMaxY, 
                                                          LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD)));

    HFCPtr<HRARaster> pSourceRaster((HRARaster*)MOSAIC);

    HFCPtr<HVEShape> pEffectiveArea(new HVEShape(*(pSourceRaster->GetEffectiveShape())));

    pEffectiveArea->ChangeCoordSys(LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD));

    pEffectiveArea->Intersect(*pVisibleDTMShapeExtent);

    if (pEffectiveArea->IsEmpty() == false)
        {  
        //double transfoMatrixVals[4][4];
   
        HGF2DExtent effectiveExtent = pEffectiveArea->GetExtent();

        double ScaleX = (effectiveExtent.GetXMax() - effectiveExtent.GetXMin()) / bitmapPhysicalSizeX;
        double ScaleY = (effectiveExtent.GetYMax() - effectiveExtent.GetYMin()) / bitmapPhysicalSizeY;

        //Take the highest resolution possible
        double Scale = min(ScaleX, ScaleY);

        if (ScaleX != Scale)
            {
            bitmapPhysicalSizeX = bitmapPhysicalSizeX  * ScaleX / Scale;
            }
        else
            {
            bitmapPhysicalSizeY = bitmapPhysicalSizeY  * ScaleY / Scale;
            }

            //Take the highest resolution possible            
            double correctedScale = Scale;

            //There is a hardware limitation for the maximum size of a texture, 
            //which we will set to 2048 for now.
            if (bitmapPhysicalSizeX > bitmapPhysicalSizeY)
                {
                if (bitmapPhysicalSizeX > 2048)
                    {
                    correctedScale = (effectiveExtent.GetXMax() - effectiveExtent.GetXMin()) / 2048;                       
                    }
                }
            else
                {
                if (bitmapPhysicalSizeY > 2048)
                    {
                    correctedScale = (effectiveExtent.GetYMax() - effectiveExtent.GetYMin()) / 2048;                       
                    }
                }

            if (correctedScale != Scale)
                {
                Scale = correctedScale;
                bitmapPhysicalSizeX = (effectiveExtent.GetXMax() - effectiveExtent.GetXMin()) / Scale;
                bitmapPhysicalSizeY = (effectiveExtent.GetYMax() - effectiveExtent.GetYMin()) / Scale;
                }       


            HFCMatrix<3,3> transfoMatrix;
            transfoMatrix[0][0] = Scale;
            transfoMatrix[0][1] = 0;
            transfoMatrix[0][2] = effectiveExtent.GetXMin(); 
            transfoMatrix[1][0] = 0;
            transfoMatrix[1][1] = -Scale; //transfoMatrix[0][0];// (MosaicExtent.GetYMax() - MosaicExtent.GetYMin()) / bitmapPhysicalSizeY;
            transfoMatrix[1][2] = effectiveExtent.GetYMax(); 
            transfoMatrix[2][0] = 0;
            transfoMatrix[2][1] = 0;
            transfoMatrix[2][2] = 1;         
                            
            textureMinXInUor = effectiveExtent.GetXMin();
            textureMinYInUor = effectiveExtent.GetYMin();
            textureMaxXInUor = effectiveExtent.GetXMax();
            textureMaxYInUor = effectiveExtent.GetYMax();
           
            bitmapPhysicalSizeY = fabs((rootMaxY - rootMinY) / transfoMatrix[0][0]);

            HFCPtr<HGF2DTransfoModel> pTransfoModel((HGF2DTransfoModel*)new HGF2DProjective(transfoMatrix));

            HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pTransfoModel->CreateSimplifiedModel();

            if (pSimplifiedModel != 0)
                {
                pTransfoModel = pSimplifiedModel;
                }                                            

            HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());
            HFCPtr<HCDCodec>     pCodec(new HCDCodecIdentity());
                                     
            HFCPtr<HRABitmap> pTextureBitmap = new HRABitmap((ULong32)bitmapPhysicalSizeX,
                                                             (ULong32)bitmapPhysicalSizeY,
                                                             pTransfoModel.GetPtr(),      
                                                             LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD),                                                                                        
                                                             pPixelType,
                                                             8, 
                                                             HRABitmap::UPPER_LEFT_HORIZONTAL, 
                                                             pCodec);


            pTextureBitmap->Clear();

            HRACopyFromOptions copyFromOptions;
               
            pTextureBitmap->CopyFrom(pSourceRaster, copyFromOptions);        

            TextureDescription textureDesc;

            textureDesc.m_pTextureBitmapData = pTextureBitmap->GetPacket()->GetBufferAddress();
            textureDesc.m_textureBitmapWidth = (ULong32)bitmapPhysicalSizeX;
            textureDesc.m_textureBitmapHeight = (ULong32)bitmapPhysicalSizeY;
            textureDesc.m_xMinInUors = textureMinXInUor;
            textureDesc.m_xMaxInUors = textureMaxXInUor;
            textureDesc.m_yMinInUors = textureMinYInUor;
            textureDesc.m_yMaxInUors = textureMaxYInUor;

            if (OutputBitmapBuffer == true)
            {    
                static int BitmapInd = 0;

                char TempBuffer[300];
                
                sprintf(TempBuffer, "file://F:\\MyDoc\\SS2 - Iteration 7\\MrDTM\\Visualization - Analysis\\Draping\\ChessTexture.bmp");
                    
                HFCPtr<HFCURL> pFileName(new HFCURLFile(TempBuffer));
                
                HFCPtr<HRPPixelType> pBMPPixelType(new HRPPixelTypeV32B8G8R8X8());            

                CreateRawFileFromImageData(pFileName, 
                                           textureDesc.m_textureBitmapWidth, 
                                           textureDesc.m_textureBitmapHeight,                                
                                           pBMPPixelType, 
                                           textureDesc.m_pTextureBitmapData);
            }     


            m_pImageppOpaqueData->m_pLastTexturesRequested.push_back(pTextureBitmap);

            rTextures.push_back(textureDesc);
            }
        }        
            
              
    }

bool TestDrillingDifferentAreas = false;

void MrDTMRasterDraping::ComputePlaneNormal(DVec3d& rNormalVec,
                                            DPoint3d*    pFencePt,
                                            int     nbFencePt)
    {
    //Vectors defining the plan on which the valid region lies 
    //in view (i.e. : screen) coordinates.
    DVec3d           uVector;
    DVec3d           vVector;    
               
    //Should be a closed shape
    assert(nbFencePt > 3);
    
    uVector.x = pFencePt[1].x - pFencePt[0].x;
    uVector.y = pFencePt[1].y - pFencePt[0].y;
    uVector.z = pFencePt[1].z - pFencePt[0].z;

    int ptInd;

    for (ptInd = 2; ptInd < nbFencePt; ptInd++)
        {
        vVector.x = pFencePt[ptInd].x - pFencePt[0].x;
        vVector.y = pFencePt[ptInd].y - pFencePt[0].y;
        vVector.z = pFencePt[ptInd].z - pFencePt[0].z;

        if (mdlVec_areParallel(&uVector, &vVector) == false)
            {
            break;
            }
        }

    //This should be a closed shape.
    assert(ptInd < nbFencePt);
    /*
    uVector.normalize();
    vVector.normalize();
    */
    // compute normal
    mdlVec_normalizedCrossProduct(&rNormalVec, &uVector, &vVector);
/*
    //Adjust the z so that it lies on the plan defined by the raster.
    for (int cornerInd = 0; cornerInd < 4; cornerInd++)
        {                                     
        if (normalVector.z != 0)
            {
            pio_rBitmapCorners3d[cornerInd].z = -1 * (normalVector.x * (pio_rBitmapCorners3d[cornerInd].x - projValidRegionPointsInScreen[0].x) + 
                                                      normalVector.y * (pio_rBitmapCorners3d[cornerInd].y - projValidRegionPointsInScreen[0].y)) / normalVector.z + 
                                                projValidRegionPointsInScreen[0].z;
            }
        else
            {
            pio_rBitmapCorners3d[cornerInd].z = projValidRegionPointsInScreen[0].z;
            }
        }*/
    }

#define       MIN_AREA_SIDE_LENGTH                  120
static double RELATIVE_DIFF_IN_PIXEL_SIZE_THRESHOLD = 0.01;

void MrDTMRasterDraping::ComputeDifferentResAreas(vector<CopyFromAreaInfo>& rCopyFromAreas,    
                                                  DMatrix4dCP  pRootToViewMatrix,
                                                  DMatrix4dCP  pViewToRootMatrix,                                                  
                                                  DPoint3d&    dtmPlanePtInView,
                                                  DVec3d&      dtmPlaneNormalInView,
                                                  double       pi_xMin,
                                                  double       pi_xMax,
                                                  double       pi_yMin,
                                                  double       pi_yMax)
    {          
        /*
    double      xMin;
    double      xMax;
    double      yMin;
    double      yMax;*/
    double      ulMinPixelSizeRequired;
    double      urMinPixelSizeRequired;
    double      llMinPixelSizeRequired;
    double      lrMinPixelSizeRequired;
    
    double      ulMinBitmapSizeInUor;
    double      urMinBitmapSizeInUor;
    double      llMinBitmapSizeInUor;
    double      lrMinBitmapSizeInUor;    

    HGF2DExtent ulQuadrantExtent;    
    HGF2DExtent urQuadrantExtent;    
    HGF2DExtent lrQuadrantExtent;    
    HGF2DExtent llQuadrantExtent;    

    CopyFromAreaInfo ulCopyFromAreaInfo;
    CopyFromAreaInfo urCopyFromAreaInfo;  
    CopyFromAreaInfo lrCopyFromAreaInfo;  
    CopyFromAreaInfo llCopyFromAreaInfo;  
           
    //UL Area
    ulQuadrantExtent.SetRawXMin(pi_xMin);
    ulQuadrantExtent.SetRawXMax(pi_xMax - (pi_xMax - pi_xMin) / 2);
    ulQuadrantExtent.SetRawYMin(pi_yMax - (pi_yMax - pi_yMin) / 2);
    ulQuadrantExtent.SetRawYMax(pi_yMax);        
        
    ulMinPixelSizeRequired = ComputeMinPixelSizeRequired(ulMinBitmapSizeInUor, 
                                                         ulCopyFromAreaInfo,
                                                         pViewToRootMatrix, 
                                                         ulQuadrantExtent.GetXMin(),
                                                         ulQuadrantExtent.GetXMax(),
                                                         ulQuadrantExtent.GetYMin(),
                                                         ulQuadrantExtent.GetYMax(),                                                         
                                                         dtmPlanePtInView,
                                                         dtmPlaneNormalInView);   


     


    //UR Area
    urQuadrantExtent.SetRawXMin(pi_xMax - (pi_xMax - pi_xMin) / 2);
    urQuadrantExtent.SetRawXMax(pi_xMax);
    urQuadrantExtent.SetRawYMin(ulQuadrantExtent.GetYMin());
    urQuadrantExtent.SetRawYMax(ulQuadrantExtent.GetYMax());
    
    urMinPixelSizeRequired = ComputeMinPixelSizeRequired(urMinBitmapSizeInUor, 
                                                         urCopyFromAreaInfo,
                                                         pViewToRootMatrix, 
                                                         urQuadrantExtent.GetXMin(),
                                                         urQuadrantExtent.GetXMax(),
                                                         urQuadrantExtent.GetYMin(),
                                                         urQuadrantExtent.GetYMax(),                                                         
                                                         dtmPlanePtInView,
                                                         dtmPlaneNormalInView);

    //LR Area    
    lrQuadrantExtent.SetRawXMin(urQuadrantExtent.GetXMin());
    lrQuadrantExtent.SetRawXMax(urQuadrantExtent.GetXMax());    
    lrQuadrantExtent.SetRawYMin(pi_yMin);
    lrQuadrantExtent.SetRawYMax(pi_yMax - (pi_yMax - pi_yMin) / 2);        

    lrMinPixelSizeRequired = ComputeMinPixelSizeRequired(lrMinBitmapSizeInUor, 
                                                         lrCopyFromAreaInfo,
                                                         pViewToRootMatrix, 
                                                         lrQuadrantExtent.GetXMin(),
                                                         lrQuadrantExtent.GetXMax(),
                                                         lrQuadrantExtent.GetYMin(),
                                                         lrQuadrantExtent.GetYMax(),
                                                         dtmPlanePtInView,
                                                         dtmPlaneNormalInView);    

    //LL Area    
    llQuadrantExtent.SetRawXMin(pi_xMin);
    llQuadrantExtent.SetRawXMax(pi_xMax - (pi_xMax - pi_xMin) / 2);
    llQuadrantExtent.SetRawYMin(lrQuadrantExtent.GetYMin());
    llQuadrantExtent.SetRawYMax(lrQuadrantExtent.GetYMax());    
    
    llMinPixelSizeRequired = ComputeMinPixelSizeRequired(llMinBitmapSizeInUor, 
                                                         llCopyFromAreaInfo,
                                                         pViewToRootMatrix, 
                                                         llQuadrantExtent.GetXMin(),
                                                         llQuadrantExtent.GetXMax(),
                                                         llQuadrantExtent.GetYMin(),
                                                         llQuadrantExtent.GetYMax(),                                                         
                                                         dtmPlanePtInView,
                                                         dtmPlaneNormalInView);

    
    //Check if the cut should be done
    double maxPixelSizeRequired = max(max(max(ulMinPixelSizeRequired, urMinPixelSizeRequired),
                                              lrMinPixelSizeRequired), llMinPixelSizeRequired);


    double minPixelSizeRequired = min(min(min(ulMinPixelSizeRequired, urMinPixelSizeRequired),
                                              lrMinPixelSizeRequired), llMinPixelSizeRequired);
                
    if (((maxPixelSizeRequired - minPixelSizeRequired) / maxPixelSizeRequired > RELATIVE_DIFF_IN_PIXEL_SIZE_THRESHOLD) && 
        (((pi_xMax - pi_xMin) / 2 >= MIN_AREA_SIDE_LENGTH) || 
         ((pi_yMax - pi_yMin) / 2 >= MIN_AREA_SIDE_LENGTH)))
        {      
            /*
        double viewXMin;
        double viewXMax;
        double viewYMin;
        double viewYMax;
*/
        //UL area  
        /*
        ComputeDestinationExtentFromSourceExtent(viewXMin,
                                                 viewXMax,
                                                 viewYMin,
                                                 viewYMax,                                                                 
                                                 pRootToViewMatrix, 
                                                 ulCopyFromAreaInfo.m_xMinInUors,
                                                 ulCopyFromAreaInfo.m_xMaxInUors,
                                                 ulCopyFromAreaInfo.m_yMinInUors,
                                                 ulCopyFromAreaInfo.m_yMaxInUors,                                                 
                                                 ulCopyFromAreaInfo.m_zMaxInUors);            


        ulCopyFromAreaInfo.m_surfaceWidthInPixels = viewXMax - viewXMin;
        ulCopyFromAreaInfo.m_surfaceHeightInPixels = viewYMax - viewYMin;

        rCopyFromAreas.push_back(ulCopyFromAreaInfo);        
    */
        ComputeDifferentResAreas(rCopyFromAreas,    
                                 pRootToViewMatrix,
                                 pViewToRootMatrix,   
                                 dtmPlanePtInView,
                                 dtmPlaneNormalInView,
                                 ulQuadrantExtent.GetXMin(),
                                 ulQuadrantExtent.GetXMax(),
                                 ulQuadrantExtent.GetYMin(),
                                 ulQuadrantExtent.GetYMax());


        //UR area  
        /*
        ComputeDestinationExtentFromSourceExtent(viewXMin,
                                                 viewXMax,
                                                 viewYMin,
                                                 viewYMax,                                                                 
                                                 pRootToViewMatrix, 
                                                 urCopyFromAreaInfo.m_xMinInUors,
                                                 urCopyFromAreaInfo.m_xMaxInUors,
                                                 urCopyFromAreaInfo.m_yMinInUors,
                                                 urCopyFromAreaInfo.m_yMaxInUors,                                                 
                                                 urCopyFromAreaInfo.m_zMaxInUors);            

        urCopyFromAreaInfo.m_surfaceWidthInPixels = viewXMax - viewXMin;
        urCopyFromAreaInfo.m_surfaceHeightInPixels = viewYMax - viewYMin;

        rCopyFromAreas.push_back(urCopyFromAreaInfo);        
    */
        ComputeDifferentResAreas(rCopyFromAreas,    
                                 pRootToViewMatrix,
                                 pViewToRootMatrix,   
                                 dtmPlanePtInView,
                                 dtmPlaneNormalInView,
                                 urQuadrantExtent.GetXMin(),
                                 urQuadrantExtent.GetXMax(),
                                 urQuadrantExtent.GetYMin(),
                                 urQuadrantExtent.GetYMax());


        //LR area    
        /*
        ComputeDestinationExtentFromSourceExtent(viewXMin,
                                                 viewXMax,
                                                 viewYMin,
                                                 viewYMax,                                                                 
                                                 pRootToViewMatrix, 
                                                 lrCopyFromAreaInfo.m_xMinInUors,
                                                 lrCopyFromAreaInfo.m_xMaxInUors,
                                                 lrCopyFromAreaInfo.m_yMinInUors,
                                                 lrCopyFromAreaInfo.m_yMaxInUors,                                                 
                                                 lrCopyFromAreaInfo.m_zMaxInUors);            

        lrCopyFromAreaInfo.m_surfaceWidthInPixels = viewXMax - viewXMin;
        lrCopyFromAreaInfo.m_surfaceHeightInPixels = viewYMax - viewYMin;

        rCopyFromAreas.push_back(lrCopyFromAreaInfo);        
*/    
        ComputeDifferentResAreas(rCopyFromAreas,    
                                 pRootToViewMatrix,
                                 pViewToRootMatrix,   
                                 dtmPlanePtInView,
                                 dtmPlaneNormalInView,
                                 lrQuadrantExtent.GetXMin(),
                                 lrQuadrantExtent.GetXMax(),
                                 lrQuadrantExtent.GetYMin(),
                                 lrQuadrantExtent.GetYMax());


        //LL area       
        /*
        ComputeDestinationExtentFromSourceExtent(viewXMin,
                                                 viewXMax,
                                                 viewYMin,
                                                 viewYMax,                                                                 
                                                 pRootToViewMatrix, 
                                                 llCopyFromAreaInfo.m_xMinInUors,
                                                 llCopyFromAreaInfo.m_xMaxInUors,
                                                 llCopyFromAreaInfo.m_yMinInUors,
                                                 llCopyFromAreaInfo.m_yMaxInUors,                                                 
                                                 llCopyFromAreaInfo.m_zMaxInUors);            

        lrCopyFromAreaInfo.m_surfaceWidthInPixels = viewXMax - viewXMin;
        lrCopyFromAreaInfo.m_surfaceHeightInPixels = viewYMax - viewYMin;

        rCopyFromAreas.push_back(llCopyFromAreaInfo);        
    */
        ComputeDifferentResAreas(rCopyFromAreas,    
                                 pRootToViewMatrix,
                                 pViewToRootMatrix,   
                                 dtmPlanePtInView,
                                 dtmPlaneNormalInView,
                                 llQuadrantExtent.GetXMin(),
                                 llQuadrantExtent.GetXMax(),
                                 llQuadrantExtent.GetYMin(),
                                 llQuadrantExtent.GetYMax());
        } 
        else
        {
            double           viewXMin;
            double           viewXMax;
            double           viewYMin;
            double           viewYMax;
            CopyFromAreaInfo copyFromAreaInfo;
            double           minBitmapSizeInUor;

            ComputeMinPixelSizeRequired(minBitmapSizeInUor, 
                                        copyFromAreaInfo,
                                        pViewToRootMatrix, 
                                        pi_xMin,
                                        pi_xMax, 
                                        pi_yMin, 
                                        pi_yMax,                                                                                                                  
                                        dtmPlanePtInView,
                                        dtmPlaneNormalInView);            
            
            //UL area        
            ComputeDestinationExtentFromSourceExtent(viewXMin,
                                                     viewXMax,
                                                     viewYMin,
                                                     viewYMax,                                                                 
                                                     pRootToViewMatrix, 
                                                     copyFromAreaInfo.m_xMinInUors,
                                                     copyFromAreaInfo.m_xMaxInUors,
                                                     copyFromAreaInfo.m_yMinInUors,
                                                     copyFromAreaInfo.m_yMaxInUors,                                                 
                                                     copyFromAreaInfo.m_zMaxInUors);            

            copyFromAreaInfo.m_surfaceWidthInPixels = viewXMax - viewXMin;
            copyFromAreaInfo.m_surfaceHeightInPixels = viewYMax - viewYMin;

            rCopyFromAreas.push_back(copyFromAreaInfo);   
        }
    }



//New version using the pixel at the extent`s center
double MrDTMRasterDraping::ComputeMinPixelSizeRequired(double&           po_rMinBitmapSizeInUor,
                                                       CopyFromAreaInfo& po_copyFromArea,                      
                                                       DMatrix4dCP pi_pViewToRootMatrix,                                                          
                                                       double      pi_xMin,
                                                       double      pi_xMax,
                                                       double      pi_yMin,
                                                       double      pi_yMax,   
                                                       DPoint3d    dtmPlanePtInView,
                                                       DVec3d      dtmPlaneNormalInView)
    {
    double   minPixelSizeRequired;
    DPoint3d rootPt;
    DPoint3d viewPt;        

    double  viewMinX = HDOUBLE_MAX;
    double  viewMinY = HDOUBLE_MAX;
    double  viewMaxX = -HDOUBLE_MAX;
    double  viewMaxY = -HDOUBLE_MAX;

    po_copyFromArea.m_xMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_xMaxInUors = -HDOUBLE_MAX;
    po_copyFromArea.m_yMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_yMaxInUors = -HDOUBLE_MAX;
    po_copyFromArea.m_zMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_zMaxInUors = -HDOUBLE_MAX;               

    double xCenter = pi_xMax + pi_xMin / 2;
    double yCenter = pi_yMax + pi_yMin / 2;
             
    for (int ptInd = 0; ptInd < 4; ptInd++)
        {       
        switch (ptInd)
            {
            case 0:
                viewPt.x = xCenter - 1; //pi_xMin;
                viewPt.y = yCenter - 1; //pi_yMin;
                break;

            case 1:
                viewPt.x = xCenter - 1; //pi_xMin;
                viewPt.y = yCenter + 1; //pi_yMax;
                break;
               
            case 2:
                viewPt.x = xCenter + 1; //pi_xMax;
                viewPt.y = yCenter + 1; //pi_yMax;
                break;

            case 3:
                viewPt.x = xCenter + 1; //pi_xMax;
                viewPt.y = yCenter - 1; //pi_yMin;
                break;

            default:
                assert(0);
            }


        if (dtmPlaneNormalInView.z != 0)
            {            
            viewPt.z = -1 * (dtmPlaneNormalInView.x * (viewPt.x - dtmPlanePtInView.x) + 
                             dtmPlaneNormalInView.y * (viewPt.y - dtmPlanePtInView.y)) / dtmPlaneNormalInView.z + 
                             dtmPlanePtInView.z;
            }
        else
            {
            viewPt.z = dtmPlanePtInView.z;
            }

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(pi_pViewToRootMatrix, &rootPt, &viewPt, 1);                

        po_copyFromArea.m_xMinInUors = min(po_copyFromArea.m_xMinInUors, rootPt.x);
        po_copyFromArea.m_xMaxInUors = max(po_copyFromArea.m_xMaxInUors, rootPt.x);
        po_copyFromArea.m_yMinInUors = min(po_copyFromArea.m_yMinInUors, rootPt.y);
        po_copyFromArea.m_yMaxInUors = max(po_copyFromArea.m_yMaxInUors, rootPt.y);
        po_copyFromArea.m_zMinInUors = min(po_copyFromArea.m_zMinInUors, rootPt.z);
        po_copyFromArea.m_zMaxInUors = max(po_copyFromArea.m_zMaxInUors, rootPt.z);               
        }    

    //Compute clip
    for (int ptInd = 0; ptInd < 4; ptInd++)
       {       
       switch (ptInd)
           {
           case 0:
               viewPt.x = pi_xMin;
               viewPt.y = pi_yMin;
               break;

           case 1:
               viewPt.x = pi_xMin;
               viewPt.y = pi_yMax;
               break;
              
           case 2:
               viewPt.x = pi_xMax;
               viewPt.y = pi_yMax;
               break;

           case 3:
               viewPt.x = pi_xMax;
               viewPt.y = pi_yMin;
               break;

           default:
               assert(0);
           }


        if (dtmPlaneNormalInView.z != 0)
            {            
            viewPt.z = -1 * (dtmPlaneNormalInView.x * (viewPt.x - dtmPlanePtInView.x) + 
                             dtmPlaneNormalInView.y * (viewPt.y - dtmPlanePtInView.y)) / dtmPlaneNormalInView.z + 
                             dtmPlanePtInView.z;
            }
        else
            {
            viewPt.z = dtmPlanePtInView.z;
            }

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(pi_pViewToRootMatrix, &rootPt, &viewPt, 1);                

        po_copyFromArea.m_clipPoints.push_back(rootPt);        
        }        
   
    minPixelSizeRequired =  (po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors) / 2;

    minPixelSizeRequired = min(minPixelSizeRequired, 
                               (po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors) / 2);

    po_rMinBitmapSizeInUor = min(po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors, 
                                 po_copyFromArea.m_yMaxInUors - po_copyFromArea.m_yMinInUors);
           
    ComputeDestinationExtentFromSourceExtent(po_copyFromArea.m_xMinInUors, 
                                             po_copyFromArea.m_xMaxInUors,
                                             po_copyFromArea.m_yMinInUors, 
                                             po_copyFromArea.m_yMaxInUors,                                             
                                             pi_pViewToRootMatrix, 
                                             pi_xMin,
                                             pi_xMax,
                                             pi_yMin,
                                             pi_yMax, 
                                             0, 
                                             &dtmPlanePtInView,
                                             &dtmPlaneNormalInView);

    po_copyFromArea.m_surfaceWidthInPixels = (po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors) / minPixelSizeRequired;
    po_copyFromArea.m_surfaceHeightInPixels = (po_copyFromArea.m_yMaxInUors - po_copyFromArea.m_yMinInUors) / minPixelSizeRequired;

    return minPixelSizeRequired;
    }

#if 0 //Old version taking the whole extent
double MrDTMRasterDraping::ComputeMinPixelSizeRequired(double&           po_rMinBitmapSizeInUor,
                                                       CopyFromAreaInfo& po_copyFromArea,                      
                                                       DMatrix4dCP pi_pViewToRootMatrix,   
                                                       double      pi_xMin,
                                                       double      pi_xMax,
                                                       double      pi_yMin,
                                                       double      pi_yMax,   
                                                       DPoint3d    dtmPlanePtInView,
                                                       DVec3d      dtmPlaneNormalInView)
    {
    double   minPixelSizeRequired;
    DPoint3d rootPt;
    DPoint3d viewPt;        

    double  viewMinX = HDOUBLE_MAX;
    double  viewMinY = HDOUBLE_MAX;
    double  viewMaxX = -HDOUBLE_MAX;
    double  viewMaxY = -HDOUBLE_MAX;

    po_copyFromArea.m_xMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_xMaxInUors = -HDOUBLE_MAX;
    po_copyFromArea.m_yMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_yMaxInUors = -HDOUBLE_MAX;
    po_copyFromArea.m_zMinInUors = HDOUBLE_MAX;
    po_copyFromArea.m_zMaxInUors = -HDOUBLE_MAX;               

             
    for (int ptInd = 0; ptInd < 4; ptInd++)
        {       
        switch (ptInd)
            {
            case 0:
                viewPt.x = pi_xMin;
                viewPt.y = pi_yMin;
                break;

            case 1:
                viewPt.x = pi_xMin;
                viewPt.y = pi_yMax;
                break;
               
            case 2:
                viewPt.x = pi_xMax;
                viewPt.y = pi_yMax;
                break;

            case 3:
                viewPt.x = pi_xMax;
                viewPt.y = pi_yMin;
                break;

            default:
                assert(0);
            }


        if (dtmPlaneNormalInView.z != 0)
            {            
            viewPt.z = -1 * (dtmPlaneNormalInView.x * (viewPt.x - dtmPlanePtInView.x) + 
                             dtmPlaneNormalInView.y * (viewPt.y - dtmPlanePtInView.y)) / dtmPlaneNormalInView.z + 
                             dtmPlanePtInView.z;
            }
        else
            {
            viewPt.z = dtmPlanePtInView.z;
            }

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(pi_pViewToRootMatrix, &rootPt, &viewPt, 1);
        
        po_copyFromArea.m_clipPoints.push_back(rootPt);

        po_copyFromArea.m_xMinInUors = min(po_copyFromArea.m_xMinInUors, rootPt.x);
        po_copyFromArea.m_xMaxInUors = max(po_copyFromArea.m_xMaxInUors, rootPt.x);
        po_copyFromArea.m_yMinInUors = min(po_copyFromArea.m_yMinInUors, rootPt.y);
        po_copyFromArea.m_yMaxInUors = max(po_copyFromArea.m_yMaxInUors, rootPt.y);
        po_copyFromArea.m_zMinInUors = min(po_copyFromArea.m_zMinInUors, rootPt.z);
        po_copyFromArea.m_zMaxInUors = max(po_copyFromArea.m_zMaxInUors, rootPt.z);               
        }    
   
    minPixelSizeRequired =  (po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors) / (pi_xMax - pi_xMin);

    minPixelSizeRequired = min(minPixelSizeRequired, 
                               (po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors) / (pi_yMax - pi_yMin));

    po_rMinBitmapSizeInUor = min(po_copyFromArea.m_xMaxInUors - po_copyFromArea.m_xMinInUors, 
                                 po_copyFromArea.m_yMaxInUors - po_copyFromArea.m_yMinInUors);

    return minPixelSizeRequired;
    }
#endif

#if 0
 
//Visible DTM extent driven algorithm - doesn't seems to work because the
//extent might be much outside the view.
void MrDTMRasterDraping::ComputeDifferentResAreasUor(DMatrix4dCP pi_pRootToViewMatrix,
                                                  double      pi_xMin,
                                                  double      pi_xMax,
                                                  double      pi_yMin,
                                                  double      pi_yMax,
                                                  double      pi_zValue)
    {          
    double      xMin;
    double      xMax;
    double      yMin;
    double      yMax;
    double      ulMinPixelSizeRequired;
    double      urMinPixelSizeRequired;
    double      llMinPixelSizeRequired;
    double      lrMinPixelSizeRequired;
    
    double      ulMaxBitmapSize;
    double      urMaxBitmapSize;
    double      llMaxBitmapSize;
    double      lrMaxBitmapSize;    

    HGF2DExtent quadrantExtent;    
           
    //UL Area
    quadrantExtent.SetRawXMin(pi_xMin);
    quadrantExtent.SetRawXMax(pi_xMax - (pi_xMax - pi_xMin) / 2);
    quadrantExtent.SetRawYMin(pi_yMax - (pi_yMax - pi_yMin) / 2);
    quadrantExtent.SetRawYMax(pi_yMax);        
        
    ulMinPixelSizeRequired = ComputeMinPixelSizeRequiredUor(ulMaxBitmapSize, 
                                                         pi_pRootToViewMatrix, 
                                                         quadrantExtent.GetXMin(),
                                                         quadrantExtent.GetXMax(),
                                                         quadrantExtent.GetYMin(),
                                                         quadrantExtent.GetYMax(),                                                         
                                                         pi_zValue);

    if (TestDrillingDifferentAreas)
        {
        ComputeDifferentResAreasUor(pi_pRootToViewMatrix,   
                                 quadrantExtent.GetXMin(),
                                 quadrantExtent.GetXMax(),
                                 quadrantExtent.GetYMin(),
                                 quadrantExtent.GetXMax(),                                                         
                                 pi_zValue);
        }

    //UR Area
    quadrantExtent.SetRawXMin(pi_xMax - (pi_xMax - pi_xMin) / 2);
    quadrantExtent.SetRawXMax(pi_xMax);
    
    urMinPixelSizeRequired = ComputeMinPixelSizeRequiredUor(urMaxBitmapSize, 
                                                         pi_pRootToViewMatrix, 
                                                         quadrantExtent.GetXMin(),
                                                         quadrantExtent.GetXMax(),
                                                         quadrantExtent.GetYMin(),
                                                         quadrantExtent.GetYMax(),                                                         
                                                         pi_zValue);

    //LR Area    
    quadrantExtent.SetRawYMin(pi_yMin);
    quadrantExtent.SetRawYMax(pi_yMax - (pi_yMax - pi_yMin) / 2);        

    lrMinPixelSizeRequired = ComputeMinPixelSizeRequiredUor(lrMaxBitmapSize, 
                                                         pi_pRootToViewMatrix, 
                                                         quadrantExtent.GetXMin(),
                                                         quadrantExtent.GetXMax(),
                                                         quadrantExtent.GetYMin(),
                                                         quadrantExtent.GetYMax(),
                                                         pi_zValue);

    //LL Area
    quadrantExtent.SetRawXMin(pi_xMin);
    quadrantExtent.SetRawXMax(pi_xMax - (pi_xMax - pi_xMin) / 2);
    
    llMinPixelSizeRequired = ComputeMinPixelSizeRequiredUor(llMaxBitmapSize, 
                                                         pi_pRootToViewMatrix, 
                                                         quadrantExtent.GetXMin(),
                                                         quadrantExtent.GetXMax(),
                                                         quadrantExtent.GetYMin(),
                                                         quadrantExtent.GetYMax(),                                                         
                                                         pi_zValue);
    }

double MrDTMRasterDraping::ComputeMinPixelSizeRequiredUor(double&     po_rMaxBitmapSize,
                                                       DMatrix4dCP pi_pRootToViewMatrix,   
                                                       double      pi_xMin,
                                                       double      pi_xMax,
                                                       double      pi_yMin,
                                                       double      pi_yMax,                                                       
                                                       double      pi_zValue)
    {
    double   minPixelSizeRequired;
    DPoint3d rootPt;
    DPoint3d viewPt;        
    double  viewMinX = HDOUBLE_MAX;
    double  viewMinY = HDOUBLE_MAX;
    double  viewMaxX = -HDOUBLE_MAX;
    double  viewMaxY = -HDOUBLE_MAX;
    
    for (int ptInd = 0; ptInd < 4; ptInd++)
        {       
        switch (ptInd)
            {
            case 0:
                rootPt.x = pi_xMin;
                rootPt.y = pi_yMin;
                break;

            case 1:
                rootPt.x = pi_xMin;
                rootPt.y = pi_yMax;
                break;
               
            case 2:
                rootPt.x = pi_xMax;
                rootPt.y = pi_yMax;
                break;

            case 3:
                rootPt.x = pi_xMax;
                rootPt.y = pi_yMin;
                break;

            default:
                assert(0);
            }

        rootPt.z = pi_zValue;

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(pi_pRootToViewMatrix, &viewPt, &rootPt, 1);
        
        viewMinX = min(viewMinX, viewPt.x);
        viewMaxX = max(viewMaxX, viewPt.x);
        viewMinY = min(viewMinY, viewPt.y);
        viewMaxY = max(viewMaxY, viewPt.y);
        }     

    minPixelSizeRequired = (pi_xMax - pi_xMin) / (viewMaxX - viewMinX);

    minPixelSizeRequired = min(minPixelSizeRequired, (pi_yMax - pi_yMin) / (viewMaxY - viewMinY));

    po_rMaxBitmapSize = max(viewMaxX - viewMinX, viewMaxY - viewMinY);

    return minPixelSizeRequired;
    }
#endif
     

void MrDTMRasterDraping::ComputeDestinationExtentFromSourceExtent(double&     po_dstXMin,
                                                                  double&     po_dstXMax,
                                                                  double&     po_dstYMin,
                                                                  double&     po_dstYMax,      
                                                                  DMatrix4dCP pi_pSourceToDestinationMatrix, 
                                                                  double      pi_srcXMin,
                                                                  double      pi_srcXMax,
                                                                  double      pi_srcYMin,
                                                                  double      pi_srcYMax, 
                                                                  double      pi_srcZ, 
                                                                  DPoint3d*   pi_pDtmPlanePtInSrc,
                                                                  DVec3d*     pi_pDtmPlaneNormalInSrc)
    {
    DPoint3d srcPt;
    DPoint3d dstPt;

    po_dstXMin = HDOUBLE_MAX;
    po_dstXMax = -HDOUBLE_MAX;
    po_dstYMin = HDOUBLE_MAX;
    po_dstYMax = -HDOUBLE_MAX;

    for (int ptInd = 0; ptInd < 4; ptInd++)
        {       
        switch (ptInd)
            {
            case 0:
                srcPt.x = pi_srcXMin;
                srcPt.y = pi_srcYMin;
                break;

            case 1:
                srcPt.x = pi_srcXMin;
                srcPt.y = pi_srcYMax;
                break;
               
            case 2:
                srcPt.x = pi_srcXMax;
                srcPt.y = pi_srcYMax;
                break;

            case 3:
                srcPt.x = pi_srcXMax;
                srcPt.y = pi_srcYMin;
                break;

            default:
                assert(0);
            }

        if ((pi_pDtmPlanePtInSrc != 0) && (pi_pDtmPlaneNormalInSrc != 0))
            {   
            if (pi_pDtmPlaneNormalInSrc->z != 0)
                {            
                srcPt.z = -1 * (pi_pDtmPlaneNormalInSrc->x * (srcPt.x - pi_pDtmPlanePtInSrc->x) + 
                                pi_pDtmPlaneNormalInSrc->y * (srcPt.y - pi_pDtmPlanePtInSrc->y)) / pi_pDtmPlaneNormalInSrc->z + 
                                pi_pDtmPlanePtInSrc->z;
                }
            else
                {
                srcPt.z = pi_pDtmPlanePtInSrc->z;
                }
            }
        else
            {
            srcPt.z = pi_srcZ;
            }        

        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray(pi_pSourceToDestinationMatrix, &dstPt, &srcPt, 1);
        
        po_dstXMin = min(po_dstXMin, dstPt.x);
        po_dstXMax = max(po_dstXMax, dstPt.x);
        po_dstYMin = min(po_dstYMin, dstPt.y);
        po_dstYMax = max(po_dstYMax, dstPt.y);
        }     
    }


void MrDTMRasterDraping::ReleaseLastTextureBitmap()
    {
    assert(m_pImageppOpaqueData != 0);
    //assert(m_pImageppOpaqueData->m_pLastTexturesRequested != 0);

    m_pImageppOpaqueData->m_pLastTexturesRequested.clear();
    }

//Temporary
#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <HRPTypedCodecs.h>


void CreateRawFileFromImageData(HFCPtr<HFCURL>&       pi_rpFileName, 
                                ULong32                pi_ImageDataWidth, 
                                ULong32                pi_ImageDataHeight, 
                                HFCPtr<HRPPixelType>& pi_rpImageDataPixelType, 
                                Byte*               pi_ImageData)
{
/*   
    HRFBmpCreator::GetInstance()->SetImageData(pi_ImageDataWidth, pi_ImageDataHeight, 0); 
    HRFBmpCreator::GetInstance()->SetImagePixelType(pi_rpImageDataPixelType); 
*/
    HFCPtr<HRFRasterFile> pRasterFile = HRFBmpCreator::GetInstance()->Create(pi_rpFileName,
                                                                             HFC_READ_WRITE_CREATE,
                                                                             0);

    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;
    HFCPtr<HCDCodecsList>                   pCodecList;
    HArrayAutoPtr<Byte>                   pData;

    // Codec
    pCodecList  = (CreateHRPTypedCodecs (new HCDCodecIdentity(), pRasterFile->GetCapabilities()))->GetCodecsList();


    pResolution =  new HRFResolutionDescriptor(
                            HFC_READ_WRITE_CREATE,                                // AccessMode,
                            pRasterFile->GetCapabilities(),                              // Capabilities,
                            1.0,                                            // XResolutionRatio,
                            1.0,                                            // YResolutionRatio,
                            pi_rpImageDataPixelType,                                   // PixelType,
                            pCodecList,                                     // CodecsList,                
                            HRFBlockAccess::RANDOM,                         // RBlockAccess,
                            HRFBlockAccess::RANDOM,                         // WBlockAccess,
                            HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                            HRFInterleaveType::PIXEL,                       // InterleaveType  
                            FALSE,                                          // IsInterlace,
                            pi_ImageDataWidth,                                   // Width image ,   
                            pi_ImageDataHeight,                                  // Height image,
                            pi_ImageDataWidth,                                   // BlockWidth,     
                            1,                                              // BlockHeight,
                            0,                                              // BlocksDataFlag
                            HRFBlockType::LINE);


    pPage = new HRFPageDescriptor (HFC_READ_WRITE_CREATE,
                                   pRasterFile->GetCapabilities(),                       // Capabilities,
                                   pResolution,                             // ResolutionDescriptor,
                                   0,                                       // RepresentativePalette,
                                   0,                                       // Histogram,
                                   0,                                       // Thumbnail,
                                   0,                                       // ClipShape,
                                   0,                                       // TransfoModel,
                                   0,                                       // Filters
                                   0);                                      // Attribute set


    pRasterFile->AddPage(pPage);    

    HAutoPtr<HRFResolutionEditor> pResolutionEditor(pRasterFile->CreateResolutionEditor(0, (UShort)0, HFC_READ_WRITE_CREATE));
    Byte*                       pImageLine = pi_ImageData;
    UShort                       BytesPerPixel = (UShort)(pi_rpImageDataPixelType->CountPixelRawDataBits() / 8);   

    for (ULong32 pi_LineInd = 0; pi_LineInd < pi_ImageDataHeight; pi_LineInd++)
    {
        pResolutionEditor->WriteBlock(0, pi_LineInd, pImageLine);
        
        pImageLine += (pi_ImageDataWidth * BytesPerPixel);
    }    
}




#if 0 //Might be useful functions 
/* -------------------------------------------------------------------------
   qv_addMosaic - add 3D tiled raster image to element
   ------------------------------------------------------------------------- */
void qv_addMosaic
(
QvElemH hElem,                 // => add geometry to this element
int numX,                      // => number of image tiles per row
int numY,                      // => number of rows
QvTextureID const *textureIDs, // => texture for each tile
                               //    (numY rows of numX tiles)
DPoint3d const *verts          // => tile corners by row
                               //    (numY + 1) rows of (numX + 1)
);

/* -------------------------------------------------------------------------
   qv_addMosaic2d - add 2D tiled raster image to element
   ------------------------------------------------------------------------- */
void qv_addMosaic2d
(
QvElemH hElem,                 // => add geometry to this element
int numX,                      // => number of image tiles per row
int numY,                      // => number of rows
QvTextureID const *textureIDs, // => texture for each tile
                               //    (numY rows of numX tiles)
DPoint2d const *verts          // => tile corners by row
                               //    (numY + 1) rows of (numX + 1)
);

#endif

/*----------------------------------------------------------------------------+
|static MrDTMRasterDraping::LocateWorldClusterInDGNWorld
|
|NOTE : Function mostly based on int HGFUSWorldCluster::Synchronize() in RasterLib. The RasterLib version
|       should be used if we decide that MrDTM.dll is linking the RasterLib.
|
+----------------------------------------------------------------------------*/
void MrDTMRasterDraping::LocateWorldClusterInCurrentModel(double meterPerMasterUnits,
                                                          double masterUnitsPerUor,
                                                          double globalOrgX,
                                                          double globalOrgY)
    {   
    HGF2DStretch dgn2HMRModel (HGF2DDisplacement(HGFDistance(HGFDistanceUnit(1.0)), HGFDistance(HGFDistanceUnit(1.0))), 
                               meterPerMasterUnits, 
                               meterPerMasterUnits);

    LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_DGNWORLD)->SetReference(dgn2HMRModel, 
                                                                               LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_HMRWORLD));
    
    // Build a transformation for converting between Intergraph World (uor) to the DGN world (Master Units)        
    double globalOrgXInMeters = globalOrgX * masterUnitsPerUor;
    double globalOrgYInMeters = globalOrgY * masterUnitsPerUor;
    
    HGF2DStretch uor2DgnModel(HGF2DDisplacement(HGFDistance(-(globalOrgXInMeters), HGFDistanceUnit(1.0)), HGFDistance(-(globalOrgYInMeters), HGFDistanceUnit(1.0))),
                              masterUnitsPerUor, 
                              masterUnitsPerUor);
        
    LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_INTERGRAPHWORLD)->SetReference(uor2DgnModel, 
                                                                                      LOCALWORLDCLUSTER->GetCoordSysReference(HGF2DWorld_DGNWORLD));        
    }

const ImageppOpaqueData* MrDTMRasterDraping::GetImageppOpaqueData() const
    {
    return m_pImageppOpaqueData;
    }