//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmttn.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants required for processing .TTN files           */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#define    EXTEN_CODE_SIZE        3

/***    structure sizes        ***/
#define ENV_HDR_SIZE             65
#define TXFILE_SIZE              56   /* was (TXOFILE_SIZE) 65 */
#define TXPARAMS_SIZE           285
#define TXRANGE_SIZE             12   /* supposed to be 24? */
#define TXTIN_SIZE               25
#define TXXYZ_SIZE               16
#define TTN_FILE_HEADER_SIZE    792
#define TXFEATURE_SIZE           64
#define TXOFILE_SIZE             65

/***    data type sizes        ***/
#define LONG_SIZE                 4
#define PTR_SIZE                  4


#ifndef BILL_LEN_LIM_TriM
#    define BILL_LEN_LIM_TriM 40
#endif
#ifndef BILL_LEN_LIM
#    define BILL_LEN_LIM BILL_LEN_LIM_TriM
#endif

#ifndef BILL_MSG_SIZE_TriM
#    define BILL_MSG_SIZE_TriM 2*BILL_LEN_LIM_TriM+1
#endif
#ifndef BILL_MSG_SIZE
#    define BILL_MSG_SIZE BILL_MSG_SIZE_TriM
#endif



/*----------------------------------------------------------------------------*/
/* (trmdslco.h)                                                               */
/*----------------------------------------------------------------------------*/

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

/*   XYZ file types  */

#define TX_REGULAR   0
#define TX_BREAK     1
#define TX_SPOT      2
#define TX_OBSCURE   3
#define TX_EDGE      4
#define TX_FAULT     5
#define TX_CONTOUR   6
#define TX_INFERRED  7
#define TX_CHECK     8
#define TX_PLANE     9
#define TX_RIDGE    10
#define TX_DRAIN    11
#define TX_PEAK     12
#define TX_PIT      13
#define TX_FENCE    14
#define TX_TIN      15


#define TX_MASK_REGULAR      (1 << TX_REGULAR)
#define TX_MASK_BREAK        (1 << TX_BREAK)
#define TX_MASK_SPOT         (1 << TX_SPOT)
#define TX_MASK_OBSCURE      (1 << TX_OBSCURE)
#define TX_MASK_EDGE         (1 << TX_EDGE)
#define TX_MASK_FAULT        (1 << TX_FAULT)
#define TX_MASK_CONTOUR      (1 << TX_CONTOUR)
#define TX_MASK_INFERRED     (1 << TX_INFERRED)
#define TX_MASK_CHECK        (1 << TX_CHECK)
#define TX_MASK_PLANE        (1 << TX_PLANE)
#define TX_MASK_RIDGE        (1 << TX_RIDGE)
#define TX_MASK_DRAIN        (1 << TX_DRAIN)
#define TX_MASK_PEAK         (1 << TX_PEAK)
#define TX_MASK_PIT          (1 << TX_PIT)
#define TX_MASK_FENCE        (1 << TX_FENCE)
#define TX_MASK_TIN          (1 << TX_TIN)

#define NUM_XYZ_FILES 14

/*   versions  */

#define TTN_VERSION 4         /*  A jump from version 1 to 4. To keep TTN */
                  /*  and XYZ versions the same. */
#define TTN_OLD_VERSION 1
#define XYZ_VERSION 4

/*    limits  */

#ifndef VHI_TriM
#define VHI_TriM    0x7fffffff
#endif
#ifndef VHI
#define VHI    VHI_TriM
#endif

#ifndef VLO_TriM
#define VLO_TriM    0x80000000
#endif

#ifndef FEATURE_NAME_SIZE_TriM
#define FEATURE_NAME_SIZE_TriM 80
#endif


#ifndef VLO
#define VLO    VLO_TriM
#endif
#ifndef NAME_SIZE_TriM
#define NAME_SIZE_TriM 128
#endif


#ifndef NAME_SIZE
#define NAME_SIZE NAME_SIZE_TriM
#endif

#ifndef PATH_SIZE_TriM
#define PATH_SIZE_TriM 128
#endif
#ifndef PATH_SIZE
#define PATH_SIZE PATH_SIZE_TriM
#endif

