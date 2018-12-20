//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* lclfnc.h                                            aec    07-Dec-1993     */
/*----------------------------------------------------------------------------*/
/* Internationalization and localization (ah, thats i18n & l10n for short)    */
/* related functions.                                                         */
/*----------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecLocale_getActiveCodePage /* <= TRUE if error                */
(
  unsigned long *codePageP             /* <= returned code page               */
);


int aecLocale_verifyCodePage /* <= TRUE if error                   */
(
  unsigned long codePage               /* => code page to check               */
);

