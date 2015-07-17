//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* hshfnc.h                                            aec        04/1994     */
/*----------------------------------------------------------------------------*/
/* Various hashing functions.                                                 */
/*----------------------------------------------------------------------------*/
#pragma once


/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecHash_create          /* <= TRUE if error                    */
    (
    void **tblPP,                        /* <= caller must aecHash_destroy      */
    int (*cmpFuncP)( const void *, const void * )  /* => compare function      */
    );

void aecHash_destroy
    (
    void *tblP                           /* => hash table                       */
    );

int aecHash_insert          /* <= TRUE if error                    */
    (
    void *tblP,                          /* => hash table                       */
    void *eleP                           /* => element to insert                */
    );

void *aecHash_find          /* <= found element                    */
    (
    void *tblP,                          /* => hash table                       */
    void *keyP                           /* => element to find                  */
    );