#ifndef DESC_SIZE_TriM
#define DESC_SIZE_TriM 256
#endif
#ifndef DESC_SIZE
#define DESC_SIZE DESC_SIZE_TriM
#endif


#ifndef INPUT_LENGTH_TriM
#define INPUT_LENGTH_TriM 80
#endif
#ifndef INPUT_LENGTH
#define INPUT_LENGTH INPUT_LENGTH_TriM   /*  input text length */
#endif

#ifndef MAX_ENVS_TriM
#define MAX_ENVS_TriM 32
#endif
#ifndef MAX_ENVS
#define MAX_ENVS MAX_ENVS_TriM
#endif

#ifndef UNIT_SIZE_TriM
#define UNIT_SIZE_TriM 32
#endif

#ifndef PROJ_SIZE_TriM
#define PROJ_SIZE_TriM 80
#endif

#ifndef DATUM_SIZE_TriM
#define DATUM_SIZE_TriM 80
#endif

#ifndef ATTR_NAME_SIZE_TriM
#define ATTR_NAME_SIZE_TriM 32
#endif


#ifndef TMVOID
#define TMVOID     0x80000000
#endif

/* TRIMDSL supplied interpolation methods and descriptions */

/*  Planar interpolation

        TMIalloc_planar, 
    TMIinit_planar, 
    TMIplanar, 
    TMIfree_planar
*/
#ifdef HARD_CODED_STRINGS
#ifndef PLANAR_TriM
#define PLANAR_TriM "PLAN"                                         /* DO_NOT_TRANSLATE */
#endif
#ifndef PLANAR
#define PLANAR PLANAR_TriM   
#endif

#ifndef PLANAR_D_TriM
#define PLANAR_D_TriM  "Planar Interpolation based on a triangle." /* DO_NOT_TRANSLATE */
#endif
#ifndef PLANAR_D
#define PLANAR_D  PLANAR_D_TriM
#endif

/* planar aspect interpolation

    TMIalloc_aspect, 
    TMIinit_aspect, 
    TMIaspect,
    TMIfree_aspect
*/

#ifndef ASPECT_TriM
#define ASPECT_TriM  "ASPE"                                        /* DO_NOT_TRANSLATE */
#endif
#ifndef ASPECT
#define ASPECT   ASPECT_TriM
#endif

#ifndef ASPECT_D_TriM
#define ASPECT_D_TriM "Aspect Interpolation is base on the \
aspect of the triangle."                                           /* DO_NOT_TRANSLATE */
#endif
#ifndef ASPECT_D
#define ASPECT_D ASPECT_D_TriM
#endif

/* planar slope interpolation

    TMIalloc_slope, 
    TMIinit_slope, 
    TMIslope, 
    TMIfree_slope
*/

#ifndef SLOPE_TriM
#define SLOPE_TriM "SLOP"                                          /* DO_NOT_TRANSLATE */
#endif
#ifndef SLOPE
#define SLOPE SLOPE_TriM
#endif

#ifndef SLOPE_D_TriM
#define SLOPE_D_TriM "Slope Interpolation is based on the \
slope of the triangle."                                            /* DO_NOT_TRANSLATE */
#endif
#ifndef SLOPE_D
#define SLOPE_D SLOPE_D_TriM
#endif

/* inverse distance weighting interpolation

    TMIalloc_inverse, 
    TMIinit_inverse, 
    TMIinverse, 
    TMIfree_inverse
*/

#ifndef INVERSE_INTERP_TriM
#define INVERSE_INTERP_TriM "INVE"                                 /* DO_NOT_TRANSLATE */
#endif
#ifndef INVERSE_INTERP
#define INVERSE_INTERP INVERSE_INTERP_TriM
#endif

#ifndef INVERSE_D_TriM
#define INVERSE_D_TriM "Based on inverse distance weighting \
from triangle vertices."                                           /* DO_NOT_TRANSLATE */
#endif
#ifndef INVERSE_D
#define INVERSE_D INVERSE_D_TriM
#endif

