/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/ConverterUtilities.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/ConverterUtilities.cpp,v 1.13 2011/07/18 21:12:33 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Implementation of ConverterUtilities
//-----------------------------------------------------------------------------

#include "ImageConverterPch.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <tchar.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HTiffTag.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include "ConverterUtilities.h"
#include "Chronometer.h"
#include "UsageInformation.h"
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HVE2DUniverse.h>

#include <Imagepp/all/h/HUTImportFromRasterExportToFile.h>
#include <Imagepp/all/h/HFCException.h>

#include <Imagepp/all/h/HCDCodecIJGAltaPhoto.h>
//-----------------------------------------------------------------------------
// HRF Includes 
//-----------------------------------------------------------------------------

#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>

//-----------------------------------------------------------------------------
// HRF File Format Registration
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Pixel Type include 
//-----------------------------------------------------------------------------

#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>

//-----------------------------------------------------------------------------
// Codec include
//-----------------------------------------------------------------------------

#include <Imagepp/all/h/HCDCodecCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
// #include <Imagepp/all/h/HCDCodecIJGAltaPhoto.h>
#include <Imagepp/all/h/HCDCodecJPEG.h>
#include <Imagepp/all/h/HCDCodecSingleColor.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#ifdef __HMR_BTPC_SUPPORTED
    #include <Imagepp/all/h/HCDCodecBTPC.h>
#endif

//-----------------------------------------------------------------------------
// Filter include
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>

//-----------------------------------------------------------------------------
// ExportProgressIndicator
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HUTExportProgressIndicator.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>


//-----------------------------------------------------------------------------
// PSS related includes
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HIMMosaic.h>
#include <Imagepp/all/h/HIMOnDemandMosaic.h>
#include <Imagepp/all/h/HPSObjectStore.h>
#include <Imagepp/all/h/HRFcTiffFile.h>

#include "PSSUtilities.h"


ExportProgressListener::ExportProgressListener()
   : HFCProgressDurationListener()
{
    m_EyePos     =  10;
    m_LeftDir    = true;
}

