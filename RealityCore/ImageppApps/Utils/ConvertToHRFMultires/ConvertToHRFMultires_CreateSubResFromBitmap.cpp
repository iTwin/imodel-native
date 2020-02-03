//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ConvertToHRFMultires/ConvertToHRFMultires_CreateSubResFromBitmap.cpp $
//:>    $RCSfile: ConvertToHRFMultires.cpp,v $
//:>   $Revision: 1.4 $
//:>       $Date: 2012/02/27 13:15:34 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <Imagepp/h/ImageppAPI.h>

#include <iostream>

#include <Imagepp/all/h/HRFMacros.h>

// HRF Includes and File format registration
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HRFiTiffFile.h>

#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRPPixelConverter.h>

// Color Space Includes
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>

// Transformation Model Includes
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>

// Other Include
#include <Imagepp/h/HAutoPtr.h>
#include <Imagepp/all/h/HPMPool.h>

//-------------------------------------------------------------------------------------------------------------


// For input file only
//   Not necessary when we want to use only the class ExportDataToItiff
#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFImgRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphCITFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRFPcxFile.h>
#include <Imagepp/all/h/HRFSunRasterFile.h>


/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct MyImageppLibHost : ImagePP::ImageppLib::Host 
{
    virtual void _RegisterFileFormat() override 
        {
        HOST_REGISTER_FILEFORMAT(HRFiTiffCreator)
        HOST_REGISTER_FILEFORMAT(HRFHMRCreator)
        HOST_REGISTER_FILEFORMAT(HRFTiffCreator)
        HOST_REGISTER_FILEFORMAT(HRFGeoTiffCreator)
        HOST_REGISTER_FILEFORMAT(HRFJpegCreator)
        HOST_REGISTER_FILEFORMAT(HRFSunRasterCreator)
        HOST_REGISTER_FILEFORMAT(HRFImgRGBCreator)
        HOST_REGISTER_FILEFORMAT(HRFBmpCreator)
        HOST_REGISTER_FILEFORMAT(HRFPcxCreator)
        HOST_REGISTER_FILEFORMAT(HRFIntergraphCitCreator)
        }

}; // MyImageppLibHost


//-----------------------------------------------------------------------------
// This function print out the usage for the current program.
//-----------------------------------------------------------------------------
void ShowUsage()
{
    // Check that we have the right number of parameters.
    cout << "ConvertToHRFMultiRes V1.0 --- " << __DATE__ << endl
         <<  endl
         << "Usage:  ConvertToHRFMultiRes <drive:\\SourceFile>" << endl
         <<  endl;
}


void CopyDataInDB(uint32_t pi_Resolution, Byte* pi_pData, uint32_t pi_Width, uint32_t pi_Height)
    {
    // split data in tiles

    char* filename[512];
    sprintf((char*)filename, "k:\\tmp\\Image_%2d_%5d_%5d.raw", pi_Resolution, pi_Width, pi_Height);
    FILE* fp=fopen((const char*)filename, "wb");
    fwrite(pi_pData, 3, pi_Width*pi_Height ,fp);
    fclose(fp);
    }

