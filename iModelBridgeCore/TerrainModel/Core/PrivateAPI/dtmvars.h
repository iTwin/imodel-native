/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PrivateAPI/dtmvars.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
** Temporary Arrays For Transitioning From Data Objects To Tin Objects
*/
thread_local long         numTmpFeatureCodes = 0;
thread_local long         *tmpFeatureCodeP = NULL;
thread_local DTM_GUID     *tmpGuidP = NULL;
thread_local DTMUserTag *tmpUserTagP = NULL;
/*
**  Global Error Message
*/
thread_local long    DTM_DTM_ERROR_STATUS = 0;
thread_local long    DTM_DTM_ERROR_NUMBER = 0;
thread_local char    DTM_DTM_ERROR_MESSAGE[256] = { 0 };
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
