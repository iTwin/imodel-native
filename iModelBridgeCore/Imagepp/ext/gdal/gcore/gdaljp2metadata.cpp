/******************************************************************************
 * $Id: gdaljp2metadata.cpp 22650 2011-07-06 00:59:22Z warmerdam $
 *
 * Project:  GDAL 
 * Purpose:  GDALJP2Metadata - Read GeoTIFF and/or GML georef info.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2005, Frank Warmerdam <warmerdam@pobox.com>
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

#include "gdaljp2metadata.h"
#include "cpl_string.h"
#include "cpl_minixml.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"
#include "ogr_api.h"
#include "gt_wkt_srs_for_gdal.h"

CPL_CVSID("$Id: gdaljp2metadata.cpp 22650 2011-07-06 00:59:22Z warmerdam $");

static const unsigned char msi_uuid2[16] =
{0xb1,0x4b,0xf8,0xbd,0x08,0x3d,0x4b,0x43,
 0xa5,0xae,0x8c,0xd7,0xd5,0xa6,0xce,0x03}; 

static const unsigned char msig_uuid[16] = 
{ 0x96,0xA9,0xF1,0xF1,0xDC,0x98,0x40,0x2D,
  0xA7,0xAE,0xD6,0x8E,0x34,0x45,0x18,0x09 };

/************************************************************************/
/*                          GDALJP2Metadata()                           */
/************************************************************************/

GDALJP2Metadata::GDALJP2Metadata()

{
    pszProjection = NULL;

    nGCPCount = 0;
    pasGCPList = NULL;

    papszGMLMetadata = NULL;

    nGeoTIFFSize = 0;
    pabyGeoTIFFData = NULL;

    nMSIGSize = 0;
    pabyMSIGData = NULL;

    bHaveGeoTransform = FALSE;
    adfGeoTransform[0] = 0.0;
    adfGeoTransform[1] = 1.0;
    adfGeoTransform[2] = 0.0;
    adfGeoTransform[3] = 0.0;
    adfGeoTransform[4] = 0.0;
    adfGeoTransform[5] = 1.0;
}

/************************************************************************/
/*                          ~GDALJP2Metadata()                          */
/************************************************************************/

GDALJP2Metadata::~GDALJP2Metadata()

{
    CPLFree( pszProjection );
    if( nGCPCount > 0 )
    {
        GDALDeinitGCPs( nGCPCount, pasGCPList );
        CPLFree( pasGCPList );
    }

    CPLFree( pabyGeoTIFFData );
    CPLFree( pabyMSIGData );
    CSLDestroy( papszGMLMetadata );
}

/************************************************************************/
/*                            ReadAndParse()                            */
/*                                                                      */
/*      Read a JP2 file and try to collect georeferencing               */
/*      information from the various available forms.  Returns TRUE     */
/*      if anything useful is found.                                    */
/************************************************************************/

int GDALJP2Metadata::ReadAndParse( const char *pszFilename )

{
    VSILFILE *fpLL;
        
    fpLL = VSIFOpenL( pszFilename, "rb" );
        
    if( fpLL == NULL )
    {
        CPLDebug( "GDALJP2Metadata", "Could not even open %s.", 
                  pszFilename );

        return FALSE;
    }

    ReadBoxes( fpLL );
    VSIFCloseL( fpLL );
            
/* -------------------------------------------------------------------- */
/*      Try JP2GeoTIFF, GML and finally MSIG to get something.          */
/* -------------------------------------------------------------------- */
    if( !ParseJP2GeoTIFF() && !ParseGMLCoverageDesc() )
        ParseMSIG();

/* -------------------------------------------------------------------- */
/*      If we still don't have a geotransform, look for a world         */
/*      file.                                                           */
/* -------------------------------------------------------------------- */
    if( !bHaveGeoTransform )
    {
        bHaveGeoTransform = 
            GDALReadWorldFile( pszFilename, NULL, adfGeoTransform )
            || GDALReadWorldFile( pszFilename, ".wld", adfGeoTransform );
    }

/* -------------------------------------------------------------------- */
/*      Return success either either of projection or geotransform      */
/*      or gcps.                                                        */
/* -------------------------------------------------------------------- */
    return bHaveGeoTransform
        || nGCPCount > 0 
        || (pszProjection != NULL && strlen(pszProjection) > 0);
}

/************************************************************************/
/*                           CollectGMLData()                           */
/*                                                                      */
/*      Read all the asoc boxes after this node, and store the          */
/*      contain xml documents along with the name from the label.       */
/************************************************************************/

