/******************************************************************************
 * $Id: ogr_srs_ozi.cpp 21264 2010-12-14 22:55:01Z rouault $
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  OGRSpatialReference translation from OziExplorer
 *           georeferencing information.
 * Author:   Andrey Kiselev, dron@ak4719.spb.edu
 *
 ******************************************************************************
 * Copyright (c) 2009, Andrey Kiselev <dron@ak4719.spb.edu>
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

#include "ogr_spatialref.h"
#include "cpl_conv.h"
#include "cpl_csv.h"

CPL_CVSID("$Id: ogr_srs_ozi.cpp 21264 2010-12-14 22:55:01Z rouault $");

/************************************************************************/
/*  Correspondence between Ozi and EPSG datum codes.                    */
/************************************************************************/

typedef struct 
{
    const char  *pszOziDatum;
    int         nEPSGCode;
} OZIDatums;

static const OZIDatums aoDatums[] =
{
    { "WGS 72", 4322 },             // WGS, 1972
    { "WGS 84", 4326 },             // WGS, 1984
    { "Pulkovo 1942 (1)", 4284 },   // Pulkovo 1942
    { "Pulkovo 1942 (2)", 4284 },   // Pulkovo 1942, XXX: What is a difference
                                    // with the previous one?
    { NULL, 0 }
};

/************************************************************************/
/*                          OSRImportFromOzi()                          */
/************************************************************************/

OGRErr OSRImportFromOzi( OGRSpatialReferenceH hSRS,
                         const char *pszDatum, const char *pszProj,
                         const char *pszProjParms )

{
    return ((OGRSpatialReference *) hSRS)->importFromOzi( pszDatum, pszProj,
                                                          pszProjParms );
}

/************************************************************************/
/*                            importFromOzi()                           */
/************************************************************************/

/**
 * Import coordinate system from OziExplorer projection definition.
 *
 * This method will import projection definition in style, used by
 * OziExplorer software.
 *
 * This function is the equivalent of the C function OSRImportFromOzi().
 *
 * @param pszDatum Datum string. This is a fifth string in the
 * OziExplorer .MAP file.
 *
 * @param pszProj Projection string. Search for line starting with
 * "Map Projection" name in the OziExplorer .MAP file and supply it as a
 * whole in this parameter.
 *
 * @param pszProjParms String containing projection parameters. Search for
 * "Projection Setup" name in the OziExplorer .MAP file and supply it as a
 * whole in this parameter.
 * 
 * @return OGRERR_NONE on success or an error code in case of failure. 
 */

OGRErr OGRSpatialReference::importFromOzi( const char *pszDatum,
                                           const char *pszProj,
                                           const char *pszProjParms )

{
    Clear();

/* -------------------------------------------------------------------- */
/*      Operate on the basis of the projection name.                    */
/* -------------------------------------------------------------------- */
    char    **papszProj = CSLTokenizeStringComplex( pszProj, ",", TRUE, TRUE );
    char    **papszProjParms = CSLTokenizeStringComplex( pszProjParms, ",", 
                                                         TRUE, TRUE );
    char    **papszDatum = NULL;
                                                         
    if (CSLCount(papszProj) < 2)
    {
        goto not_enough_data;
    }

    if ( EQUALN(papszProj[1], "Latitude/Longitude", 18) )
    {
    }

    else if ( EQUALN(papszProj[1], "Mercator", 8) )
    {
        if (CSLCount(papszProjParms) < 6) goto not_enough_data;
        double dfScale = CPLAtof(papszProjParms[3]);
        if (papszProjParms[3][0] == 0) dfScale = 1; /* if unset, default to scale = 1 */
        SetMercator( CPLAtof(papszProjParms[1]), CPLAtof(papszProjParms[2]),
                     dfScale,
                     CPLAtof(papszProjParms[4]), CPLAtof(papszProjParms[5]) );
    }

    else if ( EQUALN(papszProj[1], "Transverse Mercator", 19) )
    {
        if (CSLCount(papszProjParms) < 6) goto not_enough_data;
        SetTM( CPLAtof(papszProjParms[1]), CPLAtof(papszProjParms[2]),
               CPLAtof(papszProjParms[3]),
               CPLAtof(papszProjParms[4]), CPLAtof(papszProjParms[5]) );
    }

    else if ( EQUALN(papszProj[1], "Lambert Conformal Conic", 23) )
    {
        if (CSLCount(papszProjParms) < 8) goto not_enough_data;
        SetLCC( CPLAtof(papszProjParms[6]), CPLAtof(papszProjParms[7]),
                CPLAtof(papszProjParms[1]), CPLAtof(papszProjParms[2]),
                CPLAtof(papszProjParms[4]), CPLAtof(papszProjParms[5]) );
    }

    else if ( EQUALN(papszProj[1], "Sinusoidal", 10) )
    {
        if (CSLCount(papszProjParms) < 6) goto not_enough_data;
        SetSinusoidal( CPLAtof(papszProjParms[2]),
                       CPLAtof(papszProjParms[4]), CPLAtof(papszProjParms[5]) );
    }

    else if ( EQUALN(papszProj[1], "Albers Equal Area", 17) )
    {
        if (CSLCount(papszProjParms) < 8) goto not_enough_data;
        SetACEA( CPLAtof(papszProjParms[6]), CPLAtof(papszProjParms[7]),
                 CPLAtof(papszProjParms[1]), CPLAtof(papszProjParms[2]),
                 CPLAtof(papszProjParms[4]), CPLAtof(papszProjParms[5]) );
    }

    else
    {
        CPLDebug( "OSR_Ozi", "Unsupported projection: \"%s\"", papszProj[1] );
        SetLocalCS( CPLString().Printf("\"Ozi\" projection \"%s\"",
                                       papszProj[1]) );
    }

/* -------------------------------------------------------------------- */
/*      Try to translate the datum/spheroid.                            */
/* -------------------------------------------------------------------- */
    papszDatum = CSLTokenizeString2( pszDatum, ",",
                                               CSLT_ALLOWEMPTYTOKENS
                                               | CSLT_STRIPLEADSPACES
                                               | CSLT_STRIPENDSPACES );
    if ( papszDatum == NULL)
        goto not_enough_data;
        
    if ( !IsLocal() )
    {
        const OZIDatums   *paoDatum = aoDatums;

        // Search for matching datum
        while ( paoDatum->pszOziDatum )
        {
            if ( EQUAL( papszDatum[0], paoDatum->pszOziDatum ) )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( paoDatum->nEPSGCode );
                CopyGeogCSFrom( &oGCS );
                break;
            }
            paoDatum++;
        }

        if ( !paoDatum->pszOziDatum )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Wrong datum name \"%s\". Setting WGS84 as a fallback.",
                      papszDatum[0] );
            SetWellKnownGeogCS( "WGS84" );
        }
    }

/* -------------------------------------------------------------------- */
/*      Grid units translation                                          */
/* -------------------------------------------------------------------- */
    if( IsLocal() || IsProjected() )
        SetLinearUnits( SRS_UL_METER, 1.0 );

    FixupOrdering();
    
    CSLDestroy(papszProj);
    CSLDestroy(papszProjParms);
    CSLDestroy(papszDatum);

    return OGRERR_NONE;
    
not_enough_data:

    CSLDestroy(papszProj);
    CSLDestroy(papszProjParms);
    CSLDestroy(papszDatum);
    
    return OGRERR_NOT_ENOUGH_DATA;
}

