//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include    "stdafx.h"

#define PNTFMT L"     (%*.*lf, %*.*lf, %*.*lf)"            /* DO_NOT_TRANSLATE */

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_crossingCheckProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
static int aecDTM_crossingCheckFound( void *, void *, void * );
static int aecDTM_crossingCheckLogPoint( DPoint3d * );
static double segElevation( DPoint3d *, DPoint3d *, DPoint3d * );


typedef struct
{
  void *intObjP;
  int (*intFuncP)( wchar_t *, wchar_t *, struct CIVdtmpnt *, struct CIVdtmpnt *, void * );
  void *mdlDescP;
  void *userDataP;
  int checkExtOnly;
  struct CIVdtmpnt *extPntP;
  DPoint3d *vrt;
  int nVrt;
  int option;
  CIVdtmsrf *srfP;
  CMapStringToString *ftrGuidsP;
} CrossingCheck;


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_crossingCheckExterior
 DESC: Checks for invalid conditions in DTM data.
 HIST: Original - dgc
 MISC:
 KEYW: DTM LINEAR CROSSINGS CHECK
-----------------------------------------------------------------------------%*/

int aecDTM_crossingCheckExterior  /* TRUE if error                 */
(
  CIVdtmsrf *srfP                    /* => surface to check                   */
)
{
  CrossingCheck dat;
  int sts = SUCCESS;

  memset( &dat, 0, sizeof( dat ) );
  dat.checkExtOnly = TRUE;
  dat.srfP = srfP;

  aecDTM_countSurfacePointsInFile ( srfP->extf );

  if( sts == SUCCESS )
    sts = aecIntersect_begin( &dat.intObjP, aecDTM_crossingCheckFound, NULL, &dat, srfP->extf->nrec );

  if( sts == SUCCESS )
    sts = aecDTM_sendAllPointsHonoringFence( NULL, srfP, DTM_C_NOCONS,
      DTM_C_EXTMSK, aecDTM_crossingCheckProcess, &dat );

  if( sts == SUCCESS )
    sts = aecIntersect_end( dat.intObjP );

  if( sts == DTM_M_XNGEXT && dat.extPntP )
  {
    DPoint3d *vtsP = NULL;
    long nVts = 0;

    FeatureHandle ftrHndl = NULL;

    if ( ( sts = aecFeature_create ( &ftrHndl ) ) == SUCCESS )
    if ( ( sts = aecFeature_loadFromDTMExteriorBoundary ( ftrHndl, srfP ) ) == SUCCESS )
        if( ( sts = aecFeature_getPoints ( ftrHndl, NULL, &vtsP, NULL, &nVts ) ) == SUCCESS )
        {
        aecPolygon_removeLoops( nVts, vtsP );
        aecPolygon_removeDuplicatePoints( &nVts, vtsP, AEC_C_TOL );
        aecFeature_removePoints ( ftrHndl, 0, aecFeature_getPointCount ( ftrHndl ) );
        if ( ( sts = aecFeature_addPoints ( ftrHndl, NULL, vtsP, NULL, nVts ) ) == SUCCESS )
            sts = aecFeature_saveToDTM ( ftrHndl, srfP, 0, TRUE );
        }

    if ( ftrHndl )
    aecFeature_destroy ( ftrHndl );

    if( vtsP ) free( vtsP );
  }

  return( sts );
}