#endif /* HARD_CODED_STRINGS */

/*  error codes returned by TMroutines  */


/*   INFORMATIONAL   */

#define FINISPROC     1  /* Processing Completed */
#define ERDUPFEA      3  /* duplicate feature name */
#define OLDVERSN      5  /* file is old version   */

/*   ERRORS   */
 
#define NOPNTINDB     2  /* No Elements Found in Data Base */
#define NOCONPROD     4  /* No contours produced */
#define ERRPRODET     6  /* Error creating an Elevation Table */
#define CDNOTLOAD     8  /* A Codetable has not been defined */
#define NOTINPROD    10  /* No .TIN file produced */
#define NOELONLEV    12  /* No Elements Found on Specified Level */
#define NOTMEMORY    14  /* Memory Allocation Failure */
#define MULTOOLRG    16  /* Multiplier too large for data */
#define INVAREA      138 /* Invalid area */
#define NOTPLANAR    140 /* Feature is not coplanar */
#define INVCONTOUR   142 /* Invalid contour */

/*   file i/o   */

#define EROPENFIL    18  /* Error opening file */
#define ERCLOSFIL    20  /* Error closing file */
#define ERCREAFIL    22  /* Error creating file */
#define ERREADHDR    24  /* Error reading file header */
#define ERREADFIL    26  /* Error reading from file */
#define ERWRITHDR    28  /* Error writing file header */
#define ERWRITFIL    30  /* Error writing to file */
#define ERRRETRIV    32  /* Error Retrieving Element */

/*   XYZ file status   */

#define XYZTYPINV    34  /* File header contains invalid type */
#define INVLDFILE    36  /* Invalid file format */
#define NOXYZPROD    38  /* No .XYZ file produced */
#define FLNOTHERE    40  /* File does not exists  */

#define NOTALLTTN    42  /* not all ttn's where loaded without error  */
#define MULTIZERO    44  /* Multiplier is zero */
#define TOOMNYENV    46  /* Too many envs in the project */
#define ERLOADCTB    48  /* Error loading codetable */
#define NOENVACTV    50  /* No env is active */

#define SYNTAXERR    52  /* Syntax Error */
#define UNABLECVT    54  /* Unable to convert units */
#define NOCHKPNTS    56  /* No checkpoints in environment */
#define NOTINSFND    58  /* No Triangles in environment */
#define INCELEVUN    60  /* Inconsistant elevation units */
#define DSIGNIS2D    62  /* Design file is 2D  */

#define STOPSENDG    64  /* stop sending for 'do_to_...' routins */

#define    EREXERDAT        70 /* error exercising data */
#define    ERFREEMEM        72 /* error freeing memory */
#define    ERREMENV        74 /* error removing environment data */

#define    ERLDCDTBL        80 /* error loading codetable */
#define    ERLDEVHDR        82 /* error loading env_header */
#define    ERLDTXENV        84 /* error loading environment structure */
#define    ERLDTXFIL        86 /* error loading file headers */
#define    ERLDTXINT        88 /* error loading interpolation records */
#define    ERLDTXPAR        90 /* error loading parameters */
#define    ERLDTXTIN        92 /* error loading triangle records */
#define    ERLDTXXYZ        94 /* error loading point records */
#define    ERLDXYHDR        96 /* error loading point file header */

#define    ERSVCDTBL        98 /* error saving codetable */
#define    ERSVEVHDR        100 /* error saving env_header */
#define    ERSVTXENV        102 /* error saving environment structure */
#define    ERSVTXFIL        104 /* error saving file headers */
#define    ERSVTXINT        106 /* error saving interpolation records */
#define    ERSVTXPAR        108 /* error saving parameters */
#define    ERSVTXTIN        110 /* error saving triangle records to .ttn */
#define    ERSVTXXYZ        112 /* error saving point records to .ttn */
#define    ERSVERCOD        114 /* error saving version and code */

#define    ERSVXYHDR        116 /* error saving point file header */
#define    ERSVXYREC        118 /* error saving point record to .xyz */
#define    ERSVTNHDR        120 /* error saving tin file header */
#define    ERSVTNREC        122 /* error saving triangle record to .tin */ 

