//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmstr.h                                        aec        10-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Structures to do things to and with dtm's.                                 */
/*                                                                            */
/* Changes:    Version 2: changed coordinates to doubles.                     */
/*             Version 3: added material name definition                      */
/*             Version 4: added contour and inferred point types              */
/*             Version 5: added code page id for internationalization         */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files.                                                   */
/*----------------------------------------------------------------------------*/

#include <txtsiz.h>
#include <dtmcon.h>
#include <dtmmac.h>

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

typedef struct CIVdtmpnt               /* dtm point structure                 */
    {
    DPoint3d cor;                      /* point coordinates                   */
    byte flg;                          /* bit field flags                     */
    } CIVdtmpnt;

typedef struct CIVdtmtin               /* dtm triangle struct.                */
    {
    long op1, op2, op3, on12, on23, on31;
    byte flg;                          /* bit field flags                     */
    struct CIVdtmpnt *p1;              /* 1st triangle point                  */
    struct CIVdtmpnt *p2;              /* 2nd triangle point                  */
    struct CIVdtmpnt *p3;              /* 3rd triangle point                  */
    struct CIVdtmtin *n12;             /* 1st neighboring tin                 */
    struct CIVdtmtin *n23;             /* 2nd neighboring tin                 */
    struct CIVdtmtin *n31;             /* 3rd neighboring tin                 */
    } CIVdtmtin;

typedef struct CIVdtmftrV7            /* dtm feature struct. Version 7*/
    {
    GUID guid;
    char nam[CIV_C_NAMSIZ];             /* feature name                        */
    char des[CIV_C_DESSIZ];             /* feature description                 */
    char par[CIV_C_NAMSIZ];             /* parent feature name                 */
    long p[2];
    long numPnts;                       /* number of points in feature         */
    long numStyles;                     /* number of styles assigned to feature*/
    double pntDensity;                  /* point density interval              */
    byte flg;                           /* bit field flags                     */
    struct CIVdtmpnt *p1;               /* 1st feature point                   */
    struct CIVdtmsty *s1;               /* first style                         */
    } CIVdtmftrV7;

typedef struct CIVdtmftrV8              /* dtm feature struct. */
    {
    GUID guid;
    char nam[CIV_C_NAMSIZ];             /* feature name                        */
    char des[CIV_C_DESSIZ];             /* feature description                 */
    char par[CIV_C_NAMSIZ];             /* parent feature name                 */
    long p[3];
    long numPnts;                       /* number of points in feature         */
    long numStyles;                     /* number of styles assigned to feature*/
    long numPayItems;                   /* number of payitems in features      */
    double pntDensity;                  /* point density interval              */
    byte flg;                           /* bit field flags                     */
    struct CIVdtmpnt *p1;               /* 1st feature point                   */
    struct CIVdtmsty *s1;               /* first style                         */
    struct CIVdtmpay *pay;              /* first pay item                      */
    } CIVdtmftrV8;

typedef struct CIVdtmftr               /* dtm feature struct. */
    {
    GUID guid;
    wchar_t nam[DTM_C_NAMSIZ];          /* feature name                        */
    wchar_t des[DTM_C_NAMSIZ];          /* feature description                 */
    wchar_t par[DTM_C_NAMSIZ];          /* parent feature name                 */
    long p[3];
    long numPnts;                     /* number of points in feature         */
    long numStyles;                     /* number of styles assigned to feature*/
    long numPayItems;                   /* number of payitems in features      */
    double pntDensity;                  /* point density interval              */
    byte flg;                           /* bit field flags                     */
    struct CIVdtmpnt *p1;               /* 1st feature point                   */
    struct CIVdtmsty *s1;               /* first style                         */
    struct CIVdtmpay *pay;              /* first pay item                      */
    } CIVdtmftr;

typedef struct CIVdtmstyV8               /* dtm feature style sturcture         */
    {
    char nam[CIV_C_NAMSIZ];              /* style name                          */
    byte flg;                            /* bit field flags                     */
    } CIVdtmstyV8;

typedef struct CIVdtmsty	             /* dtm feature style sturcture         */
    {
    wchar_t nam[CIV_C_NAMSIZ];           /* style name                          */
    byte flg;                            /* bit field flags                     */
    } CIVdtmsty;

