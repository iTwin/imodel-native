/******************************************************************************
 * $Id: ogr2gmlgeometry.cpp 21023 2010-10-30 19:48:23Z rouault $
 *
 * Project:  GML Translator
 * Purpose:  Code to translate OGRGeometry to GML string representation.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2002, Frank Warmerdam <warmerdam@pobox.com>
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************
 *
 * Independent Security Audit 2003/04/17 Andrey Kiselev:
 *   Completed audit of this module. All functions may be used without buffer
 *   overflows and stack corruptions if caller could be trusted.
 *
 * Security Audit 2003/03/28 warmerda:
 *   Completed security audit.  I believe that this module may be safely used 
 *   to generate GML from arbitrary but well formed OGRGeomety objects that
 *   come from a potentially hostile source, but through a trusted OGR importer
 *   without compromising the system.
 *
 */

#include "cpl_minixml.h"
#include "ogr_geometry.h"
#include "ogr_api.h"
#include "ogr_p.h"
#include "cpl_error.h"
#include "cpl_conv.h"

/************************************************************************/
/*                        MakeGMLCoordinate()                           */
/************************************************************************/

static void MakeGMLCoordinate( char *pszTarget, 
                               double x, double y, double z, int b3D )

{
    OGRMakeWktCoordinate( pszTarget, x, y, z, b3D ? 3 : 2 );
    while( *pszTarget != '\0' )
    {
        if( *pszTarget == ' ' )
            *pszTarget = ',';
        pszTarget++;
    }

#ifdef notdef
    if( !b3D )
    {
        if( x == (int) x && y == (int) y )
            sprintf( pszTarget, "%d,%d", (int) x, (int) y );
        else if( fabs(x) < 370 && fabs(y) < 370 )
            sprintf( pszTarget, "%.16g,%.16g", x, y );
        else if( fabs(x) > 100000000.0 || fabs(y) > 100000000.0 )
            sprintf( pszTarget, "%.16g,%.16g", x, y );
        else
            sprintf( pszTarget, "%.3f,%.3f", x, y );
    }
    else
    {
        if( x == (int) x && y == (int) y && z == (int) z )
            sprintf( pszTarget, "%d,%d,%d", (int) x, (int) y, (int) z );
        else if( fabs(x) < 370 && fabs(y) < 370 )
            sprintf( pszTarget, "%.16g,%.16g,%.16g", x, y, z );
        else if( fabs(x) > 100000000.0 || fabs(y) > 100000000.0 
                 || fabs(z) > 100000000.0 )
            sprintf( pszTarget, "%.16g,%.16g,%.16g", x, y, z );
        else
            sprintf( pszTarget, "%.3f,%.3f,%.3f", x, y, z );
    }
#endif
}

/************************************************************************/
/*                            _GrowBuffer()                             */
/************************************************************************/

static void _GrowBuffer( int nNeeded, char **ppszText, int *pnMaxLength )

{
    if( nNeeded+1 >= *pnMaxLength )
    {
        *pnMaxLength = MAX(*pnMaxLength * 2,nNeeded+1);
        *ppszText = (char *) CPLRealloc(*ppszText, *pnMaxLength);
    }
}

/************************************************************************/
/*                            AppendString()                            */
/************************************************************************/

static void AppendString( char **ppszText, int *pnLength, int *pnMaxLength,
                          const char *pszTextToAppend )

{
    _GrowBuffer( *pnLength + strlen(pszTextToAppend) + 1, 
                 ppszText, pnMaxLength );

    strcat( *ppszText + *pnLength, pszTextToAppend );
    *pnLength += strlen( *ppszText + *pnLength );
}


/************************************************************************/
/*                        AppendCoordinateList()                        */
/************************************************************************/

static void AppendCoordinateList( OGRLineString *poLine, 
                                  char **ppszText, int *pnLength, 
                                  int *pnMaxLength )