void ExportProgressListener::Progression(HFCProgressIndicator* pi_pProgressIndicator, 
                                         uint32_t                pi_Processed,		 
                                           uint32_t                pi_CountProgression)  
{
    if (!pi_pProgressIndicator->IsLifeSignal())
        HFCProgressDurationListener::Progression(pi_pProgressIndicator, pi_Processed, pi_CountProgression);
     
    
    if (!pi_pProgressIndicator->IsLifeSignal())
    {
        // Rest number of items to be processed approx duration.
        WChar  buffer[200];
        double TimeToDo = (pi_CountProgression - pi_Processed) * GetAverageDuration();
        uint32_t  Hours    = ((uint32_t)TimeToDo / 3600);
        uint32_t  Minutes  = ((uint32_t)TimeToDo % 3600) / 60;
        uint32_t  Seconds  = ((uint32_t)TimeToDo % 3600) % 60;
    
        WString TimeStr = WString(_TEXT(""));
    
        if (Hours > 0)
        {
            _stprintf(buffer, _TEXT("%02ldh"), Hours); 
            TimeStr += WString(buffer);
        }
        else
            TimeStr += WString(_TEXT("   "));
    
        if ((Hours > 0) || (Minutes > 0))
        {
            _stprintf(buffer, _TEXT("%02ldm"), Minutes); 
            TimeStr += WString(buffer);
        }
        else
            TimeStr += WString(_TEXT("   "));
    
        _stprintf(buffer, _TEXT("%02lds"), Seconds); 
        TimeStr += WString(buffer);
    
        m_TimeToBeProcessed = Utf8String(TimeStr);
    }
    else
    {    
        if (m_LeftDir)
            m_EyePos--;
        else
            m_EyePos++;
    
        if (m_EyePos == 20)
            m_LeftDir = true;
    
        if (m_EyePos == 1)
            m_LeftDir = false;
    }
    
    // Draw the life signal
    WChar  LifeString[200] = _TEXT(" .................... Approx time to be processed: ");
    LifeString[m_EyePos] = _TEXT('');
    
    WString lString;
    lString = WString(LifeString) + WString(m_TimeToBeProcessed.c_str(), BentleyCharEncoding::Utf8);
    _tprintf(_TEXT("%s\r"),lString.c_str());   
}

    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void LoadPSS(const HFCPtr<HFCURL>&      pi_rpFileName, 
             HPMPool*                   pi_pPool, 
             HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster, 
             HFCPtr<HRARaster>&         po_rpRaster, 
             HFCPtr<HPSObjectStore>&    po_rpStore)
{
    HPRECONDITION(pi_rpFileName != 0);    
    
    try
    {    
        po_rpStore = new HPSObjectStore(pi_pPool, pi_rpFileName, pi_rpWorldCluster);
        po_rpRaster = po_rpStore->LoadRaster(0);
        HASSERT(po_rpRaster != NULL);    
    }
    catch(...)
    {        
        po_rpStore = 0;        
    }        
}

    
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void ExportRasterFile(ExportPropertiesContainer* pi_pExportProperties,
                      HFCPtr<HFCURLFile>&        pi_pSrcFilename,
                      HFCPtr<HFCURLFile>&        pi_pDstFilename,
                      HPMPool*                   pio_pPool)
{
#ifdef THUMBNAIL_GENERATOR
    if( pi_pExportProperties->ThumbnailSpecified )
    {
        Chronometer Chrono;
	    Chrono.Start();
        
        CreateThumbnail(pi_pSrcFilename, 
                        pi_pDstFilename, 
                        pi_pExportProperties);

        _tprintf("\b\b\b\b\b\b\b\b\b\bDONE -> %s Duration %s"), 
               WString(pi_pDstFilename->GetHost() + _TEXT("\\" + pi_pDstFilename->GetPath()).c_str(),
               Chrono.Stop().c_str());

    }
    else
    {
#endif
        #ifdef _HMR_ALTAPHOTO
            //-------------------------------------------------------------------------
            // Debug only, temporary code
            if (pi_pExportProperties->AtlaPhotoDumpBlob)
            {
                DumpBlobInFile(pi_pSrcFilename, pi_pDstFilename);
                return;
            }
            // Debug only End
            //-------------------------------------------------------------------------
        #endif
        {
            HAutoPtr<HPMPool> pLog(new HPMPool(10000, NULL));

            // Init global variable
            HFCPtr<HGF2DWorldCluster> g_pWorldCloud;
	        g_pWorldCloud = (HFCPtr<HGF2DWorldCluster>)new HGFHMRStdWorldCluster();

//            HAutoPtr<HUTImportFromFileExportToFile> pImportExport;
            HAutoPtr<HRFImportExport> pImportExport;
                     
            // Check if reprojection is desired

            if (pi_pExportProperties->Reproject)
            {
                _tprintf(_TEXT("Error : Reprojection not supported \n"));

#if 0
GT 6 novembre 2006
Blue Marble was desactivated
                // Need to reproject ...
                
                // In such case we perform an export from raster after having opened the raster file
                // and attached it to the appropriate new projection world

                // Allocate log memory

                
                // Open the raster file
                HFCPtr<HRFRasterFile> pTrueRasterFile(HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)pi_pSrcFilename, true, 0));
                HFCPtr<HRFRasterFile> pRasterFile;

                // Verify of InternetImaging
                if ((pTrueRasterFile->GetClassID() == HRFInternetImagingFile::CLASS_ID) &&
                    (!((HFCPtr<HRFInternetImagingFile>&)pTrueRasterFile)->HasLocalCachedFile()) )
                {
                    // Create the cache
                    pRasterFile = new HRFRasterFileCache(pTrueRasterFile, HRFiTiffCacheFileCreator::GetInstance());

                    // Add the progressive display for internet
                    if ((pRasterFile->CountPages() > 0) && (pRasterFile->GetPageDescriptor(0)->CountResolutions() > 1))
                        pRasterFile = new HRFRasterFileProgressive(pRasterFile);
                }
                else    
                    // Add the necessary booster to improve the raster file performance.
                    pRasterFile = GenericImprove(pTrueRasterFile, HRFiTiffCacheFileCreator::GetInstance());

                if (pRasterFile != 0)
                {
                    HFCPtr<HGF2DCoordSys> pLogicalSpace;
            
                    // Open Blue Marble database

                    double AdaptationMeanError;
                    double AdaptationMaxError;
                    double ReverseMeanError;
                    double ReverseMaxError;
                    HFCPtr<HGF2DTransfoModel> pIDToHMRTransfo  = HCPBlueMarbleDatabase::GetInstance()
                                                        ->CreateBlueMarbleAdaptedModel(pi_pExportProperties->SourceProjection, 
                                                                                       pi_pExportProperties->DestinationProjection,
                                                                                       pRasterFile,
                                                                                       0,
                                                                                       *g_pWorldCloud,
                                                                                       HGF2DWorld_HMRWORLD,
                                                                                       60.0,
                                                                                       pi_pExportProperties->MaxReprojectionError,
                                                                                       pi_pExportProperties->MaxReprojectionError,
                                                                                       false,
                                                                                       &AdaptationMeanError,
                                                                                       &AdaptationMaxError,
                                                                                       &ReverseMeanError,
                                                                                       &ReverseMaxError);


                    // Print out the errors
                    _tprintf(_TEXT("\n"));
                    _tprintf(_TEXT("Reverse Errors (introduced by Blue Marble model in area in source projection units)\n"));
                    _tprintf(_TEXT("Mean: %lf\n"), ReverseMeanError);
                    _tprintf(_TEXT("Maximum: %lf\n"), ReverseMaxError);
                    _tprintf(_TEXT("\n"));
                    _tprintf(_TEXT("Adaptation Errors (introduced by Blue Marble model adaptation in area in destination projection units)\n"));
                    _tprintf(_TEXT("Mean: %lf\n"), AdaptationMeanError);
                    _tprintf(_TEXT("Maximum: %lf\n"), AdaptationMaxError);



                    if (pIDToHMRTransfo == 0)
                    {
                        throw HFCException();
                    }



                    // Create effective logical system
                    pLogicalSpace = new HGF2DCoordSys(*pIDToHMRTransfo, g_pWorldCloud->GetCoordSysReference(HGF2DWorld_HMRWORLD));

                    // Put the HRF raster in a store so that we can pull a HRA raster afterwards
                    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (pLog, pRasterFile, 0, pLogicalSpace);
                    HASSERT(pStore != 0);
                
                    HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
                    HASSERT(pRaster != NULL);

                    HVEShape MyShape = *pRaster->GetEffectiveShape();

                    pImportExport = new HUTImportFromRasterExportToFile(pRaster, MyShape, g_pWorldCloud);
                }
GT 6 novembre 2006
Blue Marble was desactivated
#endif
            }
            else
            {                
                WString SrcFileNameStr(pi_pSrcFilename->GetURL().c_str(), BentleyCharEncoding::Utf8);
               
                if (0 == _tcsicmp (&(SrcFileNameStr.c_str())[SrcFileNameStr.size() - 4], _TEXT(".pss")))
                {                                    
                    HFCPtr<HRARaster>      pRaster;
                    HFCPtr<HPSObjectStore> pStore;

                    // Open the picture script store
                    LoadPSS((HFCPtr<HFCURL>&)pi_pSrcFilename, pio_pPool, g_pWorldCloud, pRaster, pStore);

                    if (pRaster != 0)
                    {                                                                           
                        if ((pi_pExportProperties->CreateOnDemandMosaicPSSCache == true) &&
                            ((pRaster->IsCompatibleWith(HIMMosaic::CLASS_ID) == true) ||
                             (pRaster->IsCompatibleWith(HIMOnDemandMosaic::CLASS_ID) == true)))
                        {
                            uint64_t RasterHeightInPixels;
                            uint64_t RasterWidthInPixels;

                            GetRasterSizeInPixel(pRaster, 
                                                 g_pWorldCloud, 
                                                 RasterHeightInPixels, 
                                                 RasterWidthInPixels);
                            
                            if ((RasterHeightInPixels <= pi_pExportProperties->PSSMinimumDimensionForCache) && 
                                (RasterWidthInPixels  <= pi_pExportProperties->PSSMinimumDimensionForCache))
                            {
                                //If the mosaic PSS is very small, don't create a cache.
                                return; 
                            }    

                            //Replace the last mosaic command with an on-demand mosaic command.
                            if (pRaster->IsCompatibleWith(HIMMosaic::CLASS_ID) == true)
                            {                                                                   
                                ReplacePSSMosaicByOnDemandMosaic((HFCPtr<HFCURL>&)pi_pSrcFilename, 
                                                                 pStore);

                                //Reload the PSS with the new OnDemandMosaic statement
                                LoadPSS((HFCPtr<HFCURL>&)pi_pSrcFilename, pio_pPool, g_pWorldCloud, pRaster, pStore);

                                //The newly loaded PSS must return a HIMOnDemandMosaic.
                                HASSERT((pRaster != 0) && 
                                        (pRaster->IsCompatibleWith(HIMOnDemandMosaic::CLASS_ID) == true));
                            }
                            else
                            {
                                //Verify that a cache is present and is valid                                
                                if (IsValidPSSCacheFile((HFCPtr<HFCURL>&)pi_pSrcFilename, (HFCPtr<HIMOnDemandMosaic>&)pRaster) == true)
                                {
                                    //A valid cache file for the PSS file has been found, don't generate it again.
                                    return;
                                }
                                else
                                {
                                    //This means that the cache file was found and more recent than the PSS file
                                    //but that one file composing the mosaic is more recent than the cache file. 
                                    //Thus, remove the cache file from the mosaic to be sure not to use it during 
                                    //the creation of the new cache file.
                                    if (((HFCPtr<HIMOnDemandMosaic>&)pRaster)->HasCache() == true)
                                    {
                                        ((HFCPtr<HIMOnDemandMosaic>&)pRaster)->RemoveCache();                                        
                                    }
                                }
                            }
                            
                            HAutoPtr<HUTImportFromRasterExportToFile> pTempImportExport;
                            
                            //TDB - Pass the extent as the clip shape instead of the effective shape 
                            //      because the shape library is too slow for complex shape.                            
                            pImportExport = new HUTImportFromRasterExportToFile(pRaster,
                                                                                pRaster->GetExtent(),
                                                                                g_pWorldCloud);                

                            HASSERT(RasterHeightInPixels == pImportExport->GetImageHeight());
                            HASSERT(RasterWidthInPixels == pImportExport->GetImageWidth());
                                                    
                            Utf8String OnDemandRastersInfo;                        

                            ((HFCPtr<HIMOnDemandMosaic>&)pRaster)->GetOnDemandRastersInfo(&OnDemandRastersInfo);              

                            HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeOnDemandRastersInfo(string(OnDemandRastersInfo.c_str()));
                              
                            pImportExport->SetTag(pTag);                            
                        }
                        else
                        {
                            pImportExport = new HUTImportFromRasterExportToFile(pRaster,
                                                                                *pRaster->GetEffectiveShape(),
                                                                                g_pWorldCloud);                
                        }
                    }
                }                

                //Normal operation - Convertion from an image file to a .iTiff file.
                if ((pImportExport == 0) && 
                    (pi_pExportProperties->CreateOnDemandMosaicPSSCache == false))
                {
                    HAutoPtr<HUTImportFromFileExportToFile> pTempImportExport(new HUTImportFromFileExportToFile (g_pWorldCloud));

                    pTempImportExport->SelectImportFilename((HFCPtr<HFCURL>)pi_pSrcFilename);

                    pImportExport = pTempImportExport.release();
                }
                
                if (pImportExport == 0)
                {
                    //If the user has requested to create cache files for PSS files and the current 
                    //file is not a PSS file or not a valid PSS file, do nothing.
                    return; 
                }
            }

            //pImportExport->SelectExportFileFormat(HRFRasterFileFactory::GetInstance()->FindCreator((HFCPtr<HFCURL>)pi_pDstFilename));
            // Force to always export into iTiff fileformat.

            // Before to create the output raster file, check if we resample it.
            // If yes, calculate the output size
            if (pi_pExportProperties->CubicConvolutionSpecified)
            {
                // Set the size of the output raster file.
                pImportExport->SetImageWidth((uint32_t)pImportExport->GetDefaultResampleSize().GetX());
                pImportExport->SetImageHeight((uint32_t)pImportExport->GetDefaultResampleSize().GetY());
            }

            if (pi_pExportProperties->CreateOnDemandMosaicPSSCache == true)
            {   
                double        CacheSizeFactor;
                HFCPtr<HFCURL> pDstFileName(HRFiTiffCacheFileCreator::GetInstance()->
                                                    GetCacheURLFor((HFCPtr<HFCURL>&)pi_pSrcFilename, 0, 0));
                
                HASSERT(pDstFileName->IsCompatibleWith(HFCURLFile::CLASS_ID) == true);

                pi_pDstFilename = (HFCPtr<HFCURLFile>&)pDstFileName;
                               
                pImportExport->SelectExportFileFormat(HRFcTiffCreator::GetInstance());
                pImportExport->SelectExportFilename((HFCPtr<HFCURL>)pi_pDstFilename);
                                
                if (pi_pExportProperties->IsPSSCacheDimensionAPercentage == true)
                {                    
                    CacheSizeFactor = pi_pExportProperties->PSSCacheDimension / 100.0;                                                       
                }
                else
                {
                    uint32_t  MaxDimension = max(pImportExport->GetImageWidth(), 
                                               pImportExport->GetImageHeight());
                    CacheSizeFactor = (double)pi_pExportProperties->PSSCacheDimension / MaxDimension;                                                            
                }

                uint32_t ImageWidth  = (uint32_t)(pImportExport->GetImageWidth() * CacheSizeFactor);
                uint32_t ImageHeight = (uint32_t)(pImportExport->GetImageHeight() * CacheSizeFactor);                    

                pImportExport->SetImageWidth(ImageWidth);
                pImportExport->SetImageHeight(ImageHeight);               
            }
            else
            {
                pImportExport->SelectExportFileFormat(HRFiTiffCreator::GetInstance());
                pImportExport->SelectExportFilename((HFCPtr<HFCURL>)pi_pDstFilename);
            }

            // Set the specified information
            if (pi_pExportProperties->PixelTypeSpecified)
                pImportExport->SelectPixelType(pi_pExportProperties->PixelType);

            if (pi_pExportProperties->SubPixelTypeSpecified)
                pImportExport->SelectSubResPixelType(pi_pExportProperties->SubPixelType);
    
            if (pi_pExportProperties->CodecSpecified)
            {
                if (pi_pExportProperties->pCodec != 0)
                    pImportExport->SelectCodecSample(pi_pExportProperties->pCodec);
                else
                    pImportExport->SelectCodec(pi_pExportProperties->CodecType);
            }

            if (pi_pExportProperties->SubCodecSpecified)
            {
                if (pi_pExportProperties->pSubCodec != 0)
                     pImportExport->SelectSubResCodecSample(pi_pExportProperties->pSubCodec);
                else
                     pImportExport->SelectSubResCodec(pi_pExportProperties->SubCodecType);
            }        
    
            if (pi_pExportProperties->QualitySpecified)
            {
                if (pi_pExportProperties->Quality <= pImportExport->CountCompressionStep())
                    pImportExport->SelectCompressionQuality(pi_pExportProperties->Quality);
            }
            else
            {
                pi_pExportProperties->Quality = 80;
                if (pi_pExportProperties->Quality <= pImportExport->CountCompressionStep())
                    pImportExport->SelectCompressionQuality(pi_pExportProperties->Quality);
            }

            if (pi_pExportProperties->SubQualitySpecified)
            {
                if (pi_pExportProperties->SubQuality <= pImportExport->CountSubResCompressionStep())
                    pImportExport->SelectSubResCompressionQuality(pi_pExportProperties->SubQuality);
            }
            else
            {
                pi_pExportProperties->SubQuality = 80;
                if (pi_pExportProperties->SubQuality <= pImportExport->CountSubResCompressionStep())
                    pImportExport->SelectSubResCompressionQuality(pi_pExportProperties->SubQuality);
            }
    
#if 0 // Not supported by ITiff
            if (pi_pExportProperties->EncodingTypeSpecified)
                pImportExport->SelectEncoding(pi_pExportProperties->EncodingType);
#endif

            if (pi_pExportProperties->BlockTypeSpecified)
                pImportExport->SelectBlockType(pi_pExportProperties->BlockType);

            if (pi_pExportProperties->BlockWidthSpecified)
                pImportExport->SetBlockWidth(pi_pExportProperties->BlockWidth);

            if (pi_pExportProperties->BlockHeightSpecified)
                pImportExport->SetBlockHeight(pi_pExportProperties->BlockHeight);

            if (pi_pExportProperties->ResamplingSpecified)
                pImportExport->SelectDownSamplingMethod(pi_pExportProperties->ResamplingMethod);

            // RESAMPLING SECTION
#if 0 // Where is the class DM Mai 2004    
            if (pi_pExportProperties->CubicConvolutionSpecified)
            {
                pImportExport->SetResample(true);
                pImportExport->SetResamplingMethod((HRPFilter*)HPMFactory::GetInstance()->Create(pi_pExportProperties->CubicConvolutionFilter));
            }
#endif
            
            if (pi_pExportProperties->ForceResampling)
            {
                pImportExport->SetResample(true);
                pImportExport->SetResampleIsForce(true);
            }

            Chronometer Chrono;
	        Chrono.Start();

            HAutoPtr<ExportProgressListener> pProgressListener;
            if (pi_pExportProperties->FeedbackOn)
                pProgressListener = new ExportProgressListener();
            
            _tprintf(_TEXT(" \n"));
            bool ListenerAdded = false;
            try 
            {
                if (pi_pExportProperties->FeedbackOn)
                {
                    HUTExportProgressIndicator::GetInstance()->AddListener(pProgressListener);
                    HRADrawProgressIndicator::GetInstance()->AddListener(pProgressListener);
                    ListenerAdded = true;
                }

                pImportExport->StartExport();


                if (pi_pExportProperties->FeedbackOn)
                {
                    // Remove the progression trace
                    _tprintf(_TEXT("                                                                    \r"));               
                    HUTExportProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
                    HRADrawProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
                }
            }

            catch (...)
            {
                if (ListenerAdded)
                {
                    HUTExportProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
                    HRADrawProgressIndicator::GetInstance()->RemoveListener(pProgressListener);
                }
                throw;
            }


        #ifdef _HMR_ALTAPHOTO
            if (!pi_pExportProperties->AltaPhotoBlobName.empty())
            {
                AddBlobInFile(pi_pExportProperties, pi_pDstFilename);
            }
        #endif

            _tprintf(_TEXT("\b\b\b\b\b\b\b\b\b\bDONE -> %s Duration %s"), 
                   WString((pi_pDstFilename->GetHost() + "\\" + pi_pDstFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(),
                   Chrono.Stop().c_str());
        }


        if (pi_pExportProperties->Reproject)
        {
             HFCPtr<HRFRasterFile> pRasterFile(HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)pi_pDstFilename, false, 0));

             pRasterFile->GetPageDescriptor(0)->SetTag(new HRFAttributeProjectedCSType((unsigned short)(pi_pExportProperties->DestinationProjection)));
        }

