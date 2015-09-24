
/*-------------------------------------------------------------------------------------+
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"

/*----------------------------------------------------------------------+
|                                                                       |
|   dgc.15jun10  - Created.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
CIVdtmsrf::CIVdtmsrf()
    {
    memset(&guid, 0, sizeof(guid));
    memset(pth, 0, sizeof(pth));
    memset(fil, 0, sizeof(fil));           /* file name                           */
    memset(nam, 0, sizeof(nam));           /* surface name                        */
    memset(des, 0, sizeof(des));           /* surface description                 */
    memset(mat, 0, sizeof(mat));           /* surface material                    */
    memset(pref, 0, sizeof(pref));         /* surface preference name             */
    memset(secsym, 0, sizeof(secsym));     /* named symbology for xsections       */
    memset(prfsym, 0, sizeof(prfsym));     /* named symbology for profiles        */
    memset(revby, 0, sizeof(revby));       /* surface last revision author        */
    memset(&revdate, 0, sizeof(revdate));  /* surface last revision date          */
    memset(prfoff, 0, sizeof(prfoff));     /* profile offsets                     */
    prj = NULL;                            /* owner project                       */
    tinf = NULL;                           /* triangles                           */
    corf = NULL;                           /* corridors                           */
    cmpf = NULL;                           /* components                          */
    cmpMemf = NULL;                        /* component members                   */
    styf = NULL;                           /* feature styles                      */
    payf = NULL;                           /* feature pay items                   */
    regFtrf = NULL;                        /* point features                      */
    brkFtrf = NULL;                        /* breakline features                  */
    intFtrf = NULL;                        /* interior boundry features           */
    extFtrf = NULL;                        /* exterior boundry features           */
    ctrFtrf = NULL;                        /* countour features                   */
    memset(ftrf, 0, sizeof(ftrf));
    regf = NULL;                           /* random points                       */
    brkf = NULL;                           /* break line points                   */
    intf = NULL;                           /* interior boundarie points           */
    extf = NULL;                           /* exterior boundarie points           */
    ctrf = NULL;                           /* contour points                      */
    inff = NULL;                           /* inferred breaklines                 */
    rngf = NULL;                           /* range points                        */
    memset(pntf, 0, sizeof(pntf));
    memset(&par, 0, sizeof(par));          /* surface parameters                  */
    memset(&dis, 0, sizeof(dis));          /* display functions                   */
    version = 0;                           /* version number                      */
    codePage = 0;                          /* code page where saved               */
    flg = 0;                               /* bit field flags                     */
    memset(pad, 0, sizeof(pad));           /* padding                             */
    ntinstk = 0;                           /* tin stack counter                   */
    tinstk = NULL;                         /* tin stack                           */
    ptrIndexTableP = NULL;                 /* used to keep ptr/index table alive  */
    indexPtrTableP = NULL;                 /* used to keep index/ptr table alive  */
    ftrGUIDMapP = NULL;                    /* hash table of feature BeGuid's        */
    ftrNameMapP = NULL;                    /* hash table of feature Names         */
    corGuidMapP = NULL;                    /* corridors mapped by guid            */
    corNameMapP = NULL;                    /* corridors mapped by name            */
    cmpGuidMapP = NULL;                    /* components indexed by guid          */
    cmpMemGuidMapP = NULL;                 /* component members indexed by guid   */
    cmpMemParentMapP = NULL;               /* component members indexed by parent */
    }
