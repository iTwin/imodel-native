/******************************************************************************
 * $Id: gdal_tps.cpp 16861 2009-04-26 19:22:29Z rouault $
 *
 * Project:  High Performance Image Reprojector
 * Purpose:  Thin Plate Spline transformer (GDAL wrapper portion)
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2004, Frank Warmerdam <warmerdam@pobox.com>
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

#include "thinplatespline.h"
#include "gdal_alg.h"
#include "cpl_conv.h"
#include "cpl_string.h"

CPL_CVSID("$Id: gdal_tps.cpp 16861 2009-04-26 19:22:29Z rouault $");

CPL_C_START
CPLXMLNode *GDALSerializeTPSTransformer( void *pTransformArg );
void *GDALDeserializeTPSTransformer( CPLXMLNode *psTree );
CPL_C_END

typedef struct
{
    GDALTransformerInfo  sTI;

    VizGeorefSpline2D   *poForward;
    VizGeorefSpline2D   *poReverse;

    int       bReversed;

    int       nGCPCount;
    GDAL_GCP *pasGCPList;
    
} TPSTransformInfo;

/************************************************************************/
/*                      GDALCreateTPSTransformer()                      */
/************************************************************************/

/**
 * Create Thin Plate Spline transformer from GCPs.
 *
 * The thin plate spline transformer produces exact transformation
 * at all control points and smoothly varying transformations between
 * control points with greatest influence from local control points. 
 * It is suitable for for many applications not well modelled by polynomial
 * transformations. 
 *
 * Creating the TPS transformer involves solving systems of linear equations
 * related to the number of control points involved.  This solution is
 * computed within this function call.  It can be quite an expensive operation
 * for large numbers of GCPs.  For instance, for reference, it takes on the 
 * order of 10s for 400 GCPs on a 2GHz Athlon processor. 
 *
 * TPS Transformers are serializable. 
 *
 * The GDAL Thin Plate Spline transformer is based on code provided by
 * Gilad Ronnen on behalf of VIZRT Inc (http://www.visrt.com).  Incorporation 
 * of the algorithm into GDAL was supported by the Centro di Ecologia Alpina 
 * (http://www.cealp.it). 
 *
 * @param nGCPCount the number of GCPs in pasGCPList.
 * @param pasGCPList an array of GCPs to be used as input.
 * @Param bReversed 
 * 
 * @return the transform argument or NULL if creation fails. 
 */

void *GDALCreateTPSTransformer( int nGCPCount, const GDAL_GCP *pasGCPList, 
                                int bReversed )

{
    TPSTransformInfo *psInfo;
    int    iGCP;

/* -------------------------------------------------------------------- */
/*      Allocate transform info.                                        */
/* -------------------------------------------------------------------- */
    psInfo = (TPSTransformInfo *) CPLCalloc(sizeof(TPSTransformInfo),1);

    psInfo->pasGCPList = GDALDuplicateGCPs( nGCPCount, pasGCPList );
    psInfo->nGCPCount = nGCPCount;

    psInfo->bReversed = bReversed;
    psInfo->poForward = new VizGeorefSpline2D( 2 );
    psInfo->poReverse = new VizGeorefSpline2D( 2 );

    strcpy( psInfo->sTI.szSignature, "GTI" );
    psInfo->sTI.pszClassName = "GDALTPSTransformer";
    psInfo->sTI.pfnTransform = GDALTPSTransform;
    psInfo->sTI.pfnCleanup = GDALDestroyTPSTransformer;
    psInfo->sTI.pfnSerialize = GDALSerializeTPSTransformer;

/* -------------------------------------------------------------------- */
/*      Attach all the points to the transformation.                    */
/* -------------------------------------------------------------------- */
    for( iGCP = 0; iGCP < nGCPCount; iGCP++ )
    {
        double    afPL[2], afXY[2];

        afPL[0] = pasGCPList[iGCP].dfGCPPixel;
        afPL[1] = pasGCPList[iGCP].dfGCPLine;
        afXY[0] = pasGCPList[iGCP].dfGCPX;
        afXY[1] = pasGCPList[iGCP].dfGCPY;

        if( bReversed )
        {
            psInfo->poReverse->add_point( afPL[0], afPL[1], afXY );
            psInfo->poForward->add_point( afXY[0], afXY[1], afPL );
        }
        else
        {
            psInfo->poForward->add_point( afPL[0], afPL[1], afXY );
            psInfo->poReverse->add_point( afXY[0], afXY[1], afPL );
        }
    }

    psInfo->poForward->solve();
    psInfo->poReverse->solve();

    return psInfo;
}

