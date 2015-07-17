/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/transkit/commandsMsg.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Mstn\MdlApi\rscdefs.r.h>
#include "commandsDefs.h"

/*----------------------------------------------------------------------+
|									|
|    Command Names							|
|									|
+----------------------------------------------------------------------*/
MessageList STRINGID_Message_Main =
{
    {
    /* Command names */
    { CMDNAME_LabelSpot,                           "Label Terrain Spots" },
    { CMDNAME_LabelContours,                       "Label Terrain Contours" },
    { CMDNAME_ImportLandXML,                       "Import Terrain Model from LandXML" },
    { CMDNAME_ImportTerrainModel,                  "Import Terrain Model Import" },

    /* Prompts */
    { PROMPT_IdentifyDTM,                       "Identify Terrain Model" },
    { PROMPT_SelectSpot,                        "Select Spot" },
    { PROMPT_SelectFromPoint,                   "Select From Point" },
    { PROMPT_SelectToPoint,                     "Select To Point or Reset to complete" },
    { PROMPT_AcceptReject,                      "Accept/Reject"},

    /* Errors */
    { ERROR_NotADTM,                            "Not a Terrain Model" },
    { ERROR_NoSurfaceDetected,                  "No surface detected" },
    { ERROR_UnacceptableWorkMode,               "DTM Elements are not supported in DWG workmode." },
    { ERROR_UnreadableInputFile,                "Unreadable Input File" },
    { ERROR_NoContoursDrawn,                    "No contours are drawn can't annotate" },
    { ERROR_InvalidFile,                        "Invalid File" },

    /* Dialog Titles */
    { DIALOGTITLE_SelectLandXML,                "Select LandXML File"},
    { DIALOGTITLE_SelectDTMFile,                "Select DTM File"},
    { DIALOGTITLE_SelectLandXMLConfig,          "Select LandXML import Config File"},
@localize comment("#FileFilter#");            
    { DIALOGTITLE_LandXMLFileFilter,            "*.xml,LandXML [*.xml]" },
@localize comment("");

    /* Misc */
    { RESULT_ImportDTM,                            "Terrain model %ls imported successfully" },
    { RESULT_ImportFailedDTM,                      "Failed to import Terrain model %ls" },

    }
};

MessageList WMSGLIST_Main =
{
    {
    /* */
    { WSTATUS_NotSelected,                      "<Not Selected>" },
    }
};
