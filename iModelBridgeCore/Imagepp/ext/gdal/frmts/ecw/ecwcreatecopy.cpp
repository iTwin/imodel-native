/******************************************************************************
 * $Id: ecwcreatecopy.cpp 21492 2011-01-14 04:54:09Z warmerdam $
 *
 * Project:  GDAL ECW Driver
 * Purpose:  ECW CreateCopy method implementation.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, 2004, Frank Warmerdam <warmerdam@pobox.com>
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
#include "ogr_spatialref.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "vsiiostream.h"
#include "jp2userbox.h"
#include "gdaljp2metadata.h"

CPL_CVSID("$Id: ecwcreatecopy.cpp 21492 2011-01-14 04:54:09Z warmerdam $");

void ECWInitialize( void );

GDALDataset* ECWDatasetOpenJPEG2000(GDALOpenInfo* poOpenInfo);

#if defined(FRMT_ecw) && defined(HAVE_COMPRESS)

class GDALECWCompressor : public CNCSFile {

public:
    GDALECWCompressor();
    virtual ~GDALECWCompressor();

    virtual CNCSError WriteReadLine(UINT32 nNextLine, void **ppInputArray);
    virtual void WriteStatus(UINT32 nCurrentLine);
    virtual bool WriteCancel();

    CPLErr  Initialize( const char *pszFilename, char **papszOptions, 
                        int nXSize, int nYSize, int nBands, int bRGBA,
                        GDALDataType eType, 
                        const char *pszWKT, double *padfGeoTransform,
                        int nGCPCount, const GDAL_GCP *pasGCPList,
                        int bIsJPEG2000 );
    CPLErr  CloseDown();

    CPLErr  PrepareCoverageBox( const char *pszWKT, double *padfGeoTransform );
    CPLErr  WriteJP2Box( GDALJP2Box * );

#ifdef ECW_FW
    CNCSJP2File::CNCSJPXAssocBox  m_oGMLAssoc;
#endif

    // Data

    GDALDataset *m_poSrcDS;

    VSIIOStream m_OStream;
    int m_nPercentComplete;

    int m_bCancelled;

    GDALProgressFunc  pfnProgress;
    void             *pProgressData;

    NCSFileViewFileInfoEx sFileInfo;
    GDALDataType eWorkDT;

    JP2UserBox** papoJP2UserBox;
    int          nJP2UserBox;
};

/************************************************************************/
/*                         GDALECWCompressor()                          */
/************************************************************************/

GDALECWCompressor::GDALECWCompressor()

{
    m_poSrcDS = NULL;
    m_nPercentComplete = -1;
    m_bCancelled = FALSE;
    pfnProgress = GDALDummyProgress;
    pProgressData = NULL;
    papoJP2UserBox = NULL;
    nJP2UserBox = 0;
}

/************************************************************************/
/*                         ~GDALECWCompressor()                         */
/************************************************************************/

GDALECWCompressor::~GDALECWCompressor()

{
    int i;
    for(i=0;i<nJP2UserBox;i++)
        delete papoJP2UserBox[i];
    CPLFree(papoJP2UserBox);
}

/************************************************************************/
/*                             CloseDown()                              */
/************************************************************************/

CPLErr GDALECWCompressor::CloseDown()

{
    for( int i = 0; i < sFileInfo.nBands; i++ )
    {
        CPLFree( sFileInfo.pBands[i].szDesc );
    }
    CPLFree( sFileInfo.pBands );

    Close( true );

    return CE_None;
}

/************************************************************************/
/*                           WriteReadLine()                            */
/************************************************************************/

CNCSError GDALECWCompressor::WriteReadLine( UINT32 nNextLine, 
                                            void **ppInputArray )

{
    int    iBand, *panBandMap;
    CPLErr eErr;
    GByte *pabyLineBuf;
    int nWordSize = GDALGetDataTypeSize( eWorkDT ) / 8;

    panBandMap = (int *) CPLMalloc(sizeof(int) * sFileInfo.nBands);
    for( iBand = 0; iBand < sFileInfo.nBands; iBand++ )
        panBandMap[iBand] = iBand+1;

    pabyLineBuf = (GByte *) CPLMalloc( sFileInfo.nSizeX * sFileInfo.nBands
                                       * nWordSize );

    eErr = m_poSrcDS->RasterIO( GF_Read, 0, nNextLine, sFileInfo.nSizeX, 1, 
                                pabyLineBuf, sFileInfo.nSizeX, 1, 
                                eWorkDT, 
                                sFileInfo.nBands, panBandMap,
                                nWordSize, 0, nWordSize * sFileInfo.nSizeX );

    for( iBand = 0; iBand < (int) sFileInfo.nBands; iBand++ )
    {
        memcpy( ppInputArray[iBand],
                pabyLineBuf + nWordSize * sFileInfo.nSizeX * iBand, 
                nWordSize * sFileInfo.nSizeX );
    }

    CPLFree( pabyLineBuf );
    CPLFree( panBandMap );

    if( eErr == CE_None )
        return NCS_SUCCESS;
    else
        return NCS_FILEIO_ERROR;
}

/************************************************************************/
/*                            WriteStatus()                             */
/************************************************************************/

void GDALECWCompressor::WriteStatus( UINT32 nCurrentLine )

{
    m_bCancelled = 
        !pfnProgress( nCurrentLine / (float) sFileInfo.nSizeY, 
                      NULL, pProgressData );
}

/************************************************************************/
/*                            WriteCancel()                             */
/************************************************************************/

bool GDALECWCompressor::WriteCancel()

{
    return (bool) m_bCancelled;
}

/************************************************************************/
/*                         PrepareCoverageBox()                         */
/************************************************************************/

CPLErr  GDALECWCompressor::PrepareCoverageBox( const char *pszWKT, 
                                               double *padfGeoTransform )