void CopyReduceSize(Byte* pi_InData, uint32_t pi_InWidth, uint32_t pi_InHeight, Byte* pi_OutData, uint32_t pi_OutWidth, uint32_t pi_OutHeight)
    {
    uint32_t lineSize = pi_InWidth * 3;

    // Skip the last column and row if impair.
    uint32_t OutX = pi_OutWidth  - (pi_InWidth % 2);
    uint32_t OutY = pi_OutHeight - (pi_InHeight % 2);
    short avgR;
    short avgG;
    short avgB;

    HDEBUGCODE(Byte* pOutEnd = pi_OutData + (pi_OutWidth*pi_OutHeight*3););

    for (uint32_t y=0; y<OutY; ++y)
        {
        Byte* pBbufLine1 = pi_InData + (y * 2) * lineSize;
        Byte* pBbufLine2 = pBbufLine1 + lineSize;

        for (uint32_t x=0; x<OutX; ++x)
            {
            avgR = *(pBbufLine1+0);
            avgG = *(pBbufLine1+1);
            avgB = *(pBbufLine1+2);

            avgR += *(pBbufLine1+3);
            avgG += *(pBbufLine1+4);
            avgB += *(pBbufLine1+5);

            avgR += *(pBbufLine2+0);
            avgG += *(pBbufLine2+1);
            avgB += *(pBbufLine2+2);

            avgR += *(pBbufLine2+3);
            avgG += *(pBbufLine2+4);
            avgB += *(pBbufLine2+5);

            pBbufLine1 += 6;
            pBbufLine2 += 6;

            *pi_OutData++ = (Byte)(avgR >> 2);
            *pi_OutData++ = (Byte)(avgG >> 2);
            *pi_OutData++ = (Byte)(avgB >> 2);
            }

        // Fill the last column with only 2 average values because source is impair.
        if (OutX < pi_OutWidth)
            {
            avgR = *(pBbufLine1+0);
            avgG = *(pBbufLine1+1);
            avgB = *(pBbufLine1+2);
            avgR += *(pBbufLine2+0);
            avgG += *(pBbufLine2+1);
            avgB += *(pBbufLine2+2);

            *pi_OutData++ = (Byte)(avgR >> 1);
            *pi_OutData++ = (Byte)(avgG >> 1);
            *pi_OutData++ = (Byte)(avgB >> 1);
            }
        }

    // Fill the last line with only 2 average values because source is impair.
    if (OutY < pi_OutHeight)
        {
        Byte* pBbufLine1 = pi_InData + (OutY * 2) * lineSize;
        for (uint32_t x=0; x<OutX; ++x,pBbufLine1+=6)
            {
            avgR = *(pBbufLine1+0);
            avgG = *(pBbufLine1+1);
            avgB = *(pBbufLine1+2);
            avgR += *(pBbufLine1+3);
            avgG += *(pBbufLine1+4);
            avgB += *(pBbufLine1+5);

            *pi_OutData++ = (Byte)(avgR >> 1);
            *pi_OutData++ = (Byte)(avgG >> 1);
            *pi_OutData++ = (Byte)(avgB >> 1);
            }
        }

    // Fill the pixel in the bottom right corner
    if (OutY < pi_OutHeight && OutX < pi_OutWidth)
        {
        uint32_t bufPos = (OutY * 2 * lineSize) + (OutX * 6);
        *pi_OutData++ = pi_InData[bufPos];
        *pi_OutData++ = pi_InData[bufPos+1];
        *pi_OutData++ = pi_InData[bufPos+2];
        }

    HASSERT(pi_OutData == pOutEnd);
    }


// 
// pi_pDta is a buffer of RGB values.
//
void GenerateRasterInDB (Byte* pi_pData, uint32_t pi_Width, uint32_t pi_Height)
    {
    uint32_t Resolution = 0;
    CopyDataInDB(Resolution++, pi_pData, pi_Width, pi_Height);
    Byte* pBuf1 = pi_pData;
    Byte* pBuf2 = 0;
    uint32_t dimX, dimY;

    while(pi_Width > 256 || pi_Height > 256)
        {
        dimX = (uint32_t)ceil(pi_Width / 2.0);
        dimY = (uint32_t)ceil(pi_Height / 2.0);

        pBuf2 = new Byte[dimX*dimY*3];    // RGB --> *3
        CopyReduceSize(pBuf1, pi_Width, pi_Height, pBuf2, dimX, dimY);
        CopyDataInDB(Resolution++, pBuf2, dimX, dimY);

        if (pBuf1 != pi_pData)
            delete pBuf1;
        pBuf1 = pBuf2;
        pi_Width = dimX;
        pi_Height = dimY;
        }
    if (pBuf2 != 0)
        delete pBuf2;
    }






