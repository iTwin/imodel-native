/*----------------------------------------------------------------------+
|                                                                       |
|  Copyright (1994) Bentley Systems, Inc., All rights reserved.         |
|                                                                       |
|  "MicroStation", "MDL", and "MicroCSL" are trademarks of Bentley      |
|  Systems, Inc.                                                        |
|                                                                       |
|  This program is proprietary and unpublished property of Bentley      |
|  Systems Inc. It may NOT be copied in part or in whole on any medium, |
|  either electronic or printed, without the express written consent    |
|  of Bentley Systems, Inc.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                        |
|    $Logfile:   K:/msj/mstn/mdlapps/measure/measdefs.h_v  $
|   $RCSfile: dtmcommdefs.h,v $
|   $Revision: 1.18 $
|              $Date: 2011/09/29 09:07:49 $
|     $Author: Piotr.Slowinski $
|                                                                        |
+----------------------------------------------------------------------*/
#pragma once 

#define STRINGLISTID_Commands                       0
#define STRINGLISTID_Prompts                        1
#define STRINGLISTID_Messages                       2

/*----------------------------------------------------------------------+
|                                                                       |
|   Message defines                                                     |
|                                                                       |
+----------------------------------------------------------------------*/
#define STRINGID_Message_Main                       0
#define STRINGID_Message_SubSS                      1
#define WMSGLIST_Main                               1

/*----------------------------------------------------------------------+
|                                                                        |
|   Command Names                                                        |
|                                                                        |
+----------------------------------------------------------------------*/
#define     CMDNAME_LabelSpot                   30
#define     CMDNAME_LabelContours               31
#define     CMDNAME_ImportLandXML               32
#define     CMDNAME_ImportTerrainModel          33
#define     CMDNAME_FeatureTrack                34

#define     WSTATUS_NotSelected                 1

/*----------------------------------------------------------------------+
|                                                                       |
|   Prompts                                                             |
|                                                                       |
+----------------------------------------------------------------------*/
#define PROMPT_IdentifyDTM                      120
#define PROMPT_SelectSpot                       121
#define PROMPT_SelectFromPoint                  122
#define PROMPT_SelectToPoint                    123
#define PROMPT_AcceptReject                     124

/*----------------------------------------------------------------------+
|                                                                       |
|   Errors                                                              |
|                                                                       |
+----------------------------------------------------------------------*/

#define ERROR_NotADTM                           224
#define ERROR_NoSurfaceDetected                 225
#define ERROR_UnacceptableWorkMode              226
#define ERROR_UnreadableInputFile               227
#define ERROR_NoContoursDrawn                   228
#define ERROR_InvalidFile                       229

/*----------------------------------------------------------------------+
|                                                                       |
|   Dialog Titles                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define DIALOGTITLE_SelectLandXML           402
#define DIALOGTITLE_SelectDTMFile           403
#define DIALOGTITLE_SelectLandXMLConfig     404
#define DIALOGTITLE_LandXMLFileFilter       405

/*----------------------------------------------------------------------+
|                                                                       |
|   Misc                                                                |
|                                                                       |
+----------------------------------------------------------------------*/
#define RESULT_ImportDTM                            650
#define RESULT_ImportFailedDTM                      651


/*----------------------------------------------------------------------+
|                                                                       |
|   Label Ids                                                           |
|                                                                       |
+----------------------------------------------------------------------*/
#define     LABELID_TerrainModel                    14
#define     LABELID_SelectedTerrainModel            15

/*----------------------------------------------------------------------+
|                                                                       |
|   Option Button Ids                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define     OPTIONBUTTONID_AnnotationCountoursMode  7
#define     OPTIONBUTTONID_AnnotationCountoursTxtAligment  8

/*----------------------------------------------------------------------+
|                                                                       |
|   Annotate Mode Option Button Item Indexes                            |
|                                                                       |
+----------------------------------------------------------------------*/
#define     OPTIONBUTTONIDX_AllContours             0
#define     OPTIONBUTTONIDX_MajorContoursOnly       1

/*----------------------------------------------------------------------+
|                                                                       |
|   Annotate Countours Text Aligment Option Button Item Indexes         |
|                                                                       |
+----------------------------------------------------------------------*/
#define     OPTIONBUTTONIDX_UpSlope                 0
#define     OPTIONBUTTONIDX_FollowLine              1

/*----------------------------------------------------------------------+
|                                                                       |
|   Annotate Mode Option Button Item Indexes                            |
|                                                                       |
+----------------------------------------------------------------------*/
#define     OPTIONBUTTONIDX_AllContours             0
#define     OPTIONBUTTONIDX_MajorContoursOnly       1

enum    TMItemLists
    {
    ItemList_LabelContours = 1000,
    ItemList_LabelSpot,
    };

#define HOOKITEMID_CommonOverriddenDistanceProp 227