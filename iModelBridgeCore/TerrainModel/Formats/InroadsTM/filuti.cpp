//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* filuti.c                                            tmi    06-Jan-1993     */
/*----------------------------------------------------------------------------*/
/* Various file related utilities.                                            */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "filfnc.h"

//
// We DO NOT need to call _wmakepath directly.  It assumes complete pathnames can be 260 (which 
// is the Windows NT limit) but we are using 256 so we do not have to translate .alg files.  
//
// We will robably need to check other calls that generate file/path names
//
static void _AECwmakepath
(
wchar_t *path, 
const wchar_t *drive, 
const wchar_t *dir, 
const wchar_t *fname, 
const wchar_t *ext 
)
{
    wchar_t wcsFileName[512];
    _wmakepath( wcsFileName, drive, dir, fname, ext );

    size_t len = wcslen(wcsFileName);
    if (len >= CIV_MAX_PATH)
    {
        // we gotta do somethin
        wcsncpy(path, wcsFileName, CIV_MAX_PATH - 1);
    }
    else
    {
        wcscpy(path, wcsFileName);
    }
}


/*%-----------------------------------------------------------------------------
 FUNC: aecFile_checkExtension
 DESC: Takes a filename and an extension and determines if the filename
       already has the extension on it. If it doesn't then the extension is
       added to the filename. Note that the string holding the filename
       should be long enough to hold the filename and the extension.
 HIST: Original - tmi 20-Jan-1991
 MISC:
 KEYW: FILE EXTENSION CHECK
-----------------------------------------------------------------------------%*/

void aecFile_checkExtension
(
LPWSTR fileNameP,                     /* <=> string with file name           */
LPWSTR extensionP                     /*  => string with extension           */
)
{
    int i,j;
    wchar_t *origExtP = NULL, *sysExtP = NULL;

    wchar_t wcsDrive[CIV_MAX_DRIVE];
    wchar_t wcsDir[CIV_MAX_DIR];
    wchar_t wcsFname[CIV_MAX_FNAME];
    wchar_t wcsExt[CIV_MAX_EXT];

    memset( wcsDrive, 0, sizeof( wcsDrive ) );
    memset( wcsDir,   0, sizeof( wcsDir ) );
    memset( wcsFname, 0, sizeof( wcsFname ) );
    memset( wcsExt,   0, sizeof( wcsExt ) );

    _wsplitpath( fileNameP, wcsDrive, wcsDir, wcsFname, wcsExt );

    // If it has an extension, verify that it is correct
    if (wcslen(wcsExt))
    {
        origExtP = extensionP;
        for ( i = 0; (*origExtP == L'*' || *origExtP == L'.') && i < CIV_MAX_EXT; i++, origExtP++ );

        sysExtP = wcsExt;
        for ( j = 0; (*sysExtP == L'*' || *sysExtP == L'.') && j < CIV_MAX_EXT; j++, sysExtP++ );

        if (wcscmp(origExtP, sysExtP) != 0)
        {
            _AECwmakepath( fileNameP, wcsDrive, wcsDir, wcsFname, sysExtP );
        }
    }
    else
    {
        // remove the "*" and/or "." from the extension if it exists
        origExtP = extensionP;
        for ( i = 0; (*origExtP == L'*' || *origExtP == L'.') && i < CIV_MAX_EXT; i++, origExtP++ );

        // Does not have an extension, so add it
        _AECwmakepath( fileNameP, wcsDrive, wcsDir, wcsFname, origExtP );
    }

    return;
}
