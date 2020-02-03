/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/ImageConverter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/ImageConverter.cpp,v 1.11 2011/07/18 21:12:37 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Implementation of ImageConverter
//-----------------------------------------------------------------------------

#include "ImageConverterPch.h"

#include <iostream>
#include "winbase.h"

#include <stdexcpt.h>
#include <direct.h>
#include <tchar.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HPMPool.h>

#include "ConverterUtilities.h"
#include "DirectoryScanner.h"
#include "ErrorHandling.h"
#include "PSSUtilities.h"
#include "UsageInformation.h"

//-----------------------------------------------------------------------------
// HRF Includes 
//-----------------------------------------------------------------------------

#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFImgRGBFile.h>
#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HRFPcxFile.h>
#include <Imagepp/all/h/HRFIntergraphCITFile.h>
#include <Imagepp/all/h/HRFIntergraphCOT29File.h>
#include <Imagepp/all/h/HRFIntergraphCotFile.h>
#include <Imagepp/all/h/HRFIntergraphRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphRLEFile.h>
#include <Imagepp/all/h/HRFIntergraphTG4File.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFPngFile.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HRFRLCFile.h>
#include <Imagepp/all/h/HRFEpsFile.h>
#include <Imagepp/all/h/HRFImgMappedFile.h>
#include <Imagepp/all/h/HRFSunRasterFile.h>
#include <Imagepp/all/h/HRFTiffIntgrFile.h>


//-----------------------------------------------------------------------------
// HRF File Format Registration
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRFFileFormats.h>

//-----------------------------------------------------------------------------
// HGS Implementation Registration
//-----------------------------------------------------------------------------
IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

int wmain(int argc, wchar_t *argv[])

