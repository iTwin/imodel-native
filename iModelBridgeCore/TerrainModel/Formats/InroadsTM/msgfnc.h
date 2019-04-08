//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* msgfnc.h                                          aec    06-Jan-1993       */
/*----------------------------------------------------------------------------*/
/* Message related functions.                                                 */
/*----------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------------*/
/* Include file dependencies.                                                 */
/*----------------------------------------------------------------------------*/
#include <aecuti.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes.                                                       */
/*----------------------------------------------------------------------------*/

void aecOutput_blankAllFields
    (
    void
    );

int aecOutput_alert         /* <= TRUE if ok button picked         */
    (
    int msgId,                           /* => message number                   */
    wchar_t *inp                         /* => argument for message             */
    );

void aecOutput_setMessage
    (
    int msgNumber,                       /* => message number                   */
    wchar_t *msgString                   /* => message string                   */
    );

wchar_t *aecOutput_getMessageString /* <= returned message            */
    (
    unsigned long msgId                  /* => Id of message-string             */
    );

wchar_t *aecOutput_getMessage  /* <= returned message                 */
    (
    wchar_t *msg,                        /* <= msg from rsc. file (or NULL)     */
    unsigned long msgId,                 /* => Id of message-string             */
    int doFmt,                           /* => 1 == Treat as form. string       */
    ...                                  /* => Other parameters                 */
    );

void aecOutput_status
    ( 
    int msgId,                           // => msg number
    ...                                  // => other, optional args.
    );

int aecOutput_msgbox
    ( 
    int msgId,                           // => msg number
    int msgType,                         // => is this a yes/no, ok/cancel, ...
    ...                                  // => other, optional args.
    );

//DHTODO
//void SetResourceStringHook
//(
//    int (*pAfxLoadString)(UINT,LPWSTR,UINT)
//);
//
