/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/fileutil/textfile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <string.h>
#include    <DgnPlatform/Tools/mstxtfil.h>

USING_NAMESPACE_BENTLEY_DGN

static WCharCP s_openOptions[] = {L"rb", L"wb", L"ab"};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        12/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public FILE     *mdlTextFile_open
(
WCharCP         fileName,   /* => Name of file to open */
int             option      /* => Open options */
)
    {
    FILE        *pFile;
    WString     wOption;
    BeFileSharing sh = BeFileSharing::None;
    
    if (option <= TEXTFILE_APPEND)
        {
        wOption = s_openOptions[option];
        sh = BeFileSharing::Read;
        }
    else
        return NULL;
    
    pFile = BeFile::Fsopen (fileName, wOption.c_str(), sh);

    return pFile;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlTextFile_close                                       |
|                                                                       |
|   Currently this function is no different from fclose. It should be   |
|   used with all mdlTextFile_ functions in case we need to make changes|
|   to the behavior fclose for some platform or type of text file in    |
|   the future.                                                         |
|                                                                       |
| author        JimBartlett                             2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      mdlTextFile_close
(
FILE            *stream
)
    {
    return fclose (stream);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlTextFile_getString                                   |
|                                                                       |
|   This function works similar to fgets. Characters are read from the  |
|   input stream until, 1) any type of newline is seen (CR, LF, CR/LF), |
|   2) end-of-file is reached or 3) maxLen-1 characters have been read  |
|   without encountering end-of-file or a newline. A null character is  |
|   then appended to the string.                                        |
|   If the input is terminated because of a newline and the             |
|   the TEXTFILE_KEEP_NEWLINE option is used, the newline character     |
|   is stored in the array just before the terminating NULL character.  |
|   The argument "string" is returned upon successful completion.       |
|                                                                       |
|   If end-of-file is encountered before any characters have been read  |
|   then a NULL pointer is returned and the contents of the output      |
|   string are unchanged.                                               |
|                                                                       |
| author        JimBartlett                             2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public WCharCP  mdlTextFile_getString
(
WStringR        outstring,   /* <= String read from file               */
FILE            *stream,     /* => File opened by mdlTextFile_open     */
int             option       /* => Read options (see mstxtfil.h)       */
)
    {
    char        string[4096];
    size_t      maxLen = _countof(string);
    char        *pChr, *pEnd;
    int         inChar, nextChar = 0;

    if (maxLen < 2 || (inChar = fgetc (stream)) == EOF || inChar == CNTRL_Z)
        return  NULL;

    pChr = string;
    pEnd = pChr + (maxLen-1);

    do
        {
        if (inChar != CR && inChar != LF)
            {
            *pChr++ = (char)inChar;
            }
        else
            {
            /*--------------------------------------------------------------
            Check next char. If its a CR or LF, eat it. If not, put it back.
            --------------------------------------------------------------*/
            if (inChar == CR)
                {
                nextChar = fgetc (stream);
                if (nextChar != LF)
                    ungetc (nextChar, stream);
                }
            /*-----------------------------------------------------------
            If requested, add the '\n' to the end like fgets() except if
            we are mapping line breaks to a line with a single space as
            per TEXTFILE_NEWLINES_TO_LINES.  In this we keep the newline
            as a line.  Using the actual newline itself causes glyphs
            to be displayed for the char codes which are non-displayable
            and thus means we get undefined char glyphs.
            -----------------------------------------------------------*/
            if (option & TEXTFILE_KEEP_NEWLINE)
                if (!(option & TEXTFILE_NEWLINES_TO_LINES))
                    *pChr++ = '\n';
                else if (pChr == string)
                    *pChr++ = ' ';

            break;
            }

        } while (pChr < pEnd && (inChar = fgetc (stream)) != EOF && inChar != CNTRL_Z);

    *pChr = '\0';

    outstring.AssignA(string);
    return  outstring.c_str();
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlTextFile_putString                                   |
|                                                                       |
|   This function works similar to fputs. All the characters in         |
|   "string" except the terminating null character are written to the   |
|   output stream.                                                      |
|   The appropriate newline (CR, LF or CR/LF or VAX goop) will also be  |
|   written to the output stream unless the TEXTFILE_NO_NEWLINE option  |
|   is used.                                                            |
|                                                                       |
|   If an error occurs, mdlTextFile_putString returns EOF; otherwise it |
|   returns the last character written to the output stream.            |
|                                                                       |
| author        JimBartlett                             2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      mdlTextFile_putString
(
WCharCP         string,      /* => String to be written                */
FILE            *stream,     /* => File opened by mdlTextFile_open     */
int             option       /* => Write options (see mstxtfil.h)      */
)
    {
    // This always writes locale-encoded strings because mdlTextFile_open does not write unicode inducers.
    //  Use BeTextFile for UTF-8 and UTF-16 encodings.
    WString wstr(string);
    size_t localeSize = wstr.GetMaxLocaleCharBytes ();
    
    CharP localeString = (CharP)_alloca (localeSize);
    wstr.ConvertToLocaleChars (localeString);

    char        *pChar = localeString;
    int         status = SUCCESS;

    while (*pChar && (status = fputc ((int)*pChar, stream)) != EOF)
        pChar++;

    if (status != EOF && !(option & TEXTFILE_NO_NEWLINE))
        {
        if ((status = fputc (CR, stream)) != EOF)
            status = fputc (LF, stream);
        }

    return  status;
    }

