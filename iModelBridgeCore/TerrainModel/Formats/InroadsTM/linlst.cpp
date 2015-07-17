//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* linlst.c                                        tmi    26-Feb-1991         */
/*----------------------------------------------------------------------------*/
/* It takes a surface, a line as input, and starting triangle.  Then,         */
/* it calls a function which you specify for each triangle the line           */
/* passes over.  The search starts at the input triangle, which must          */
/* lie underneath the line, and moves towards the second endpoint of          */
/* line, ending at the endpoint input triangle, which also must lie           */
/* underneath the line.                                                       */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_sendTrianglesAlongLineSide(DPoint3d *,DPoint3d *,double *,DPoint3d *,DPoint3d *,DPoint3d *,int *,int *);
static int aecDTM_compareTinPtrs(const void *,const void *);




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendTrianglesAlongLine
 DESC: It takes a surface, a line as input, and starting triangle.  Then,
       it calls a function which you specify for each triangle the line
       passes over.  The search starts at the input triangle, which must
       lie underneath the line, and moves towards the second endpoint of
       line, ending at the endpoint input triangle, which also must lie
       underneath the line.
 HIST: Original - tmi 26-Feb-1991
 MISC:
 KEYW: DTM TRIANGLES LINE LIST GET
-----------------------------------------------------------------------------%*/

