//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmedt.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used with DTM point and area editing funcs.  */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <vector>

using namespace std;

#include <dtmstr.h>
#include <dtmdis.h>


/*----------------------------------------------------------------------------*/
/* Adding points                                                              */
/*----------------------------------------------------------------------------*/

#define ADDPNT_FTR    0x00             /* Add point as surface feature        */
#define ADDPNT_DTM    0x10             /* Add point to DTM model              */

#define ADDPNT_TYPMSK 0x0f




/*----------------------------------------------------------------------------*/
/* Point editing data                                                         */
/*----------------------------------------------------------------------------*/

#define PNTEDT_TIN     0x4             /* display tin's                       */
#define PNTEDT_CON     0x8             /* display contours                    */
#define PNTEDT_DYN    0x10             /* dynamic mode                        */
#define PNTEDT_ELV    0x20             /* use elev. field                     */
#define PNTEDT_NOTRI  0x40
#define PNTEDT_TAG    0x80             /* display with tag                    */
#define PNTEDT_TICK  0x100             /* keep ticker going                   */
#define PNTEDT_NOPATCH 0x200           /* delete triangles but don't patch hole */

#define PNTEDT_INIT   0                /* initialize                          */
#define PNTEDT_STRD   1                /* start dynamics                      */
#define PNTEDT_PRCD   2                /* process dynamics                    */
#define PNTEDT_STPD   3                /* stop dynamics                       */
#define PNTEDT_ADD    4                /* add point to dtm                    */
#define PNTEDT_CLEAN  5                /* clean up                            */

#define DTMsiz_pntedt (offsetof(struct CIVpntedt,srf))

typedef struct CIVpntedt
{
    long sts;                          /* returned status                     */
    long srfptr;                       /* for dialog box                      */
    long opt;                          /* see above define                    */
    double elv;                        /* dialog box elv.                     */
    struct CIVdtmsrf *srf;             /* target surface                      */
    struct CIVdtmpnt *pnt;             /* added point                         */
    struct CIVdtmtin *tin;             /* working triangle                    */
    DPoint3d loc;                      /* location                            */
    struct CIVdiscon discon;           /* contour params.                     */
    struct CIVdistri distri;           /* triangle params.                    */
    long npnt;                         /* # points in dtm                     */
    long ntin;                         /* # tins in dtm                       */
    long ntinlst;                      /* # tins in list                      */
    long *tinlst;                      /* tin list                            */
    long state;                        /* command state                       */
    int rpt;                           /* repeat point                        */
} DTMrsc_pntedt;





/*----------------------------------------------------------------------------*/
/* Area editing                                                               */
/*----------------------------------------------------------------------------*/

#define DTM_C_AREPAD             1
#define DTM_C_AREHOR             2
#define DTM_C_AREVRT             3
#define DTM_C_AREROT             4
#define DTM_C_ARELIN             5
#define DTM_C_ARETIN             6
#define DTM_C_AREDEL             7
#define DTM_C_AREMOV             8
#define DTM_C_AREMRG             9
#define DTM_C_AREEXT            10