{
#ifndef ECW_FW
    return CE_Failure;
#else
/* -------------------------------------------------------------------- */
/*      Try do determine a PCS or GCS code we can use.                  */
/* -------------------------------------------------------------------- */
    OGRSpatialReference oSRS;
    char *pszWKTCopy = (char *) pszWKT;
    int nEPSGCode = 0;
    char szSRSName[100];

    if( oSRS.importFromWkt( &pszWKTCopy ) != OGRERR_NONE )
        return CE_Failure;

    if( oSRS.IsProjected() )
    {
        const char *pszAuthName = oSRS.GetAuthorityName( "PROJCS" );

        if( pszAuthName != NULL && EQUAL(pszAuthName,"epsg") )
        {
            nEPSGCode = atoi(oSRS.GetAuthorityCode( "PROJCS" ));
        }
    }
    else if( oSRS.IsGeographic() )
    {
        const char *pszAuthName = oSRS.GetAuthorityName( "GEOGCS" );

        if( pszAuthName != NULL && EQUAL(pszAuthName,"epsg") )
        {
            nEPSGCode = atoi(oSRS.GetAuthorityCode( "GEOGCS" ));
        }
    }

    if( nEPSGCode != 0 )
        sprintf( szSRSName, "urn:ogc:def:crs:EPSG::%d", nEPSGCode );
    else
        strcpy( szSRSName, 
                "gmljp2://xml/CRSDictionary.gml#ogrcrs1" );

/* -------------------------------------------------------------------- */
/*      For now we hardcode for a minimal instance format.              */
/* -------------------------------------------------------------------- */
    char szDoc[4000];

    sprintf( szDoc, 
"<gml:FeatureCollection\n"
"   xmlns:gml=\"http://www.opengis.net/gml\"\n"
"   xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"   xsi:schemaLocation=\"http://www.opengis.net/gml http://www.math.ubc.ca/~burggraf/gml/gml4jp2.xsd\">\n"
"  <gml:boundedBy>\n"
"    <gml:Null>withheld</gml:Null>\n"
"  </gml:boundedBy>\n"
"  <gml:featureMember>\n"
"    <gml:FeatureCollection>\n"
"      <gml:featureMember>\n"
"        <gml:RectifiedGridCoverage dimension=\"2\" gml:id=\"RGC0001\">\n"
"          <gml:rectifiedGridDomain>\n"
"            <gml:RectifiedGrid dimension=\"2\">\n"
"              <gml:limits>\n"
"                <gml:GridEnvelope>\n"
"                  <gml:low>0 0</gml:low>\n"
"                  <gml:high>%d %d</gml:high>\n"
"                </gml:GridEnvelope>\n"
"              </gml:limits>\n"
"              <gml:axisName>x</gml:axisName>\n"
"              <gml:axisName>y</gml:axisName>\n"
"              <gml:origin>\n"
"                <gml:Point gml:id=\"P0001\" srsName=\"%s\">\n"
"                  <gml:pos>%.15g %.15g</gml:pos>\n"
"                </gml:Point>\n"
"              </gml:origin>\n"
"              <gml:offsetVector srsName=\"%s\">%.15g %.15g</gml:offsetVector>\n"
"              <gml:offsetVector srsName=\"%s\">%.15g %.15g</gml:offsetVector>\n"
"            </gml:RectifiedGrid>\n"
"          </gml:rectifiedGridDomain>\n"
"          <gml:rangeSet>\n"
"            <gml:File>\n"
"              <gml:fileName>urn:ogc:tc:gmljp2:codestream:0</gml:fileName>\n"
"              <gml:fileStructure>Record Interleaved</gml:fileStructure>\n"
"            </gml:File>\n"
"          </gml:rangeSet>\n"
"        </gml:RectifiedGridCoverage>\n"
"      </gml:featureMember>\n"
"    </gml:FeatureCollection>\n"
"  </gml:featureMember>\n"
"</gml:FeatureCollection>\n",
             sFileInfo.nSizeX-1, sFileInfo.nSizeY-1, 
             szSRSName,
             padfGeoTransform[0] + padfGeoTransform[1] * 0.5
                                 + padfGeoTransform[4] * 0.5, 
             padfGeoTransform[3] + padfGeoTransform[2] * 0.5
                                 + padfGeoTransform[5] * 0.5,
             szSRSName, 
             padfGeoTransform[1], padfGeoTransform[2],
             szSRSName,
             padfGeoTransform[4], padfGeoTransform[5] );

/* -------------------------------------------------------------------- */
/*      If we need a user defined CRSDictionary entry, prepare it       */
/*      here.                                                           */
/* -------------------------------------------------------------------- */
    char *pszDictBox = NULL;

    if( nEPSGCode == 0 )
    {
        char *pszGMLDef = NULL;

        if( oSRS.exportToXML( &pszGMLDef, NULL ) == OGRERR_NONE )
        {
            pszDictBox = (char *) CPLMalloc(strlen(pszGMLDef) + 4000);
            
            sprintf( pszDictBox, 
"<gml:Dictionary gml:id=\"CRSU1\" \n"
"        xmlns:gml=\"http://www.opengis.net/gml\"\n"
"        xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
"        xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
"  <gml:dictionaryEntry>\n"
"%s\n"
"  </gml:dictionaryEntry>\n"
"</gml:Dictionary>\n",
                     pszGMLDef );
        }
        CPLFree( pszGMLDef );
    }

/* -------------------------------------------------------------------- */
/*      Setup the various required boxes.                               */
/* -------------------------------------------------------------------- */
    JP2UserBox *poGMLData;
    CNCSJP2File::CNCSJPXAssocBox *poAssoc;
    CNCSJP2File::CNCSJPXLabelBox *poLabel;

    poLabel = new CNCSJP2File::CNCSJPXLabelBox();
    poLabel->SetLabel( "gml.data" );
    poLabel->m_bValid = true;
    m_oGMLAssoc.m_OtherBoxes.push_back( poLabel );
    m_oGMLAssoc.m_OwnedBoxes.push_back( poLabel );
    
    poAssoc = new CNCSJP2File::CNCSJPXAssocBox();
    m_oGMLAssoc.m_OtherBoxes.push_back( poAssoc );
    m_oGMLAssoc.m_OwnedBoxes.push_back( poAssoc );
    poAssoc->m_bValid = true;

    poLabel = new CNCSJP2File::CNCSJPXLabelBox();
    poLabel->SetLabel( "gml.root-instance" );
    poLabel->m_bValid = true;
    poAssoc->m_OtherBoxes.push_back( poLabel );
    poAssoc->m_OwnedBoxes.push_back( poLabel );

    poGMLData = new JP2UserBox();
    poGMLData->m_nTBox = 'xml ';
    poGMLData->SetData( strlen( szDoc ), (unsigned char *) szDoc );
    poAssoc->m_OtherBoxes.push_back( poGMLData );
    poAssoc->m_OwnedBoxes.push_back( poGMLData );

    if( pszDictBox != NULL )
    {
        poAssoc = new CNCSJP2File::CNCSJPXAssocBox();
        m_oGMLAssoc.m_OtherBoxes.push_back( poAssoc );
        m_oGMLAssoc.m_OwnedBoxes.push_back( poAssoc );
        poAssoc->m_bValid = true;
        
        poLabel = new CNCSJP2File::CNCSJPXLabelBox();
        poLabel->SetLabel( "CRSDictionary.gml" );
        poLabel->m_bValid = true;
        poAssoc->m_OtherBoxes.push_back( poLabel );
        poAssoc->m_OwnedBoxes.push_back( poLabel );

        poGMLData = new JP2UserBox();
        poGMLData->m_nTBox = 'xml ';
        poGMLData->SetData( strlen(pszDictBox), 
                            (unsigned char *) pszDictBox );
        poAssoc->m_OtherBoxes.push_back( poGMLData );
        poAssoc->m_OwnedBoxes.push_back( poGMLData );

        CPLFree( pszDictBox );
    }

    m_oGMLAssoc.m_bValid = true;
    AddBox( &m_oGMLAssoc );

    return CE_None;
#endif /* def ECW_FW */
}