int aecDTM_sendTrianglesAlongLine /* <= TRUE if error              */
(
  void *mdlDescP,                /* => mdl app descriptor (or NULL)           */
  struct CIVdtmsrf *srfP,        /* => pointer to surface                     */
  DPoint3d *p0P,                 /* => first end point of line                */
  DPoint3d *p1P,                 /* => second end point of line               */
  struct CIVdtmtin *startTinP,   /* => starting triangle                      */
  struct CIVdtmtin *endTinP,     /* => ending triangle                        */
  int (*userFncP)(               /* => your function                          */
     struct CIVdtmtin *, DPoint3d *,DPoint3d *,int,int,void *),
  void *userDatP                 /* => your data                              */
)
{
  struct CIVdtmtin *prvTinP = startTinP, *lstTinP, *originalStartTinP = NULL;
  struct CIVdtmtin **tinsPP = NULL;
  DPoint3d nrm, tmp[2], tin[4];
  int sts = SUCCESS, dir, pntIndex, sidIndex = 0;
  long sida, sidb;
  double plnzer;
  long numTins = 0;
  long ctr = 0;


  VSUBXY ( *p1P, *p0P, tmp[0] );
  tmp[0].z = 0.; tmp[1] = tmp[0];  tmp[1].z += 1e6;
  VCROSS ( tmp[0], tmp[1], nrm );
  if ( ( plnzer = VLEN ( nrm ) ) > AEC_C_TOL ) VSCALE ( nrm, (1./plnzer), nrm );
  plnzer = VDOT ( nrm, *p1P );
  tmp[0] = *p0P;

  while ( sts == SUCCESS  &&  sidIndex < 3  &&  startTinP != (struct CIVdtmtin *)0 && startTinP != originalStartTinP )
  {
    if ( originalStartTinP == NULL )
        originalStartTinP = startTinP;

    if ( aecDTM_validateTinPtr ( NULL, srfP, startTinP) != SUCCESS ||
         bsearch( &startTinP, tinsPP, numTins, sizeof ( struct CIVdtmtin * ), aecDTM_compareTinPtrs ) )
    {
        break;
    }

    if( ctr == 1000 )
    {
        ctr = 0;

        if ( sts == SUCCESS )
        {
            aecTable_insert ( (void **)&tinsPP, (int *)&numTins, (void *)&startTinP, sizeof ( struct CIVdtmtin * ) );
            qsort ( tinsPP, numTins, sizeof ( struct CIVdtmtin * ), aecDTM_compareTinPtrs );
        }
    }

    DTMTINTODPOINT ( srfP, startTinP, tin );
    tin[3] = tin[0];

    if ( aecDTM_getTriangleSideIndex ( &sida, &sidb, startTinP, prvTinP, FALSE ) != SUCCESS )
      sidIndex = 3;
    else
      sidIndex = sida - 1;
    lstTinP = prvTinP;
    prvTinP = startTinP;

    if ( startTinP == endTinP )
    {
      sts = (*userFncP) ( startTinP, tin, p1P, 3, 3, userDatP );
      break;
    }
    else if ( ( sts = aecDTM_sendTrianglesAlongLineSide ( tin, &nrm, &plnzer, p0P, p1P, tmp, &sidIndex, &pntIndex ) ) == SUCCESS )
      if ( sidIndex > 2 )
      {
        sidIndex = 0;
        aecDTM_getIndexOfTrianglePointByCoordinates ( &pntIndex, tmp, tin );

        if ( pntIndex == 0 )
          dir = startTinP->n31 == lstTinP ? 1 : 0;
        else
          dir = *(&(startTinP->n12)+pntIndex-1) == lstTinP ? 1 : 0;

        aecDTM_rotateAroundPoint ( (struct CIVdtmpnt **)0, &startTinP, (int *)0, (int *)0, *(&(startTinP->p1)+pntIndex), startTinP, dir );
      }
      else
      {
        sts = (*userFncP) ( startTinP, tin, tmp, pntIndex, sidIndex, userDatP );

        if ( sts == SUCCESS )
          if ( pntIndex > 2 )
            startTinP = *(&(startTinP->n12) + sidIndex);
          else
          {
            if ( pntIndex == 0 )
              dir = startTinP->n31 == lstTinP ? 1 : 0;
            else
              dir = *(&(startTinP->n12)+pntIndex-1) == lstTinP ? 1 : 0;

            aecDTM_rotateAroundPoint ( (struct CIVdtmpnt **)0, &startTinP, (int *)0, (int *)0, *(&(startTinP->p1)+pntIndex), startTinP, dir );
        }
      }

    if( sts == SUCCESS && ( ctr % 10 ) == 0 )
      sts = aecInterrupt_check();

    ctr++;
  }  

  if ( tinsPP )
    free ( tinsPP );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendTrianglesAlongLineSide
 DESC: Finds where the line intersects the side of the triangle.
 HIST: Original - tmi 26-Feb-1991
 MISC: static
 KEYW: DTM TRIANGLES LINE LIST SIDE
-----------------------------------------------------------------------------%*/

static int aecDTM_sendTrianglesAlongLineSide
(
  DPoint3d *t,
  DPoint3d *nrm,
  double *plnzer,
  DPoint3d *p0,
  DPoint3d *p1,
  DPoint3d *pnt,
  int *sidIndex,
  int *pntIndex
)
{
  DPoint3d a[3], tmp;
  int sts = SUCCESS, i, j, sid[3];
  double q, q1, q2, dis[3];

  *pntIndex = 3;
  dis[0] = dis[1] = dis[2] = AEC_C_MAXDBL;
  sid[0] = sid[1] = sid[2] = 0;

  for ( i = j = 0; i < 3; i++ )
    if ( i != *sidIndex )
    {
      q1 = VDOT ( *nrm, t[i] );
      q2 = VDOT ( *nrm, t[i+1] );
      q1 -= *plnzer;
      q2 -= *plnzer;
      if ( ISSMALL( q1, AEC_C_TOL ) ) q1 = 0.;
      if ( ISSMALL( q2, AEC_C_TOL ) ) q2 = 0.;

      if ( q1 * q2 <= 0.  &&  ( q1 != 0.  ||  q2 != 0. ) )
      {
        q = q1 / ( q1 - q2 );
        VSUB ( t[i+1], t[i], a[j] );
        VSCALE ( a[j], q, a[j] );
        VADD ( t[i], a[j], a[j] );

        if ( VINSIDEXY ( *p0, *p1, a[j], AEC_C_TOL ) )
        {
      	  VSUBXY ( a[j], *p1, tmp );
	  dis[j] = VDOTXY ( tmp, tmp );
	  sid[j++] = i;
        }
      }
    }

  if      ( dis[0] == AEC_C_MAXDBL                   ) i = *sidIndex = 3;
  else if ( dis[0] <= dis[1]    &&  dis[0] <= dis[2] ) i = 0;
  else if ( dis[1] <= dis[0]    &&  dis[1] <= dis[2] ) i = 1;
  else if ( dis[2] <= dis[0]    &&  dis[2] <= dis[1] ) i = 2;
  else                                                 i = *sidIndex = 3;

  if ( i < 3 )
  {
    VSUBXY ( t[sid[i]], a[i], tmp );
    q1 = VDOTXY ( tmp, tmp );
    VSUBXY ( t[sid[i]+1], a[i], tmp );
    q2 = VDOTXY ( tmp, tmp );
    if ( q1 < AEC_C_TOL3SQR  ||  q2 < AEC_C_TOL3SQR )
      if ( q1 < q2 )
      {
        a[i] = t[sid[i]];
        if ( q1 < AEC_C_TOLSQR ) *pntIndex = sid[i];
      }
      else
      {
        a[i] = t[sid[i]+1];
        if ( q2 < AEC_C_TOLSQR ) *pntIndex = ( sid[i] == 2 ) ? 0 : sid[i] + 1;
      }

    *pnt = a[i];
    *sidIndex = sid[i];
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendTrianglesAlongLineFirst
 DESC: Finds the triangle where the line exits.
 HIST: Original - tmi 31-Jul-1991
 MISC:
 KEYW: DTM TRIANGLES LINE LIST FIRST
-----------------------------------------------------------------------------%*/

int aecDTM_sendTrianglesAlongLineFirst /* <= TRUE if error         */
(
  struct CIVdtmtin **tinPP,            /* <= exiting triangle                 */
  struct CIVdtmtin **neiPP,            /* <= neighbor to exiting tin          */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *startPntP,         /* => starting point                   */
  struct CIVdtmpnt *endPntP            /* => ending point                     */
)
{
  DPoint3d nrm, tmp[2], t[4];
  CIVdtmtin *pTempTin = NULL;
  int sts = SUCCESS, sidIndex = 3, pntIndex = 0, rpt;
  double plnzer;
  struct CIVdtmtin **tinsPP = NULL;
  long numTins = 0;
  long ctr = 0;

  sts = aecDTM_findTriangle ( tinPP, (int *)0, &rpt, (int *)0, srfP, &startPntP->cor );

  pTempTin = *tinPP;
  while ( sts == SUCCESS  &&  pntIndex != 3 && tinPP && *tinPP )
  {
    if ( aecDTM_validateTinPtr ( NULL, srfP, *tinPP) != SUCCESS ||
         bsearch( tinPP, tinsPP, numTins, sizeof ( struct CIVdtmtin * ), aecDTM_compareTinPtrs ) )
    {
        sts = DTM_M_TOLPRB;
        break;
    }

    if( ctr == 1000 )
    {
        ctr = 0;

        if ( sts == SUCCESS )
        {
            aecTable_insert ( (void **)&tinsPP, (int *)&numTins, (void *)tinPP, sizeof ( struct CIVdtmtin * ) );
            qsort ( tinsPP, numTins, sizeof ( struct CIVdtmtin * ), aecDTM_compareTinPtrs );
        }
    }

    DTMTINTODPOINT ( srfP, *tinPP, t );
    t[3] = t[0];

    VSUBXY ( endPntP->cor, startPntP->cor, tmp[0] );
    tmp[0].z = 0.; tmp[1] = tmp[0];  tmp[1].z += 1e6;
    VCROSS ( tmp[0], tmp[1], nrm );
    if ( ( plnzer = VLEN ( nrm ) ) > AEC_C_TOL ) VSCALE ( nrm, (1./plnzer), nrm );
    plnzer = VDOT ( nrm, endPntP->cor );

    sidIndex = 3;
    if ( ( sts = aecDTM_sendTrianglesAlongLineSide ( t, &nrm, &plnzer, &startPntP->cor, &endPntP->cor, tmp, &sidIndex, &pntIndex ) ) == SUCCESS )
      if ( pntIndex != 3 )
        if ( startPntP == *(&(*tinPP)->p1+pntIndex) )
        {
          sts = aecDTM_rotateAroundPoint ( (struct CIVdtmpnt **)0, tinPP, (int *)0, (int *)0, startPntP, *tinPP, 0 );
          if ( *tinPP == pTempTin ) //we've gone all the way around without success so break out
          {
            sts = DTM_M_TOLPRB;
          }
        }
        else
          break;

    if( sts == SUCCESS )
      sts = aecInterrupt_check();

    ctr++;
  }

  if ( sts == SUCCESS )
    if ( neiPP != (struct CIVdtmtin **)0 )
      if ( tinPP && *tinPP && *tinPP != *neiPP )
        *neiPP = *(&(*tinPP)->n12+sidIndex);

  if ( tinsPP )
    free ( tinsPP );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_compareTinPtrs
 DESC: 
 HIST: Original - twl 20-May-2002
 MISC: static
 KEYW: 
-----------------------------------------------------------------------------%*/

static int aecDTM_compareTinPtrs
(
  const void *elm1,
  const void * elm2
)
{
  struct CIVdtmtin **tin1PP = (struct CIVdtmtin **) elm1;
  struct CIVdtmtin **tin2PP = (struct CIVdtmtin **) elm2;

  if ( *tin1PP < *tin2PP )
    return -1;
  
  if ( *tin1PP > *tin2PP )
    return 1;

  return 0;
}