/************************************************************************/
/*                     GDALDestroyTPSTransformer()                      */
/************************************************************************/

/**
 * Destroy TPS transformer.
 *
 * This function is used to destroy information about a GCP based
 * polynomial transformation created with GDALCreateTPSTransformer(). 
 *
 * @param pTransformArg the transform arg previously returned by 
 * GDALCreateTPSTransformer(). 
 */

void GDALDestroyTPSTransformer( void *pTransformArg )

{
    VALIDATE_POINTER0( pTransformArg, "GDALDestroyTPSTransformer" );

    TPSTransformInfo *psInfo = (TPSTransformInfo *) pTransformArg;

    delete psInfo->poForward;
    delete psInfo->poReverse;

    GDALDeinitGCPs( psInfo->nGCPCount, psInfo->pasGCPList );
    CPLFree( psInfo->pasGCPList );
    
    CPLFree( pTransformArg );
}

/************************************************************************/
/*                          GDALTPSTransform()                          */
/************************************************************************/

/**
 * Transforms point based on GCP derived polynomial model.
 *
 * This function matches the GDALTransformerFunc signature, and can be
 * used to transform one or more points from pixel/line coordinates to
 * georeferenced coordinates (SrcToDst) or vice versa (DstToSrc).
 *
 * @param pTransformArg return value from GDALCreateTPSTransformer(). 
 * @param bDstToSrc TRUE if transformation is from the destination 
 * (georeferenced) coordinates to pixel/line or FALSE when transforming
 * from pixel/line to georeferenced coordinates.
 * @param nPointCount the number of values in the x, y and z arrays.
 * @param x array containing the X values to be transformed.
 * @param y array containing the Y values to be transformed.
 * @param z array containing the Z values to be transformed.
 * @param panSuccess array in which a flag indicating success (TRUE) or
 * failure (FALSE) of the transformation are placed.
 *
 * @return TRUE.
 */

int GDALTPSTransform( void *pTransformArg, int bDstToSrc, 
                      int nPointCount, 
                      double *x, double *y, double *z, 
                      int *panSuccess )

{
    VALIDATE_POINTER1( pTransformArg, "GDALTPSTransform", 0 );

    int    i;
    TPSTransformInfo *psInfo = (TPSTransformInfo *) pTransformArg;

    for( i = 0; i < nPointCount; i++ )
    {
        double xy_out[2];

        if( bDstToSrc )
        {
            psInfo->poReverse->get_point( x[i], y[i], xy_out );
            x[i] = xy_out[0];
            y[i] = xy_out[1];
        }
        else
        {
            psInfo->poForward->get_point( x[i], y[i], xy_out );
            x[i] = xy_out[0];
            y[i] = xy_out[1];
        }
        panSuccess[i] = TRUE;
    }

    return TRUE;
}

/************************************************************************/
/*                    GDALSerializeTPSTransformer()                     */
/************************************************************************/

CPLXMLNode *GDALSerializeTPSTransformer( void *pTransformArg )

