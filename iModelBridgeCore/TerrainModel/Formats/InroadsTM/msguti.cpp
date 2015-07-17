//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* msguti.mc                                     pr    01-Nov-1992            */
/*----------------------------------------------------------------------------*/
/* Various utilities for outputing messages.                                  */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define    AECMSGSIZE          512     /* global message size                 */

/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static wchar_t aecMessage[AECMSGSIZE];
static int (__stdcall *s_pAfxLoadString)(UINT,LPWSTR,UINT) = NULL;


BOOL IsUstn()
    {
    return TRUE;
    }

void SetResourceStringHook
    (
    int (__stdcall *pAfxLoadString)(UINT,LPWSTR,UINT)
    )
    {
    s_pAfxLoadString = pAfxLoadString;
    }

//---------------------------------------------------------------------------
// DESC: Takes the variant argument list and the message id and converts it
//       to a string value
// HIST: Original - dakloske - 10/98
// MISC: Pass in a message id of 0 to use the literal string
//       Pass in any other message id to look it up in the resource file.
//---------------------------------------------------------------------------
static int aecOutput_valistToString
    (
    wchar_t *pstrOutput,
    int msgId,                           // => message number
    BOOL bDoFmt,                         // => use the valist?
    va_list ap                           // => varible argument list
    )
    {
    int sts = SUCCESS;

    if ( msgId == SUCCESS )
        {
        // Clear the message.
        pstrOutput[0] = '\0';
        }
    else if ( msgId == AEC_M_USELITERAL )
        {
        // First Argument is the string
        wchar_t *pLiteral = va_arg ( ap, wchar_t * );
        if ( pLiteral )
            wcscpy ( pstrOutput, pLiteral );
        else
            sts = ERROR;
        }
    else
        {
        wchar_t fmt[AECMSGSIZE];
        memset ( fmt,  0, sizeof(fmt) );

        // Load the string into pstrOutput
        if ( s_pAfxLoadString )
            (*(int (__stdcall *)(UINT,LPWSTR,UINT))s_pAfxLoadString)( (UINT) msgId, fmt, AECMSGSIZE );

        // If it was a CAD-Dependant message, swap it out
        //XplCallFunction (LIB_CIVUTIUI, "aecUtil_MessageStringReplace", msgId, fmt, TRUE);

        // See if the arguments need to be re-ordered for i18n purporses
        //
        //      This isn't going to work the way we used to do it - we used
        //      to open an rsc file and look for a particular datastructure telling
        //      us which args were what and swap their orders.  We need to do it
        //      the way that Imagineer does it with the "%1$d, %2$d" style of formatting
        //      if we truely need to swap arguments (which I'm sure we will when someone
        //      is ready for an internationalized version).   We'll write it if/when
        //      that need arises.  Until then... do nothing.

        // Put everything together
        if ( bDoFmt )
            vswprintf ( pstrOutput, fmt, ap );
        else
            wcscpy ( pstrOutput, fmt );
        }

    return sts;
    }


//---------------------------------------------------------------------------
// DESC: Sends the message to the status-bar on the civil explorer
// HIST: Original - dakloske - 10/98
// MISC: 
//---------------------------------------------------------------------------
void aecOutput_status
    ( 
    int msgId,                           // => msg number
    ...                                  // => other, optional args.
    )
    {
    wchar_t wideOutput[AECMSGSIZE];
    va_list ap;
    va_start (ap, msgId);

    if (IsUstn())
        if ( aecOutput_valistToString ( wideOutput, msgId, TRUE, ap ) == SUCCESS )
            {
            //mdlOutput_status ( wideOutput );
            }

        va_end ( ap );
    }


//---------------------------------------------------------------------------
// DESC: Brings up a message box.
// HIST: Original - dakloske - 10/98
// MISC: Valid values for msgType are:
//              MB_OK
//              MB_OKCANCEL
//              MB_RETRYCANCEL
//              MB_YESNO
//              MB_YESNOCANCEL 
//      
//       Possible Return Values are:
//              ACTIONBUTTON_OK         - Yes, Ok, Retry
//              ACTIONBUTTON_NO         - No
//              ACTIONBUTTON_CANCEL     - anything else
//---------------------------------------------------------------------------
static int aecOutput_msgboxVAList
    ( 
    int msgId,                           // => msg number
    int msgType,                         // => is this a yes/no, ok/cancel, ...
    va_list ap                           // => other, optional args.
    )
    {
#ifdef DHTODO
    wchar_t wideOutput[AECMSGSIZE];
    int nRet = ACTIONBUTTON_CANCEL;

    if ( aecOutput_valistToString ( wideOutput, msgId, TRUE, ap ) == SUCCESS )
        {
        wchar_t szProdName[256];
        HWND hWnd = NULL;
        //		HWND hWndCAD = GetCadWindowHandle();

        aecParams_getProductNameAndID( szProdName, NULL, NULL );

        if ( !msgType )
            msgType = MB_OK;                // User needs at least an OK button
        else if ( !(msgType & (MB_ICONEXCLAMATION|MB_ICONWARNING|MB_ICONINFORMATION|MB_ICONASTERISK|MB_ICONQUESTION|MB_ICONSTOP|MB_ICONERROR|MB_ICONHAND) ) )
            msgType |= MB_ICONEXCLAMATION;  // If no icon supplied by caller, give an exclamation icon by default

        if ( !hWnd )
            msgType |= MB_TOPMOST;

        nRet = MessageBox( hWnd, wideOutput, szProdName, msgType );

        if ( nRet == IDOK || nRet == IDYES || nRet == IDRETRY )
            nRet = ACTIONBUTTON_OK;
        else if ( nRet == IDNO )
            nRet = ACTIONBUTTON_NO;
        else
            nRet = ACTIONBUTTON_CANCEL;
        }

    return nRet;
#endif
    return ACTIONBUTTON_OK;
    }

