//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmdis.h                                            aec    07-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used by DTM display functions.               */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <disfnc.h>
#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Display perimeter things                                                   */
/*----------------------------------------------------------------------------*/

#define DISBND_FLT  0x4                /* fixed elevation                     */

/*----------------------------------------------------------------------------*/
/* Isopach (Display Cut and Fill) Things                                      */
/*----------------------------------------------------------------------------*/

#define DISCAF_INS                0x1  /* inside fence                        */
#define DISCAF_OUT                0x2  /* outside fence                       */
#define DISCAF_PLN                0x4  /* fixed elev.                         */
#define DISCAF_CUT                0x8  /* dis cut only                        */
#define DISCAF_FIL               0x10  /* dis fil only                        */
#define DISCAF_ISO               0x20  /* build isopach                       */

#define DISCAF_CTR               0x40  /* tin centers                         */
#define DISCAF_GRD               0x80  /* grid points                         */
#define DISCAF_ALL              0x100  /* all points                          */
#define DISCAF_MODMSK           0x1C0  /* mode mask                           */

#define DISCAF_TRI              0x200  /* triangulate when complete           */

#define DISCAF_SRF1         0x1000000  /* internal only                       */

/*----------------------------------------------------------------------------*/
/* Display contours things                                                    */
/*----------------------------------------------------------------------------*/

#define DISCON_INS            0x1       /* inside fence                       */
#define DISCON_OUT            0x2       /* outside fence                      */
#define DISCON_FLT            0x4       /* fix elevation                      */
#define DISCON_LBL            0x8       /* generate labels                    */
#define DISCON_SMT           0x10       /* smooth contours                    */
#define DISCON_DIS           0x20       /* segmented display (this is used everywhere besides display contours) */
#define DISCON_LS            0x40       /* dis. as linestring                 */
#define DISCON_CS            0x80       /* dis. as curvestr.                  */
#define DISCON_BS           0x100       /* dis. as b-spline                   */
#define DISCON_DISMSK       0x1E0       /* display mask                       */
#define DISCON_NOR          0x200       /* label faces north                  */
#define DISCON_EAS          0x400       /* label faces east                   */
#define DISCON_WES          0x800       /* label faces west                   */
#define DISCON_SOU         0x1000       /* label faces south                  */
#define DISCON_DIRMSK      0x1E00       /* label direction mask               */
#define DISCON_THN         0x2000       /* thin contour strgs                 */
#define DISCON_ARE         0x4000       /* minimum area flag                  */
#define DISCON_MIN         0x8000       /* minimum elev flag                  */

#define DISCON_LMJ        0x10000       /* label major                        */
#define DISCON_LMN        0x20000       /* label minor                        */
#define DISCON_FTR        0x40000       /* display as feature                 */
#define DISCON_PTMOD      0X80000       /* point & linestring method          */
#define DISCON_MAJOR     0x100000       /* don't display major contours       */
#define DISCON_MINOR     0x200000       /* don't display minor contours       */
#define DISCON_OMITM     0x400000       /* omit minor contours on steep slope */

#define DISCON_CLP      0x1000000       /* don't clip lines                   */
#define DISCON_CLPMSK   0x3000000       /* label clipping mask                */

#define DISCON_MAJDEP   0x4000000       /* don't display major depressions    */
#define DISCON_MINDEP   0x8000000       /* don't display minor depressions    */
#define DISCON_SEG     0x10000000       /* segmented display for display contour command  */
#define DISCON_ELV     0x20000000       /* contour elevation range to display */

