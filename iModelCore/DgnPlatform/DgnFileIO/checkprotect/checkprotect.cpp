/*----------------------------------------------------------------------+
|
|   $Source: DgnFileIO/checkprotect/checkprotect.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <Mstn/MicroStationAPI.h>

#include <stdlib.h>
#include <string.h>

#include <DgnPlatform/DesktopTools/envvutil.h>

#include <MsjInternal/MdlUtil/optmerge.h>
#include <MsjInternal/MdlUtil/stdprt.h>

#include <ToolsGenSrc/prg.h> // Must include this! We must be able to detect a PRG build!

USING_NAMESPACE_BENTLEY

/*----------------------------------------------------------------------+
|                                                                       |
|   Local Defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define MSG_Syntax          0
#define MSG_Protected       1
#define MSG_NotProtected    2
#define MSG_AccessViolation 3
#define MSG_NotV8File       4
#define MSG_UnknownOpenError 5
/*----------------------------------------------------------------------+
|                                                                       |
|   Typedefs                                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global Variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
char s_msg[512];

/*----------------------------------------------------------------------+
|                                                                       |
|   External Variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static char*   getMessage
(
int             msgno
)
    {
    switch (msgno)
        {
        default:
            BeAssert (false);

        case MSG_Syntax:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_SYNTAX", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "usage:\n\
\tcheckprotect [-v] <v8filename>\n\
where:\n\
\t<v8filename>\tname of V8 design file to check\n\
\t\t-v\tverbose");
            break;

        case MSG_Protected:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_PROTECTED", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "%hs : Protected\n");
            break;

        case MSG_NotProtected:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_NOT_PROTECTED", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "%hs : Not Protected\n");
            break;

        case MSG_AccessViolation:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_ACCESS_VIOLATION", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "%hs : ERROR : Sharing or access violation\n");
            break;

        case MSG_NotV8File:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_NOT_V8_FILE", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "%hs : Not a V8 file\n");
            break;

        case MSG_UnknownOpenError:
            if (util_getSysEnv (s_msg, "CHECKPROTECT_MSG_UNKNOWN_OPEN_ERROR", sizeof(s_msg)) != SUCCESS)
                strcpy (s_msg, "%hs : ERROR : unknown error %x when attempting to open design file\n");
            break;
        }

    return s_msg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
#include <MsjInternal/MdlUtil/ignoreseg.h>
static void    syntax
(
)
    {
    IGNORESEG static char version_comptools[] = VERSION_COMPTOOLS;
    optMerge_displayVersion (version_comptools, "Microstation File Protection Checking Utility");
    fprintf (stderr, getMessage (MSG_Syntax));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt parseCmdLine
(
char*           filename,       // <=
bool*        verbose,        // <=
int             argc,
char**          argv
)
    {
    char        ch;
    char        *parg;

    *verbose = false;
    *filename = '\0';

    /*-----------------------------------------------------------------------------------
        Parse args
    -----------------------------------------------------------------------------------*/
    argv++;                 /*  Step over command name          */
    while (--argc)
        {
        /*-------------------------------------------------------------------------------
            command line switch
        -------------------------------------------------------------------------------*/
        if (*(parg = *argv) == '-' || *parg == '/')
            {
            parg++;                 /*  Step over option signifier */
            switch (ch = *parg++)
                {
                default:
                    fprintf (stderr, "%hs : ?\n", *argv);
                    syntax ();
/*<=*/              return ERROR;

                /*-----------------------------------------------------------------------
                    -? Help
                -----------------------------------------------------------------------*/
                case '?':
                    syntax ();
/*<=*/              return ERROR;

                /*-----------------------------------------------------------------------
                    -v
                -----------------------------------------------------------------------*/
                case 'v':
                    *verbose = true;
                    break;
                }
            }
        else
            {
            /*---------------------------------------------------------------------------
                Not a switch
            ---------------------------------------------------------------------------*/
            if (*filename)
                {
                fprintf (stderr, "%hs : ?\n", *argv);
                syntax ();
/*<=*/          return ERROR;
                }

            strcpy (filename, parg);
            }

        argv++;
        }

    if (!*filename)
        {
        syntax ();
/*<=*/  return ERROR;
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      main                                                        |
|                                                                       |
| author    SamWilson                                   02/02           |
|                                                                       |
+----------------------------------------------------------------------*/
void        main
(
int         argc,
char        **argv
)
    {
    /*-----------------------------------------------------------------------------------
        Get the args
    -----------------------------------------------------------------------------------*/
    argv = optMerge_mergeArgLists ( "CHECKPROTECT_ARGS", argc, argv, &argc );

//    optMerge_displayVersion (version_comptools, "Microstation File Protection Checking Utility");

    bool    verbose = false;
    char    filename[MAXFILELENGTH];

    if (parseCmdLine (filename, &verbose, argc, argv) != SUCCESS)
        {
/*<=*/  exit (0);
        }

    /*-----------------------------------------------------------------------------------
        Open and check the file
    -----------------------------------------------------------------------------------*/
    BeFileName wName (filename);
    DgnProjectP     file = DgnFile::Create (wName, DgnFileOpenMode::ReadOnly);

    StatusInt s = file->LoadDgnFile(NULL);

    if (DGNOPEN_STATUS_IsEncrypted == s)
        {
        if (verbose)
            printf (getMessage (MSG_Protected), filename);

        exit (1);
        }

    if (SUCCESS == s)
        {
        if (verbose)
            printf (getMessage (MSG_NotProtected), filename);
        }
    else
    if (DGNOPEN_STATUS_SharingViolation == s
     || DGNOPEN_STATUS_AccessViolation == s
     || DGNOPEN_STATUS_AlreadyOpen == s)
        {
        fprintf (stderr, getMessage (MSG_AccessViolation), filename);
        }
    else
    if (DGNOPEN_STATUS_UnrecognizedFormat == s)
        {
        fprintf (stderr, getMessage (MSG_NotV8File), filename);
        }
    else
        {
        fprintf (stderr, getMessage (MSG_UnknownOpenError), filename, s);
        }

    exit (0);
    }