#define ERLONGNAME              126 /* name too long */
#define MATRIXSET               128 /* matrix is already set */
#define ISNOTSET                130 /* the requested info is not present */
#define INVLDMATRIX             132 /* invalid matrix */

#define TOOMANYENV              134 /* too many models */

#define NOMATRIX                136 /* no matrix present */


/*----------------------------------------------------------------------------*/
/* (trmdslst.h)                                                               */
/*----------------------------------------------------------------------------*/

typedef int (*FUNCT_PTR)();

typedef struct TXxyz *TXXP;
typedef struct TXtin  *TXTP;
typedef struct TXfile *TXFP;
typedef struct TXheader *TXHP;
typedef struct TMpf *TXPF;

typedef struct TXxyz TXXS;
typedef struct TXtin  TXTS;
typedef struct TXfile TXFS;
typedef struct TXheader TXHS;

typedef double TXmatrix[16];



struct TXpoint
{
  long x,y,z;
};

struct TXrange
{
  struct TXpoint min, max;

};


struct TXsymbology
{
   byte clr_index;
   byte red;
   byte green;
   byte blue;
   byte weight;
   byte style;
};

#if !defined (mdl)

struct TXxyz      /*   XYZ record  */
{
#ifndef BITFIELDS_REVERSED
  byte pd :1;
  byte rsrv :1; 
  byte type :6;  /* file type */
  byte weight;
  byte weight_flag :1;
  byte processed :1;
  byte dummy :6;
  byte r1 :7;
  byte deleted :1;    /*   point is deletd */
#else
  byte deleted :1;    /*   point is deletd */
  byte r1 :7;
  byte dummy :6;
  byte processed :1;
  byte weight_flag :1;
  byte weight;
  byte type :6;  /* file type */
  byte rsrv :1; 
  byte pd :1;
#endif
  long  x;             /*   X  coordinate   */
  long  y;             /*   Y  coordinate   */
  long  z;             /*   Z  coordinate   */
} ;


struct TXtin   /*  TIN record  */
{
  TXXP p1;    /*  first point  */
  TXXP p2;    /*  second point */
  TXXP p3;    /*  third point  */
  TXTP n12;   /*  adjacent triangle to edge p1-p2 */
  TXTP n23;   /*  adjacent triangle to edge p2-p3 */
  TXTP n31;   /*  adjacent triangle to edge p3-p1 */
#ifndef BITFIELDS_REVERSED
  byte b12 :1;        /*  p1-p2 is part of a break line */
  byte b23 :1;        /*  p2-p3 is part of a break line */
  byte b31 :1;        /*  p3-p1 is part of a break line */
  byte deleted :1;    /*  triangle is deleted  */
  byte user_bit  :1;  /*  This bit can be used as another processed bit*/
  byte removed :1;    /*  triangle is removed from network */
  byte processed :1;  /*  triangle is processed for current
                                process  */
  byte planar :1;     /*  the terrain is planer in this area */
                               /*  no byte alignment needed here because
                                   of how data was written  (wbw 2/93) */
#else
  byte planar :1;     /*  the terrain is planer in this area */
                               /*  no byte alignment needed here because
                                   of how data was written  (wbw 2/93) */
  byte processed :1;  /*  triangle is processed for current
                                process  */
  byte removed :1;    /*  triangle is removed from network */
  byte user_bit  :1;  /*  This bit can be used as another processed bit*/
  byte deleted :1;    /*  triangle is deleted  */
  byte b31 :1;        /*  p3-p1 is part of a break line */
  byte b23 :1;        /*  p2-p3 is part of a break line */
  byte b12 :1;        /*  p1-p2 is part of a break line */
#endif	
} ;

#endif /* mdl */


struct TXheader /*   header of a series of TIN or XYZ records*/
{
  long allocated;       /*  #of TIN records allocted  */
  long used;            /*  #of TIN records used      */
  union
  {
    TXTP tins;  /*  pointer to TIN records    */
    TXXP xyzs;  /*  pointer to XYZ records    */
  } data;
  TXHP next; /*  pointer to the next header in the chain of headers
                 belonging to the same file type  */

