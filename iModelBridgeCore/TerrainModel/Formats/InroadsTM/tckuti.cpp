//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

#include "stdafx.h"
#include "tckfnc.h"

#define TIME_INCREMENT              1
#define RESPONSE_TIME_INCREMENT     1

static short dots, state;
static unsigned long prvtim;
static unsigned long responsePrvtim;
static wchar_t processMessage[24];
static BOOL s_bSuppress = FALSE;

void aecTicker_initialize( void )
    {
    if( !s_bSuppress )
        {
        unsigned char first = TRUE;

        if ( first )
            {
            first = FALSE;
            aecOutput_getMessage ( processMessage, AEC_S_PROCES, 0 );
            }

        prvtim = (unsigned long) time ( NULL );
        dots = state = 1;
        }

    responsePrvtim = (unsigned long) time ( NULL );
    }

void aecTicker_show( void )
    {
#ifdef DHTODO
    if( !s_bSuppress )
        {
        if ( state )
            {
            unsigned long curtim = (unsigned long) time ( NULL );
            if ( curtim - prvtim >= TIME_INCREMENT )
                {
                if ( dots > 10 ) 
                    dots = 1;
                wchar_t string[40];
                swprintf ( string, L"%s %.*s", processMessage, dots++, L".........." ); /* DO_NOT_TRANSLATE */
                aecOutput_status ( AEC_M_USELITERAL, string );
                prvtim = curtim;
                }
            }
        }

    unsigned long curtim = (unsigned long) time ( NULL );
    if ( curtim - responsePrvtim >= RESPONSE_TIME_INCREMENT )
        {
        MSG msg; 
        ::PeekMessage ( &msg, NULL, 0, 0, PM_NOREMOVE );
        responsePrvtim = curtim;
        }
#endif
    }

void aecTicker_stop( void )
    {
    if( !s_bSuppress )
        {
        aecOutput_status ( AEC_M_BLANK );
        state = 0;
        }
    }


