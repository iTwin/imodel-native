/******************************************************************************
 * $Id: jpgdataset.cpp 20996 2010-10-28 18:38:15Z rouault $
 *
 * Project:  JPEG JFIF Driver
 * Purpose:  Implement GDAL JPEG Support based on IJG libjpeg.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Frank Warmerdam
 *
 * Portions Copyright (c) Her majesty the Queen in right of Canada as
 * represented by the Minister of National Defence, 2006.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "gdal_pam.h"
#include "cpl_string.h"
#include "gdalexif.h"
#include <zlib/zlib.h>

#include <setjmp.h>


CPL_CVSID("$Id: jpgdataset.cpp 20996 2010-10-28 18:38:15Z rouault $");

CPL_C_START
#ifdef LIBJPEG_12_PATH 
#  include LIBJPEG_12_PATH
#else
#  include "jpeglib.h"
#endif
CPL_C_END

#if defined(JPEG_DUAL_MODE_8_12) && !defined(JPGDataset)
GDALDataset* JPEGDataset12Open(GDALOpenInfo* poOpenInfo);
GDALDataset*
        JPEGCreateCopy12( const char * pszFilename, GDALDataset *poSrcDS, 
                          int bStrict, char ** papszOptions, 
                          GDALProgressFunc pfnProgress, void * pProgressData );
#endif

CPL_C_START
void	GDALRegister_JPEG(void);
CPL_C_END

void jpeg_vsiio_src (j_decompress_ptr cinfo, VSILFILE * infile);
void jpeg_vsiio_dest (j_compress_ptr cinfo, VSILFILE * outfile);

/*  
* Do we want to do special processing suitable for when JSAMPLE is a 
* 16bit value?   
*/ 
#if defined(JPEG_LIB_MK1)
#  define JPEG_LIB_MK1_OR_12BIT 1
#elif BITS_IN_JSAMPLE == 12
#  define JPEG_LIB_MK1_OR_12BIT 1
#endif

/************************************************************************/
/* ==================================================================== */
/*				JPGDataset				*/
/* ==================================================================== */
/************************************************************************/

class JPGRasterBand;
class JPGMaskBand;

class JPGDataset : public GDALPamDataset
{
    friend class JPGRasterBand;
    friend class JPGMaskBand;

    struct jpeg_decompress_struct sDInfo;
    struct jpeg_error_mgr sJErr;
    jmp_buf setjmp_buffer;

    char   *pszProjection;
    int	   bGeoTransformValid;
    double adfGeoTransform[6];
    int	   nGCPCount;
    GDAL_GCP *pasGCPList;

    VSILFILE   *fpImage;
    GUIntBig nSubfileOffset;

    int    nLoadedScanline;
    GByte  *pabyScanline;

    int    bHasReadEXIFMetadata;
    char   **papszMetadata;
    char   **papszSubDatasets;
    int	   bigendian;
    int    nExifOffset;
    int    nInterOffset;
    int    nGPSOffset;
    int	   bSwabflag;
    int    nTiffDirStart;
    int    nTIFFHEADER;
    int    bHasDoneJpegStartDecompress;

    CPLErr LoadScanline(int);
    void   Restart();
    
    CPLErr EXIFExtractMetadata(VSILFILE *, int);
    int    EXIFInit(VSILFILE *);
    void   EXIFPrintData(char *, GUInt16, GUInt32, unsigned char* );

    int    nQLevel;
    void   LoadDefaultTables(int);

    void   CheckForMask();
    void   DecompressMask();

    void   ReadEXIFMetadata();

    int    bHasCheckedForMask;
    JPGMaskBand *poMaskBand;
    GByte  *pabyBitMask;

    GByte  *pabyCMask;
    int    nCMaskSize;

    J_COLOR_SPACE eGDALColorSpace;   /* color space exposed by GDAL. Not necessarily the in_color_space nor */
                                     /* the out_color_space of JPEG library */

  public:
                 JPGDataset();
                 ~JPGDataset();

    virtual CPLErr      IRasterIO( GDALRWFlag, int, int, int, int,
                                   void *, int, int, GDALDataType,
                                   int, int *, int, int, int );

    virtual CPLErr GetGeoTransform( double * );

    virtual int    GetGCPCount();
    virtual const char *GetGCPProjection();
    virtual const GDAL_GCP *GetGCPs();

    virtual char  **GetMetadata( const char * pszDomain = "" );
    virtual const char *GetMetadataItem( const char * pszName,
                                         const char * pszDomain = "" );


    static GDALDataset *Open( GDALOpenInfo * );
    static int          Identify( GDALOpenInfo * );

    static void ErrorExit(j_common_ptr cinfo);
};

/************************************************************************/
/* ==================================================================== */
/*                            JPGRasterBand                             */
/* ==================================================================== */
/************************************************************************/

class JPGRasterBand : public GDALPamRasterBand
{
    friend class JPGDataset;

    /* We have to keep a pointer to the JPGDataset that this JPGRasterBand
       belongs to. In some case, we may have this->poGDS != this->poDS
       For example for a JPGRasterBand that is set to a NITFDataset...
       In other words, this->poDS doesn't necessary point to a JPGDataset
       See ticket #1807.
    */
    JPGDataset	   *poGDS;

  public:

                   JPGRasterBand( JPGDataset *, int );

    virtual CPLErr IReadBlock( int, int, void * );
    virtual GDALColorInterp GetColorInterpretation();

    virtual GDALRasterBand *GetMaskBand();
    virtual int             GetMaskFlags();
};

/************************************************************************/
/* ==================================================================== */
/*                             JPGMaskBand                              */
/* ==================================================================== */
/************************************************************************/

class JPGMaskBand : public GDALRasterBand
{
  protected:
    virtual CPLErr IReadBlock( int, int, void * );

  public:
		JPGMaskBand( JPGDataset *poDS );
};

/************************************************************************/
/*                       ReadEXIFMetadata()                             */
/************************************************************************/
void JPGDataset::ReadEXIFMetadata()
{
    if (bHasReadEXIFMetadata)
        return;

    CPLAssert(papszMetadata == NULL);

    /* Save current position to avoid disturbing JPEG stream decoding */
    vsi_l_offset nCurOffset = VSIFTellL(fpImage);

    if( EXIFInit(fpImage) )
    {
        EXIFExtractMetadata(fpImage,nTiffDirStart);

        if(nExifOffset  > 0){ 
            EXIFExtractMetadata(fpImage,nExifOffset);
        }
        if(nInterOffset > 0) {
            EXIFExtractMetadata(fpImage,nInterOffset);
        }
        if(nGPSOffset > 0) {
            EXIFExtractMetadata(fpImage,nGPSOffset);
        }

        /* Avoid setting the PAM dirty bit just for that */
        int nOldPamFlags = nPamFlags;

        /* Append metadata from PAM after EXIF metadata */
        papszMetadata = CSLMerge(papszMetadata, GDALPamDataset::GetMetadata());
        SetMetadata( papszMetadata );

        nPamFlags = nOldPamFlags;
    }

    VSIFSeekL( fpImage, nCurOffset, SEEK_SET );

    bHasReadEXIFMetadata = TRUE;
}

/************************************************************************/
/*                           GetMetadata()                              */
/************************************************************************/
char  **JPGDataset::GetMetadata( const char * pszDomain )
{
    if (eAccess == GA_ReadOnly && !bHasReadEXIFMetadata &&
        (pszDomain == NULL || EQUAL(pszDomain, "")))
        ReadEXIFMetadata();
    return GDALPamDataset::GetMetadata(pszDomain);
}

/************************************************************************/
/*                       GetMetadataItem()                              */
/************************************************************************/
const char *JPGDataset::GetMetadataItem( const char * pszName,
                                         const char * pszDomain )
{
    if (eAccess == GA_ReadOnly && !bHasReadEXIFMetadata &&
        (pszDomain == NULL || EQUAL(pszDomain, "")) &&
        pszName != NULL && EQUALN(pszName, "EXIF_", 5))
        ReadEXIFMetadata();
    return GDALPamDataset::GetMetadataItem(pszName, pszDomain);
}

