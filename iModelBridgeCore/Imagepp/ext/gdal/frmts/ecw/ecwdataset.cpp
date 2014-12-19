/******************************************************************************
 * $Id: ecwdataset.cpp 22589 2011-06-25 21:02:42Z rouault $
 *
 * Project:  GDAL 
 * Purpose:  ECW (ERDAS Wavelet Compression Format) Driver
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam <warmerdam@pobox.com>
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
#include "gdaljp2metadata.h"
#include "ogr_spatialref.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "vsiiostream.h"
#include "cpl_multiproc.h"
#include "cpl_minixml.h"
#include "ogr_api.h"
#include "ogr_geometry.h"

CPL_CVSID("$Id: ecwdataset.cpp 22589 2011-06-25 21:02:42Z rouault $");

#undef NOISY_DEBUG

#ifdef FRMT_ecw

static const unsigned char jpc_header[] = {0xff,0x4f};
static const unsigned char jp2_header[] = 
    {0x00,0x00,0x00,0x0c,0x6a,0x50,0x20,0x20,0x0d,0x0a,0x87,0x0a};

static void *hECWDatasetMutex = NULL;
static int    bNCSInitialized = FALSE;

void ECWInitialize( void );

GDALDataset* ECWDatasetOpenJPEG2000(GDALOpenInfo* poOpenInfo);

/************************************************************************/
/* ==================================================================== */
/*				ECWDataset				*/
/* ==================================================================== */
/************************************************************************/

class ECWRasterBand;

class CPL_DLL ECWDataset : public GDALPamDataset
{
    friend class ECWRasterBand;

    CNCSJP2FileView *poFileView;
    NCSFileViewFileInfoEx *psFileInfo;

    GDALDataType eRasterDataType;
    NCSEcwCellType eNCSRequestDataType;

    int         bUsingCustomStream;

    // Current view window. 
    int         bWinActive;
    int         nWinXOff, nWinYOff, nWinXSize, nWinYSize;
    int         nWinBufXSize, nWinBufYSize;
    int         nWinBandCount;
    int         *panWinBandList;
    int         nWinBufLoaded;
    void        **papCurLineBuf;

    int         bGeoTransformValid;
    double      adfGeoTransform[6];
    char        *pszProjection;
    int         nGCPCount;
    GDAL_GCP    *pasGCPList;

    char        **papszGMLMetadata;

    void        ECW2WKTProjection();

    void        CleanupWindow();
    int         TryWinRasterIO( GDALRWFlag, int, int, int, int,
                                GByte *, int, int, GDALDataType,
                                int, int *, int, int, int );
    CPLErr      LoadNextLine();

  public:
		ECWDataset(int bIsJPEG2000);
		~ECWDataset();
                
    static GDALDataset *Open( GDALOpenInfo *, int bIsJPEG2000 );
    static int          IdentifyJPEG2000( GDALOpenInfo * poOpenInfo );
    static GDALDataset *OpenJPEG2000( GDALOpenInfo * );
    static int          IdentifyECW( GDALOpenInfo * poOpenInfo );
    static GDALDataset *OpenECW( GDALOpenInfo * );

    virtual CPLErr IRasterIO( GDALRWFlag, int, int, int, int,
                              void *, int, int, GDALDataType,
                              int, int *, int, int, int );

    virtual CPLErr GetGeoTransform( double * );
    virtual const char *GetProjectionRef();

    virtual int    GetGCPCount();
    virtual const char *GetGCPProjection();
    virtual const GDAL_GCP *GetGCPs();

    virtual char      **GetMetadata( const char * pszDomain = "" );

    virtual CPLErr AdviseRead( int nXOff, int nYOff, int nXSize, int nYSize,
                               int nBufXSize, int nBufYSize, 
                               GDALDataType eDT, 
                               int nBandCount, int *panBandList,
                               char **papszOptions );
};

/************************************************************************/
/* ==================================================================== */
/*                            ECWRasterBand                             */
/* ==================================================================== */
/************************************************************************/

class ECWRasterBand : public GDALPamRasterBand
{
    friend class ECWDataset;
    
    // NOTE: poDS may be altered for NITF/JPEG2000 files!
    ECWDataset     *poGDS;

    GDALColorInterp         eBandInterp;

    //IPP
    GDALColorTable         *pColorTable;

    int                          iOverview; // -1 for base. 

    std::vector<ECWRasterBand*>  apoOverviews;

    virtual CPLErr IRasterIO( GDALRWFlag, int, int, int, int,
                              void *, int, int, GDALDataType,
                              int, int );

  public:

                   ECWRasterBand( ECWDataset *, int, int = -1 );
                   ~ECWRasterBand();

    virtual CPLErr IReadBlock( int, int, void * );
    virtual int    HasArbitraryOverviews() { return apoOverviews.size() == 0; }
    virtual int    GetOverviewCount() { return apoOverviews.size(); }
    virtual GDALRasterBand *GetOverview(int);

    virtual GDALColorInterp GetColorInterpretation();
    virtual CPLErr SetColorInterpretation( GDALColorInterp );

    //IPP
    virtual GDALColorTable *GetColorTable();
    virtual CPLErr SetColorTable( GDALColorTable * );

    virtual CPLErr AdviseRead( int nXOff, int nYOff, int nXSize, int nYSize,
                               int nBufXSize, int nBufYSize, 
                               GDALDataType eDT, char **papszOptions );
};

/************************************************************************/
/*                           ECWRasterBand()                            */
/************************************************************************/

ECWRasterBand::ECWRasterBand( ECWDataset *poDS, int nBand, int iOverview )

{
    this->poDS = poDS;
    poGDS = poDS;

    this->iOverview = iOverview;
    this->nBand = nBand;
    eDataType = poDS->eRasterDataType;
    
    nRasterXSize = poDS->GetRasterXSize() / ( 1 << (iOverview+1));
    nRasterYSize = poDS->GetRasterYSize() / ( 1 << (iOverview+1));

    //IPP - ERMapper seems to work much faster when accessing the image data by
    //      tile compared to by line.
    nBlockXSize = 256;
    nBlockYSize = 256;

    //IPP
    pColorTable = 0;

/* -------------------------------------------------------------------- */
/*      Work out band color interpretation.                             */
/* -------------------------------------------------------------------- */
    if( poDS->psFileInfo->eColorSpace == NCSCS_NONE )
        eBandInterp = GCI_Undefined;
    else if( poDS->psFileInfo->eColorSpace == NCSCS_GREYSCALE )
        eBandInterp = GCI_GrayIndex;
    else if( poDS->psFileInfo->eColorSpace == NCSCS_MULTIBAND )
        eBandInterp = GCI_Undefined;
    else if( poDS->psFileInfo->eColorSpace == NCSCS_sRGB )
    {
        if( nBand == 1 )
            eBandInterp = GCI_RedBand;
        else if( nBand == 2 )
            eBandInterp = GCI_GreenBand;
        else if( nBand == 3 )
            eBandInterp = GCI_BlueBand;
        else if( nBand == 4 )
            eBandInterp = GCI_AlphaBand;
        else
            eBandInterp = GCI_Undefined;
    }
    else if( poDS->psFileInfo->eColorSpace == NCSCS_YCbCr )
    {
        if( CSLTestBoolean( CPLGetConfigOption("CONVERT_YCBCR_TO_RGB","YES") ))
        {
            if( nBand == 1 )
                eBandInterp = GCI_RedBand;
            else if( nBand == 2 )
                eBandInterp = GCI_GreenBand;
            else if( nBand == 3 )
                eBandInterp = GCI_BlueBand;
            else
                eBandInterp = GCI_Undefined;
        }
        else
        {
            if( nBand == 1 )
                eBandInterp = GCI_YCbCr_YBand;
            else if( nBand == 2 )
                eBandInterp = GCI_YCbCr_CbBand;
            else if( nBand == 3 )
                eBandInterp = GCI_YCbCr_CrBand;
            else
                eBandInterp = GCI_Undefined;
        }
    }
    else
        eBandInterp = GCI_Undefined;

/* -------------------------------------------------------------------- */
/*      If this is the base level, create a set of overviews.           */
/* -------------------------------------------------------------------- */
    if( iOverview == -1 )
    {
        int i;
        for( i = 0; 
             nRasterXSize / (1 << (i+1)) > 128 
                 && nRasterYSize / (1 << (i+1)) > 128;
             i++ )
        {
            apoOverviews.push_back( new ECWRasterBand( poDS, nBand, i ) );
        }
    }
}