static int aecDTM_crossingCheckProcess
(
  void *tmp,
  int typ,
  long np,
  DPoint3d *,
  struct CIVdtmpnt *pnt
)
{
  double xlo, ylo, xhi, yhi;
  CrossingCheck *datP = ( CrossingCheck * )tmp;
  int i, sts = SUCCESS;

  if ( datP->ftrGuidsP )
  {
    struct CIVdtmftr *ftrP = NULL;
    BOOL bPass = FALSE;

    if ( aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, pnt ) == SUCCESS )
    {
      CString strGuidString;
      wchar_t guidString[GUID_STRING_MAX_SIZE];
      aecGuid_toString ( guidString, &ftrP->guid );
      strGuidString = guidString;

      if ( datP->ftrGuidsP->Lookup ( strGuidString, strGuidString ) )
          bPass = TRUE;
    }

    if ( !bPass )
        return sts;
  }

  if( datP->checkExtOnly && typ == DTM_C_DTMEXT )
    datP->extPntP = pnt;

  //  The following check is made to fix a TL.  The problem here is that np corresponds with the *p buffer,
  //  but we are iterating through the point buffer, *pnt.  Especially when the series of points is closed,
  //  I have found that there are 1 or 2 more points in *p than are actually contained in *pnt.  This has
  //  to do with the clip/join logic that determines whether a series of segments are closed.  Therefore,
  //  if you use np as passed to you as the number of points in the breakline (or any other linear series)
  //  you are actually checking a crossing with (a) the first point(s) of the next breakline in the dtm, or
  //  (b) whatever garbage happens to be in memory after the last point in the breakline.  To ensure that 
  //  we do not walk into invalid memory, we are counting the true number of points that are in this linear 
  //  feature according to what the dtm data structure knows about.  We are also checking below to see if the 
  //  pen-up/down flag is not set on the next point, which indicates that we are no longer on the breakline.
  //  This is just a safety check that shouldn't be encountered if 're-counting' the points works in all
  //  cases, and the check is not fail safe if the 'garbage' happens to have the DTM_C_PNTPUD set.
  //          - dakloske - 10/25/98
  if ( typ == DTM_C_DTMBRK || typ == DTM_C_DTMCTR || typ == DTM_C_DTMINF || typ == DTM_C_DTMINT || typ == DTM_C_DTMEXT ) 
  {
      long numActualPoints = 0;
      aecDTM_countPointsInLinearFeature ( &numActualPoints, datP->srfP, NULL, pnt);
      if ( numActualPoints )
          np = numActualPoints;
  }

  for ( i = 0; i < np - 1  &&  sts == SUCCESS; i++ )
  {
    int next = i + 1;

    if ( ! ( pnt[next].flg & DTM_C_PNTPUD ) || pnt[next].flg & DTM_C_PNTDEL )
        break;

    if( !datP->checkExtOnly ) pnt[i].flg &= ~DTM_C_PNTIGN;
    xlo = pnt[i].cor.x < pnt[next].cor.x ? pnt[i].cor.x : pnt[next].cor.x;
    ylo = pnt[i].cor.y < pnt[next].cor.y ? pnt[i].cor.y : pnt[next].cor.y;
    xhi = pnt[i].cor.x > pnt[next].cor.x ? pnt[i].cor.x : pnt[next].cor.x;
    yhi = pnt[i].cor.y > pnt[next].cor.y ? pnt[i].cor.y : pnt[next].cor.y;

    sts = aecIntersect_insert( datP->intObjP, &pnt[i], xlo, ylo, xhi, yhi );
  }

  return ( sts );
}