/************************************************************************/
/*                            WriteJP2Box()                             */
/************************************************************************/

CPLErr GDALECWCompressor::WriteJP2Box( GDALJP2Box * poBox )

{
    JP2UserBox  *poECWBox;

    if( poBox == NULL )
        return CE_None;

    poECWBox = new JP2UserBox();
    memcpy( &(poECWBox->m_nTBox), poBox->GetType(), 4 );

    poECWBox->SetData( (int) poBox->GetDataLength(), 
                       poBox->GetWritableData() );

    AddBox( poECWBox );

    delete poBox;

    papoJP2UserBox =(JP2UserBox**) CPLRealloc(papoJP2UserBox,
                                    (nJP2UserBox + 1) * sizeof(JP2UserBox*));
    papoJP2UserBox[nJP2UserBox] = poECWBox;
    nJP2UserBox ++;

    return CE_None;
}

/************************************************************************/
/*                        ECWTranslateFromWKT()                         */
/************************************************************************/

static int ECWTranslateFromWKT( const char *pszWKT, 
                                char *pszProjection,
                                int nProjectionLen,
                                char *pszDatum,
                                int nDatumLen)

{
    OGRSpatialReference oSRS;
    char *pszWKTIn = (char *) pszWKT;

    strcpy( pszProjection, "RAW" );
    strcpy( pszDatum, "RAW" );

    if( pszWKT == NULL || strlen(pszWKT) == 0 )
        return FALSE;
    
    oSRS.importFromWkt( &pszWKTIn );
    
    if( oSRS.IsLocal() )
        return TRUE;

/* -------------------------------------------------------------------- */
/*      Do we have an overall EPSG number for this coordinate system?   */
/* -------------------------------------------------------------------- */
    const char *pszAuthorityCode = NULL;
    const char *pszAuthorityName = NULL;
    UINT32 nEPSGCode = 0;

    if( oSRS.IsProjected() )
    {
        pszAuthorityCode =  oSRS.GetAuthorityCode( "PROJCS" );
        pszAuthorityName =  oSRS.GetAuthorityName( "PROJCS" );
    }
    else if( oSRS.IsGeographic() )
    {
        pszAuthorityCode =  oSRS.GetAuthorityCode( "GEOGCS" );
        pszAuthorityName =  oSRS.GetAuthorityName( "GEOGCS" );
    }

    if( pszAuthorityName != NULL && EQUAL(pszAuthorityName,"EPSG") 
        && pszAuthorityCode != NULL && atoi(pszAuthorityCode) > 0 )
        nEPSGCode = (UINT32) atoi(pszAuthorityCode);

    if( nEPSGCode != 0 )
    {
        char *pszEPSGProj = NULL, *pszEPSGDatum = NULL;
        CNCSError oErr;

        oErr = 
            CNCSJP2FileView::GetProjectionAndDatum( atoi(pszAuthorityCode), 
                                                 &pszEPSGProj, &pszEPSGDatum );

        CPLDebug( "ECW", "GetGDTProjDat(%d) = %s/%s", 
                  atoi(pszAuthorityCode), pszEPSGProj, pszEPSGDatum );

        if( oErr.GetErrorNumber() == NCS_SUCCESS
            && pszEPSGProj != NULL && pszEPSGDatum != NULL )
        {
            strncpy( pszProjection, pszEPSGProj, nProjectionLen );
            strncpy( pszDatum, pszEPSGDatum, nDatumLen );
            pszProjection[nProjectionLen - 1] = 0;
            pszDatum[nDatumLen - 1] = 0;
            NCSFree( pszEPSGProj );
            NCSFree( pszEPSGDatum );
            return TRUE;
        }

        NCSFree( pszEPSGProj );
        NCSFree( pszEPSGDatum );

    }

/* -------------------------------------------------------------------- */
/*      Fallback to translating based on the ecw_cs.wkt file, and       */
/*      various jiffy rules.                                            */
/* -------------------------------------------------------------------- */
    char szUnits[32];

    return oSRS.exportToERM( pszProjection, pszDatum, szUnits ) == OGRERR_NONE;
}

/************************************************************************/
/*                             Initialize()                             */
/*                                                                      */
/*      Initialize compressor output.                                   */
/************************************************************************/

CPLErr GDALECWCompressor::Initialize( 
    const char *pszFilename, char **papszOptions, 
    int nXSize, int nYSize, int nBands, int bRGBA,
    GDALDataType eType, 
    const char *pszWKT, double *padfGeoTransform,
    int nGCPCount, const GDAL_GCP *pasGCPList,
    int bIsJPEG2000 )

{
/* -------------------------------------------------------------------- */
/*      Do some rudimentary checking in input.                          */
/* -------------------------------------------------------------------- */
    if( nBands == 0 )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "ECW driver requires at least one band." );
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Parse out some known options.                                   */
/* -------------------------------------------------------------------- */
    float      fTargetCompression;

    // Default compression based on image type per request from Paul Beaty.
    if( nBands > 1 ) 
        fTargetCompression = 95.0;
    else
        fTargetCompression = 90.0;

    if( CSLFetchNameValue(papszOptions, "TARGET") != NULL )
    {
        fTargetCompression = (float) 
            atof(CSLFetchNameValue(papszOptions, "TARGET"));
        
        if( fTargetCompression < 0.0 || fTargetCompression > 99.0 )
        {
            CPLError( CE_Failure, CPLE_NotSupported, 
                      "TARGET compression of %.3f invalid, should be a\n"
                      "value between 0 and 99 percent.\n", 
                      (double) fTargetCompression );
            return CE_Failure;
        }
    }
        
/* -------------------------------------------------------------------- */
/*      Create and initialize compressor.                               */
/* -------------------------------------------------------------------- */
    NCSFileViewFileInfoEx    *psClient = &(sFileInfo);
    
    psClient->nBands = (UINT16) nBands;
    psClient->nSizeX = nXSize;
    psClient->nSizeY = nYSize;
    psClient->nCompressionRate = (UINT16) MAX(1,100/(100-fTargetCompression));
    psClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;

    if( nBands == 1 )
        psClient->eColorSpace = NCSCS_GREYSCALE;
    else if( nBands == 3 )
        psClient->eColorSpace = NCSCS_sRGB;