/************************************************************************/
/*                          ~ECWRasterBand()                           */
/************************************************************************/

ECWRasterBand::~ECWRasterBand()

{
    //IPP
    if (pColorTable != 0)
    {
        delete pColorTable;
        pColorTable = 0;
    }

    FlushCache();

    while( apoOverviews.size() > 0 )
    {
        delete apoOverviews.back();
        apoOverviews.pop_back();
    }
}

/************************************************************************/
/*                            GetOverview()                             */
/************************************************************************/

GDALRasterBand *ECWRasterBand::GetOverview( int iOverview )

{
    if( iOverview >= 0 && iOverview < (int) apoOverviews.size() )
        return apoOverviews[iOverview];
    else
        return NULL;
}

/************************************************************************/
/*                       GetColorInterpretation()                       */
/************************************************************************/

GDALColorInterp ECWRasterBand::GetColorInterpretation()

{
    return eBandInterp;
}

/************************************************************************/
/*                       SetColorInterpretation()                       */
/*                                                                      */
/*      This would normally just be used by folks using the ECW code    */
/*      to read JP2 streams in other formats (such as NITF) and         */
/*      providing their own color interpretation regardless of what     */
/*      ECW might think the stream itself says.                         */
/************************************************************************/

CPLErr ECWRasterBand::SetColorInterpretation( GDALColorInterp eNewInterp )

{
    eBandInterp = eNewInterp;

    return CE_None;
}

//IPP :
/************************************************************************/
/*                       GetColorTable()                                */
/*                                                                      */
/*      This would normally just be used by folks using the ECW code    */
/*      to read JP2 streams in other formats (such as NITF) and         */
/*      providing their own color interpretation regardless of what     */
/*      ECW might think the stream itself says.                         */
/************************************************************************/

GDALColorTable *ECWRasterBand::GetColorTable()
{
    return pColorTable;
}


/************************************************************************/
/*                       SetColorTable()                                */
/*                                                                      */
/*      This would normally just be used by folks using the ECW code    */
/*      to read JP2 streams in other formats (such as NITF) and         */
/*      providing their own color interpretation regardless of what     */
/*      ECW might think the stream itself says.                         */
/************************************************************************/

CPLErr ECWRasterBand::SetColorTable( GDALColorTable * pi_pColorTable)
{
    if (pColorTable != 0)
        delete pColorTable;

    pColorTable = pi_pColorTable;

    return CE_None;
}

//IPP - End of modifications

/************************************************************************/
/*                             AdviseRead()                             */
/************************************************************************/

CPLErr ECWRasterBand::AdviseRead( int nXOff, int nYOff, int nXSize, int nYSize,
                                  int nBufXSize, int nBufYSize, 
                                  GDALDataType eDT, 
                                  char **papszOptions )
{
    int nResFactor = 1 << (iOverview+1);
    
    return poGDS->AdviseRead( nXOff * nResFactor, 
                              nYOff * nResFactor, 
                              nXSize * nResFactor, 
                              nYSize * nResFactor, 
                              nBufXSize, nBufYSize, eDT, 
                              1, &nBand, papszOptions );
}

/************************************************************************/
/*                             IRasterIO()                              */
/************************************************************************/

CPLErr ECWRasterBand::IRasterIO( GDALRWFlag eRWFlag,
                                 int nXOff, int nYOff, int nXSize, int nYSize,
                                 void * pData, int nBufXSize, int nBufYSize,
                                 GDALDataType eBufType,
                                 int nPixelSpace, int nLineSpace )
    