typedef struct CIVdiscon
{
    long sts;                          /* returned status                     */
    long srfptr;                       /* for dialog box                      */
    long opt;                          /* see above defines                   */
    long mpm;                          /* minors per major                    */
    long spc;                          /* spacing bet. lbls.                  */
    long ndc;                          /* # dec. in label                     */
    wchar_t pre[CIV_C_NAMSIZ];         /* label prefix                        */
    wchar_t suf[CIV_C_NAMSIZ];         /* label suffix                        */
    double stp;                        /* contour interval                    */
    double fltelv;                     /* fixed elevation                     */
    double base;                       /* base elevation                      */
    double scl;                        /* exaggeration                        */
    double thinAngle;                  /* thinning angle                      */
    double minArea;                    /* minimum area                        */
    struct CADlinpar majpar;           /* major ctr pars.                     */
    struct CADlinpar minpar;           /* minor ctr pars.                     */
    struct CADtxtpar lblpar;           /* label parameters                    */
    double maxSlope;                   /* max slope for omitting contours     */
    wchar_t maxSlopeStr[32];           /* max slope string                    */
    double txh;                        /* real world txt hgt                  */
    double txw;                        /* real world txt wth                  */
    struct CIVdtmsrf *srf;             /* target surface                      */

    void (*insfnc)();                  /* internal only                       */
    void (*outfnc)();                  /* internal only                       */
    double low;                        /* internal only                       */
    double minElv;                     /* internal only                       */
    double maxElv;                     /* internal only                       */
    long *ctr;                         /* internal only                       */
    long actctr;                       /* internal only                       */
    long fnc;                          /* internal only                       */
    void *pBoxes;                      /* internal only                       */
    long boxSeg;                       /* internal only                       */
    long finalJoin;                    /* internal only                       */
    unsigned long nctr;                /* internal only                       */
    unsigned long nLabelCtr;           /* internal only                       */
    long savmpm;                       /* internal only                       */
    double baseLabelElevation;         /* internal only                       */
    double labelInterval;              /* internal only                       */

    int (*polylineCallbackP)           /* => func. to receive lines (or NULL) */
    (
      long nVrt,                       /*    # vertices in polyline           */
      DPoint3d *vrtsP,                 /*    polyline vertices                */
      int isMajor,                     /*    TRUE: major contour, FALSE: minor*/
      void *userDataP                  /*    user data from next argument     */
    );
    void *polylineCallbackUserDataP;   /* => user data passed to P.L. callback*/
    int (*labelCallbackP)              /* => func. to receive labels (or NULL)*/
    (
      wchar_t *textP,                  /*    label text                       */
      DPoint3d *originP,               /*    lower left text origin           */
      double angle,                    /*    orientation angle of text        */
      double height,                   /*    text height                      */
      double width,                    /*    text width                       */
      void *userDataP                  /*    user data from next argument     */
    );
    void *labelCallbackUserDataP;      /* => user data passed to text callback*/

    void *pAttributeData;
    long depCCW;                       /* direction for depression contours   */
    long isDepression;                 /* indicates if current contour is dep.*/
    DPoint3d *curVrts;                 /* points to current contour before clipping */
    int numCurVrts;                    /* number of current countrour points */
    int checkClippedDirs;              /* check direction of clipped contours */
    struct CADlinpar majdeppar;        /* major depression ctr pars.          */
    struct CADlinpar mindeppar;        /* minor depression ctr pars.          */
    struct CADtxtpar minlblpar;        /* minor label parameters              */
    double mintxh;                     /* real world minor txt hgt            */
    double mintxw;                     /* real world minor txt wth            */
    long minspc;                       /* minor spacing bet. lbls.            */
    long minndc;                       /* # dec. in minor label               */
    wchar_t minpre[CIV_C_NAMSIZ];      /* label minor prefix                  */
    wchar_t minsuf[CIV_C_NAMSIZ];      /* label minor suffix                  */
    long mindiropt;                    /* minor orientation                   */

} DTMrsc_discon;

/*----------------------------------------------------------------------------*/
/* Display point elevation things                                             */
/*----------------------------------------------------------------------------*/

#define DISELV_INS     0x1              /* inside fence                       */
#define DISELV_OUT     0x2              /* outside fence                      */
#define DISELV_PLN     0x4              /* planarize                          */
#define DISELV_SNG     0x8              /* single point mode                  */
#define DISELV_CTR    0x10              /* centerpoint mode                   */
#define DISELV_GRD    0x20              /* grid point mode                    */
#define DISELV_ALL    0x40              /* every point mode                   */
#define DISELV_DISMSK 0x78              /* display mode mask                  */
#define DISELV_DEC    0x80              /* align on decimal                   */
#define DISELV_ACAD   0x100             /* running with autocad               */
#define DISELV_TXTJST 0x200             /* align with text justification      */
#define DISELV_DISELV 0x400             /* display Elevation                  */
#define DISELV_DISPNT 0x800             /* display Point                      */

/*----------------------------------------------------------------------------*/
/* Display gridded mesh things                                                */
/*----------------------------------------------------------------------------*/

#define DISGRD_INS  0x1                /* inside fence                        */
#define DISGRD_OUT  0x2                /* outside fence                       */
#define DISGRD_FLT  0x4                /* fix elevation                       */
#define DISGRD_TRU  0x8                /* used with fence                     */

/*----------------------------------------------------------------------------*/
/* Display points things                                                      */
/*----------------------------------------------------------------------------*/

#define DISPNT_INS  0x1                /* inside fence                        */
#define DISPNT_OUT  0x2                /* outside fence                       */
#define DISPNT_FLT  0x4                /* fix a elevation                     */
#define DISPNT_FTR  0x8                /* attach feature tag                  */

                                       /* dispnt->dispntSym.opt bits          */
#define DISPNT_REG  0x01
#define DISPNT_BRK  0x02
#define DISPNT_CTR  0x04
#define DISPNT_INF  0x08
#define DISPNT_INT  0x10
#define DISPNT_EXT  0x20