#if ECWSDK_VERSION >= 40
    else if( nBands == 4 && bRGBA )
        psClient->eColorSpace = NCSCS_sRGB;
#endif
    else
        psClient->eColorSpace = NCSCS_MULTIBAND;

/* -------------------------------------------------------------------- */
/*      Figure out the data type.                                       */
/* -------------------------------------------------------------------- */
    int bSigned = FALSE;
    int nBits = 8;
    eWorkDT = eType;

    switch( eWorkDT  )
    {
        case GDT_Byte:
            psClient->eCellType = NCSCT_UINT8;
            nBits = 8;
            bSigned = FALSE;
            break;
            
        case GDT_UInt16:
            psClient->eCellType = NCSCT_UINT16;
            nBits = 16;
            bSigned = FALSE;
            break;
            
        case GDT_UInt32:
            psClient->eCellType = NCSCT_UINT32;
            nBits = 32;
            bSigned = FALSE;
            break;
            
        case GDT_Int16:
            psClient->eCellType = NCSCT_INT16;
            nBits = 16;
            bSigned = TRUE;
            break;
            
        case GDT_Int32:
            psClient->eCellType = NCSCT_INT32;
            nBits = 32;
            bSigned = TRUE;
            break;
            
        case GDT_Float32:
            psClient->eCellType = NCSCT_IEEE4;
            nBits = 32;
            bSigned = TRUE;
            break;
            
        case GDT_Float64:
            psClient->eCellType = NCSCT_IEEE8;
            nBits = 64;
            bSigned = TRUE;
            break;

        default:
            // We treat complex types as float.  
            psClient->eCellType = NCSCT_IEEE4;
            nBits = 32;
            bSigned = TRUE;
            eWorkDT = GDT_Float32;
            break;
    }

/* -------------------------------------------------------------------- */
/*      Create band information structures.                             */
/* -------------------------------------------------------------------- */
    int iBand;

    psClient->pBands = (NCSFileBandInfo *) 
        CPLMalloc( sizeof(NCSFileBandInfo) * nBands );
    for( iBand = 0; iBand < nBands; iBand++ )
    {
        psClient->pBands[iBand].nBits = (UINT8) nBits;
        psClient->pBands[iBand].bSigned = (BOOLEAN)bSigned;
        psClient->pBands[iBand].szDesc = CPLStrdup(
            CPLSPrintf("Band%d",iBand+1) );
    }

/* -------------------------------------------------------------------- */
/*      Allow CNCSFile::SetParameter() requests.                        */
/* -------------------------------------------------------------------- */
    const char *pszOption;

    if( bIsJPEG2000 )
    {
        pszOption = CSLFetchNameValue(papszOptions, "PROFILE");
        if( pszOption != NULL && EQUAL(pszOption,"BASELINE_0") ) 
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROFILE_BASELINE_0 );
        else if( pszOption != NULL && EQUAL(pszOption,"BASELINE_1") ) 
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROFILE_BASELINE_1 );
        else if( pszOption != NULL && EQUAL(pszOption,"BASELINE_2") )
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROFILE_BASELINE_2 );
        else if( pszOption != NULL && EQUAL(pszOption,"NPJE") ) 
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROFILE_NITF_BIIF_NPJE );
        else if( pszOption != NULL && EQUAL(pszOption,"EPJE") ) 
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROFILE_NITF_BIIF_EPJE );
        
        pszOption = CSLFetchNameValue(papszOptions, "CODESTREAM_ONLY" );
        if( pszOption != NULL ) 
            SetParameter(
                CNCSJP2FileView::JP2_COMPRESS_CODESTREAM_ONLY, 
                (bool) CSLTestBoolean( pszOption ) );
        
        pszOption = CSLFetchNameValue(papszOptions, "LEVELS");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_LEVELS, 
                                      (UINT32) atoi(pszOption) );
        
        pszOption = CSLFetchNameValue(papszOptions, "LAYERS");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_LAYERS, 
                                      (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, "PRECINCT_WIDTH");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_PRECINCT_WIDTH,
                                      (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, "PRECINCT_HEIGHT");
        if( pszOption != NULL )
            SetParameter(CNCSJP2FileView::JP2_COMPRESS_PRECINCT_HEIGHT, 
                                     (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, "TILE_WIDTH");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_TILE_WIDTH, 
                                      (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, "TILE_HEIGHT");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_TILE_HEIGHT, 
                                      (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, "INCLUDE_SOP");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_INCLUDE_SOP, 
                                      (bool) CSLTestBoolean( pszOption ) );
    
        pszOption = CSLFetchNameValue(papszOptions, "INCLUDE_EPH");
        if( pszOption != NULL )
            SetParameter( CNCSJP2FileView::JP2_COMPRESS_INCLUDE_EPH, 
                                      (bool) CSLTestBoolean( pszOption ) );
    
        pszOption = CSLFetchNameValue(papszOptions, "PROGRESSION");
        if( pszOption != NULL && EQUAL(pszOption,"LRCP") )
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROGRESSION_LRCP );
                                  
        else if( pszOption != NULL && EQUAL(pszOption,"RLCP") )
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROGRESSION_RLCP );

        else if( pszOption != NULL && EQUAL(pszOption,"RPCL") )
            SetParameter( 
                CNCSJP2FileView::JP2_COMPRESS_PROGRESSION_RPCL );

        pszOption = CSLFetchNameValue(papszOptions, "GEODATA_USAGE");
        if( pszOption == NULL )
            // Default to supressing ECW SDK geodata, just use our own stuff.
            SetGeodataUsage( JP2_GEODATA_USE_NONE );
        else if( EQUAL(pszOption,"NONE") )
            SetGeodataUsage( JP2_GEODATA_USE_NONE );
        else if( EQUAL(pszOption,"PCS_ONLY") )
            SetGeodataUsage( JP2_GEODATA_USE_PCS_ONLY );
        else if( EQUAL(pszOption,"GML_ONLY") )
            SetGeodataUsage( JP2_GEODATA_USE_GML_ONLY );
        else if( EQUAL(pszOption,"PCS_GML") )
            SetGeodataUsage( JP2_GEODATA_USE_PCS_GML );
        else if( EQUAL(pszOption,"GML_PCS") )
            SetGeodataUsage( JP2_GEODATA_USE_GML_PCS );
        else if( EQUAL(pszOption,"ALL") )
            SetGeodataUsage( JP2_GEODATA_USE_GML_PCS_WLD );

        pszOption = CSLFetchNameValue(papszOptions, "DECOMPRESS_LAYERS");
        if( pszOption != NULL )
            SetParameter( 
                CNCSJP2FileView::JP2_DECOMPRESS_LAYERS, 
                (UINT32) atoi(pszOption) );

        pszOption = CSLFetchNameValue(papszOptions, 
                                      "DECOMPRESS_RECONSTRUCTION_PARAMETER");
        if( pszOption != NULL )
            SetParameter( 
                CNCSJP2FileView::JPC_DECOMPRESS_RECONSTRUCTION_PARAMETER, 
                (IEEE4) atof(pszOption) );
    }
                                  