{
    int          iBand, bDirect;
    GByte        *pabyWorkBuffer = NULL;
    int nResFactor = 1 << (iOverview+1);

/* -------------------------------------------------------------------- */
/*      Try to do it based on existing "advised" access.                */
/* -------------------------------------------------------------------- */
    if( poGDS->TryWinRasterIO( eRWFlag, 
                               nXOff * nResFactor, nYOff * nResFactor, 
                               nXSize * nResFactor, nYSize * nResFactor, 
                               (GByte *) pData, nBufXSize, nBufYSize, 
                               eBufType, 1, &nBand, 
                               nPixelSpace, nLineSpace, 0 ) )
        return CE_None;

/* -------------------------------------------------------------------- */
/*      We will drop down to the block oriented API if only a single    */
/*      scanline was requested. This is based on the assumption that    */
/*      doing lots of single scanline windows is expensive.             */
/* -------------------------------------------------------------------- */
    if( nYSize == 1 )
    {
#ifdef NOISY_DEBUG
        CPLDebug( "ECWRasterBand", 
                  "RasterIO(%d,%d,%d,%d -> %dx%d) - redirected.", 
                  nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );
#endif
        return GDALRasterBand::IRasterIO(eRWFlag, nXOff, nYOff, nXSize, nYSize,
                                         pData, nBufXSize, nBufYSize, 
                                         eBufType, nPixelSpace, nLineSpace );
    }

/* -------------------------------------------------------------------- */
/*      The ECW SDK doesn't supersample, so adjust for this case.       */
/* -------------------------------------------------------------------- */
    CPLDebug( "ECWRasterBand", 
              "RasterIO(nXOff=%d,nYOff=%d,nXSize=%d,nYSize=%d -> %dx%d)", 
              nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );

    nXOff *= nResFactor;
    nYOff *= nResFactor;
    nXSize *= nResFactor;
    nYSize *= nResFactor;

    int          nNewXSize = nBufXSize, nNewYSize = nBufYSize;

    if ( nXSize < nBufXSize )
        nNewXSize = nXSize;

    if ( nYSize < nBufYSize )
        nNewYSize = nYSize;

/* -------------------------------------------------------------------- */
/*      Default line and pixel spacing if needed.                       */
/* -------------------------------------------------------------------- */
    if ( nPixelSpace == 0 )
        nPixelSpace = GDALGetDataTypeSize( eBufType ) / 8;

    if ( nLineSpace == 0 )
        nLineSpace = nPixelSpace * nBufXSize;

/* -------------------------------------------------------------------- */
/*      Can we perform direct loads, or must we load into a working     */
/*      buffer, and transform?                                          */
/* -------------------------------------------------------------------- */
    int	    nRawPixelSize = GDALGetDataTypeSize(poGDS->eRasterDataType) / 8;

    bDirect = nPixelSpace == 1 && eBufType == GDT_Byte
	    && nNewXSize == nBufXSize && nNewYSize == nBufYSize;
    if( !bDirect )
        pabyWorkBuffer = (GByte *) CPLMalloc(nNewXSize * nRawPixelSize);

/* -------------------------------------------------------------------- */
/*      Establish access at the desired resolution.                     */
/* -------------------------------------------------------------------- */
    CNCSError oErr;

    poGDS->CleanupWindow();

    iBand = nBand-1;
    oErr = poGDS->poFileView->SetView( 1, (unsigned int *) (&iBand),
                                       nXOff, nYOff, 
                                       nXOff + nXSize - 1, 
                                       nYOff + nYSize - 1,
                                       nNewXSize, nNewYSize );
    if( oErr.GetErrorNumber() != NCS_SUCCESS )
    {
        CPLFree( pabyWorkBuffer );
        char* pszErrorMessage = oErr.GetErrorMessage();
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "%s", pszErrorMessage );
        NCSFree(pszErrorMessage);
        
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Read back one scanline at a time, till request is satisfied.    */
/*      Supersampling is not supported by the ECW API, so we will do    */
/*      it ourselves.                                                   */
/* -------------------------------------------------------------------- */
    double	dfSrcYInc = (double)nNewYSize / nBufYSize;
    double	dfSrcXInc = (double)nNewXSize / nBufXSize;
    int	        iSrcLine, iDstLine;

    for( iSrcLine = 0, iDstLine = 0; iDstLine < nBufYSize; iDstLine++ )
    {
        NCSEcwReadStatus eRStatus;
        int	        iDstLineOff = iDstLine * nLineSpace;
        unsigned char	*pabySrcBuf;

        if( bDirect )
            pabySrcBuf = ((GByte *)pData) + iDstLineOff;
        else
            pabySrcBuf = pabyWorkBuffer;

	if ( nNewYSize == nBufYSize || iSrcLine == (int)(iDstLine * dfSrcYInc) )
	{
            eRStatus = poGDS->poFileView->ReadLineBIL( 
                poGDS->eNCSRequestDataType, 1, (void **) &pabySrcBuf );

	    if( eRStatus != NCSECW_READ_OK )
	    {
	        CPLFree( pabyWorkBuffer );
                CPLDebug( "ECW", "ReadLineBIL status=%d", (int) eRStatus );
	        CPLError( CE_Failure, CPLE_AppDefined,
			  "NCScbmReadViewLineBIL failed." );
		return CE_Failure;
	    }

            if( !bDirect )
            {
                if ( nNewXSize == nBufXSize )
                {
                    GDALCopyWords( pabyWorkBuffer, poGDS->eRasterDataType, 
                                   nRawPixelSize, 
                                   ((GByte *)pData) + iDstLine * nLineSpace, 
                                   eBufType, nPixelSpace, nBufXSize );
                }
		else
		{
	            int	iPixel;

                    for ( iPixel = 0; iPixel < nBufXSize; iPixel++ )
                    {
                        GDALCopyWords( pabyWorkBuffer 
                                       + nRawPixelSize*((int)(iPixel*dfSrcXInc)),
                                       poGDS->eRasterDataType, nRawPixelSize,
                                       (GByte *)pData + iDstLineOff
				       + iPixel * nPixelSpace,
                                       eBufType, nPixelSpace, 1 );
                    }
		}
            }

            iSrcLine++;
	}
	else
	{
	    // Just copy the previous line in this case
            GDALCopyWords( (GByte *)pData + (iDstLineOff - nLineSpace),
                            eBufType, nPixelSpace,
                            (GByte *)pData + iDstLineOff,
                            eBufType, nPixelSpace, nBufXSize );
	}
    }

    CPLFree( pabyWorkBuffer );

    return CE_None;
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr ECWRasterBand::IReadBlock( int, int nBlockYOff, void * pImage )

{
    CPLErr eErr = CE_None;
    int nXOff = 0, nYOff = nBlockYOff, nXSize = nBlockXSize, nYSize = 1;
    int nResFactor = 1 << (iOverview+1);
    int nDSXOff, nDSYOff, nDSXSize, nDSYSize;

    nDSXOff = nXOff * nResFactor;
    nDSYOff = nYOff * nResFactor;
    nDSXSize = nXSize * nResFactor;
    nDSYSize = nYSize * nResFactor;

#ifdef NOISY_DEBUG
    CPLDebug( "ECW", 
              "ECWRasterBand::IReadBlock(0,%d) from overview %d, size %dx%d",
              nBlockYOff,
              iOverview, nRasterXSize, nRasterYSize );
#endif

    if( poGDS->TryWinRasterIO( GF_Read, nDSXOff, nDSYOff, nDSXSize, nDSYSize,
                               (GByte *) pImage, nBlockXSize, 1, 
                               eDataType, 1, &nBand, 0, 0, 0 ) )
        return CE_None;

    eErr = AdviseRead( 0, nYOff, 
                       nRasterXSize, nRasterYSize - nYOff,
                       nRasterXSize, nRasterYSize - nBlockYOff,
                       eDataType, NULL );
    if( eErr != CE_None )
        return eErr;

    if( poGDS->TryWinRasterIO( GF_Read, 
                               nDSXOff, nDSYOff, nDSXSize, nDSYSize,
                               (GByte *) pImage, nBlockXSize, 1, 
                               eDataType, 1, &nBand, 0, 0, 0 ) )
        return CE_None;

    CPLError( CE_Failure, CPLE_AppDefined, 
              "TryWinRasterIO() failed for blocked scanline %d of band %d.",
              nBlockYOff, nBand );

    return CE_Failure;
}

/************************************************************************/
/* ==================================================================== */
/*                            ECWDataset                               */
/* ==================================================================== */
/************************************************************************/


/************************************************************************/
/*                            ECWDataset()                              */
/************************************************************************/

ECWDataset::ECWDataset(int bIsJPEG2000)

{
    bUsingCustomStream = FALSE;
    pszProjection = NULL;
    poFileView = NULL;
    bWinActive = FALSE;
    panWinBandList = NULL;
    eRasterDataType = GDT_Byte;
    nGCPCount = 0;
    pasGCPList = NULL;
    papszGMLMetadata = NULL;
    
    bGeoTransformValid = FALSE;
    adfGeoTransform[0] = 0.0;
    adfGeoTransform[1] = 1.0;
    adfGeoTransform[2] = 0.0;
    adfGeoTransform[3] = 0.0;
    adfGeoTransform[4] = 0.0;
    adfGeoTransform[5] = 1.0;
    
    poDriver = (GDALDriver*) GDALGetDriverByName( bIsJPEG2000 ? "JP2ECW" : "ECW" );
}

/************************************************************************/
/*                           ~ECWDataset()                              */
/************************************************************************/

ECWDataset::~ECWDataset()

{
    FlushCache();
    CleanupWindow();
    CPLFree( pszProjection );
    CSLDestroy( papszGMLMetadata );

    if( nGCPCount > 0 )
    {
        GDALDeinitGCPs( nGCPCount, pasGCPList );
        CPLFree( pasGCPList );
    }

/* -------------------------------------------------------------------- */
/*      Release / dereference iostream.                                 */
/* -------------------------------------------------------------------- */
    // The underlying iostream of the CNCSJP2FileView (poFileView) object may 
    // also be the underlying iostream of other CNCSJP2FileView (poFileView) 
    // objects.  Consequently, when we delete the CNCSJP2FileView (poFileView) 
    // object, we must decrement the nFileViewCount attribute of the underlying
    // VSIIOStream object, and only delete the VSIIOStream object when 
    // nFileViewCount is equal to zero.

    CPLMutexHolder oHolder( &hECWDatasetMutex );

    if( poFileView != NULL )
    {
        VSIIOStream *poUnderlyingIOStream = (VSIIOStream *)NULL;

        poUnderlyingIOStream = ((VSIIOStream *)(poFileView->GetStream()));
        delete poFileView;

        if( bUsingCustomStream )
        {
            if( --poUnderlyingIOStream->nFileViewCount == 0 )
                delete poUnderlyingIOStream;
        }
    }
}

/************************************************************************/
/*                             AdviseRead()                             */
/************************************************************************/

CPLErr ECWDataset::AdviseRead( int nXOff, int nYOff, int nXSize, int nYSize,
                               int nBufXSize, int nBufYSize, 
                               GDALDataType eDT, 
                               int nBandCount, int *panBandList,
                               char **papszOptions )

{
    int *panAdjustedBandList = NULL;

    CPLDebug( "ECW",
              "ECWDataset::AdviseRead(%d,%d,%d,%d->%d,%d)",
              nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );

    if( nBufXSize > nXSize || nBufYSize > nYSize )
    {
        CPLError( CE_Warning, CPLE_AppDefined, 
                  "Supersampling not directly supported by ECW toolkit,\n"
                  "ignoring AdviseRead() request." );
        return CE_Warning; 
    }

/* -------------------------------------------------------------------- */
/*      Adjust band numbers to be zero based.                           */
/* -------------------------------------------------------------------- */
    panAdjustedBandList = (int *) 
        CPLMalloc(sizeof(int) * nBandCount );
    for( int ii= 0; ii < nBandCount; ii++ )
        panAdjustedBandList[ii] = panBandList[ii] - 1;

/* -------------------------------------------------------------------- */
/*      Cleanup old window cache information.                           */
/* -------------------------------------------------------------------- */
    CleanupWindow();

/* -------------------------------------------------------------------- */
/*      Set the new requested window.                                   */
/* -------------------------------------------------------------------- */
    CNCSError oErr;
    
    oErr = poFileView->SetView( nBandCount, (UINT32 *) panAdjustedBandList, 
                                nXOff, nYOff, 
                                nXOff + nXSize-1, nYOff + nYSize-1,
                                nBufXSize, nBufYSize );

    CPLFree( panAdjustedBandList );
    if( oErr.GetErrorNumber() != NCS_SUCCESS )
    {
        char* pszErrorMessage = oErr.GetErrorMessage();
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "%s", pszErrorMessage );
        NCSFree(pszErrorMessage);
        bWinActive = FALSE;
        return CE_Failure;
    }

    bWinActive = TRUE;

/* -------------------------------------------------------------------- */
/*      Record selected window.                                         */
/* -------------------------------------------------------------------- */
    nWinXOff = nXOff;
    nWinYOff = nYOff;
    nWinXSize = nXSize;
    nWinYSize = nYSize;
    nWinBufXSize = nBufXSize;
    nWinBufYSize = nBufYSize;

    panWinBandList = (int *) CPLMalloc(sizeof(int)*nBandCount);
    memcpy( panWinBandList, panBandList, sizeof(int)* nBandCount);
    nWinBandCount = nBandCount;

    nWinBufLoaded = -1;

/* -------------------------------------------------------------------- */
/*      Allocate current scanline buffer.                               */
/* -------------------------------------------------------------------- */
    papCurLineBuf = (void **) CPLMalloc(sizeof(void*) * nWinBandCount );
    for( int iBand = 0; iBand < nWinBandCount; iBand++ )
        papCurLineBuf[iBand] = 
            CPLMalloc(nBufXSize * (GDALGetDataTypeSize(eRasterDataType)/8) );
        
    return CE_None;
}

