//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HTiffInfo/HTiffInfo.cpp $
//:>    $RCSfile: HTiffInfo.cpp,v $
//:>   $Revision: 1.15 $
//:>       $Date: 2011/07/18 21:12:32 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <windows.h>

#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HTIFFFile.h>

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFJpegFile.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>


USING_NAMESPACE_IMAGEPP

static const TCHAR* sUsage[] = {
_TEXT("usage: HTiffInfo [options] <Filename>"),
_TEXT("where options are:"),
_TEXT(" -c        Display ColorMap or Data for grey/color response curve"),
_TEXT(" -d        Display image data"),
_TEXT(" -f        Display Free blocks offsets and Byte counts"),
_TEXT(" -g        Display GeoTags"),
_TEXT(" -p        Pause at the end of execution"),
_TEXT(" -r        Read data"),
_TEXT(" -s        Display Strip/Tile offsets and Byte counts"),
_TEXT(" -<0-12>   Display info for the specify directory only, from 0 to 12"),
_TEXT(""),
_TEXT(" -h        Show HMR Histogram"),
_TEXT(" -sh       Display HMR shapes"),
_TEXT(" -tf       Display HMR2 TileFlag info"),
_TEXT(" -jpgt     Display JPEG table if stored in JPEG Tag table."),
_TEXT(" -egpst    Display EXIF related GPS tags."),
_TEXT(" -exift    Display EXIF tags."),
_TEXT(""),
_TEXT(" -?        Display Help\n"),
NULL
};

static void       sDisplayUsage       ();
static void       sDisplayBuffer      (const Byte* pi_pBuffer, uint32_t pi_Count);
static void       sDisplayFileData    (HTIFFFile& pi_rFile, bool pi_ReadOnly);
static HTIFFFile* sGetHTIFFforJpegExifTags(HFCPtr<HFCURL>& pi_prUrl);

struct HTiffInfoImageppLibHost : ImagePP::ImageppLib::Host  
{                                                       
virtual void _RegisterFileFormat() override             
    {                                                       
    // Not needed. REGISTER_SUPPORTED_FILEFORMAT                           
    }                                                       
};    