{
    VALIDATE_POINTER1( pTransformArg, "GDALSerializeTPSTransformer", NULL );

    CPLXMLNode *psTree;
    TPSTransformInfo *psInfo = static_cast<TPSTransformInfo *>(pTransformArg);

    psTree = CPLCreateXMLNode( NULL, CXT_Element, "TPSTransformer" );

/* -------------------------------------------------------------------- */
/*      Serialize bReversed.                                            */
/* -------------------------------------------------------------------- */
    CPLCreateXMLElementAndValue( 
        psTree, "Reversed", 
        CPLString().Printf( "%d", psInfo->bReversed ) );
                                 
/* -------------------------------------------------------------------- */
/*	Attach GCP List. 						*/
/* -------------------------------------------------------------------- */
    if( psInfo->nGCPCount > 0 )
    {
        int iGCP;
        CPLXMLNode *psGCPList = CPLCreateXMLNode( psTree, CXT_Element, 
                                                  "GCPList" );

        for( iGCP = 0; iGCP < psInfo->nGCPCount; iGCP++ )
        {
            CPLXMLNode *psXMLGCP;
            GDAL_GCP *psGCP = psInfo->pasGCPList + iGCP;

            psXMLGCP = CPLCreateXMLNode( psGCPList, CXT_Element, "GCP" );

            CPLSetXMLValue( psXMLGCP, "#Id", psGCP->pszId );

            if( psGCP->pszInfo != NULL && strlen(psGCP->pszInfo) > 0 )
                CPLSetXMLValue( psXMLGCP, "Info", psGCP->pszInfo );

            CPLSetXMLValue( psXMLGCP, "#Pixel", 
                            CPLString().Printf( "%.4f", psGCP->dfGCPPixel ) );

            CPLSetXMLValue( psXMLGCP, "#Line", 
                            CPLString().Printf( "%.4f", psGCP->dfGCPLine ) );

            CPLSetXMLValue( psXMLGCP, "#X", 
                            CPLString().Printf( "%.12E", psGCP->dfGCPX ) );

            CPLSetXMLValue( psXMLGCP, "#Y", 
                            CPLString().Printf( "%.12E", psGCP->dfGCPY ) );

            if( psGCP->dfGCPZ != 0.0 )
                CPLSetXMLValue( psXMLGCP, "#GCPZ", 
                                CPLString().Printf( "%.12E", psGCP->dfGCPZ ) );
        }
    }

    return psTree;
}

/************************************************************************/
/*                   GDALDeserializeTPSTransformer()                    */
/************************************************************************/

void *GDALDeserializeTPSTransformer( CPLXMLNode *psTree )

{
    GDAL_GCP *pasGCPList = 0;
    int nGCPCount = 0;
    void *pResult;
    int bReversed;

    /* -------------------------------------------------------------------- */
    /*      Check for GCPs.                                                 */
    /* -------------------------------------------------------------------- */
    CPLXMLNode *psGCPList = CPLGetXMLNode( psTree, "GCPList" );

    if( psGCPList != NULL )
    {
        int  nGCPMax = 0;
        CPLXMLNode *psXMLGCP;
         
        // Count GCPs.
        for( psXMLGCP = psGCPList->psChild; psXMLGCP != NULL; 
             psXMLGCP = psXMLGCP->psNext )
            nGCPMax++;
         
        pasGCPList = (GDAL_GCP *) CPLCalloc(sizeof(GDAL_GCP),nGCPMax);

        for( psXMLGCP = psGCPList->psChild; psXMLGCP != NULL; 
             psXMLGCP = psXMLGCP->psNext )
        {
            GDAL_GCP *psGCP = pasGCPList + nGCPCount;

            if( !EQUAL(psXMLGCP->pszValue,"GCP") || 
                psXMLGCP->eType != CXT_Element )
                continue;
             
            GDALInitGCPs( 1, psGCP );
             
            CPLFree( psGCP->pszId );
            psGCP->pszId = CPLStrdup(CPLGetXMLValue(psXMLGCP,"Id",""));
             
            CPLFree( psGCP->pszInfo );
            psGCP->pszInfo = CPLStrdup(CPLGetXMLValue(psXMLGCP,"Info",""));
             
            psGCP->dfGCPPixel = atof(CPLGetXMLValue(psXMLGCP,"Pixel","0.0"));
            psGCP->dfGCPLine = atof(CPLGetXMLValue(psXMLGCP,"Line","0.0"));
             
            psGCP->dfGCPX = atof(CPLGetXMLValue(psXMLGCP,"X","0.0"));
            psGCP->dfGCPY = atof(CPLGetXMLValue(psXMLGCP,"Y","0.0"));
            psGCP->dfGCPZ = atof(CPLGetXMLValue(psXMLGCP,"Z","0.0"));
            nGCPCount++;
        }
    }

/* -------------------------------------------------------------------- */
/*      Get other flags.                                                */
/* -------------------------------------------------------------------- */
    bReversed = atoi(CPLGetXMLValue(psTree,"Reversed","0"));

/* -------------------------------------------------------------------- */
/*      Generate transformation.                                        */
/* -------------------------------------------------------------------- */
    pResult = GDALCreateTPSTransformer( nGCPCount, pasGCPList, bReversed );
    
/* -------------------------------------------------------------------- */
/*      Cleanup GCP copy.                                               */
/* -------------------------------------------------------------------- */
    GDALDeinitGCPs( nGCPCount, pasGCPList );
    CPLFree( pasGCPList );

    return pResult;
}
