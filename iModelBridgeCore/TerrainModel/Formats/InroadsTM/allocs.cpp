//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* allocs.c                                            tmi	10-Apr-199    */
/*----------------------------------------------------------------------------*/
/* Contains all the functions which allocate memory for the terrain           */
/* modeling functions.                                                        */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_allocateSurface
 DESC: Allocates memory for a new surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM SURFACE ALLOCATE MEMORY
-----------------------------------------------------------------------------%*/

int aecDTM_allocateSurface /* <= TRUE if error                     */
(
  struct CIVdtmsrf **srfPP,           /* <= ptr to new surface (or NULL)*/
  struct CIVdtmprj *prjP,             /* => DTM prj to use (or NULL)          */
  wchar_t *,                          /* => surface file name (or NULL)       */
  wchar_t *namP,                      /* => surface name (or NULL)            */
  wchar_t *desP,                      /* => surface desc (or NULL)            */
  double *sclP,                       /* => surface scale (or NULL)           */
  double *mtlP,                       /* => max. tri. length (or NULL)        */
  unsigned long *versionP,            /* => version # (or NULL)               */
  wchar_t *matP,                      /* => surface material (or NULL)        */
  long type                           /* => surface type                      */
)
{
  CIVdtmsrf* srfP = new CIVdtmsrf();
  int sts = SUCCESS;

  if ( srfP == (struct CIVdtmsrf *)0 )
    return ( DTM_M_MEMALF );

  if ( namP != (wchar_t *)0  &&  namP[0] != '\0' )
    wcsncpy ( srfP->nam, namP, DTM_C_NAMSIZ-1 );
  else
    wcsncpy ( srfP->nam, aecOutput_getMessageString (DTM_M_NONAME), DTM_C_NAMSIZ-1 );
  srfP->nam[DTM_C_NAMSIZ-1] = '\0';

  if ( desP != (wchar_t *)0  &&  desP[0] != '\0' )
  {
    wcsncpy ( srfP->des, desP, DTM_C_NAMSIZ-1 );
    srfP->des[DTM_C_NAMSIZ-1] = '\0';
  }
  else
  {
    srfP->des[0] = '\0';
  }

  if ( matP != (wchar_t *)0  &&  matP[0] != '\0' )
  {
    wcsncpy ( srfP->mat, matP, CIV_C_NAMSIZ-1 );
    srfP->mat[CIV_C_NAMSIZ-1] = '\0';}
  else
  {
    srfP->mat[0] = '\0';
  }

  srfP->par.type = (short)type;

  wcsncpy ( srfP->pref, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
  wcsncpy ( srfP->secsym, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
  wcsncpy ( srfP->prfsym, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );

  if ( ( sts = aecDTM_allocateFile ( &srfP->tinf, DTM_C_DTMTIN ) ) == SUCCESS )
      if ( ( sts = aecDTM_allocateFile ( &srfP->rngf, DTM_C_DTMRNG ) ) == SUCCESS )
        if ( ( sts = aecDTM_allocateFile ( &srfP->styf, DTM_C_DTMSTY ) ) == SUCCESS )
          if ( ( sts = aecDTM_allocateFile ( &srfP->payf, DTM_C_DTMPAY ) ) == SUCCESS )
          {
            for ( int i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
	          sts = aecDTM_allocateFile ( &srfP->pntf[i], i );

            for ( int i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
	          sts = aecDTM_allocateFile ( &srfP->ftrf[i], i + DTM_C_FSTFTRF );
          }

  if ( sts == SUCCESS )
    sts = aecDTM_hashFeatureCreate ( srfP );

  if ( sts == SUCCESS )
    if ( ( sts = aecDTM_allocateFile ( &srfP->corf, DTM_C_DTMCOR ) ) == SUCCESS )
      if ( ( sts = aecDTM_allocateFile ( &srfP->cmpf, DTM_C_DTMCMP ) ) == SUCCESS )
        sts = aecDTM_allocateFile ( &srfP->cmpMemf, DTM_C_DTMCMPMEM );

  if ( sts == SUCCESS )
  {
    srfP->flg &= ~DTM_C_DTMPRJ;

    if ( sclP != (double *)0 ) srfP->par.scl = *sclP;
    if ( mtlP != (double *)0 ) srfP->par.maxsid = *mtlP;
    srfP->version = ( versionP != (unsigned long *)0  &&  *versionP > 0 ) ? *versionP : DTM_C_DTMVER;
    aecLocale_getActiveCodePage ( &srfP->codePage );
    srfP->regf = srfP->pntf[0];
    srfP->brkf = srfP->pntf[1];
    srfP->intf = srfP->pntf[2];
    srfP->extf = srfP->pntf[3];
    srfP->ctrf = srfP->pntf[4];
    srfP->inff = srfP->pntf[5];
    srfP->pntf[6] = srfP->rngf;
    srfP->regFtrf = srfP->ftrf[0];
    srfP->brkFtrf = srfP->ftrf[1];
    srfP->intFtrf = srfP->ftrf[2];
    srfP->extFtrf = srfP->ftrf[3];
    srfP->ctrFtrf = srfP->ftrf[4];

    if ( prjP != (struct CIVdtmprj *)0 )
      sts = aecDTM_addSurfaceToProject ( prjP, srfP );

    if ( srfPP != (struct CIVdtmsrf **)0 )
      *srfPP = srfP;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_allocateFile
 DESC: Allocates a control structure for a single type of DTM entity.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FILE ALLOCATE MEMORY
-----------------------------------------------------------------------------%*/

int aecDTM_allocateFile    /* <= TRUE if error                     */
(
  struct CIVdtmfil **filPP,           /* <= ptr to allocated file             */
  long pointType                      /* => type of point                     */
)
{
  int sts = SUCCESS, num = 1;

  if ( ( (*filPP) = (struct CIVdtmfil *) calloc ( (unsigned int)num, sizeof(struct CIVdtmfil) ) ) == (struct CIVdtmfil *)0 )
    sts = DTM_M_MEMALF;
  else
  {
    (*filPP)->type = pointType;
    (*filPP)->min.x = (*filPP)->min.y = (*filPP)->min.z = AEC_C_MAXDBL;
    (*filPP)->max.x = (*filPP)->max.y = (*filPP)->max.z = AEC_C_MINDBL;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_allocateBlock
 DESC: Allocates one block of memory to hold a some DTM entities if an
       existing block does not have enough space for the data.
 HIST: Original - tmi 17-Jan-1994
 MISC:
 KEYW: DTM BLOCK ALLOCATE MEMORY
-----------------------------------------------------------------------------%*/

int aecDTM_allocateBlock    /* <= TRUE if error                    */
(
  struct CIVdtmblk **blkPP,            /* <= ptr to allocated block           */
  struct CIVdtmfil *filP,              /* => file to allocate block in        */
  long nrec,                           /* => # records to allocate            */
  int frc                              /* => TRUE: force fract. alloc.        */
)
{
  int sts = SUCCESS;

  *blkPP = filP->blk;

  if ( !frc )
    if ( *blkPP != NULL )
      if ( (*blkPP)->alc != (*blkPP)->use )
        return ( sts );

  if ( ( (*blkPP) = (struct CIVdtmblk *) calloc ( (unsigned int)1, sizeof(struct CIVdtmblk) ) ) == (struct CIVdtmblk *)0 )
    return ( DTM_M_MEMALF );

  (*blkPP)->fil = filP;
  (*blkPP)->nxt = filP->blk;
  filP->blk     = *blkPP;
  (*blkPP)->alc = nrec;

  switch ( (int) filP->type )
  {
    case ( DTM_C_DTMTIN ) :
      if ( ( (*blkPP)->rec.tin = (struct CIVdtmtin *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmtin) ) ) == (struct CIVdtmtin *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMREGFTR ) :
    case ( DTM_C_DTMBRKFTR ) :
    case ( DTM_C_DTMINTFTR ) :
    case ( DTM_C_DTMEXTFTR ) :
    case ( DTM_C_DTMCTRFTR ) :
      if ( ( (*blkPP)->rec.ftr = (struct CIVdtmftr *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmftr) ) ) == (struct CIVdtmftr *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMSTY ) :
      if ( ( (*blkPP)->rec.sty = (struct CIVdtmsty *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmsty) ) ) == (struct CIVdtmsty *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMPAY ) :
      if ( ( (*blkPP)->rec.pay = (struct CIVdtmpay *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmpay) ) ) == (struct CIVdtmpay *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMCOR ) :
      if ( ( (*blkPP)->rec.cor = (struct CIVdtmcor *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmcor) ) ) == (struct CIVdtmcor *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMCMP ) :
      if ( ( (*blkPP)->rec.cmp = (struct CIVdtmcmp *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmcmp) ) ) == (struct CIVdtmcmp *)0 )
        sts = DTM_M_MEMALF;
      break;

    case ( DTM_C_DTMCMPMEM ) :
      if ( ( (*blkPP)->rec.cmpMem = (struct CIVdtmcmpmem *) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmcmpmem) ) ) == (struct CIVdtmcmpmem *)0 )
        sts = DTM_M_MEMALF;
      break;

    default :
      if ( ( (*blkPP)->rec.pnt = ( struct CIVdtmpnt * ) calloc ( (unsigned int)nrec, sizeof(struct CIVdtmpnt) ) ) == (struct CIVdtmpnt *)0 )
        sts = DTM_M_MEMALF;
      break;
  }

  return ( sts );
}