{
    char        szCoordinate[256];
    int         b3D = (poLine->getGeometryType() & wkb25DBit);

    *pnLength += strlen(*ppszText + *pnLength);
    _GrowBuffer( *pnLength + 20, ppszText, pnMaxLength );

    strcat( *ppszText + *pnLength, "<gml:coordinates>" );
    *pnLength += strlen(*ppszText + *pnLength);

    
    for( int iPoint = 0; iPoint < poLine->getNumPoints(); iPoint++ )
    {
        MakeGMLCoordinate( szCoordinate, 
                           poLine->getX(iPoint),
                           poLine->getY(iPoint),
                           poLine->getZ(iPoint),
                           b3D );
        _GrowBuffer( *pnLength + strlen(szCoordinate)+1, 
                     ppszText, pnMaxLength );

        if( iPoint != 0 )
            strcat( *ppszText + *pnLength, " " );
            
        strcat( *ppszText + *pnLength, szCoordinate );
        *pnLength += strlen(*ppszText + *pnLength);
    }
    
    _GrowBuffer( *pnLength + 20, ppszText, pnMaxLength );
    strcat( *ppszText + *pnLength, "</gml:coordinates>" );
    *pnLength += strlen(*ppszText + *pnLength);
}

/************************************************************************/
/*                       OGR2GMLGeometryAppend()                        */
/************************************************************************/

static int OGR2GMLGeometryAppend( OGRGeometry *poGeometry, 
                                  char **ppszText, int *pnLength, 
                                  int *pnMaxLength,
                                  int bIsSubGeometry )