  TXHP fea_chain; /*  pointer to the next header in the chain of headers
                      belonging to the same feature  */

  struct TXfeature *fea;
  void *app_ptr;  /* a pointer to whatever software developer wants */
} ;


struct TXfeature
{
  wchar_t *name;
  struct TXrange range;
  long  no_pnts;     /*  number of non deleted points  */
  long  no_deleted;  /*  number of deleted records */
  TXHP data;  /*  pointer to first data header  */
  struct TXfeature *next;
  TXFP file;
  struct TXsymbology symb;
  wchar_t pad1[2];/* for byte alignment (wbw 2/93) */
  void *app_ptr;  /* a pointer to whatever software developer wants */
#ifndef BITFIELDS_REVERSED
  unsigned long exclude :1;  /*  if set, exclude this feature
                 from triangulation */
  unsigned long round_ctr :1; /*  if set, the countours should be smooth */
                              /*  even if the feature is linear/areal */
  unsigned long exercised :1;  /*  if set, the feature has not been modified */
                  /*  since last exercise */
  unsigned long pad2:29;        /* for byte alignment (wbw 2/93) */
#else
  unsigned long pad2:29;        /* for byte alignment (wbw 2/93) */

  unsigned long exercised :1;  /*  if set, the feature has not been modified */
                  /*  since last exercise */
  unsigned long round_ctr :1; /*  if set, the countours should be smooth */
                              /*  even if the feature is linear/areal */

  unsigned long exclude :1;  /*  if set, exclude this feature
                 from triangulation */
#endif	
};

struct TXfile     /*  output file descriptor  */
{
  struct TXrange range;
  long type;
  long  no_pnts;     /*  number of non deleted points  */
  long  no_deleted;  /*  number of deleted records */
  TXHP data;  /*  pointer to first data header  */
  long no_features; /* also used to store mlr in the tinfile */
  struct TXfeature *feature;
  void *app_ptr;  /* a pointer to whatever software developer wants */
#ifndef BITFIELDS_REVERSED
  unsigned long exercised :1;    /*  TRUE if the file has been exercised     */
  unsigned long exclude :1;      /*  if set, exclude this file from triangulation */
  unsigned long triangulate :1;  /* TRUE for immediate triangulation */
  unsigned long pad:29;          /* for byte alignment (wbw 2/93) */
#else
  unsigned long pad:29;          /* for byte alignment (wbw 2/93) */
  unsigned long triangulate :1;  /* TRUE for immediate triangulation */
  unsigned long exclude :1;      /*  if set, exclude this file from triangulation */
  unsigned long exercised :1;    /*  TRUE if the file has been exercised     */
#endif	
};




struct TXinterpol
{
  wchar_t    desc[DESC_SIZE_TriM+1];    /* description */
  wchar_t    code[5];  /*  4 characters code  */
  FUNCT_PTR  alloc;  /* pointer to allocate function  */
  FUNCT_PTR  init;   /* pointer to initiate function  */
  FUNCT_PTR  calc;   /* pointer to calculate function */
  FUNCT_PTR  free;   /* pointer to free function */
  struct TXinterpol *next; /* next interpolation method */
};


/*----------------------------------------------------------------------------*/
/* (filehdrs.h)                                                               */
/*----------------------------------------------------------------------------*/

/*   This file describes the version 2 XYZ and TTN file headers  */

/* these 'MAGIC' definitions are not translatable. They are read from  */
/* and written to TTN and XYZ binary files, and never seen by the user */

#define XYZ_MAGIC L"XYZ"  /* DO_NOT_TRANSLATE */
#define TTN_MAGIC "TTN"  /* DO_NOT_TRANSLATE */

typedef struct
{
  byte  magic[3]; // Do not convert to unicode.
  wchar_t  version;
} hdr_magic;  

