//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* loddtm.c                                         tmi    24-Apr-1990        */
/*----------------------------------------------------------------------------*/
/* Loads a complete .DTM file into memory.                                    */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "cmputi.h"
#include "cmpmemuti.h"
#include "coruti.h"


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define    TMPSRFNAME    L"0x121462"                      /* DO_NOT_TRANSLATE */

/*----------------------------------------------------------------------------*/
/* Extrenal functions                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_loadOpen(wchar_t *,unsigned long *,unsigned long *,FILE **);
static int aecDTM_loadCreateSurface(struct CIVdtmprj *,unsigned long,struct CIVdtmsrf **);
static int aecDTM_loadGUID(struct CIVdtmsrf *, unsigned long, FILE *);
static int aecDTM_loadVerifyName(struct CIVdtmprj *,struct CIVdtmsrf *,wchar_t *,FILE *);
static int aecDTM_loadPreferenceName(struct CIVdtmsrf *, FILE *);
static int aecDTM_loadXSectionSymbologyName(struct CIVdtmsrf *, FILE *);
static int aecDTM_loadProfileSymbologyName(struct CIVdtmsrf *, FILE *);
static int aecDTM_loadProfileOffsets(struct CIVdtmsrf *, FILE *);
static int aecDTM_loadRevision(struct CIVdtmsrf *, FILE *);
static int aecDTM_loadParameters(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadFiles(struct CIVdtmsrf *,unsigned long,FILE *);
static int aecDTM_loadPoints(struct CIVdtmsrf *,unsigned long,FILE *);
static int aecDTM_loadRange(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadTriangles(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadFeatures(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadFeatureStyles(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadCorridors(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadComponents(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadComponentMembers(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadFeaturePayItems(struct CIVdtmsrf *,FILE *);
static int aecDTM_loadCheckFormat(int*, wchar_t*);


/*----------------------------------------------------------------------------*/
/* Private static globals                                                     */
/*----------------------------------------------------------------------------*/
static struct CIVdtmsrf *pDupSrf = NULL;



/*%-----------------------------------------------------------------------------
FUNC: aecDTM_load
DESC: Opens and loads up terrain models based upon type.
HIST: Original - wbw 21-Jan-1994
MISC: static
KEYW: DTM LOAD TYPE CHECK
-----------------------------------------------------------------------------%*/

int aecDTM_load             /* <= TRUE if error                    */
    (
struct CIVdtmsrf **srfPP,            /* <= new, loaded surface              */
    int *fileWasTTN,                     /* <= TRUE if TTN load (or NULL)       */
struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
    wchar_t *fileNameP                   /* => file to load                     */
    )
    {
    struct CIVdtmsrf *srfP = (struct CIVdtmsrf *)0;
    int sts, format;

    pDupSrf = NULL;

    if ( fileWasTTN != (int *)0 ) *fileWasTTN = 0;


    if ( ( sts = aecDTM_loadCheckFormat ( &format, fileNameP ) ) == SUCCESS )
        if ( format == 1 )
            {
            sts = DTM_M_INVDFF;
            }
        else if ( format == 4 )
            {
            sts = DTM_M_INVDFF;
            }
        else
            if ( ( sts = aecDTM_loadDTM ( &srfP, prjP, fileNameP ) ) == SUCCESS )
                {
                srfP->flg &= ~DTM_C_SAVTTN;
                }

            if ( srfPP != (struct CIVdtmsrf **)0 ) *srfPP = srfP;

            return ( sts );
    }



/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadDTM
DESC: Loads a complete .DTM file into memory.
HIST: Original - tmi 24-Apr-1990
MISC:
KEYW: DTM LOAD OPEN
-----------------------------------------------------------------------------%*/