/************************************************************************/
/*                         EXIFPrintData()                              */
/************************************************************************/
void JPGDataset::EXIFPrintData(char* pszData, GUInt16 type, 
			    GUInt32 count, unsigned char* data)
{
  const char* sep = "";
  char  pszTemp[128];
  char* pszDataEnd = pszData;

  pszData[0]='\0';

  switch (type) {

  case TIFF_UNDEFINED:
  case TIFF_BYTE:
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%#02x", sep, *data++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;

  case TIFF_SBYTE:
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%d", sep, *(char *)data++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
	  
  case TIFF_ASCII:
    memcpy( pszData, data, count );
    pszData[count] = '\0';
    break;

  case TIFF_SHORT: {
    register GUInt16 *wp = (GUInt16*)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%u", sep, *wp++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_SSHORT: {
    register GInt16 *wp = (GInt16*)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%d", sep, *wp++), sep = " ";
      strcat(pszData,pszTemp);
    }
    break;
  }
  case TIFF_LONG: {
    register GUInt32 *lp = (GUInt32*)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%lu", sep, (unsigned long) *lp++);
      sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_SLONG: {
    register GInt32 *lp = (GInt32*)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%ld", sep, (long) *lp++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_RATIONAL: {
      register GUInt32 *lp = (GUInt32*)data;
      //      if(bSwabflag)
      //	  TIFFSwabArrayOfLong((GUInt32*) data, 2*count);
      for(;count>0;count--) {
	  if( (lp[0]==0) && (lp[1] == 0) ) {
	      sprintf(pszTemp,"%s(0)",sep);
	  }
	  else{
	      sprintf(pszTemp, "%s(%g)", sep,
		      (double) lp[0]/ (double)lp[1]);
	  }
	  sep = " ";
	  lp += 2;
	  strcat(pszData,pszTemp);
      }
      break;
  }
  case TIFF_SRATIONAL: {
    register GInt32 *lp = (GInt32*)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s(%g)", sep,
	      (float) lp[0]/ (float) lp[1]);
      sep = " ";
      lp += 2;
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_FLOAT: {
    register float *fp = (float *)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%g", sep, *fp++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_DOUBLE: {
    register double *dp = (double *)data;
    for(;count>0;count--) {
      sprintf(pszTemp, "%s%g", sep, *dp++), sep = " ";
      if (strlen(pszTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,pszTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }

  default:
    return;
  }

  if (type != TIFF_ASCII && count != 0)
  {
      CPLError(CE_Warning, CPLE_AppDefined, "EXIF metadata truncated");
  }
}

/************************************************************************/
/*                        EXIFInit()                                    */
/*                                                                      */
/*           Create Metadata from Information file directory APP1       */
/************************************************************************/
int JPGDataset::EXIFInit(VSILFILE *fp)
{
    int           one = 1;
    TIFFHeader    hdr;
  
    bigendian = (*(char *)&one == 0);

/* -------------------------------------------------------------------- */
/*      Search for APP1 chunk.                                          */
/* -------------------------------------------------------------------- */
    GByte abyChunkHeader[10];
    int nChunkLoc = 2;

    for( ; TRUE; ) 
    {
        if( VSIFSeekL( fp, nChunkLoc, SEEK_SET ) != 0 )
            return FALSE;

        if( VSIFReadL( abyChunkHeader, sizeof(abyChunkHeader), 1, fp ) != 1 )
            return FALSE;

        if( abyChunkHeader[0] != 0xFF 
            || (abyChunkHeader[1] & 0xf0) != 0xe0 )
            return FALSE; // Not an APP chunk.

        if( abyChunkHeader[1] == 0xe1 
            && strncmp((const char *) abyChunkHeader + 4,"Exif",4) == 0 )
        {
            nTIFFHEADER = nChunkLoc + 10;
            break; // APP1 - Exif
        }

        nChunkLoc += 2 + abyChunkHeader[2] * 256 + abyChunkHeader[3];
    }

/* -------------------------------------------------------------------- */
/*      Read TIFF header                                                */
/* -------------------------------------------------------------------- */
    VSIFSeekL(fp, nTIFFHEADER, SEEK_SET);
    if(VSIFReadL(&hdr,1,sizeof(hdr),fp) != sizeof(hdr)) 
        CPLError( CE_Failure, CPLE_FileIO,
                  "Failed to read %d Byte from image header.",
                  (int) sizeof(hdr));

    if (hdr.tiff_magic != TIFF_BIGENDIAN && hdr.tiff_magic != TIFF_LITTLEENDIAN)
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Not a TIFF file, bad magic number %u (%#x)",
                  hdr.tiff_magic, hdr.tiff_magic);

    if (hdr.tiff_magic == TIFF_BIGENDIAN)    bSwabflag = !bigendian;
    if (hdr.tiff_magic == TIFF_LITTLEENDIAN) bSwabflag = bigendian;


    if (bSwabflag) {
        TIFFSwabShort(&hdr.tiff_version);
        TIFFSwabLong(&hdr.tiff_diroff);
    }


    if (hdr.tiff_version != TIFF_VERSION)
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Not a TIFF file, bad version number %u (%#x)",
                 hdr.tiff_version, hdr.tiff_version); 
    nTiffDirStart = hdr.tiff_diroff;

    CPLDebug( "JPEG", "Magic: %#x <%s-endian> Version: %#x\n",
              hdr.tiff_magic,
              hdr.tiff_magic == TIFF_BIGENDIAN ? "big" : "little",
              hdr.tiff_version );

    return TRUE;
}

/************************************************************************/
/*                        EXIFExtractMetadata()                         */
/*                                                                      */
/*      Extract all entry from a IFD                                    */
/************************************************************************/
CPLErr JPGDataset::EXIFExtractMetadata(VSILFILE *fp, int nOffset)
{
    GUInt16        nEntryCount;
    int space;
    unsigned int           n,i;
    char          pszTemp[MAXSTRINGLENGTH];
    char          pszName[128];

    TIFFDirEntry *poTIFFDirEntry;
    TIFFDirEntry *poTIFFDir;
    const struct tagname *poExifTags ;
    const struct intr_tag *poInterTags = intr_tags;
    const struct gpsname *poGPSTags;

/* -------------------------------------------------------------------- */
/*      Read number of entry in directory                               */
/* -------------------------------------------------------------------- */
    if( VSIFSeekL(fp, nOffset+nTIFFHEADER, SEEK_SET) != 0 
        || VSIFReadL(&nEntryCount,1,sizeof(GUInt16),fp) != sizeof(GUInt16) )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Error reading EXIF Directory count at %d.",
                  nOffset + nTIFFHEADER );
        return CE_Failure;
    }

    if (bSwabflag)
        TIFFSwabShort(&nEntryCount);

    // Some apps write empty directories - see bug 1523.
    if( nEntryCount == 0 )  
        return CE_None;

    // Some files are corrupt, a large entry count is a sign of this.
    if( nEntryCount > 125 )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Ignoring EXIF directory with unlikely entry count (%d).",
                  nEntryCount );
        return CE_Warning;
    }

    poTIFFDir = (TIFFDirEntry *)CPLMalloc(nEntryCount * sizeof(TIFFDirEntry));
  
/* -------------------------------------------------------------------- */
/*      Read all directory entries                                      */
/* -------------------------------------------------------------------- */
    n = VSIFReadL(poTIFFDir, 1,nEntryCount*sizeof(TIFFDirEntry),fp);
    if (n != nEntryCount*sizeof(TIFFDirEntry)) 
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Could not read all directories");
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Parse all entry information in this directory                   */
/* -------------------------------------------------------------------- */
    for(poTIFFDirEntry = poTIFFDir,i=nEntryCount; i > 0; i--,poTIFFDirEntry++) {
        if (bSwabflag) {
            TIFFSwabShort(&poTIFFDirEntry->tdir_tag);
            TIFFSwabShort(&poTIFFDirEntry->tdir_type);
            TIFFSwabLong (&poTIFFDirEntry->tdir_count);
            TIFFSwabLong (&poTIFFDirEntry->tdir_offset);
        }

/* -------------------------------------------------------------------- */
/*      Find Tag name in table                                          */
/* -------------------------------------------------------------------- */
        pszName[0] = '\0';
        pszTemp[0] = '\0';

        for (poExifTags = tagnames; poExifTags->tag; poExifTags++)
            if(poExifTags->tag == poTIFFDirEntry->tdir_tag) {
                CPLAssert( NULL != poExifTags && NULL != poExifTags->name );

                strcpy(pszName, poExifTags->name);
                break;
            }

    
        if( nOffset == nGPSOffset) {
            for( poGPSTags = gpstags; poGPSTags->tag != 0xffff; poGPSTags++ ) 
                if( poGPSTags->tag == poTIFFDirEntry->tdir_tag ) {
                    CPLAssert( NULL != poGPSTags && NULL != poGPSTags->name );
                    strcpy(pszName, poGPSTags->name);
                    break;
                }
        }
/* -------------------------------------------------------------------- */
/*      If the tag was not found, look into the interoperability table  */
/* -------------------------------------------------------------------- */
        if( nOffset == nInterOffset ) {
            for(poInterTags = intr_tags; poInterTags->tag; poInterTags++)
                if(poInterTags->tag == poTIFFDirEntry->tdir_tag) {
                    CPLAssert( NULL != poInterTags && NULL != poInterTags->name );
                    strcpy(pszName, poInterTags->name);
                    break;
                }
        }

/* -------------------------------------------------------------------- */
/*      Save important directory tag offset                             */
/* -------------------------------------------------------------------- */
        if( poTIFFDirEntry->tdir_tag == EXIFOFFSETTAG )
            nExifOffset=poTIFFDirEntry->tdir_offset;
        if( poTIFFDirEntry->tdir_tag == INTEROPERABILITYOFFSET )
            nInterOffset=poTIFFDirEntry->tdir_offset;
        if( poTIFFDirEntry->tdir_tag == GPSOFFSETTAG ) {
            nGPSOffset=poTIFFDirEntry->tdir_offset;
        }

/* -------------------------------------------------------------------- */
/*      If we didn't recognise the tag just ignore it.  To see all      */
/*      tags comment out the continue.                                  */
/* -------------------------------------------------------------------- */
        if( pszName[0] == '\0' )
        {
            sprintf( pszName, "EXIF_%d", poTIFFDirEntry->tdir_tag );
            continue;
        }

/* -------------------------------------------------------------------- */
/*      For UserComment we need to ignore the language binding and      */
/*      just return the actual contents.                                */
/* -------------------------------------------------------------------- */
        if( EQUAL(pszName,"EXIF_UserComment")  )
        {
            poTIFFDirEntry->tdir_type = TIFF_ASCII;
            
            if( poTIFFDirEntry->tdir_count >= 8 )
            {
                poTIFFDirEntry->tdir_count -= 8;
                poTIFFDirEntry->tdir_offset += 8;
            }
        }

/* -------------------------------------------------------------------- */
/*      Make some UNDEFINED or BYTE fields ASCII for readability.       */
/* -------------------------------------------------------------------- */
        if( EQUAL(pszName,"EXIF_ExifVersion")
            || EQUAL(pszName,"EXIF_FlashPixVersion")
            || EQUAL(pszName,"EXIF_MakerNote")
            || EQUAL(pszName,"GPSProcessingMethod") )
            poTIFFDirEntry->tdir_type = TIFF_ASCII;

/* -------------------------------------------------------------------- */
/*      Print tags                                                      */
/* -------------------------------------------------------------------- */
        int nDataWidth = TIFFDataWidth((TIFFDataType) poTIFFDirEntry->tdir_type);
        space = poTIFFDirEntry->tdir_count * nDataWidth;

        /* Previous multiplication could overflow, hence this additional check */
        if (poTIFFDirEntry->tdir_count > MAXSTRINGLENGTH)
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Too many bytes in tag: %u, ignoring tag.", 
                      poTIFFDirEntry->tdir_count );
        }
        else if (nDataWidth == 0 || poTIFFDirEntry->tdir_type >= TIFF_IFD )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Invalid or unhandled EXIF data type: %d, ignoring tag.", 
                      poTIFFDirEntry->tdir_type );
        }