typedef struct
{
#ifndef BITFIELDS_REVERSED
  unsigned long   projection_set:1;
  unsigned long   projection:1;
  unsigned long   non_elevation:1;
  unsigned long   continous:1;
  unsigned long   apply_mtx:12;
  unsigned long   resv:16;
#else
  unsigned long   resv:16;
  unsigned long   apply_mtx:12;
  unsigned long   continous:1;
  unsigned long   non_elevation:1;
  unsigned long   projection:1;
  unsigned long   projection_set:1;
#endif	
} hdr_flags;

typedef struct
{
  wchar_t        name[PROJ_SIZE_TriM+1];
  wchar_t        vertical[DATUM_SIZE_TriM+1];
  wchar_t        horizontal[DATUM_SIZE_TriM+1];
} hdr_projection;  

/*   XYZ file header  */

typedef struct
{
  hdr_magic         magic;
  wchar_t           date[26];
  wchar_t           type;
  wchar_t           feature[FEATURE_NAME_SIZE_TriM+1];
  struct TXrange    range;
  hdr_flags         flags;
  TXmatrix          matrix;
  long              no_points;
  long              no_records;
  long              no_deleted;
  wchar_t           description[DESC_SIZE_TriM+1];
  wchar_t           unit[UNIT_SIZE_TriM+1];
  hdr_projection    proj;
  wchar_t           attribute_name[ATTR_NAME_SIZE_TriM+1];
  double            rmse;
} xyz_file_header;


typedef struct ttn_file_header_struct    /* structure name  (wbw 2/93) */
{
  hdr_magic         magic;
  wchar_t           date[26];
  wchar_t           pad1[2];    /* for byte alignment  (wbw 2/93) */
  struct TXrange    range;
  hdr_flags         flags;
  wchar_t           pad2[4];    /* for byte alignment  (wbw 2/93) */
  TXmatrix          matrix;
  long              no_points;
  long              no_records;
  long              no_deleted_points;
  long              no_triangles;
  long              no_deleted_triangles;
  long              no_features;
  wchar_t           description[DESC_SIZE_TriM+1];
  wchar_t           unit[UNIT_SIZE_TriM+1];
  hdr_projection    proj;
  wchar_t           attribute_name[ATTR_NAME_SIZE_TriM+1];
  wchar_t           pad3[2];    /* for byte alignment  (wbw 2/93) */
  double            rmse;
} ttn_file_header;


/*----------------------------------------------------------------------------*/
/* (devstruct.h)                                                              */
/*----------------------------------------------------------------------------*/

struct TXdbgdis  /*  THIS IS ONLY FOR TriM SOFTWARE  */
{
  FUNCT_PTR just_wait_add;
  FUNCT_PTR set_just_wait_add;
  FUNCT_PTR display_triangle_dbg_add;
  FUNCT_PTR display_point_dbg_add;
  FUNCT_PTR set_dbg_view_add;
  FUNCT_PTR flush_dbg_view_add;
  FUNCT_PTR  percent_init_add;
  FUNCT_PTR  percent_next_add;
  FUNCT_PTR  percent_end_add;
};


struct FLjust_point
{
  long x, y, z;
};


struct FLtin
{
  struct TXtin *tin;
  wchar_t type;             /* type = 1, 2, 3 */
  wchar_t its_a_start;
  wchar_t entry_index;
  wchar_t exit_index;
  struct FLtin *next_t;
};


struct FLnode
{
  struct FLpetal    *p;     /* first petal in a circular list
                     of petals which orbit the node */
  struct FLjust_point *pnt; /* geometry of node */
  wchar_t          type;    /* type = 1,3,-1,-2,-3.  Refers to 
                   location of node on triangle.
                   1 = vertex of type 1 tri. 3 = center
                   of type 3 tri. -1 = exit edge of
                   type 1 tri.  -2 = edge of type 2 tri.
                   -3 = edge of type 3 tri. */
  struct FLnode     *nxt_nd; /* next node in the list of node 
                    assoc. with a network */
  wchar_t        visited;
};

