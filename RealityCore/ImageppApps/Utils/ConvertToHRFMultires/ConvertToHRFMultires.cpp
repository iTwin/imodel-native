//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ConvertToHRFMultires/ConvertToHRFMultires.cpp $
//:>    $RCSfile: ConvertToHRFMultires.cpp,v $
//:>   $Revision: 1.4 $
//:>       $Date: 2012/02/27 13:15:34 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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

USING_NAMESPACE_IMAGEPP

#define TILESIZES       256


class ExportDataToItiff
{
public:
    ExportDataToItiff();
    virtual ~ExportDataToItiff();

    bool InitFile(HFCPtr<HFCURLFile>& pi_Filename, uint32_t pi_ImageWidht, uint32_t pi_ImageHeight, 
        HFCPtr<HRPPixelType>& pi_PixelType, HFCPtr<HCDCodec>& pi_Compression);

    bool AddLines(Byte* pi_pData, uint32_t pi_NbLines);

private:
    HFCPtr<HRFRasterFile>           m_pDestination;
    HAutoPtr<HRFResolutionEditor>   m_pResolutionEditor;        
    HFCPtr<HRFResolutionDescriptor> m_pResolutionDescriptor;
    HAutoPtr<Byte>                 m_pBufferOut;
    uint32_t                          m_CurentLine;
    uint32_t                          m_LinePos;
    uint32_t                          m_ByteByLine;
};

ExportDataToItiff::ExportDataToItiff()
{
    m_CurentLine = 0;
    m_LinePos = 0;
    m_ByteByLine = 0;
} 


ExportDataToItiff::~ExportDataToItiff()
{
    // Create a raster to generate all the sub resolution.

    // Close the file.
    m_pResolutionDescriptor = 0;
    m_pResolutionEditor = 0;

    HFCPtr<HFCURL> Filename(m_pDestination->GetURL());
    m_pDestination = 0;

    // Reopen the destination file.
    m_pDestination = HRFRasterFileFactory::GetInstance()->OpenFile(Filename); 

    // Create a raster to generate the sub resolution.

    // Create the pool
    HFCPtr<HPMPool> pLog = new HPMPool(64*1024, NULL); // 64 meg

    // Create the reference CoordSys
    HFCPtr<HGFHMRStdWorldCluster> pWorldCluster = new HGFHMRStdWorldCluster();

    // Put the HRF raster in a store so that we can
    // pull a HRA raster afterwards
    HFCPtr<HRSObjectStore> pStore = new HRSObjectStore (pLog, m_pDestination, 0, pWorldCluster->GetCoordSysReference(m_pDestination->GetWorldIdentificator()));
    HASSERT(pStore != 0);

    // Get the raster from the store
    HFCPtr<HRARaster> pRaster(pStore->LoadRaster());
    HASSERT(pRaster != NULL);

    // Force update sub-resolution.    
    ((HFCPtr<HRAPyramidRaster>&)pRaster)->SetResamplingForSubResolution(HGSResampling::AVERAGE, true);
    ((HFCPtr<HRAPyramidRaster>&)pRaster)->UpdateSubResolution();

    // Save the destination raster file.
    pRaster->Save();
}