/* -------------------------------------------------------------------- */
/*      Georeferencing.                                                 */
/* -------------------------------------------------------------------- */

    psClient->fOriginX = 0.0;
    psClient->fOriginY = psClient->nSizeY;
    psClient->fCellIncrementX = 1.0;
    psClient->fCellIncrementY = -1.0;
    psClient->fCWRotationDegrees = 0.0;
    
    if( padfGeoTransform[2] != 0.0 || padfGeoTransform[4] != 0.0 )
        CPLError( CE_Warning, CPLE_NotSupported, 
                  "Rotational coefficients ignored, georeferencing of\n"
                  "output ECW file will be incorrect.\n" );
    else
    {
        psClient->fOriginX = padfGeoTransform[0];
        psClient->fOriginY = padfGeoTransform[3];
        psClient->fCellIncrementX = padfGeoTransform[1];
        psClient->fCellIncrementY = padfGeoTransform[5];
    }

/* -------------------------------------------------------------------- */
/*      Projection.                                                     */
/* -------------------------------------------------------------------- */
    char szProjection[128];
    char szDatum[128];

    strcpy( szProjection, "RAW" );
    strcpy( szDatum, "RAW" );
    
    if( CSLFetchNameValue(papszOptions, "PROJ") != NULL )
    {
        strncpy( szProjection, 
                CSLFetchNameValue(papszOptions, "PROJ"), sizeof(szProjection) );
        szProjection[sizeof(szProjection)-1] = 0;
    }

    if( CSLFetchNameValue(papszOptions, "DATUM") != NULL )
    {
        strncpy( szDatum, CSLFetchNameValue(papszOptions, "DATUM"), sizeof(szDatum) );
        szDatum[sizeof(szDatum)-1] = 0;
        if( EQUAL(szProjection,"RAW") )
            strcpy( szProjection, "GEODETIC" );
    }

    if( EQUAL(szProjection,"RAW") && pszWKT != NULL )
    {
        ECWTranslateFromWKT( pszWKT, szProjection, sizeof(szProjection), szDatum, sizeof(szDatum) );
    }

    psClient->szDatum = szDatum;
    psClient->szProjection = szProjection;

    CPLDebug( "ECW", "Writing with PROJ=%s, DATUM=%s", 
              szProjection, szDatum );

/* -------------------------------------------------------------------- */
/*      Setup GML and GeoTIFF information.                              */
/* -------------------------------------------------------------------- */
    GDALJP2Metadata oJP2MD;

    oJP2MD.SetProjection( pszWKT );
    oJP2MD.SetGeoTransform( padfGeoTransform );
    oJP2MD.SetGCPs( nGCPCount, pasGCPList );

    if( CSLFetchBoolean( papszOptions, "GMLJP2", TRUE ) )
        WriteJP2Box( oJP2MD.CreateGMLJP2(nXSize,nYSize) );
    if( CSLFetchBoolean( papszOptions, "GeoJP2", TRUE ) )
        WriteJP2Box( oJP2MD.CreateJP2GeoTIFF() );

/* -------------------------------------------------------------------- */
/*      Handle special case of a JPEG2000 data stream in another file.  */
/* -------------------------------------------------------------------- */
    VSILFILE *fpVSIL = NULL;

    if( EQUALN(pszFilename,"J2K_SUBFILE:",12) )
    {
        int  subfile_offset=-1, subfile_size=-1;
        const char *real_filename = NULL;

        if( sscanf( pszFilename, "J2K_SUBFILE:%d,%d", 
                    &subfile_offset, &subfile_size ) != 2 )
        {
            CPLError( CE_Failure, CPLE_OpenFailed, 
                      "Failed to parse J2K_SUBFILE specification." );
            return CE_Failure;
        }

        real_filename = strstr(pszFilename,",");
        if( real_filename != NULL )
            real_filename = strstr(real_filename+1,",");
        if( real_filename != NULL )
            real_filename++;
        else
        {
            CPLError( CE_Failure, CPLE_OpenFailed, 
                      "Failed to parse J2K_SUBFILE specification." );
            return CE_Failure;
        }

        fpVSIL = VSIFOpenL( real_filename, "rb+" );
        if( fpVSIL == NULL )
        {
            CPLError( CE_Failure, CPLE_OpenFailed, 
                      "Failed to open %s.",  real_filename );
            return CE_Failure;
        }

        m_OStream.Access( fpVSIL, TRUE, real_filename,
                          subfile_offset, subfile_size );
    }

/* -------------------------------------------------------------------- */
/*      Check if we can enable large files.  This option should only    */
/*      be set when the application is adhering to one of the           */
/*      ERMapper options for licensing larger than 500MB input          */
/*      files.  See Bug 767.  This option no longer exists with         */
/*      version 4+.                                                     */
/* -------------------------------------------------------------------- */
#if ECWSDK_VERSION < 40
    const char *pszLargeOK = CSLFetchNameValue(papszOptions, "LARGE_OK");
    if( pszLargeOK == NULL )
        pszLargeOK = "NO";

    pszLargeOK = CPLGetConfigOption( "ECW_LARGE_OK", pszLargeOK );

    if( CSLTestBoolean(pszLargeOK) )
    {
        CNCSFile::SetKeySize();
        CPLDebug( "ECW", "Large file generation enabled." );
    }
#endif /* ECWSDK_VERSION < 40 */

/* -------------------------------------------------------------------- */
/*      Set the file info.                                              */
/* -------------------------------------------------------------------- */
    CNCSError oError;

    oError = SetFileInfo( sFileInfo );

    if( oError.GetErrorNumber() == NCS_SUCCESS )
    {
        if( fpVSIL == NULL )
            oError = Open( (char *) pszFilename, false, true );
        else
            oError = CNCSJP2FileView::Open( &(m_OStream) );
    }

    if( oError.GetErrorNumber() == NCS_SUCCESS )
        return CE_None;
    else if( oError.GetErrorNumber() == NCS_INPUT_SIZE_EXCEEDED )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "ECW SDK 500MB compress limit exceeded." );
        return CE_Failure;
    }
    else
    {
        char* pszErrorMessage = oError.GetErrorMessage();
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "%s", pszErrorMessage );
        NCSFree(pszErrorMessage);

        return CE_Failure;
    }
}

/************************************************************************/
/*                           ECWCreateCopy()                            */
/************************************************************************/