/* -------------------------------------------------------------------- */
/*      This is at most 4 byte data so we can read it from tdir_offset  */
/* -------------------------------------------------------------------- */
        else if (space >= 0 && space <= 4) {

            unsigned char data[4];
            memcpy(data, &poTIFFDirEntry->tdir_offset, 4);
            if (bSwabflag)
            {
                // Unswab 32bit value, and reswab per data type.
                TIFFSwabLong((GUInt32*) data);

                switch (poTIFFDirEntry->tdir_type) {
                  case TIFF_LONG:
                  case TIFF_SLONG:
                  case TIFF_FLOAT: 
                    TIFFSwabLong((GUInt32*) data);
                    break;

                  case TIFF_SSHORT:
                  case TIFF_SHORT:
                    TIFFSwabArrayOfShort((GUInt16*) data, 
                                         poTIFFDirEntry->tdir_count);
                  break;

                  default:
                    break;
                }
            }

            EXIFPrintData(pszTemp,
                          poTIFFDirEntry->tdir_type, 
                          poTIFFDirEntry->tdir_count, data);
        }
/* -------------------------------------------------------------------- */
/*      The data is being read where tdir_offset point to in the file   */
/* -------------------------------------------------------------------- */
        else if (space > 0 && space < MAXSTRINGLENGTH) 
        {
            unsigned char *data = (unsigned char *)VSIMalloc(space);

            if (data) {
                VSIFSeekL(fp,poTIFFDirEntry->tdir_offset+nTIFFHEADER,SEEK_SET);
                VSIFReadL(data, 1, space, fp);

                if (bSwabflag) {
                    switch (poTIFFDirEntry->tdir_type) {
                      case TIFF_SHORT:
                      case TIFF_SSHORT:
                        TIFFSwabArrayOfShort((GUInt16*) data, 
                                             poTIFFDirEntry->tdir_count);
                        break;
                      case TIFF_LONG:
                      case TIFF_SLONG:
                      case TIFF_FLOAT:
                        TIFFSwabArrayOfLong((GUInt32*) data, 
                                            poTIFFDirEntry->tdir_count);
                        break;
                      case TIFF_RATIONAL:
                      case TIFF_SRATIONAL:
                        TIFFSwabArrayOfLong((GUInt32*) data, 
                                            2*poTIFFDirEntry->tdir_count);
                        break;
                      case TIFF_DOUBLE:
                        TIFFSwabArrayOfDouble((double*) data, 
                                              poTIFFDirEntry->tdir_count);
                        break;
                      default:
                        break;
                    }
                }

                EXIFPrintData(pszTemp, poTIFFDirEntry->tdir_type,
                              poTIFFDirEntry->tdir_count, data);
                CPLFree(data);
            }
        }
        else
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Invalid EXIF header size: %ld, ignoring tag.", 
                      (long) space );
        }

        papszMetadata = CSLSetNameValue(papszMetadata, pszName, pszTemp);
    }
    CPLFree(poTIFFDir);

    return CE_None;
}

/************************************************************************/
/*                            JPGMaskBand()                             */
/************************************************************************/

JPGMaskBand::JPGMaskBand( JPGDataset *poDS )

