//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmvol.h                                        aec    07-Feb-1994         */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used for volume commands.                    */
/*----------------------------------------------------------------------------*/

#pragma once


/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <disfnc.h>
#include <dtmstr.h>
#include <jonfnc.h>

/*----------------------------------------------------------------------------*/
/* Generate constants                                                         */
/*----------------------------------------------------------------------------*/

#define CNVVOL_CUF     0x4             /* cubic feet                          */
#define CNVVOL_CUY     0x8             /* cubic yards                         */
#define CNVVOL_CUM     0x10            /* cubic meters                        */
#define CNVVOL_UNTMASK 0x1c

/*----------------------------------------------------------------------------*/
/* Triangle volume things                                                     */
/*----------------------------------------------------------------------------*/
#define VOLTIN_SRF  0x100              /* entire surface                      */
#define VOLTIN_INS  0x200              /* fence                               */
#define VOLTIN_SHA  0x400              /* shapes                              */
#define VOLTIN_PTS  0x800              /* has the points                      */

/*----------------------------------------------------------------------------*/
/* Grid volume things                                                         */
/*----------------------------------------------------------------------------*/
#define VOLGRD_INS    0x1              /* inside fence                        */
#define VOLGRD_OUT    0x2              /* outside fence                       */
#define VOLGRD_CUF    CNVVOL_CUF       /* cubic feet                          */
#define VOLGRD_CUY    CNVVOL_CUY       /* cubic yards                         */
#define VOLGRD_CUM    CNVVOL_CUM       /* cubic meters                        */
#define VOLGRD_UNTMSK (CNVVOL_CUF|CNVVOL_CUY|CNVVOL_CUM) /* unit mask         */
#define VOLGRD_RPT    0x20             /* generate report                     */

/*----------------------------------------------------------------------------*/
/* Surface area things                                                        */
/*----------------------------------------------------------------------------*/
#define SRFARE_INS    0x1              /* inside fence                        */
#define SRFARE_OUT    0x2              /* outside fence                       */
#define SRFARE_FEET   0x4              /* square feet                         */
#define SRFARE_YARD   0x8              /* square yards                        */
#define SRFARE_MILE   0x10             /* square miles                        */
#define SRFARE_ACRE   0x20             /* acres                               */
#define SRFARE_METR   0x40             /* square meters                       */
#define SRFARE_KILO   0x80             /* square kilometers                   */
#define SRFARE_HECT   0x100            /* hectares                            */

