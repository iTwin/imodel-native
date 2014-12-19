/******************************************************************************
 * $Id: sdtsrasterreader.cpp 21298 2010-12-20 10:58:34Z rouault $
 *
 * Project:  SDTS Translator
 * Purpose:  Implementation of SDTSRasterReader class.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
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

#include "sdts_al.h"

CPL_CVSID("$Id: sdtsrasterreader.cpp 21298 2010-12-20 10:58:34Z rouault $");

/************************************************************************/
/*                          SDTSRasterReader()                          */
/************************************************************************/

SDTSRasterReader::SDTSRasterReader()

{
    nXSize = 0;
    nYSize = 0;
    nXBlockSize = 0;
    nYBlockSize = 0;
    nXStart = 0;
    nYStart = 0;
    
    strcpy( szINTR, "CE" );
}

/************************************************************************/
/*                             ~SDTSRasterReader()                     */
/************************************************************************/

SDTSRasterReader::~SDTSRasterReader()
{
}

/************************************************************************/
/*                               Close()                                */
/************************************************************************/

void SDTSRasterReader::Close()

{
    oDDFModule.Close();
}

/************************************************************************/
/*                                Open()                                */
/*                                                                      */
/*      Open the requested cell file, and collect required              */
/*      information.                                                    */
/************************************************************************/

int SDTSRasterReader::Open( SDTS_CATD * poCATD, SDTS_IREF * poIREF,
                            const char * pszModule )

{
    strncpy( szModule, pszModule, sizeof(szModule) );
    szModule[sizeof(szModule) - 1] = '\0';
    
/* ==================================================================== */
/*      Search the LDEF module for the requested cell module.           */
/* ==================================================================== */
    DDFModule   oLDEF;
    DDFRecord   *poRecord;

/* -------------------------------------------------------------------- */
/*      Open the LDEF module, and report failure if it is missing.      */
/* -------------------------------------------------------------------- */
    if( poCATD->GetModuleFilePath("LDEF") == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find LDEF entry in CATD module ... "
                  "can't treat as raster.\n" );
        return FALSE;
    }
    
    if( !oLDEF.Open( poCATD->GetModuleFilePath("LDEF") ) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Read each record, till we find what we want.                    */
/* -------------------------------------------------------------------- */
    while( (poRecord = oLDEF.ReadRecord() ) != NULL )
    {
        if( EQUAL(poRecord->GetStringSubfield("LDEF",0,"CMNM",0), pszModule) )
            break;
    }

    if( poRecord == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find module `%s' in LDEF file.\n",
                  pszModule );
        return FALSE;
    }
    
/* -------------------------------------------------------------------- */
/*      Extract raster dimensions, and origin offset (0/1).             */
/* -------------------------------------------------------------------- */
    nXSize = poRecord->GetIntSubfield( "LDEF", 0, "NCOL", 0 );
    nYSize = poRecord->GetIntSubfield( "LDEF", 0, "NROW", 0 );

    nXStart = poRecord->GetIntSubfield( "LDEF", 0, "SOCI", 0 );
    nYStart = poRecord->GetIntSubfield( "LDEF", 0, "SORI", 0 );

/* -------------------------------------------------------------------- */
/*      Get the point in the pixel that the origin defines.  We only    */
/*      support top left and center.                                    */
/* -------------------------------------------------------------------- */
    strcpy( szINTR, poRecord->GetStringSubfield(  "LDEF", 0, "INTR", 0 ) );
    if( EQUAL(szINTR,"") )
        strcpy( szINTR, "CE" );
    
    if( !EQUAL(szINTR,"CE") && !EQUAL(szINTR,"TL") )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Unsupported INTR value of `%s', assume CE.\n"
                  "Positions may be off by one pixel.\n",
                  szINTR );
        strcpy( szINTR, "CE" );
    }

/* -------------------------------------------------------------------- */
/*      Record the LDEF record number we used so we can find the        */
/*      corresponding RSDF record.                                      */
/* -------------------------------------------------------------------- */
    int         nLDEF_RCID;

    nLDEF_RCID = poRecord->GetIntSubfield( "LDEF", 0, "RCID", 0 );
    
    oLDEF.Close();

