//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define TBLBUFSZ 256


/*%-----------------------------------------------------------------------------
FUNC: aecTable_insert
DESC:
HIST: original                                               dgc 9/1992
MISC:
KEYW: TABLE ELEMENT INSERT
----------------------------------------------------------------------------%*/

int aecTable_insert         /* <= TRUE if error                    */
    (
    void **tblPP,                        /* <= caller must free          */
    int *nTblP,                          /* <=                                  */
    void *eleP,                          /* =>                                  */
    int eSize                            /* =>                                  */
    )
    {
    byte *baseP = ( byte * )*tblPP;
    int index = *nTblP;

    if( ( index % TBLBUFSZ ) == 0 )
        {
        if( baseP )
            baseP = (byte *) realloc( baseP, ( index + TBLBUFSZ ) * eSize );
        else
            baseP = (byte *) calloc( TBLBUFSZ, eSize );
        }

    if( !baseP )
        return( AEC_E_MEMALF );

    memcpy( &baseP[index * eSize], eleP, eSize );

    *tblPP = baseP;
    *nTblP = index + 1;

    return( SUCCESS );
    }


