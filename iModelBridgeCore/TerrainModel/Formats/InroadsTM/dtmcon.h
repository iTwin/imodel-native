//---------------------------------------------------------------------------+
// $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmcon.h                                            aec    07-Sep-1990     */
/*----------------------------------------------------------------------------*/
/* Constants related to the dtm structures.                                   */
/*----------------------------------------------------------------------------*/

#pragma once

#define SRF_C_MAXDTM              25   /* maximum # surfaces                  */
#define DTM_C_FTRHSHSIZ         5003
#define DTM_C_CORHSHSIZ           11
#define DTM_C_CMPHSHSIZ          251
#define DTM_C_CMPMEMHSHSIZ      1009

#define DTM_C_NAMSIZ              64   /* sie of name in DTM structures       */

#define DTM_C_DTMCOD            "DTM"  /* identifies .dtm file                */ /* DO_NOT_TRANSLATE */
#define DTM_C_DTMVER              11   /* current .dtm version                */

                                       /* surface bit fld vals                */
#define DTM_C_SRFMOD             0x1   /* surface mod. bit                    */
#define DTM_C_GENINF             0x2   /* gen. inferred b.l.'s                */
#define DTM_C_DTMPRJ             0x8   /* included (save) dtm in proj.        */
#define DTM_C_SAVTTN            0x10   /* save file as .TTN                   */
#define DTM_C_PRJUDT            0x20   /* write surface to binary file        */
#define DTM_C_NOGUI             0x40   /* NO GUI                              */

#define DTM_C_BLKSIZFTR         (1000000 / sizeof (struct CIVdtmftr))
#define DTM_C_BLKSIZTIN         (1000000 / sizeof (struct CIVdtmtin))
#define DTM_C_BLKSIZ             100   /* data block size                     */
#define DTM_C_RNGTOL              10   /* range cube tolerance                */

// Surface types (for volumes)
#define DTM_C_SRFEXT               0   /* exisiting                           */
#define DTM_C_SRFDES               1   /* design                              */
#define DTM_C_SRFSUB               2   /* sub strata                          */
#define DTM_C_SRFIGN               3   /* ignore                              */
#define DTM_C_SRFSGD               4   /* subgrade                            */

#define DTM_C_DTMREG               0   /* random point type                   */
#define DTM_C_DTMBRK               1   /* breakline point type                */
#define DTM_C_DTMINT               2   /* interior boundary                   */
#define DTM_C_DTMEXT               3   /* exterior boundary                   */
#define DTM_C_DTMCTR               4   /* contour point type                  */
#define DTM_C_DTMINF               5   /* inferred point type                 */
#define DTM_C_DTMUTI               6   /* utility point type                  */
#define DTM_C_DTMNUL               7   /* null point type                     */
#define DTM_C_DTMRNG              14   /* range point type                    */
#define DTM_C_DTMTIN              15   /* triangle type                       */
#define DTM_C_DTMSTY              16   /* style type                          */
#define DTM_C_DTMREGFTR           17   /* random point feature type           */
#define DTM_C_DTMBRKFTR           18   /* breakline feature type              */
#define DTM_C_DTMINTFTR           19   /* interior boundary feature           */
#define DTM_C_DTMEXTFTR           20   /* exterior boundary feature           */
#define DTM_C_DTMCTRFTR           21   /* contour feature                     */
#define DTM_C_DTMPAY              22   /* pay item type                       */
#define DTM_C_DTMCOR              23   /* corridor                            */
#define DTM_C_DTMCMP              24   /* component                           */
#define DTM_C_DTMCMPMEM           25   /* shape component                     */

#define DTM_C_NMPNTFV3             4   /* vers. 3 # pnt types                 */
#define DTM_C_NMPNTF               6   /* num. of point types                 */
#define DTM_C_FSTFTRF              17   /* first feature type                  */
#define DTM_C_NMFTRF               5   /* num. of feature types               */

#define DTM_C_REGMSK             0x1   /* random point mask                   */
#define DTM_C_BRKMSK             0x2   /* breakline mask                      */
#define DTM_C_INTMSK             0x4   /* int. boundary mask                  */
#define DTM_C_EXTMSK             0x8   /* ext. boundary mask                  */
#define DTM_C_CTRMSK            0x10   /* contour point mask                  */
#define DTM_C_INFMSK            0x20   /* inferred point mask                 */
#define DTM_C_ALPMSK            0x3F   /* all dtm point types                 */
#define DTM_C_UTIMSK            0x40   /* utiilty feature mask                */
#define DTM_C_NULMSK            0x80   /* null feature mask                   */
                                       /* 'opt' variable flags                */
