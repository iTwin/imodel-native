//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* delsrf.c                                              tmi    07-Jun-1990   */
/*----------------------------------------------------------------------------*/
/* It deletes a surface, lock, stock, and barrel.                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Extrenal functions                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_deleteFile(struct CIVdtmfil *);
static int aecDTM_removeSurface(struct CIVdtmsrf *);

/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteSurface
 DESC: It deletes a surface, lock, stock, and barrel.
 HIST: Original - tmi 07-Jun-1990
 MISC:
 KEYW: DTM SURFACE DELETE MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_deleteSurface    /* <= TRUE if error                    */
(
  struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
  struct CIVdtmsrf *srfP,              /* => surface to delete                */
  int emptyOnly                        /* => TRUE: just empty surface         */
)
{
  return aecDTM_deleteSurfaceEx ( prjP, srfP, emptyOnly, TRUE );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteSurfaceEx
 DESC: It deletes a surface, lock, stock, and barrel.  Gives option to 
       not update explorer window.
 HIST: Original - tmi 07-Jun-1990
 MISC:
 KEYW: DTM SURFACE DELETE MEMORY FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_deleteSurfaceEx  /* <= TRUE if error                    */
(
  struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
  struct CIVdtmsrf *srfP,              /* => surface to delete                */
  int emptyOnly,                       /* => TRUE: just empty surface         */
  int updateExplorer                   /* => remove surface from explorer win */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
    if ( ( sts = aecDTM_removeSurface ( srfP ) ) == SUCCESS )
      if ( !emptyOnly )
      {
        long i;

        for ( i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
          sts = aecDTM_deleteFile ( srfP->pntf[i] );

        for ( i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
          sts = aecDTM_deleteFile ( srfP->ftrf[i] );

        if ( sts == SUCCESS )
          if ( ( sts = aecDTM_deleteFile ( srfP->rngf ) ) == SUCCESS )
            if ( ( sts = aecDTM_deleteFile ( srfP->tinf ) ) == SUCCESS )
               if ( ( sts = aecDTM_deleteFile ( srfP->styf ) ) == SUCCESS )
                  if ( ( sts = aecDTM_deleteFile ( srfP->payf ) ) == SUCCESS )
                     if ( ( sts = aecDTM_deleteFile ( srfP->corf ) ) == SUCCESS )
                        if ( ( sts = aecDTM_deleteFile ( srfP->cmpf ) ) == SUCCESS )
                           if ( ( sts = aecDTM_deleteFile ( srfP->cmpMemf ) ) == SUCCESS )
                           {
                              if ( prjP == (struct CIVdtmprj *)0 && srfP->prj != NULL )
                                 prjP = srfP->prj;

                              if ( prjP != (struct CIVdtmprj *)0 )
                                 sts = aecDTM_removeSurfaceFromProject ( prjP, srfP );

                              if ( sts == SUCCESS )
                                 delete srfP;
                              else
                                 srfP = (struct CIVdtmsrf *)0;
                        }
      }
      else
      {
        sts = aecDTM_hashFeatureCreate ( srfP );
      }

  return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteFile
 DESC: Deletes all the memory associated with a single DTM file.
 HIST: Original - tmi 24-Oct-1990
 MISC:
 KEYW: DTM FILE MEMORY DELETE FREE DEALLOCATE
-----------------------------------------------------------------------------%*/

static int aecDTM_deleteFile        /* <= TRUE if error                       */
(
  struct CIVdtmfil *filP            /* => file to delete                      */
)
{
  int sts = SUCCESS;

  if ( filP )
    if ( ( sts = aecDTM_deallocateFile ( filP ) ) == SUCCESS )
      free ( filP );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_removeSurface
 DESC: Removes point and triangle memory associated with a surface. The
       basic surface memory is kept.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM SURFACE REMOVE
-----------------------------------------------------------------------------%*/

static int aecDTM_removeSurface        /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP               /* => surface to remove                */
)
{
  int sts = SUCCESS, i;

  for ( i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
    sts = aecDTM_deallocateFile ( srfP->pntf[i] );

  for ( i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
    sts = aecDTM_deallocateFile ( srfP->ftrf[i] );

  if ( sts == SUCCESS )
    if ( ( sts = aecDTM_deallocateFile ( srfP->rngf ) ) == SUCCESS )
      if ( ( sts = aecDTM_deallocateFile ( srfP->tinf ) ) == SUCCESS )
        if ( ( sts = aecDTM_deallocateFile ( srfP->styf ) ) == SUCCESS )
          if ( ( sts = aecDTM_deallocateFile ( srfP->payf ) ) == SUCCESS )
            if ( ( sts = aecDTM_deallocateFile ( srfP->corf ) ) == SUCCESS )
              if ( ( sts = aecDTM_deallocateFile ( srfP->cmpf ) ) == SUCCESS )
                sts = aecDTM_deallocateFile ( srfP->cmpMemf );

  if ( sts == SUCCESS )
    sts = aecDTM_deallocateTriangleStack ( srfP );

  if ( sts == SUCCESS )
    aecDTM_hashFeatureDestroy ( srfP );

  aecDTM_destroyCorridorsComponentsMembersIndexes ( srfP );

  aecDTM_setSurfaceModifiedFlag ( srfP );
  aecDTM_clearSurfaceTinOutOfDateFlag ( srfP );

  return ( sts );
}