{
    int ExitCode = 0;

//    __debugbreak();
    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());
    
    try
    {
        if (argc < 2)
        {
            wcout << "<no input> or <no output>" << endl << endl;
            PrintShortUsage();
            ExitCode = 1;
            goto WRAPUP;
        }
        else
        {

            ExportPropertiesContainer ExportProperties;
            int32_t                    CurrentParamPos = 1;
            bool                     InvaliddnOption = false;
            
            CurrentParamPos = ArgumentParser(&ExportProperties, argc, argv, &ExitCode);
            if ((CurrentParamPos + 2) < argc)
            {
                ExportProperties.WrongArgumentFound = true;
                PrintArgumentError(_TEXT("[options] should be placed before filename."));
                PrintShortUsage();
                ExitCode = 1;
            }

            if (!ExportProperties.WrongArgumentFound)
            {
                if (ExportProperties.CreateOnDemandMosaicPSS == true)
                {                   
                    Utf8String folderName(argv[CurrentParamPos]);
                    Utf8String pssFilename(argv[CurrentParamPos + 1]);
                    GenerateOnDemandMosaicPssFile(folderName,pssFilename);                                        
                }
                else
                {
                    // Allocate Pool...
                    // Need a memory manager to avoid memory fragmentation when 
                    // exporting huge images.                                      
                    HAutoPtr<HPMPool> pPool;
                    pPool = new HPMPool(ExportProperties.PoolSizeInKB, HPMPool::KeepLastBlock);

    #if 0 //VC7
                    if (ExportProperties.Reproject)
                        HCPBlueMarbleDatabase::GetInstance()->Open(ExportProperties.ReprojectDatabase.c_str());
    #endif

                    if (*(argv[CurrentParamPos] +  _tcslen(argv[CurrentParamPos]) - 1) == _TEXT('\\'))
                        *(argv[CurrentParamPos] +  _tcslen(argv[CurrentParamPos]) - 1) = 0;
                    
                    HFCPtr<HFCURLFile> pSrcFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(argv[CurrentParamPos])); 
                    pSrcFilename = UnRelativePath(pSrcFilename);
                    
                    HFCPtr<HFCURLFile> pDstFilename;
                    CurrentParamPos++;

                    if (IsDirectory(pSrcFilename) || IsWildcardFound(pSrcFilename) )
                    {
                        DirectoryScanner *DirScan = 0;

                        if (ExportProperties.ScanAllSubDirectorySpecified)
                            DirScan = new DirectoryScanner(true);
                        else
                            DirScan = new DirectoryScanner(false);

                        Utf8String DirectoryToScan(pSrcFilename->GetHost() + "\\" + pSrcFilename->GetPath());
                        if (!IsWildcardFound(pSrcFilename) && (DirectoryToScan[DirectoryToScan.size() - 1] != '/'))
                            DirectoryToScan += "\\";
                        
                        DirectoryScanner::FILELIST List = DirScan->GetFileList(DirectoryToScan);
                        DirectoryScanner::FILELIST_ITR Itr;

                        // Print copyright notice and current version information before processing.
                        PrintVersionInfo();
                        for(Itr = List.begin(); Itr != List.end(); Itr++ )
                        {
                            HFCPtr<HFCURLFile> pFilename = new HFCURLFile("file://" + Utf8String(*Itr));

                            // Build a destination filename with its path
                            // check if the destination is a dir and/or a file name
                            if (CurrentParamPos >= argc)
                            {
                                // There is no destination directory, so create it!
                                if (ExportProperties.ReplaceExtensionSpecified)
                                    pDstFilename = ConvertExtension(pFilename, ExportProperties.FileFormat);
                                else
                                    pDstFilename = ConvertName(pFilename, ExportProperties.FileFormat);
                            }
                            else
                            {
                                // If the destination filename is ended with a '\', remove it.
                                if (*(argv[CurrentParamPos] +  _tcslen(argv[CurrentParamPos]) - 1) == _TEXT('\\'))
                                    *(argv[CurrentParamPos] +  _tcslen(argv[CurrentParamPos]) - 1) = 0;

                                Utf8String TempFilename = PathInverseSubstractor(pFilename, pSrcFilename);
                                pDstFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + 
                                                                     Utf8String(argv[CurrentParamPos]) + 
                                                                     TempFilename);
                                pDstFilename = UnRelativePath(pDstFilename);

                                // A destination has been given, change its name
                                // if it is a directory, add it the src filename...
                                if (IsDirectory(pDstFilename))
                                {
                                    // We want to export into a specified directory.
                                    pDstFilename = ComposeFileNameWithNewPath(pFilename, pDstFilename);
                                }
                                else
                                {
                                    Utf8String URLpath = "file://" + pDstFilename->GetHost() + "\\" + pDstFilename->GetPath() +  "\\";
                                    HFCPtr<HFCURLFile> pHFCUrl(new HFCURLFile(URLpath));
                                    // If the destination does not exist, try to create it as a directory.
                                    // If this does not work, Print an error and stop the program
                                    if (!IsFileNameExist(pDstFilename) && !BuildPathTree(pHFCUrl))
                                    {
                                        Utf8String Msg = "E002 - Invalid destination file(s) specification : ";
                                        Msg += pDstFilename->GetHost() + "\\" + pDstFilename->GetPath();
                                        cout << endl << Msg << endl;
                                        ExitCode = 1;
                                        goto WRAPUP;
                                    }
                                    else
                                    {
                                        // We want to export into a specified directory.
                                        pDstFilename = ComposeFileNameWithNewPath(pFilename, pDstFilename);
                                    }
                                }
                            
                                // Be sure of its extension...
                                if (ExportProperties.ReplaceExtensionSpecified)
                                    pDstFilename = ConvertExtension(pDstFilename, ExportProperties.FileFormat);
                                else
                                    pDstFilename = ConvertName(pDstFilename, ExportProperties.FileFormat);
                            }

                            // ouput the current working file
	                        _tprintf(_TEXT("\n%s PROCESSING"),WString((*Itr).c_str(), BentleyCharEncoding::Utf8).c_str());

                            // If destination file already exist, be sure we can overwrite it
                            // we cannot throw an error, because the program will exit immediatly, even 
                            // if many items into the list have not been exported.
                            if (IsFileNameExist(pDstFilename) && !ExportProperties.OutputOverwriteSpecified)
                            {
                                ExitCode = 1;
                                _tprintf(_TEXT("\b\b\b\b\b\b\b\b\b\bFAILED : R006 - Output file already exists."));
                            }
                            else
                            {
                                // Keep as debugging code
                                //if (ValidateIfCopyPyramidIsChoose(pSrcFilename, ExportProperties))
                                //    _tprintf("\b\b\b\b\b\b\b\b\b\bExported To %s", WString(pDstFilename->GetHost() + "\\" + pDstFilename->GetPath()).c_str());
                                
                                // Export the source file into destination file...
                                if (ValidateIfCopyPyramidIsChoose(pSrcFilename, ExportProperties))
                                {
                                    try
                                    {
                                        ExportRasterFile(&ExportProperties, pFilename, pDstFilename, pPool);
                                    }
                                    catch (...)
                                    {
                                        ExitCode = 1;
                                        MyExceptionHandler();
                                    }
                                }
                            }
                        }
                        delete DirScan;
                    }
                    else // Not Wildcard or directory
                    {
                        // We have only a single file to export.
                        if (!IsFileNameExist(pSrcFilename))
                        {
                            ExitCode = 1;

                            // Print copyright notice and current version information before processing.
                            PrintVersionInfo();

                            // ouput the current working file
	                        _tprintf(_TEXT("\n%s PROCESSING"),WString((pSrcFilename->GetHost() + "\\" + pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str());
                            
                            // If destination file already exist, be sure we can overwrite it
                            // if we cannot, throw an error.                        
                            throw(HFCFileNotFoundException(pSrcFilename->GetHost() + "\\" + pSrcFilename->GetPath()));
                        }
                        else
                        {

                            // Build a destination filename with its path
                            // check if the distination is a dir and/or a file name
                            if (CurrentParamPos >= argc)
                            {
                                // There is no destination directory, so create it!
                                if (ExportProperties.ReplaceExtensionSpecified)
                                    pDstFilename = ConvertExtension(pSrcFilename, ExportProperties.FileFormat);
                                else
                                    pDstFilename = ConvertName(pSrcFilename, ExportProperties.FileFormat);
                            }
                            else
                            {
                                pDstFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(argv[CurrentParamPos]));
                                pDstFilename = UnRelativePath(pDstFilename);

                                // A destination has been given, change its name
                                // if it is a directory, add it the src filename...

                                if (IsDirectory(pDstFilename))
                                {
                                    // We want to export into a specified directory.
                                    pDstFilename = ComposeFileNameWithNewPath(pSrcFilename, pDstFilename);

                                    // Be sure of it's extension...
                                    if (ExportProperties.ReplaceExtensionSpecified)
                                        pDstFilename = ConvertExtension(pDstFilename, ExportProperties.FileFormat);
                                    else
                                        pDstFilename = ConvertName(pDstFilename, ExportProperties.FileFormat);
                                }
                                else if (ExportProperties.dnOptionSpecified)
                                    InvaliddnOption = true;
                            }
                        }
                        
                        // Print copyright notice and current version information before processing.
                        PrintVersionInfo();

                        
                        if (InvaliddnOption)
                            _tprintf(_TEXT("\nWarning: The -dn option is ignored because a complete destination file name was specified.\n"));

                        // ouput the current working file
	                    _tprintf(_TEXT("\n%s PROCESSING"),WString((pSrcFilename->GetHost() + "\\" + pSrcFilename->GetPath()).c_str(), BentleyCharEncoding::Utf8).c_str());

                        // If destination file already exist, be sure we can overwrite it
                        // if we can't, throw an error.
                        if (IsFileNameExist(pDstFilename) && !ExportProperties.OutputOverwriteSpecified)
                        {
                            ExitCode = 1;
                            throw(HFCFileExistException(pDstFilename->GetHost() + "\\" + pDstFilename->GetPath()));
                        }
                        else
                        {
                            // Export the source file into destination file...
                            if (ValidateIfCopyPyramidIsChoose(pSrcFilename, ExportProperties))
                            {
                                ExportRasterFile(&ExportProperties, pSrcFilename, pDstFilename, pPool);
                                //_tprintf("\b\b\b\b\b\b\b\b\b\bExported To %s", WString(pDstFilename->GetHost() + "\\" + pDstFilename->GetPath()).c_str());
                            }


                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
        ExitCode = 1;
        MyExceptionHandler();
    }
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

WRAPUP:
    return(ExitCode);
}