/************************************************************************/
/*                           TryWinRasterIO()                           */
/*                                                                      */
/*      Try to satisfy the given request based on the currently         */
/*      defined window.  Return TRUE on success or FALSE on             */
/*      failure.  On failure, the caller should satisfy the request     */
/*      another way (not report an error).                              */
/************************************************************************/

int ECWDataset::TryWinRasterIO( GDALRWFlag eFlag, 
                                int nXOff, int nYOff, int nXSize, int nYSize,
                                GByte *pabyData, int nBufXSize, int nBufYSize, 
                                GDALDataType eDT,
                                int nBandCount, int *panBandList, 
                                int nPixelSpace, int nLineSpace, 
                                int nBandSpace )

{
    int iBand, i;

/* -------------------------------------------------------------------- */
/*      Provide default buffer organization.                            */
/* -------------------------------------------------------------------- */
    if( nPixelSpace == 0 )
        nPixelSpace = GDALGetDataTypeSize( eDT ) / 8;
    if( nLineSpace == 0 )
        nLineSpace = nPixelSpace * nBufXSize;
    if( nBandSpace == 0 )
        nBandSpace = nLineSpace * nBufYSize;

/* -------------------------------------------------------------------- */
/*      Do some simple tests to see if the current window can           */
/*      satisfy our requirement.                                        */
/* -------------------------------------------------------------------- */
#ifdef NOISY_DEBUG
    CPLDebug( "ECW", "TryWinRasterIO(%d,%d,%d,%d,%d,%d)", 
              nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );
#endif

    if( !bWinActive )
        return FALSE;
    
    if( nXOff != nWinXOff || nXSize != nWinXSize )
        return FALSE;

    if( nBufXSize != nWinBufXSize )
        return FALSE;

    for( iBand = 0; iBand < nBandCount; iBand++ )
    {
        for( i = 0; i < nWinBandCount; i++ )
        {
            if( panWinBandList[i] == panBandList[iBand] )
                break;
        }

        if( i == nWinBandCount )
            return FALSE;
    }

    if( nYOff < nWinYOff || nYOff + nYSize > nWinYOff + nWinYSize )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Now we try more subtle tests.                                   */
/* -------------------------------------------------------------------- */
    {
        static int nDebugCount = 0;

        if( nDebugCount < 30 )
            CPLDebug( "ECWDataset", 
                      "TryWinRasterIO(%d,%d,%d,%d -> %dx%d) - doing advised read.", 
                      nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );

        if( nDebugCount == 29 )
            CPLDebug( "ECWDataset", "No more TryWinRasterIO messages will be reported" );
        
        nDebugCount++;
    }

/* -------------------------------------------------------------------- */
/*      Actually load data one buffer line at a time.                   */
/* -------------------------------------------------------------------- */
    int iBufLine;

    for( iBufLine = 0; iBufLine < nBufYSize; iBufLine++ )
    {
        double fFileLine = ((iBufLine+0.5) / nBufYSize) * nYSize + nYOff;
        int iWinLine = 
            (int) (((fFileLine - nWinYOff) / nWinYSize) * nWinBufYSize);
        
        if( iWinLine == nWinBufLoaded + 1 )
            LoadNextLine();

        if( iWinLine != nWinBufLoaded )
            return FALSE;

/* -------------------------------------------------------------------- */
/*      Copy out all our target bands.                                  */
/* -------------------------------------------------------------------- */
        int iWinBand;
        for( iBand = 0; iBand < nBandCount; iBand++ )
        {
            for( iWinBand = 0; iWinBand < nWinBandCount; iWinBand++ )
            {
                if( panWinBandList[iWinBand] == panBandList[iBand] )
                    break;
            }

            GDALCopyWords( papCurLineBuf[iWinBand], eRasterDataType,
                           GDALGetDataTypeSize( eRasterDataType ) / 8, 
                           pabyData + nBandSpace * iBand 
                           + iBufLine * nLineSpace, eDT, nPixelSpace,
                           nBufXSize );
        }
    }

    return TRUE;
}

/************************************************************************/
/*                            LoadNextLine()                            */
/************************************************************************/

CPLErr ECWDataset::LoadNextLine()

{
    if( !bWinActive )
        return CE_Failure;

    if( nWinBufLoaded == nWinBufYSize-1 )
    {
        CleanupWindow();
        return CE_Failure;
    }

    NCSEcwReadStatus  eRStatus;
    eRStatus = poFileView->ReadLineBIL( eNCSRequestDataType, 
                                        (UINT16) nWinBandCount,
                                        papCurLineBuf );
    if( eRStatus != NCSECW_READ_OK )
        return CE_Failure;

    nWinBufLoaded++;

    return CE_None;
}

/************************************************************************/
/*                           CleanupWindow()                            */
/************************************************************************/

void ECWDataset::CleanupWindow()

{
    if( !bWinActive )
        return;

    bWinActive = FALSE;
    CPLFree( panWinBandList );
    panWinBandList = NULL;

    for( int iBand = 0; iBand < nWinBandCount; iBand++ )
        CPLFree( papCurLineBuf[iBand] );
    CPLFree( papCurLineBuf );
    papCurLineBuf = NULL;
}

/************************************************************************/
/*                             IRasterIO()                              */
/************************************************************************/

CPLErr ECWDataset::IRasterIO( GDALRWFlag eRWFlag,
                              int nXOff, int nYOff, int nXSize, int nYSize,
                              void * pData, int nBufXSize, int nBufYSize,
                              GDALDataType eBufType, 
                              int nBandCount, int *panBandMap,
                              int nPixelSpace, int nLineSpace, int nBandSpace)
    