/* ==================================================================== */
/*      Search the RSDF module for the requested cell module.           */
/* ==================================================================== */
    DDFModule   oRSDF;

/* -------------------------------------------------------------------- */
/*      Open the RSDF module, and report failure if it is missing.      */
/* -------------------------------------------------------------------- */
    if( poCATD->GetModuleFilePath("RSDF") == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find RSDF entry in CATD module ... "
                  "can't treat as raster.\n" );
        return FALSE;
    }
    
    if( !oRSDF.Open( poCATD->GetModuleFilePath("RSDF") ) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Read each record, till we find what we want.                    */
/* -------------------------------------------------------------------- */
    while( (poRecord = oRSDF.ReadRecord() ) != NULL )
    {
        if( poRecord->GetIntSubfield("LYID",0,"RCID",0) == nLDEF_RCID )
            break;
    }

    if( poRecord == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find LDEF:%d record in RSDF file.\n",
                  nLDEF_RCID );
        return FALSE;
    }
    
/* -------------------------------------------------------------------- */
/*      Establish the raster pixel/line to georef transformation.       */
/* -------------------------------------------------------------------- */
    double      dfZ;
        
    if( poRecord->FindField( "SADR" ) == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find SADR field in RSDF record.\n" );
        return FALSE;
    }
    
    poIREF->GetSADR( poRecord->FindField( "SADR" ), 1,
                     adfTransform + 0, adfTransform + 3, &dfZ );

    adfTransform[1] = poIREF->dfXRes;
    adfTransform[2] = 0.0;
    adfTransform[4] = 0.0;
    adfTransform[5] = -1 * poIREF->dfYRes;

/* -------------------------------------------------------------------- */
/*      If the origin is the center of the pixel, then shift it back    */
/*      half a pixel to the top left of the top left.                   */
/* -------------------------------------------------------------------- */
    if( EQUAL(szINTR,"CE") )
    {
        adfTransform[0] -= adfTransform[1] * 0.5;
        adfTransform[3] -= adfTransform[5] * 0.5;
    }

/* -------------------------------------------------------------------- */
/*      Verify some other assumptions.                                  */
/* -------------------------------------------------------------------- */
    const char  *pszString;
    
    pszString = poRecord->GetStringSubfield( "RSDF", 0, "OBRP", 0); 
    if( !EQUAL(pszString,"G2") )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "OBRP value of `%s' not expected 2D raster code (G2).\n",
                  pszString );
        return FALSE;
    }
    
    pszString = poRecord->GetStringSubfield( "RSDF", 0, "SCOR", 0); 
    if( !EQUAL(pszString,"TL") )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "SCOR (origin) is `%s' instead of expected top left.\n"
                  "Georef coordinates will likely be incorrect.\n",
                  pszString );
    }

    oRSDF.Close();
    
/* -------------------------------------------------------------------- */
/*      For now we will assume that the block size is one scanline.     */
/*      We will blow a gasket later while reading the cell file if      */
/*      this isn't the case.                                            */
/*                                                                      */
/*      This isn't a very flexible raster implementation!               */
/* -------------------------------------------------------------------- */
    nXBlockSize = nXSize;
    nYBlockSize = 1;

/* ==================================================================== */
/*      Fetch the data type used for the raster, and the units from     */
/*      the data dictionary/schema record (DDSH).                       */
/* ==================================================================== */
    DDFModule   oDDSH;