{
    this->poDS = poDS;
    nBand = 0;

    nRasterXSize = poDS->GetRasterXSize();
    nRasterYSize = poDS->GetRasterYSize();

    eDataType = GDT_Byte;
    nBlockXSize = nRasterXSize;
    nBlockYSize = 1;
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr JPGMaskBand::IReadBlock( int nBlockX, int nBlockY, void *pImage )

{
    JPGDataset *poJDS = (JPGDataset *) poDS;

/* -------------------------------------------------------------------- */
/*      Make sure the mask is loaded and decompressed.                  */
/* -------------------------------------------------------------------- */
    poJDS->DecompressMask();
    if( poJDS->pabyBitMask == NULL )
        return CE_Failure;

/* -------------------------------------------------------------------- */
/*      Set mask based on bitmask for this scanline.                    */
/* -------------------------------------------------------------------- */
    int iX;
    int iBit = nBlockY * nBlockXSize;

    for( iX = 0; iX < nBlockXSize; iX++ )
    {
        if( poJDS->pabyBitMask[iBit>>3] & (0x1 << (iBit&7)) )
            ((GByte *) pImage)[iX] = 255;
        else
            ((GByte *) pImage)[iX] = 0;
        iBit++;
    }

    return CE_None;
}

/************************************************************************/
/*                           JPGRasterBand()                            */
/************************************************************************/

JPGRasterBand::JPGRasterBand( JPGDataset *poDS, int nBand )

{
    this->poDS = poGDS = poDS;

    this->nBand = nBand;
    if( poDS->sDInfo.data_precision == 12 )
        eDataType = GDT_UInt16;
    else
        eDataType = GDT_Byte;

    nBlockXSize = poDS->nRasterXSize;;
    nBlockYSize = 1;

    GDALMajorObject::SetMetadataItem("COMPRESSION","JPEG","IMAGE_STRUCTURE");
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr JPGRasterBand::IReadBlock( int nBlockXOff, int nBlockYOff,
                                  void * pImage )

{
    CPLErr      eErr;
    int         nXSize = GetXSize();
    int         nWordSize = GDALGetDataTypeSize(eDataType) / 8;
    
    CPLAssert( nBlockXOff == 0 );

/* -------------------------------------------------------------------- */
/*      Load the desired scanline into the working buffer.              */
/* -------------------------------------------------------------------- */
    eErr = poGDS->LoadScanline( nBlockYOff );
    if( eErr != CE_None )
        return eErr;

/* -------------------------------------------------------------------- */
/*      Transfer between the working buffer the the callers buffer.     */
/* -------------------------------------------------------------------- */
    if( poGDS->GetRasterCount() == 1 )
    {
#ifdef JPEG_LIB_MK1_OR_12BIT
        GDALCopyWords( poGDS->pabyScanline, GDT_UInt16, 2, 
                       pImage, eDataType, nWordSize, 
                       nXSize );
#else
        memcpy( pImage, poGDS->pabyScanline, nXSize * nWordSize );
#endif
    }
    else
    {
#ifdef JPEG_LIB_MK1_OR_12BIT
        GDALCopyWords( poGDS->pabyScanline + (nBand-1) * 2, 
                       GDT_UInt16, 6, 
                       pImage, eDataType, nWordSize, 
                       nXSize );
#else
        if (poGDS->eGDALColorSpace == JCS_RGB &&
            poGDS->sDInfo.out_color_space == JCS_CMYK)
        {
            CPLAssert(eDataType == GDT_Byte);
            int i;
            if (nBand == 1)
            {
                for(i=0;i<nXSize;i++)
                {
                    int C = poGDS->pabyScanline[i * 4 + 0];
                    int K = poGDS->pabyScanline[i * 4 + 3];
                    ((GByte*)pImage)[i] = (C * K) / 255;
                }
            }
            else  if (nBand == 2)
            {
                for(i=0;i<nXSize;i++)
                {
                    int M = poGDS->pabyScanline[i * 4 + 1];
                    int K = poGDS->pabyScanline[i * 4 + 3];
                    ((GByte*)pImage)[i] = (M * K) / 255;
                }
            }
            else if (nBand == 3)
            {
                for(i=0;i<nXSize;i++)
                {
                    int Y = poGDS->pabyScanline[i * 4 + 2];
                    int K = poGDS->pabyScanline[i * 4 + 3];
                    ((GByte*)pImage)[i] = (Y * K) / 255;
                }
            }
        }
        else
        {
            GDALCopyWords( poGDS->pabyScanline + (nBand-1) * nWordSize, 
                        eDataType, nWordSize * poGDS->GetRasterCount(), 
                        pImage, eDataType, nWordSize, 
                        nXSize );
        }
#endif
    }

/* -------------------------------------------------------------------- */
/*      Forceably load the other bands associated with this scanline.   */
/* -------------------------------------------------------------------- */
    if( nBand == 1 )
    {
        GDALRasterBlock *poBlock;

        int iBand;
        for(iBand = 2; iBand <= poGDS->GetRasterCount() ; iBand++)
        {
            poBlock = 
                poGDS->GetRasterBand(iBand)->GetLockedBlockRef(nBlockXOff,nBlockYOff);
            poBlock->DropLock();
        }
    }


    return CE_None;
}

/************************************************************************/
/*                       GetColorInterpretation()                       */
/************************************************************************/

GDALColorInterp JPGRasterBand::GetColorInterpretation()

{
    if( poGDS->eGDALColorSpace == JCS_GRAYSCALE )
        return GCI_GrayIndex;

    else if( poGDS->eGDALColorSpace == JCS_RGB)
    {
        if ( nBand == 1 )
            return GCI_RedBand;

        else if( nBand == 2 )
            return GCI_GreenBand;

        else 
            return GCI_BlueBand;
    }
    else if( poGDS->eGDALColorSpace == JCS_CMYK)
    {
        if ( nBand == 1 )
            return GCI_CyanBand;

        else if( nBand == 2 )
            return GCI_MagentaBand;

        else if ( nBand == 3 )
            return GCI_YellowBand;

        else
            return GCI_BlackBand;
    }
    else if( poGDS->eGDALColorSpace == JCS_YCbCr ||
             poGDS->eGDALColorSpace == JCS_YCCK)
    {
        if ( nBand == 1 )
            return GCI_YCbCr_YBand;

        else if( nBand == 2 )
            return GCI_YCbCr_CbBand;

        else if ( nBand == 3 )
            return GCI_YCbCr_CrBand;

        else
            return GCI_BlackBand;
    }
    else
    {
        CPLAssert(0);
        return GCI_Undefined;
    }
}

/************************************************************************/
/*                            GetMaskBand()                             */
/************************************************************************/

GDALRasterBand *JPGRasterBand::GetMaskBand()

{
    if( !poGDS->bHasCheckedForMask)
    {
        poGDS->CheckForMask();
        poGDS->bHasCheckedForMask = TRUE;
    }
    if( poGDS->pabyCMask )
    {
        if( poGDS->poMaskBand == NULL )
            poGDS->poMaskBand = new JPGMaskBand( (JPGDataset *) poDS );

        return poGDS->poMaskBand;
    }
    else
        return GDALPamRasterBand::GetMaskBand();
}

/************************************************************************/
/*                            GetMaskFlags()                            */
/************************************************************************/

int JPGRasterBand::GetMaskFlags()

{
    GetMaskBand();
    if( poGDS->poMaskBand != NULL )
        return GMF_PER_DATASET;
    else
        return GDALPamRasterBand::GetMaskFlags();
}

/************************************************************************/
/* ==================================================================== */
/*                             JPGDataset                               */
/* ==================================================================== */
/************************************************************************/


/************************************************************************/
/*                            JPGDataset()                              */
/************************************************************************/

JPGDataset::JPGDataset()

{
    pabyScanline = NULL;
    nLoadedScanline = -1;

    bHasReadEXIFMetadata = FALSE;
    papszMetadata   = NULL;
    papszSubDatasets= NULL;
    nExifOffset     = -1;
    nInterOffset    = -1;
    nGPSOffset      = -1;

    pszProjection = NULL;
    bGeoTransformValid = FALSE;
    adfGeoTransform[0] = 0.0;
    adfGeoTransform[1] = 1.0;
    adfGeoTransform[2] = 0.0;
    adfGeoTransform[3] = 0.0;
    adfGeoTransform[4] = 0.0;
    adfGeoTransform[5] = 1.0;
    nGCPCount = 0;
    pasGCPList = NULL;

    bHasDoneJpegStartDecompress = FALSE;

    bHasCheckedForMask = FALSE;
    poMaskBand = NULL;
    pabyBitMask = NULL;
    pabyCMask = NULL;
    nCMaskSize = 0;

    eGDALColorSpace = JCS_UNKNOWN;
}

/************************************************************************/
/*                           ~JPGDataset()                            */
/************************************************************************/

JPGDataset::~JPGDataset()

{
    FlushCache();

    jpeg_abort_decompress( &sDInfo );
    jpeg_destroy_decompress( &sDInfo );

    if( fpImage != NULL )
        VSIFCloseL( fpImage );

    if( pabyScanline != NULL )
        CPLFree( pabyScanline );
    if( papszMetadata != NULL )
      CSLDestroy( papszMetadata );

    if ( pszProjection )
        CPLFree( pszProjection );

    if ( nGCPCount > 0 )
    {
        GDALDeinitGCPs( nGCPCount, pasGCPList );
        CPLFree( pasGCPList );
    }

    CPLFree( pabyBitMask );
    CPLFree( pabyCMask );
    delete poMaskBand;
}

/************************************************************************/
/*                            LoadScanline()                            */
/************************************************************************/

CPLErr JPGDataset::LoadScanline( int iLine )

{
    if( nLoadedScanline == iLine )
        return CE_None;

    // setup to trap a fatal error.
    if (setjmp(setjmp_buffer)) 
        return CE_Failure;

    if (!bHasDoneJpegStartDecompress)
    {
        jpeg_start_decompress( &sDInfo );
        bHasDoneJpegStartDecompress = TRUE;
    }

    if( pabyScanline == NULL )
    {
        int nJPEGBands = 0;
        switch(sDInfo.out_color_space)
        {
            case JCS_GRAYSCALE:
                nJPEGBands = 1;
                break;
            case JCS_RGB:
            case JCS_YCbCr:
                nJPEGBands = 3;
                break;
            case JCS_CMYK:
            case JCS_YCCK:
                nJPEGBands = 4;
                break;

            default:
                CPLAssert(0);
        }

        pabyScanline = (GByte *)
            CPLMalloc(nJPEGBands * GetRasterXSize() * 2);
    }

    if( iLine < nLoadedScanline )
        Restart();
        
    while( nLoadedScanline < iLine )
    {
        JSAMPLE	*ppSamples;
            
        ppSamples = (JSAMPLE *) pabyScanline;
        jpeg_read_scanlines( &sDInfo, &ppSamples, 1 );
        nLoadedScanline++;
    }

    return CE_None;
}

/************************************************************************/
/*                         LoadDefaultTables()                          */
/************************************************************************/

const static GByte Q1table[64] = 
{
   8,    72,  72,  72,  72,  72,  72,  72, // 0 - 7
    72,   72,  78,  74,  76,  74,  78,  89, // 8 - 15
    81,   84,  84,  81,  89, 106,  93,  94, // 16 - 23
    99,   94,  93, 106, 129, 111, 108, 116, // 24 - 31
    116, 108, 111, 129, 135, 128, 136, 145, // 32 - 39
    136, 128, 135, 155, 160, 177, 177, 160, // 40 - 47
    155, 193, 213, 228, 213, 193, 255, 255, // 48 - 55
    255, 255, 255, 255, 255, 255, 255, 255  // 56 - 63
};

const static GByte Q2table[64] = 
{ 
    8, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 39, 37, 38, 37, 39, 45, 41, 42, 42, 41, 45, 53,
    47, 47, 50, 47, 47, 53, 65, 56, 54, 59, 59, 54, 56, 65, 68, 64, 69, 73,
    69, 64, 68, 78, 81, 89, 89, 81, 78, 98,108,115,108, 98,130,144,144,130,
    178,190,178,243,243,255
};

const static GByte Q3table[64] = 
{ 
     8, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 11, 10, 11, 10, 11, 13, 11, 12, 12, 11, 13, 15, 
    13, 13, 14, 13, 13, 15, 18, 16, 15, 16, 16, 15, 16, 18, 19, 18, 19, 21, 
    19, 18, 19, 22, 23, 25, 25, 23, 22, 27, 30, 32, 30, 27, 36, 40, 40, 36, 
    50, 53, 50, 68, 68, 91 
}; 

const static GByte Q4table[64] = 
{
    8, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 8, 7, 8, 7, 8, 9, 8, 8, 8, 8, 9, 11, 
    9, 9, 10, 9, 9, 11, 13, 11, 11, 12, 12, 11, 11, 13, 14, 13, 14, 15, 
    14, 13, 14, 16, 16, 18, 18, 16, 16, 20, 22, 23, 22, 20, 26, 29, 29, 26, 
    36, 38, 36, 49, 49, 65
};

const static GByte Q5table[64] = 
{
    4, 4, 4, 4, 
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 6, 5, 5, 6, 7, 6, 6, 6, 6, 6, 6, 7, 8, 7, 8, 8, 
    8, 7, 8, 9, 9, 10, 10, 9, 9, 11, 12, 13, 12, 11, 14, 16, 16, 14, 
    20, 21, 20, 27, 27, 36
};

static const GByte AC_BITS[16] = 
{ 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125 };

static const GByte AC_HUFFVAL[256] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,          
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
    0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
    0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
    0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
    0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
    0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
    0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
    0xF9, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const GByte DC_BITS[16] = 
{ 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };

static const GByte DC_HUFFVAL[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0A, 0x0B };


void JPGDataset::LoadDefaultTables( int n )

{
    if( nQLevel < 1 )
        return;

/* -------------------------------------------------------------------- */
/*      Load quantization table						*/
/* -------------------------------------------------------------------- */
    int i;
    JQUANT_TBL  *quant_ptr;
    const GByte *pabyQTable;

    if( nQLevel == 1 )
        pabyQTable = Q1table;
    else if( nQLevel == 2 )
        pabyQTable = Q2table;
    else if( nQLevel == 3 )
        pabyQTable = Q3table;
    else if( nQLevel == 4 )
        pabyQTable = Q4table;
    else if( nQLevel == 5 )
        pabyQTable = Q5table;
    else
        return;

    if (sDInfo.quant_tbl_ptrs[n] == NULL)
        sDInfo.quant_tbl_ptrs[n] = 
            jpeg_alloc_quant_table((j_common_ptr) &(sDInfo));
    
    quant_ptr = sDInfo.quant_tbl_ptrs[n];	/* quant_ptr is JQUANT_TBL* */
    for (i = 0; i < 64; i++) {
        /* Qtable[] is desired quantization table, in natural array order */
        quant_ptr->quantval[i] = pabyQTable[i];
    }

/* -------------------------------------------------------------------- */
/*      Load AC huffman table.                                          */
/* -------------------------------------------------------------------- */
    JHUFF_TBL  *huff_ptr;

    if (sDInfo.ac_huff_tbl_ptrs[n] == NULL)
        sDInfo.ac_huff_tbl_ptrs[n] =
            jpeg_alloc_huff_table((j_common_ptr)&sDInfo);

    huff_ptr = sDInfo.ac_huff_tbl_ptrs[n];	/* huff_ptr is JHUFF_TBL* */

    for (i = 1; i <= 16; i++) {
        /* counts[i] is number of Huffman codes of length i bits, i=1..16 */
        huff_ptr->bits[i] = AC_BITS[i-1];
    }

    for (i = 0; i < 256; i++) {
        /* symbols[] is the list of Huffman symbols, in code-length order */
        huff_ptr->huffval[i] = AC_HUFFVAL[i];
    }

/* -------------------------------------------------------------------- */
/*      Load DC huffman table.                                          */
/* -------------------------------------------------------------------- */
    if (sDInfo.dc_huff_tbl_ptrs[n] == NULL)
        sDInfo.dc_huff_tbl_ptrs[n] =
            jpeg_alloc_huff_table((j_common_ptr)&sDInfo);

    huff_ptr = sDInfo.dc_huff_tbl_ptrs[n];	/* huff_ptr is JHUFF_TBL* */

    for (i = 1; i <= 16; i++) {
        /* counts[i] is number of Huffman codes of length i bits, i=1..16 */
        huff_ptr->bits[i] = DC_BITS[i-1];
    }

    for (i = 0; i < 256; i++) {
        /* symbols[] is the list of Huffman symbols, in code-length order */
        huff_ptr->huffval[i] = DC_HUFFVAL[i];
    }

}

/************************************************************************/
/*                              Restart()                               */
/*                                                                      */
/*      Restart compressor at the beginning of the file.                */
/************************************************************************/

void JPGDataset::Restart()

{
    J_COLOR_SPACE colorSpace = sDInfo.out_color_space;

    jpeg_abort_decompress( &sDInfo );
    jpeg_destroy_decompress( &sDInfo );
    jpeg_create_decompress( &sDInfo );

    LoadDefaultTables( 0 );
    LoadDefaultTables( 1 );
    LoadDefaultTables( 2 );
    LoadDefaultTables( 3 );

/* -------------------------------------------------------------------- */
/*      restart io.                                                     */
/* -------------------------------------------------------------------- */
    VSIFSeekL( fpImage, nSubfileOffset, SEEK_SET );

    jpeg_vsiio_src( &sDInfo, fpImage );
    jpeg_read_header( &sDInfo, TRUE );
    
    sDInfo.out_color_space = colorSpace;
    nLoadedScanline = -1;
    jpeg_start_decompress( &sDInfo );
    bHasDoneJpegStartDecompress = TRUE;
}

/************************************************************************/
/*                          GetGeoTransform()                           */
/************************************************************************/

CPLErr JPGDataset::GetGeoTransform( double * padfTransform )

{
    if( bGeoTransformValid )
    {
        memcpy( padfTransform, adfGeoTransform, sizeof(double)*6 );
        
        return CE_None;
    }
    else 
        return GDALPamDataset::GetGeoTransform( padfTransform );
}

/************************************************************************/
/*                            GetGCPCount()                             */
/************************************************************************/

int JPGDataset::GetGCPCount()

{
    return nGCPCount;
}

/************************************************************************/
/*                          GetGCPProjection()                          */
/************************************************************************/

const char *JPGDataset::GetGCPProjection()

{
    if( pszProjection && nGCPCount > 0 )
        return pszProjection;
    else
        return "";
}

/************************************************************************/
/*                               GetGCPs()                              */
/************************************************************************/

const GDAL_GCP *JPGDataset::GetGCPs()

{
    return pasGCPList;
}

/************************************************************************/
/*                             IRasterIO()                              */
/*                                                                      */
/*      Checks for what might be the most common read case              */
/*      (reading an entire interleaved, 8bit, RGB JPEG), and            */
/*      optimizes for that case                                         */
/************************************************************************/

CPLErr JPGDataset::IRasterIO( GDALRWFlag eRWFlag, 
                              int nXOff, int nYOff, int nXSize, int nYSize,
                              void *pData, int nBufXSize, int nBufYSize, 
                              GDALDataType eBufType,
                              int nBandCount, int *panBandMap, 
                              int nPixelSpace, int nLineSpace, int nBandSpace )

{
    if((eRWFlag == GF_Read) &&
       (nBandCount == 3) &&
       (nBands == 3) &&
       (nXOff == 0) && (nXOff == 0) &&
       (nXSize == nBufXSize) && (nXSize == nRasterXSize) &&
       (nYSize == nBufYSize) && (nYSize == nRasterYSize) &&
       (eBufType == GDT_Byte) && (sDInfo.data_precision != 12) &&
       /*(nPixelSpace >= 3)*/(nPixelSpace > 3) &&
       (nLineSpace == (nPixelSpace*nXSize)) &&
       (nBandSpace == 1) &&
       (pData != NULL) &&
       (panBandMap != NULL) &&
       (panBandMap[0] == 1) && (panBandMap[1] == 2) && (panBandMap[2] == 3))
    {
        Restart();
        int y;
        CPLErr tmpError;
        int x;

        // handles copy with padding case
        for(y = 0; y < nYSize; ++y)
        {
            tmpError = LoadScanline(y);
            if(tmpError != CE_None) return tmpError;

            for(x = 0; x < nXSize; ++x)
            {
                tmpError = LoadScanline(y);
                if(tmpError != CE_None) return tmpError;
                memcpy(&(((GByte*)pData)[(y*nLineSpace) + (x*nPixelSpace)]), 
                       (const GByte*)&(pabyScanline[x*3]), 3);
            }
        }

        return CE_None;
    }

    return GDALPamDataset::IRasterIO(eRWFlag, nXOff, nYOff, nXSize, nYSize,
                                     pData, nBufXSize, nBufYSize, eBufType, 
                                     nBandCount, panBandMap, 
                                     nPixelSpace, nLineSpace, nBandSpace);
}

/************************************************************************/
/*                    JPEGDatasetIsJPEGLS()                             */
/************************************************************************/

static int JPEGDatasetIsJPEGLS( GDALOpenInfo * poOpenInfo )

{
    GByte  *pabyHeader = poOpenInfo->pabyHeader;
    int    nHeaderBytes = poOpenInfo->nHeaderBytes;

    if( nHeaderBytes < 10 )
        return FALSE;

    if( pabyHeader[0] != 0xff
        || pabyHeader[1] != 0xd8 )
        return FALSE;

    int nOffset = 2;
    for (;nOffset + 4 < nHeaderBytes;)
    {
        if (pabyHeader[nOffset] != 0xFF)
            return FALSE;

        int nMarker = pabyHeader[nOffset + 1];
        if (nMarker == 0xF7 /* JPEG Extension 7, JPEG-LS */)
            return TRUE;
        if (nMarker == 0xF8 /* JPEG Extension 8, JPEG-LS Extension */)
            return TRUE;
        if (nMarker == 0xC3 /* Start of Frame 3 */)
            return TRUE;
        if (nMarker == 0xC7 /* Start of Frame 7 */)
            return TRUE;
        if (nMarker == 0xCB /* Start of Frame 11 */)
            return TRUE;
        if (nMarker == 0xCF /* Start of Frame 15 */)
            return TRUE;

        nOffset += 2 + pabyHeader[nOffset + 2] * 256 + pabyHeader[nOffset + 3];
    }

    return FALSE;
}

/************************************************************************/
/*                              Identify()                              */
/************************************************************************/

int JPGDataset::Identify( GDALOpenInfo * poOpenInfo )

{
    GByte  *pabyHeader = NULL;
    int    nHeaderBytes = poOpenInfo->nHeaderBytes;

/* -------------------------------------------------------------------- */
/*      If it is a subfile, read the JPEG header.                       */
/* -------------------------------------------------------------------- */
    if( EQUALN(poOpenInfo->pszFilename,"JPEG_SUBFILE:",13) )
        return TRUE;

/* -------------------------------------------------------------------- */
/*	First we check to see if the file has the expected header	*/
/*	bytes.								*/    
/* -------------------------------------------------------------------- */
    pabyHeader = poOpenInfo->pabyHeader;

    if( nHeaderBytes < 10 )
        return FALSE;

    if( pabyHeader[0] != 0xff
        || pabyHeader[1] != 0xd8
        || pabyHeader[2] != 0xff )
        return FALSE;

    if (JPEGDatasetIsJPEGLS(poOpenInfo))
    {
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *JPGDataset::Open( GDALOpenInfo * poOpenInfo )

{
    if( !Identify( poOpenInfo ) )
        return NULL;

    if( poOpenInfo->eAccess == GA_Update )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "The JPEG driver does not support update access to existing"
                  " datasets.\n" );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      If it is a subfile, read the JPEG header.                       */
/* -------------------------------------------------------------------- */
    int bIsSubfile = FALSE;
    GUIntBig subfile_offset = 0;
    GUIntBig subfile_size = 0;
    const char *real_filename = poOpenInfo->pszFilename;
    int nQLevel = -1;

    if( ( poOpenInfo->fp == NULL ) &&
        ( EQUALN(poOpenInfo->pszFilename,"JPEG_SUBFILE:",13) ) )
    {
        char** papszTokens;
        int bScan = FALSE;

        if( EQUALN(poOpenInfo->pszFilename,"JPEG_SUBFILE:Q",14) )
        {
            papszTokens = CSLTokenizeString2(poOpenInfo->pszFilename + 14, ",", 0);
            if (CSLCount(papszTokens) >= 3)
            {
                nQLevel = atoi(papszTokens[0]);
                subfile_offset = CPLScanUIntBig(papszTokens[1], strlen(papszTokens[1]));
                subfile_size = CPLScanUIntBig(papszTokens[2], strlen(papszTokens[2]));
                bScan = TRUE;
            }
            CSLDestroy(papszTokens);
        }
        else
        {
            papszTokens = CSLTokenizeString2(poOpenInfo->pszFilename + 13, ",", 0);
            if (CSLCount(papszTokens) >= 2)
            {
                subfile_offset = CPLScanUIntBig(papszTokens[0], strlen(papszTokens[0]));
                subfile_size = CPLScanUIntBig(papszTokens[1], strlen(papszTokens[1]));
                bScan = TRUE;
            }
            CSLDestroy(papszTokens);
        }

        if( !bScan ) 
        {
            CPLError( CE_Failure, CPLE_OpenFailed, 
                      "Corrupt subfile definition: %s", 
                      poOpenInfo->pszFilename );
            return NULL;
        }

        real_filename = strstr(poOpenInfo->pszFilename,",");
        if( real_filename != NULL )
            real_filename = strstr(real_filename+1,",");
        if( real_filename != NULL && nQLevel != -1 )
            real_filename = strstr(real_filename+1,",");
        if( real_filename != NULL )
            real_filename++;
        else
        {
            CPLError( CE_Failure, CPLE_OpenFailed, 
                      "Could not find filename in subfile definition.");
            return NULL;
        }

        CPLDebug( "JPG",
                  "real_filename %s, offset=" CPL_FRMT_GUIB ", size=" CPL_FRMT_GUIB "\n", 
                  real_filename, subfile_offset, subfile_size);

        bIsSubfile = TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Create a corresponding GDALDataset.                             */
/* -------------------------------------------------------------------- */
    JPGDataset	*poDS;

    poDS = new JPGDataset();
    poDS->nQLevel = nQLevel;

/* -------------------------------------------------------------------- */
/*      Open the file using the large file api.                         */
/* -------------------------------------------------------------------- */
    poDS->fpImage = VSIFOpenL( real_filename, "rb" );
    
    if( poDS->fpImage == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed, 
                  "VSIFOpenL(%s) failed unexpectedly in jpgdataset.cpp", 
                  real_filename );
        delete poDS;
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Move to the start of jpeg data.                                 */
/* -------------------------------------------------------------------- */
    poDS->nSubfileOffset = subfile_offset;
    VSIFSeekL( poDS->fpImage, poDS->nSubfileOffset, SEEK_SET );

    poDS->eAccess = GA_ReadOnly;

    poDS->sDInfo.err = jpeg_std_error( &(poDS->sJErr) );
    poDS->sJErr.error_exit = JPGDataset::ErrorExit;
    poDS->sDInfo.client_data = (void *) &(poDS->setjmp_buffer);

    jpeg_create_decompress( &(poDS->sDInfo) );

    /* This is to address bug related in ticket #1795 */
    if (CPLGetConfigOption("JPEGMEM", NULL) == NULL)
    {
        /* If the user doesn't provide a value for JPEGMEM, we want to be sure */
        /* that at least 500 MB will be used before creating the temporary file */
        poDS->sDInfo.mem->max_memory_to_use =
                MAX(poDS->sDInfo.mem->max_memory_to_use, 500 * 1024 * 1024);
    }

/* -------------------------------------------------------------------- */
/*      Preload default NITF JPEG quantization tables.                  */
/* -------------------------------------------------------------------- */
    poDS->LoadDefaultTables( 0 );
    poDS->LoadDefaultTables( 1 );
    poDS->LoadDefaultTables( 2 );
    poDS->LoadDefaultTables( 3 );

/* -------------------------------------------------------------------- */
/*      If a fatal error occurs after this, we will return NULL         */
/* -------------------------------------------------------------------- */
    if (setjmp(poDS->setjmp_buffer)) 
    {
#if defined(JPEG_DUAL_MODE_8_12) && !defined(JPGDataset)
        if (poDS->sDInfo.data_precision == 12)
        {
            delete poDS;
            return JPEGDataset12Open(poOpenInfo);
        }
#endif
        delete poDS;
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*	Read pre-image data after ensuring the file is rewound.         */
/* -------------------------------------------------------------------- */
    VSIFSeekL( poDS->fpImage, poDS->nSubfileOffset, SEEK_SET );

    jpeg_vsiio_src( &(poDS->sDInfo), poDS->fpImage );
    jpeg_read_header( &(poDS->sDInfo), TRUE );

    if( poDS->sDInfo.data_precision != 8
        && poDS->sDInfo.data_precision != 12 )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "GDAL JPEG Driver doesn't support files with precision of"
                  " other than 8 or 12 bits." );
        delete poDS;
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Capture some information from the file that is of interest.     */
/* -------------------------------------------------------------------- */
    poDS->nRasterXSize = poDS->sDInfo.image_width;
    poDS->nRasterYSize = poDS->sDInfo.image_height;

    poDS->sDInfo.out_color_space = poDS->sDInfo.jpeg_color_space;
    poDS->eGDALColorSpace = poDS->sDInfo.jpeg_color_space;

    if( poDS->sDInfo.jpeg_color_space == JCS_GRAYSCALE )
    {
        poDS->nBands = 1;
    }
    else if( poDS->sDInfo.jpeg_color_space == JCS_RGB )
    {
        poDS->nBands = 3;
    }
    else if( poDS->sDInfo.jpeg_color_space == JCS_YCbCr )
    {
        poDS->nBands = 3;
        if (CSLTestBoolean(CPLGetConfigOption("GDAL_JPEG_TO_RGB", "YES")))
        {
            poDS->sDInfo.out_color_space = JCS_RGB;
            poDS->eGDALColorSpace = JCS_RGB;
            poDS->SetMetadataItem( "SOURCE_COLOR_SPACE", "YCbCr", "IMAGE_STRUCTURE" );
        }
    }
    else if( poDS->sDInfo.jpeg_color_space == JCS_CMYK )
    {
        if (CSLTestBoolean(CPLGetConfigOption("GDAL_JPEG_TO_RGB", "YES")))
        {
            poDS->eGDALColorSpace = JCS_RGB;
            poDS->nBands = 3;
            poDS->SetMetadataItem( "SOURCE_COLOR_SPACE", "CMYK", "IMAGE_STRUCTURE" );
        }
        else
        {
            poDS->nBands = 4;
        }
    }
    else if( poDS->sDInfo.jpeg_color_space == JCS_YCCK )
    {
        if (CSLTestBoolean(CPLGetConfigOption("GDAL_JPEG_TO_RGB", "YES")))
        {
            poDS->eGDALColorSpace = JCS_RGB;
            poDS->nBands = 3;
            poDS->SetMetadataItem( "SOURCE_COLOR_SPACE", "YCbCrK", "IMAGE_STRUCTURE" );
        }
        else
        {
            poDS->nBands = 4;
        }
        /* libjpeg does the translation from YCrCbK -> CMYK internally */
        /* and we'll do the translation to RGB in IReadBlock() */
        poDS->sDInfo.out_color_space = JCS_CMYK;
    }
    else
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "Unrecognised jpeg_color_space value of %d.\n", 
                  poDS->sDInfo.jpeg_color_space );
        delete poDS;
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create band information objects.                                */
/* -------------------------------------------------------------------- */
    for( int iBand = 0; iBand < poDS->nBands; iBand++ )
        poDS->SetBand( iBand+1, new JPGRasterBand( poDS, iBand+1 ) );

/* -------------------------------------------------------------------- */
/*      More metadata.                                                  */
/* -------------------------------------------------------------------- */
    if( poDS->nBands > 1 )
    {
        poDS->SetMetadataItem( "INTERLEAVE", "PIXEL", "IMAGE_STRUCTURE" );
        poDS->SetMetadataItem( "COMPRESSION", "JPEG", "IMAGE_STRUCTURE" );
    }

/* -------------------------------------------------------------------- */
/*      Initialize any PAM information.                                 */
/* -------------------------------------------------------------------- */
    poDS->SetDescription( poOpenInfo->pszFilename );
    
    if( !bIsSubfile )
        poDS->TryLoadXML();
    else
        poDS->nPamFlags |= GPF_NOSAVE;

/* -------------------------------------------------------------------- */
/*      Open overviews.                                                 */
/* -------------------------------------------------------------------- */
    poDS->oOvManager.Initialize( poDS, real_filename );

/* -------------------------------------------------------------------- */
/*      Check for world file.                                           */
/* -------------------------------------------------------------------- */
    if( !bIsSubfile )
    {
        int bEndsWithWld = strlen(poOpenInfo->pszFilename) > 4 &&
                           EQUAL( poOpenInfo->pszFilename + strlen(poOpenInfo->pszFilename) - 4, ".wld");
        poDS->bGeoTransformValid = 
            GDALReadWorldFile( poOpenInfo->pszFilename, NULL, 
                               poDS->adfGeoTransform )
            || GDALReadWorldFile( poOpenInfo->pszFilename, ".jpw", 
                                  poDS->adfGeoTransform )
            || ( !bEndsWithWld && GDALReadWorldFile( poOpenInfo->pszFilename, ".wld",
                                  poDS->adfGeoTransform ));

        if( !poDS->bGeoTransformValid )
        {
            int bTabFileOK =
                GDALReadTabFile( poOpenInfo->pszFilename, poDS->adfGeoTransform,
                                 &poDS->pszProjection,
                                 &poDS->nGCPCount, &poDS->pasGCPList );
            
            if( bTabFileOK && poDS->nGCPCount == 0 )
                poDS->bGeoTransformValid = TRUE;
        }
    }

    return poDS;
}

/************************************************************************/
/*                            CheckForMask()                            */
/************************************************************************/

void JPGDataset::CheckForMask()

{
    GIntBig nFileSize;
    GUInt32 nImageSize;

    /* Save current position to avoid disturbing JPEG stream decoding */
    vsi_l_offset nCurOffset = VSIFTellL(fpImage);

/* -------------------------------------------------------------------- */
/*      Go to the end of the file, pull off four bytes, and see if      */
/*      it is plausibly the size of the real image data.                */
/* -------------------------------------------------------------------- */
    VSIFSeekL( fpImage, 0, SEEK_END );
    nFileSize = VSIFTellL( fpImage );
    VSIFSeekL( fpImage, nFileSize - 4, SEEK_SET );
    
    VSIFReadL( &nImageSize, 4, 1, fpImage );
    CPL_LSBPTR32( &nImageSize );

    if( nImageSize < nFileSize / 2 || nImageSize > nFileSize - 4 )
        goto end;

/* -------------------------------------------------------------------- */
/*      If that seems ok, seek back, and verify that just preceeding    */
/*      the bitmask is an apparent end-of-jpeg-data marker.             */
/* -------------------------------------------------------------------- */
    GByte abyEOD[2];

    VSIFSeekL( fpImage, nImageSize - 2, SEEK_SET );
    VSIFReadL( abyEOD, 2, 1, fpImage );
    if( abyEOD[0] != 0xff || abyEOD[1] != 0xd9 )
        goto end;

/* -------------------------------------------------------------------- */
/*      We seem to have a mask.  Read it in.                            */
/* -------------------------------------------------------------------- */
    nCMaskSize = (int) (nFileSize - nImageSize - 4);
    pabyCMask = (GByte *) VSIMalloc(nCMaskSize);
    if (pabyCMask == NULL)
    {
        CPLError(CE_Failure, CPLE_OutOfMemory,
                 "Cannot allocate memory (%d bytes) for mask compressed buffer",
                 nCMaskSize);
        goto end;
    }
    VSIFReadL( pabyCMask, nCMaskSize, 1, fpImage );

    CPLDebug( "JPEG", "Got %d Byte compressed bitmask.",
              nCMaskSize );

end:
    VSIFSeekL( fpImage, nCurOffset, SEEK_SET );
}

/************************************************************************/
/*                           DecompressMask()                           */
/************************************************************************/

void JPGDataset::DecompressMask()

{
    if( pabyCMask == NULL || pabyBitMask != NULL )
        return;

/* -------------------------------------------------------------------- */
/*      Allocate 1bit buffer - may be slightly larger than needed.      */
/* -------------------------------------------------------------------- */
    int nBufSize = nRasterYSize * ((nRasterXSize+7)/8);
    pabyBitMask = (GByte *) VSIMalloc( nBufSize );
    if (pabyBitMask == NULL)
    {
        CPLError(CE_Failure, CPLE_OutOfMemory,
                 "Cannot allocate memory (%d bytes) for mask uncompressed buffer",
                 nBufSize);
        CPLFree(pabyCMask);
        pabyCMask = NULL;
        return;
    }
    
/* -------------------------------------------------------------------- */
/*      Decompress                                                      */
/* -------------------------------------------------------------------- */
    z_stream sStream;

    memset( &sStream, 0, sizeof(z_stream) );
    
    inflateInit( &sStream );
    
    sStream.next_in = pabyCMask;
    sStream.avail_in = nCMaskSize;

    sStream.next_out = pabyBitMask;
    sStream.avail_out = nBufSize;

    int nResult = inflate( &sStream, Z_FINISH );

    inflateEnd( &sStream );

/* -------------------------------------------------------------------- */
/*      Cleanup if an error occurs.                                     */
/* -------------------------------------------------------------------- */
    if( nResult != Z_STREAM_END )
    {
        
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Failure decoding JPEG validity bitmask." );
        CPLFree( pabyCMask );
        pabyCMask = NULL;

        CPLFree( pabyBitMask );
        pabyBitMask = NULL;
    }
}

/************************************************************************/
/*                             ErrorExit()                              */
/************************************************************************/

void JPGDataset::ErrorExit(j_common_ptr cinfo)
{
    jmp_buf *setjmp_buffer = (jmp_buf *) cinfo->client_data;
    char buffer[JMSG_LENGTH_MAX];

    /* Create the message */
    (*cinfo->err->format_message) (cinfo, buffer);

/* Avoid error for a 12bit JPEG if reading from the 8bit JPEG driver and */
/* we have JPEG_DUAL_MODE_8_12 support, as we'll try again with 12bit JPEG */
/* driver */
#if defined(JPEG_DUAL_MODE_8_12) && !defined(JPGDataset)
    if (strstr(buffer, "Unsupported JPEG data precision 12") == NULL)
#endif
    CPLError( CE_Failure, CPLE_AppDefined,
              "libjpeg: %s", buffer );

    /* Return control to the setjmp point */
    longjmp(*setjmp_buffer, 1);
}

/************************************************************************/
/*                           JPGAppendMask()                            */
/*                                                                      */
/*      This function appends a zlib compressed bitmask to a JPEG       */
/*      file (or really any file) pulled from an existing mask band.    */
/************************************************************************/

static void JPGAppendMask( const char *pszJPGFilename, GDALRasterBand *poMask )

{
    int nXSize = poMask->GetXSize();
    int nYSize = poMask->GetYSize();
    int nBitBufSize = nYSize * ((nXSize+7)/8);
    int iX, iY;
    GByte *pabyBitBuf, *pabyMaskLine;
    CPLErr eErr = CE_None;

/* -------------------------------------------------------------------- */
/*      Allocate uncompressed bit buffer.                               */
/* -------------------------------------------------------------------- */
    pabyBitBuf = (GByte *) VSICalloc(1,nBitBufSize);

    pabyMaskLine = (GByte *) VSIMalloc(nXSize);
    if (pabyBitBuf == NULL || pabyMaskLine == NULL)
    {
        CPLError( CE_Failure, CPLE_OutOfMemory, "Out of memory");
        eErr = CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Set bit buffer from mask band, scanline by scanline.            */
/* -------------------------------------------------------------------- */
    int iBit = 0;
    for( iY = 0; eErr == CE_None && iY < nYSize; iY++ )
    {
        eErr = poMask->RasterIO( GF_Read, 0, iY, nXSize, 1,
                                 pabyMaskLine, nXSize, 1, GDT_Byte, 0, 0 );
        if( eErr != CE_None )
            break;

        for( iX = 0; iX < nXSize; iX++ )
        {
            if( pabyMaskLine[iX] != 0 )
                pabyBitBuf[iBit>>3] |= (0x1 << (iBit&7));

            iBit++;
        }
    }
    
    CPLFree( pabyMaskLine );

/* -------------------------------------------------------------------- */
/*      Compress                                                        */
/* -------------------------------------------------------------------- */
    GByte *pabyCMask = NULL;
    z_stream sStream;

    if( eErr == CE_None )
    {
        pabyCMask = (GByte *) VSIMalloc(nBitBufSize + 30);
        if (pabyCMask == NULL)
        {
            CPLError( CE_Failure, CPLE_OutOfMemory, "Out of memory");
            eErr = CE_Failure;
        }
    }

    if ( eErr == CE_None )
    {
        memset( &sStream, 0, sizeof(z_stream) );
        
        deflateInit( &sStream, 9 );
        
        sStream.next_in = pabyBitBuf;
        sStream.avail_in = nBitBufSize;
        
        sStream.next_out = pabyCMask;
        sStream.avail_out = nBitBufSize + 30;
        
        int nResult = deflate( &sStream, Z_FINISH );
        
        deflateEnd( &sStream );

        if( nResult != Z_STREAM_END )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "Deflate compression of jpeg bit mask failed." );
            eErr = CE_Failure;
        }
    }

/* -------------------------------------------------------------------- */
/*      Write to disk, along with image file size.                      */
/* -------------------------------------------------------------------- */
    if( eErr == CE_None )
    {
        VSILFILE *fpOut;
        GUInt32 nImageSize;

        fpOut = VSIFOpenL( pszJPGFilename, "r+" );
        if( fpOut == NULL )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "Failed to open jpeg to append bitmask." );
            eErr = CE_Failure;
        }
        else
        {
            VSIFSeekL( fpOut, 0, SEEK_END );

            nImageSize = VSIFTellL( fpOut );
            CPL_LSBPTR32( &nImageSize );

            if( VSIFWriteL( pabyCMask, 1, sStream.total_out, fpOut ) 
                != sStream.total_out )
            {
                CPLError( CE_Failure, CPLE_FileIO,
                          "Failure writing compressed bitmask.\n%s",
                          VSIStrerror( errno ) );
                eErr = CE_Failure;
            }
            else
                VSIFWriteL( &nImageSize, 4, 1, fpOut );

            VSIFCloseL( fpOut );
        }
    }

    CPLFree( pabyBitBuf );
    CPLFree( pabyCMask );
}