typedef struct CIVdtmpayV8	             /* dtm feature pay item sturcture      */
    {
    char nam[CIV_C_NAMSIZ];              /* pay item name                       */
    byte flg;                            /* bit field flags                     */
    } CIVdtmpayV8;

typedef struct CIVdtmpay	             /* dtm feature pay item sturcture      */
    {
    wchar_t nam[CIV_C_NAMSIZ];           /* pay item name                       */
    byte flg;                            /* bit field flags                     */
    } CIVdtmpay;

typedef struct CIVdtmcor               /* dtm corridor structure              */
    {
    GUID guid;                         /* corridor guid                       */
    wchar_t nam[DTM_C_NAMSIZ];         /* corridor name                       */
    GUID ctrlGuid1;                    /* surface or horz. alignment guid     */
    GUID ctrlGuid2;                    /* feature or vert. alignment guid     */
    long ctrlGuidType;                 /* DTM_C_CORCTRLFTR or DTM_C_CORCTRLALG*/
    byte flg;                          /* bit field flags                     */
    } CIVdtmcor;

typedef struct CIVdtmcmpV87prel        /* Pre-release structure               */
    {
    GUID guid;                         /* component guid                      */
    wchar_t nam[DTM_C_NAMSIZ];         /* component name                      */
    GUID corGuid;                      /* guid of parent corridor             */
    wchar_t stynam[CIV_C_NAMSIZ];      /* material style name                 */
    DPoint2d startXY;                  /* start XY of component               */
    DPoint2d stopXY;                   /* stop XY of component                */
    double startStn;                   /* start station of component          */
    double stopStn;                    /* stop station of component           */
    byte flg;                          /* bit field flags                     */
    } CIVdtmcmpV87prel;

typedef struct CIVdtmcmpV10            /* dtm shape structure                 */
    {
    GUID guid;                         /* component guid                      */
    GUID corGuid;                      /* guid of parent corridor             */
    wchar_t nam[DTM_C_NAMSIZ];         /* component name                      */
    wchar_t des[DTM_C_NAMSIZ];         /* component description               */
    wchar_t stynam[CIV_C_NAMSIZ];      /* material style name                 */
    DPoint2d startXY;                  /* start XY of component               */
    DPoint2d stopXY;                   /* stop XY of component                */
    double startStn;                   /* start station of component          */
    double stopStn;                    /* stop station of component           */
    byte flg;                          /* bit field flags                     */
    } CIVdtmcmpV10;

typedef struct CIVdtmcmp               /* dtm shape structure                 */
    {
    GUID guid;                         /* component guid                      */
    GUID corGuid;                      /* guid of parent corridor             */
    wchar_t nam[DTM_C_NAMSIZ];         /* component name                      */
    wchar_t des[DTM_C_NAMSIZ];         /* component description               */
    wchar_t stynam[CIV_C_NAMSIZ];      /* material style name                 */
    DPoint2d startXY;                  /* start XY of component               */
    DPoint2d stopXY;                   /* stop XY of component                */
    double startStn;                   /* start station of component          */
    double stopStn;                    /* stop station of component           */
    long topflg;                       /* follow top flag                     */
    long botflg;                       /* follow bottom flag                  */
    byte flg;                          /* bit field flags                     */
    } CIVdtmcmp;

typedef struct CIVdtmcmpmemV10
    {
    GUID cmpGuid;                      /* guid of parent component            */
    GUID cmpMemGuid;                   /* component member guid               */
    long type;                         /* indicates type of component member  */
    long index;                        /* index of member in component        */
    byte flg;                          /* bit field flags                     */
    } CIVdtmcmpmemV10;

typedef struct CIVdtmcmpmem
    {
    GUID cmpGuid;                      /* guid of parent component            */
    GUID cmpMemGuid;                   /* component member guid               */
    long type;                         /* indicates type of component member  */
    long index;                        /* index of member in component        */
    double componentThickness;         /* only used for overlay members       */
    double surfaceThickness;           /* only used for overlay members       */
    byte flg;                          /* bit field flags                     */
    } CIVdtmcmpmem;