void GDALJP2Metadata::CollectGMLData( GDALJP2Box *poGMLData )

{
    GDALJP2Box oChildBox( poGMLData->GetFILE() );

    oChildBox.ReadFirstChild( poGMLData );

    while( strlen(oChildBox.GetType()) > 0 )
    {
        if( EQUAL(oChildBox.GetType(),"asoc") )
        {
            GDALJP2Box oSubChildBox( oChildBox.GetFILE() );

            char *pszLabel = NULL;
            char *pszXML = NULL;

            oSubChildBox.ReadFirstChild( &oChildBox );
            
            while( strlen(oSubChildBox.GetType()) > 0 )
            {
                if( EQUAL(oSubChildBox.GetType(),"lbl ") )
                    pszLabel = (char *)oSubChildBox.ReadBoxData();
                else if( EQUAL(oSubChildBox.GetType(),"xml ") )
                    pszXML = (char *) oSubChildBox.ReadBoxData();

                oSubChildBox.ReadNextChild( &oChildBox );
            }
            
            if( pszLabel != NULL && pszXML != NULL )
                papszGMLMetadata = CSLSetNameValue( papszGMLMetadata, 
                                                    pszLabel, pszXML );
            CPLFree( pszLabel );
            CPLFree( pszXML );
        }
        
        oChildBox.ReadNextChild( poGMLData );
    }
}

/************************************************************************/
/*                             ReadBoxes()                              */
/************************************************************************/

int GDALJP2Metadata::ReadBoxes( VSILFILE *fpVSIL )

{
    GDALJP2Box oBox( fpVSIL );
    int iBox = 0;

    oBox.ReadFirst(); 

    while( strlen(oBox.GetType()) > 0 )
    {
/* -------------------------------------------------------------------- */
/*      Collect geotiff box.                                            */
/* -------------------------------------------------------------------- */
        if( EQUAL(oBox.GetType(),"uuid") 
            && memcmp( oBox.GetUUID(), msi_uuid2, 16 ) == 0 )
        {
	    nGeoTIFFSize = (int) oBox.GetDataLength();
            pabyGeoTIFFData = oBox.ReadBoxData();
        }

/* -------------------------------------------------------------------- */
/*      Collect MSIG box.                                               */
/* -------------------------------------------------------------------- */
        if( EQUAL(oBox.GetType(),"uuid") 
            && memcmp( oBox.GetUUID(), msig_uuid, 16 ) == 0 )
        {
	    nMSIGSize = (int) oBox.GetDataLength();
            pabyMSIGData = oBox.ReadBoxData();

            if( nMSIGSize < 70 
                || memcmp( pabyMSIGData, "MSIG/", 5 ) != 0 )
            {
                CPLFree( pabyMSIGData );
                pabyMSIGData = NULL;
                nMSIGSize = 0;
            }
        }

/* -------------------------------------------------------------------- */
/*      Process asoc box looking for Labelled GML data.                 */
/* -------------------------------------------------------------------- */
        if( EQUAL(oBox.GetType(),"asoc") )
        {
            GDALJP2Box oSubBox( fpVSIL );

            oSubBox.ReadFirstChild( &oBox );
            if( EQUAL(oSubBox.GetType(),"lbl ") )
            {
                char *pszLabel = (char *) oSubBox.ReadBoxData();
                if( EQUAL(pszLabel,"gml.data") )
                {
                    CollectGMLData( &oBox );
                }
                CPLFree( pszLabel );
            }
        }

/* -------------------------------------------------------------------- */
/*      Process simple xml boxes.                                       */
/* -------------------------------------------------------------------- */
        if( EQUAL(oBox.GetType(),"xml ") )
        {
            CPLString osBoxName;
            char *pszXML = (char *) oBox.ReadBoxData();

            osBoxName.Printf( "BOX_%d", iBox++ );
            
            papszGMLMetadata = CSLSetNameValue( papszGMLMetadata, 
                                                osBoxName, pszXML );
            CPLFree( pszXML );
        }

/* -------------------------------------------------------------------- */
/*      Check for a resd box in jp2h.                                   */
/* -------------------------------------------------------------------- */
        if( EQUAL(oBox.GetType(),"jp2h") )
        {
            GDALJP2Box oSubBox( fpVSIL );

            for( oSubBox.ReadFirstChild( &oBox );
                 strlen(oSubBox.GetType()) > 0;
                 oSubBox.ReadNextChild( &oBox ) )
            {
                if( EQUAL(oSubBox.GetType(),"res ") )
                {
                    GDALJP2Box oResBox( fpVSIL );

                    oResBox.ReadFirstChild( &oSubBox );
                    
                    // we will use either the resd or resc box, which ever
                    // happens to be first.  Should we prefer resd?
                    if( oResBox.GetDataLength() == 10 )
                    {
                        unsigned char *pabyResData = oResBox.ReadBoxData();
                        int nVertNum, nVertDen, nVertExp;
                        int nHorzNum, nHorzDen, nHorzExp;
                        
                        nVertNum = pabyResData[0] * 256 + pabyResData[1];
                        nVertDen = pabyResData[2] * 256 + pabyResData[3];
                        nHorzNum = pabyResData[4] * 256 + pabyResData[5];
                        nHorzDen = pabyResData[6] * 256 + pabyResData[7];
                        nVertExp = pabyResData[8];
                        nHorzExp = pabyResData[9];
                        
                        // compute in pixels/cm 
                        double dfVertRes = 
                            (nVertNum/(double)nVertDen) * pow(10.0,nVertExp)/100;
                        double dfHorzRes = 
                            (nHorzNum/(double)nHorzDen) * pow(10.0,nHorzExp)/100;
                        CPLString osFormatter;

                        papszGMLMetadata = CSLSetNameValue( 
                            papszGMLMetadata, 
                            "TIFFTAG_XRESOLUTION",
                            osFormatter.Printf("%g",dfHorzRes) );
                        
                        papszGMLMetadata = CSLSetNameValue( 
                            papszGMLMetadata, 
                            "TIFFTAG_YRESOLUTION",
                            osFormatter.Printf("%g",dfVertRes) );
                        papszGMLMetadata = CSLSetNameValue( 
                            papszGMLMetadata, 
                            "TIFFTAG_RESOLUTIONUNIT", 
                            "3 (pixels/cm)" );
                        
                        CPLFree( pabyResData );
                    }
                }
            }
        }

        oBox.ReadNext();
    }
    
    return TRUE;
}