/************************************************************************/
/*                           JPEGCreateCopy()                           */
/************************************************************************/

GDALDataset *
JPEGCreateCopy( const char * pszFilename, GDALDataset *poSrcDS, 
                int bStrict, char ** papszOptions, 
                GDALProgressFunc pfnProgress, void * pProgressData )

{
    int  nBands = poSrcDS->GetRasterCount();
    int  nXSize = poSrcDS->GetRasterXSize();
    int  nYSize = poSrcDS->GetRasterYSize();
    int  nQuality = 75;
    int  bProgressive = FALSE;
    int  nCloneFlags = GCIF_PAM_DEFAULT;

    if( !pfnProgress( 0.0, NULL, pProgressData ) )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Some some rudimentary checks                                    */
/* -------------------------------------------------------------------- */
    if( nBands != 1 && nBands != 3 && nBands != 4 )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "JPEG driver doesn't support %d bands.  Must be 1 (grey), "
                  "3 (RGB) or 4 bands.\n", nBands );

        return NULL;
    }

    if (nBands == 1 &&
        poSrcDS->GetRasterBand(1)->GetColorTable() != NULL)
    {
        CPLError( (bStrict) ? CE_Failure : CE_Warning, CPLE_NotSupported, 
                  "JPEG driver ignores color table. "
                  "The source raster band will be considered as grey level.\n"
                  "Consider using color table expansion (-expand option in gdal_translate)\n");
        if (bStrict)
            return NULL;
    }

    GDALDataType eDT = poSrcDS->GetRasterBand(1)->GetRasterDataType();