#ifdef THUMBNAIL_GENERATOR
    }
#endif
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

uint32_t ArgumentParser(ExportPropertiesContainer* po_pExportProperties, int argc, WChar *argv[], int32_t * po_pExitCode)
{
    bool    Found                           = true;
#ifdef _HMR_ALTAPHOTO
    bool    ErrorAltaPhotoCodecIncompatible = false;
    bool    AltaPhotoCodec                  = false;
    bool    AltaPhotoCodecOptimization      = false;
#endif
    int32_t   CurrentParamPos                 = 1;
    
    while (Found)
    {
        if (CurrentParamPos >= argc)
            PrintShortUsage();
        else if (_tcsicmp(_TEXT("-?"), argv[CurrentParamPos]) == 0)
        {
             PrintExtendedUsage();
        }
        else if (_tcsicmp(_TEXT("-ss"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ScanAllSubDirectorySpecified  = true;
        }
        else if (_tcsicmp(_TEXT("-dnRetainExt"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ReplaceExtensionSpecified = false;
            po_pExportProperties->dnOptionSpecified = true;
        }
        else if (_tcsicmp(_TEXT("-dnReplaceExt"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ReplaceExtensionSpecified = true;
            po_pExportProperties->dnOptionSpecified = true;
        }
        else if (_tcsicmp(_TEXT("-ow"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->OutputOverwriteSpecified = true;
        }
        else if (_tcsicmp(_TEXT("-v"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->FeedbackOn = true;
        }

        // PIXEL TYPE SECTION
        else if (_tcsicmp(_TEXT("-pGrayscale1"), argv[CurrentParamPos]) == 0)
        {
            #ifdef _HMR_ALTAPHOTO
                if ((po_pExportProperties->CodecSpecified) && 
                    (po_pExportProperties->CodecType == HCDCodecIJG::CLASS_ID))
                    ErrorAltaPhotoCodecIncompatible = true;
                else
                {
                    po_pExportProperties->PixelType = HRPPixelTypeV1Gray1::CLASS_ID;   
                    po_pExportProperties->PixelTypeSpecified  = true;
                }
            #else
                po_pExportProperties->PixelType = HRPPixelTypeV1Gray1::CLASS_ID;   
                po_pExportProperties->PixelTypeSpecified  = true;
            #endif
        }
        else if (_tcsicmp(_TEXT("-p256Colors"), argv[CurrentParamPos]) == 0)
        {
           #ifdef _HMR_ALTAPHOTO
                if ((po_pExportProperties->CodecSpecified) && 
                    (po_pExportProperties->CodecType == HCDCodecIJG::CLASS_ID))
                    ErrorAltaPhotoCodecIncompatible = true;
                else
                {
                    po_pExportProperties->PixelType = HRPPixelTypeI8R8G8B8::CLASS_ID;    
                    po_pExportProperties->PixelTypeSpecified  = true;
                }
            #else
                po_pExportProperties->PixelType = HRPPixelTypeI8R8G8B8::CLASS_ID;    
                po_pExportProperties->PixelTypeSpecified  = true;
            #endif
        }
        else if (_tcsicmp(_TEXT("-pTrueColor24"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->PixelType = HRPPixelTypeV24R8G8B8::CLASS_ID;   
            po_pExportProperties->PixelTypeSpecified  = true;
#ifdef THUMBNAIL_GENERATOR
            po_pExportProperties->ThumbnailNbBits = 24;
#endif
        }
        else if (_tcsicmp(_TEXT("-pGrayscale8"), argv[CurrentParamPos]) == 0)
        {
            #ifdef _HMR_ALTAPHOTO
                if ((po_pExportProperties->CodecSpecified) && 
                    (po_pExportProperties->CodecType == HCDCodecIJG::CLASS_ID))
                    ErrorAltaPhotoCodecIncompatible = true;
                else
                {
                    po_pExportProperties->PixelType = HRPPixelTypeV8Gray8::CLASS_ID;   
                    po_pExportProperties->PixelTypeSpecified  = true;
                }
            #else
                po_pExportProperties->PixelType = HRPPixelTypeV8Gray8::CLASS_ID;   
                po_pExportProperties->PixelTypeSpecified  = true;
            #endif
#ifdef THUMBNAIL_GENERATOR
                po_pExportProperties->ThumbnailNbBits = 8;
#endif
        }
#ifdef __HMR_MULTI_PIXELTYPE_SUPPORTED
        else if (_tcsicmp(_TEXT("-pI1V8"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->PixelType = HRPPixelTypeV1Gray1::CLASS_ID;
            po_pExportProperties->PixelTypeSpecified = true;

            po_pExportProperties->SubPixelType = HRPPixelTypeV8Gray8::CLASS_ID;   
            po_pExportProperties->SubPixelTypeSpecified  = true;

            po_pExportProperties->SubCodecType = HCDCodecZlib::CLASS_ID;
            po_pExportProperties->SubCodecSpecified = true;
        }
#endif
        // CODEC SECTION
        else if (_tcsicmp(_TEXT("-cccitt"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CodecType = HCDCodecHMRCCITT::CLASS_ID;  
            po_pExportProperties->CodecSpecified  = true;
        }
        else if (_tcsicmp(_TEXT("-cdeflate"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CodecType = HCDCodecZlib::CLASS_ID; 
            po_pExportProperties->CodecSpecified  = true;
        }
        else if (_tcsicmp(_TEXT("-cnone"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CodecType = HCDCodecIdentity::CLASS_ID; 
            po_pExportProperties->CodecSpecified  = true;
        }
        else if (_tcsicmp(_TEXT("-cjpeg"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CodecType = HCDCodecIJG::CLASS_ID;  
            po_pExportProperties->CodecSpecified  = true;
        }
#ifdef __HMR_BTPC_SUPPORTED
        else if (_tcsicmp(_TEXT("-cbtpc"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CodecType = HCDCodecBTPC::CLASS_ID;  
            po_pExportProperties->CodecSpecified  = true;
        }
#endif
#ifdef _HMR_ALTAPHOTO
        else if (_tcsicmp(_TEXT("-cAltaPhotoJPEG"), argv[CurrentParamPos]) == 0)
        {
            if ((po_pExportProperties->PixelTypeSpecified) && 
                (po_pExportProperties->PixelType != HRPPixelTypeV24R8G8B8::CLASS_ID))
                 ErrorAltaPhotoCodecIncompatible = true;
             else
             {                 
                 AltaPhotoCodec = true;
                 HCDCodecIJG::s_UseIJL = false;
                 po_pExportProperties->CodecType = HCDCodecIJG::CLASS_ID;  
                 po_pExportProperties->CodecSpecified  = true;

                 // set the specific JPEG parameters for AltaPhoto
                 po_pExportProperties->pCodec = new HCDCodecIJGAltaPhoto();
                 ((HFCPtr<HCDCodecIJG>&)po_pExportProperties->pCodec)->SetBitsPerPixel(24);
                 po_pExportProperties->QualitySpecified = true;
                 po_pExportProperties->Quality = 50;
                 ((HFCPtr<HCDCodecIJG>&)po_pExportProperties->pCodec)->SetOptimizeCoding(AltaPhotoCodecOptimization);

                 // Force to 24 bits, because any ways, in can't be anything else.
                 po_pExportProperties->PixelType = HRPPixelTypeV24R8G8B8::CLASS_ID;   
                 po_pExportProperties->PixelTypeSpecified  = true;
             }
        }
        // AltaPhoto codec optimization
        else if (_tcsicmp(_TEXT("-hNone"), argv[CurrentParamPos]) == 0)
        {
            AltaPhotoCodecOptimization = false;
            if (AltaPhotoCodec)
                ((HFCPtr<HCDCodecIJG>&)po_pExportProperties->pCodec)->SetOptimizeCoding(false);
        }
        else if (_tcsicmp(_TEXT("-hOptimized"), argv[CurrentParamPos]) == 0)
        {
            AltaPhotoCodecOptimization = true;
            if (AltaPhotoCodec)
                ((HFCPtr<HCDCodecIJG>&)po_pExportProperties->pCodec)->SetOptimizeCoding(true);
        }

        
#endif
        // COMPRESSION QUALITY SECTION
        else if (_tcsncmp(_TEXT("-q"), argv[CurrentParamPos], 2) == 0)
        {
            _stscanf_s(argv[CurrentParamPos]+2, _TEXT("%ld"), &po_pExportProperties->Quality);
            if ((po_pExportProperties->Quality < 1) || (po_pExportProperties->Quality > 100))
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
            else
                po_pExportProperties->QualitySpecified = true;
        }                
        else if (_tcsncmp(_TEXT("-rp"), argv[CurrentParamPos], 3) == 0)
        {
            po_pExportProperties->Reproject = true;

            if( _stscanf_s(argv[CurrentParamPos]+3, 
                       _TEXT("%ld,%ld,%lf"), 
                       &po_pExportProperties->SourceProjection, 
                       &po_pExportProperties->DestinationProjection, 
                       &po_pExportProperties->MaxReprojectionError) != 3 )
            {
                // Error
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
        }
        else if (_tcsncmp(_TEXT("-rf"), argv[CurrentParamPos], 3) == 0)
        {
            po_pExportProperties->ReprojectDatabase = Utf8String(argv[CurrentParamPos]+3);
        }

        // ENCODING TYPE SECTION
#if 0 // Not supported by ITiff
        else if (_tcsicmp(_TEXT("-estandard"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->EncodingType = HRFEncodingType::STANDARD;          
            po_pExportProperties->EncodingTypeSpecified = true;
        }
#endif
        else if (_tcsicmp(_TEXT("-emultires"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->EncodingType = HRFEncodingType::MULTIRESOLUTION;   
            po_pExportProperties->EncodingTypeSpecified = true;
        }

        else if (_tcsicmp(_TEXT("-mrNearestNeighbor"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ResamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
            po_pExportProperties->ResamplingSpecified = true;
        }
        else if (_tcsicmp(_TEXT("-mrAverage"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ResamplingMethod = HRFDownSamplingMethod::AVERAGE;
            po_pExportProperties->ResamplingSpecified = true;
        }

        else if (_tcsicmp(_TEXT("-mrOring4,0"), argv[CurrentParamPos]) == 0)
        {
            // Check if it's possible to used the ORING4
            if (1)
            {
                po_pExportProperties->ResamplingMethod = HRFDownSamplingMethod::ORING4;
                po_pExportProperties->ResamplingMethod.m_ForegroundIndex = 0;
                po_pExportProperties->ResamplingSpecified = true;
            }
            else
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }

        }
        else if (_tcsicmp(_TEXT("-mrOring4,1"), argv[CurrentParamPos]) == 0)
        {
            // Check if it's possible to used the ORING4
            // if (po_pExportProperties->CodecType == HCDCodecHMRCCITT::CLASS_ID || (Src is 1Bit palette)
            if (1)
            {
                po_pExportProperties->ResamplingMethod = HRFDownSamplingMethod::ORING4;
                po_pExportProperties->ResamplingMethod.m_ForegroundIndex = 1;
                po_pExportProperties->ResamplingSpecified = true;
            }
            else
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
        }

        else if (_tcsicmp(_TEXT("-mrCopyPyramid"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ResamplingMethod = HRFDownSamplingMethod::UNKOWN;
            po_pExportProperties->ResamplingSpecified   = true;
            po_pExportProperties->ResamplingCopyPyramid = true;
        }

        // BLOCK TYPE SECTION
        else if (_tcsicmp(_TEXT("-btile"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->BlockType = HRFBlockType::TILE;      
            po_pExportProperties->BlockTypeSpecified  = true;
        }
        else if (_tcsicmp(_TEXT("-bstrip"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->BlockType = HRFBlockType::STRIP;     
            po_pExportProperties->BlockTypeSpecified  = true;
        }  
#ifdef THUMBNAIL_GENERATOR
        else if( (_tcsicmp(_TEXT("-bimage"), argv[CurrentParamPos]) == 0) )
        {
            po_pExportProperties->BlockType = HRFBlockType::IMAGE;     
            po_pExportProperties->BlockTypeSpecified  = true;
        }
#endif
        // RESAMPLING SECTION
#if 0 // Where is the class DM Mai 2004        
        else if (_tcsicmp(_TEXT("-rCubicConvolution"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CubicConvolutionSpecified = true;
            po_pExportProperties->CubicConvolutionFilter = HRPCubicConvolution::CLASS_ID;
        }
        else if (_tcsicmp(_TEXT("-rDescartesCubicConvolution"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CubicConvolutionSpecified = true;
            po_pExportProperties->CubicConvolutionFilter = HRPDescartesCubicConvolution::CLASS_ID;
        }
       
        else if (_tcsicmp(_TEXT("-rHMRCubicConvolution"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->CubicConvolutionSpecified = true;
            po_pExportProperties->CubicConvolutionFilter = HRPHMRCubicConvolution::CLASS_ID;
        }
#endif 
        else if (_tcsicmp(_TEXT("-rForceResampling"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->ForceResampling = true;
        }
        else if (_tcsncmp(_TEXT("-pool:"), argv[CurrentParamPos], 6) == 0)
        {
            _stscanf_s(argv[CurrentParamPos]+6, _TEXT("%ld"), &po_pExportProperties->PoolSizeInKB);
            if (po_pExportProperties->PoolSizeInKB < 16192)
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
        }
        else if (_tcsicmp(_TEXT("-cpss"), argv[CurrentParamPos]) == 0)        
        {
            po_pExportProperties->CreateOnDemandMosaicPSS = true;
        }
        else if (_tcsicmp(_TEXT("-cpssc"), argv[CurrentParamPos]) == 0)        
        {
            po_pExportProperties->CreateOnDemandMosaicPSSCache = true;
        }
        else if (_tcsncmp(_TEXT("-cdim"), argv[CurrentParamPos], 5) == 0)        
        {            
            int PSSCacheDim = _tstoi(&(argv[CurrentParamPos][5]));

            //Invalid argument
            if (PSSCacheDim <= 0)
            {                
              po_pExportProperties->WrongArgumentFound = true;
              PrintArgumentError(argv[CurrentParamPos]);
              (*po_pExitCode) = 1;                
            }
            else
            {
                if (_tcsstr(&(argv[CurrentParamPos][5]), _TEXT("%")) != 0)
                {
                    po_pExportProperties->IsPSSCacheDimensionAPercentage = true;
                }
                else
                {
                    po_pExportProperties->IsPSSCacheDimensionAPercentage = false;
                }

                po_pExportProperties->PSSCacheDimension = PSSCacheDim;    
            }                        
        }        
        else if (_tcsncmp(_TEXT("-mdim"), argv[CurrentParamPos], 5) == 0)        
        {
            int PSSMinDimForCache = _tstoi(&(argv[CurrentParamPos][5]));

            //Invalid argument
            if (PSSMinDimForCache <= 0)
            {
              po_pExportProperties->WrongArgumentFound = true;
              PrintArgumentError(argv[CurrentParamPos]);
              (*po_pExitCode) = 1;                
            }
            else
            {                
                po_pExportProperties->PSSMinimumDimensionForCache = PSSMinDimForCache;                    
            }            
        }
#ifdef THUMBNAIL_GENERATOR
        else if ( _tcsncmp(_TEXT("-fjpeg"), argv[CurrentParamPos], 6) == 0 )
        {
            po_pExportProperties->FileFormatSpecified = true;
            po_pExportProperties->FileFormat = _TEXT("jpg");
        }
        else if ( _tcsncmp(_TEXT("-fitiff"), argv[CurrentParamPos], 6) == 0 )
        {
            po_pExportProperties->FileFormatSpecified = true;
            po_pExportProperties->FileFormat = _TEXT("itiff");
        }
        else if ( _tcsncmp(_TEXT("-f"), argv[CurrentParamPos], 2) == 0 )
        {
            po_pExportProperties->WrongArgumentFound = true;
            PrintArgumentError(argv[CurrentParamPos]);
            (*po_pExitCode) = 1;
        }
        else if (_tcsncmp(_TEXT("-t"), argv[CurrentParamPos], 2) == 0)
        {
            po_pExportProperties->ThumbnailSpecified = true;

            if( _stscanf(argv[CurrentParamPos]+2, _TEXT("%ld,%ld"), &po_pExportProperties->ThumbnailWidth, &po_pExportProperties->ThumbnailHeight) == 2 )
            {
                po_pExportProperties->ThumbnailSizeSpecified = true;
            }
            else
            {
                po_pExportProperties->ThumbnailSizeSpecified = true;
                po_pExportProperties->ThumbnailWidth         = 96;
                po_pExportProperties->ThumbnailHeight        = 96;
            }
        }
        else if (_tcsncmp(_TEXT("-bgc"), argv[CurrentParamPos], 4) == 0)
        {
            po_pExportProperties->BackgroundSpecified = true;
            char Buffer[255];
            
            if( _stscanf(argv[CurrentParamPos] + 4, _TEXT("%s"), Buffer) == 1 )
            {
                uint32_t Red = 0, Green = 0, Blue = 0;

                _stscanf(Buffer, _TEXT("%ld,%ld,%ld"), &Red, &Green, &Blue);

                po_pExportProperties->BackgroundRed   = Red;
                po_pExportProperties->BackgroundGreen = Green;
                po_pExportProperties->BackgroundBlue  = Blue;
            }
            else
            {
                po_pExportProperties->BackgroundRed   = 0;
                po_pExportProperties->BackgroundGreen = 0;
                po_pExportProperties->BackgroundBlue  = 0;
            }
        }
#endif
#ifdef _HMR_ALTAPHOTO
        else if (_tcsncmp("-ab"), argv[CurrentParamPos], 3) == 0)
        {
            if (_tcslen(argv[CurrentParamPos]+3) > 0)
            {
                po_pExportProperties->AltaPhotoBlobName = (argv[CurrentParamPos]+3);
            }
            else
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
        }
        // For debugging only, not in documentation
        else if (_tcsicmp("-AltaPhotoDumpBlob"), argv[CurrentParamPos]) == 0)
        {
            po_pExportProperties->AtlaPhotoDumpBlob  = true;
        }
        else if (_tcsncmp("-af"), argv[CurrentParamPos], 3) == 0)
        {
            
            double FirstValue;
            double SecondValue;
            double ThirdValue;
            
            if (GetFilterValueFromString(Utf8String(argv[CurrentParamPos]), 
                                          &FirstValue,
                                          &SecondValue,
                                          &ThirdValue))
            {
                ;
                // Set the filter value...
                ;
            }
            else
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }

        }
        else if (_tcsncmp("-at"), argv[CurrentParamPos], 3) == 0)
        {
            double ThresholdValue;
            bool   ValidThresholdValue = false;
            uint32_t Index = 2;
            
            if (ExtractValueFromStringAtIndex(Utf8String(argv[CurrentParamPos]), &Index, &ThresholdValue))
            {
                if ((ThresholdValue >= 0.0) && (ThresholdValue <= 255.0))
                {
                    // Set the Threshold value
                    ValidThresholdValue = true;
                }
            }
            if (!ValidThresholdValue)
            {
                po_pExportProperties->WrongArgumentFound = true;
                PrintArgumentError(argv[CurrentParamPos]);
                (*po_pExitCode) = 1;
            }
        }
#endif //_HMR_ALTAPHOTO
        // If nothing valid have been previously found...
        else if (_tcsncmp(_TEXT("-"), argv[CurrentParamPos], 1) == 0)
        {
            po_pExportProperties->WrongArgumentFound = true;
            PrintArgumentError(argv[CurrentParamPos]);
            (*po_pExitCode) = 1;
        }
        else
            Found = false;

        // Increment the current position and check that it's valid.
        if (Found)
        {
            CurrentParamPos++;
            if (CurrentParamPos >= argc)
                PrintShortUsage();
        }
    }
#ifdef _HMR_ALTAPHOTO
    if (ErrorAltaPhotoCodecIncompatible)
    {
        po_pExportProperties->WrongArgumentFound = true;
        PrintArgumentError("-cAltaPhotoJPEG");
        (*po_pExitCode) = 1;
    }
#endif //_HMR_ALTAPHOTO

#ifdef THUMBNAIL_GENERATOR
    CheckJpegDefault(po_pExportProperties);
    CheckJpegError(po_pExportProperties, po_pExitCode);
#endif        

    return CurrentParamPos;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool IsDirectory(HFCPtr<HFCURLFile>& pi_pSrcFilename)
{
    bool IsDir = false;
    struct _stat StatBuffer;

    int ErrorFound = _tstat(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(), &StatBuffer);
    
    if ( (!ErrorFound) && ((StatBuffer.st_mode & _S_IFDIR) == _S_IFDIR))
        IsDir = true;
    
    return IsDir;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool IsWildcardFound(HFCPtr<HFCURLFile>& pi_pSrcFilename)
{
    bool IsFound = false;
    
    if ((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).find_first_of('*') != -1)
        IsFound = true;

    return IsFound;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool IsRelativePath(HFCPtr<HFCURLFile>& pi_pSrcFilename)
{
    bool IsRelative = false;
    Utf8String TempFileName;

    if (!pi_pSrcFilename->GetPath().empty())
        TempFileName = pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath();
    else
        TempFileName = pi_pSrcFilename->GetHost();
    
    if (TempFileName.find_first_of(':') == -1)
        IsRelative = true;

    return IsRelative;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

HFCPtr<HFCURLFile> UnRelativePath(HFCPtr<HFCURLFile>& pi_pSrcFilename)
{
    bool IsRelative = IsRelativePath(pi_pSrcFilename);
    HFCPtr<HFCURLFile> pFilename;

    WChar FileName[_MAX_PATH];
    
    if (IsRelative)
    {
        if (!pi_pSrcFilename->GetPath().empty())
        {
            if(_tfullpath(FileName,
                          WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(),
                          _MAX_PATH ) != NULL)

                // Add it to the relative path
                pFilename  = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(FileName));
        }
        else
        {
            if(_tfullpath(FileName, WString(pi_pSrcFilename->GetHost().c_str(), BentleyCharEncoding::Utf8).c_str(), _MAX_PATH ) != NULL)
                // Add it to the relative path
                pFilename  = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(FileName));
        }
    }
    else
        pFilename = new HFCURLFile(pi_pSrcFilename->GetURL());
    
    return pFilename;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool IsFileNameExist(HFCPtr<HFCURLFile>& pi_pSrcFilename)
{
    bool Existance = false;
    WString NewName;
    WChar FileName[_MAX_FNAME];
    WChar Dir[_MAX_DIR];
    WChar Ext[_MAX_EXT];
    struct _stat StatBuffer;

    int ErrorFound = -1;
    if ((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).find_first_of('*') == -1)
    {
        // If no wildcard has been used...
        ErrorFound = _tstat(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(),  &StatBuffer);
    }
    else
    {
        // Be sure to remove any wildcard extention, before executing Stat!
        _tsplitpath_s(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(), 0, 0, Dir, _MAX_DIR, FileName, _MAX_FNAME, Ext, _MAX_EXT);
        
        NewName =  WString(pi_pSrcFilename->GetHost().c_str(), BentleyCharEncoding::Utf8);
        NewName += Dir;

        if (!NewName.empty())
            ErrorFound = _tstat(NewName.c_str(),  &StatBuffer);
    }

    if (!ErrorFound)
        Existance = true;

    return Existance;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

HFCPtr<HFCURLFile> ConvertName(HFCPtr<HFCURLFile>& pi_pSrcFilename, const Utf8String& pi_rNewExt)
{
    WString NewName;
    WChar FileName[_MAX_FNAME];
    WChar Dir[_MAX_DIR];
    WChar Ext[_MAX_EXT];

    _tsplitpath_s(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(), 0, 0, Dir, _MAX_DIR, FileName, _MAX_FNAME, Ext, _MAX_EXT);

    NewName =  WString(pi_pSrcFilename->GetHost().c_str(), BentleyCharEncoding::Utf8);
    NewName += Dir;
    NewName += FileName;
    NewName += _TEXT("_");
    NewName += (Ext + 1);
    NewName += _TEXT(".");
    NewName += WString(pi_rNewExt.c_str(), BentleyCharEncoding::Utf8);

    HFCPtr<HFCURLFile> pFilename = new HFCURLFile("file://" + Utf8String(NewName));
    return pFilename;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

HFCPtr<HFCURLFile> ConvertExtension(HFCPtr<HFCURLFile>& pi_pSrcFilename, const Utf8String& pi_rNewExt)
{
    WString NewName;
    WChar FileName[_MAX_FNAME];
    WChar Dir[_MAX_DIR];
    WChar Ext[_MAX_EXT];

    _tsplitpath_s(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(), 0, 0, Dir, _MAX_DIR, FileName, _MAX_FNAME, Ext, _MAX_EXT);

    NewName =  WString(pi_pSrcFilename->GetHost().c_str(), BentleyCharEncoding::Utf8);
    NewName += Dir;
    NewName += FileName;
    NewName += _TEXT(".");
    NewName += WString(pi_rNewExt.c_str(), BentleyCharEncoding::Utf8);

    HFCPtr<HFCURLFile> pFilename = new HFCURLFile("file://" + Utf8String(NewName));
    return pFilename;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

HFCPtr<HFCURLFile> ComposeFileNameWithNewPath(HFCPtr<HFCURLFile>& pi_pSrcFilename, HFCPtr<HFCURLFile>& pi_pDestPath)
{
    Utf8String NewName;
    WChar FileName[_MAX_FNAME];
    WChar Dir[_MAX_DIR];
    WChar Ext[_MAX_EXT];
    
    NewName = pi_pDestPath->GetHost() + "\\" + pi_pDestPath->GetPath();
    if (NewName[NewName.size() - 1] != '\\' && NewName[NewName.size() - 1] != '/')
        NewName += '\\';
    
    _tsplitpath_s(WString((pi_pSrcFilename->GetHost() + "\\" + pi_pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str(),
                0, 0, Dir, _MAX_DIR, FileName, _MAX_FNAME, Ext, _MAX_EXT);
    
    NewName += Utf8String(FileName);
    NewName += Utf8String(Ext);

    HFCPtr<HFCURLFile> pFilename = new HFCURLFile("file://" + NewName);
    return pFilename;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool ValidateIfCopyPyramidIsChoose(HFCPtr<HFCURLFile>& pi_pSrcFilename,
                                    const ExportPropertiesContainer& pi_rExportProperties)
{
    bool Ret = true;
    WString Msg;

    // Ask CopyPyramid
    if (pi_rExportProperties.ResamplingCopyPyramid)
    {
        if (pi_rExportProperties.EncodingType != HRFEncodingType::MULTIRESOLUTION)
        {
            Msg = _TEXT("E014 -mrCopyPyramid requires the source to be multi-resolution images.");
            Ret = false;
        }
        else
        {
            HFCPtr<HRFRasterFile> pSourceFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>&)pi_pSrcFilename, true);
            if (!((pSourceFile->CountPages() > 0) && 
                  (pSourceFile->GetPageDescriptor(0)->CountResolutions() > 1)))
            {
                Msg = _TEXT("E015 -mrCopyPyramid requires the destination to be multi-resolution images.");
                Ret = false;
            }
        }
#ifdef _HMR_ALTAPHOTO
        if (pi_rExportProperties.ConvolutionFilterSpecified)
        {
            Msg = _TEXT("E501 -mrCopyPyramid invalid when -af specified.");
            Ret = false;
        }
#endif
        // If Error found.
        if (!Ret)
        {
            cout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
        }
    }
    return Ret;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void PrintArgumentError(const WChar* pi_pInvalidArgument)
{
    WString Msg;
    
    if (_tcsncmp(_TEXT("-pool:"), pi_pInvalidArgument, 5) == 0)
    {
        Msg = _TEXT("E016 Pool size must be superior to 16192: ");
        Msg += pi_pInvalidArgument;
    }
#ifdef _HMR_ALTAPHOTO
    else if (_tcsncmp(_TEXT("-cAltaPhotoJPEG"), pi_pInvalidArgument, 12) == 0)
    {
        Msg = _TEXT("E006 Specified codec is not compatible with the selected color space");
        Msg += pi_pInvalidArgument;
    }
    else if (_tcsncmp(_TEXT("-at"), pi_pInvalidArgument, 3) == 0)
    {
        Msg = _TEXT("E503 Filter threshold must be between 0 and 255 ");
        Msg += pi_pInvalidArgument;
    }
#endif
    else if (_tcsncmp(_TEXT("-p"), pi_pInvalidArgument, 2) == 0)
    {
        Msg = _TEXT("E004 Invalid color space specified : ");
        Msg += pi_pInvalidArgument;
    }
    else if (_tcsncmp(_TEXT("-c"), pi_pInvalidArgument, 2) == 0)
    {
        Msg = _TEXT("E005 Invalid codec specified :");
        Msg += pi_pInvalidArgument;
    }
    else if (_tcsncmp(_TEXT("-q"), pi_pInvalidArgument, 2) == 0)
        Msg = _TEXT("E007 Quality must be between 1 and 100.");

#if 0 // Not supported by ITiff
    else if (_tcsncmp(_TEXT("-e"), pi_pInvalidArgument, 2) == 0)
    {
        Msg = _TEXT("E008 Invalid encoding specified : ");
        Msg += pi_pInvalidArgument;
    }
#endif

    else if (_tcsncmp(_TEXT("-b"), pi_pInvalidArgument, 2) == 0)
    {
        Msg = _TEXT("E009 Invalid block type specified : ");
        Msg += pi_pInvalidArgument;
    }
    else if (_tcsncmp(_TEXT("-mr"), pi_pInvalidArgument, 3) == 0)
    {
        Msg = _TEXT("E012 Invalid multiresolution resampling method specified : ");
        Msg += pi_pInvalidArgument;
    }
    else if (_tcsncmp(_TEXT("-f"), pi_pInvalidArgument, 2) == 0)
    {
        Msg = _TEXT("E021 Invalid output file format specified : ");
        Msg += pi_pInvalidArgument;
    }    
    else
    {
        Msg = _TEXT("E000 Invalid argument specified : ");
        Msg += pi_pInvalidArgument;
    }

    wcout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool BuildPathTree(HFCPtr<HFCURLFile>& pi_pCompletePath)
{
    bool BuildSuccess = true;
    
    unsigned int LeafLevel = 0;
    unsigned int LeafCount = 0;;
    Utf8String Leaf;
    
    LeafCount = GetLeafCount(pi_pCompletePath->GetHost() + "\\" + pi_pCompletePath->GetPath());
    Leaf = GetLeaf(Utf8String(pi_pCompletePath->GetHost() + "\\" + pi_pCompletePath->GetPath()).c_str(), LeafLevel);

    do
    {
        HFCPtr<HFCURLFile> aFilePath(new HFCURLFile("file://" + Leaf + '\\'));
        HFCPtr<HFCURLFile> aFile(new HFCURLFile("file://" + Leaf));
        if (!IsFileNameExist(aFilePath) && !IsFileNameExist(aFile))
        {
            if (_tmkdir(WString(Leaf.c_str(), BentleyCharEncoding::Utf8).c_str()))
                BuildSuccess = false;
        }
        Utf8String completePath(pi_pCompletePath->GetHost() + "\\" + pi_pCompletePath->GetPath());
        Leaf = GetLeaf(completePath, ++LeafLevel);
    }
    while(BuildSuccess && (LeafCount > LeafLevel));

    return BuildSuccess;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

Utf8String GetLeaf(Utf8String CompletePath, int LeafLevel)
{
    Utf8String NewPath;

    size_t Pos = CompletePath.find_first_of("\\");
    for (int Level=0; Level<LeafLevel; ++Level)
    {
        ++Pos;
        Pos = CompletePath.find_first_of("\\", Pos);
    }
    
    NewPath = CompletePath.substr(0,Pos);

    return NewPath;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

unsigned int GetLeafCount(Utf8String CompletePath)
{
    unsigned int LeafCount = 0;    
    size_t Pos = 0;

    do
        Pos = CompletePath.find_first_of("\\", Pos);
    while (Pos > 0 && ++LeafCount && ++Pos);
    
    return LeafCount;
}

//-----------------------------------------------------------------------------
// Ex.: URL("D:\Images\Jpeg\toto.jpg") - URL("D:\Images\") return: "\Jpeg\"
//-----------------------------------------------------------------------------

Utf8String PathInverseSubstractor(HFCPtr<HFCURLFile>& pi_pCompletePath, 
                                          HFCPtr<HFCURLFile>& pi_pPathPartToRemove)
{
    
    Utf8String CompletePath(pi_pCompletePath->GetHost() + "\\" + pi_pCompletePath->GetPath());
    Utf8String PathPartToRemove(pi_pPathPartToRemove->GetHost() + "\\" + pi_pPathPartToRemove->GetPath());
    Utf8String NewPath = "";

    
    size_t SizeToRemove = PathPartToRemove.size();
    size_t WildCardPosition = PathPartToRemove.find_first_of('*');
    size_t DotPosition      = CompletePath.find_first_of('.');
    size_t CompletePathSize = CompletePath.size();

    if (WildCardPosition != -1)
        SizeToRemove = WildCardPosition - 1;

    if (DotPosition != -1)
        CompletePathSize = CompletePath.find_last_of('\\', DotPosition);
    
    for (size_t Index=SizeToRemove; Index < CompletePathSize; Index++)
        NewPath += CompletePath[Index];
    
    return NewPath;
}


//-----------------------------------------------------------------------------
// GetResampleImageSize
//-----------------------------------------------------------------------------
void GetResampleImageSize(const HFCPtr<HRFRasterFile>&      pi_rpRasterFile, 
                          const HFCPtr<HGF2DWorldCluster>&  pi_rpWorldCluster,                                            
                          uint32_t*                           po_pWidth, 
                          uint32_t*                           po_pHeight)
{
	// A raster must be provided
    HPRECONDITION(pi_rpRasterFile != 0);
    
	// Raster file may not be empty
    HPRECONDITION(pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() != 0.0 &&
                  pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() != 0.0);
    
    HFCPtr<HGF2DCoordSys> pPhysicalCoordSys = pi_rpWorldCluster->GetCoordSysReference(pi_rpRasterFile->GetWorldIdentificator());
    HFCPtr<HGF2DTransfoModel> pModel;
    // get transfo model
    if (pi_rpRasterFile->GetPageDescriptor(0)->HasTransfoModel())
        pModel = pi_rpRasterFile->GetPageDescriptor(0)->GetTransfoModel();
    else
        pModel = new HGF2DIdentity();

    // calculate the output shape
    HFCPtr<HGF2DCoordSys> pLogicalCoordSys = new HGF2DCoordSys(*pModel, pPhysicalCoordSys);

    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
    CHECK_HUINT64_TO_HDOUBLE_CONV(pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())

    HVEShape Shape(0, 
                   0, 
                   (double)pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(),
                   (double)pi_rpRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight(),
                   pPhysicalCoordSys);
    Shape.ChangeCoordSys(pLogicalCoordSys);
    HGF2DExtent Extent = Shape.GetExtent();
    Shape = HVEShape(Extent);
    Shape.ChangeCoordSys(pPhysicalCoordSys);
    Extent = Shape.GetExtent();

    *po_pWidth  = (uint32_t)Extent.GetWidth();
    *po_pHeight = (uint32_t)Extent.GetHeight();
}


#ifdef THUMBNAIL_GENERATOR

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void CheckJpegDefault(ExportPropertiesContainer* po_pExportProperties)
{
    if( !po_pExportProperties->PixelTypeSpecified )
    {
        po_pExportProperties->PixelTypeSpecified = true;
        po_pExportProperties->PixelType = HRPPixelTypeV24R8G8B8::CLASS_ID;
        po_pExportProperties->ThumbnailNbBits = 24;
    }

    if( !po_pExportProperties->CodecSpecified )
    {
        po_pExportProperties->CodecSpecified = true;
        po_pExportProperties->CodecType = HCDCodecIJG::CLASS_ID;
        
        po_pExportProperties->Quality = 85;
        po_pExportProperties->QualitySpecified = true;
    }

    if( !po_pExportProperties->EncodingTypeSpecified )
    {
        po_pExportProperties->EncodingTypeSpecified = true;
        po_pExportProperties->EncodingType = HRFEncodingType::STANDARD;
    }

    if( !po_pExportProperties->BlockTypeSpecified )
    {
        po_pExportProperties->BlockTypeSpecified = true;
        po_pExportProperties->BlockType = HRFBlockType::IMAGE;
    }

    if( po_pExportProperties->ThumbnailSpecified )
    {
        if( !po_pExportProperties->ThumbnailSizeSpecified )
        {
            po_pExportProperties->ThumbnailWidth  = 96;
            po_pExportProperties->ThumbnailHeight = 96;
            po_pExportProperties->ThumbnailSizeSpecified = true;
        }
        if( !po_pExportProperties->FileFormatSpecified )
        {
            po_pExportProperties->FileFormatSpecified = true;
            po_pExportProperties->FileFormat = _TEXT("jpg");
        }
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void CheckJpegError(ExportPropertiesContainer* pi_pExportProperties, int32_t * po_pExitCode)
{
    WString Msg;
    WString Format = pi_pExportProperties->FileFormat;
    ctype<char>().tolower(Format.begin(), Format.end());
    
    if( pi_pExportProperties->FileFormatSpecified &&
        Format == _TEXT("jpg" )
    {
        if( !(pi_pExportProperties->PixelType == HRPPixelTypeV8Gray8::CLASS_ID ||
              pi_pExportProperties->PixelType == HRPPixelTypeV24R8G8B8::CLASS_ID) )
        {
            Msg = _TEXT("E016 Invalid color space specified for this format. [-p]");
            cout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
            (*po_pExitCode) = 1;
            pi_pExportProperties->WrongArgumentFound = true;
        }
        
        if( pi_pExportProperties->CodecType != HCDCodecIJG::CLASS_ID )
        {
            Msg = _TEXT("E017 Invalid codec selected for this format. [-c]");
            cout << endl << _TEXT("FAILED : " << Msg.c_str() << endl;
            (*po_pExitCode) = 1;
            pi_pExportProperties->WrongArgumentFound = true;
        }
        
        if( pi_pExportProperties->BlockType != HRFBlockType::IMAGE )
        {
            Msg = _TEXT("E018 Invalid encoding selected for this format. [-b]");
            cout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
            (*po_pExitCode) = 1;
            pi_pExportProperties->WrongArgumentFound = true;
        }

        if( pi_pExportProperties->EncodingType != HRFEncodingType::STANDARD )
        {
            Msg = _TEXT("E019 Invalid block type selected for this format. [-e]");
            cout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
            (*po_pExitCode) = 1;
            pi_pExportProperties->WrongArgumentFound = true;
        }

        if( pi_pExportProperties->ResamplingSpecified )
        {
            Msg = _TEXT("E020 Invalid resampling method specified. [-r]");
            cout << endl << _TEXT("FAILED : ") << Msg.c_str() << endl;
            (*po_pExitCode) = 1;
            pi_pExportProperties->WrongArgumentFound = true;
        }

        if( pi_pExportProperties->ThumbnailSizeSpecified )
        {
            if( pi_pExportProperties->ThumbnailWidth > 256 ||
                pi_pExportProperties->ThumbnailHeight > 256)
            {
                Msg = _TEXT("E022 Invalid thumbnail dimensions specified. [-t]");
                cout << endl << _TEXT("FAILED : " << Msg.c_str() << endl;
                (*po_pExitCode) = 1;
                pi_pExportProperties->WrongArgumentFound = true;
            }
        }
        
        if( pi_pExportProperties->BackgroundSpecified )
        {
            if( pi_pExportProperties->BackgroundRed > 255 ||
                pi_pExportProperties->BackgroundGreen > 255 ||
                pi_pExportProperties->BackgroundBlue > 255 )
            {
                Msg = _TEXT("E023 Invalid background colour specified. [-bgc] ");
                cout << endl << _TEXT("FAILED : " << Msg.c_str() << endl;
                (*po_pExitCode) = 1;
                pi_pExportProperties->WrongArgumentFound = true;
            }
        }
    }
    else
    {
        pi_pExportProperties->WrongArgumentFound = true;
        (*po_pExitCode) = 1;
    }
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool CreateThumbnail(HFCPtr<HFCURLFile>& pi_rpINURLFile,
                      HFCPtr<HFCURLFile>& pi_rpOUTURLFile,
                      ExportPropertiesContainer* pi_pProperties)
{
    bool Status  = false;
    uint32_t Width  = pi_pProperties->ThumbnailWidth;
    uint32_t Height = pi_pProperties->ThumbnailHeight;

    HFCPtr<HRFRasterFile> pRasterFile;
    
    pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>&)pi_rpINURLFile);

    if ((pRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
        (pRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID))       )
    {
        if (HRFSLOStripAdapter::NeedSLOAdapterFor(pRasterFile))
        {
            // Adapt only when the raster file has not a standard scan line orientation
            // i.e. with an upper left origin, horizontal scan line.
            pRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pRasterFile);
        }
    }
                           
    HFCPtr<HRFThumbnail> pThumbnail;
    pThumbnail = HRFThumbnailMaker(pRasterFile, 
                                   0, 
                                   &Width, 
                                   &Height, 
                                   true);

    HFCBuffer SrcBuffer(1024);
    HFCBuffer DstBuffer(1024);

    if( ConvertPixelType(pThumbnail,
                         pi_pProperties->PixelType,
                         SrcBuffer) )
    {
        if( ConvertToJPEG(SrcBuffer.GetData(),
                          pThumbnail->GetSizeInBytes(),
                          DstBuffer,
                          Width, 
                          Height, 
                          pi_pProperties->Quality,
                          pi_pProperties->ThumbnailNbBits) )
        {
            Utf8String OutputFile;

            OutputFile  = pi_rpOUTURLFile->GetHost();
            OutputFile += "\\";
            OutputFile += pi_rpOUTURLFile->GetPath();

            FILE* pFile = fopen(OutputFile.c_str(), "wb");
    
            if( pFile )
            {
                fwrite(DstBuffer.GetData(), DstBuffer.GetDataSize(), 1, pFile);
                fclose(pFile);
                Status = true;
            }
            else
                Status = false;
        }
    }

    return Status;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool ConvertPixelType(HFCPtr<HRFThumbnail> pi_pThumbnail,
                       HPMClassKey pi_DstPixelTypeClasskey,
                       HFCBuffer& po_rBuffer)
{
    bool Status = false;
    HFCPtr<HRPPixelType> pDstPixelType;
    
    // Create destination pixel type
    if( pi_DstPixelTypeClasskey == HRPPixelTypeV24R8G8B8::CLASS_ID )
    {
        Status = true;
        pDstPixelType = new HRPPixelTypeV24R8G8B8;
    }
    else if( pi_DstPixelTypeClasskey == HRPPixelTypeV8Gray8::CLASS_ID )
    {
        Status = true;
        pDstPixelType = new HRPPixelTypeV8Gray8;
    }
    else
        Status = false;

    // Create the converter
    if( Status )
    {
        HArrayAutoPtr<Byte> pSrcPixels;
        pSrcPixels = new Byte[pi_pThumbnail->GetSizeInBytes()];
        pi_pThumbnail->Read(pSrcPixels);

        HFCPtr<HRPPixelConverter> pConverter(pi_pThumbnail->GetPixelType()->GetConverterTo(pDstPixelType));

        uint32_t BitsAlignment = 8;
        uint32_t DstBytesPerWidth = ((pDstPixelType->CountPixelRawDataBits() * pi_pThumbnail->GetWidth()) + (BitsAlignment-1)) / BitsAlignment * BitsAlignment / 8;
        uint32_t SrcBytesPerWidth = ((pi_pThumbnail->GetPixelType()->CountPixelRawDataBits() * pi_pThumbnail->GetWidth()) + (BitsAlignment-1)) / BitsAlignment * BitsAlignment / 8;

        uint32_t DstUncompressedDataSize = DstBytesPerWidth * pi_pThumbnail->GetHeight();
    
	    HArrayAutoPtr<Byte>  pDstPixels(new Byte[DstUncompressedDataSize]);
	    Byte* pSrcBuffer = pSrcPixels;
        Byte* pDstBuffer = pDstPixels;

	    // Convert each line
        for(uint32_t LineIndex = 0; LineIndex < pi_pThumbnail->GetHeight(); LineIndex++)
	    {
		    pConverter->Convert(pSrcBuffer, pDstBuffer, pi_pThumbnail->GetWidth());
		    pDstBuffer += DstBytesPerWidth;
		    pSrcBuffer += SrcBytesPerWidth;
	    }
    
        po_rBuffer.AddData(pDstPixels.get(), DstUncompressedDataSize);

        Status = true;
    }

    return Status;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

bool ConvertToJPEG(const Byte*   pi_pData,
                    size_t         pi_DataSize,
                    HFCBuffer&     po_rBuffer,
                    size_t         pi_Width,
                    size_t         pi_Height,
                    Byte         pi_Quality,
                    Byte         pi_Bits)
{
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);
    bool Result = false;

    // Clear the buffer
    po_rBuffer.Clear();

    // Create the code
    HFCPtr<HCDCodecImage> 
        pCodec = new HCDCodecIJG(pi_Width, pi_Height, pi_Bits);
    ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality(pi_Quality);
    pCodec->Reset();
    ((HFCPtr<HCDCodecIJG>&)pCodec)->SetAbbreviateMode(false);

    // Prepare the JPEG buffer for the compressed data
    size_t BufferSize = max(1024, pCodec->GetSubsetMaxCompressedSize());
    Byte* pBuffer    = po_rBuffer.PrepareForNewData(BufferSize);
    
    // Build an uncompressed packet around the input data
    HCDPacket Uncompressed((Byte*)pi_pData, pi_DataSize, pi_DataSize);

    // Prepare a compressed packet around the ouput data (the JPEG buffer)
    HCDPacket Compressed((HFCPtr<HCDCodec>&)pCodec, pBuffer, BufferSize);

    // Compress
    if (Result = Uncompressed.Compress(&Compressed))
        po_rBuffer.SetNewDataSize(Compressed.GetDataSize());

    return Result;
}

#endif

#ifdef _HMR_ALTAPHOTO

//-----------------------------------------------------------------------------
// Add the Blob to the file
//-----------------------------------------------------------------------------

bool AddBlobInFile(const ExportPropertiesContainer* pi_pExportProperties, 
                   HFCPtr<HFCURLFile>&              pi_pDstFilename)
{
    bool BlobInsertSuccess = false;
    Utf8String  Msg = "FAILED : ";

    HFCPtr<HRFRasterFile> pSource;
    
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>&)pi_pDstFilename);
    if ((pSource != 0) && (pSource->GetClassID() == HRFiTiffFile::CLASS_ID))
    {
        vector<Byte> Data;

        FILE* pBlob = fopen(pi_pExportProperties->AltaPhotoBlobName.c_str(), _TEXT("rb");
        if (pBlob != 0)
        {
            fseek(pBlob, 0, SEEK_END);
            uint32_t DataSize(ftell(pBlob));
            Data.resize(DataSize);

            fseek(pBlob, 0, SEEK_SET);
            fread(&(Data[0]), 1, DataSize, pBlob);

            fclose(pBlob);

            ((HFCPtr<HRFiTiffFile>&)pSource)->Write_AltaPhotoBlob(Data);
            
            // SUCESS:  Blob file was inserted
            /*
            Msg =  "SUCESS : Blob ");
            Msg +=  pi_pExportProperties->AltaPhotoBlobName.c_str();
            Msg +=  " has been correctly insert into ");
            Msg +=  Utf8String(pi_pDstFilename->GetHost() + "\\" + pi_pDstFilename->GetPath());
            */
            BlobInsertSuccess = true;;
        }
        else
        {
            // FAILED : Blob file not found!
            Msg += "E501 : Blob input file ";
            Msg +=  pi_pExportProperties->AltaPhotoBlobName.c_str();
            Msg +=  " not found.";
            cout << Msg.c_str() << endl;
        }
    }
    else
    {
        // Invalid source File
        HASSERT(false);
        /*
        if (pSource)
        {
            // FAILED : Blob can only be insert into iTiff File
            Msg += "E500 : Blob can only be insert into an existing iTiff File");
        }
        else
        {
            // FAILED : iTiff dst file not found.
            Msg += "E500 : iTiff file ");
            Msg += Utf8String(pi_pDstFilename->GetHost() + "\\" + pi_pDstFilename->GetPath());
            Msg += " not found");
        }
        */
    }
    //cout << Msg.c_str() << endl;
    return BlobInsertSuccess;
}

//-----------------------------------------------------------------------------
// Debuging code for blob file insertion.
//-----------------------------------------------------------------------------

void DumpBlobInFile (HFCPtr<HFCURLFile>& pi_pSrcFilename,
                     HFCPtr<HFCURLFile>& pi_pDstFilename)
{
    HFCPtr<HRFRasterFile> pSource;
    
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)pi_pSrcFilename);
    if ((pSource != 0) && (pSource->GetClassID() == HRFiTiffFile::CLASS_ID))
    {
        vector<Byte> Data;

        ((HFCPtr<HRFiTiffFile>&)pSource)->Read_AltaPhotoBlob(&Data);

        HAutoPtr<HFCBinStream> pStream(HFCBinStream::Instanciate((HFCPtr<HFCURL>&)pi_pDstFilename, HFC_READ_WRITE_CREATE));
        pStream->Write((void*)&(Data[0]), Data.size());
    }
}


//-----------------------------------------------------------------------------
// Add the Blob to the file
//-----------------------------------------------------------------------------

bool GetFilterValueFromString(Utf8String pi_StringFilterValue, 
                               double* po_pFirstValue,
                               double* po_pSecondValue,
                               double* po_pThirdValue)
{
    bool IsValidString = false;
    uint32_t Index = 1;

    if (ExtractValueFromStringAtIndex(pi_StringFilterValue, &Index, po_pFirstValue))
        if (ExtractValueFromStringAtIndex(pi_StringFilterValue, &Index, po_pSecondValue))
            if (ExtractValueFromStringAtIndex(pi_StringFilterValue, &Index, po_pThirdValue))
                IsValidString = true;

    return IsValidString;
}

//-----------------------------------------------------------------------------
// Debuging code for blob file insertion.
//-----------------------------------------------------------------------------

bool ExtractValueFromStringAtIndex(Utf8String pi_StringFilterValue, uint32_t* pio_pIndex, double* po_Value)
{
    bool  IsValidString = true;
    Utf8String StringValue;
    
    // Skip all caracter until we got a valid "accept in a number" char.
    while(!isdigit(pi_StringFilterValue[(*pio_pIndex)]) && 
          (pi_StringFilterValue[(*pio_pIndex)] != '-')  &&
          (pi_StringFilterValue[(*pio_pIndex)] != '.')  &&
          IsValidString)
    {
        if ((*pio_pIndex) < (pi_StringFilterValue.size() - 1))
            (*pio_pIndex)++;
        else
            IsValidString = false;
    }

    // Get all valid "accept in a number" char to process.
    while((isdigit(pi_StringFilterValue[*pio_pIndex]) || 
          (pi_StringFilterValue[(*pio_pIndex)] == '.')  || 
          (pi_StringFilterValue[(*pio_pIndex)] == '-')) &&
          IsValidString)
    {
        StringValue += pi_StringFilterValue[(*pio_pIndex)];
        if ((*pio_pIndex) < (pi_StringFilterValue.size() - 1))
            (*pio_pIndex)++;
        else
            IsValidString = false;
    }

    // If one or more valid char have been found, process it!
    if (StringValue.size() > 0)
    {
        IsValidString = true;
        (*po_Value) = atof(StringValue.c_str());
    }
    else
        (*po_Value) = 0.0;

    return IsValidString;
}


#endif //_HMR_ALTAPHOTO