int wmain(int pi_Argc, wchar_t *pi_ppArgv[])
{
    HTIFFError* pMsg;
    uint32_t    i = 1;

    uint32_t SelectedOptions     = 0;
    int32_t  DisplaySpcifyDirOnly = -1;          // -1 --> all directory
    bool   DisplayData         = false;
    bool   DisplayReadData     = false;
    bool   DisplayJpegTable    = false;
    bool   DisplayExifGpsTags  = false;
    bool   DisplayExifTags     = false;
    bool   PauseProg           = false;


    if (pi_Argc <= 1)
        sDisplayUsage();

    while (*pi_ppArgv[i] == _TEXT('-'))
    {
        if (_tcsicmp (pi_ppArgv[i], _TEXT("-c")) == 0)
            SelectedOptions |= (TIFFPRINT_CURVES | TIFFPRINT_COLORMAP);
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-d")) == 0)
            DisplayData = true;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-f")) == 0)
            SelectedOptions |= TIFFPRINT_FREEBLOCKS;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-g")) == 0)
            SelectedOptions |= (TIFFPRINT_GEOKEYDIRECTORY | TIFFPRINT_GEOKEYPARAMS);
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-p")) == 0)
            PauseProg = true;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-r")) == 0)
            DisplayReadData = true;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-s")) == 0)
            SelectedOptions |= TIFFPRINT_STRIPS;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-h")) == 0)
            SelectedOptions |= TIFFPRINT_HMR_HISTOGRAM;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-sh")) == 0)
            SelectedOptions |= (TIFFPRINT_HMR_LOGICALSHAPE | TIFFPRINT_HMR_TRANSPARENTSHAPE | TIFFPRINT_HMR_USERDATA); 
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-tf")) == 0)
            SelectedOptions |= TIFFPRINT_HMR2_TILEFLAG; 
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-0")) == 0)
            DisplaySpcifyDirOnly = 0;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-1")) == 0)
            DisplaySpcifyDirOnly = 1;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-2")) == 0)
            DisplaySpcifyDirOnly = 2;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-3")) == 0)
            DisplaySpcifyDirOnly = 3;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-4")) == 0)
            DisplaySpcifyDirOnly = 4;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-5")) == 0)
            DisplaySpcifyDirOnly = 5;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-6")) == 0)
            DisplaySpcifyDirOnly = 6;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-7")) == 0)
            DisplaySpcifyDirOnly = 7;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-8")) == 0)
            DisplaySpcifyDirOnly = 8;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-9")) == 0)
            DisplaySpcifyDirOnly = 9;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-10")) == 0)
            DisplaySpcifyDirOnly = 10;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-11")) == 0)
            DisplaySpcifyDirOnly = 11;
        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-12")) == 0)
            DisplaySpcifyDirOnly = 12;

        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-jpgt")) == 0)
            DisplayJpegTable = true; 

        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-egpst")) == 0)
            DisplayExifGpsTags = true;         

        else if (_tcsicmp (pi_ppArgv[i], _TEXT("-exift")) == 0)
            DisplayExifTags = true;         

        else
            sDisplayUsage();
        i++;
    }

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new HTiffInfoImageppLibHost());

            
    HAutoPtr<HTIFFFile> pTiffFile;

    HFCPtr<HFCURL> pURL;
        
    pURL = new HFCURLFile("", Utf8String(pi_ppArgv[i]));                

    if (HRFJpegCreator::GetInstance()->IsKindOfFile(pURL, 0) == true)
    {     
        pTiffFile = sGetHTIFFforJpegExifTags(pURL);

        if (pTiffFile != 0)
        {
            _tprintf (_TEXT("\nJPEG file with EXIF Tags: %s\n"), pi_ppArgv[i]);
        }        
    }
    else
    {    
        pTiffFile = new HTIFFFile(Utf8String(pi_ppArgv[i]), HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
        _tprintf (_TEXT("\nTIFF file: %s\n"), pi_ppArgv[i]);
    }
        
    if (pTiffFile != 0)
    {    
        if (!pTiffFile->IsValid(&pMsg))
        {        
            Utf8String ErrorMsg;        
            pMsg->GetErrorMsg(ErrorMsg);
            WString ErrorMsgW(ErrorMsg.c_str(), BentleyCharEncoding::Utf8);

            _tprintf (_TEXT("\n--------- ErrorMSG ---------\n%s\n"), ErrorMsgW.c_str());
        }
        if ((pMsg == 0 || (pMsg != 0 && !pMsg->IsFatal())) && pTiffFile->IsTiff64())
        {
            _tprintf (_TEXT("\n--------- BigTiff - Tiff64 bit ---------\n"));
        }

        for (i = 0; i < pTiffFile->NumberOfDirectory(); i++)
        {        // Display only the specify directory
            if ((DisplaySpcifyDirOnly == -1) || (DisplaySpcifyDirOnly == i))
            {
                _tprintf (_TEXT("\nDirectory %ld\n"), i);
                pTiffFile->PrintDirectory (stdout, 
                                           HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i), 
                                           SelectedOptions);

                // If first Directory, check for HMR
                if (i == 0 && pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, 0)))
                {
                    _tprintf (_TEXT("\nHMR Directory.\n"));
                    pTiffFile->PrintDirectory (stdout, 
                                               HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, 0), 
                                               SelectedOptions);
                
                    pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i));
                }
                
                //Display EXIF related GPS tags, if any
                if (DisplayExifGpsTags)
                {
                    pTiffFile->PrintEXIFDefinedGPSTags(i,
                                                       stdout);
                }            

                //Display EXIF tags, if any
                if (DisplayExifTags)
                {
                    pTiffFile->PrintEXIFTags(i,
                                             stdout);
                }                        
                                        
                // Display JPEG table
                if (DisplayJpegTable)
                {
                    pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i));

                    Byte *pTable;
                    uint32_t TableSize = 0;
                    if(pTiffFile->GetField (JPEGTABLES, &TableSize, &pTable) && TableSize > 0)
                    {
                	    _ftprintf(stdout, _TEXT("JPEG Table (Nb Entries:%ld)\n "), TableSize);
                        sDisplayBuffer(pTable, TableSize);
                    }
                }

                // Display or Read Data
                if (DisplayData || DisplayReadData)
                {
                    pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i));
                    sDisplayFileData (*pTiffFile, DisplayReadData);
                
                    if (!pTiffFile->IsValid(&pMsg))
                    {
                        Utf8String ErrorMsg;
                        pMsg->GetErrorMsg(ErrorMsg);
                        WString ErrorMsgW(ErrorMsg.c_str(), BentleyCharEncoding::Utf8);

                        _tprintf (_TEXT("\n--------- ErrorMSG ---------\n%s\n"), ErrorMsgW.c_str());
                    }
                }
            }            

            // Display First Directory Only
            //
            if (DisplaySpcifyDirOnly == i)
                break;
        }
    }

    if (PauseProg)
        _getch();

    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

    return 0;
}



static 
void sDisplayUsage()
{
    _ftprintf (stderr, _TEXT("\nHTiffInfo V2.0 --- %s\n\n"),  _T(__DATE__));
	for (uint32_t i = 0; sUsage[i] != 0; i++)
		_ftprintf(stderr, _TEXT("%s\n"), sUsage[i]);

	exit(-1);
}

static
void sDisplayBuffer(const Byte* pi_pBuffer, uint32_t pi_Count)
{
	for (uint32_t i=0; i<pi_Count; i++) 
    {
		_ftprintf(stdout, _TEXT(" %02x"), *pi_pBuffer++);
		if (((i+1) % 32) == 0)
			_ftprintf(stdout, _TEXT("\n "));
	}
	_ftprintf(stdout, _TEXT("\n"));
}



