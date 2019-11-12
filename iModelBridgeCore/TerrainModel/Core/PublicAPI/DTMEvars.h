/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
/*__PUBLISH_SECTION_START__*/
#ifndef __DTMEVARS_H__
#define __DTMEVARS_H__

/*__PUBLISH_SECTION_END__*/
/*-------------------------------------------------------------------+
|                                                                    |
|   Global Variables                                                 |
|                                                                    |
+-------------------------------------------------------------------*/
#ifdef __BENTLEYDTM_BUILD__
BENTLEYDTM_EXPORT extern double  DTM_PYE,DTM_2PYE ;
extern FILE    *fpLOG ; 
/*
**  Global Variables
*/  
extern long    DTM_LAST_DAT_FILE_TYPE ;
extern long    DTM_LAST_DAT_FILE_NUM_DEC_PTS ;
/*-------------------------------------------------------------------+
|                                                                    |
|   BC_DTM Global Object Variables                                   |
|                                                                    |
+-------------------------------------------------------------------*/
extern BC_DTM_BTREE  *glbDtmObjBtreeP       ;    //  Global Btree For BC_DTM_OBJ Occurrences
BENTLEYDTM_EXPORT extern long DTM_NUM_PROCESSORS              ;    //  Number Of Machine Processors
BENTLEYDTM_EXPORT extern long DTM_GEOPAK_ACTIVE               ;    //  Set to 1 If Geopak Active



#else
BENTLEYDTM_EXPORT extern FILE    *fpLOG ; 
BENTLEYDTM_EXPORT extern double  DTM_PYE,DTM_2PYE ;

/*
** Temporary Arrays For Transitioning From Data Objects To Tin Objects
*/
//extern long         numTmpFeatureCodes;
//extern long         *tmpFeatureCodeP ;
//extern DTMUserTag *tmpUserTagP ; 

/*
**  Global Variables
*/  
extern long    DTM_LAST_DAT_FILE_TYPE ;
extern long    DTM_LAST_DAT_FILE_NUM_DEC_PTS ;
/*-------------------------------------------------------------------+
|                                                                    |
|   Global Variables                                                 |
|                                                                    |
+-------------------------------------------------------------------*/
/*-------------------------------------------------------------------+
|                                                                    |
|   BC_DTM Global Object Variables                                   |
|                                                                    |
+-------------------------------------------------------------------*/
extern BC_DTM_BTREE  *glbDtmObjBtreeP                       ;     //  Global Btree For BC_DTM_OBJ Occurrences
BENTLEYDTM_EXPORT extern long DTM_NUM_PROCESSORS            ;     //  Number Of Machine Processors
BENTLEYDTM_EXPORT extern long DTM_GEOPAK_ACTIVE             ;     //  Set to 1 If Geopak Active

#endif

/*__PUBLISH_SECTION_START__*/
#endif
     
/*__PUBLISH_SECTION_END__*/