static GDALDataset *
ECWCreateCopy( const char * pszFilename, GDALDataset *poSrcDS, 
               int bStrict, char ** papszOptions, 
               GDALProgressFunc pfnProgress, void * pProgressData,
               int bIsJPEG2000 )

{
    ECWInitialize();

/* -------------------------------------------------------------------- */
/*      For 4.x and beyond you need a license key to compress data.     */
/*      Check for it as a configuration option or a creation option.    */
/* -------------------------------------------------------------------- */
#if ECWSDK_VERSION >= 40 
    const char* pszECWKey = CSLFetchNameValue( papszOptions, "ECW_ENCODE_KEY");
    if( pszECWKey == NULL )
        pszECWKey = CPLGetConfigOption( "ECW_ENCODE_KEY", NULL );
    
    const char* pszECWCompany = 
        CSLFetchNameValue( papszOptions, "ECW_ENCODE_COMPANY");
    if( pszECWCompany == NULL )
        pszECWCompany = CPLGetConfigOption( "ECW_ENCODE_COMPANY", NULL );
    
    if( pszECWKey && pszECWCompany)
    {
        CPLDebug( "ECW", "SetOEMKey(%s,%s)", pszECWCompany, pszECWKey );
        CNCSFile::SetOEMKey( (char *) pszECWCompany, (char *)pszECWKey );
    }
    else if( pszECWKey || pszECWCompany )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Only one of ECW_ENCODE_KEY and ECW_ENCODE_COMPANY were provided.\nBoth are required." );
        return NULL;
    }

#endif /* ECWSDK_VERSION >= 40

/* -------------------------------------------------------------------- */
/*      Get various values from the source dataset.                     */
/* -------------------------------------------------------------------- */
    int  nBands = poSrcDS->GetRasterCount();
    int  nXSize = poSrcDS->GetRasterXSize();
    int  nYSize = poSrcDS->GetRasterYSize();

    if (nBands == 0)
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "ECW driver does not support source dataset with zero band.\n");
        return NULL;
    }

    GDALDataType eType = poSrcDS->GetRasterBand(1)->GetRasterDataType();

    const char *pszWKT = poSrcDS->GetProjectionRef();
    double adfGeoTransform[6] = { 0, 1, 0, 0, 0, 1 };;

    poSrcDS->GetGeoTransform( adfGeoTransform );

    if( poSrcDS->GetGCPCount() > 0 )
        pszWKT = poSrcDS->GetGCPProjection();

/* -------------------------------------------------------------------- */
/*      Confirm the datatype is 8bit.  It appears no other datatype     */
/*      is supported in ECW format.                                     */
/* -------------------------------------------------------------------- */
    if( eType != GDT_Byte && !bIsJPEG2000 )
    {
        if( bStrict )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Attempt to create ECW file with pixel data type %s failed.\n"
                      "Only Byte data type supported.",
                      GDALGetDataTypeName( eType ) );
        }
        else
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "ECW only supports Byte pixel data type, ignoring request for %s.",
                      GDALGetDataTypeName( eType ) );
            eType = GDT_Byte;
        }
    }

/* -------------------------------------------------------------------- */
/*      Is the input RGBA?                                              */
/* -------------------------------------------------------------------- */
    int bRGBA = FALSE;

    if( nBands == 4 )
    {
        bRGBA = (poSrcDS->GetRasterBand(4)->GetColorInterpretation() 
                 == GCI_AlphaBand);
    }        

/* -------------------------------------------------------------------- */
/*      Setup the compressor.                                           */
/* -------------------------------------------------------------------- */
    GDALECWCompressor         oCompressor;

    oCompressor.pfnProgress = pfnProgress;
    oCompressor.pProgressData = pProgressData;
    oCompressor.m_poSrcDS = poSrcDS;

    if( !pfnProgress( 0.0, NULL, pProgressData ) )
        return NULL;

    if( oCompressor.Initialize( pszFilename, papszOptions, 
                                nXSize, nYSize, nBands, bRGBA,
                                eType, pszWKT, adfGeoTransform, 
                                poSrcDS->GetGCPCount(), 
                                poSrcDS->GetGCPs(),
                                bIsJPEG2000 )
        != CE_None )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Start the compression.                                          */
/* -------------------------------------------------------------------- */
    oCompressor.Write();

/* -------------------------------------------------------------------- */
/*      Cleanup, and return read-only handle.                           */
/* -------------------------------------------------------------------- */
    oCompressor.CloseDown();
    pfnProgress( 1.001, NULL, pProgressData );

/* -------------------------------------------------------------------- */
/*      Re-open dataset, and copy any auxilary pam information.         */
/* -------------------------------------------------------------------- */
    GDALOpenInfo oOpenInfo(pszFilename, GA_ReadOnly);
    GDALPamDataset *poDS;
    
    if (bIsJPEG2000)
        poDS = (GDALPamDataset*) ECWDatasetOpenJPEG2000(&oOpenInfo);
    else
        poDS = (GDALPamDataset*) GDALOpen(pszFilename, GA_ReadOnly);

    if( poDS )
        poDS->CloneInfo( poSrcDS, GCIF_PAM_DEFAULT );

    return poDS;
}

/************************************************************************/
/*                          ECWCreateCopyECW()                          */
/************************************************************************/

GDALDataset *
ECWCreateCopyECW( const char * pszFilename, GDALDataset *poSrcDS, 
                  int bStrict, char ** papszOptions, 
                  GDALProgressFunc pfnProgress, void * pProgressData )

{
    int nBands = poSrcDS->GetRasterCount();
    if (nBands == 0)
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "ECW driver does not support source dataset with zero band.\n");
        return NULL;
    }

    if( !EQUAL(CPLGetExtension(pszFilename),"ecw") )
    {
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "ECW driver does not support creating ECW files\n"
                  "with an extension other than .ecw" );
        return NULL;
    }

    if( poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Byte 
        && bStrict )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "ECW driver doesn't support data type %s. "
                  "Only eight bit bands supported.\n", 
                  GDALGetDataTypeName( 
                      poSrcDS->GetRasterBand(1)->GetRasterDataType()) );

        return NULL;
    }

    if( poSrcDS->GetRasterXSize() < 128 || poSrcDS->GetRasterYSize() < 128 )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "ECW driver requires image to be at least 128x128,\n"
                  "the source image is %dx%d.\n", 
                  poSrcDS->GetRasterXSize(),
                  poSrcDS->GetRasterYSize() );
                  
        return NULL;
    }

    if (poSrcDS->GetRasterBand(1)->GetColorTable() != NULL)
    {
        CPLError( (bStrict) ? CE_Failure : CE_Warning, CPLE_NotSupported, 
                  "ECW driver ignores color table. "
                  "The source raster band will be considered as grey level.\n"
                  "Consider using color table expansion (-expand option in gdal_translate)\n");
        if (bStrict)
            return NULL;
    }

    return ECWCreateCopy( pszFilename, poSrcDS, bStrict, papszOptions, 
                          pfnProgress, pProgressData, FALSE );
}