/* -------------------------------------------------------------------- */
/*      Open the DDSH module, and report failure if it is missing.      */
/* -------------------------------------------------------------------- */
    if( poCATD->GetModuleFilePath("DDSH") == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find DDSH entry in CATD module ... "
                  "can't treat as raster.\n" );
        return FALSE;
    }
    
    if( !oDDSH.Open( poCATD->GetModuleFilePath("DDSH") ) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Read each record, till we find what we want.                    */
/* -------------------------------------------------------------------- */
    while( (poRecord = oDDSH.ReadRecord() ) != NULL )
    {
        if( EQUAL(poRecord->GetStringSubfield("DDSH",0,"NAME",0),pszModule) )
            break;
    }

    if( poRecord == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't find DDSH record for %s.\n",
                  pszModule );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Get some values we are interested in.                           */
/* -------------------------------------------------------------------- */
    if( poRecord->GetStringSubfield("DDSH",0,"FMT",0) != NULL )
        strcpy( szFMT, poRecord->GetStringSubfield("DDSH",0,"FMT",0) );
    else
        strcpy( szFMT, "BUI16" );

    if( poRecord->GetStringSubfield("DDSH",0,"UNIT",0) != NULL )
        strcpy( szUNITS, poRecord->GetStringSubfield("DDSH",0,"UNIT",0) );
    else
        strcpy( szUNITS, "METERS" );

    if( poRecord->GetStringSubfield("DDSH",0,"ATLB",0) != NULL )
        strcpy( szLabel, poRecord->GetStringSubfield("DDSH",0,"ATLB",0) );
    else
        strcpy( szLabel, "" );
    
/* -------------------------------------------------------------------- */
/*      Open the cell file.                                             */
/* -------------------------------------------------------------------- */
    return( oDDFModule.Open( poCATD->GetModuleFilePath(pszModule) ) );
}

/************************************************************************/
/*                              GetBlock()                              */
/*                                                                      */
/*      Read a requested block of raster data from the file.            */
/*                                                                      */
/*      Currently we will always use sequential access.  In the         */
/*      future we should modify the iso8211 library to support          */
/*      seeking, and modify this to seek directly to the right          */
/*      record once it's location is known.                             */
/************************************************************************/

/**
  Read a block of raster data from the file.

  @param nXOffset X block offset into the file.  Normally zero for scanline
  organized raster files.

  @param nYOffset Y block offset into the file.  Normally the scanline offset
  from top of raster for scanline organized raster files.

  @param pData pointer to GInt16 (signed short) buffer of data into which to
  read the raster.

  @return TRUE on success and FALSE on error.

  */

int SDTSRasterReader::GetBlock( int nXOffset, int nYOffset, void * pData )

{
    DDFRecord   *poRecord = NULL;
    int         nBytesPerValue;
    int         iTry;
    
    CPLAssert( nXOffset == 0 );

/* -------------------------------------------------------------------- */
/*      Analyse the datatype.                                           */
/* -------------------------------------------------------------------- */
    CPLAssert( EQUAL(szFMT,"BI16") || EQUAL(szFMT,"BFP32") );

    if( EQUAL(szFMT,"BI16") )
        nBytesPerValue = 2;
    else
        nBytesPerValue = 4;

    for(iTry=0;iTry<2;iTry++)
    {
    /* -------------------------------------------------------------------- */
    /*      Read through till we find the desired record.                   */
    /* -------------------------------------------------------------------- */
        CPLErrorReset();
        while( (poRecord = oDDFModule.ReadRecord()) != NULL )
        {
            if( poRecord->GetIntSubfield( "CELL", 0, "ROWI", 0 )
                == nYOffset + nYStart )
            {
                break;
            }
        }

        if( CPLGetLastErrorType() == CE_Failure )
            return FALSE;

    /* -------------------------------------------------------------------- */
    /*      If we didn't get what we needed just start over.                */
    /* -------------------------------------------------------------------- */
        if( poRecord == NULL )
        {
            if (iTry == 0)
                oDDFModule.Rewind();
            else
            {
                CPLError( CE_Failure, CPLE_AppDefined,
                          "Cannot read scanline %d.  Raster access failed.\n",
                          nYOffset );
                return FALSE;
            }
        }
        else
        {
            break;
        }
    }

/* -------------------------------------------------------------------- */
/*      Validate the records size.  Does it represent exactly one       */
/*      scanline?                                                       */
/* -------------------------------------------------------------------- */
    DDFField    *poCVLS;

    poCVLS = poRecord->FindField( "CVLS" );
    if( poCVLS == NULL )
        return FALSE;

    if( poCVLS->GetRepeatCount() != nXSize )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Cell record is %d long, but we expected %d, the number\n"
                  "of pixels in a scanline.  Raster access failed.\n",
                  poCVLS->GetRepeatCount(), nXSize );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Does the CVLS field consist of exactly 1 B(16) field?           */
/* -------------------------------------------------------------------- */
    if( poCVLS->GetDataSize() < nBytesPerValue * nXSize
        || poCVLS->GetDataSize() > nBytesPerValue * nXSize + 1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Cell record is not of expected format.  Raster access "
                  "failed.\n" );

        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Copy the data to the application buffer, and byte swap if       */