{
/* -------------------------------------------------------------------- */
/*      Try to do it based on existing "advised" access.                */
/* -------------------------------------------------------------------- */
    if( TryWinRasterIO( eRWFlag, nXOff, nYOff, nXSize, nYSize, 
                        (GByte *) pData, nBufXSize, nBufYSize, 
                        eBufType, nBandCount, panBandMap,
                        nPixelSpace, nLineSpace, nBandSpace ) )
        return CE_None;

/* -------------------------------------------------------------------- */
/*      If we are requesting a single line at 1:1, we do a multi-band   */
/*      AdviseRead() and then TryWinRasterIO() again.                   */
/* -------------------------------------------------------------------- */
    if( nYSize == 1 && nBufYSize == 1 && nBandCount > 1 )
    {
        CPLErr eErr;

        eErr = AdviseRead( nXOff, nYOff, nXSize, GetRasterYSize() - nYOff,
                           nBufXSize, GetRasterYSize() - nYOff, eBufType, 
                           nBandCount, panBandMap, NULL );
        if( eErr == CE_None 
            && TryWinRasterIO( eRWFlag, nXOff, nYOff, nXSize, nYSize, 
                               (GByte *) pData, nBufXSize, nBufYSize, 
                               eBufType, nBandCount, panBandMap,
                               nPixelSpace, nLineSpace, nBandSpace ) )
            return CE_None;
    }

/* -------------------------------------------------------------------- */
/*      If we are supersampling we need to fall into the general        */
/*      purpose logic.  We also use the general logic if we are in      */
/*      some cases unlikely to benefit from interleaved access.         */
/*                                                                      */
/*      The one case we would like to handle better here is the         */
/*      nBufYSize == 1 case (requesting a scanline at a time).  We      */
/*      should eventually have some logic similiar to the band by       */
/*      band case where we post a big window for the view, and allow    */
/*      sequential reads.                                               */
/* -------------------------------------------------------------------- */
    if( nXSize < nBufXSize || nYSize < nBufYSize || nYSize == 1 
        || nBandCount > 100 || nBandCount == 1 || nBufYSize == 1 
        || nBandCount > GetRasterCount() )
    {
        return 
            GDALDataset::IRasterIO( eRWFlag, nXOff, nYOff, nXSize, nYSize,
                                    pData, nBufXSize, nBufYSize,
                                    eBufType, 
                                    nBandCount, panBandMap,
                                    nPixelSpace, nLineSpace, nBandSpace);
    }

    CPLDebug( "ECWDataset", 
              "RasterIO(%d,%d,%d,%d -> %dx%d) - doing interleaved read.", 
              nXOff, nYOff, nXSize, nYSize, nBufXSize, nBufYSize );

/* -------------------------------------------------------------------- */
/*      Setup view.                                                     */
/* -------------------------------------------------------------------- */
    UINT32 anBandIndices[100];
    int    i;
    NCSError     eNCSErr;
    CNCSError    oErr;
    
    for( i = 0; i < nBandCount; i++ )
        anBandIndices[i] = panBandMap[i] - 1;

    CleanupWindow();

    oErr = poFileView->SetView( nBandCount, anBandIndices,
                                nXOff, nYOff, 
                                nXOff + nXSize - 1, 
                                nYOff + nYSize - 1,
                                nBufXSize, nBufYSize );
    eNCSErr = oErr.GetErrorNumber();
    
    if( eNCSErr != NCS_SUCCESS )
    {
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "%s", NCSGetErrorText(eNCSErr) );
        
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Setup working scanline, and the pointers into it.               */
/* -------------------------------------------------------------------- */
    int nDataTypeSize = (GDALGetDataTypeSize(eRasterDataType) / 8);
    GByte *pabyBILScanline = (GByte *) CPLMalloc(nBufXSize * nDataTypeSize *
                                                 nBandCount);
    GByte **papabyBIL = (GByte **) CPLMalloc(nBandCount * sizeof(void*));

    for( i = 0; i < nBandCount; i++ )
        papabyBIL[i] = pabyBILScanline + i * nBufXSize * nDataTypeSize;

/* -------------------------------------------------------------------- */
/*      Read back all the data for the requested view.                  */
/* -------------------------------------------------------------------- */
    for( int iScanline = 0; iScanline < nBufYSize; iScanline++ )
    {
        NCSEcwReadStatus  eRStatus;

        eRStatus = poFileView->ReadLineBIL( eNCSRequestDataType, 
                                            (UINT16) nBandCount,
                                            (void **) papabyBIL );
        if( eRStatus != NCSECW_READ_OK )
        {
            CPLFree( papabyBIL );
            CPLFree( pabyBILScanline );
            CPLError( CE_Failure, CPLE_AppDefined,
                      "NCScbmReadViewLineBIL failed." );
            return CE_Failure;
        }

        for( i = 0; i < nBandCount; i++ )
        {
            GDALCopyWords( 
                pabyBILScanline + i * nDataTypeSize * nBufXSize,
                eRasterDataType, nDataTypeSize, 
                ((GByte *) pData) + nLineSpace * iScanline + nBandSpace * i, 
                eBufType, nPixelSpace, 
                nBufXSize );
        }
    }

    CPLFree( pabyBILScanline );
    CPLFree( papabyBIL );

    return CE_None;
}

/************************************************************************/
/*                        IdentifyJPEG2000()                            */
/*                                                                      */
/*          Open method that only supports JPEG2000 files.              */
/************************************************************************/

int ECWDataset::IdentifyJPEG2000( GDALOpenInfo * poOpenInfo )

{
    if( EQUALN(poOpenInfo->pszFilename,"J2K_SUBFILE:",12) )
        return TRUE;

    else if( poOpenInfo->nHeaderBytes >= 16 
        && (memcmp( poOpenInfo->pabyHeader, jpc_header, 
                    sizeof(jpc_header) ) == 0
            || memcmp( poOpenInfo->pabyHeader, jp2_header, 
                    sizeof(jp2_header) ) == 0) )
        return TRUE;
    
    else
        return FALSE;
}

/************************************************************************/
/*                            OpenJPEG2000()                            */
/*                                                                      */
/*          Open method that only supports JPEG2000 files.              */
/************************************************************************/

GDALDataset *ECWDataset::OpenJPEG2000( GDALOpenInfo * poOpenInfo )

{
    if (!IdentifyJPEG2000(poOpenInfo))
        return NULL;

    return Open( poOpenInfo, TRUE );
}
    
/************************************************************************/
/*                           IdentifyECW()                              */
/*                                                                      */
/*      Identify method that only supports ECW files.                   */
/************************************************************************/

int ECWDataset::IdentifyECW( GDALOpenInfo * poOpenInfo )

{
/* -------------------------------------------------------------------- */
/*      This has to either be a file on disk ending in .ecw or a        */
/*      ecwp: protocol url.                                             */
/* -------------------------------------------------------------------- */
    if( (!EQUAL(CPLGetExtension(poOpenInfo->pszFilename),"ecw")
         || poOpenInfo->nHeaderBytes == 0)
        && !EQUALN(poOpenInfo->pszFilename,"ecwp:",5) )
        return FALSE;

    return TRUE;
}

/************************************************************************/
/*                              OpenECW()                               */
/*                                                                      */
/*      Open method that only supports ECW files.                       */
/************************************************************************/

GDALDataset *ECWDataset::OpenECW( GDALOpenInfo * poOpenInfo )

{
    if (!IdentifyECW(poOpenInfo))
        return NULL;

    return Open( poOpenInfo, FALSE );
}
    
/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *ECWDataset::Open( GDALOpenInfo * poOpenInfo, int bIsJPEG2000 )