bool ExportDataToItiff::InitFile(HFCPtr<HFCURLFile>& pi_Filename, 
                                 uint32_t pi_ImageWidht, uint32_t pi_ImageHeight, 
                                 HFCPtr<HRPPixelType>& pi_PixelType,
                                 HFCPtr<HCDCodec>& pi_Compression)
{
    bool RetValue = true;
    try {
        m_pDestination = HRFRasterFileFactory::GetInstance()->NewFileAs((HFCPtr<HFCURL>)pi_Filename, 
            HRFiTiffCreator::GetInstance());

        if (m_pDestination == 0)
            return false;

        // Create the transfo model
        HFCPtr<HGF2DTransfoModel> pModel;
        double OriginX    = 0.0;  // *** TO DO ***
        double OriginY    = 0.0;  // *** TO DO ***
        double PixelSizeX = 1;    // *** TO DO ***
        double PixelSizeY = 1;    // *** TO DO ***

        pModel = (HFCPtr<HGF2DTransfoModel>) new HGF2DStretch( HGF2DDisplacement(OriginX, OriginY), PixelSizeX, PixelSizeY);
        // Create the resolution editor
        HRFPageDescriptor::ListOfResolutionDescriptor  ResDescList;
        HAutoPtr<HGFResolutionDescriptor>              pPyramidDesc;

        HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
        pMultiResCapability = static_cast<HRFMultiResolutionCapability*>(m_pDestination->GetCapabilities()->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_WRITE_ONLY).GetPtr());

        pPyramidDesc = new HGFResolutionDescriptor(pi_ImageWidht,
            pi_ImageHeight, 
            pMultiResCapability->GetSmallestResWidth(), 
            pMultiResCapability->GetSmallestResHeight());   

        // Create the resolution list.
        for (unsigned short ResCount = 0 ; ResCount < pPyramidDesc->CountResolutions() ; ResCount++)
        {
            // Create a resolution descriptor.
            HFCPtr<HRFResolutionDescriptor> pResDesc = 
                new HRFResolutionDescriptor(HFC_CREATE_ONLY,
                m_pDestination->GetCapabilities(),
                pPyramidDesc->GetResolution(ResCount),
                pPyramidDesc->GetResolution(ResCount),
                pi_PixelType,
                pi_Compression,
                HRFBlockAccess::RANDOM,
                HRFBlockAccess::RANDOM,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,
                HRFInterleaveType::PIXEL,
                false,                                          // Interlace
                pPyramidDesc->GetWidth(ResCount),
                pPyramidDesc->GetHeight(ResCount),
                TILESIZES,     
                TILESIZES,
                0,
                HRFBlockType::TILE,   
                1,
                8,  
                HRFDownSamplingMethod::AVERAGE);   

            // Add the resolution descriptor to the resolution descriptor list.
            ResDescList.push_back(pResDesc);
        } 

        // Create the page descriptor.
        HFCPtr<HRFPageDescriptor>   pPageDescriptor;
        pPageDescriptor = new HRFPageDescriptor (HFC_CREATE_ONLY,
            m_pDestination->GetCapabilities(),
            ResDescList, // List of resolution descriptor
            0,           // RepresentativePalette  
            0,           // Histogram
            0,           // Thumbnail  
            0,           // ClipShape
            pModel,      // TransfoModel
            0,           // Filters 
            0,           // TagList              
            0);          // Duration

        // Add the page to the destination
        m_pDestination->AddPage(pPageDescriptor);

        // Adapt the destination rasterfile to STRIP 
        m_pDestination = new HRFRasterFileBlockAdapter(m_pDestination, 
            HRFBlockType(HRFBlockType::STRIP),
            (uint32_t)pi_ImageWidht,
            TILESIZES);

        m_pResolutionEditor = m_pDestination->CreateResolutionEditor(0, (unsigned short)0, HFC_WRITE_ONLY);        
        m_pResolutionDescriptor = m_pDestination->GetPageDescriptor(0)->GetResolutionDescriptor(0);
        m_pBufferOut = new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()];
        m_ByteByLine = m_pResolutionDescriptor->GetBytesPerBlockWidth();
    }
    catch(...)
    {
        RetValue = false;
    }

    return RetValue;
}

bool ExportDataToItiff::AddLines(Byte* pi_pData, uint32_t pi_NbLines)
{
    do {
        Byte* pData = &(m_pBufferOut[m_CurentLine*m_ByteByLine]);
        if ((TILESIZES-m_CurentLine) >= pi_NbLines)
        {
            memcpy(pData, pi_pData, pi_NbLines*m_ByteByLine);
            m_CurentLine += pi_NbLines;
            pi_NbLines = 0;
        }
        else
        {
            uint32_t LineToAdd = TILESIZES - m_CurentLine;
            pi_NbLines -= LineToAdd;
            memcpy(pData, pi_pData, LineToAdd*m_ByteByLine);

            pi_pData += (LineToAdd*m_ByteByLine);
            m_CurentLine = TILESIZES;
        }

        if (m_CurentLine >= TILESIZES)
        {
            m_pResolutionEditor->WriteBlock(0, m_LinePos, m_pBufferOut);
            m_pResolutionDescriptor->SetBlockDataFlag(m_pResolutionDescriptor->ComputeBlockIndex(0, m_LinePos), HRFDATAFLAG_DIRTYFORSUBRES);
            m_CurentLine = 0;
            m_LinePos += TILESIZES;
        }
    }
    while (pi_NbLines > 0);

    return true;
}


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

