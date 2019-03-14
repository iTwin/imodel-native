//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* getact.c                                        tmi    02-Jan-1993         */
/*----------------------------------------------------------------------------*/
/* Several utilities to get the active surface and project.                   */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Statics                                                                    */
/*----------------------------------------------------------------------------*/
static CIVdtmprj* *CIV_G_DTMPRJ = 0L;
static short CIV_G_DTMNUM = 0;
static short CIV_G_DTMACT = -1;

static int aecDTM_doGetActiveSurface(struct CIVdtmsrf **,wchar_t *,wchar_t *,short,struct CIVdtmprj *);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getActiveSurface
 DESC: Returns the active surface and/or name and/or description.
 HIST: Original - tmi 10-Apr-1990
       10/94 - wbw - moved aecDTM_getActiveSurface code to aecDTM_doGetActiveSurface
 MISC:
 KEYW: DTM SURFACE ACTIVE GET NAME DESCRIPTION
-----------------------------------------------------------------------------%*/

int aecDTM_getActiveSurface /* <= TRUE if error                    */
(
  struct CIVdtmsrf **srfPP,          /* <= pointer to active surface          */
  wchar_t *srfNameP,                 /* <= name of active surface             */
  wchar_t *srfDescP                  /* <= description of active surf.        */
)
{
  struct CIVdtmprj *prjP;
  int sts = SUCCESS;

  if ( ( sts = aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 ) ) == SUCCESS )
    sts = aecDTM_doGetActiveSurface( srfPP, srfNameP, srfDescP, prjP->asrf, NULL );

  return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_doGetActiveSurface
 DESC: Actually returns the active surface and/or name and/or description.
 HIST: Original - tmi 10-Apr-1990
       10/94 - wbw - C&P from aecDTM_getActiveSurface
 MISC:
 KEYW: DTM SURFACE ACTIVE GET NAME DESCRIPTION
-----------------------------------------------------------------------------%*/