{

/* -------------------------------------------------------------------- */
/*      Check for Spatial Reference System attached to given geometry   */
/* -------------------------------------------------------------------- */

    // Buffer for srsName attribute (srsName="...")
    char szSrsName[30] = { 0 }; 
    int nSrsNameLength = 0;

    const OGRSpatialReference* poSRS = NULL;
    poSRS = poGeometry->getSpatialReference();

    if( NULL != poSRS && !bIsSubGeometry )
    {
        const char* pszAuthName = NULL;
        const char* pszAuthCode = NULL;
        const char* pszTarget = NULL;

        if (poSRS->IsProjected())
            pszTarget = "PROJCS";
        else
            pszTarget = "GEOGCS";

        pszAuthName = poSRS->GetAuthorityName( pszTarget );
        if( NULL != pszAuthName )
        {
            if( EQUAL( pszAuthName, "EPSG" ) )
            {
                pszAuthCode = poSRS->GetAuthorityCode( pszTarget );
                if( NULL != pszAuthCode && strlen(pszAuthCode) < 10 )
                {
                    sprintf( szSrsName, " srsName=\"%s:%s\"",
                            pszAuthName, pszAuthCode );

                    nSrsNameLength = strlen(szSrsName);
                }
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      2D Point                                                        */
/* -------------------------------------------------------------------- */
    if( poGeometry->getGeometryType() == wkbPoint )
    {
        char    szCoordinate[256];
        OGRPoint *poPoint = (OGRPoint *) poGeometry;

        MakeGMLCoordinate( szCoordinate, 
                           poPoint->getX(), poPoint->getY(), 0.0, FALSE );

        _GrowBuffer( *pnLength + strlen(szCoordinate) + 60 + nSrsNameLength, 
                     ppszText, pnMaxLength );

        sprintf( *ppszText + *pnLength, 
                "<gml:Point%s><gml:coordinates>%s</gml:coordinates></gml:Point>",
                 szSrsName, szCoordinate );

        *pnLength += strlen( *ppszText + *pnLength );
    }
/* -------------------------------------------------------------------- */
/*      3D Point                                                        */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbPoint25D )
    {
        char    szCoordinate[256];
        OGRPoint *poPoint = (OGRPoint *) poGeometry;

        MakeGMLCoordinate( szCoordinate, 
                           poPoint->getX(), poPoint->getY(), poPoint->getZ(), 
                           TRUE );
                           
        _GrowBuffer( *pnLength + strlen(szCoordinate) + 70 + nSrsNameLength, 
                     ppszText, pnMaxLength );

        sprintf( *ppszText + *pnLength, 
                "<gml:Point%s><gml:coordinates>%s</gml:coordinates></gml:Point>",
                 szSrsName, szCoordinate );

        *pnLength += strlen( *ppszText + *pnLength );
    }

/* -------------------------------------------------------------------- */
/*      LineString and LinearRing                                       */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbLineString 
             || poGeometry->getGeometryType() == wkbLineString25D )
    {
        int bRing = EQUAL(poGeometry->getGeometryName(),"LINEARRING");

        // Buffer for tag name + srsName attribute if set
        const size_t nLineTagLength = 16;
        char* pszLineTagName = NULL;
        pszLineTagName = (char *) CPLMalloc( nLineTagLength + nSrsNameLength + 1 );

        if( bRing )
        {
            sprintf( pszLineTagName, "<gml:LinearRing%s>", szSrsName );

            AppendString( ppszText, pnLength, pnMaxLength,
                          pszLineTagName );
        }
        else
        {
            sprintf( pszLineTagName, "<gml:LineString%s>", szSrsName );

            AppendString( ppszText, pnLength, pnMaxLength,
                          pszLineTagName );
        }

        // FREE TAG BUFFER
        CPLFree( pszLineTagName );

        AppendCoordinateList( (OGRLineString *) poGeometry, 
                              ppszText, pnLength, pnMaxLength );
        
        if( bRing )
            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:LinearRing>" );
        else
            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:LineString>" );
    }

/* -------------------------------------------------------------------- */
/*      Polygon                                                         */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbPolygon 
             || poGeometry->getGeometryType() == wkbPolygon25D )
    {
        OGRPolygon      *poPolygon = (OGRPolygon *) poGeometry;

        // Buffer for polygon tag name + srsName attribute if set
        const size_t nPolyTagLength = 13;
        char* pszPolyTagName = NULL;
        pszPolyTagName = (char *) CPLMalloc( nPolyTagLength + nSrsNameLength + 1 );

        // Compose Polygon tag with or without srsName attribute
        sprintf( pszPolyTagName, "<gml:Polygon%s>", szSrsName );

        AppendString( ppszText, pnLength, pnMaxLength,
                      pszPolyTagName );

        // FREE TAG BUFFER
        CPLFree( pszPolyTagName );

        // Don't add srsName to polygon rings

        if( poPolygon->getExteriorRing() != NULL )
        {
            AppendString( ppszText, pnLength, pnMaxLength,
                          "<gml:outerBoundaryIs>" );

            if( !OGR2GMLGeometryAppend( poPolygon->getExteriorRing(), 
                                        ppszText, pnLength, pnMaxLength,
                                        TRUE ) )
            {
                return FALSE;
            }
            
            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:outerBoundaryIs>" );
        }

        for( int iRing = 0; iRing < poPolygon->getNumInteriorRings(); iRing++ )
        {
            OGRLinearRing *poRing = poPolygon->getInteriorRing(iRing);

            AppendString( ppszText, pnLength, pnMaxLength,
                          "<gml:innerBoundaryIs>" );
            
            if( !OGR2GMLGeometryAppend( poRing, ppszText, pnLength, 
                                        pnMaxLength, TRUE ) )
                return FALSE;
            
            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:innerBoundaryIs>" );
        }

        AppendString( ppszText, pnLength, pnMaxLength,
                      "</gml:Polygon>" );
    }

/* -------------------------------------------------------------------- */
/*      MultiPolygon, MultiLineString, MultiPoint, MultiGeometry        */
/* -------------------------------------------------------------------- */
    else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon 
             || wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString
             || wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint
             || wkbFlatten(poGeometry->getGeometryType()) == wkbGeometryCollection )
    {
        OGRGeometryCollection *poGC = (OGRGeometryCollection *) poGeometry;
        int             iMember;
        const char *pszElemClose = NULL;
        const char *pszMemberElem = NULL;

        // Buffer for opening tag + srsName attribute
        char* pszElemOpen = NULL;

        if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon )
        {
            pszElemOpen = (char *) CPLMalloc( 13 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiPolygon%s>", szSrsName );

            pszElemClose = "MultiPolygon>";
            pszMemberElem = "polygonMember>";
        }
        else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString )
        {
            pszElemOpen = (char *) CPLMalloc( 16 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiLineString%s>", szSrsName );

            pszElemClose = "MultiLineString>";
            pszMemberElem = "lineStringMember>";
        }
        else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint )
        {
            pszElemOpen = (char *) CPLMalloc( 11 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiPoint%s>", szSrsName );

            pszElemClose = "MultiPoint>";
            pszMemberElem = "pointMember>";
        }
        else
        {
            pszElemOpen = (char *) CPLMalloc( 19 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiGeometry%s>", szSrsName );

            pszElemClose = "MultiGeometry>";
            pszMemberElem = "geometryMember>";
        }

        AppendString( ppszText, pnLength, pnMaxLength, "<gml:" );
        AppendString( ppszText, pnLength, pnMaxLength, pszElemOpen );

        for( iMember = 0; iMember < poGC->getNumGeometries(); iMember++)
        {
            OGRGeometry *poMember = poGC->getGeometryRef( iMember );

            AppendString( ppszText, pnLength, pnMaxLength, "<gml:" );
            AppendString( ppszText, pnLength, pnMaxLength, pszMemberElem );
            
            if( !OGR2GMLGeometryAppend( poMember, 
                                        ppszText, pnLength, pnMaxLength,
                                        TRUE ) )
            {
                return FALSE;
            }
            
            AppendString( ppszText, pnLength, pnMaxLength, "</gml:" );
            AppendString( ppszText, pnLength, pnMaxLength, pszMemberElem );
        }

        AppendString( ppszText, pnLength, pnMaxLength, "</gml:" );
        AppendString( ppszText, pnLength, pnMaxLength, pszElemClose );

        // FREE TAG BUFFER
        CPLFree( pszElemOpen );
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                   OGR_G_ExportEnvelopeToGMLTree()                    */
/*                                                                      */
/*      Export the envelope of a geometry as a gml:Box.                 */
/************************************************************************/

CPLXMLNode *OGR_G_ExportEnvelopeToGMLTree( OGRGeometryH hGeometry )

{
    CPLXMLNode *psBox, *psCoord;
    OGREnvelope sEnvelope;
    char        szCoordinate[256];
    char       *pszY;

    memset( &sEnvelope, 0, sizeof(sEnvelope) );
    ((OGRGeometry *) hGeometry)->getEnvelope( &sEnvelope );

    if( sEnvelope.MinX == 0 && sEnvelope.MaxX == 0 
        && sEnvelope.MaxX == 0 && sEnvelope.MaxY == 0 )
    {
        /* there is apparently a special way of representing a null box
           geometry ... we should use it here eventually. */

        return NULL;
    }

    psBox = CPLCreateXMLNode( NULL, CXT_Element, "gml:Box" );

/* -------------------------------------------------------------------- */
/*      Add minxy coordinate.                                           */
/* -------------------------------------------------------------------- */
    psCoord = CPLCreateXMLNode( psBox, CXT_Element, "gml:coord" );
    
    MakeGMLCoordinate( szCoordinate, sEnvelope.MinX, sEnvelope.MinY, 0.0, 
                       FALSE );
    pszY = strstr(szCoordinate,",") + 1;
    pszY[-1] = '\0';

    CPLCreateXMLElementAndValue( psCoord, "gml:X", szCoordinate );
    CPLCreateXMLElementAndValue( psCoord, "gml:Y", pszY );

/* -------------------------------------------------------------------- */
/*      Add maxxy coordinate.                                           */
/* -------------------------------------------------------------------- */
    psCoord = CPLCreateXMLNode( psBox, CXT_Element, "gml:coord" );
    
    MakeGMLCoordinate( szCoordinate, sEnvelope.MaxX, sEnvelope.MaxY, 0.0, 
                       FALSE );
    pszY = strstr(szCoordinate,",") + 1;
    pszY[-1] = '\0';

    CPLCreateXMLElementAndValue( psCoord, "gml:X", szCoordinate );
    CPLCreateXMLElementAndValue( psCoord, "gml:Y", pszY );

    return psBox;
}


/************************************************************************/
/*                     AppendGML3CoordinateList()                       */
/************************************************************************/

static void AppendGML3CoordinateList( OGRLineString *poLine, int bCoordSwap,
                                  char **ppszText, int *pnLength,
                                  int *pnMaxLength )

{
    char        szCoordinate[256];
    int         b3D = (poLine->getGeometryType() & wkb25DBit);

    *pnLength += strlen(*ppszText + *pnLength);
    _GrowBuffer( *pnLength + 40, ppszText, pnMaxLength );

    if (b3D)
        strcat( *ppszText + *pnLength, "<gml:posList srsDimension=\"3\">" );
    else
        strcat( *ppszText + *pnLength, "<gml:posList>" );
    *pnLength += strlen(*ppszText + *pnLength);


    for( int iPoint = 0; iPoint < poLine->getNumPoints(); iPoint++ )
    {
        if (bCoordSwap)
            OGRMakeWktCoordinate( szCoordinate,
                           poLine->getY(iPoint),
                           poLine->getX(iPoint),
                           poLine->getZ(iPoint),
                           b3D ? 3 : 2 );
        else
            OGRMakeWktCoordinate( szCoordinate,
                           poLine->getX(iPoint),
                           poLine->getY(iPoint),
                           poLine->getZ(iPoint),
                           b3D ? 3 : 2 );
        _GrowBuffer( *pnLength + strlen(szCoordinate)+1,
                     ppszText, pnMaxLength );

        if( iPoint != 0 )
            strcat( *ppszText + *pnLength, " " );

        strcat( *ppszText + *pnLength, szCoordinate );
        *pnLength += strlen(*ppszText + *pnLength);
    }

    _GrowBuffer( *pnLength + 20, ppszText, pnMaxLength );
    strcat( *ppszText + *pnLength, "</gml:posList>" );
    *pnLength += strlen(*ppszText + *pnLength);
}

/************************************************************************/
/*                      OGR2GML3GeometryAppend()                        */
/************************************************************************/

static int OGR2GML3GeometryAppend( OGRGeometry *poGeometry,
                                   const OGRSpatialReference* poParentSRS,
                                  char **ppszText, int *pnLength,
                                  int *pnMaxLength,
                                  int bIsSubGeometry,
                                  int bLongSRS,
                                  int bLineStringAsCurve)

{

/* -------------------------------------------------------------------- */
/*      Check for Spatial Reference System attached to given geometry   */
/* -------------------------------------------------------------------- */

    // Buffer for srsName attribute (srsName="...")
    char szSrsName[50] = { 0 };
    int nSrsNameLength = 0;

    const OGRSpatialReference* poSRS = NULL;
    if (poParentSRS)
        poSRS = poParentSRS;
    else
        poParentSRS = poSRS = poGeometry->getSpatialReference();

    int bCoordSwap = FALSE;

    if( NULL != poSRS )
    {
        const char* pszAuthName = NULL;
        const char* pszAuthCode = NULL;
        const char* pszTarget = NULL;

        if (poSRS->IsProjected())
            pszTarget = "PROJCS";
        else
            pszTarget = "GEOGCS";

        pszAuthName = poSRS->GetAuthorityName( pszTarget );
        if( NULL != pszAuthName )
        {
            if( EQUAL( pszAuthName, "EPSG" ) )
            {
                pszAuthCode = poSRS->GetAuthorityCode( pszTarget );
                if( NULL != pszAuthCode && strlen(pszAuthCode) < 10 )
                {
                    if (bLongSRS && !((OGRSpatialReference*)poSRS)->EPSGTreatsAsLatLong())
                    {
                        OGRSpatialReference oSRS;
                        if (oSRS.importFromEPSGA(atoi(pszAuthCode)) == OGRERR_NONE)
                        {
                            if (oSRS.EPSGTreatsAsLatLong())
                                bCoordSwap = TRUE;
                        }
                    }

                    if (!bIsSubGeometry)
                    {
                        if (bLongSRS)
                        {
                            sprintf( szSrsName, " srsName=\"urn:ogc:def:crs:%s::%s\"",
                                pszAuthName, pszAuthCode );
                        }
                        else
                        {
                            sprintf( szSrsName, " srsName=\"%s:%s\"",
                                    pszAuthName, pszAuthCode );
                        }

                        nSrsNameLength = strlen(szSrsName);
                    }
                }
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      2D Point                                                        */
/* -------------------------------------------------------------------- */
    if( poGeometry->getGeometryType() == wkbPoint )
    {
        char    szCoordinate[256];
        OGRPoint *poPoint = (OGRPoint *) poGeometry;

        if (bCoordSwap)
            OGRMakeWktCoordinate( szCoordinate,
                           poPoint->getY(), poPoint->getX(), 0.0, 2 );
        else
            OGRMakeWktCoordinate( szCoordinate,
                           poPoint->getX(), poPoint->getY(), 0.0, 2 );

        _GrowBuffer( *pnLength + strlen(szCoordinate) + 60 + nSrsNameLength,
                     ppszText, pnMaxLength );

        sprintf( *ppszText + *pnLength,
                "<gml:Point%s><gml:pos>%s</gml:pos></gml:Point>",
                 szSrsName, szCoordinate );

        *pnLength += strlen( *ppszText + *pnLength );
    }
/* -------------------------------------------------------------------- */
/*      3D Point                                                        */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbPoint25D )
    {
        char    szCoordinate[256];
        OGRPoint *poPoint = (OGRPoint *) poGeometry;

        if (bCoordSwap)
            OGRMakeWktCoordinate( szCoordinate,
                           poPoint->getY(), poPoint->getX(), poPoint->getZ(), 3 );
        else
            OGRMakeWktCoordinate( szCoordinate,
                           poPoint->getX(), poPoint->getY(), poPoint->getZ(), 3 );

        _GrowBuffer( *pnLength + strlen(szCoordinate) + 70 + nSrsNameLength,
                     ppszText, pnMaxLength );

        sprintf( *ppszText + *pnLength,
                "<gml:Point%s><gml:pos>%s</gml:pos></gml:Point>",
                 szSrsName, szCoordinate );

        *pnLength += strlen( *ppszText + *pnLength );
    }

/* -------------------------------------------------------------------- */
/*      LineString and LinearRing                                       */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbLineString
             || poGeometry->getGeometryType() == wkbLineString25D )
    {
        int bRing = EQUAL(poGeometry->getGeometryName(),"LINEARRING");
        if (!bRing && bLineStringAsCurve)
        {
            AppendString( ppszText, pnLength, pnMaxLength,
                            "<gml:Curve" );
            AppendString( ppszText, pnLength, pnMaxLength,
                            szSrsName );
            AppendString( ppszText, pnLength, pnMaxLength,
                            "><gml:segments><gml:LineStringSegment>" );
            AppendGML3CoordinateList( (OGRLineString *) poGeometry, bCoordSwap,
                                ppszText, pnLength, pnMaxLength );
            AppendString( ppszText, pnLength, pnMaxLength,
                            "</gml:LineStringSegment></gml:segments></gml:Curve>" );
        }
        else
        {
            // Buffer for tag name + srsName attribute if set
            const size_t nLineTagLength = 16;
            char* pszLineTagName = NULL;
            pszLineTagName = (char *) CPLMalloc( nLineTagLength + nSrsNameLength + 1 );

            if( bRing )
            {
                /* LinearRing isn't supposed to have srsName attribute according to GML3 SF-0 */
                AppendString( ppszText, pnLength, pnMaxLength,
                            "<gml:LinearRing>" );
            }
            else
            {
                sprintf( pszLineTagName, "<gml:LineString%s>", szSrsName );

                AppendString( ppszText, pnLength, pnMaxLength,
                            pszLineTagName );
            }

            // FREE TAG BUFFER
            CPLFree( pszLineTagName );

            AppendGML3CoordinateList( (OGRLineString *) poGeometry, bCoordSwap,
                                ppszText, pnLength, pnMaxLength );

            if( bRing )
                AppendString( ppszText, pnLength, pnMaxLength,
                            "</gml:LinearRing>" );
            else
                AppendString( ppszText, pnLength, pnMaxLength,
                            "</gml:LineString>" );
        }
    }

/* -------------------------------------------------------------------- */
/*      Polygon                                                         */
/* -------------------------------------------------------------------- */
    else if( poGeometry->getGeometryType() == wkbPolygon
             || poGeometry->getGeometryType() == wkbPolygon25D )
    {
        OGRPolygon      *poPolygon = (OGRPolygon *) poGeometry;

        // Buffer for polygon tag name + srsName attribute if set
        const size_t nPolyTagLength = 13;
        char* pszPolyTagName = NULL;
        pszPolyTagName = (char *) CPLMalloc( nPolyTagLength + nSrsNameLength + 1 );

        // Compose Polygon tag with or without srsName attribute
        sprintf( pszPolyTagName, "<gml:Polygon%s>", szSrsName );

        AppendString( ppszText, pnLength, pnMaxLength,
                      pszPolyTagName );

        // FREE TAG BUFFER
        CPLFree( pszPolyTagName );

        // Don't add srsName to polygon rings

        if( poPolygon->getExteriorRing() != NULL )
        {
            AppendString( ppszText, pnLength, pnMaxLength,
                          "<gml:exterior>" );

            if( !OGR2GML3GeometryAppend( poPolygon->getExteriorRing(), poSRS,
                                        ppszText, pnLength, pnMaxLength,
                                        TRUE, bLongSRS, bLineStringAsCurve ) )
            {
                return FALSE;
            }

            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:exterior>" );
        }

        for( int iRing = 0; iRing < poPolygon->getNumInteriorRings(); iRing++ )
        {
            OGRLinearRing *poRing = poPolygon->getInteriorRing(iRing);

            AppendString( ppszText, pnLength, pnMaxLength,
                          "<gml:interior>" );

            if( !OGR2GML3GeometryAppend( poRing, poSRS, ppszText, pnLength,
                                        pnMaxLength, TRUE, bLongSRS, bLineStringAsCurve ) )
                return FALSE;

            AppendString( ppszText, pnLength, pnMaxLength,
                          "</gml:interior>" );
        }

        AppendString( ppszText, pnLength, pnMaxLength,
                      "</gml:Polygon>" );
    }

/* -------------------------------------------------------------------- */
/*      MultiPolygon, MultiLineString, MultiPoint, MultiGeometry        */
/* -------------------------------------------------------------------- */
    else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon
             || wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString
             || wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint
             || wkbFlatten(poGeometry->getGeometryType()) == wkbGeometryCollection )
    {
        OGRGeometryCollection *poGC = (OGRGeometryCollection *) poGeometry;
        int             iMember;
        const char *pszElemClose = NULL;
        const char *pszMemberElem = NULL;

        // Buffer for opening tag + srsName attribute
        char* pszElemOpen = NULL;

        if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon )
        {
            pszElemOpen = (char *) CPLMalloc( 13 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiSurface%s>", szSrsName );

            pszElemClose = "MultiSurface>";
            pszMemberElem = "surfaceMember>";
        }
        else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiLineString )
        {
            pszElemOpen = (char *) CPLMalloc( 16 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiCurve%s>", szSrsName );

            pszElemClose = "MultiCurve>";
            pszMemberElem = "curveMember>";
        }
        else if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPoint )
        {
            pszElemOpen = (char *) CPLMalloc( 11 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiPoint%s>", szSrsName );

            pszElemClose = "MultiPoint>";
            pszMemberElem = "pointMember>";
        }
        else
        {
            pszElemOpen = (char *) CPLMalloc( 19 + nSrsNameLength + 1 );
            sprintf( pszElemOpen, "MultiGeometry%s>", szSrsName );

            pszElemClose = "MultiGeometry>";
            pszMemberElem = "geometryMember>";
        }

        AppendString( ppszText, pnLength, pnMaxLength, "<gml:" );
        AppendString( ppszText, pnLength, pnMaxLength, pszElemOpen );

        for( iMember = 0; iMember < poGC->getNumGeometries(); iMember++)
        {
            OGRGeometry *poMember = poGC->getGeometryRef( iMember );

            AppendString( ppszText, pnLength, pnMaxLength, "<gml:" );
            AppendString( ppszText, pnLength, pnMaxLength, pszMemberElem );

            if( !OGR2GML3GeometryAppend( poMember, poSRS,
                                        ppszText, pnLength, pnMaxLength,
                                        TRUE, bLongSRS, bLineStringAsCurve ) )
            {
                return FALSE;
            }

            AppendString( ppszText, pnLength, pnMaxLength, "</gml:" );
            AppendString( ppszText, pnLength, pnMaxLength, pszMemberElem );
        }

        AppendString( ppszText, pnLength, pnMaxLength, "</gml:" );
        AppendString( ppszText, pnLength, pnMaxLength, pszElemClose );

        // FREE TAG BUFFER
        CPLFree( pszElemOpen );
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/*                       OGR_G_ExportToGMLTree()                        */
/************************************************************************/

CPLXMLNode *OGR_G_ExportToGMLTree( OGRGeometryH hGeometry )

{
    char        *pszText;
    CPLXMLNode  *psTree;

    pszText = OGR_G_ExportToGML( hGeometry );
    if( pszText == NULL )
        return NULL;

    psTree = CPLParseXMLString( pszText );

    CPLFree( pszText );

    return psTree;
}

/************************************************************************/
/*                         OGR_G_ExportToGML()                          */
/************************************************************************/

/**
 * \brief Convert a geometry into GML format.
 *
 * The GML geometry is expressed directly in terms of GML basic data
 * types assuming the this is available in the gml namespace.  The returned
 * string should be freed with CPLFree() when no longer required.
 *
 * This method is the same as the C++ method OGRGeometry::exportToGML().
 *
 * @param hGeometry handle to the geometry.
 * @return A GML fragment or NULL in case of error.
 */

char *OGR_G_ExportToGML( OGRGeometryH hGeometry )

{
    return OGR_G_ExportToGMLEx(hGeometry, NULL);
}

/************************************************************************/
/*                        OGR_G_ExportToGMLEx()                         */
/************************************************************************/

/**
 * \brief Convert a geometry into GML format.
 *
 * The GML geometry is expressed directly in terms of GML basic data
 * types assuming the this is available in the gml namespace.  The returned
 * string should be freed with CPLFree() when no longer required.
 *
 * The supported options in OGR 1.8.0 are :
 * <ul>
 * <li> FORMAT=GML3. Otherwise it will default to GML 2.1.2 output.
 * <li> GML3_LINESTRING_ELEMENT=curve. (Only valid for FORMAT=GML3) To use gml:Curve element for linestrings.
 *     Otherwise gml:LineString will be used .
 * <li> GML3_LONGSRS=YES/NO. (Only valid for FORMAT=GML3) Default to YES. If YES, SRS with EPSG authority will
 *      be written with the "urn:ogc:def:crs:EPSG::" prefix.
 *      In the case, if the SRS is a geographic SRS without explicit AXIS order, but that the same SRS authority code
 *      imported with ImportFromEPSGA() should be treated as lat/long, then the function will take care of coordinate order swapping.
 *      If set to NO, SRS with EPSG authority will be written with the "EPSG:" prefix, even if they are in lat/long order.
 * </ul>
 *
 * This method is the same as the C++ method OGRGeometry::exportToGML().
 *
 * @param hGeometry handle to the geometry.
 * @param papszOptions NULL-terminated list of options.
 * @return A GML fragment or NULL in case of error.
 *
 * @since OGR 1.8.0
 */

char *OGR_G_ExportToGMLEx( OGRGeometryH hGeometry, char** papszOptions )

{
    char        *pszText;
    int         nLength = 0, nMaxLength = 1;

    if( hGeometry == NULL )
        return CPLStrdup( "" );

    pszText = (char *) CPLMalloc(nMaxLength);
    pszText[0] = '\0';

    const char* pszFormat = CSLFetchNameValue(papszOptions, "FORMAT");
    if (pszFormat && EQUAL(pszFormat, "GML3"))
    {
        const char* pszLineStringElement = CSLFetchNameValue(papszOptions, "GML3_LINESTRING_ELEMENT");
        int bLineStringAsCurve = (pszLineStringElement && EQUAL(pszLineStringElement, "curve"));
        int bLongSRS = CSLTestBoolean(CSLFetchNameValueDef(papszOptions, "GML3_LONGSRS", "YES"));
        if( !OGR2GML3GeometryAppend( (OGRGeometry *) hGeometry, NULL, &pszText,
                                    &nLength, &nMaxLength, FALSE, bLongSRS, bLineStringAsCurve ))
        {
            CPLFree( pszText );
            return NULL;
        }
        else
            return pszText;
    }

    if( !OGR2GMLGeometryAppend( (OGRGeometry *) hGeometry, &pszText,
                                &nLength, &nMaxLength, FALSE ))
    {
        CPLFree( pszText );
        return NULL;
    }
    else
        return pszText;
}