/************************************************************************/
/*                          ParseJP2GeoTIFF()                           */
/************************************************************************/

int GDALJP2Metadata::ParseJP2GeoTIFF()

{
    if( nGeoTIFFSize < 1 )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Convert raw data into projection and geotransform.              */
/* -------------------------------------------------------------------- */
    int bSuccess = TRUE;

    if( GTIFWktFromMemBuf( nGeoTIFFSize, pabyGeoTIFFData,
                           &pszProjection, adfGeoTransform,
                           &nGCPCount, &pasGCPList ) != CE_None )
    {
        bSuccess = FALSE;
    }

    if( pszProjection == NULL || strlen(pszProjection) == 0 )
        bSuccess = FALSE;

    if( bSuccess )
        CPLDebug( "GDALJP2Metadata", 
                  "Got projection from GeoJP2 (geotiff) box: %s", 
                 pszProjection );

    if( adfGeoTransform[0] != 0 
        || adfGeoTransform[1] != 1 
        || adfGeoTransform[2] != 0
        || adfGeoTransform[3] != 0 
        || adfGeoTransform[4] != 0
        || adfGeoTransform[5] != 1 )
        bHaveGeoTransform = TRUE;

    return bSuccess;;
}

/************************************************************************/
/*                             ParseMSIG()                              */
/************************************************************************/

int GDALJP2Metadata::ParseMSIG()

{
    if( nMSIGSize < 70 )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Try and extract worldfile parameters and adjust.                */
/* -------------------------------------------------------------------- */
    memcpy( adfGeoTransform + 0, pabyMSIGData + 22 + 8 * 4, 8 );
    memcpy( adfGeoTransform + 1, pabyMSIGData + 22 + 8 * 0, 8 );
    memcpy( adfGeoTransform + 2, pabyMSIGData + 22 + 8 * 2, 8 );
    memcpy( adfGeoTransform + 3, pabyMSIGData + 22 + 8 * 5, 8 );
    memcpy( adfGeoTransform + 4, pabyMSIGData + 22 + 8 * 1, 8 );
    memcpy( adfGeoTransform + 5, pabyMSIGData + 22 + 8 * 3, 8 );

    // data is in LSB (little endian) order in file.
    CPL_LSBPTR64( adfGeoTransform + 0 );
    CPL_LSBPTR64( adfGeoTransform + 1 );
    CPL_LSBPTR64( adfGeoTransform + 2 );
    CPL_LSBPTR64( adfGeoTransform + 3 );
    CPL_LSBPTR64( adfGeoTransform + 4 );
    CPL_LSBPTR64( adfGeoTransform + 5 );

    // correct for center of pixel vs. top left of pixel
    adfGeoTransform[0] -= 0.5 * adfGeoTransform[1];
    adfGeoTransform[0] -= 0.5 * adfGeoTransform[2];
    adfGeoTransform[3] -= 0.5 * adfGeoTransform[4];
    adfGeoTransform[3] -= 0.5 * adfGeoTransform[5];

    bHaveGeoTransform = TRUE;

    return TRUE;
}