union  CIVdtmunn                       /* union of dtm entities               */
    {
    struct CIVdtmpnt *pnt;             /* pointer to points                   */
    struct CIVdtmtin *tin;             /* pointer to tins                     */
    struct CIVdtmftr *ftr;             /* pointer to features                 */
    struct CIVdtmsty *sty;             /* pointer to styles                   */
    struct CIVdtmpay *pay;             /* pointer to pay items                */
    struct CIVdtmcor *cor;             /* pointer to corridors                */
    struct CIVdtmcmp *cmp;             /* pointer to components               */
    struct CIVdtmcmpmem *cmpMem;       /* pointer to component members        */
    };

struct CIVdtmblk                       /* block of data                       */
    {
    struct CIVdtmblk *nxt;             /* pointer to next blk.                */
    struct CIVdtmfil *fil;             /* pointer to owner                    */
    union  CIVdtmunn rec;              /* pointer to data                     */
    long alc;                          /* number slots alloc.                 */
    long use;                          /* number slots used                   */
    };

struct CIVdtmfil                       /* dtm file structure                  */
    {
    DPoint3d min;                      /* range cube minimum                  */
    DPoint3d max;                      /* range cube maximum                  */
    long a;
    long type;                         /* type of file                        */
    long nrec;                         /* total # of recs.                    */
    long ndel;                         /* number deleted recs.                */
    byte flg;                          /* bit field flags                     */
    struct CIVdtmblk *blk;             /* first data header                   */
    };

struct CIVdtmparV5                     /* general dtm pars. (ver. 5 and earlier) */
    {
    double scl;
    double maxlin;                     /* max. input ftr. len.                */
    double maxsid;                     /* max. tri. side len.                 */
    };

struct CIVdtmparV6                     /* general dtm pars. (ver. 6 and earlier) */
    {
    double scl;
    double maxlin;                     /* max. input ftr. len.                */
    double maxsid;                     /* max. tri. side len.                 */
    long ftrsOnly;                     /* use features only with approriate commmands */
    };

struct CIVdtmpar                       /* general dtm pars.                   */
    {
    double scl;
    double maxlin;                     /* max. input ftr. len.                */
    double maxsid;                     /* max. tri. side len.                 */
    short secFtrsOnly;                 /* use features only when cutting cross section */
    short type;                        /* existing, design, substrata, ignore */
    byte extDatChk;                    /* triangulate with extended data checks */
    byte lockOffSymbs;                 /* lock offset symbologies             */
    byte tinOutOfDate;                 /* triangles are not in synch with dtm data */
    byte lockTin;                      /* lock triangle model                      */
    };

struct CIVdtmdis                       /* display things                      */
    {
    int (*tinfnc)(void *,              /* for interactive dis.                */
    struct CIVdtmtin *,
        int,void *);
    int (*confnc)(void *,              /* for interactive dis.                */
    struct CIVdtmtin *,
        int,void *);
    struct CIVdistri *tinsym;          /* for interactive dis.                */
    struct CIVdiscon *consym;          /* for interactive dis.                */
    };

struct CIVdtmoffV8                     /* profile offset settings             */
    {
    double dis;                        /* offset distance                     */
    char sym[CIV_C_NAMSIZ];            /* offset named symbology              */
    };

struct CIVdtmoff                       /* profile offset settings             */
    {
    double dis;                        /* offset distance                     */
    wchar_t sym[CIV_C_NAMSIZ];         /* offset named symbology              */
    };

#pragma warning (disable:4481)