//--------------------------------------------------------------------------------
// Main
//--------------------------------------------------------------------------------
void main(int argc, char *argv[])
{

#ifdef __HMR_DEBUGxx
//    static CMemoryState oldstate, newstate, diffstate;

//    oldstate.Checkpoint();
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    static bool CheckHeap = true;
    //HASSERT(_CrtCheckMemory());

    if (CheckHeap)
    {
        int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        // _CRTDBG_CHECK_ALWAYS_DF |
        _CrtSetDbgFlag(tmpFlag | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF);
        //  | checkAlwaysMemDF
//        afxMemDF = allocMemDF | delayFreeMemDF   ;
    }
#endif
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());


    // Source          
    HFCPtr<HRFRasterFile>           pSource;     
    HFCPtr<HFCURLFile>              SrcFileName;

    // Check that we have the right number of parameters.
    if (argc != 2)
    {
        ShowUsage();
        exit(1);
    }

    // Open the source file
    SrcFileName  = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://")+WString(argv[1])); 
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName); 
    
    if (pSource == 0)
    {
        cout << "Can't open the source file." << endl;
        exit(1);
    }

    // Create the destination URL 
    WString DriveDirName;
    
    // Extract the Path
    WString Path(SrcFileName->GetHost()+WString(L"\\")+SrcFileName->GetPath());
    
    // Find the file extension
    WString::size_type DotPos = Path.rfind(L'.');
    
    // Extract the drive dir name
    if (DotPos != WString::npos)
    {
        DriveDirName = Path.substr(0, DotPos);
    }
    else
    {
        cout << "Can't create the destination file." << endl;
        exit(1);
    }

    //----------------------------------------------------------------------------------------
    // Create the descriptor
    //----------------------------------------------------------------------------------------    

    // Init
    HFCPtr<HRFPageDescriptor>       pSrcPageDescriptor = pSource->GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pSrcResolutionDescriptor = pSrcPageDescriptor->GetResolutionDescriptor(0);

    if (HRFRasterFileBlockAdapter::CanAdapt(pSource, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT))
    {
        pSource = new HRFRasterFileBlockAdapter(pSource, 
                                                    HRFBlockType::IMAGE,
                                                    HRF_EQUAL_TO_RESOLUTION_WIDTH,
                                                    HRF_EQUAL_TO_RESOLUTION_HEIGHT);
    }
    else
    {
        // Can't adapt with the specify value for strip height...
        cout << "Can't adapt to strip the source file." << endl;
        exit(1);
    }

    // Reset, because the adapter can change these value
    pSrcPageDescriptor       = pSource->GetPageDescriptor(0);
    pSrcResolutionDescriptor = pSrcPageDescriptor->GetResolutionDescriptor(0);
       

    // Init Destination dimensions 
    uint32_t       DstWidth       = (uint32_t)pSrcResolutionDescriptor->GetWidth();       
    uint32_t       DstHeight      = (uint32_t)pSrcResolutionDescriptor->GetHeight();      
    
    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV24R8G8B8());       // To adjust
                                               
    //----------------------------------------------------------------------------------------
    // Copy the file pixels
    //----------------------------------------------------------------------------------------    
    
    // Create the destination and source editor.
    HAutoPtr<HRFResolutionEditor>   
        pSrcResolutionEditor(pSource->CreateResolutionEditor(0, 0, HFC_READ_ONLY));

    HAutoPtr<Byte> pBufferIn (new Byte[pSrcResolutionDescriptor->GetBlockSizeInBytes()]);
    HAutoPtr<Byte> pBufferOut(new Byte[(((pPixelType->CountPixelRawDataBits() * DstWidth) + 7) /8) * DstHeight]);

    // Create pixelConverter
    HFCPtr<HRPPixelConverter> 
        pConverter(pSrcResolutionDescriptor->GetPixelType()->GetConverterTo(pPixelType));

	// Copy the resolution block by block.
	for (uint32_t BlockPosY=0; BlockPosY < pSrcResolutionDescriptor->GetHeight(); BlockPosY += pSrcResolutionDescriptor->GetBlockHeight())
	{
		for (uint32_t BlockPosX=0; BlockPosX<pSrcResolutionDescriptor->GetWidth(); BlockPosX += pSrcResolutionDescriptor->GetBlockWidth())
		{
			// Read the source buffer
            pSrcResolutionEditor->ReadBlock(BlockPosX, BlockPosY, pBufferIn);
            // Convert pixel
            pConverter->Convert(pBufferIn, pBufferOut, DstWidth*DstHeight);		    

			GenerateRasterInDB (pBufferOut, DstWidth, DstHeight);
		}
	}

    printf("\n----------------------------------------------------------------------\n");
	cout << "Copyright (c) 2012 Bentley Systems Inc, ConvertToHRF " << "version 1.0 --- " << __DATE__ << endl;
    printf("\n----------------------------------------------------------------------\n\n");

    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

}