/*      required.                                                       */
/* -------------------------------------------------------------------- */
    memcpy( pData, poCVLS->GetData(), nXSize * nBytesPerValue );

#ifdef CPL_LSB
    if( nBytesPerValue == 2 )
    {
        for( int i = 0; i < nXSize; i++ )
        {
            ((GInt16 *) pData)[i] = CPL_MSBWORD16(((GInt16 *) pData)[i]);
        }
    }
    else
    {
        for( int i = 0; i < nXSize; i++ )
        {
            CPL_MSBPTR32( ((GByte *)pData) + i*4 );
        }
    }
#endif
    
    return TRUE;
}

/************************************************************************/
/*                            GetTransform()                            */
/************************************************************************/

/**
  Fetch the transformation between pixel/line coordinates and georeferenced
  coordinates.

  @param padfTransformOut pointer to an array of six doubles which will be
  filled with the georeferencing transform.

  @return TRUE is returned, indicating success.

  The padfTransformOut array consists of six values.  The pixel/line coordinate
  (Xp,Yp) can be related to a georeferenced coordinate (Xg,Yg) or (Easting,
  Northing).

  <pre>
  Xg = padfTransformOut[0] + Xp * padfTransform[1] + Yp * padfTransform[2]
  Yg = padfTransformOut[3] + Xp * padfTransform[4] + Yp * padfTransform[5]
  </pre>

  In other words, for a north up image the top left corner of the top left
  pixel is at georeferenced coordinate (padfTransform[0],padfTransform[3])
  the pixel width is padfTransform[1], the pixel height is padfTransform[5]
  and padfTransform[2] and padfTransform[4] will be zero.

  */

int SDTSRasterReader::GetTransform( double * padfTransformOut )

{
    memcpy( padfTransformOut, adfTransform, sizeof(double)*6 );

    return TRUE;
}

/************************************************************************/
/*                           GetRasterType()                            */
/************************************************************************/

/**
 * Fetch the pixel data type.
 *
 * Returns one of SDTS_RT_INT16 (1) or SDTS_RT_FLOAT32 (6) indicating the
 * type of buffer that should be passed to GetBlock().
 */

int SDTSRasterReader::GetRasterType()

{
    if( EQUAL(szFMT,"BFP32") )
        return 6;
    else
        return 1;
}

/************************************************************************/
/*                             GetMinMax()                              */
/************************************************************************/

/**
 * Fetch the minimum and maximum raster values that occur in the file.
 *
 * Note this operation current results in a scan of the entire file.
 *
 * @param pdfMin variable in which the minimum value encountered is returned.
 * @param pdfMax variable in which the maximum value encountered is returned.
 * @param dfNoData a value to ignore when computing min/max, defaults to
 * -32766.
 *
 * @return TRUE on success, or FALSE if an error occurs.
 */

int SDTSRasterReader::GetMinMax( double * pdfMin, double * pdfMax,
                                 double dfNoData )

{
    void	*pBuffer;
    int		bFirst = TRUE;
    int		b32Bit = GetRasterType() == SDTS_RT_FLOAT32;

    CPLAssert( GetBlockXSize() == GetXSize() && GetBlockYSize() == 1 );
    
    pBuffer = CPLMalloc(sizeof(float) * GetXSize());

    for( int iLine = 0; iLine < GetYSize(); iLine++ )
    {
        if( !GetBlock( 0, iLine, pBuffer ) )
        {
            CPLFree( pBuffer );
            return FALSE;
        }

        for( int iPixel = 0; iPixel < GetXSize(); iPixel++ )
        {
            double	dfValue;
            
            if( b32Bit )
                dfValue = ((float *) pBuffer)[iPixel];
            else
                dfValue = ((short *) pBuffer)[iPixel];

            if( dfValue != dfNoData )
            {
                if( bFirst )
                {
                    *pdfMin = *pdfMax = dfValue;
                    bFirst = FALSE;
                }
                else
                {
                    *pdfMin = MIN(*pdfMin,dfValue);
                    *pdfMax = MAX(*pdfMax,dfValue);
                }
            }
        }
    }

    CPLFree( pBuffer );
    
    return !bFirst;
}