/*----------------------------------------------------------------------------*/
/* Display profiled model things                                              */
/*----------------------------------------------------------------------------*/

#define DISPRO_INS  0x1                /* inside fence                        */
#define DISPRO_OUT  0x2                /* outside fence                       */
#define DISPRO_FLT  0x4                /* fixed elevation                     */
#define DISPRO_XON  0x8                /* x lines on                          */
#define DISPRO_YON  0x10               /* y lines on                          */
#define DISPRO_DON  0x20               /* datum lines on                      */

#define DISPRO_LEN  24                 /* string length                       */

/*----------------------------------------------------------------------------*/
/* Display triangles things                                                   */
/*----------------------------------------------------------------------------*/

#define DISTRI_INS  0x1                /* inside fence                        */
#define DISTRI_OUT  0x2                /* outside fence                       */
#define DISTRI_FLT  0x4                /* fix at elevation                    */
#define DISTRI_CLM  0x8                /* colored model                       */
#define DISTRI_MSH  0x10               /* mesh element                        */

typedef struct CIVdistri
{
    long sts;                          /* returned status                     */
    long srfptr;                       /* for dialog box                      */
    long opt;                          /* see above defines                   */
    void *pAttributeData;              /* attribute data                      */
    double fltelv;                     /* fixed elevation                     */
    double base;                       /* base elevation                      */
    double scl;                        /* exaggeration                        */
    struct CADlinpar par;              /* display parameters                  */
    struct CIVdtmsrf *srf;             /* target surface                      */
    CIVdtmsty *pStyles;                /* style list                          */
    CADlinpar *pPars;                  /* linpar list                         */
    long numStyleLinpars;              /* number in list                      */
} DTMrsc_distri;

/*----------------------------------------------------------------------------*/
/* Display slope vectors things                                               */
/*----------------------------------------------------------------------------*/

#define DISVEC_INS     0x1             /* inside fence                        */
#define DISVEC_OUT     0x2             /* outside fence                       */
#define DISVEC_FLT     0x4             /* fixed elevation                     */
#define DISVEC_SLP     0x8             /* annotate slope                      */
#define DISVEC_DIR    0x10             /* annotate direction                  */
#define DISVEC_SNG    0x20             /* single point mode                   */
#define DISVEC_CTR    0x40             /* centroid mode                       */
#define DISVEC_GRD    0x80             /* grid mode                           */    
#define DISVEC_VEC    0x100            /* display vector                      */
#define DISVEC_VLEN   0x200            /* display vector length				  */ 

/*----------------------------------------------------------------------------*/
/* Color-table things (used with color-coded display functions)               */
/*----------------------------------------------------------------------------*/

#define CLRTBL_NDAT 255
#define CLRTBL_NCLR (CLRTBL_NDAT+1)
#define CLRTBL_LEN  32

/*----------------------------------------------------------------------------*/
/* Color-table report things (used with color-coded display functions)        */
/*----------------------------------------------------------------------------*/
#define TBLRPT_RPT      0x1            /* generate report file                */
#define TBLRPT_FEET     0x4            /* square feet                         */
#define TBLRPT_YARD     0x8            /* square yards                        */
#define TBLRPT_MILE    0x10            /* square miles                        */
#define TBLRPT_ACRE    0x20            /* acres                               */
#define TBLRPT_METR    0x40            /* square meters                       */
#define TBLRPT_KILO    0x80            /* square kilometers                   */
#define TBLRPT_HECT   0x100            /* hectares                            */
#define TBLRPT_UNTMSK 0x1FC            /* units mask                          */

/*----------------------------------------------------------------------------*/
/* Color-Coded Aspect Things                                                  */
/*----------------------------------------------------------------------------*/

#define DISASP_NASP  255
#define DISASP_NCLR (DISASP_NASP+1)

#define DISASP_INS  0x1                /* inside fence                        */
#define DISASP_OUT  0x2                /* outside fence                       */
#define DISASP_FLT  0x4                /* fixed elevation                     */
#define DISASP_SCL  0x8                /* display scale                       */
#define DISASP_AUT  0x10               /* automatic mode                      */
#define DISASP_UTSF 0x20               /* use text scale factor               */

/*----------------------------------------------------------------------------*/
/* Display color-coded elevations things                                      */
/*----------------------------------------------------------------------------*/

#define DISCTR_NELV  255
#define DISCTR_NCLR  (DISCTR_NELV+1)

#define DISCTR_INS  0x1                /* inside fence                        */
#define DISCTR_OUT  0x2                /* outside fence                       */
#define DISCTR_FLT  0x4                /* fix elevation                       */
#define DISCTR_SCL  0x8                /* display scale                       */
#define DISCTR_AUT  0x10               /* automatic mode                      */
#define DISCTR_UTSF 0x20               /* use text scale factor               */