static int aecDTM_crossingCheckFound
(
  void *tmp1P,
  void *tmp2P,
  void *tmp3P
)
{
  struct CIVdtmpnt *pnt1P = ( struct CIVdtmpnt * )tmp1P;
  struct CIVdtmpnt *pnt2P = ( struct CIVdtmpnt * )tmp2P;
  struct CIVdtmftr *ftrP = NULL;
  CrossingCheck *datP = ( CrossingCheck * )tmp3P;
  wchar_t name1[DTM_C_NAMSIZ];
  wchar_t name2[DTM_C_NAMSIZ];
  int sts = SUCCESS;

  if( VEQUAL( pnt1P[0].cor, pnt2P[0].cor, AEC_C_TOL1 ) || VEQUAL( pnt1P[0].cor, pnt2P[1].cor, AEC_C_TOL1 ) )
    if( VEQUAL( pnt1P[0].cor, pnt1P[1].cor, AEC_C_TOL1 ) || VEQUAL( pnt2P[0].cor, pnt2P[1].cor, AEC_C_TOL1 ) )
      return( sts );
    else if( aecVec_pointPositionInLine( &pnt2P[0].cor, &pnt2P[1].cor, &pnt1P[1].cor ) != 0 )
      return( sts );

  if( VEQUAL( pnt1P[1].cor, pnt2P[0].cor, AEC_C_TOL1 ) || VEQUAL( pnt1P[1].cor, pnt2P[1].cor, AEC_C_TOL1 ) )
    if( VEQUAL( pnt1P[0].cor, pnt1P[1].cor, AEC_C_TOL1 ) || VEQUAL( pnt2P[0].cor, pnt2P[1].cor, AEC_C_TOL1 ) )
      return( sts );
    else if( aecVec_pointPositionInLine( &pnt2P[0].cor, &pnt2P[1].cor, &pnt1P[0].cor ) != 0 )
      return( sts );

  if( aecVec_doLinesIntersect( &pnt1P[0].cor, &pnt1P[1].cor, &pnt2P[0].cor, &pnt2P[1].cor ) )
  {
    if( datP->checkExtOnly )
      sts = DTM_M_XNGEXT;
    else
    {
      DPoint3d intPnt;
      double z1, z2;
      int overLap, misElev;

      aecPolygon_intersect( &intPnt, NULL, NULL, &pnt1P[0].cor, &pnt1P[1].cor, &pnt2P[0].cor, &pnt2P[1].cor );

      if ( ( (datP->option & DTM_C_INSIDE) || (datP->option & DTM_C_OUTSIDE) ) && datP->nVrt > 1 )
      {
        if( (datP->option & DTM_C_INSIDE) != 0 && !aecPolygon_isPointInside( datP->nVrt, datP->vrt, &intPnt ) )
          return SUCCESS;

        if( (datP->option & DTM_C_OUTSIDE) != 0 && aecPolygon_isPointInside( datP->nVrt, datP->vrt, &intPnt ) )
          return SUCCESS;
      }

      z1 = segElevation( &intPnt, &pnt1P[0].cor, &pnt1P[1].cor );
      z2 = segElevation( &intPnt, &pnt2P[0].cor, &pnt2P[1].cor );

      overLap = aecPolygon_colinear( &pnt1P[0].cor, &pnt1P[1].cor, &pnt2P[0].cor, &pnt2P[1].cor );
      misElev = ( fabs( z1 - z2 ) > AEC_C_TOL );

      if( overLap || misElev )
      {
        if( !( pnt1P->flg & DTM_C_PNTIGN ) && !( pnt2P->flg & DTM_C_PNTIGN ) )
        {
          long pnt1Typ;
          long pnt2Typ;

          if ( aecDTM_findPointType ( &pnt1Typ, 0, 0, datP->srfP, pnt1P ) != SUCCESS ||
               aecDTM_findPointType ( &pnt2Typ, 0, 0, datP->srfP, pnt2P ) != SUCCESS    )
          {
            pnt1Typ = DTM_C_DTMREG;
            pnt2Typ = DTM_C_DTMREG;
          }

          if ( pnt1Typ == DTM_C_DTMEXT && pnt2Typ != DTM_C_DTMEXT )
          {
            pnt2P[0].flg |= DTM_C_PNTIGN;
          }
          else if ( pnt2Typ == DTM_C_DTMEXT && pnt1Typ != DTM_C_DTMEXT )
          {
            pnt1P[0].flg |= DTM_C_PNTIGN;
          }
          else if ( pnt1Typ == DTM_C_DTMINT && pnt2Typ != DTM_C_DTMINT )
          {
            pnt2P[0].flg |= DTM_C_PNTIGN;
          }
          else if ( pnt2Typ == DTM_C_DTMINT && pnt1Typ != DTM_C_DTMINT )
          {
            pnt1P[0].flg |= DTM_C_PNTIGN;
          }
          else
          {
            DPoint3d vec1, vec2;
  
            VSUBXY( pnt1P[0].cor, pnt1P[1].cor, vec1 );
            VSUBXY( pnt2P[0].cor, pnt2P[1].cor, vec2 );

            if( VLENXY( vec1 ) < VLENXY( vec2 ) )
              pnt1P[0].flg |= DTM_C_PNTIGN;
            else
              pnt2P[0].flg |= DTM_C_PNTIGN;
          }
        }
      }

      if( overLap )
      {
        if ( aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt1P[0] ) == SUCCESS ||
             aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt1P[1] ) == SUCCESS )
        {
            wcscpy ( name1, ftrP->nam );
        }
        else
            wcscpy ( name1, aecOutput_getMessageString ( DTM_M_SEGMENT1 ) );

        aecDTM_crossingCheckLogPoint( &pnt1P[0].cor );
        aecDTM_crossingCheckLogPoint( &pnt1P[1].cor );


        if ( aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt2P[0] ) == SUCCESS ||
             aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt2P[1] ) == SUCCESS )
        {
            wcscpy ( name2, ftrP->nam );
        }
        else
            wcscpy ( name2, aecOutput_getMessageString ( DTM_M_SEGMENT2 ) );

        aecDTM_crossingCheckLogPoint( &pnt2P[0].cor );
        aecDTM_crossingCheckLogPoint( &pnt2P[1].cor );
      }
      else if( misElev )
      {
        if ( aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt1P[0] ) == SUCCESS ||
             aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt1P[1] ) == SUCCESS )
        {
            wcscpy ( name1, ftrP->nam );
        }
        else
            wcscpy ( name1, aecOutput_getMessageString ( DTM_M_SEGMENT1 ) );

        aecDTM_crossingCheckLogPoint( &pnt1P[0].cor );
        aecDTM_crossingCheckLogPoint( &pnt1P[1].cor );


        if ( aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt2P[0] ) == SUCCESS ||
             aecDTM_findFeatureByPoint ( &ftrP, datP->srfP, &pnt2P[1] ) == SUCCESS )
        {
            wcscpy ( name2, ftrP->nam );
        }
        else
            wcscpy ( name2, aecOutput_getMessageString ( DTM_M_SEGMENT2 ) );

        aecDTM_crossingCheckLogPoint( &pnt2P[0].cor );
        aecDTM_crossingCheckLogPoint( &pnt2P[1].cor );

        intPnt.z = z1;
        aecDTM_crossingCheckLogPoint( &intPnt );
        intPnt.z = z2;
        aecDTM_crossingCheckLogPoint( &intPnt );
      }

      if( datP->intFuncP )
      {
          sts = (*datP->intFuncP)( name1, name2, pnt1P, pnt2P, datP->userDataP );
      }
    }
  }

  return( sts );
}