{
    CNCSJP2FileView *poFileView = NULL;
    NCSError         eErr;
    CNCSError        oErr;
    int              i;
    VSILFILE        *fpVSIL = NULL;
    VSIIOStream *poIOStream = NULL;
    int              bUsingCustomStream = FALSE;

    ECWInitialize();

/* -------------------------------------------------------------------- */
/*      This will disable automatic conversion of YCbCr to RGB by       */
/*      the toolkit.                                                    */
/* -------------------------------------------------------------------- */
    if( !CSLTestBoolean( CPLGetConfigOption("CONVERT_YCBCR_TO_RGB","YES") ) )
        NCSecwSetConfig(NCSCFG_JP2_MANAGE_ICC, FALSE);

/* -------------------------------------------------------------------- */
/*      Handle special case of a JPEG2000 data stream in another file.  */
/* -------------------------------------------------------------------- */
    int bIsVirtualFile = FALSE;
try_again:
    if( EQUALN(poOpenInfo->pszFilename,"J2K_SUBFILE:",12) ||
        bIsVirtualFile )
    {
        GIntBig            subfile_offset=-1, subfile_size=-1;
        const char *real_filename = NULL;

          if (EQUALN(poOpenInfo->pszFilename,"J2K_SUBFILE:",12))
          {
            char** papszTokens = CSLTokenizeString2(poOpenInfo->pszFilename + 12, ",", 0);
            if (CSLCount(papszTokens) >= 2)
            {
                subfile_offset = CPLScanUIntBig(papszTokens[0], strlen(papszTokens[0]));
                subfile_size = CPLScanUIntBig(papszTokens[1], strlen(papszTokens[1]));
            }
            else
            {
                CPLError( CE_Failure, CPLE_OpenFailed, 
                            "Failed to parse J2K_SUBFILE specification." );
                CSLDestroy(papszTokens);
                return NULL;
            }
            CSLDestroy(papszTokens);

            real_filename = strstr(poOpenInfo->pszFilename,",");
            if( real_filename != NULL )
                real_filename = strstr(real_filename+1,",");
            if( real_filename != NULL )
                real_filename++;
            else
            {
                CPLError( CE_Failure, CPLE_OpenFailed, 
                            "Failed to parse J2K_SUBFILE specification." );
                return NULL;
            }

          }
          else
          {
              real_filename = poOpenInfo->pszFilename;
              subfile_offset = 0;
          }

          fpVSIL = VSIFOpenL( real_filename, "rb" );
          if( fpVSIL == NULL )
          {
              CPLError( CE_Failure, CPLE_OpenFailed, 
                        "Failed to open %s.",  real_filename );
              return NULL;
          }

          if( hECWDatasetMutex == NULL )
          {
              hECWDatasetMutex = CPLCreateMutex();
          }
          else if( !CPLAcquireMutex( hECWDatasetMutex, 60.0 ) )
          {
              CPLDebug( "ECW", "Failed to acquire mutex in 60s." );
          }
          else
          {
              CPLDebug( "ECW", "Got mutex." );
          }
          poIOStream = new VSIIOStream();
          poIOStream->Access( fpVSIL, FALSE, real_filename,
                              subfile_offset, subfile_size );

          poFileView = new CNCSJP2FileView();
          oErr = poFileView->Open( poIOStream, false );

          // The CNCSJP2FileView (poFileView) object may not use the iostream 
          // (poIOStream) passed to the CNCSJP2FileView::Open() method if an 
          // iostream is already available to the ECW JPEG 2000 SDK for a given
          // file.  Consequently, if the iostream passed to 
          // CNCSJP2FileView::Open() does not become the underlying iostream 
          // of the CNCSJP2FileView object, then it should be deleted.
          //
          // In addition, the underlying iostream of the CNCSJP2FileView object
          // should not be deleted until all CNCSJP2FileView objects using the 
          // underlying iostream are deleted. Consequently, each time a 
          // CNCSJP2FileView object is created, the nFileViewCount attribute 
          // of the underlying VSIIOStream object must be incremented for use 
          // in the ECWDataset destructor.
		  
          VSIIOStream * poUnderlyingIOStream = 
              ((VSIIOStream *)(poFileView->GetStream()));

          if ( poUnderlyingIOStream )
              poUnderlyingIOStream->nFileViewCount++;

          if ( poIOStream != poUnderlyingIOStream ) 
          {
              delete poIOStream;
          }
          else
          {
              bUsingCustomStream = TRUE;
          }

          CPLReleaseMutex( hECWDatasetMutex );

          if( oErr.GetErrorNumber() != NCS_SUCCESS )
          {
              if (poFileView)
                  delete poFileView;

              char* pszErrorMessage = oErr.GetErrorMessage();
              CPLError( CE_Failure, CPLE_AppDefined, 
                        "%s", pszErrorMessage );
              NCSFree(pszErrorMessage);

              return NULL;
          }
    }

/* -------------------------------------------------------------------- */
/*      This has to either be a file on disk ending in .ecw or a        */
/*      ecwp: protocol url.                                             */
/* -------------------------------------------------------------------- */
    else if( poOpenInfo->nHeaderBytes >= 16 
        && (memcmp( poOpenInfo->pabyHeader, jpc_header, 
                    sizeof(jpc_header) ) == 0
            || memcmp( poOpenInfo->pabyHeader, jp2_header, 
                    sizeof(jp2_header) ) == 0) )
    {
        /* accept JPEG2000 files */
    }
    else if( (!EQUAL(CPLGetExtension(poOpenInfo->pszFilename),"ecw")
              || poOpenInfo->nHeaderBytes == 0)
             && !EQUALN(poOpenInfo->pszFilename,"ecwp:",5) )
        return( NULL );
    
/* -------------------------------------------------------------------- */
/*      Open the client interface.                                      */
/* -------------------------------------------------------------------- */
    if( poFileView == NULL )
    {
        poFileView = new CNCSFile();
        oErr = poFileView->Open( (char *) poOpenInfo->pszFilename, FALSE );
        eErr = oErr.GetErrorNumber();
        CPLDebug( "ECW", "NCScbmOpenFileView(%s): eErr = %d", 
                  poOpenInfo->pszFilename, (int) eErr );
        if( eErr != NCS_SUCCESS )
        {
            delete poFileView;

            /* If the file is not a 'real' file but recognized as a */
            /* virtual file by the VSIL API, try again by using a */
            /* VSIIOStream object, like in the J2K_SUBFILE case */
            VSIStatBuf sBuf;
            VSIStatBufL sBufL;
            if (!bIsVirtualFile &&
                VSIStat(poOpenInfo->pszFilename, &sBuf) != 0 &&
                VSIStatL(poOpenInfo->pszFilename, &sBufL) == 0)
            {
                bIsVirtualFile = TRUE;
                goto try_again;
            }

            CPLError( CE_Failure, CPLE_AppDefined, 
                      "%s", NCSGetErrorText(eErr) );
            return NULL;
        }
    }

    if( poOpenInfo->eAccess == GA_Update )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "The ECW driver does not support update access to existing"
                  " datasets.\n" );
        return NULL;
    }
    
/* -------------------------------------------------------------------- */
/*      Create a corresponding GDALDataset.                             */
/* -------------------------------------------------------------------- */
    ECWDataset  *poDS;

    poDS = new ECWDataset(bIsJPEG2000);

    poDS->poFileView = poFileView;

    if( fpVSIL != NULL )
        poDS->nPamFlags |= GPF_DISABLED;

    poDS->bUsingCustomStream = bUsingCustomStream;

/* -------------------------------------------------------------------- */
/*      Fetch general file information.                                 */
/* -------------------------------------------------------------------- */
    poDS->psFileInfo = poFileView->GetFileInfo();

    CPLDebug( "ECW", "FileInfo: SizeXY=%d,%d Bands=%d\n"
              "       OriginXY=%g,%g  CellIncrementXY=%g,%g\n"
              "       ColorSpace=%d, eCellType=%d\n", 
              poDS->psFileInfo->nSizeX,
              poDS->psFileInfo->nSizeY,
              poDS->psFileInfo->nBands,
              poDS->psFileInfo->fOriginX,
              poDS->psFileInfo->fOriginY,
              poDS->psFileInfo->fCellIncrementX,
              poDS->psFileInfo->fCellIncrementY,
              (int) poDS->psFileInfo->eColorSpace,
              (int) poDS->psFileInfo->eCellType );

/* -------------------------------------------------------------------- */
/*      Establish raster info.                                          */
/* -------------------------------------------------------------------- */
    poDS->nRasterXSize = poDS->psFileInfo->nSizeX; 
    poDS->nRasterYSize = poDS->psFileInfo->nSizeY;

/* -------------------------------------------------------------------- */
/*      Establish the GDAL data type that corresponds.  A few NCS       */
/*      data types have no direct corresponding value in GDAL so we     */
/*      will coerce to something sufficiently similar.                  */
/* -------------------------------------------------------------------- */
    poDS->eNCSRequestDataType = poDS->psFileInfo->eCellType;
    switch( poDS->psFileInfo->eCellType )
    {
        case NCSCT_UINT8:
            poDS->eRasterDataType = GDT_Byte;
            break;

        case NCSCT_UINT16:
            poDS->eRasterDataType = GDT_UInt16;
            break;

        case NCSCT_UINT32:
        case NCSCT_UINT64:
            poDS->eRasterDataType = GDT_UInt32;
            poDS->eNCSRequestDataType = NCSCT_UINT32;
            break;

        case NCSCT_INT8:
        case NCSCT_INT16:
            poDS->eRasterDataType = GDT_Int16;
            poDS->eNCSRequestDataType = NCSCT_INT16;
            break;

        case NCSCT_INT32:
        case NCSCT_INT64:
            poDS->eRasterDataType = GDT_Int32;
            poDS->eNCSRequestDataType = NCSCT_INT32;
            break;

        case NCSCT_IEEE4:
            poDS->eRasterDataType = GDT_Float32;
            break;

        case NCSCT_IEEE8:
            poDS->eRasterDataType = GDT_Float64;
            break;
    }

/* -------------------------------------------------------------------- */
/*      Create band information objects.                                */
/* -------------------------------------------------------------------- */
    for( i=0; i < poDS->psFileInfo->nBands; i++ )
        poDS->SetBand( i+1, new ECWRasterBand( poDS, i+1 ) );

/* -------------------------------------------------------------------- */
/*      Look for supporting coordinate system information.              */
/* -------------------------------------------------------------------- */
    GDALJP2Metadata oJP2Geo;

    if( bIsJPEG2000 && oJP2Geo.ReadAndParse( poOpenInfo->pszFilename ) )
    {
        poDS->pszProjection = CPLStrdup(oJP2Geo.pszProjection);
        poDS->bGeoTransformValid = oJP2Geo.bHaveGeoTransform;
        memcpy( poDS->adfGeoTransform, oJP2Geo.adfGeoTransform, 
                sizeof(double) * 6 );
        poDS->nGCPCount = oJP2Geo.nGCPCount;
        poDS->pasGCPList = oJP2Geo.pasGCPList;
        oJP2Geo.pasGCPList = NULL;
        oJP2Geo.nGCPCount = 0;
    }
    else
    {
        poDS->ECW2WKTProjection();
    }