int aecDTM_loadDTM         /* <= TRUE if error                     */
    (
struct CIVdtmsrf **srfPP,           /* => pointer to new loaded srf         */
struct CIVdtmprj *prjP,             /* => DTM project (or NULL)             */
    wchar_t *fileNameP                  /* => file to load                      */
    )
    {
    FILE *handleP;
    struct CIVptrind *piP, *epiP;
    int sts = SUCCESS, nblk;
    wchar_t name[DTM_C_NAMSIZ];
    unsigned long version, codePage;

    aecTicker_initialize();

    if ( ( sts = aecDTM_loadOpen ( fileNameP, &version, &codePage, &handleP ) ) == SUCCESS )
        if ( ( sts = aecDTM_loadCreateSurface ( prjP, version, srfPP ) ) == SUCCESS )
            if ( ( sts = aecDTM_loadGUID ( *srfPP, version, handleP ) ) == SUCCESS )
                if ( ( sts = aecDTM_loadVerifyName ( prjP, *srfPP, name, handleP ) ) == SUCCESS )
                    if ( ( sts = aecDTM_loadPreferenceName ( *srfPP, handleP ) ) == SUCCESS )
                        if ( ( sts = aecDTM_loadXSectionSymbologyName ( *srfPP, handleP ) ) == SUCCESS )
                            if ( ( sts = aecDTM_loadProfileSymbologyName ( *srfPP, handleP ) ) == SUCCESS )
                                if ( ( sts = aecDTM_loadRevision ( *srfPP, handleP ) ) == SUCCESS )
                                    if ( ( sts = aecDTM_loadProfileOffsets ( *srfPP, handleP ) ) == SUCCESS )
                                        if ( ( sts = aecDTM_loadParameters ( *srfPP, handleP ) ) == SUCCESS )
                                            if ( ( sts = aecDTM_loadFiles ( *srfPP, version, handleP ) ) == SUCCESS )
                                                if ( ( sts = aecDTM_loadPoints ( *srfPP, version, handleP ) ) == SUCCESS )
                                                    if ( ( sts = aecDTM_loadRange ( *srfPP, handleP ) ) == SUCCESS )
                                                        if ( ( sts = aecDTM_loadTriangles ( *srfPP, handleP ) ) == SUCCESS )
                                                            if ( ( sts = aecDTM_loadFeatures ( *srfPP, handleP ) ) == SUCCESS )
                                                                if ( ( sts = aecDTM_loadFeatureStyles ( *srfPP, handleP ) ) == SUCCESS )
                                                                    if ( ( sts = aecDTM_loadFeaturePayItems ( *srfPP, handleP ) ) == SUCCESS )
                                                                        if ( ( sts = aecDTM_loadCorridors ( *srfPP, handleP ) ) == SUCCESS )
                                                                            if ( ( sts = aecDTM_loadComponents ( *srfPP, handleP ) ) == SUCCESS )
                                                                                if ( ( sts = aecDTM_loadComponentMembers ( *srfPP, handleP ) ) == SUCCESS )
                                                                                    {
                                                                                    aecDTM_countSurfaceBlocks ( &nblk, *srfPP );
                                                                                    if ( nblk > 0 )
                                                                                        if ( ( piP = ( struct CIVptrind * ) calloc ( (unsigned int)nblk, sizeof(struct CIVptrind) ) ) == 0L )
                                                                                            sts = DTM_M_MEMALF;
                                                                                        else
                                                                                            {
                                                                                            epiP = piP + nblk - 1;
                                                                                            aecDTM_buildPtrIndexTable ( *srfPP, piP, NULL );
                                                                                            aecDTM_sortPtrIndexTableByIndex ( piP, epiP );
                                                                                            aecDTM_convertIndexToPtr ( *srfPP, piP, epiP );
                                                                                            free ( (void *)piP );
                                                                                            sts = aecDTM_hashAllFeatures ( *srfPP );
                                                                                            }
                                                                                    }


                                                                                if ( sts == SUCCESS ) 
                                                                                    sts = aecDTM_indexCorridorsComponentsMembers ( *srfPP );

                                                                                if ( handleP ) fclose ( handleP );

                                                                                if ( sts == SUCCESS )
                                                                                    {
                                                                                    aecDTM_clearSurfaceModifiedFlag ( *srfPP );
                                                                                    (*srfPP)->version = DTM_C_DTMVER;
                                                                                    (*srfPP)->codePage = codePage;
                                                                                    wcsncpy ( (*srfPP)->pth, fileNameP, CIV_MAX_PATH-1 );
                                                                                    }
                                                                                else
                                                                                    {
                                                                                    if ( *srfPP )
                                                                                        aecDTM_deleteSurface ( (struct CIVdtmprj *)0, *srfPP, 0 );

                                                                                    aecOutput_setMessage ( sts, fileNameP );
                                                                                    }

                                                                                aecTicker_stop();

                                                                                return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadOpen
DESC: Opens up the .DTM file, makes sure it is a .DTM file, and then makes
sure it is the correct version.

07-Dec-1993: added ability to read and check code page.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD OPEN
-----------------------------------------------------------------------------%*/

static int aecDTM_loadOpen
    (
    wchar_t *filP,
    unsigned long *versionP,
    unsigned long *codePageP,
    FILE **handlePP
    )
    {
    unsigned char buf[3]; // Do not convert to unicode.
    int sts = SUCCESS;

    aecFile_checkExtension ( filP, aecParams_getFileExtension(DTMEXT) );

    if ( ( *handlePP = _wfopen ( filP, L"rb" ) ) == (FILE *)0 )                     /* DO_NOT_TRANSLATE */
        return ( sts = DTM_M_FILNTF );
    else if ( fread ( buf, sizeof(buf), 1, *handlePP ) != 1 )
        sts = DTM_M_RDFILF;
    else if ( _mbsncmp ( (unsigned char *)DTM_C_DTMCOD, buf, 3 ) )
        sts = DTM_M_INVDFF;
    else if ( fread ( versionP, sizeof(unsigned long), 1, *handlePP ) != 1 )
        sts = DTM_M_RDFILF;
    else
        {
        if ( *versionP != 2 &&
            *versionP != 3 &&
            *versionP != 4 &&
            *versionP != 5 &&
            *versionP != 6 &&
            *versionP != 7 &&
            *versionP != 8 &&
            *versionP != 9 &&
            *versionP != 10 &&
            *versionP != 11 )
            {
            sts = DTM_M_DTMVER;
            }
        else if ( *versionP < 5 )
            aecLocale_getActiveCodePage ( codePageP );
        else if ( fread ( codePageP, sizeof(unsigned long), 1, *handlePP ) != 1 )
            sts = DTM_M_RDFILF;
        else
            {
            sts = aecLocale_verifyCodePage ( *codePageP );
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadCreateSurface
DESC: It creates a new surface to load the memory into.
HIST: Original - tmi 05-Mar-1991
MISC: static
KEYW: DTM LOAD SURFACE NAME CREATE
-----------------------------------------------------------------------------%*/

static int aecDTM_loadCreateSurface
    (
struct CIVdtmprj *prjP,
    unsigned long version,
struct CIVdtmsrf **srfPP
    )
    {
    int sts = SUCCESS;

    if ( ( sts = aecDTM_createSurface ( srfPP, prjP, TMPSRFNAME, NULL, NULL, NULL, 0., 0. ) ) == SUCCESS )  /* DO_NOT_TRANSLATE */
        {
        (*srfPP)->version = version;
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadGUID
DESC: Loads the surface BeSQLite::BeGuid from the .DTM file.
HIST: Original - twl 22-Oct-1998
MISC: static
KEYW: DTM LOAD BeSQLite::BeGuid
-----------------------------------------------------------------------------%*/

static int aecDTM_loadGUID
    (
struct CIVdtmsrf *srf,
    unsigned long,
    FILE *handleP
    )
    {
    struct CIVdtmsrf *existingSrfP = NULL;
    int sts = SUCCESS;

    if ( srf->version < 6 )
        {
        aecGuid_generate ( &srf->guid );
        }
    else
        {
        BeSQLite::BeGuid guid;

        if ( fread ( &guid, sizeof(srf->guid), 1, handleP ) != 1 )
            sts = DTM_M_RDFILF;
        else if ( aecDTM_findSurfaceByGUID ( &existingSrfP, NULL, &guid ) == SUCCESS && existingSrfP )
            {
            sts = DTM_M_DUP_SRF;
            pDupSrf = existingSrfP;
            }
        else
            aecGuid_copy ( &srf->guid, &guid );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadVerifyName
DESC: Loads the name and description section of the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD NAME VERIFY
-----------------------------------------------------------------------------%*/

static int aecDTM_loadVerifyName
    (
struct CIVdtmprj *prj,
struct CIVdtmsrf *srf,
    wchar_t *nam,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 8 )
        {
        wchar_t buf[DTM_C_NAMSIZ];

        if ( fread ( srf->des, sizeof(wchar_t), DTM_C_NAMSIZ, handleP ) != DTM_C_NAMSIZ )
            sts = DTM_M_RDFILF;
        else if ( fread ( buf, sizeof(wchar_t), DTM_C_NAMSIZ, handleP ) != DTM_C_NAMSIZ )
            sts = DTM_M_RDFILF;
        else
            {
            aecDTM_generateUniqueSurfaceName ( nam, buf, prj );
            wcsncpy ( srf->nam, nam, DTM_C_NAMSIZ-1 );
            srf->nam[DTM_C_NAMSIZ-1] = '\0';

            if ( fread ( srf->mat, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;
            }
        }
    else
        {
        char buf[CIV_C_DESSIZ]; // Do not convert to unicode.
        char des[CIV_C_DESSIZ];

        if ( fread ( des, sizeof(char), CIV_C_DESSIZ, handleP ) != CIV_C_DESSIZ ) // Do not convert to unicode.
            sts = DTM_M_RDFILF;
        else if ( fread ( buf, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ ) // Do not convert to unicode.
            sts = DTM_M_RDFILF;
        else
            {
            wchar_t wbuf[CIV_C_DESSIZ];

            memset ( wbuf, 0, sizeof ( wbuf ) );
            mbstowcs ( srf->des, des, CIV_C_DESSIZ );
            mbstowcs ( wbuf, buf, CIV_C_DESSIZ );
            aecDTM_generateUniqueSurfaceName ( nam, wbuf, prj );
            wcsncpy ( srf->nam, nam, DTM_C_NAMSIZ-1 );
            srf->nam[DTM_C_NAMSIZ-1] = '\0';

            if ( srf->version > 2 )
                {
                char mat[CIV_C_NAMSIZ]; // Do not conver to unicode.

                memset ( mat, 0, sizeof ( mat ) );

                if ( fread ( mat, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ ) // Do not convert to unicode.
                    sts = DTM_M_RDFILF;

                mbstowcs ( srf->mat, mat, CIV_C_NAMSIZ );
                }
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadPreferenceName
DESC: Loads the preference name from the .DTM file.
HIST: Original - twl 23-Oct-1998
MISC: static
KEYW: DTM LOAD PREFENCE NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_loadPreferenceName
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 5 )
        {
        if ( srf->version > 8 )
            {
            if ( fread ( srf->pref, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;
            }
        else
            {
            char pref[CIV_C_NAMSIZ]; // Do not convert to wide character.

            if ( fread ( pref, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;

            mbstowcs ( srf->pref, pref, CIV_C_NAMSIZ );
            }
        }
    else
        {
        wcsncpy ( srf->pref, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadXSectionSymbologyName
DESC: Loads the named symbology for cross sections.
HIST: Original - twl 23-Oct-1998
MISC: static
KEYW: DTM LOAD XSECTION NAMED SYMBOLOGY
-----------------------------------------------------------------------------%*/

static int aecDTM_loadXSectionSymbologyName
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 5 )
        {
        if ( srf->version > 8 )
            {
            if ( fread ( srf->secsym, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;
            }
        else
            {
            char secsym[CIV_C_NAMSIZ];  // Do not convert to wide character.

            if ( fread ( secsym, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;

            mbstowcs ( srf->secsym, secsym, CIV_C_NAMSIZ );
            }
        }
    else
        {
        wcsncpy ( srf->secsym, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadProfileSymbologyName
DESC: Loads the named symbology to be used when displaying surfaces in profiles.
HIST: Original - twl 27-Jun-2000
MISC: static
KEYW: DTM LOAD PROFILE SYMBOLOGY NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_loadProfileSymbologyName
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 6 )
        {
        if ( srf->version > 8 )
            {
            if ( fread ( srf->prfsym, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;
            }
        else
            {
            char prfsym[CIV_C_NAMSIZ]; // Do not convert to wide.

            if ( fread ( prfsym, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;

            mbstowcs ( srf->prfsym, prfsym, CIV_C_NAMSIZ );
            }
        }
    else
        {
        wcsncpy ( srf->prfsym, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadProfileOffsets
DESC: Loads the offset properties to be used when displaying offsets in profiles.
HIST: Original - twl 27-Jun-2000
MISC: static
KEYW: DTM LOAD PROFILE OFFSETS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadProfileOffsets
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int i;
    int sts = SUCCESS;

    if ( srf->version > 6 )
        {
        if ( srf->version > 8 )
            {
            if ( fread ( srf->prfoff, sizeof(struct CIVdtmoff), DTM_C_NUMPRFOFF, handleP ) != DTM_C_NUMPRFOFF )
                sts = DTM_M_RDFILF;
            }
        else
            {
            struct CIVdtmoffV8 prfoff[DTM_C_NUMPRFOFF];

            memset ( prfoff, 0, sizeof ( prfoff ) );

            if ( fread ( prfoff, sizeof(struct CIVdtmoffV8), DTM_C_NUMPRFOFF, handleP ) != DTM_C_NUMPRFOFF )
                sts = DTM_M_RDFILF;

            for ( i = 0; i < DTM_C_NUMPRFOFF; i++ )
                {
                mbstowcs ( srf->prfoff[i].sym, prfoff[i].sym, CIV_C_NAMSIZ );
                srf->prfoff[i].dis = prfoff[i].dis;
                }
            }
        }
    else
        {
        for ( i = 0; i < DTM_C_NUMPRFOFF; i++ )
            {
            wcsncpy ( srf->prfoff[i].sym, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
            srf->prfoff[i].dis = 0.0;
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadRevision
DESC: Loads the last revision author and date from the .DTM file.
HIST: Original - twl 213-Jun-99
MISC: static
KEYW: DTM LOAD REVISION
-----------------------------------------------------------------------------%*/

static int aecDTM_loadRevision
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 5 )
        {
        if ( srf->version > 8 )
            {
            if ( fread ( srf->revby, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;
            }
        else
            {
            char revby[CIV_C_NAMSIZ]; // Do not convert to wide.

            if ( fread ( revby, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
                sts = DTM_M_RDFILF;

            mbstowcs ( srf->revby, revby, CIV_C_NAMSIZ );
            }

        if ( sts == SUCCESS )
            {
            if ( fread ( &srf->revdate, sizeof(SYSTEMTIME), 1, handleP ) != 1 )
                sts = DTM_M_RDFILF;
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadParameters
DESC: Loads the parameters section of the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD PARAMETERS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadParameters
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    int sts = SUCCESS;

    if ( srf->version > 6 )
        {
        if ( fread ( &srf->par, sizeof(struct CIVdtmpar), 1, handleP ) != 1 )
            sts = DTM_M_RDFILF;
        }
    else if ( srf->version > 5 )
        {
        struct CIVdtmparV6 parV6;

        if ( fread ( &parV6, sizeof(struct CIVdtmparV6), 1, handleP ) != 1 )
            sts = DTM_M_RDFILF;
        else
            {
            srf->par.scl    = parV6.scl;
            srf->par.maxlin = parV6.maxlin;
            srf->par.maxsid = parV6.maxsid;
            srf->par.secFtrsOnly = (short) parV6.ftrsOnly;
            }
        }
    else
        {
        struct CIVdtmparV5 parV5;

        if ( fread ( &parV5, sizeof(struct CIVdtmparV5), 1, handleP ) != 1 )
            sts = DTM_M_RDFILF;
        else
            {
            srf->par.scl    = parV5.scl;
            srf->par.maxlin = parV5.maxlin;
            srf->par.maxsid = parV5.maxsid;
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadFiles
DESC: Loads the data file headers in from the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD FILES
-----------------------------------------------------------------------------%*/

static int aecDTM_loadFiles
    (
struct CIVdtmsrf *srf,
    unsigned long version,
    FILE *handleP
    )
    {
    int sts = SUCCESS, i, numPntFile;

    numPntFile = ( version < 4 ) ? DTM_C_NMPNTFV3 : DTM_C_NMPNTF;

    for ( i = 0; i < numPntFile  &&  sts == SUCCESS; i++ )
        if ( fread ( srf->pntf[i], CIVdtmfilSize, 1, handleP ) != 1 )
            sts = DTM_M_RDFILF;
        else
            {
            srf->pntf[i]->blk = 0;
            }

        if ( sts == SUCCESS )
            if ( fread ( srf->rngf, CIVdtmfilSize, 1, handleP ) != 1 )
                sts = DTM_M_RDFILF;
            else
                {
                srf->rngf->blk = 0;
                }

            if ( sts == SUCCESS )
                if ( fread ( srf->tinf, CIVdtmfilSize, 1, handleP ) != 1 )
                    sts = DTM_M_RDFILF;
                else
                    {
                    srf->tinf->blk = 0;
                    }

                if ( sts == SUCCESS && srf->version > 5 )
                    {
                    for ( i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
                        if ( fread ( srf->ftrf[i], CIVdtmfilSize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        else
                            {
                            srf->ftrf[i]->blk = 0;
                            }

                        if ( fread ( srf->styf, CIVdtmfilSize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        else
                            {
                            srf->styf->blk = 0;
                            }
                    }

                if ( srf->version > 7 )
                    {
                    if ( fread ( srf->payf, CIVdtmfilSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    else
                        {
                        srf->payf->blk = 0;
                        }
                    }

                if ( sts == SUCCESS && srf->version > 8 )
                    {
                    if ( fread ( srf->corf, CIVdtmfilSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    else
                        {
                        srf->corf->blk = 0;
                        }

                    if ( fread ( srf->cmpf, CIVdtmfilSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    else
                        {
                        srf->cmpf->blk = 0;
                        }

                    if ( fread ( srf->cmpMemf, CIVdtmfilSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    else
                        {
                        srf->cmpMemf->blk = 0;
                        }
                    }

                return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadPoints
DESC: Loads the points in from the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadPoints
    (
struct CIVdtmsrf *srf,
    unsigned long version,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmpnt *p;
    int sts = SUCCESS, i, j, numPntFile;

    numPntFile = ( version < 4 ) ? DTM_C_NMPNTFV3 : DTM_C_NMPNTF;

    for ( j = i = 0; i < numPntFile  &&  sts == SUCCESS; i++ )
        {
        srf->pntf[i]->blk = 0;
        if ( srf->pntf[i]->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->pntf[i], srf->pntf[i]->nrec, 0 ) ) == SUCCESS )
                for ( p = blk->rec.pnt; blk->use < srf->pntf[i]->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 )
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( fread ( &p[blk->use], CIVdtmpntSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadRange
DESC: Loads the range points in from the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD POINTS RANGE
-----------------------------------------------------------------------------%*/

static int aecDTM_loadRange
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmpnt *p;
    int sts = SUCCESS;

    srf->rngf->blk = 0;
    if ( srf->rngf->nrec > 0L )
        if ( ( sts = aecDTM_allocateBlock ( &blk, srf->rngf, srf->rngf->nrec, 0 ) ) == SUCCESS )
            for ( p = blk->rec.pnt; blk->use < srf->rngf->nrec  &&  sts == SUCCESS; blk->use++ )
                {
                aecTicker_show();
                if ( fread ( &p[blk->use], CIVdtmpntSize, 1, handleP ) != 1 )
                    sts = DTM_M_RDFILF;
                }

            return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadTriangles
DESC: Loads the triangles in from the .DTM file.
HIST: Original - tmi 24-Apr-1990
MISC: static
KEYW: DTM LOAD TRIANGLES
-----------------------------------------------------------------------------%*/

static int aecDTM_loadTriangles
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmtin *t;
    int j = 0, sts = SUCCESS;
    long numRead = 0;

    srf->tinf->blk = 0;
    if ( srf->tinf->nrec > 0L )
        {
        while ( numRead < srf->tinf->nrec && sts == SUCCESS )
            {
            int numAlloc = srf->tinf->nrec - numRead;

            if ( numAlloc > DTM_C_BLKSIZTIN )
                numAlloc = DTM_C_BLKSIZTIN;

            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->tinf, numAlloc, 0 ) ) == SUCCESS )
                {
                for ( t = blk->rec.tin; blk->use < blk->alc && numRead < srf->tinf->nrec &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 ) 
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( fread ( &t[blk->use], CIVdtmtinSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;
                    else
                        {
                        t[blk->use].p1 = (CIVdtmpnt*)t[blk->use].op1;
                        t[blk->use].p2 = (CIVdtmpnt*)t[blk->use].op2;
                        t[blk->use].p3 = (CIVdtmpnt*)t[blk->use].op3;
                        t[blk->use].n12 = (CIVdtmtin*)t[blk->use].on12;
                        t[blk->use].n23 = (CIVdtmtin*)t[blk->use].on23;
                        t[blk->use].n31 = (CIVdtmtin*)t[blk->use].on31;
                        numRead++;
                        }
                    }
                }
            }

        if ( sts == SUCCESS )
            {
            struct CIVdtmblk *curblk = srf->tinf->blk;
            struct CIVdtmblk *prvblk = NULL;
            struct CIVdtmblk *nxtblk = NULL;

            while ( curblk )
                {
                nxtblk = curblk->nxt;
                curblk->nxt = prvblk;
                prvblk = curblk;
                curblk = nxtblk;
                }

            srf->tinf->blk = prvblk;
            }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadFeatures
DESC: Loads the features in from the .DTM file.
HIST: Original - twl 23-Oct-1998
MISC: static
KEYW: DTM LOAD FEATURES
-----------------------------------------------------------------------------%*/

static int aecDTM_loadFeatures
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmftr *f;
    int i = 0, j = 0, sts = SUCCESS;

    if ( srf->version > 5 )
        {
        for ( j = i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
            {
            srf->ftrf[i]->blk = 0;
            if ( srf->ftrf[i]->nrec > 0L )
                {
                int numRead = 0;

                while ( numRead < srf->ftrf[i]->nrec && sts == SUCCESS )
                    {
                    int numAlloc = srf->ftrf[i]->nrec - numRead;

                    if ( numAlloc > DTM_C_BLKSIZFTR )
                        numAlloc = DTM_C_BLKSIZFTR;

                    if ( ( sts = aecDTM_allocateBlock ( &blk, srf->ftrf[i], numAlloc, 0 ) ) == SUCCESS )
                        {
                        for ( f = blk->rec.ftr; blk->use < blk->alc && numRead < srf->ftrf[i]->nrec  &&  sts == SUCCESS; blk->use++ )
                            {
                            if ( j++ == 100 )
                                {
                                aecTicker_show();
                                j = 0;
                                }

                            if ( srf->version > 8 )
                                {
                                if (fread (&f[blk->use], CIVdtmftrSize, 1, handleP) != 1)
                                    {
                                    sts = DTM_M_RDFILF;
                                    }
                                else
                                    {
                                    f[blk->use].p1 = (CIVdtmpnt*)f[blk->use].p[0];
                                    f[blk->use].s1 = (CIVdtmsty*)f[blk->use].p[1];
                                    f[blk->use].pay = (CIVdtmpay*)f[blk->use].p[2];
                                    }
                                }
                            else if ( srf->version > 7 )
                                {
                                struct CIVdtmftrV8 ftrV8;
                                memset ( &ftrV8, 0, sizeof ( ftrV8 ) );

                                if ( fread ( &ftrV8, sizeof ( struct CIVdtmftrV8 ), 1, handleP ) != 1 )
                                    sts = DTM_M_RDFILF;

                                aecGuid_copy ( &f[blk->use].guid, &ftrV8.guid );
                                mbstowcs ( f[blk->use].nam, ftrV8.nam, CIV_C_NAMSIZ );
                                mbstowcs ( f[blk->use].des, ftrV8.des, CIV_C_DESSIZ );
                                mbstowcs ( f[blk->use].par, ftrV8.par, CIV_C_NAMSIZ );
                                f[blk->use].p1 = ftrV8.p1;
                                f[blk->use].s1 = ftrV8.s1;
                                f[blk->use].pay = ftrV8.pay;
                                f[blk->use].numPnts = ftrV8.numPnts;
                                f[blk->use].numStyles = ftrV8.numStyles;
                                f[blk->use].numPayItems = ftrV8.numPayItems;
                                f[blk->use].pntDensity = ftrV8.pntDensity;
                                f[blk->use].flg = ftrV8.flg;
                                }
                            else
                                {
                                struct CIVdtmftrV7 ftrV7;
                                memset ( &ftrV7, 0, sizeof ( ftrV7 ) );

                                if ( fread ( &ftrV7, sizeof ( struct CIVdtmftrV7 ), 1, handleP ) != 1 )
                                    sts = DTM_M_RDFILF;

                                aecGuid_copy ( &f[blk->use].guid, &ftrV7.guid );
                                mbstowcs ( f[blk->use].nam, ftrV7.nam, CIV_C_NAMSIZ );
                                mbstowcs ( f[blk->use].des, ftrV7.des, CIV_C_DESSIZ );
                                mbstowcs ( f[blk->use].par, ftrV7.par, CIV_C_NAMSIZ );
                                f[blk->use].p1 = ftrV7.p1;
                                f[blk->use].s1 = ftrV7.s1;
                                f[blk->use].numPnts = ftrV7.numPnts;
                                f[blk->use].numStyles = ftrV7.numStyles;
                                f[blk->use].pntDensity = ftrV7.pntDensity;
                                f[blk->use].flg = ftrV7.flg;
                                }

                            if ( sts == SUCCESS )
                                {
                                int namChars = sizeof ( f[blk->use].nam ) / sizeof ( f[blk->use].nam[0] );
                                int desChars = sizeof ( f[blk->use].des ) / sizeof ( f[blk->use].des[0] );
                                int parChars = sizeof ( f[blk->use].par ) / sizeof ( f[blk->use].par[0] );
                                f[blk->use].nam[namChars-1] = '\0';
                                f[blk->use].des[desChars-1] = '\0';
                                f[blk->use].par[parChars-1] = '\0';
                                numRead++;
                                }
                            }
                        }
                    }

                if ( sts == SUCCESS )
                    {
                    struct CIVdtmblk *curblk = srf->ftrf[i]->blk;
                    struct CIVdtmblk *prvblk = NULL;
                    struct CIVdtmblk *nxtblk = NULL;

                    while ( curblk )
                        {
                        nxtblk = curblk->nxt;
                        curblk->nxt = prvblk;
                        prvblk = curblk;
                        curblk = nxtblk;
                        }

                    srf->ftrf[i]->blk = prvblk;
                    }
                }
            }
        }
    else
        {
        sts = aecDTM_generateFeaturesFromPoints ( srf );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadFeatureStyles
DESC: Loads the feature styles in from the .DTM file.
HIST: Original - twl 23-Oct-1998
MISC: static
KEYW: DTM LOAD FEATURE STYLES
-----------------------------------------------------------------------------%*/

static int aecDTM_loadFeatureStyles
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmsty *s;
    int j = 0, sts = SUCCESS;

    if ( srf->version > 5 )
        {
        srf->styf->blk = 0;
        if ( srf->styf->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->styf, srf->styf->nrec, 0 ) ) == SUCCESS )
                for ( s = blk->rec.sty; blk->use < srf->styf->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 ) 
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( srf->version > 8 )
                        {
                        if ( fread ( &s[blk->use], CIVdtmstySize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        }
                    else
                        {
                        CIVdtmstyV8 sV8;

                        memset ( &sV8, 0, sizeof ( sV8 ) );

                        if ( fread ( &sV8, sizeof ( sV8 ), 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;

                        s[blk->use].flg = sV8.flg;
                        mbstowcs ( s[blk->use].nam, sV8.nam, CIV_C_NAMSIZ );
                        }

                    if ( sts == SUCCESS )
                        {
                        CString strStyle = s[blk->use].nam;

                        if ( !strStyle.CompareNoCase ( DEFAULT_PREFERENCE_NAME ) )
                            wcscpy ( s[blk->use].nam, DEFAULT_PREFERENCE_NAME );

                        int styChars = sizeof ( s[blk->use].nam ) / sizeof ( s[blk->use].nam[0] );
                        s[blk->use].nam[styChars-1] = '\0';
                        }         
                    }
        }
    else
        {
        sts = aecDTM_generateFeatureStyles ( srf );
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadFeaturePayItems
DESC: Loads the feature pay items in from the .DTM file.
HIST: Original - twl 10-Oct-2003
MISC: static
KEYW: DTM LOAD FEATURE PAY ITEMS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadFeaturePayItems
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmpay *p;
    int j = 0, sts = SUCCESS;

    if ( srf->version > 7 )
        {
        srf->payf->blk = 0;
        if ( srf->payf->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->payf, srf->payf->nrec, 0 ) ) == SUCCESS )
                for ( p = blk->rec.pay; blk->use < srf->payf->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 ) 
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( srf->version > 8 )
                        {
                        if ( fread ( &p[blk->use], CIVdtmpaySize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        }
                    else
                        {
                        CIVdtmpayV8 pV8;

                        memset ( &pV8, 0, sizeof ( pV8 ) );

                        if ( fread ( &pV8, sizeof ( pV8 ), 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;

                        p[blk->use].flg = pV8.flg;
                        mbstowcs ( p[blk->use].nam, pV8.nam, CIV_C_NAMSIZ );
                        }

                    if ( sts == SUCCESS )
                        {
                        int payChars = sizeof ( p[blk->use].nam ) / sizeof ( p[blk->use].nam[0] );
                        p[blk->use].nam[payChars-1] = '\0';
                        }         
                    }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadCorridors
DESC: Loads the corridors in from the .DTM file.
HIST: Original - twl 29-Oct-2004
MISC: static
KEYW: DTM LOAD CORRIDORS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadCorridors
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmcor *c;
    int j = 0, sts = SUCCESS;

    if ( srf->version > 8 )
        {
        srf->corf->blk = 0;
        if ( srf->corf->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->corf, srf->corf->nrec, 0 ) ) == SUCCESS )
                for ( c = blk->rec.cor; blk->use < srf->corf->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 )
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( fread ( &c[blk->use], CIVdtmcorSize, 1, handleP ) != 1 )
                        sts = DTM_M_RDFILF;

                    if ( sts == SUCCESS )
                        {
                        int corChars = sizeof ( c[blk->use].nam ) / sizeof ( c[blk->use].nam[0] );
                        c[blk->use].nam[corChars-1] = '\0';
                        }         
                    }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadComponents
DESC: Loads the components in from the .DTM file.
HIST: Original - twl 24-Apr-2002
MISC: static
KEYW: DTM LOAD COMPONENTS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadComponents
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmcmp *c;
    int j = 0, sts = SUCCESS;

    if ( srf->version > 8 )
        {
        srf->cmpf->blk = 0;
        if ( srf->cmpf->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->cmpf, srf->cmpf->nrec, 0 ) ) == SUCCESS )
                for ( c = blk->rec.cmp; blk->use < srf->cmpf->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 )
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( srf->version > 10 )
                        {
                        if ( fread ( &c[blk->use], CIVdtmcmpSize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        }
                    else if ( srf->version > 9 )
                        {
                        CIVdtmcmpV10 v10Cmp;

                        if ( fread ( &v10Cmp, sizeof (v10Cmp), 1, handleP ) != 1 )
                            {
                            sts = DTM_M_RDFILF;
                            }
                        else
                            {
                            aecGuid_copy ( &c[blk->use].guid,     &v10Cmp.guid );
                            aecGuid_copy ( &c[blk->use].corGuid,  &v10Cmp.corGuid );
                            memcpy       ( &c[blk->use].nam,      &v10Cmp.nam,      sizeof ( c[blk->use].nam ) );
                            memcpy       ( &c[blk->use].des,      &v10Cmp.des,      sizeof ( c[blk->use].des ) );
                            memcpy       ( &c[blk->use].stynam,   &v10Cmp.stynam,   sizeof ( c[blk->use].stynam ) );
                            memcpy       ( &c[blk->use].startXY,  &v10Cmp.startXY,  sizeof ( c[blk->use].startXY ) );
                            memcpy       ( &c[blk->use].stopXY,   &v10Cmp.stopXY,   sizeof ( c[blk->use].stopXY ) );
                            memcpy       ( &c[blk->use].startStn, &v10Cmp.startStn, sizeof ( c[blk->use].startStn ) );
                            memcpy       ( &c[blk->use].stopStn,  &v10Cmp.stopStn,  sizeof ( c[blk->use].stopStn ) );
                            memcpy       ( &c[blk->use].flg,      &v10Cmp.flg,      sizeof ( c[blk->use].flg ) );  

                            c[blk->use].topflg = 0;
                            c[blk->use].botflg = 0;
                            }
                        }
                    else
                        {
                        CIVdtmcmpV87prel v87prelCmp;

                        if ( fread ( &v87prelCmp, sizeof (CIVdtmcmpV87prel), 1, handleP ) != 1 )
                            {
                            sts = DTM_M_RDFILF;
                            }
                        else
                            {
                            aecGuid_copy ( &c[blk->use].guid,     &v87prelCmp.guid );
                            aecGuid_copy ( &c[blk->use].corGuid,  &v87prelCmp.corGuid );
                            memcpy       ( &c[blk->use].nam,      &v87prelCmp.nam,      sizeof ( c[blk->use].nam ) );
                            memcpy       ( &c[blk->use].stynam,   &v87prelCmp.stynam,   sizeof ( c[blk->use].stynam ) );
                            memcpy       ( &c[blk->use].startXY,  &v87prelCmp.startXY,  sizeof ( c[blk->use].startXY ) );
                            memcpy       ( &c[blk->use].stopXY,   &v87prelCmp.stopXY,   sizeof ( c[blk->use].stopXY ) );
                            memcpy       ( &c[blk->use].startStn, &v87prelCmp.startStn, sizeof ( c[blk->use].startStn ) );
                            memcpy       ( &c[blk->use].stopStn,  &v87prelCmp.stopStn,  sizeof ( c[blk->use].stopStn ) );
                            memcpy       ( &c[blk->use].flg,      &v87prelCmp.flg,      sizeof ( c[blk->use].flg ) );              
                            }
                        }

                    if ( sts == SUCCESS )
                        {
                        int namChars = sizeof ( c[blk->use].nam ) / sizeof ( c[blk->use].nam[0] );
                        int desChars = sizeof ( c[blk->use].des ) / sizeof ( c[blk->use].des[0] );
                        int styChars = sizeof ( c[blk->use].stynam ) / sizeof ( c[blk->use].stynam[0] );
                        c[blk->use].nam[namChars-1] = '\0';
                        c[blk->use].des[desChars-1] = '\0';
                        c[blk->use].stynam[styChars-1] = '\0';
                        }         
                    }
        }

    return ( sts );
    }




/*%-----------------------------------------------------------------------------
FUNC: aecDTM_loadComponentMembers
DESC: Loads the component's members in from the .DTM file.
HIST: Original - twl 24-Apr-2002
MISC: static
KEYW: DTM LOAD COMPONENT MEMBERS
-----------------------------------------------------------------------------%*/

static int aecDTM_loadComponentMembers
    (
struct CIVdtmsrf *srf,
    FILE *handleP
    )
    {
    struct CIVdtmblk *blk;
    struct CIVdtmcmpmem *s;
    int j = 0, sts = SUCCESS;

    if ( srf->version > 8 )
        {
        srf->cmpMemf->blk = 0;
        if ( srf->cmpMemf->nrec > 0L )
            if ( ( sts = aecDTM_allocateBlock ( &blk, srf->cmpMemf, srf->cmpMemf->nrec, 0 ) ) == SUCCESS )
                for ( s = blk->rec.cmpMem; blk->use < srf->cmpMemf->nrec  &&  sts == SUCCESS; blk->use++ )
                    {
                    if ( j++ == 100 ) 
                        {
                        aecTicker_show();
                        j = 0;
                        }

                    if ( srf->version > 10 )
                        {
                        if ( fread ( &s[blk->use], CIVdtmcmpmemSize, 1, handleP ) != 1 )
                            sts = DTM_M_RDFILF;
                        }
                    else
                        {
                        CIVdtmcmpmemV10 v10cmpMem;

                        if ( fread ( &v10cmpMem, sizeof (v10cmpMem), 1, handleP ) != 1 )
                            {
                            sts = DTM_M_RDFILF;
                            }
                        else
                            {
                            aecGuid_copy ( &s[blk->use].cmpGuid,     &v10cmpMem.cmpGuid );
                            aecGuid_copy ( &s[blk->use].cmpMemGuid,  &v10cmpMem.cmpMemGuid );
                            s[blk->use].type = v10cmpMem.type;
                            s[blk->use].index = v10cmpMem.index;
                            s[blk->use].flg = v10cmpMem.flg;

                            s[blk->use].componentThickness = 0.0;
                            s[blk->use].surfaceThickness = 0.0;
                            }
                        }
                    }
        }

    return ( sts );
    }



/*----------------------------------------------------------------------+
|  FUNC: aecDTM_loadCheckFormat                                        |
|  DESC: Checks for a .TTN file format, returns version.               |
|        If no extension, .ttn appended.                               |
|  HIST: original                                            wbw 1/93  |
|  MISC: The file is openned and closed in this function, and          |
|        called before branching to appropriate load function.         |
|  KEYW:                                                               |
+----------------------------------------------------------------------*/

static int aecDTM_loadCheckFormat  /* <= TRUE if error              */
    (
    int  *ttnFormatP,                    /* <= returned file format       */
    wchar_t *filP                           /* => file to check              */
    )
    {
    unsigned char buf[3]; // Do not convert to unicode.
    FILE *fptr = (FILE *)0;
    int  sts = SUCCESS;

    *ttnFormatP = 0;
    if ( ( fptr = _wfopen ( filP, L"rb" ) ) == 0 )                                  /* DO_NOT_TRANSLATE */
        sts = DTM_M_FILNTF;
    else if ( fread ( buf, sizeof ( char ), 3, fptr ) != 3 )
        sts = DTM_M_RDFILF;
    else if ( _mbsncmp ( (unsigned char *)TTN_MAGIC, buf, EXTEN_CODE_SIZE ) == 0 )
        {
        if ( fread ( buf, sizeof ( char ), 1, fptr ) != 1 )
            sts = DTM_M_RDFILF;
        else 
            *ttnFormatP = ( int ) buf[0];
        }
    if ( fptr != (FILE *)0 ) fclose ( fptr );

    return ( sts );
    }