/*----------------------------------------------------------------------------*/
/* Display color-coded slope things                                           */
/*----------------------------------------------------------------------------*/

#define DISSLP_NSLP  255
#define DISSLP_NCLR (DISSLP_NSLP+1)

#define DISSLP_INS  0x1                /* inside fence                        */
#define DISSLP_OUT  0x2                /* outside fence                       */
#define DISSLP_FLT  0x4                /* fixed elevation                     */
#define DISSLP_SCL  0x8                /* display scale                       */
#define DISSLP_AUT  0x10               /* automatic mode                      */
#define DISSLP_UTSF 0x20               /* use text scale factor               */

/*----------------------------------------------------------------------------*/
/* Drape element(s) on surface things                                         */
/*----------------------------------------------------------------------------*/

#define DRAPE_INS       0x1            /* elements inside fence               */
#define DRAPE_OUT       0x2            /* elements outside fence              */
#define DRAPE_SNG       0x4            /* single element mode                 */
#define DRAPE_LVL       0x8            /* load elements by level              */
#define DRAPE_FEN      0x10            /* load elements within fence          */
#define DRAPE_MODMSK   0x1C            /* mode mask                           */
#define DRAPE_CPX    0x2000            /* internal flag                       */

/*----------------------------------------------------------------------------*/
/* Display colored triangles things                                           */
/*----------------------------------------------------------------------------*/

#define SINGLE 0
#define ALL    1

/*----------------------------------------------------------------------------*/
/* Label contours things                                                      */
/* using discon now 7-94                                                      */
/*----------------------------------------------------------------------------*/

#define LABCON_NOR       0x200         /* label faces north                   */
#define LABCON_EAS       0x400         /* label faces east                    */
#define LABCON_WES       0x800         /* label faces west                    */
#define LABCON_SOU      0x1000         /* label faces south                   */
#define LABCON_DIRMSK   0x1E00         /* label mask                          */

#define LABCON_CLP     0x10000         /* don't skip clip                     */

#define POINTMODE 0
#define LSTMODE 1


/*----------------------------------------------------------------------------*/
/* Slope annotation things                                                    */
/*----------------------------------------------------------------------------*/

#define ANNUTI_CELL           0x01
#define ANNUTI_SYMBOL         0x02
#define ANNUTI_TICK           0x04

#define ANNUTI_DEFAULT_PCTTICK 0.50
#define ANNUTI_ATAN(y,x) (((x) == 0.) ? ( ((y) >= 0.) ? PI12 : -PI12 ) : ( ((x) < 0.) ? PI2+atan2 ((y),(x)) : atan2((y),(x)) ))

/*----------------------------------------------------------------------------*/
/* Report crossing breaklines                                                 */
/*----------------------------------------------------------------------------*/

#define RPTXLN_INSIDE       0x0001     /* inside fence                        */
#define RPTXLN_OUSIDE       0x0002     /* outside fence                       */
#define RPTXLN_DISCRS       0x0010     /* display crossing segments           */
#define RPTXLN_RPTCRS       0x0020     /* report crossing segments            */
#define RPTXLN_DISELV       0x0040     /* display mismatched elevations       */
#define RPTXLN_RPTELV       0x0080     /* report mismatched elevations        */

/*----------------------------------------------------------------------------*/
/* Coordinate tracking                                                        */
/*----------------------------------------------------------------------------*/

#define TRACK_DP          0x1          /* data button input                   */
#define TRACK_NRT         0x10         /* display northing                    */
#define TRACK_EST         0x20         /* display easting                     */
#define TRACK_ELV         0x40         /* display elevation                   */
#define TRACK_SLP         0x80         /* display slope                       */
#define TRACK_ASP         0x100        /* display aspect                      */
#define TRACK_STN         0x200        /* display station                     */
#define TRACK_OFF         0x400        /* display offset                      */
#define TRACK_DDI         0x800        /* display dual dimension              */
#define TRACK_DPT        0x1000        /* display point                       */
#define TRACK_TXT        0x2000        /* display text                        */
#define TRACK_LAT        0x4000        /* display latitude                    */
#define TRACK_LON        0x8000        /* display longitude                   */
#define TRACK_EN     0x40000000        /* display coords as easting then nort.*/
#define TRACK_DISALG 0x80000000        /* display alignment                   */
#define TRACK_ITEMS          10        /* number of tracking list above       */

#define TRACK_MSK        (TRACK_NRT|TRACK_EST|TRACK_ELV|TRACK_SLP|TRACK_ASP|TRACK_STN|TRACK_OFF|TRACK_DDI)