#define DTM_C_INSIDE             0x2   /* get inside features                 */
#define DTM_C_OUTSIDE            0x4   /* get outside features                */
#define DTM_C_SNDONE             0x8   /* single point send                   */
#define DTM_C_DELETE            0x10   /* use del/non-del pnts                */
#define DTM_C_FENCE             0x20   /* use fence, if active                */
#define DTM_C_OPNTIN            0x40   /* return open tin's                   */
#define DTM_C_NOBREK            0x80   /* don't interrupt prc.                */
#define DTM_C_NOOVER           0x100   /* no overlapping tins                 */
#define DTM_C_NOCONS           0x200   /* don't send con. pnts                */
#define DTM_C_CLSTIN           0x400   /* return closed tin's                 */
#define DTM_C_FNCINS           0x800   /* chk fence inside tin                */
#define DTM_C_PNTWEL          0x1000   /* send only points with elevations    */
#define DTM_C_PNTWOE          0x2000   /* send only points without elevs.     */
#define DTM_C_NODFYS          0x4000   /* don't send density points           */
#define DTM_C_NOTINS          0x8000   /* don'd send points excluded from tri.*/
#define DTM_C_USETHD      0x80000000   /* use threads, if available           */


/* Options for aecDTM_addFeature.  Indicates how a feature being added to a   */
/* DTM should be processed if a feature with the same name already exist in   */
/* the DTM.                                                                   */
#define DTM_C_RENAME               0   /* Generates a unique feature name     */
#define DTM_C_APPEND               1   /* Appends the two features together   */
#define DTM_C_REPLACE              2   /* Replaces the existing feature       */

/* Options for aecDTM_setFeatureInfo.  These flags can be passed by the       */
/* caller to optimize retriangulation  of features as they are edited.        */
#define DTM_C_DELONLY              4    /* Points were deleted only           */
#define DTM_C_ADDONLY              5   /* Points were added only              */
#define DTM_C_MOVONLY              6   /* Points were moved only              */
#define DTM_C_ELVONLY              7   /* Point elevations changed only       */


#define DTM_C_NUMPRFOFF           16   /* number of profile offsets in srf. properties */

#define DTM_C_PRFOFF1              0   /* Indexes of profile offsets in prfoff */
#define DTM_C_PRFOFF2              1
#define DTM_C_PRFOFF3              2
#define DTM_C_PRFOFF4              3
#define DTM_C_PRFOFF5              4
#define DTM_C_PRFOFF6              5
#define DTM_C_PRFOFF7              6
#define DTM_C_PRFOFF8              7
#define DTM_C_PRFOFF9              8
#define DTM_C_PRFOFF10             9
#define DTM_C_PRFOFF11             10
#define DTM_C_PRFOFF12             11
#define DTM_C_PRFOFF13             12
#define DTM_C_PRFOFF14             13
#define DTM_C_PRFOFF15             14
#define DTM_C_PRFOFF16             15

                                              /* plot.c specific's            */
#define DTM_C_MAXOFF    (DTM_C_NUMPRFOFF + 1) /* sixteen offset prf's max     */
#define CIV_C_MAXOFF    DTM_C_MAXOFF          /* for compatibility            */

#define DTM_C_CORCTRLALG           0   /* corridor parent control line is ftr */
#define DTM_C_CORCTRLFTR           1   /* corridor parent control line is alg */

                                       /* component member types              */
#define DTM_C_CMPMEMFTR            1   /* feature component member            */
#define DTM_C_CMPMEMSRF            2   /* surface component member            */
#define DTM_C_CMPMEMOVR            3   /* overlay component member            */

                                       /* point bit field vals.               */
#define DTM_C_PNTDEL             0x1   /* delete bit                          */
#define DTM_C_PNTPRC             0x2   /* processing bit                      */
#define DTM_C_PNTTIN             0x4   /* do not triangulate                  */
#define DTM_C_PNTPUD             0x8   /* pen up/down flag                    */
#define DTM_C_PNTCON            0x10   /* construction point                  */
#define DTM_C_PNTELV            0x20   /* don't use input elv.                */
#define DTM_C_PNTIGN            0x40   /* ignore segment                      */
#define DTM_C_PNTDFY            0x80   /* density point                       */

                                       /* tin bit field values                */