/* -------------------------------------------------------------------- */
/*      Check for world file for ecw files.                             */
/* -------------------------------------------------------------------- */
    if( !poDS->bGeoTransformValid 
        && EQUAL(CPLGetExtension(poOpenInfo->pszFilename),"ecw") )
    {
        poDS->bGeoTransformValid |= 
            GDALReadWorldFile( poOpenInfo->pszFilename, ".eww", 
                               poDS->adfGeoTransform )
            || GDALReadWorldFile( poOpenInfo->pszFilename, ".ecww", 
                                  poDS->adfGeoTransform )
            || GDALReadWorldFile( poOpenInfo->pszFilename, ".wld", 
                                  poDS->adfGeoTransform );
    }

/* -------------------------------------------------------------------- */
/*      Initialize any PAM information.                                 */
/* -------------------------------------------------------------------- */
    poDS->SetDescription( poOpenInfo->pszFilename );
    poDS->TryLoadXML();
    
/* -------------------------------------------------------------------- */
/*      Confirm the requested access is supported.                      */
/* -------------------------------------------------------------------- */
    if( poOpenInfo->eAccess == GA_Update )
    {
        delete poDS;
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "The ECW driver does not support update access to existing"
                  " datasets.\n" );
        return NULL;
    }
    
    return( poDS );
}

/************************************************************************/
/*                            GetGCPCount()                             */
/************************************************************************/

int ECWDataset::GetGCPCount()

{
    if( nGCPCount != 0 )
        return nGCPCount;
    else
        return GDALPamDataset::GetGCPCount();
}

/************************************************************************/
/*                          GetGCPProjection()                          */
/************************************************************************/

const char *ECWDataset::GetGCPProjection()

{
    if( nGCPCount > 0 )
        return pszProjection;
    else
        return GDALPamDataset::GetGCPProjection();
}

/************************************************************************/
/*                               GetGCP()                               */
/************************************************************************/

const GDAL_GCP *ECWDataset::GetGCPs()

{
    if( nGCPCount != 0 )
        return pasGCPList;
    else
        return GDALPamDataset::GetGCPs();
}

/************************************************************************/
/*                          GetProjectionRef()                          */
/*                                                                      */
/*      We let PAM coordinate system override the one stored inside     */
/*      our file.                                                       */
/************************************************************************/

const char *ECWDataset::GetProjectionRef() 

{
    const char* pszPamPrj = GDALPamDataset::GetProjectionRef();

    if( pszProjection != NULL && strlen(pszPamPrj) == 0 )
        return pszProjection;
    else
        return pszPamPrj;
}

/************************************************************************/
/*                          GetGeoTransform()                           */
/*                                                                      */
/*      Let the PAM geotransform override the native one if it is       */
/*      available.                                                      */
/************************************************************************/

CPLErr ECWDataset::GetGeoTransform( double * padfTransform )

{
    CPLErr eErr = GDALPamDataset::GetGeoTransform( padfTransform );

    if( eErr != CE_None && bGeoTransformValid )
    {
        memcpy( padfTransform, adfGeoTransform, sizeof(double) * 6 );
        return( CE_None );
    }
    else
        return eErr;
}

/************************************************************************/
/*                            GetMetadata()                             */
/************************************************************************/

char **ECWDataset::GetMetadata( const char *pszDomain )

{
    if( pszDomain == NULL || !EQUAL(pszDomain,"GML") )
        return GDALPamDataset::GetMetadata( pszDomain );
    else
        return papszGMLMetadata;
}

/************************************************************************/
/*                         ECW2WKTProjection()                          */
/*                                                                      */
/*      Set the dataset pszProjection string in OGC WKT format by       */
/*      looking up the ECW (GDT) coordinate system info in              */
/*      ecw_cs.dat support data file.                                   */
/*                                                                      */
/*      This code is likely still broken in some circumstances.  For    */
/*      instance, I haven't been careful about changing the linear      */
/*      projection parameters (false easting/northing) if the units     */
/*      is feet.  Lots of cases missing here, and in ecw_cs.dat.        */
/************************************************************************/

void ECWDataset::ECW2WKTProjection()

{
    if( psFileInfo == NULL )
        return;

/* -------------------------------------------------------------------- */
/*      Capture Geotransform.                                           */
/*                                                                      */
/*      We will try to ignore the provided file information if it is    */
/*      origin (0,0) and pixel size (1,1).  I think sometimes I have    */
/*      also seen pixel increments of 0 on invalid datasets.            */
/* -------------------------------------------------------------------- */
    if( psFileInfo->fOriginX != 0.0 
        || psFileInfo->fOriginY != 0.0 
        || (psFileInfo->fCellIncrementX != 0.0 
            && psFileInfo->fCellIncrementX != 1.0)
        || (psFileInfo->fCellIncrementY != 0.0 
            && psFileInfo->fCellIncrementY != 1.0) )
    {
        bGeoTransformValid = TRUE;
        
        adfGeoTransform[0] = psFileInfo->fOriginX;
        adfGeoTransform[1] = psFileInfo->fCellIncrementX;
        adfGeoTransform[2] = 0.0;
        
        adfGeoTransform[3] = psFileInfo->fOriginY;
        adfGeoTransform[4] = 0.0;
        adfGeoTransform[5] = -fabs(psFileInfo->fCellIncrementY);
    }

/* -------------------------------------------------------------------- */
/*      do we have projection and datum?                                */
/* -------------------------------------------------------------------- */
    CPLDebug( "ECW", "projection=%s, datum=%s",
              psFileInfo->szProjection, psFileInfo->szDatum );

    if( EQUAL(psFileInfo->szProjection,"RAW") )
        return;

/* -------------------------------------------------------------------- */
/*      Set projection if we have it.                                   */
/* -------------------------------------------------------------------- */
    OGRSpatialReference oSRS;
    CPLString osUnits = "METERS";

    if( psFileInfo->eCellSizeUnits == ECW_CELL_UNITS_FEET )
        osUnits = "FEET";

    if( oSRS.importFromERM( psFileInfo->szProjection, 
                            psFileInfo->szDatum, 
                            osUnits ) != OGRERR_NONE )
        return;

    oSRS.exportToWkt( &pszProjection );
}

#endif /* def FRMT_ecw */

/************************************************************************/
/*                           ECWInitialize()                            */
/*                                                                      */
/*      Initialize NCS library.  We try to defer this as late as        */
/*      possible since de-initializing it seems to be expensive/slow    */
/*      on some system.                                                 */
/************************************************************************/

void ECWInitialize()

{
    CPLMutexHolder oHolder( &hECWDatasetMutex );

    if( bNCSInitialized )
        return;

    NCSecwInit();
    bNCSInitialized = TRUE;

/* -------------------------------------------------------------------- */
/*      Initialize cache memory limit.  Default is apparently 1/4 RAM.  */
/* -------------------------------------------------------------------- */
    const char *pszEcwCacheSize = 
        CPLGetConfigOption("GDAL_ECW_CACHE_MAXMEM",NULL);
    if( pszEcwCacheSize == NULL )
        CPLGetConfigOption("ECW_CACHE_MAXMEM",NULL);

    if( pszEcwCacheSize != NULL )
        NCSecwSetConfig(NCSCFG_CACHE_MAXMEM, (UINT32) atoi(pszEcwCacheSize) );

/* -------------------------------------------------------------------- */
/*      Allow configuration of a local cache based on configuration     */
/*      options.  Setting the location turns things on.                 */
/* -------------------------------------------------------------------- */
    const char *pszOpt;

#if ECWSDK_VERSION >= 40
    pszOpt = CPLGetConfigOption( "ECWP_CACHE_SIZE_MB", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_ECWP_CACHE_SIZE_MB, (INT32) atoi( pszOpt ) );

    pszOpt = CPLGetConfigOption( "ECWP_CACHE_LOCATION", NULL );
    if( pszOpt )
    {
        NCSecwSetConfig( NCSCFG_ECWP_CACHE_LOCATION, pszOpt );
        NCSecwSetConfig( NCSCFG_ECWP_CACHE_ENABLED, (BOOLEAN) TRUE );
    }
#endif

/* -------------------------------------------------------------------- */
/*      Various other configuration items.                              */
/* -------------------------------------------------------------------- */
    pszOpt = CPLGetConfigOption( "ECW_TEXTURE_DITHER", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_TEXTURE_DITHER, 
                         (BOOLEAN) CSLTestBoolean( pszOpt ) );


    pszOpt = CPLGetConfigOption( "ECW_FORCE_FILE_REOPEN", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_FORCE_FILE_REOPEN, 
                         (BOOLEAN) CSLTestBoolean( pszOpt ) );

    pszOpt = CPLGetConfigOption( "ECW_CACHE_MAXOPEN", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_CACHE_MAXOPEN, (UINT32) atoi(pszOpt) );

    pszOpt = CPLGetConfigOption( "ECW_AUTOGEN_J2I", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_FORCE_FILE_REOPEN, 
                         (BOOLEAN) CSLTestBoolean( pszOpt ) );

#if ECWSDK_VERSION >= 40
    pszOpt = CPLGetConfigOption( "ECW_OPTIMIZE_USE_NEAREST_NEIGHBOUR", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_OPTIMIZE_USE_NEAREST_NEIGHBOUR, 
                         (BOOLEAN) CSLTestBoolean( pszOpt ) );


    pszOpt = CPLGetConfigOption( "ECW_RESILIENT_DECODING", NULL );
    if( pszOpt )
        NCSecwSetConfig( NCSCFG_RESILIENT_DECODING, 
                         (BOOLEAN) CSLTestBoolean( pszOpt ) );
#endif
}

