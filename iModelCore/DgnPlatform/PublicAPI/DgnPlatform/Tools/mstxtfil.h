/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/mstxtfil.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#define TEXTFILE_READ           0
#define TEXTFILE_WRITE          1
#define TEXTFILE_APPEND         2

#define TEXTFILE_DEFAULT            0x0
#define TEXTFILE_KEEP_NEWLINE       0x1 /* only for use with mdlTextFile_getString */
#define TEXTFILE_NO_NEWLINE         0x1 /* only for use with mdlTextFile_putString */
#define TEXTFILE_NEWLINES_TO_LINES  0x2

/*------------------------------------------------------------------------*//**
* Opens a text file for use with ~mmdlTextFile_getString and ~mdlTextFile_putString.
*   This function has been superceded by the BeTextFile class
* @Param        fileName IN name of file to open
* @Param        option   IN File open mode. Appropriate values are:
                        <table border="1" bordercolorlight="#FFFFFF" bordercolordark="#000000" cellpadding="5" cellspacing="0">
                          <caption></caption>
                          <tr bgcolor="#CCCCCC">
                            <th><a name="525263"> </a><div class="CellBody">openMode value</div></th>
                            <th><a name="525265"> </a><div class="CellBody">Meaning</div></th>
                          </tr>
                          <tr>
                            <td><a name="525267"> </a><div class="CellBody">TEXTFILE_READ</div></td>
                            <td><a name="525269"> </a><div class="CellBody">Open the file for read access only. If the file does not exist or cannot be opened the call will fail.</div></td>
                          </tr>
                          <tr>
                            <td><a name="525271"> </a><div class="CellBody">TEXTFILE_WRITE</div></td>
                            <td><a name="525273"> </a><div class="CellBody">Open the file for write access. If the specified file does not exist it will be created. If the file does exist it will be truncated.</div></td>
                          </tr>
                          <tr>
                            <td><a name="525275"> </a><div class="CellBody">TEXTFILE_APPEND</div></td>
                            <td><a name="525277"> </a><div class="CellBody">Opens the file for writing at the end of the file. If the file does not exist it will be created.</div></td>
                          </tr>
                        </table>
* @Return       a pointer to the newly opened file or NULL if the requested operation was not successful.
* @Remarks      Although this function performs essentially the same operation as the
*               standard C run-time library function fopen, the pointer returned by
*               mdlTextFile_open is not guaranteed to be suitable for use with fputs or fgets.
* @ALinkJoin    usmthmdlTextFile_closeC usmthmdlTextFile_getStringC usmthmdlTextFile_putStringC usmthfopenC
* @Group        "Text Files"
* @bsimethod

+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT FILE *mdlTextFile_open (WCharCP fileName, int option);

/*------------------------------------------------------------------------*//**
* Closes a file that was opened by ~mmdlTextFile_open.
*   This function has been superceded by the BeTextFile class
* @Param        stream  IN  FILE previously opened by mdlTextFile_open
* @Return       SUCCESS if the operation was successful or a non-zero error code.
* @ALinkJoin    usmthmdlTextFile_openC usmthmdlTextFile_getStringC usmthmdlTextFile_putStringC usmthfcloseC
* @Group        "Text Files"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int mdlTextFile_close (FILE *stream);

/*------------------------------------------------------------------------*//**
* Gets a string from a file opened with ~mmdlTextFile_open.
*               This function is similar to the standard C run-time
*               library function fgets but it provides more flexible handling of line termination.
*               Characters are read from the input stream, stream, and copied to string until:
*    1.  Any type of newline is encountered (CR, LF, CR/LF).
*    2.  End-of-file is reached, or:
*    3.  maxLen-1 characters have been read without encountering end-of-file or a newline.
*   This function has been superceded by BeTextFile::GetLine()
*
* @Param        string  OUT Output string
* @Param        stream  IN  File to read from; must be pointer to an ASCII text file open by a previous call to ~mmdlTextFile_open.
* @Param        option  IN  Newline option
* @Return       the value of stringP upon successful completion. If end-of-file is encountered
*               before any characters have been read then a NULL pointer is returned
*               and the content of the buffer pointed to by stringP is unchanged.
* @Remarks      The output string is NULL terminated.
* @Remarks      If the input is terminated because of a newline and the value of option is
*               set to TEXTFILE_KEEP_NEWLINE a newline character `\n' is appended to the
*               character string, directly preceding the terminating NULL character.
* @Remarks      If the value of option is set to TEXTFILE_DEFAULT (zero) the newline
*               character is not appended to string.
* @ALinkJoin    usmthmdlTextFile_openC usmthmdlTextFile_closeC usmthmdlTextFile_putStringC usmthfgetsC
* @Group        "Text Files"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT WCharCP mdlTextFile_getString (WStringR string, FILE *stream, int option);

/*------------------------------------------------------------------------*//**
* Puts a string out to a file opened with ~mmdlTextFile_open.
*               This funciton is similar to the standard C run-time
*               library function fputs. All characters except the terminating NULL from the input
*               string, are written to the output stream.  The string will be converted to Locale-encoded characters.
*   This function has been superceded by BeTextFile::PutLine()
* @Param        string  OUT  output string
* @Param        stream  IN  file to wrtie to; must be pointer to an ASCII text file open by a previous call to ~mmdlTextFile_open.
* @Param        option IN newline option
* @Return       If successful, mdlTextFile_putString returns a non-negative value. If an error occurs, mdlTextFile_putString returns EOF.
* @Remarks      If the value of option is set to TEXTFILE_DEFAULT (zero) then the
*               appropriate newline character(s) (CR, LF, CR/LF) for the current platform
*               and operating system will also be appended to the output stream.
* @Remarks      If the value of option is set to TEXTFILE_NO_NEWLINE, no terminating
*               newline character(s) will be written to the output stream.
* @ALinkJoin usmthmdlTextFile_openC usmthmdlTextFile_closeC usmthmdlTextFile_getStringC usmthfputsC
* @Group        "Text Files"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int mdlTextFile_putString (WCharCP string, FILE *stream, int option);
