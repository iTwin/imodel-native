/*----------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/commandsCmd.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/

#pragma suppressREQCmds

#include <Mstn\MdlApi\rscdefs.r.h>
#include <Mstn\MdlApi\cmdclass.r.h>

#include <TransKitGenSrc\PowerPlatform.FeatureAspects.h>

#include "commandsDefs.h"

#define HID4DRAFT   NONE
#define HID4USTN    HID
#define DEF4DRAFT   NONE
#define DEF4USTN    DEF
#define DB_CMD      NONE


/*----------------------------------------------------------------------+
|									|
|   Immediate Defines							|
|									|
+----------------------------------------------------------------------*/
#define CT_NONE					0
#define	CT_MAIN 				1
#define	CT_MEASURE				2
#define CT_MEASURE_DISPLAY		3
#define CT_MEASURE_DTMAREA		4
#define CT_DTM					5
#define CT_MEASURE_DISTANCE     6
#define CT_IMPORT               7
#define CT_IMPORT_LANDXML       8
#define CT_LABEL                9

/*----------------------------------------------------------------------+
|									|
|   Main Commands							|
|									|
+----------------------------------------------------------------------*/
CommandTable	CT_MAIN =
{
    {  1, CT_DTM,               MANIPULATION,   NONE,   "TERRAINMODEL", },
}

CommandTable	CT_DTM =
{
    {  1, CT_IMPORT,  	        INHERIT,        NONE,       "IMPORT",           NoCmdNum,                   NoItemList,                 PP_ID_FeatureAspects_TerrainModel_Import    },
    {  2, CT_LABEL,  	        INHERIT,        NONE,	    "LABEL",            NoCmdNum,                   NoItemList,                 PP_ID_FeatureAspects_TerrainModel_Annotate  },
    {  3, CT_NONE,              INHERIT,        HID,        "FEATURETRACK",     CMDNAME_FeatureTrack, },
}    

CommandTable	CT_LABEL =
{
    {  1, CT_NONE,  	        INHERIT,        NONE,       "CONTOURS",         CMDNAME_LabelContours,      ItemList_LabelContours,     PP_ID_FeatureAspects_TerrainModel_Annotate_Contours },
    {  2, CT_NONE,  	        INHERIT,        NONE,       "SPOTS",            CMDNAME_LabelSpot,          ItemList_LabelSpot,         PP_ID_FeatureAspects_TerrainModel_Annotate_Spots    },
}

CommandTable	CT_IMPORT =
{
    {  1, CT_IMPORT_LANDXML,    INHERIT,        TRY,        "LANDXML",          CMDNAME_ImportLandXML,      },
    {  2, CT_NONE,              INHERIT,        HID,        "DTM",              CMDNAME_ImportTerrainModel, },
}

CommandTable CT_IMPORT_LANDXML =
{
    {  1, CT_NONE,              INHERIT,        HID,        "PREPARE_CONFIG",   },
}


/*-----------------------------------------------------------------------
Setup for native code only MDL app
-----------------------------------------------------------------------*/
#define DLLAPP_PRIMARY 1

DllMdlApp   DLLAPP_PRIMARY = {L"TerrainModelCommands", L"TerrainModelCommands"}

/*
AddInMdlApp DLLAPP_PRIMARY = { "dtmcommands", "Bentley.DTM.Commands", "CurrentDomain" }
*/
