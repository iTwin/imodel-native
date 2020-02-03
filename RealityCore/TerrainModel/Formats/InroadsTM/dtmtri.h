//---------------------------------------------------------------------------------------------
// Copyright (c) Bentley Systems, Incorporated. All rights reserved.
// See COPYRIGHT.md in the repository root for full copyright notice.
//---------------------------------------------------------------------------------------------
/*----------------------------------------------------------------------------*/
/* dtmtri.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used for triangulating.                      */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Triangulate surface things                                                 */
/*----------------------------------------------------------------------------*/

#define TRISRF_DIS         0x4         /* interactive displ.                  */
#define TRISRF_CON         0x8         /* interactive displ.                  */
#define TRISRF_INT        0x10         /* interuptable                        */
#define TRISRF_FTR        0x20         /* load features                       */
#define TRISRF_REM        0x40         /* remove existing                     */
#define TRISRF_BLK       0x100         /* don't use huge triangle allocation  */


struct CIVtinLineData
{
    struct CIVdtmsrf *srf;             /* surface to triangulate              */
    struct CIVdtmpnt *conPnt;          /* construction point                  */
    struct CIVdtmtin *tmp;             /* working triangle                    */
    struct CIVdtmtin *startTin;        /* working triangle                    */
    struct CIVdtmtin *endTin;          /* working triangle                    */
    long tinNum;                       /* # of affected tins                  */
    long tinAlc;                       /* # of allocated groups               */
    CIVdtmtin* *tin;                   /* ptrs to affected tins               */
    long leftNum;                      /* # points in left list               */
    long leftAlc;                      /* # left allocated grps               */
    CIVdtmpnt* *leftPnt;               /* left side points                    */
    CIVdtmtin* *leftNei;               /* left side neighbors                 */
    long leftTinNum;                   /* # new left side tins                */
    CIVdtmtin* *leftTin;               /* ptrs to new left tins               */
    long rightNum;                     /* # points in right list              */
    long rightAlc;                     /* # right allocated grps              */
    CIVdtmpnt* *rightPnt;              /* right side points                   */
    CIVdtmtin* *rightNei;              /* right side neighbors                */
    long rightTinNum;                  /* # new right side tins               */
    CIVdtmtin* *rightTin;              /* ptrs to new right tins              */
    long direct;                       /* true if direct connect.             */
    int robust;                        /* TRUE: use robust algorithm          */
};