/************************************************************************/
/*                       ECWCreateCopyJPEG2000()                        */
/************************************************************************/

GDALDataset *
ECWCreateCopyJPEG2000( const char * pszFilename, GDALDataset *poSrcDS, 
                       int bStrict, char ** papszOptions, 
                       GDALProgressFunc pfnProgress, void * pProgressData )

{
    int nBands = poSrcDS->GetRasterCount();
    if (nBands == 0)
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "JP2ECW driver does not support source dataset with zero band.\n");
        return NULL;
    }

    if( EQUAL(CPLGetExtension(pszFilename),"ecw") )
    {
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "JP2ECW driver does not support creating JPEG2000 files\n"
                  "with a .ecw extension.  Please use anything else." );
        return NULL;
    }

    if( poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Byte 
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Byte 
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Int16
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_UInt16
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Int32
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_UInt32
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Float32
        && poSrcDS->GetRasterBand(1)->GetRasterDataType() != GDT_Float64
        && bStrict )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "JP2ECW driver doesn't support data type %s. ",
                  GDALGetDataTypeName( 
                      poSrcDS->GetRasterBand(1)->GetRasterDataType()) );

        return NULL;
    }

    if (poSrcDS->GetRasterBand(1)->GetColorTable() != NULL)
    {
        CPLError( (bStrict) ? CE_Failure : CE_Warning, CPLE_NotSupported, 
                  "JP2ECW driver ignores color table. "
                  "The source raster band will be considered as grey level.\n"
                  "Consider using color table expansion (-expand option in gdal_translate)\n");
        if (bStrict)
            return NULL;
    }

    return ECWCreateCopy( pszFilename, poSrcDS, bStrict, papszOptions, 
                          pfnProgress, pProgressData, TRUE );
}

/************************************************************************/
/************************************************************************
 
               ECW/JPEG200 Create() Support
               ----------------------------

  The remainder of the file is code to implement the Create() method. 
  New dataset and raster band classes are defined specifically for the
  purpose of being write-only.  In particular, you cannot read back data
  from these datasets, and writing must occur in a pretty specific order.
  
  That is, you need to write all metadata (projection, georef, etc) first
  and then write the image data.  All bands data for the first scanline
  should be written followed by all bands for the second scanline and so on.

  Creation supports the same virtual subfile names as CreateCopy() supports. 

 ************************************************************************/
/************************************************************************/

/************************************************************************/
/* ==================================================================== */
/*				ECWWriteDataset				*/
/* ==================================================================== */
/************************************************************************/

class ECWWriteRasterBand;

class CPL_DLL ECWWriteDataset : public GDALDataset
{
    friend class ECWWriteRasterBand;

    char      *pszFilename;

    int       bIsJPEG2000;
    GDALDataType eDataType;
    char    **papszOptions;
  
    char     *pszProjection;
    double    adfGeoTransform[6];

    GDALECWCompressor oCompressor;
    int       bCrystalized;
    
    int       nLoadedLine;
    GByte     *pabyBILBuffer;
    
    CPLErr    Crystalize();
    CPLErr    FlushLine();

  public:
    		ECWWriteDataset( const char *, int, int, int, 
                                 GDALDataType, char **papszOptions,
                                 int );
    		~ECWWriteDataset();

    virtual void   FlushCache( void );

    virtual CPLErr GetGeoTransform( double * );
    virtual const char* GetProjectionRef();
    virtual CPLErr SetGeoTransform( double * );
    virtual CPLErr SetProjection( const char *pszWKT );
};

/************************************************************************/
/* ==================================================================== */
/*                         ECWWriteRasterBand                           */
/* ==================================================================== */
/************************************************************************/
 
class ECWWriteRasterBand : public GDALRasterBand
{
    friend class ECWWriteDataset;
    
    // NOTE: poDS may be altered for NITF/JPEG2000 files!
    ECWWriteDataset     *poGDS;

    GDALColorInterp     eInterp;

  public:

                   ECWWriteRasterBand( ECWWriteDataset *, int );

    virtual CPLErr SetColorInterpretation( GDALColorInterp eInterpIn ) 
        { eInterp = eInterpIn; return CE_None; }
    virtual GDALColorInterp GetColorInterpretation() 
        { return eInterp; }

    virtual CPLErr IReadBlock( int, int, void * );
    virtual CPLErr IWriteBlock( int, int, void * );
};

/************************************************************************/
/*                          ECWWriteDataset()                           */
/************************************************************************/

ECWWriteDataset::ECWWriteDataset( const char *pszFilename, 
                                  int nXSize, int nYSize, int nBandCount, 
                                  GDALDataType eType,
                                  char **papszOptions, int bIsJPEG2000 )

{
    bCrystalized = FALSE;
    pabyBILBuffer = NULL;
    nLoadedLine = -1;

    this->bIsJPEG2000 = bIsJPEG2000;
    this->eDataType = eType;
    this->papszOptions = CSLDuplicate( papszOptions );
    this->pszFilename = CPLStrdup( pszFilename );

    nRasterXSize = nXSize;
    nRasterYSize = nYSize;
    pszProjection = NULL;

    adfGeoTransform[0] = 0.0;
    adfGeoTransform[1] = 1.0;
    adfGeoTransform[2] = 0.0;
    adfGeoTransform[3] = 0.0;
    adfGeoTransform[4] = 0.0;
    adfGeoTransform[5] = 1.0;

    // create band objects.
    for( int iBand = 1; iBand <= nBandCount; iBand++ )
    {
        SetBand( iBand, new ECWWriteRasterBand( this, iBand ) );
    }
}

/************************************************************************/
/*                          ~ECWWriteDataset()                          */
/************************************************************************/

ECWWriteDataset::~ECWWriteDataset()

{
    FlushCache();

    if( bCrystalized )
    {
        if( nLoadedLine == nRasterYSize - 1 )
            FlushLine();
        oCompressor.CloseDown();
    }

    CPLFree( pabyBILBuffer );
    CPLFree( pszProjection );
    CSLDestroy( papszOptions );
    CPLFree( pszFilename );
}

/************************************************************************/
/*                             FlushCache()                             */
/************************************************************************/

void ECWWriteDataset::FlushCache()

{
    BlockBasedFlushCache();
}

/************************************************************************/
/*                         GetProjectionRef()                           */
/************************************************************************/

const char*  ECWWriteDataset::GetProjectionRef()
{
    return pszProjection;
}