static int aecDTM_crossingCheckLogPoint
(
  DPoint3d *pntP
)
{
  wchar_t buf[ AEC_C_BUFLEN ];
  int widX, prcX;
  int widZ, prcZ;

  aecParams_getX( &prcX );
  aecParams_getZ( &prcZ );
  widX = prcX + 8;
  widZ = prcZ + 8;

  swprintf( buf, PNTFMT, widX, prcX, pntP->x, widX, prcX, pntP->y, widZ, prcZ, pntP->z );

  return( SUCCESS );
}


/*%-----------------------------------------------------------------------------
 FUNC: segElevation
 DESC:
 HIST: Original - dgc
 MISC: static
 KEYW:
-----------------------------------------------------------------------------%*/

static double segElevation
(
  DPoint3d *pntP,                      /* =>                                  */
  DPoint3d *v0,                        /* =>                                  */
  DPoint3d *v1                         /* =>                                  */
)
{
  double z;
  if( fabs( v1->x - v0->x ) > AEC_C_TOL )
    z = ( pntP->x - v0->x ) / ( v1->x - v0->x ) * ( v1->z - v0->z ) + v0->z;
  else if( fabs( v1->y - v0->y ) > AEC_C_TOL )
    z = ( pntP->y - v0->y ) / ( v1->y - v0->y ) * ( v1->z - v0->z ) + v0->z;
  else
    z = v0->z;
  return( z );
}