#define DTM_C_TINDEL             0x1   /* delete bit                          */
#define DTM_C_TINPRC             0x2   /* processing bit                      */
#define DTM_C_TINFIR             0x4   /* first tin in block                  */
#define DTM_C_TINB12             0x8   /* side 12 is breakline                */
#define DTM_C_TINB23            0x10   /* side 23 is breakline                */
#define DTM_C_TINB31            0x20   /* side 31 is breakline                */
#define DTM_C_TINREM            0x40   /* removed from network                */

                                       /* feature bit field values            */
#define DTM_C_FTRDEL             0x1   /* delete bit                          */
#define DTM_C_FTRPRC             0x2   /* processing bit                      */
#define DTM_C_FTRTIN             0x4   /* do not triangulate                  */

                                       /* style bit field values              */
#define DTM_C_STYDEL             0x1   /* delete bit                          */

                                       /* pay item bit field values           */
#define DTM_C_PAYDEL             0x1   /* delete bit                          */

                                       /* corridor bit field values           */
#define DTM_C_CORDEL             0x1   /* delete bit                          */

                                       /* component bit field values          */
#define DTM_C_CMPDEL             0x1   /* delete bit                          */
#define DTM_C_CMPCLS             0x2   /* closed bit                          */
#define DTM_C_CMPLST             0x4   /* last bit                            */
#define DTM_C_CMPOVR             0x8   /* overlay type bit                    */
#define DTM_C_CMPRMV             0x10  /* removal type bit                    */

                                       /* component overlay top follow values */
#define DTM_C_CMPTOPFOLSRF       0     /* follows surface                     */
#define DTM_C_CMPTOPFOLSEG       1     /* follows segments                    */
#define DTM_C_CMPTOPFOLOPT       2     /* follows highest                     */

                                       /* component overlay botom follow values */
#define DTM_C_CMPBOTFOLSRF       0     /* follows surface                     */
#define DTM_C_CMPBOTFOLSEG       1     /* follows segments                    */
#define DTM_C_CMPBOTFOLOPT       2     /* follows lowest                      */
#define DTM_C_CMPBOTFOLLOW       3     /* follows highest                     */

                                       /* component member bit field values   */
#define DTM_C_CMPMEMDEL          0x1   /* delete bit                          */
#define DTM_C_CMPMEMVOID         0x2   /* void bit                            */

                                       /* file bit field value                */
#define DTM_C_FILEXR             0x1   /* exercise bit                        */
#define DTM_C_NORANG             0x2   /* don't compute range                 */

                                       /* DTMWorks logicals                   */
#define DTM_C_SWPRJD   "CIVIL_PRJ_DIR" /* project files                       */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWPRFF   "CIVIL_PRF_DIR" /* preference files                    */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWDTMD   "CIVIL_DTM_DIR" /* .dtm files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWDATD   "CIVIL_DAT_DIR" /* .dat files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWPNTD   "CIVIL_DAT_DIR" /* .pnt files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWTIND   "CIVIL_DAT_DIR" /* .tin files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWSOED   "CIVIL_DAT_DIR" /* .soe files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWSED    "CIVIL_DAT_DIR" /* .se  files                          */ /* DO_NOT_TRANSLATE */
#define DTM_C_SWDBA    "CIVIL_DBA_DIR" /* dbaccess  files                     */ /* DO_NOT_TRANSLATE */

                                   /* Sizes for file I/O                      */
#define CIVdtmpntSize     (  sizeof(DPoint3d)   + \
			     sizeof(byte)        )
#define CIVdtmtinSize     (3*sizeof(long) + \
			   3*sizeof(long) + \
			     sizeof(byte)        )
#define CIVdtmftrSize (offsetof (struct CIVdtmftr, flg) + 8)
#define CIVdtmftrV8Size (offsetof (struct CIVdtmftrV8, flg) + 8)
#define CIVdtmftrV7Size (offsetof (struct CIVdtmftrV7, flg) + 8)
//(sizeof(struct CIVdtmftr) - (sizeof (void*)* 3))
#define CIVdtmstySize (sizeof(struct CIVdtmsty))
#define CIVdtmpaySize (sizeof(struct CIVdtmpay))
#define CIVdtmcorSize (sizeof(struct CIVdtmcor))
#define CIVdtmcmpSize (sizeof(struct CIVdtmcmp))
#define CIVdtmcmpmemSize (sizeof (struct CIVdtmcmpmem))
#define CIVdtmfilSize     (2*sizeof(DPoint3d)   + \
			     sizeof(long) + \
			   3*sizeof(long)               + \
			     sizeof(byte)        )

