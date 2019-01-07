/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/dtmvars.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifndef __DTMVARS_H__
#define __DTMVARS_H__

/*-------------------------------------------------------------------+
|                                                                    |
|   Global Variables                                                 |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT double  DTM_PYE=0.0,DTM_2PYE=0.0 ;
BENTLEYDTM_EXPORT extern FILE    *fpLOG; 

/*
**  Global Variables
*/  
long    DTM_LAST_DAT_FILE_TYPE=0 ;
long    DTM_LAST_DAT_FILE_NUM_DEC_PTS=0 ;
/*-------------------------------------------------------------------+
|                                                                    |
|   BC_DTM Global Object Variables                                   |
|                                                                    |
+-------------------------------------------------------------------*/
BC_DTM_BTREE  *glbDtmObjBtreeP     = NULL ;     // Global Btree For BC_DTM_OBJ Occurrences
BENTLEYDTM_EXPORT long DTM_NUM_PROCESSORS  =  1   ;     // Number Of Machine Processors 
BENTLEYDTM_EXPORT long DTM_GEOPAK_ACTIVE             =  0   ;     //  Set to 1 If Geopak Active

#endif