//--------------------------------------------------------------------------------
// Main
//--------------------------------------------------------------------------------
int main(int argc, char *argv[])
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
    Utf8String path = Utf8String(argv[1]);
    Utf8String fileName(HFCURLFile::s_SchemeName() + "://" + path);
    SrcFileName  = new HFCURLFile(fileName);
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName); 
    
    if (pSource == 0)
    {
        cout << "Can't open the source file." << endl;
        exit(1);
    }

    // Create the destination URL 
    Utf8String DriveDirName;
    
    // Extract the Path
    Utf8String Path(SrcFileName->GetHost() + "\\" + SrcFileName->GetPath());
    
    // Find the file extension
    Utf8String::size_type DotPos = Path.rfind('.');
    
    // Extract the drive dir name
    if (DotPos != Utf8String::npos)
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

    // Copy the first resolutions.
    unsigned short Resolution = 0;
    uint32_t  StripSize = TILESIZES;

    // Init
    HFCPtr<HRFPageDescriptor>       pSrcPageDescriptor = pSource->GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pSrcResolutionDescriptor = pSrcPageDescriptor->GetResolutionDescriptor(Resolution);

    // Adapt the source raster file to srip.
    // The strip adapter is the fastest access that we can use when we export a file.
    if (HRFRasterFileBlockAdapter::CanAdapt(pSource, 
                                            HRFBlockType(HRFBlockType::STRIP), 
                                            (uint32_t)pSrcResolutionDescriptor->GetWidth(),
                                            StripSize))
	{
        pSource = new HRFRasterFileBlockAdapter(pSource, 
                                                       HRFBlockType(HRFBlockType::STRIP), 
                                                       (uint32_t)pSrcResolutionDescriptor->GetWidth(),
                                                       StripSize);
	}
    else
    {
        // Can't adapt with the specify value for strip height...
        cout << "Can't adapt to strip the source file." << endl;
        exit(1);
    }

    // Reset, because the adapter can change these value
    pSrcPageDescriptor       = pSource->GetPageDescriptor(0);
    pSrcResolutionDescriptor = pSrcPageDescriptor->GetResolutionDescriptor(Resolution);
       

    // Init Destination dimensions 
    uint32_t       DstWidth       = (uint32_t)pSrcResolutionDescriptor->GetWidth();       
    uint32_t       DstHeight      = (uint32_t)pSrcResolutionDescriptor->GetHeight();      
    
    // Create the pixel type and supported compressions
//    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV32R8G8B8A8());     // To adjust
//    HFCPtr<HCDCodec> pCompression(new HCDCodecIdentity());              // To adjust
//    HFCPtr<HCDCodec> pCompression(new HCDCodecZlib());                  // To adjust

    HFCPtr<HRPPixelType> pPixelType(new HRPPixelTypeV24R8G8B8());       // To adjust
//    HFCPtr<HCDCodec> pCompression(new HCDCodecIdentity());              // To adjust
//    HFCPtr<HCDCodec> pCompression(new HCDCodecZlib());                  // To adjust
    HFCPtr<HCDCodec> pCompression(new HCDCodecIJG());                   // To adjust


    // Create the destination file.
    HFCPtr<HFCURLFile> DestFileName(new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + DriveDirName + "_copy.iTiff"));
    ExportDataToItiff Export_iTiff;
    if (!Export_iTiff.InitFile(DestFileName, DstWidth, DstHeight, pPixelType, pCompression))
    {
        cout << "Enable to create the output file." << endl;
        exit(1);
    }

                                               
    //----------------------------------------------------------------------------------------
    // Copy the file pixels
    //----------------------------------------------------------------------------------------    
    
    // Create the destination and source editor.
    HAutoPtr<HRFResolutionEditor>   
        pSrcResolutionEditor(pSource->CreateResolutionEditor(0, Resolution, HFC_READ_ONLY));

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
            pConverter->Convert(pBufferIn, pBufferOut, DstWidth*StripSize);		    

			// Write the source buffer
			Export_iTiff.AddLines(pBufferOut, StripSize);
		}
	}

    printf("\n----------------------------------------------------------------------\n");
	cout << "Copyright (c) 2012 Bentley Systems Inc, ConvertToHRF " << "version 1.0 --- " << __DATE__ << endl;
    printf("\n----------------------------------------------------------------------\n\n");

    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

}