/************************************************************************/
/*                         GDALDeregister_ECW()                         */
/************************************************************************/

void GDALDeregister_ECW( GDALDriver * )

{
    /* For unknown reason, this cleanup can take up to 3 seconds (see #3134). */
    /* Not worth it */
#ifdef notdef
    if( bNCSInitialized )
    {
        bNCSInitialized = FALSE;
        NCSecwShutdown();
    }

    if( hECWDatasetMutex != NULL )
    {
        CPLDestroyMutex( hECWDatasetMutex );
        hECWDatasetMutex = NULL;
    }
#endif
}

/************************************************************************/
/*                          GDALRegister_ECW()                        */
/************************************************************************/

void GDALRegister_ECW()

{
#ifdef FRMT_ecw 
    GDALDriver	*poDriver;

    if (! GDAL_CHECK_VERSION("ECW driver"))
        return;

    if( GDALGetDriverByName( "ECW" ) == NULL )
    {
        poDriver = new GDALDriver();
        
        poDriver->SetDescription( "ECW" );

        CPLString osLongName = "ERDAS Compressed Wavelets (SDK ";

#ifdef NCS_ECWSDK_VERSION_STRING
        osLongName += NCS_ECWSDK_VERSION_STRING;
#else
        osLongName += "3.x";
#endif        
        osLongName += ")";

        poDriver->SetMetadataItem( GDAL_DMD_LONGNAME, osLongName );
        poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC, 
                                   "frmt_ecw.html" );
        poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "ecw" );
        
        poDriver->pfnIdentify = ECWDataset::IdentifyECW;
        poDriver->pfnOpen = ECWDataset::OpenECW;
        poDriver->pfnUnloadDriver = GDALDeregister_ECW;
#ifdef HAVE_COMPRESS
// The create method seems not to work properly.
//        poDriver->pfnCreate = ECWCreateECW;  
        poDriver->pfnCreateCopy = ECWCreateCopyECW;
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONDATATYPES, 
                                   "Byte" );
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONOPTIONLIST, 
"<CreationOptionList>"
"   <Option name='TARGET' type='float' description='Compression Percentage' />"
"   <Option name='PROJ' type='string' description='ECW Projection Name'/>"
"   <Option name='DATUM' type='string' description='ECW Datum Name' />"

#if ECWSDK_VERSION < 40
"   <Option name='LARGE_OK' type='boolean' description='Enable compressing 500+MB files'/>"
#else
"   <Option name='ECW_ENCODE_KEY' type='string' description='OEM Compress Key from ERDAS.'/>"
"   <Option name='ECW_ENCODE_COMPANY' type='string' description='OEM Company Name.'/>"
#endif

"</CreationOptionList>" );
#endif

        GetGDALDriverManager()->RegisterDriver( poDriver );
    }
#endif /* def FRMT_ecw */
}

/************************************************************************/
/*                      GDALRegister_ECW_JP2ECW()                       */
/*                                                                      */
/*      This function exists so that when built as a plugin, there      */
/*      is a function that will register both drivers.                  */
/************************************************************************/

void GDALRegister_ECW_JP2ECW()

{
    GDALRegister_ECW();
    GDALRegister_JP2ECW();
}

/************************************************************************/
/*                     ECWDatasetOpenJPEG2000()                         */
/************************************************************************/
GDALDataset* ECWDatasetOpenJPEG2000(GDALOpenInfo* poOpenInfo)
{
    return ECWDataset::OpenJPEG2000(poOpenInfo);
}

/************************************************************************/
/*                        GDALRegister_JP2ECW()                         */
/************************************************************************/
void GDALRegister_JP2ECW()

{
#ifdef FRMT_ecw 
    GDALDriver	*poDriver;

    if (! GDAL_CHECK_VERSION("JP2ECW driver"))
        return;

    if( GDALGetDriverByName( "JP2ECW" ) == NULL )
    {
        poDriver = new GDALDriver();
        
        poDriver->SetDescription( "JP2ECW" );

        CPLString osLongName = "ERDAS JPEG2000 (SDK ";

#ifdef NCS_ECWSDK_VERSION_STRING
        osLongName += NCS_ECWSDK_VERSION_STRING;
#else
        osLongName += "3.x";
#endif        
        osLongName += ")";

        poDriver->SetMetadataItem( GDAL_DMD_LONGNAME, osLongName );
        poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC, 
                                   "frmt_jp2ecw.html" );
        poDriver->SetMetadataItem( GDAL_DMD_EXTENSION, "jp2" );
        
        poDriver->pfnIdentify = ECWDataset::IdentifyJPEG2000;
        poDriver->pfnOpen = ECWDataset::OpenJPEG2000;
#ifdef HAVE_COMPRESS
        poDriver->pfnCreate = ECWCreateJPEG2000;
        poDriver->pfnCreateCopy = ECWCreateCopyJPEG2000;
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONDATATYPES, 
                                   "Byte uint16_t int16_t uint32_t int32_t Float32 Float64" );
        poDriver->SetMetadataItem( GDAL_DMD_CREATIONOPTIONLIST, 
"<CreationOptionList>"
"   <Option name='TARGET' type='float' description='Compression Percentage' />"
"   <Option name='PROJ' type='string' description='ECW Projection Name'/>"
"   <Option name='DATUM' type='string' description='ECW Datum Name' />"

#if ECWSDK_VERSION < 40
"   <Option name='LARGE_OK' type='boolean' description='Enable compressing 500+MB files'/>"
#else
"   <Option name='ECW_ENCODE_KEY' type='string' description='OEM Compress Key from ERDAS.'/>"
"   <Option name='ECW_ENCODE_COMPANY' type='string' description='OEM Company Name.'/>"
#endif

"   <Option name='GeoJP2' type='boolean' description='defaults to ON'/>"
"   <Option name='GMLJP2' type='boolean' description='defaults to ON'/>"
"   <Option name='PROFILE' type='string-select'>"
"       <Value>BASELINE_0</Value>"
"       <Value>BASELINE_1</Value>"
"       <Value>BASELINE_2</Value>"
"       <Value>NPJE</Value>"
"       <Value>EPJE</Value>"
"   </Option>"
"   <Option name='PROGRESSION' type='string-select'>"
"       <Value>LRCP</Value>"
"       <Value>RLCP</Value>"
"       <Value>RPCL</Value>"
"   </Option>"
"   <Option name='CODESTREAM_ONLY' type='boolean' description='No JP2 wrapper'/>"
"   <Option name='LEVELS' type='int'/>"
"   <Option name='LAYERS' type='int'/>"
"   <Option name='PRECINCT_WIDTH' type='int'/>"
"   <Option name='PRECINCT_HEIGHT' type='int'/>"
"   <Option name='TILE_WIDTH' type='int'/>"
"   <Option name='TILE_HEIGHT' type='int'/>"
"   <Option name='INCLUDE_SOP' type='boolean'/>"
"   <Option name='INCLUDE_EPH' type='boolean'/>"
"   <Option name='DECOMPRESS_LAYERS' type='int'/>"
"   <Option name='DECOMPRESS_RECONSTRUCTION_PARAMETER' type='float'/>"
"</CreationOptionList>" );
#endif

        GetGDALDriverManager()->RegisterDriver( poDriver );
    }
#endif /* def FRMT_ecw */
}




