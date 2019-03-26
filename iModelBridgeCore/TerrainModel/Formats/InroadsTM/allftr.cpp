//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* allftr.c                                        twl    27-Oct-1998         */
/*----------------------------------------------------------------------------*/
/* Utilities to send all (or some) features to a user-specified function.     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h" 



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllFeatures
 DESC: When it is necessary to loop through all features in a surface,
       this function allows you to do that.  You have the option to do all
       all features (deleted and nondeleted), or just the nondeleted
       features.  Additionally, you can pass information directly through
       to your routine by passing a pointer to a structure containing your
       information as the 'dat' argument. The format of the function that
       is called is:
              int usrfnc
              (
                void *datP,
                int typ,
                long np,
                DPoint3d *corP,
                struct CIVdtmftr *ftrP
              )
       You can also control which features types are sent to your function.
       Do this using the 'typmsk' argument.  OR the values defined in
       civdtm.h for the types you want.  If 'typmsk' is zero, all types
       will be processed.
 HIST: Original - twl 27-Oct-1998
 MISC:
 KEYW: DTM FEATURES SEND ALL
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllFeatures  /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface with triangles           */
  int opt,                             /* => options                          */
  int typmsk,                          /* => point type (zero for all)        */
  int (*usrfncP)(void *,               /* => your function                    */
       struct CIVdtmsrf *,int,
       struct CIVdtmftr *),
  void *datP                           /* => your data                        */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmftr *fP;
  long ftrTyp;
  int sts = SUCCESS;
  int i;

  for ( i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
    if ( !typmsk  ||  ( 1 << i & typmsk ) )
    {
      for ( blkP = srfP->ftrf[i]->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
        for ( fP = blkP->rec.ftr; fP < blkP->rec.ftr + blkP->use  &&  sts == SUCCESS; fP++ )
        {
          if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
            sts = DTM_M_PRCINT;
          else if ( opt & DTM_C_DELETE  ||  !aecDTM_isFeatureDeletedFlagSet ( fP ) )
          {
      
            aecDTM_findFeatureType ( &ftrTyp, NULL, srfP, fP );
            aecTicker_show();
            sts = (*usrfncP)( datP, srfP, ftrTyp, fP);
          }
        }
    }

  return ( sts );
}


