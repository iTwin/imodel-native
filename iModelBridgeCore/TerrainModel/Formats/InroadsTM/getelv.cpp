//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* getelv.c                                  tmi    15-Jan-1993               */
/*----------------------------------------------------------------------------*/
/* Various utilities to retrieve an elevation.                                */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getElevationAtXY
 DESC: Given a pointer to a surface and an easting,northing location on
       that surface, this function computes and returns the elevation at
       location.  If the point is outside the model or inside an interior
       boundary, the function returns DTM_M_PNTNIN.
 HIST: Original - tmi 16-Apr-1990
 MISC:
 KEYW: DTM ELEVATION GET
-----------------------------------------------------------------------------%*/

int aecDTM_getElevationAtXY     /* <=  TRUE if error               */
(
  DPoint3d *pntP,                          /* <=> point's loc and elev.       */
  struct CIVdtmsrf *srfP,                  /*  => surface to use              */
  struct CIVdtmtin **seedTriPP,            /*  => seed tri. (or NULL)         */
  BOOL bIgnoreRangeTins, /* = FALSE */     /* <=                              */
  BOOL bIgnoreDeletedTins  /* = FALSE */   /* <=                              */
)
{
  struct CIVdtmtin *tin = (struct CIVdtmtin *)0;
  int sts, side, rpt;

  if ( seedTriPP ) tin = *seedTriPP;
  if ( ( sts = aecDTM_findTriangle ( &tin, 0, &rpt, &side, srfP, pntP ) ) == SUCCESS )
  {
        DPoint3d t[3];

        aecDTM_findNonDeletedNeighborTriangle ( &tin, rpt, side );
        if ( seedTriPP ) *seedTriPP = tin;
        DTMTINTODPOINT ( srfP, tin, t );
        aecDTM_getElevationAtXYGivenTriangle ( pntP, t );
  }

  if ( sts == SUCCESS && tin && srfP )
  {
    if ( ( bIgnoreRangeTins   && aecDTM_isTriangleRangeTriangle ( srfP, tin ) ) ||
         ( bIgnoreDeletedTins && aecDTM_isTriangleDeletedFlagSet ( tin ) ) )
    {
      struct CIVdtmtin *pTin = NULL;
      BOOL bFound = FALSE;
      DPoint3d *p1 = NULL;
      DPoint3d *p2 = NULL;

      for ( int i = 0; i < 3 && !bFound; i++ )
      {
        switch ( i )
        {
        case 0:
            p1 = &tin->p1->cor;
            p2 = &tin->p2->cor;
            pTin = tin->n12;
            break;

        case 1:
            p1 = &tin->p2->cor;
            p2 = &tin->p3->cor;
            pTin = tin->n23;
            break;

        case 2:
            p1 = &tin->p3->cor;
            p2 = &tin->p1->cor;
            pTin = tin->n31;
            break;
        }

        if ( p1 && p2 && pTin &&
             !aecDTM_isTriangleRangeTriangle ( srfP, pTin ) &&
             !aecDTM_isTriangleDeletedFlagSet ( pTin ) )
        {
            double offset = AEC_C_MAXDBL;

            if ( aecVec_projectToLine ( (double*)p1, (double*)p2, (double*)pntP, &offset, NULL, NULL ) &&
                 fabs ( offset ) <= AEC_C_TOL3 )
            {
                DPoint3d t[3];
                tin = pTin;
                DTMTINTODPOINT ( srfP, tin, t );
                aecDTM_getElevationAtXYGivenTriangle ( pntP, t );
                bFound = TRUE;
            }
        }
      }

      if ( !bFound )        
        sts = DTM_M_PNTNIN;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getElevationAtXYGivenTriangle
 DESC: Given a triangle and a location, this function computes the
       elevation at the given location using the triangle parameters.
 HIST: Original - tmi 28-Apr-1990
 MISC:
 KEYW: DTM ELEVATION GET TRIANGLE
-----------------------------------------------------------------------------%*/

void aecDTM_getElevationAtXYGivenTriangle  /* <=  TRUE if error       */
(
  DPoint3d *pP,                            /* <=> location and elev.          */
  DPoint3d *triP                           /*  => triangle to use             */
)
{
  DPoint3d a, b;

  VSUB ( triP[1], triP[0], a );
  VSUB ( triP[2], triP[0], b );
  VCROSSXY ( a, b, *pP );

  if ( fabs(pP->z) < 1e-15 )
    pP->z = 0.;
  else
    pP->z = triP[0].z + ( (a.x*b.z-a.z*b.x)*(pP->y-triP[0].y) - (a.y*b.z-a.z*b.y)*(pP->x-triP[0].x) ) / pP->z;
}