#if defined(JPEG_LIB_MK1_OR_12BIT) || defined(JPEG_DUAL_MODE_8_12)
    if( eDT != GDT_Byte && eDT != GDT_UInt16 )
    {
        CPLError( (bStrict) ? CE_Failure : CE_Warning, CPLE_NotSupported, 
                  "JPEG driver doesn't support data type %s. "
                  "Only eight and twelve bit bands supported (Mk1 libjpeg).\n",
                  GDALGetDataTypeName( 
                      poSrcDS->GetRasterBand(1)->GetRasterDataType()) );

        if (bStrict)
            return NULL;
    }

    if( eDT == GDT_UInt16 || eDT == GDT_Int16 )
    {
#if defined(JPEG_DUAL_MODE_8_12) && !defined(JPGDataset)
        return JPEGCreateCopy12(pszFilename, poSrcDS,
                                bStrict, papszOptions, 
                                pfnProgress, pProgressData );
#else
        eDT = GDT_UInt16;
#endif
    }
    else
        eDT = GDT_Byte;

#else
    if( eDT != GDT_Byte )
    {
        CPLError( (bStrict) ? CE_Failure : CE_Warning, CPLE_NotSupported, 
                  "JPEG driver doesn't support data type %s. "
                  "Only eight bit Byte bands supported.\n", 
                  GDALGetDataTypeName( 
                      poSrcDS->GetRasterBand(1)->GetRasterDataType()) );

        if (bStrict)
            return NULL;
    }
    
    eDT = GDT_Byte; // force to 8bit. 