/************************************************************************/
/*                          GetGeoTransform()                           */
/************************************************************************/

CPLErr ECWWriteDataset::GetGeoTransform( double *padfGeoTransform )

{
    memcpy( padfGeoTransform, adfGeoTransform, sizeof(double) * 6 );
    return CE_None;
}

/************************************************************************/
/*                          SetGeoTransform()                           */
/************************************************************************/

CPLErr ECWWriteDataset::SetGeoTransform( double *padfGeoTransform )

{
    memcpy( adfGeoTransform, padfGeoTransform, sizeof(double) * 6 );
    return CE_None;
}

/************************************************************************/
/*                           SetProjection()                            */
/************************************************************************/

CPLErr ECWWriteDataset::SetProjection( const char *pszWKT )

{
    CPLFree( pszProjection );
    pszProjection = CPLStrdup( pszWKT );

    return CE_None;
}


/************************************************************************/
/*                             Crystalize()                             */
/************************************************************************/

CPLErr ECWWriteDataset::Crystalize()

{
    int nWordSize = GDALGetDataTypeSize( eDataType ) / 8;

    CPLErr eErr;
    CNCSError oError;

    if( bCrystalized )
        return CE_None;

    eErr = oCompressor.Initialize( pszFilename, papszOptions, 
                                   nRasterXSize, nRasterYSize, nBands, FALSE,
                                   eDataType, 
                                   pszProjection, adfGeoTransform, 
                                   0, NULL,
                                   bIsJPEG2000 );

    if( eErr == CE_None )
        bCrystalized = TRUE;

    nLoadedLine = -1;
    pabyBILBuffer = (GByte *) CPLMalloc( nWordSize * nBands * nRasterXSize );

    return eErr;
}

/************************************************************************/
/*                             FlushLine()                              */
/************************************************************************/

CPLErr ECWWriteDataset::FlushLine()

{
    int nWordSize = GDALGetDataTypeSize( eDataType ) / 8;
    CPLErr eErr;

/* -------------------------------------------------------------------- */
/*      Crystalize if not already done.                                 */
/* -------------------------------------------------------------------- */
    if( !bCrystalized )
    {
        eErr = Crystalize();

        if( eErr != CE_None )
            return eErr;
    }

/* -------------------------------------------------------------------- */
/*      Write out the currently loaded line.                            */
/* -------------------------------------------------------------------- */
    if( nLoadedLine != -1 )
    {
        CNCSError oError;
        void **papOutputLine;

        papOutputLine = (void **) CPLMalloc(sizeof(void*) * nBands);
        for( int i = 0; i < nBands; i++ )
            papOutputLine[i] = 
                (void *) (pabyBILBuffer + i * nWordSize * nRasterXSize);
        

        oError = oCompressor.WriteLineBIL( oCompressor.sFileInfo.eCellType, 
                                           (UINT16) nBands, papOutputLine );
        CPLFree( papOutputLine );
        if( oError.GetErrorNumber() != NCS_SUCCESS )
        {
            char* pszErrorMessage = oError.GetErrorMessage();
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Scanline write write failed.\n%s",
                      pszErrorMessage );
            NCSFree(pszErrorMessage);

            return CE_Failure;
        }
    }

/* -------------------------------------------------------------------- */
/*      Clear the buffer and increment the "current line" indicator.    */
/* -------------------------------------------------------------------- */
    memset( pabyBILBuffer, 0, nWordSize * nRasterXSize * nBands );
    nLoadedLine++;

    return CE_None;
}


/************************************************************************/
/* ==================================================================== */
/*                          ECWWriteRasterBand                          */
/* ==================================================================== */
/************************************************************************/

/************************************************************************/
/*                         ECWWriteRasterBand()                         */
/************************************************************************/

ECWWriteRasterBand::ECWWriteRasterBand( ECWWriteDataset *poDSIn,
                                        int nBandIn )

{
    nBand = nBandIn;
    poDS = poDSIn;
    poGDS = poDSIn;
    nBlockXSize = poDSIn->GetRasterXSize();
    nBlockYSize = 1;
    eDataType = poDSIn->eDataType;
    eInterp = GCI_Undefined;
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr ECWWriteRasterBand::IReadBlock( int nBlockX, int nBlockY, 
                                       void *pBuffer )

{
    int nWordSize = GDALGetDataTypeSize( eDataType ) / 8;

    // We zero stuff out here, but we can't really read stuff from
    // a write only stream. 

    memset( pBuffer, 0, nBlockXSize * nWordSize );

    return CE_None;
}

/************************************************************************/
/*                            IWriteBlock()                             */
/************************************************************************/

CPLErr ECWWriteRasterBand::IWriteBlock( int nBlockX, int nBlockY, 
                                        void *pBuffer )

{
    int nWordSize = GDALGetDataTypeSize( eDataType ) / 8;
    CPLErr eErr;

/* -------------------------------------------------------------------- */
/*      Flush previous line if needed.                                  */
/* -------------------------------------------------------------------- */
    if( nBlockY == poGDS->nLoadedLine + 1 )
    {
        eErr = poGDS->FlushLine();
        if( eErr != CE_None )
            return eErr;
    }

/* -------------------------------------------------------------------- */
/*      Blow a gasket if we have been asked to write something out      */
/*      of order.                                                       */
/* -------------------------------------------------------------------- */
    if( nBlockY != poGDS->nLoadedLine )
    {
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "Apparent attempt to write to ECW non-sequentially.\n"
                  "Loaded line is %d, but %d of band %d was written to.",
                  poGDS->nLoadedLine, nBlockY, nBand );
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Copy passed data into current line buffer.                      */
/* -------------------------------------------------------------------- */
    memcpy( poGDS->pabyBILBuffer + (nBand-1) * nWordSize * nRasterXSize, 
            pBuffer, 
            nWordSize * nRasterXSize );

    return CE_None;
}

/************************************************************************/
/*                         ECWCreateJPEG2000()                          */
/************************************************************************/

GDALDataset *
ECWCreateJPEG2000(const char *pszFilename, int nXSize, int nYSize, int nBands, 
                  GDALDataType eType, char **papszOptions )

{
    ECWInitialize();

    return new ECWWriteDataset( pszFilename, nXSize, nYSize, nBands, 
                                eType, papszOptions, TRUE );
}

/************************************************************************/
/*                            ECWCreateECW()                            */
/************************************************************************/

GDALDataset *
ECWCreateECW( const char *pszFilename, int nXSize, int nYSize, int nBands, 
              GDALDataType eType, char **papszOptions )

{
    ECWInitialize();

    return new ECWWriteDataset( pszFilename, nXSize, nYSize, nBands, 
                                eType, papszOptions, FALSE );
}

#endif /* def FRMT_ecw && def HAVE_COMPRESS */