/************************************************************************/
/*                         GetDictionaryItem()                          */
/************************************************************************/

static CPLXMLNode *
GetDictionaryItem( char **papszGMLMetadata, const char *pszURN )

{
    char *pszLabel;
    const char *pszFragmentId = NULL;
    int i;


    if( EQUALN(pszURN,"urn:jp2k:xml:", 13) )
        pszLabel = CPLStrdup( pszURN + 13 );
    else if( EQUALN(pszURN,"urn:ogc:tc:gmljp2:xml:", 22) )
        pszLabel = CPLStrdup( pszURN + 22 );
    else if( EQUALN(pszURN,"gmljp2://xml/",13) )
        pszLabel = CPLStrdup( pszURN + 13 );
    else
        pszLabel = CPLStrdup( pszURN );

/* -------------------------------------------------------------------- */
/*      Split out label and fragment id.                                */
/* -------------------------------------------------------------------- */
    for( i = 0; pszLabel[i] != '#'; i++ )
    {
        if( pszLabel[i] == '\0' )
            return NULL;
    }

    pszFragmentId = pszLabel + i + 1;
    pszLabel[i] = '\0';

/* -------------------------------------------------------------------- */
/*      Can we find an XML box with the desired label?                  */
/* -------------------------------------------------------------------- */
    const char *pszDictionary = 
        CSLFetchNameValue( papszGMLMetadata, pszLabel );

    if( pszDictionary == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Try and parse the dictionary.                                   */
/* -------------------------------------------------------------------- */
    CPLXMLNode *psDictTree = CPLParseXMLString( pszDictionary );

    if( psDictTree == NULL )
    {
        CPLDestroyXMLNode( psDictTree );
        return NULL;
    }

    CPLStripXMLNamespace( psDictTree, NULL, TRUE );

    CPLXMLNode *psDictRoot = CPLSearchXMLNode( psDictTree, "=Dictionary" );
    
    if( psDictRoot == NULL )
    {
        CPLDestroyXMLNode( psDictTree );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Search for matching id.                                         */
/* -------------------------------------------------------------------- */
    CPLXMLNode *psEntry, *psHit = NULL;
    for( psEntry = psDictRoot->psChild; 
         psEntry != NULL && psHit == NULL; 
         psEntry = psEntry->psNext )
    {
        const char *pszId;

        if( psEntry->eType != CXT_Element )
            continue;

        if( !EQUAL(psEntry->pszValue,"dictionaryEntry") )
            continue;
        
        if( psEntry->psChild == NULL )
            continue;

        pszId = CPLGetXMLValue( psEntry->psChild, "id", "" );

        if( EQUAL(pszId, pszFragmentId) )
            psHit = CPLCloneXMLTree( psEntry->psChild );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    CPLFree( pszLabel );
    CPLDestroyXMLNode( psDictTree );

    return psHit;
}

        
/************************************************************************/
/*                            GMLSRSLookup()                            */
/*                                                                      */
/*      Lookup an SRS in a dictionary inside this file.  We will get    */
/*      something like:                                                 */
/*        urn:jp2k:xml:CRSDictionary.xml#crs1112                        */
/*                                                                      */
/*      We need to split the filename from the fragment id, and         */
/*      lookup the fragment in the file if we can find it our           */
/*      list of labelled xml boxes.                                     */
/************************************************************************/

int GDALJP2Metadata::GMLSRSLookup( const char *pszURN )

{
    CPLXMLNode *psDictEntry = GetDictionaryItem( papszGMLMetadata, pszURN );

    if( psDictEntry == NULL )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Reserialize this fragment.                                      */
/* -------------------------------------------------------------------- */
    char *pszDictEntryXML = CPLSerializeXMLTree( psDictEntry );
    CPLDestroyXMLNode( psDictEntry );

/* -------------------------------------------------------------------- */
/*      Try to convert into an OGRSpatialReference.                     */
/* -------------------------------------------------------------------- */
    OGRSpatialReference oSRS;
    int bSuccess = FALSE;

    if( oSRS.importFromXML( pszDictEntryXML ) == OGRERR_NONE )
    {
        CPLFree( pszProjection );
        pszProjection = NULL;

        oSRS.exportToWkt( &pszProjection );
        bSuccess = TRUE;
    }

    CPLFree( pszDictEntryXML );

    return bSuccess;
}

/************************************************************************/
/*                        ParseGMLCoverageDesc()                        */
/************************************************************************/

int GDALJP2Metadata::ParseGMLCoverageDesc() 

{
/* -------------------------------------------------------------------- */
/*      Do we have an XML doc that is apparently a coverage             */
/*      description?                                                    */
/* -------------------------------------------------------------------- */
    const char *pszCoverage = CSLFetchNameValue( papszGMLMetadata, 
                                                 "gml.root-instance" );

    if( pszCoverage == NULL )
        return FALSE;

    CPLDebug( "GDALJP2Metadata", "Found GML Box:\n%s", pszCoverage );

/* -------------------------------------------------------------------- */
/*      Try parsing the XML.  Wipe any namespace prefixes.              */
/* -------------------------------------------------------------------- */
    CPLXMLNode *psXML = CPLParseXMLString( pszCoverage );

    if( psXML == NULL )
        return FALSE;

    CPLStripXMLNamespace( psXML, NULL, TRUE );

/* -------------------------------------------------------------------- */
/*      Isolate RectifiedGrid.  Eventually we will need to support      */
/*      other georeferencing objects.                                   */
/* -------------------------------------------------------------------- */
    CPLXMLNode *psRG = CPLSearchXMLNode( psXML, "=RectifiedGrid" );
    CPLXMLNode *psOriginPoint = NULL;
    const char *pszOffset1=NULL, *pszOffset2=NULL;

    if( psRG != NULL )
    {
        psOriginPoint = CPLGetXMLNode( psRG, "origin.Point" );

        
        CPLXMLNode *psOffset1 = CPLGetXMLNode( psRG, "offsetVector" );
        if( psOffset1 != NULL )
        {
            pszOffset1 = CPLGetXMLValue( psOffset1, "", NULL );
            pszOffset2 = CPLGetXMLValue( psOffset1->psNext, "=offsetVector", 
                                         NULL );
        }
    }

/* -------------------------------------------------------------------- */
/*      If we are missing any of the origin or 2 offsets then give up.  */
/* -------------------------------------------------------------------- */
    if( psOriginPoint == NULL || pszOffset1 == NULL || pszOffset2 == NULL )
    {
        CPLDestroyXMLNode( psXML );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Extract origin location.                                        */
/* -------------------------------------------------------------------- */
    OGRPoint *poOriginGeometry = NULL;
    const char *pszSRSName = NULL;

    if( psOriginPoint != NULL )
    {
        poOriginGeometry = (OGRPoint *) 
            OGR_G_CreateFromGMLTree( psOriginPoint );

        if( poOriginGeometry != NULL 
            && wkbFlatten(poOriginGeometry->getGeometryType()) != wkbPoint )
        {
            delete poOriginGeometry;
            poOriginGeometry = NULL;
        }

        // SRS?
        pszSRSName = CPLGetXMLValue( psOriginPoint, "srsName", NULL );
    }

/* -------------------------------------------------------------------- */
/*      Extract offset(s)                                               */
/* -------------------------------------------------------------------- */
    char **papszOffset1Tokens = NULL;
    char **papszOffset2Tokens = NULL;
    int bSuccess = FALSE;

    papszOffset1Tokens = 
        CSLTokenizeStringComplex( pszOffset1, " ,", FALSE, FALSE );
    papszOffset2Tokens = 
        CSLTokenizeStringComplex( pszOffset2, " ,", FALSE, FALSE );

    if( CSLCount(papszOffset1Tokens) >= 2
        && CSLCount(papszOffset2Tokens) >= 2
        && poOriginGeometry != NULL )
    {
        adfGeoTransform[0] = poOriginGeometry->getX();
        adfGeoTransform[1] = atof(papszOffset1Tokens[0]);
        adfGeoTransform[2] = atof(papszOffset2Tokens[0]);
        adfGeoTransform[3] = poOriginGeometry->getY();
        adfGeoTransform[4] = atof(papszOffset1Tokens[1]);
        adfGeoTransform[5] = atof(papszOffset2Tokens[1]);

        // offset from center of pixel.
        adfGeoTransform[0] -= adfGeoTransform[1]*0.5;
        adfGeoTransform[0] -= adfGeoTransform[2]*0.5;
        adfGeoTransform[3] -= adfGeoTransform[4]*0.5;
        adfGeoTransform[3] -= adfGeoTransform[5]*0.5;

        bSuccess = TRUE;
        bHaveGeoTransform = TRUE;
    }

    CSLDestroy( papszOffset1Tokens );
    CSLDestroy( papszOffset2Tokens );

    if( poOriginGeometry != NULL )
        delete poOriginGeometry;

/* -------------------------------------------------------------------- */
/*      If we still don't have an srsName, check for it on the          */
/*      boundedBy Envelope.  Some products                              */
/*      (ie. EuropeRasterTile23.jpx) use this as the only srsName       */
/*      delivery vehicle.                                               */
/* -------------------------------------------------------------------- */
    if( pszSRSName == NULL )
    {
        pszSRSName = 
            CPLGetXMLValue( psXML,
                            "=FeatureCollection.boundedBy.Envelope.srsName",
                            NULL );
    }

/* -------------------------------------------------------------------- */
/*      If we have gotten a geotransform, then try to interprete the    */
/*      srsName.                                                        */
/* -------------------------------------------------------------------- */
    int bNeedAxisFlip = FALSE;

    if( bSuccess && pszSRSName != NULL 
        && (pszProjection == NULL || strlen(pszProjection) == 0) )
    {
        OGRSpatialReference oSRS;

        if( EQUALN(pszSRSName,"epsg:",5) )
        {
            if( oSRS.SetFromUserInput( pszSRSName ) == OGRERR_NONE )
                oSRS.exportToWkt( &pszProjection );
        }
        else if( EQUALN(pszSRSName,"urn:",4) 
                 && strstr(pszSRSName,":def:") != NULL
                 && oSRS.importFromURN(pszSRSName) == OGRERR_NONE )
        {
            const char *pszCode = strrchr(pszSRSName,':') + 1;

            oSRS.exportToWkt( &pszProjection );

            // Per #2131
            if( atoi(pszCode) >= 4000 && atoi(pszCode) <= 4999 )
            {
                CPLDebug( "GMLJP2", "Request axis flip for SRS=%s",
                          pszSRSName );
                bNeedAxisFlip = TRUE;
            }
        }
        else if( !GMLSRSLookup( pszSRSName ) )
        {
            CPLDebug( "GDALJP2Metadata", 
                      "Unable to evaluate SRSName=%s", 
                      pszSRSName );
        }
    }

    if( pszProjection )
        CPLDebug( "GDALJP2Metadata", 
                  "Got projection from GML box: %s", 
                 pszProjection );

    CPLDestroyXMLNode( psXML );
    psXML = NULL;

/* -------------------------------------------------------------------- */
/*      Do we need to flip the axes?                                    */
/* -------------------------------------------------------------------- */
    if( bNeedAxisFlip
        && CSLTestBoolean( CPLGetConfigOption( "GDAL_IGNORE_AXIS_ORIENTATION",
                                               "FALSE" ) ) )
    {
        bNeedAxisFlip = FALSE;
        CPLDebug( "GMLJP2", "Supressed axis flipping based on GDAL_IGNORE_AXIS_ORIENTATION." );
    }

    if( bNeedAxisFlip )
    {
        double dfTemp;

        CPLDebug( "GMLJP2", 
                  "Flipping axis orientation in GMLJP2 coverage description." );

        dfTemp = adfGeoTransform[0];
        adfGeoTransform[0] = adfGeoTransform[3];
        adfGeoTransform[3] = dfTemp;

        dfTemp = adfGeoTransform[1];
        adfGeoTransform[1] = adfGeoTransform[4];
        adfGeoTransform[4] = dfTemp;

        dfTemp = adfGeoTransform[2];
        adfGeoTransform[2] = adfGeoTransform[5];
        adfGeoTransform[5] = dfTemp;
    }

    return pszProjection != NULL && bSuccess;
}

/************************************************************************/
/*                           SetProjection()                            */
/************************************************************************/

void GDALJP2Metadata::SetProjection( const char *pszWKT )

{
    CPLFree( pszProjection );
    pszProjection = CPLStrdup(pszWKT);
}

/************************************************************************/
/*                              SetGCPs()                               */
/************************************************************************/

void GDALJP2Metadata::SetGCPs( int nCount, const GDAL_GCP *pasGCPsIn )

{
    if( nGCPCount > 0 )
    {
        GDALDeinitGCPs( nGCPCount, pasGCPList );
        CPLFree( pasGCPList );
    }

    nGCPCount = nCount;
    pasGCPList = GDALDuplicateGCPs(nGCPCount, pasGCPsIn);
}

/************************************************************************/
/*                          SetGeoTransform()                           */
/************************************************************************/

void GDALJP2Metadata::SetGeoTransform( double *padfGT )

{
    memcpy( adfGeoTransform, padfGT, sizeof(double) * 6 );
}

/************************************************************************/
/*                          CreateJP2GeoTIFF()                          */
/************************************************************************/

GDALJP2Box *GDALJP2Metadata::CreateJP2GeoTIFF()

{
/* -------------------------------------------------------------------- */
/*      Prepare the memory buffer containing the degenerate GeoTIFF     */
/*      file.                                                           */
/* -------------------------------------------------------------------- */
    int         nGTBufSize = 0;
    unsigned char *pabyGTBuf = NULL;

    if( GTIFMemBufFromWkt( pszProjection, adfGeoTransform, 
                           nGCPCount, pasGCPList,
                           &nGTBufSize, &pabyGTBuf ) != CE_None )
        return NULL;

    if( nGTBufSize == 0 )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Write to a box on the JP2 file.                                 */
/* -------------------------------------------------------------------- */
    GDALJP2Box *poBox;

    poBox = GDALJP2Box::CreateUUIDBox( msi_uuid2, nGTBufSize, pabyGTBuf );
    
    CPLFree( pabyGTBuf );

    return poBox;
}

/************************************************************************/
/*                         PrepareCoverageBox()                         */
/************************************************************************/

GDALJP2Box *GDALJP2Metadata::CreateGMLJP2( int nXSize, int nYSize )

{
/* -------------------------------------------------------------------- */
/*      This is a backdoor to let us embed a literal gmljp2 chunk       */
/*      supplied by the user as an external file.  This is mostly       */
/*      for preparing test files with exotic contents.                  */
/* -------------------------------------------------------------------- */
    if( CPLGetConfigOption( "GMLJP2OVERRIDE", NULL ) != NULL )
    {
        VSILFILE *fp = VSIFOpenL( CPLGetConfigOption( "GMLJP2OVERRIDE",""), "r" );
        char *pszGML = NULL;

        if( fp == NULL )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "Unable to open GMLJP2OVERRIDE file." );
            return NULL;
        }
        
        VSIFSeekL( fp, 0, SEEK_END );
        int nLength = (int) VSIFTellL( fp );
        pszGML = (char *) CPLCalloc(1,nLength+1);
        VSIFSeekL( fp, 0, SEEK_SET );
        VSIFReadL( pszGML, 1, nLength, fp );
        VSIFCloseL( fp );

        GDALJP2Box *apoGMLBoxes[2];

        apoGMLBoxes[0] = GDALJP2Box::CreateLblBox( "gml.data" );
        apoGMLBoxes[1] = 
            GDALJP2Box::CreateLabelledXMLAssoc( "gml.root-instance", 
                                                pszGML );

        GDALJP2Box *poGMLData = GDALJP2Box::CreateAsocBox( 2, apoGMLBoxes);
        
        delete apoGMLBoxes[0];
        delete apoGMLBoxes[1];

        CPLFree( pszGML );
        
        return poGMLData;
    }

/* -------------------------------------------------------------------- */
/*      Try do determine a PCS or GCS code we can use.                  */
/* -------------------------------------------------------------------- */
    OGRSpatialReference oSRS;
    char *pszWKTCopy = (char *) pszProjection;
    int nEPSGCode = 0;
    char szSRSName[100];
    int  bNeedAxisFlip = FALSE;

    if( oSRS.importFromWkt( &pszWKTCopy ) != OGRERR_NONE )
        return NULL;

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
            bNeedAxisFlip = TRUE;
        }
    }

    if( nEPSGCode != 0 )
        sprintf( szSRSName, "urn:ogc:def:crs:EPSG::%d", nEPSGCode );
    else
        strcpy( szSRSName, 
                "gmljp2://xml/CRSDictionary.gml#ogrcrs1" );

/* -------------------------------------------------------------------- */
/*      Prepare coverage origin and offset vectors.  Take axis          */
/*      order into account if needed.                                   */
/* -------------------------------------------------------------------- */
    double adfOrigin[2];
    double adfXVector[2];
    double adfYVector[2];
    
    adfOrigin[0] = adfGeoTransform[0] + adfGeoTransform[1] * 0.5
        + adfGeoTransform[4] * 0.5;
    adfOrigin[1] = adfGeoTransform[3] + adfGeoTransform[2] * 0.5
        + adfGeoTransform[5] * 0.5;
    adfXVector[0] = adfGeoTransform[1];
    adfXVector[1] = adfGeoTransform[2];
        
    adfYVector[0] = adfGeoTransform[4];
    adfYVector[1] = adfGeoTransform[5];
    
    if( bNeedAxisFlip
        && CSLTestBoolean( CPLGetConfigOption( "GDAL_IGNORE_AXIS_ORIENTATION",
                                               "FALSE" ) ) )
    {
        bNeedAxisFlip = FALSE;
        CPLDebug( "GMLJP2", "Supressed axis flipping on write based on GDAL_IGNORE_AXIS_ORIENTATION." );
    }

    if( bNeedAxisFlip )
    {
        double dfTemp;
        
        CPLDebug( "GMLJP2", "Flipping GML coverage axis order." );
        
        dfTemp = adfOrigin[0];
        adfOrigin[0] = adfOrigin[1];
        adfOrigin[1] = dfTemp;

        dfTemp = adfXVector[0];
        adfXVector[0] = adfXVector[1];
        adfXVector[1] = dfTemp;

        dfTemp = adfYVector[0];
        adfYVector[0] = adfYVector[1];
        adfYVector[1] = dfTemp;
    }

/* -------------------------------------------------------------------- */
/*      For now we hardcode for a minimal instance format.              */
/* -------------------------------------------------------------------- */
    CPLString osDoc;

    osDoc.Printf( 
"<gml:FeatureCollection\n"
"   xmlns:gml=\"http://www.opengis.net/gml\"\n"
"   xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
"   xsi:schemaLocation=\"http://www.opengeospatial.net/gml http://schemas.opengis.net/gml/3.1.1/profiles/gmlJP2Profile/1.0.0/gmlJP2Profile.xsd\">\n"
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
"              <gml:fileName>gmljp2://codestream/0</gml:fileName>\n"
"              <gml:fileStructure>Record Interleaved</gml:fileStructure>\n"
"            </gml:File>\n"
"          </gml:rangeSet>\n"
"        </gml:RectifiedGridCoverage>\n"
"      </gml:featureMember>\n"
"    </gml:FeatureCollection>\n"
"  </gml:featureMember>\n"
"</gml:FeatureCollection>\n",
             nXSize-1, nYSize-1, szSRSName, adfOrigin[0], adfOrigin[1],
             szSRSName, adfXVector[0], adfXVector[1], 
             szSRSName, adfYVector[0], adfYVector[1] );

/* -------------------------------------------------------------------- */
/*      If we need a user defined CRSDictionary entry, prepare it       */
/*      here.                                                           */
/* -------------------------------------------------------------------- */
    CPLString osDictBox;

    if( nEPSGCode == 0 )
    {
        char *pszGMLDef = NULL;

        if( oSRS.exportToXML( &pszGMLDef, NULL ) == OGRERR_NONE )
        {
            osDictBox.Printf(  
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
/*      Setup the gml.data label.                                       */
/* -------------------------------------------------------------------- */
    GDALJP2Box *apoGMLBoxes[5];
    int nGMLBoxes = 0;

    apoGMLBoxes[nGMLBoxes++] = GDALJP2Box::CreateLblBox( "gml.data" );

/* -------------------------------------------------------------------- */
/*      Setup gml.root-instance.                                        */
/* -------------------------------------------------------------------- */
    apoGMLBoxes[nGMLBoxes++] = 
        GDALJP2Box::CreateLabelledXMLAssoc( "gml.root-instance", osDoc );

/* -------------------------------------------------------------------- */
/*      Add optional dictionary.                                        */
/* -------------------------------------------------------------------- */
    if( strlen(osDictBox) > 0 )
        apoGMLBoxes[nGMLBoxes++] = 
            GDALJP2Box::CreateLabelledXMLAssoc( "CRSDictionary.gml",
                                                osDictBox );
        
/* -------------------------------------------------------------------- */
/*      Bundle gml.data boxes into an association.                      */
/* -------------------------------------------------------------------- */
    GDALJP2Box *poGMLData = GDALJP2Box::CreateAsocBox( nGMLBoxes, apoGMLBoxes);

/* -------------------------------------------------------------------- */
/*      Cleanup working boxes.                                          */
/* -------------------------------------------------------------------- */
    while( nGMLBoxes > 0 )
        delete apoGMLBoxes[--nGMLBoxes];

    return poGMLData;
}