#endif

/* -------------------------------------------------------------------- */
/*      What options has the user selected?                             */
/* -------------------------------------------------------------------- */
    if( CSLFetchNameValue(papszOptions,"QUALITY") != NULL )
    {
        nQuality = atoi(CSLFetchNameValue(papszOptions,"QUALITY"));
        if( nQuality < 10 || nQuality > 100 )
        {
            CPLError( CE_Failure, CPLE_IllegalArg,
                      "QUALITY=%s is not a legal value in the range 10-100.",
                      CSLFetchNameValue(papszOptions,"QUALITY") );
            return NULL;
        }
    }

    bProgressive = CSLFetchBoolean( papszOptions, "PROGRESSIVE", FALSE );

/* -------------------------------------------------------------------- */
/*      Create the dataset.                                             */
/* -------------------------------------------------------------------- */
    VSILFILE	*fpImage;

    fpImage = VSIFOpenL( pszFilename, "wb" );
    if( fpImage == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed, 
                  "Unable to create jpeg file %s.\n", 
                  pszFilename );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Initialize JPG access to the file.                              */
/* -------------------------------------------------------------------- */
    struct jpeg_compress_struct sCInfo;
    struct jpeg_error_mgr sJErr;
    jmp_buf setjmp_buffer;
    
    if (setjmp(setjmp_buffer)) 
        return NULL;

    sCInfo.err = jpeg_std_error( &sJErr );
    sJErr.error_exit = JPGDataset::ErrorExit;
    sCInfo.client_data = (void *) &(setjmp_buffer);

    jpeg_create_compress( &sCInfo );
    
    jpeg_vsiio_dest( &sCInfo, fpImage );
    
    sCInfo.image_width = nXSize;
    sCInfo.image_height = nYSize;
    sCInfo.input_components = nBands;

    if( nBands == 3 )
        sCInfo.in_color_space = JCS_RGB;
    else if( nBands == 1 )
        sCInfo.in_color_space = JCS_GRAYSCALE;
    else
        sCInfo.in_color_space = JCS_UNKNOWN;

    jpeg_set_defaults( &sCInfo );
    
    if( eDT == GDT_UInt16 )
    {
        sCInfo.data_precision = 12;
    }
    else
    {
        sCInfo.data_precision = 8;
    }

    GDALDataType eWorkDT;