static int aecDTM_doGetActiveSurface /* <= TRUE if error                      */
(
  struct CIVdtmsrf **srfPP,          /* <= pointer to active surface          */
  wchar_t *srfNameP,                 /* <= name of active surface             */
  wchar_t *srfDescP,                 /* <= description of active surf.        */
  short activeSrf,                   /* => active surface index               */
  struct CIVdtmprj *prjP             /* => project to use, or NULL            */
)
{
  int sts = SUCCESS;

  if ( srfPP != (struct CIVdtmsrf **)0 ) *srfPP = (struct CIVdtmsrf *)0;
  if ( srfNameP != (wchar_t *)0 ) srfNameP[0] = '\0';
  if ( srfDescP != (wchar_t *)0 ) srfDescP[0] = '\0';

  if ( prjP == (struct CIVdtmprj *)0 )
    sts = aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

  if ( sts == SUCCESS )
    if ( prjP != (struct CIVdtmprj *)0 )
    {
      struct CIVdtmsrf *srfP;

      srfP = (struct CIVdtmsrf *) ( prjP->nsrf > 0 ? prjP->srfs[activeSrf] : 0 );

      if ( srfPP != (struct CIVdtmsrf **)0 )
        *srfPP = srfP;

      if ( srfP == (struct CIVdtmsrf *)0 )
        sts = DTM_M_NOSRFS;
      else
      {
        if ( srfNameP != (wchar_t *)0 )
          if ( srfP->nam[0] != '\0' )
            wcscpy ( srfNameP, srfP->nam );

        if ( srfDescP != (wchar_t *)0 )
          if ( srfP->des[0] != '\0' )
            wcscpy ( srfDescP, srfP->des );
      }
    }

  return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getActiveProject
 DESC: Returns the active project and/or name and/or description.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM PROJECT ACTIVE GET NAME DESCRIPTION
-----------------------------------------------------------------------------%*/

int aecDTM_getActiveProject /* <= TRUE if error                    */
(
  struct CIVdtmprj **prjPP,          /* <= pointer to active project          */
  wchar_t *prjNameP,                 /* <= active surface's name              */
  wchar_t *prjDescP                  /* <= active surface's desc.             */
)
{
  struct CIVdtmprj *tmpP;
  int sts = SUCCESS;

  if ( prjPP != (struct CIVdtmprj **)0 ) *prjPP = (struct CIVdtmprj *)0;
  if ( prjNameP != (wchar_t *)0 ) prjNameP[0] = '\0';
  if ( prjDescP != (wchar_t *)0 ) prjDescP[0] = '\0';

  tmpP =  ( CIV_G_DTMNUM > 0 ? CIV_G_DTMPRJ[CIV_G_DTMACT] : 0 );

  if ( tmpP == (struct CIVdtmprj *)0 )
    sts = DTM_M_NOPRJS;
  else
  {
    if ( prjPP != (struct CIVdtmprj **)0 )
      *prjPP = tmpP;

    if ( prjNameP != (wchar_t *)0 )
      if ( tmpP->nam[0] != '\0' )
        wcscpy ( prjNameP, tmpP->nam );

    if ( prjDescP != (wchar_t *)0 )
      if ( tmpP->des[0] != '\0' )
        wcscpy ( prjDescP, tmpP->des );
  }

  return ( sts );
}

static int _sPrjRefCount = 0;

static void aecDTM_getProjectInfo
(
  CIVdtmprj  ****basePrjPPP,               /* <= pointer to list of prjs            */
  short *numPrjP,                    /* <= total # of DTM projects            */
  short *actPrjP                     /* <= pointer to active project          */
)
{
  if (basePrjPPP != nullptr) *basePrjPPP = &CIV_G_DTMPRJ;
  if ( numPrjP    != (short *)0  ) *numPrjP    =  CIV_G_DTMNUM;
  if ( actPrjP    != (short *)0  ) *actPrjP    =  CIV_G_DTMACT;
}

static void aecDTM_addProject
(
  struct CIVdtmprj *prjP             /* => DTM project to add                 */
)
{
  CIV_G_DTMACT = CIV_G_DTMNUM++;
  CIV_G_DTMPRJ[CIV_G_DTMACT] = prjP;
}

static void aecDTM_freeProject
(
  void
)
{
  if( CIV_G_DTMPRJ )
    free( CIV_G_DTMPRJ );

  CIV_G_DTMPRJ = nullptr;
  CIV_G_DTMNUM = 0;
  CIV_G_DTMACT = -1;
}


static int aecDTM_allocateProject    /* <= TRUE if error                  */
(
  struct CIVdtmprj **prjPP,              /* <= created prj. (or NULL)         */
  wchar_t *filP,                         /* => prj. file name (or NULL)       */
  wchar_t *namP,                         /* => prj. name (or NULL)            */
  wchar_t *desP                          /* => prj. desc (or NULL)            */
)
{
  struct CIVdtmprj *prjP;
  short numPrj;
  CIVdtmprj ***basePrjPP;
  int sts = SUCCESS, num = 1;

  if ( ( prjP = ( struct CIVdtmprj *) calloc ( (unsigned int)num, sizeof(struct CIVdtmprj) ) ) == (struct CIVdtmprj *)0 )
    return ( DTM_M_MEMALF );

  prjP->srfs = 0;
  prjP->nsrf = 0;
  prjP->asrf = -1;
  prjP->asrf1 = 0;
  prjP->asrf2 = 0;

  if ( filP != (wchar_t *)0  &&  filP[0] != '\0' )
    wcsncpy ( prjP->fil, filP, CIV_MAX_FNAME-1 );
  prjP->fil[CIV_MAX_FNAME-1] = '\0';

  if ( namP != (wchar_t *)0  &&  namP[0] != '\0' )
    wcsncpy ( prjP->nam, namP, DTM_C_NAMSIZ-1 );
  else
    wcsncpy ( prjP->nam, aecOutput_getMessageString (DTM_M_NONAME), DTM_C_NAMSIZ-1 );
  prjP->nam[DTM_C_NAMSIZ-1] = '\0';

  if ( desP != (wchar_t *)0  &&  desP[0] != '\0' )
  {
    wcsncpy ( prjP->des, desP, DTM_C_NAMSIZ-1 );
    prjP->des[DTM_C_NAMSIZ-1] = '\0';
  }
  else
  {
    prjP->des[0] = '\0';
  }

  aecDTM_getProjectInfo ( &basePrjPP, &numPrj, (short *)0 );

  if ( numPrj == 0 )
      *basePrjPP = (CIVdtmprj* *)calloc ((unsigned int)(numPrj = 1), sizeof(CIVdtmprj*));
  else
      *basePrjPP = (CIVdtmprj* *)realloc ((void *)*basePrjPP, (unsigned int)(numPrj += 1) * sizeof(CIVdtmprj*));

  if (*basePrjPP == (CIVdtmprj* *)0)
    sts = DTM_M_MEMALF;
  else
    aecDTM_addProject ( prjP );

  if ( prjPP != (struct CIVdtmprj **)0 ) *prjPP = (struct CIVdtmprj *) prjP;

  return ( sts );
}

static int aecDTM_createProject    /* <= TRUE if error                    */
(
  struct CIVdtmprj **prjPP,            /* <= pointer to created proj.         */
  wchar_t *nameP,                      /* => name of project                  */
  wchar_t *descP                       /* => description of project           */
)
{
  struct CIVdtmprj *prjP;
  int sts;

  sts = aecDTM_allocateProject ( &prjP, NULL, nameP, descP );
  if ( prjPP != (struct CIVdtmprj **)0 ) *prjPP = prjP;

  return ( sts );
}

int  aecDTM_connectToActiveProject ( struct CIVdtmprj **p, wchar_t *defaultName, wchar_t *defaultDesc )
{
    int sts = SUCCESS;

    sts = aecDTM_getActiveProject ( p, 0, 0 );
    if ( sts != SUCCESS )
        sts = aecDTM_createProject ( p, defaultName, defaultDesc );

    if ( sts == SUCCESS )
        _sPrjRefCount++;
    else
        *p = 0;

    return sts;
}


void aecDTM_disconnectFromActiveProject ( void )
{
    _sPrjRefCount--;
    assert(_sPrjRefCount>=0);

    if (  _sPrjRefCount <= 0 )
    {
        struct CIVdtmprj *p = 0;
        int sts = aecDTM_getActiveProject ( &p, 0, 0 );

        aecDTM_freeProject();

        if ( sts == SUCCESS && p )
            free ( p );
    }
}


