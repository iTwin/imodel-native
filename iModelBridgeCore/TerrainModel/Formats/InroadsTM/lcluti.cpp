//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* lcluti.c                                            tmi    07-Dec-1993     */
/*----------------------------------------------------------------------------*/
/* Several internationalization and localization utilities.                   */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <locale.h>
#include <lclfnc.h>





/*%-----------------------------------------------------------------------------
 FUNC: aecLocale_getActiveCodePage
 DESC: Returns the active code page.
 HIST: Original - tmi 07-Dec-1993
 MISC:
 KEYW: INTERNATIONALIZATION LOCALE CODE PAGE GET
-----------------------------------------------------------------------------%*/

int aecLocale_getActiveCodePage /* <= TRUE if error          */
(
  unsigned long *codePageP              /* <= active code page id       */
)
{
  int sts = SUCCESS;

  if ( codePageP == (unsigned long *)0 )
    sts = AEC_E_INVDAT;
  else
    *codePageP = 0L; /*** fix this - assign active code page id ***/

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecLocale_verifyCodePage
 DESC: Checks the input code page to determine if it matches the currently
       active code page.
 HIST: Original - tmi 07-Dec-1993
 MISC:
 KEYW: INTERNATIONALIZATION LOCALE CODE PAGE VERTIFY
-----------------------------------------------------------------------------%*/

int aecLocale_verifyCodePage /* <= TRUE if doesn't match     */
(
    unsigned long codePage                /* => code page id to check     */
)
{
    int sts = SUCCESS;

    unsigned long EXTERN_CPage;

    if ( ( sts = aecLocale_getActiveCodePage ( &EXTERN_CPage ) ) == SUCCESS )
        if ( codePage != EXTERN_CPage )
            sts = AEC_E_INVCDP;

    return ( sts );
}
