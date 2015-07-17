//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include "txtsiz.h"
#include "aecuti.h"

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

#define  DTMEXT    8

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecParams_getFeatureState /* <= feature state                  */
    (
    int *state                           /* <= state: 0 or 1 (or NULL)          */
    );

void aecParams_setStatusState
    (
    int *state                           /* => state: 0, 1, or -1               */
    );

int aecParams_getStatusState /* <= state: 0 or 1                   */
    (
    int *state                           /* <= state: 0 or 1 (or NULL)          */
    );

LPWSTR aecParams_getFileExtension /* <= file extension              */
    (
    int  ext                             /* => file extension to get            */
    );

void aecParams_getLinearUnits
    (
    int *unitsP,                         /* => unitsP (or NULL)                 */
    int *precisionP                      /* => precision (or NULL)              */
    );

int aecParams_getX          /* <= precision                        */
    (
    int *precisionX                      /* <= precision                        */
    );

int aecParams_getZ          /* <= precision                        */
    (
    int *precisionZ                      /* <= precision                        */
    );

void aecParams_getProductNameAndID
    (
    LPWSTR nameP,                         /* => product name                     */
    LPWSTR appNameP,                      /* => application name                 */
    int  *idP                            /* => product id                       */
    );