static
void sDisplayFileData (HTIFFFile& pi_rFile, bool pi_ReadOnly)
{
    unsigned short Planar;

	if ((pi_rFile.GetField(PLANARCONFIG, &Planar)) && (Planar != PLANARCONFIG_CONTIG))
    {
        _ftprintf(stdout, _TEXT("Planar configuration not supported: %d\n"), Planar);
        return;
    }


    uint32_t ImageW;
    uint32_t ImageL;
    uint32_t TileW;
    uint32_t TileL;
    Byte* pBuf = 0;
    pi_rFile.GetField(IMAGEWIDTH,  &ImageW);
    pi_rFile.GetField(IMAGELENGTH, &ImageL);
    pi_rFile.GetField(TILEWIDTH,   &TileW);
    pi_rFile.GetField(TILELENGTH,  &TileL);

	if (pi_rFile.IsTiled())
    {
        // Tile
        //
	    pBuf = new Byte[pi_rFile.TileSize()];
        HASSERT(pBuf != 0);

	    uint32_t Row;
        uint32_t Col;
        
	    for (Row = 0; Row < ImageL; Row += TileL) 
        {
	    	for (Col = 0; Col < ImageW; Col += TileW) 
            {
	    		if (pi_rFile.TileRead(pBuf, Col, Row) != H_SUCCESS)
                    break;      // Error

                // Display Data
	    		else if (!pi_ReadOnly)
                {
                	_ftprintf(stdout, _TEXT("Tile (%lu,%lu)\n "), Col, Row);
	    			sDisplayBuffer(pBuf, pi_rFile.TileSize());
                }
	    	}
	    }
	}
    else 
    {
        // Strip
        //
	    pBuf = new Byte[pi_rFile.StripSize()];
        HASSERT(pBuf != 0);

	    uint32_t Strip;
        uint32_t NbStrip = pi_rFile.NumberOfStrips();
        
	    for (Strip = 0; Strip < NbStrip; Strip++) 
        {
	    	if (pi_rFile.StripRead(pBuf, Strip) != H_SUCCESS)
                break;      // Error

            // Display Data
	    	else if (!pi_ReadOnly)
            {
            	_ftprintf(stdout, _TEXT("Strip (%lu)\n "), Strip);
	    		sDisplayBuffer(pBuf, pi_rFile.TileSize());
            }
	    }
	}

    delete[] pBuf;
}


static
HTIFFFile* sGetHTIFFforJpegExifTags(HFCPtr<HFCURL>& pi_prUrl)
{   
    HTIFFFile* pTIFFFile = 0;
          
    HAutoPtr<HFCBinStream> pJpegFile(HFCBinStream::Instanciate(pi_prUrl, HFC_READ_ONLY |HFC_SHARE_READ_ONLY));
    
    if (pJpegFile == 0)
    {
        _tprintf (_TEXT("\n--------- ErrorMSG ---------\nCannot open the file : %s\n"), 
                  WString(pi_prUrl->GetURL().c_str(), BentleyCharEncoding::Utf8).c_str());
    }
    else
    if (pJpegFile->GetLastException() != 0)
    {

        _tprintf (_TEXT("\n--------- ErrorMSG ---------\n%s\n"), 
                  WString(pJpegFile->GetLastException()->GetExceptionMessage().c_str(), BentleyCharEncoding::Utf8).c_str());
    }
    else
    {            
        /* -------------------------------------------------------------------- */
        /*      Search for APP1 chunk.                                          */
        /* -------------------------------------------------------------------- */
        Byte Header[10];
        int32_t  HeaderPos = 2;    
        int32_t  App1Offset; 


        while (true) 
        {
            pJpegFile->SeekToPos(HeaderPos);

            if (pJpegFile->GetCurrentPos() != HeaderPos)
                break;

            if (pJpegFile->Read(Header, sizeof(Header)) != sizeof(Header))
                break;

            if ((Header[0] != 0xFF) || 
                (Header[1] & 0xf0) != 0xe0)
                break; // Not an APP chunk.

            if ((Header[1] == 0xe1) && 
                strncmp((const char *) Header + 4,"Exif",4) == 0 )
            {
                App1Offset = HeaderPos + 10;

                if(pi_prUrl->IsCompatibleWith(HFCURLFile::CLASS_ID))
                    {
                    pTIFFFile = new HTIFFFile(static_cast<HFCURLFile*>(pi_prUrl.GetPtr())->GetAbsoluteFileName(), 
                                                HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 
                                                App1Offset, 
                                                false, 
                                                false);

                    HASSERT(pTIFFFile->GetFilePtr()->GetLastException() == 0);
                    }
                else
                    {
                    _tprintf (_TEXT("\n--------- ErrorMSG ---------\nScheme type not supported\n"));
                    }
                
                break; // APP1 - Exif
            }

            HeaderPos += 2 + Header[2] * 256 + Header[3];
        }

        if (pTIFFFile == 0)
        {
            _tprintf (_TEXT("\nWarning : No EXIF section found in the JPEG file : %s\n"), 
                      WString(pi_prUrl->GetURL().c_str(), BentleyCharEncoding::Utf8).c_str());
        }
    }

    return pTIFFFile;
}