#ifdef JPEG_LIB_MK1
    sCInfo.bits_in_jsample = sCInfo.data_precision;
    eWorkDT = GDT_UInt16; /* Always force to 16 bit for JPEG_LIB_MK1 */
#else
    eWorkDT = eDT;
#endif

    jpeg_set_quality( &sCInfo, nQuality, TRUE );

    if( bProgressive )
        jpeg_simple_progression( &sCInfo );

    jpeg_start_compress( &sCInfo, TRUE );

/* -------------------------------------------------------------------- */
/*      Loop over image, copying image data.                            */
/* -------------------------------------------------------------------- */
    GByte 	*pabyScanline;
    CPLErr      eErr = CE_None;
    int         nWorkDTSize = GDALGetDataTypeSize(eWorkDT) / 8;
    bool        bClipWarn = false;

    pabyScanline = (GByte *) CPLMalloc( nBands * nXSize * nWorkDTSize );

    for( int iLine = 0; iLine < nYSize && eErr == CE_None; iLine++ )
    {
        JSAMPLE      *ppSamples;

        eErr = poSrcDS->RasterIO( GF_Read, 0, iLine, nXSize, 1, 
                                  pabyScanline, nXSize, 1, eWorkDT,
                                  nBands, NULL,
                                  nBands*nWorkDTSize, 
                                  nBands * nXSize * nWorkDTSize, nWorkDTSize );

        // clamp 16bit values to 12bit.
        if( nWorkDTSize == 2 )
        {
            GUInt16 *panScanline = (GUInt16 *) pabyScanline;
            int iPixel;

            for( iPixel = 0; iPixel < nXSize*nBands; iPixel++ )
            {
                if( panScanline[iPixel] > 4095 )
                {
                    panScanline[iPixel] = 4095;
                    if( !bClipWarn )
                    {
                        bClipWarn = true;
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "One or more pixels clipped to fit 12bit domain for jpeg output." );
                    }
                }
            }
        }

        ppSamples = (JSAMPLE *) pabyScanline;

        if( eErr == CE_None )
            jpeg_write_scanlines( &sCInfo, &ppSamples, 1 );

        if( eErr == CE_None 
            && !pfnProgress( (iLine+1) / (double) nYSize,
                             NULL, pProgressData ) )
        {
            eErr = CE_Failure;
            CPLError( CE_Failure, CPLE_UserInterrupt, 
                      "User terminated CreateCopy()" );
        }
    }

/* -------------------------------------------------------------------- */
/*      Cleanup and close.                                              */
/* -------------------------------------------------------------------- */
    CPLFree( pabyScanline );

    if( eErr == CE_None )
        jpeg_finish_compress( &sCInfo );
    jpeg_destroy_compress( &sCInfo );

    VSIFCloseL( fpImage );

    if( eErr != CE_None )
    {
        VSIUnlink( pszFilename );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Does the source have a mask?  If so, append it to the jpeg file.*/
/* -------------------------------------------------------------------- */
    int nMaskFlags = poSrcDS->GetRasterBand(1)->GetMaskFlags();
    if( !(nMaskFlags & GMF_ALL_VALID) 
        && (nBands == 1 || (nMaskFlags & GMF_PER_DATASET)) )
    {
        CPLDebug( "JPEG", "Appending Mask Bitmap" ); 
        JPGAppendMask( pszFilename, poSrcDS->GetRasterBand(1)->GetMaskBand() );
        nCloneFlags &= (~GCIF_MASK);
    }

/* -------------------------------------------------------------------- */
/*      Do we need a world file?                                        */
/* -------------------------------------------------------------------- */
    if( CSLFetchBoolean( papszOptions, "WORLDFILE", FALSE ) )
    {
	double      adfGeoTransform[6];
	
	poSrcDS->GetGeoTransform( adfGeoTransform );
	GDALWriteWorldFile( pszFilename, "wld", adfGeoTransform );
    }

/* -------------------------------------------------------------------- */
/*      Re-open dataset, and copy any auxilary pam information.         */
/* -------------------------------------------------------------------- */
    JPGDataset *poDS = (JPGDataset *) GDALOpen( pszFilename, GA_ReadOnly );

    if( poDS )
        poDS->CloneInfo( poSrcDS, nCloneFlags );

    return poDS;
}

/************************************************************************/
/*                         GDALRegister_JPEG()                          */
/************************************************************************/

#if !defined(JPGDataset)
void GDALRegister_JPEG()

{
    GDALDriver	*poDriver;

    if( GDALGetDriverByName( "JPEG" ) == NULL )
    {
        poDriver = new GDALDriver();
        
        poDriver->SetDescription( "JPEG" );
        poDriver->SetMetadataItem( GDAL_DMD_LONGNAME, 
                                   "JPEG JFIF" );
        poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC, 
                                   "frmt_jpeg.html" );
        poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "jpg" );
        poDriver->SetMetadataItem( GDAL_DMD_MIMETYPE, "image/jpeg" );

#if defined(JPEG_LIB_MK1_OR_12BIT) || defined(JPEG_DUAL_MODE_8_12)
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONDATATYPES, 
                                   "Byte uint16_t" );
#else
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONDATATYPES, 
                                   "Byte" );
#endif
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONOPTIONLIST, 
"<CreationOptionList>\n"
"   <Option name='PROGRESSIVE' type='boolean'/>\n"
"   <Option name='QUALITY' type='int' description='good=100, bad=0, default=75'/>\n"
"   <Option name='WORLDFILE' type='boolean'/>\n"
"</CreationOptionList>\n" );

        poDriver->SetMetadataItem( GDAL_DCAP_VIRTUALIO, "YES" );

        poDriver->pfnIdentify = JPGDataset::Identify;
        poDriver->pfnOpen = JPGDataset::Open;
        poDriver->pfnCreateCopy = JPEGCreateCopy;

        GetGDALDriverManager()->RegisterDriver( poDriver );
    }
}
#endif
