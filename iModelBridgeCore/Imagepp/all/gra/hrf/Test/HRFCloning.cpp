//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HRFResolutionEditor.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/HFCURL.h>
#include <ImagePP/all/h/HFCURLFile.h>



// HRF Includes and File format registration
#include <ImagePP/all/h/HRFBmpFile.h>
#include <ImagePP/all/h/HRFCalsFile.h>
#include <ImagePP/all/h/HRFGeoTiffFile.h>
#include <ImagePP/all/h/HRFHMRFile.h>
#include <ImagePP/all/h/HRFIntergraphCITFile.h>
#include <ImagePP/all/h/HRFIntergraphCOT29File.h>
#include <ImagePP/all/h/HRFIntergraphCotFile.h>
#include <ImagePP/all/h/HRFIntergraphRGBFile.h>
#include <ImagePP/all/h/HRFIntergraphRLEFile.h>
#include <ImagePP/all/h/HRFIntergraphTG4File.h>
#include <ImagePP/all/h/HRFiTiffFile.h>
#include <ImagePP/all/h/HRFJpegFile.h>
#include <ImagePP/all/h/HRFPngFile.h>
#include <ImagePP/all/h/HRFTiffFile.h>
#include <ImagePP/all/h/HRFGifFile.h>


HRF_REGISTER_FILEFORMAT(HRFHMRCreator)
HRF_REGISTER_FILEFORMAT(HRFiTiffCreator)
HRF_REGISTER_FILEFORMAT(HRFTiffCreator)
HRF_REGISTER_FILEFORMAT(HRFGeoTiffCreator)
HRF_REGISTER_FILEFORMAT(HRFJpegCreator)
HRF_REGISTER_FILEFORMAT(HRFPngCreator)
HRF_REGISTER_FILEFORMAT(HRFBmpCreator)
HRF_REGISTER_FILEFORMAT(HRFGifCreator)

HRF_REGISTER_FILEFORMAT(HRFCalsCreator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphCitCreator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphCot29Creator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphCotCreator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphRGBCreator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphRleCreator)
HRF_REGISTER_FILEFORMAT(HRFIntergraphTG4Creator)

//HRF_REGISTER_FILEFORMAT(HRFInternetFileSocketCreator)
//HRF_REGISTER_FILEFORMAT(HRFInternetFileHTTPCreator)

int32_t _tmain(int32_t argc, _TCHAR* argv[])
    {
    // Source
    HFCPtr<HRFRasterFile>           pSource;
    HAutoPtr<HRFResolutionEditor>   pSrcResolutionEditor;
    HFCPtr<HRFResolutionDescriptor> pSrcResolutionDescriptor;
    HFCPtr<HFCURLFile>              SrcFileName;
    HFCAccessMode                   SrcEditorAccess = HFC_READ_ONLY;

    // Destination
    HFCPtr<HRFRasterFile>           pDestination;
    HAutoPtr<HRFResolutionEditor>   pDstResolutionEditor;
    HFCPtr<HFCURLFile>              DestFileName;
    HFCAccessMode                   DstFileAccess   = HFC_READ_WRITE_CREATE;
    HFCAccessMode                   DstEditorAccess = HFC_WRITE_ONLY;

    //----------------------------------------------------------------------------------------
    // Check that we have the right number of parameters.
    if (argc != 2)
        {
        cout << "HRFClonning <no input>" << endl;
        exit(1);
        }

    //----------------------------------------------------------------------------------------
    // Open the source file
    SrcFileName  = new HFCURLFile(Utf8String(HFCURLFile::s_SchemeName() + _T("://"))+Utf8String(argv[1]));
    pSource = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName);
    if (pSource == 0)
        {
        cout << "Can't open the source file." << endl;
        exit(1);
        }

    //----------------------------------------------------------------------------------------
    // Create the destination URL
    Utf8String Extension;
    Utf8String DriveDirName;

    // Extract the Path
    Utf8String Path(SrcFileName->GetHost()+Utf8String(_T("\\"))+SrcFileName->GetPath());

    // Find the file extension
    Utf8String::size_type DotPos = Path.rfind('.');

    // Extract the extension and the drive dir name
    if (DotPos != string::npos)
        {
        Extension = Path.substr(DotPos+1, Path.length() - DotPos - 1);
        DriveDirName = Path.substr(0, DotPos);
        }
    else
        {
        cout << "Can't create the destination file." << endl;
        exit(1);
        }

    //----------------------------------------------------------------------------------------
    // Create the destination file.
    DestFileName = new HFCURLFile(Utf8String(HFCURLFile::s_SchemeName() + _T("://"))+DriveDirName+Utf8String(_T(".clone."))+Extension);
    pDestination = HRFRasterFileFactory::GetInstance()->NewFile((HFCPtr<HFCURL>)DestFileName);
    if (pDestination == 0)
        {
        cout << "Enable to create the output file." << endl;
        exit(1);
        }

    //----------------------------------------------------------------------------------------
    // Clone the file
        {
        uint32_t                    Page = 0;
        HFCPtr<HRFPageDescriptor>   pPageDescriptor = pSource->GetPageDescriptor(Page);

        // Best Math the page descriptor and resolution descriptor
        //pPageDescriptor = new HRFPageDescriptor(pDestination->GetCapabilities()->GetPageCapabilities(),
        //                                        HFC_WRITE_ONLY,
        //                                        pSource->GetPageDescriptor(Page));

        // Add the page to the destination
        pDestination->AddPage(pPageDescriptor);

        // Copy all resolutions.
        for (uint16_t Resolution=0 ; Resolution<pPageDescriptor->CountResolutions() ; Resolution++)
            {
            // Create the destination and source editor.
            pSrcResolutionEditor     = pSource->CreateResolutionEditor(Page, Resolution, SrcEditorAccess);
            pDstResolutionEditor     = pDestination->CreateResolutionEditor(Page, Resolution, DstEditorAccess);
            pSrcResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(Resolution);

            Byte* pTileBuffer = (Byte*)GlobalAlloc(GMEM_FIXED, pSrcResolutionDescriptor->GetBlockSizeInBytes());

            // Copy the resolution block by block.
            for (uint32_t PosY=0; PosY<pSrcResolutionDescriptor->GetHeight(); PosY += pSrcResolutionDescriptor->GetBlockHeight())
                {
                for (uint32_t PosX=0; PosX<pSrcResolutionDescriptor->GetWidth(); PosX += pSrcResolutionDescriptor->GetBlockWidth())
                    {
                    if (pSrcResolutionEditor->ReadBlock(PosX, PosY, pTileBuffer) == H_SUCCESS)
                        pDstResolutionEditor->WriteBlock(PosX, PosY, pTileBuffer);
                    }
                }
            GlobalFree(pTileBuffer);
            }
        }
    }