struct FLpetal
{
  struct FLnode *nd;        /* end node of the petal */
  struct FLtin  *start_t;
  struct FLtin  *end_t;
  struct FLjust_point *pnt; /* list of points from start to end of 
                   the petal */
  double dist;            /* xy length of list of points */
  wchar_t   trend;         /*   ?  */
  short  no_t;            /* number of tins */
  struct FLpetal *nxt_p;    /* next petal pointer for the circular
                   list of pointers which orbit a 
                   start node */
};


struct FLnetwork
{
  struct FLnode    *start_nd;
  struct FLnode    *end_nd;
  struct FLnetwork *next_n;
  long             no_nd;
};


struct FLpath
{
  long        no_alloc;
  long        no_used;
  struct FLpetal  *petal;
  struct FLpath   *next;
  struct FLpath   *prev;
};


struct FLpath_node
{
  struct FLnode    *node;
  struct FLpath    *path;
  struct FLpath_node *next;
};

struct FLnetwork *FLroot(), *FLend();
struct FLpath_node *FLpath_root();


struct TXparams
{
   ttn_file_header  hdr;
   long multiplier;
   long in_max_length;
   long out_max_length;


/*  MATRICES  */

   TXmatrix env_uor;     /*  converts from model values to design file uors */
   TXmatrix comp_env;    /*  converts from computational to model values  */
   TXmatrix uor_env;     /*  converts from design file uors to model values */
#ifndef BITFIELDS_REVERSED
   unsigned long env_comp_set : 1;
   unsigned long design_comp_set : 1;
#else
   unsigned long design_comp_set : 1;
   unsigned long env_comp_set : 1;
#endif	
};



struct OUTpoint    /* Graphics point */
 {
    long x;
        long y;
    long z;
 };
struct GRApoint    /* Graphics point */
 {
        long outx;
    long outy;
    long outz;
 };


struct TM3d_dbl_vector
{
  double     x, y, z;
};

struct TM2d_int_vector
{
  long    x, y;
};




/*----------------------------------------------------------------------------*/
/* (v1struct.h)                                                               */
/*----------------------------------------------------------------------------*/

/* had a conflict with ustfnc.h
  double * unconvert_double(double * double_buff );
*/


typedef struct TXOfile *TXOFP;
typedef struct TXOfile TXOFS;


#if !defined (mdl)

struct TXOfile     /*  output file descriptor  */
{
  long  line_style;   /*  characteristics of the line which will   */
  long  line_color;   /*  connect the points in this point file.   */
  long  line_weight;  /*  Line_style = -1 replesents no line.      */
  long  symbol;       /*  characteristics of the symbol which will */
  long  symbol_color; /*  be places at each point of this file.    */
  long  symbol_weight;
  struct
  {
   long x;
   long y;
   long z;
  }  min,max;
  long type;
  long  no_pnts;     /*  number of non deleted points  */
  long  no_deleted;  /*  number of deleted records */
  TXHP data;  /*  pointer to first data header  */
#ifndef BITFIELDS_REVERSED
  byte fence_exists :1;  /* =1 if user has defined a fence */
  byte connect :1;       /* TRUE if the points should be connected  */
  byte exercised :1;     /* TRUE if the file has been exercised     */
  byte triangulate :1;   /* TRUE for immediate triangulation */
#else
  byte triangulate :1;   /* TRUE for immediate triangulation */
  byte exercised :1;     /* TRUE if the file has been exercised     */
  byte connect :1;       /* TRUE if the points should be connected  */
  byte fence_exists :1;  /* =1 if user has defined a fence */
#endif	
};

#endif /* mdl */


struct elev_tbl_rec
{
  long index;     /* relative position of rec in the table.
                   The first record has index 0  */
  long elev;    /* actual elevation */
  long color;
  long weight;
  long style;
  long major;    /* TRUE if this elevation is a major contour */
};


struct elev_tbl_hdr
{
  unsigned tbl_size;        /* bytes allocated for elev. table */
  long nor;            /* no of record in elevation table */
  long minz,maxz;            /* range of z values in table      */
  long interval;            /* contour interval after mult.    */
  long   mult;            /* system mult when created        */
  struct elev_tbl_rec *et_pnt;    /* pointer to first record        */
};