typedef struct CIVdtmsrf
    {
    public:
        CIVdtmsrf();

    public:
        GUID guid;                         /* surface GUID                        */
        wchar_t pth[CIV_MAX_PATH];         /* file path                           */
        wchar_t fil[CIV_MAX_FNAME];        /* file name                           */
        wchar_t nam[DTM_C_NAMSIZ];         /* surface name                        */
        wchar_t des[DTM_C_NAMSIZ];         /* surface description                 */
        wchar_t mat[CIV_C_NAMSIZ];         /* surface material                    */
        wchar_t pref[CIV_C_NAMSIZ];        /* surface preference name             */
        wchar_t secsym[CIV_C_NAMSIZ];      /* named symbology for xsections       */
        wchar_t prfsym[CIV_C_NAMSIZ];      /* named symbology for profiles        */
        wchar_t revby[CIV_C_NAMSIZ];       /* surface last revision author        */
        SYSTEMTIME revdate;                /* surface last revision date          */
        struct CIVdtmoff prfoff[DTM_C_NUMPRFOFF]; /* profile offsets              */
        struct CIVdtmprj *prj;             /* owner project                       */
        struct CIVdtmfil *tinf;            /* triangles                           */
        struct CIVdtmfil *corf;            /* corridors                           */
        struct CIVdtmfil *cmpf;            /* components                          */
        struct CIVdtmfil *cmpMemf;         /* component members                   */
        struct CIVdtmfil *styf;            /* feature styles                      */
        struct CIVdtmfil *payf;            /* feature pay items                   */
        struct CIVdtmfil *regFtrf;         /* point features                      */
        struct CIVdtmfil *brkFtrf;         /* breakline features                  */
        struct CIVdtmfil *intFtrf;         /* interior boundry features           */
        struct CIVdtmfil *extFtrf;         /* exterior boundry features           */
        struct CIVdtmfil *ctrFtrf;         /* countour features                   */
        struct CIVdtmfil *ftrf[DTM_C_NMFTRF];
        struct CIVdtmfil *regf;            /* random points                       */
        struct CIVdtmfil *brkf;            /* break line points                   */
        struct CIVdtmfil *intf;            /* interior boundarie points           */
        struct CIVdtmfil *extf;            /* exterior boundarie points           */
        struct CIVdtmfil *ctrf;            /* contour points                      */
        struct CIVdtmfil *inff;            /* inferred breaklines                 */
        struct CIVdtmfil *rngf;            /* range points                        */
        struct CIVdtmfil *pntf[DTM_C_NMPNTF+1];
        struct CIVdtmpar par;              /* surface parameters                  */
        struct CIVdtmdis dis;              /* display functions                   */
        unsigned long version;             /* version number                      */
        unsigned long codePage;            /* code page where saved               */
        byte flg;                          /* bit field flags                     */
        byte pad[3];                       /* padding                             */
        long ntinstk;                      /* tin stack counter                   */
        long *tinstk;                      /* tin stack                           */
        void *ptrIndexTableP;              /* used to keep ptr/index table alive  */
        void *indexPtrTableP;              /* used to keep index/ptr table alive  */
        CMapStringToPtr *ftrGUIDMapP;      /* hash table of feature GUID's        */
        CMapStringToPtr *ftrNameMapP;      /* hash table of feature Names         */
        CMapStringToPtr *corGuidMapP;      /* corridors mapped by guid            */
        CMapStringToPtr *corNameMapP;      /* corridors mapped by name            */
        CMapStringToPtr *cmpGuidMapP;      /* components indexed by guid          */
        CMapStringToPtr *cmpMemGuidMapP;   /* component members indexed by guid   */
        CMapStringToPtr *cmpMemParentMapP; /* component members indexed by parent */
    } CIVdtmsrf;

typedef struct CIVdtmprj               /* dtm project struct.                 */
    {
    CIVdtmsrf* *srfs;                        /* pointers to surfaces                */
    short nsrf;                        /* number of surfaces                  */
    short asrf;                        /* active surface                      */
    short asrf1;                       /* active surface2 (target)            */
    short asrf2;                       /* active surface3 (design)            */
    long unused;                       /* for padding                         */
    wchar_t pth[CIV_MAX_PATH];         /* file path                           */
    wchar_t fil[CIV_MAX_FNAME];        /* file name                           */
    wchar_t nam[DTM_C_NAMSIZ];         /* project name                        */
    wchar_t des[DTM_C_NAMSIZ];         /* project description                 */
    } CIVdtmprj;

struct CIVptrind                       /* pointer / index stuff               */
    {
    byte *p;
    size_t ind;
    };

struct CIVptrindTwo                    /* for keeping permanent index table   */
    {
    struct CIVptrind *piP;
    struct CIVptrind *epiP;
    size_t tinOffset;
    byte buildTable;
    };

typedef wchar_t CIVdtmstynam[CIV_C_NAMSIZ];
typedef wchar_t CIVdtmpaynam[CIV_C_NAMSIZ];

#pragma warning (default:4481)