#define AREEDT_INS             0x1     /* edit inside                         */
#define AREEDT_OUT             0x2     /* edit outside                        */
#define AREEDT_TIN             0x4     /* display tins on fly                 */
#define AREEDT_CON             0x8     /* display contours                    */
#define AREEDT_BRK            0x10     /* add edge as breakline               */
#define AREEDT_DEL            0x20     /* remove org srf points               */
#define AREEDT_RELMOV         0x40     /* relative move                       */
#define AREEDT_ABSMOV         0x80     /* absolute move                       */
#define AREEDT_TWOWAY        0x100     /* command is twoway                   */
#define AREEDT_DIS           0x200     /* display line to srf.                */
#define AREEDT_DYN           0x400     /* display dynamics                    */
#define AREEDT_RNDEXTC       0x800     /* round exterior corner               */
#define AREEDT_RNDINTC      0x1000     /* round interior corner               */
#define AREEDT_PADSNG       0x2000     /* single side pad mode                */
#define AREEDT_ELV          0x4000     /* use specified elev.                 */
#define AREEDT_INTERIOR     0x8000     /* interior corner flag                */
#define AREEDT_EXTERIOR    0x10000     /* exterior corner flag                */
#define AREEDT_FIRST       0x20000     /* do first half of crnr               */
#define AREEDT_LAST        0x40000     /* do last half of crnr                */
#define AREEDT_INSIDE      0x80000     /* model inside of pad                 */
#define AREEDT_BOTH       0x100000     /* model boths sides                   */
#define AREEDT_PHANTOM    0x200000     /* phantom side                        */
#define AREEDT_LINPRC     0x400000     /* returned lin. prc flg               */
#define AREEDT_NOTRIL     0x800000     /* don't auto tri w/lin                */
#define AREEDT_FTRAPND   0x1000000     /* append features with same name      */
#define AREEDT_FTRREPL   0x2000000     /* replace features with same name     */
#define AREEDT_FTRRENM   0x4000000     /* rename features with same name      */
#define AREEDT_FTRFLTR   0x8000000     /* only process features passing filter*/
#define AREEDT_RETDNT   0x10000000     /* retain features excluded from trian.*/
#define AREEDT_FTRLST   0x20000000     /* process only features in list       */
#define AREEDT_FTRDSP   0x40000000     /* display features while processing   */
#define AREEDT_MRGORG   0x80000000     /* merge selected areas from original ground */


#define AREEDT_LEN       24            /* max. text field len.                */
#define AREEDT_MAXSIDES  11            /* max. single side info               */
#define AREEDT_DELTA     1e8           /* use in shooting vec.                */
#define AREEDT_OFFDIST   0.1           /* for 'vertical' sides                */
#define AREEDT_ALCSIZ    100           /* allocation block size               */

#define AREEDT_INIT      1             /* initialize things                   */
#define AREEDT_STRD      2             /* start dynamics                      */
#define AREEDT_PRCD      3             /* process dynamics                    */
#define AREEDT_STPD      4             /* stop dynamics                       */
#define AREEDT_ADD       5             /* place pad                           */
#define AREEDT_DONE      6             /* clean things up                     */
#define AREEDT_HILITE    7             /* hilite triangles                    */
#define AREEDT_DELETE    8             /* delete triangles                    */


struct CIVarepadSideData               /* data for each side                  */
{
    double slope;                      /* side slope                          */
    double maxDistance;                /* maximum distance                    */
    short sideShots;                   /* # of side shots                     */
    short cornerShots;                 /* # corner shots                      */
    short opt;                         /* options                             */
    short side;                        /* side data pertains to               */
};


struct CIVpadPoints                    /* data for each vector                */
{
    DPoint3d points[7];
    struct CIVdtmtin *tin;
    double angle;
    double direction;
    double maxDistance;
    long numPoints;
    long opt;
};


struct CIVpadVectors                   /* data for vectors                    */
{
    struct CIVpadPoints *vectors;
    long numVectors;
    long opt;
};


/*----------------------------------------------------------------------------*/
/* Sloped surface                                                             */
/*----------------------------------------------------------------------------*/

#define DTMsiz_slpsrf    (offsetof(struct CIVslpsrf,actSrfP)

typedef struct CIVslpsrf
{
    long sts;                          /* returned status                     */
    long opt;                          /* see above defines                   */
    long actSrfPtr;                    /* for dialog box                      */
    long trgSrfPtr;                    /* for dialog box                      */
    long npntAdded;                    /* # points added                      */
    void *ftrHndl;                     /* Handle for CFeature object          */
    struct CIVdtmsrf *actSrfP;         /* existing surface ptr                */
    struct CIVdtmsrf *trgSrfP;         /* target surface ptr                  */
} CIVrsc_slpsrf;