//---------------------------------------------------------------------------
// DESC: Brings up a message box.
// HIST: Original - dakloske - 10/98
// MISC: Valid values for msgType are:
//              MB_OK
//              MB_OKCANCEL
//              MB_RETRYCANCEL
//              MB_YESNO
//              MB_YESNOCANCEL 
//      
//       Possible Return Values are:
//              ACTIONBUTTON_OK         - Yes, Ok, Retry
//              ACTIONBUTTON_NO         - No
//              ACTIONBUTTON_CANCEL     - anything else
//---------------------------------------------------------------------------
int aecOutput_msgbox
    ( 
    int msgId,                           // => msg number
    int msgType,                         // => is this a yes/no, ok/cancel, ...
    ...                                  // => other, optional args.
    )
    {
    int nRet;

    va_list ap;
    va_start (ap, msgType);

    nRet = aecOutput_msgboxVAList ( msgId, msgType, ap );

    va_end ( ap );

    return nRet;
    }

/*%-----------------------------------------------------------------------------
FUNC: aecOutput_alert
DESC: Opens up the alert box and returns true is ok button was hit.
HIST: Original - tmi 09-Jan-1991
Converted - dak - Oct-1998
MISC:
KEYW: MESSAGE ALERT
-----------------------------------------------------------------------------%*/

int aecOutput_alert         /* <= TRUE if ok button picked         */
    (
    int msgId,                           /* => message number                   */
    wchar_t *inp                            /* => argument for message             */
    )
    {
    int nRet;

    nRet = aecOutput_msgbox( msgId, MB_OKCANCEL, inp );

    return nRet == ACTIONBUTTON_OK;
    }


/*%-----------------------------------------------------------------------------
FUNC: aecOutput_blankAllFields
DESC: Blanks all fields in uStn command window.
HIST: Original - pr 01-Oct-1992
MISC:
KEYW: MESSAGE FIELDS ALL BLANK
-----------------------------------------------------------------------------%*/

void aecOutput_blankAllFields
    (
    void
    )
    {
    //mdlOutput_prompt (L"");
    //mdlOutput_status (L"");
    }


/*%-----------------------------------------------------------------------------
FUNC: aecOutput_setMessage
DESC: Set a message string using this function.
HIST: Original - tmi 06-Jan-1993
MISC:
KEYW: MESSAGE SET
-----------------------------------------------------------------------------%*/

void aecOutput_setMessage
    (
    int msgNumber,                       /* => message number                   */
    wchar_t *msgString                   /* => message string                   */
    )
    {
    if ( msgString != (wchar_t *)0 )
        {
        memset ( aecMessage, 0, sizeof ( aecMessage ) );
        wcsncpy ( aecMessage, msgString, (sizeof(aecMessage)/sizeof(aecMessage[0]))-1 );
        aecMessage[(sizeof(aecMessage)/sizeof(aecMessage[0]))-1] = '\0';
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecOutput_getMessageString
DESC: Gets a specified message from default resource file.
HIST: Original - dgc 01-Oct-1992
MISC:
KEYW: MESSAGE STRING GET
-----------------------------------------------------------------------------%*/

wchar_t *aecOutput_getMessageString         /* <= returned message            */
    (
    unsigned long msgId                  /* => Id of message-string             */
    )
    {
    return ( aecOutput_getMessage ( NULL, msgId, 0 ) );
    }



/*%-----------------------------------------------------------------------------
FUNC: aecOutput_getMessage
DESC: Gets a specified message from default resource file.
HIST: Original - pr 01-Oct-1992
MISC:
KEYW: MESSAGE GET
-----------------------------------------------------------------------------%*/

wchar_t *aecOutput_getMessage  /* <= returned message                 */
    (
    wchar_t *msg,                        /* <= msg from rsc. file (or NULL)     */
    unsigned long msgId,                 /* => Id of message-string             */
    int doFmt,                           /* => 1 == Treat as form. string       */
    ...                                  /* => Other parameters                 */
    )
    {
    // Note doFmt always done
    static wchar_t res[AECMSGSIZE];
    va_list ap;
    va_start ( ap, doFmt );

    res[0] = '\0';
    aecOutput_valistToString ( res, msgId, doFmt, ap );

    va_end ( ap );

    if ( msg ) 
        wcscpy ( msg, res );
    return res;
    }

