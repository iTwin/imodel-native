//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmuti.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used for various utility DTM functions.      */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Transform surface                                                          */
/*----------------------------------------------------------------------------*/
#define DTM_C_TRNSCUST      0x0   /* modeOpt flag - Custom */
#define DTM_C_TRNSUSFT2M    0x1   /* modeOpt flag - US Feet to Meters */
#define DTM_C_TRNSINFT2M    0x2   /* modeOpt flag - Intl Feet to Meters */
#define DTM_C_TRNSM2USFT    0x3   /* modeOpt flag - Meters to US Feet */
#define DTM_C_TRNSM2INFT    0x4   /* modeOpt flag - Meters to Intl Feet */
#define DTM_C_TRNSREL       0x5   /* modeOpt flag - Relative elevation */
#define DTM_C_TRNSABS       0x6   /* modeOpt flag - Absolute elevation */
#define DTM_C_TRNSMSK       0x7   /* modeOpt flag - mask */
#define DTM_C_TRNSINFT2USFT 0x8   /* modeOpt flag - Intl Feet to US Feet */
#define DTM_C_TRNSUSFT2INFT 0x9   /* modeOpt flag - US Feet to Intl Feet */
#define DTM_C_TRNSM2M       0xA   /* modeOpt flag - Meters to Meters */


/*----------------------------------------------------------------------------*/
/* Thin surface                                                               */
/*----------------------------------------------------------------------------*/

typedef struct CIVthndtm
{
    long sts;                          /* returned status                     */
    long srfptr1;                      /* for dialog box                      */
    long srfptr2;                      /* for dialog box                      */
    long opt;                          /* see above defines                   */
    long typ;                          /* copy brk,int,ext                    */
    long pad;                          /* padding                             */
    double hgt;                        /* tolerance height                    */
    double min;                        /* minimum distance                    */
    double max;                        /* maximum distance                    */
    struct CIVdtmblk *blkP;            /* pointer to block to be thinned      */
    long pntDeleted;                   /* was a point deleted during thin     */

    struct CIVdtmsrf *srf1;            /* original surface                    */
    struct CIVdtmsrf *srf2;            /* thinned surface                     */

    long norg;                         /* returned                            */
    long nthn;                         /* returned                            */
    unsigned char tru;                          /* internal only                       */
    unsigned char retin;                        /* internal only                       */

    void *ftrHndlHashP;                /* Feature handle hash table           */
    void *ppFli;                       /* Feature list items ( or NULL )      */
    int nFlis;                         /* Number of feature list items        */
} DTMrsc_thndtm;


/*----------------------------------------------------------------------------*/
/* Form gridded model                                                         */
/*----------------------------------------------------------------------------*/

#define GRDDTM_INS           0x1       /* inside fence                        */
#define GRDDTM_OUT           0x2       /* outside fence                       */
#define GRDDTM_FTRLST        0x4       /* List of features                    */

#define GRDDTM_COPY_BREAK    0x2       /* copy breaklines                     */
#define GRDDTM_COPY_INTERIOR 0x4       /* copy interior                       */
#define GRDDTM_COPY_EXTERIOR 0x8       /* copy exterior                       */

typedef struct CIVgrddtm
{
    long sts;                          /* returned status                     */
    long srfptr1;                      /* for dialog box                      */
    long srfptr2;                      /* for dialog box                      */
    long opt;                          /* see above defines                   */
    long typ;                          /* copy brk,int,ext                    */
    long pad;                          /* padding                             */
    double xinc;                       /* x increment                         */
    double yinc;                       /* y increment                         */
    struct CIVdtmsrf *srf1;            /* original surface                    */
    struct CIVdtmsrf *srf2;            /* gridded model                       */

    long nsrc;                         /* returned                            */
    long ntrg;                         /* returned                            */
    unsigned char tru;                       /* internal only                       */
    void *gridDataP;                   /* internal only                       */

    void *ftrHndlHashP;                /* Feature handle hash table           */
    void *ppFli;                       /* Feature list items ( or NULL )      */
    int nFlis;                         /* Number of feature list items        */
} DTMrsc_grddtm;
