/*----------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/transkit/commandsPal.r $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $	
|
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|
|   Include Files
|
+----------------------------------------------------------------------*/
#include <Mstn\MdlApi\dlogbox.r.h>
#include <Mstn\MdlApi\dlogids.r.h>
#include <Mstn\MdlApi\helpids.r.h>
#include <Mstn\MdlApi\cmdclass.r.h>
#include <DgnPlatform\Tcb\toolset.r.h>
#include <DgnPlatform\DimensionStyleProps.r.h>
#include <Mstn\Ids\dimapps.ids>
#include <TransKitGenSrc\PowerPlatform.FeatureAspects.h>

#define OPTIONBUTTONID_GenericIntegerProp       1

#include <commandsDefs.h>

#define APPLICATIONID_TERRAINMODEL              (200)
#define TEXTID_GenericDistanceProp 4
#define GENERICID_GenericLock 2

#define HOOKITEMID_CommonDistanceProp           205
#define HOOKITEMID_CommonLock                   219

#define OPTIONBUTTONID_DimAlignmentMode         1
#define OPTIONBUTTONID_DimLocation              2

#define XCMDLISTID_TerrainModelCommands         1

enum   TERRAINMODELICONCMDIDS     // Icon Command IDs
    {
    ICONCMDID_TerrainModelImportLandXML = APPLICATIONID_TERRAINMODEL,
    ICONCMDID_TerrainModelLabelContours,
    ICONCMDID_TerrainModelLabelSpots,
    };

enum TERRAINMODELIDS
    {
    ICONPOPUPID_TERRAINMODELs,
    };


#include "commandstxt.h"
#include "commandscmd.h"	

#define COMBOBOXID_ChangeTextStyleName          1

/*----------------------------------------------------------------------+
|									|
|   Local Defines       						|
|									|
+----------------------------------------------------------------------*/

#define XOB  19*XC
#define OBW  12*XC

CmdItemListRsc ItemList_LabelSpot =
    {{
    BEGIN_VSTACK_LAYOUT(VSTACKLAYOUTID_ToolSettingsMain,"")

        BEGIN_GRID_LAYOUT(GRIDLAYOUTID_ToolSettings3Cols, "")
            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},     Label,          LABELID_TerrainModel,   ON,  ALIGN_RIGHT, ANNOTTXT_SelectedDTM, "" },
                {{AUTO_XY, 19*XC, 0},     Label,          LABELID_SelectedTerrainModel, ON,  ALIGN_LEFT, "<Model>", "" },
            END_ROW

            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   ComboBox,       COMBOBOXID_ChangeTextStyleName, ON, 0, TXT3_TextSettingsTextStyleLabel, "owner=\"Modify\""},
                {AUTO_XYWH,             IconCmdX,       ICONID_TextStyleBrowse,     ON, 0, "", "owner=\"Modify\""},
            END_ROW


            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   ComboBox,       COMBOBOXID_DimStyle, ON, 0, TXT_ComboDimStyle, "owner=\"dimCreate\""},
                BEGIN_HSTACK_LAYOUT(HSTACKLAYOUTID_LabeledItem, "")
                    {AUTO_XYWH,           IconCmdX,       ICONCMDID_DimSettings,    ON, 0, "", "owner=\"dimCreate\""},
                    {AUTO_XYWH,           IconCmdX,       ICONCMDID_DimStyleRestore,ON, 1, "", "owner=\"dimCreate\""},        
                END_LAYOUT
            END_ROW
            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   OptionButton,   OPTIONBUTTONID_DimNoteTextRotation, ON, DIMSTYLE_PROP_MLNote_TextRotation_INTEGER, "", "owner=\"TextEditor\""},
            END_ROW
            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   OptionButton,   OPTIONBUTTONID_DimTextPlacementNoSemi, ON, DIMSTYLE_PROP_Placement_TextPosition_INTEGER, "", "owner=\"TextEditor\""},
//        {{0,        D_ROW (6.00),   14*XC,  0}, OptionButton,   OPTIONBUTTONID_WPDimStartAt, ON, 0, "", "owner=\"TextEditor\""},
            END_ROW
            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   OptionButton,   OPTIONBUTTONID_DimNoteHorAttachment, ON, DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER, "", "owner=\"TextEditor\""},
            END_ROW
            GRID_ROW(0, "")
                {AUTO_XYWH,  ToggleIcon,     TOGGLEICONID_UseAnnotationScale,    ON, 0,  "",                         "owner=\"TextEditor\",column='1'"  },
            END_ROW
        END_LAYOUT
    END_LAYOUT
    }};

CmdItemListInformationRsc ItemList_LabelSpot = 
    { 
    "TERRAINMODEL LABEL SPOTS", "TerrainModel Label Spots", "", __BFILE__, __LINE__, 0, CMDITEMLISTINFO_LayoutManager | CMDITEMLISTINFO_AspectRatioNotNeeded 
    };

/// <summary>
/// </summary>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
CmdItemListRsc ItemList_LabelContours =
    {{
    BEGIN_VSTACK_LAYOUT(VSTACKLAYOUTID_ToolSettingsMain,"")

        BEGIN_GRID_LAYOUT(GRIDLAYOUTID_ToolSettings3Cols, "")
            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},     Label,          LABELID_TerrainModel,   ON,  ALIGN_RIGHT, ANNOTTXT_SelectedDTM, "" },
                {{AUTO_XY, 19*XC, 0},     Label,          LABELID_SelectedTerrainModel, ON,  ALIGN_LEFT, "<Model>", "" },
            END_ROW

            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   OptionButton,    OPTIONBUTTONID_AnnotationCountoursMode, ON, 0, "", ""},
            END_ROW

            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0},   OptionButton,    OPTIONBUTTONID_AnnotationCountoursTxtAligment, ON, 0, "", ""},
            END_ROW

            GRID_ROW(0, "")
                {{AUTO_XY, 16*XC, 0},   Label,          0, ON, LABEL_LINKNEXT|ALIGN_RIGHT, "", ""},
                {{AUTO_XY, 19*XC, 0}, ComboBox,       COMBOBOXID_DimStyle, ON, 0, TXT_ComboDimStyle, "owner=\"dimCreate\""},
                BEGIN_HSTACK_LAYOUT(HSTACKLAYOUTID_LabeledItem, "")
                    {AUTO_XYWH,           IconCmdX,       ICONCMDID_DimSettings,    ON, 0, "", "owner=\"dimCreate\""},
                    {AUTO_XYWH,           IconCmdX,       ICONCMDID_DimStyleRestore,ON, 1, "", "owner=\"dimCreate\""},        
                END_LAYOUT
            END_ROW

            GRID_ROW(0, "")
                {AUTO_XYWH,  ToggleIcon,     TOGGLEICONID_UseAnnotationScale,    ON, 0,  "",                         "owner=\"TextEditor\",column='1'"  },
            END_ROW
        END_LAYOUT
    }
    };

CmdItemListInformationRsc ItemList_LabelContours = 
    { 
    "TERRAINMODEL LABEL CONTOURS", "TerrainModel Label Contours", "", __BFILE__, __LINE__, 0, CMDITEMLISTINFO_LayoutManager | CMDITEMLISTINFO_AspectRatioNotNeeded 
    };

/// <summary>
/// </summary>
/// <author>Piotr.Slowinski</author>                          <date>3/2011</date>
DItem_OptionButtonRsc OPTIONBUTTONID_AnnotationCountoursMode =
    {
    NOSYNONYM, NOHELP, LHELPCMD, /*HOOKITEMID_???*/ NOHOOK,
    NOARG | OPTNBTNATTR_NEWSTYLE, ANNOTTXT_AnnotationCountoursMode,
    "dtmcommandsInfoP->annotateContoursMode",
        {
        {NOTYPE, NOICON, NOCMD, MCMD, OPTIONBUTTONIDX_AllContours,       NOMASK, ON, ANNOTTXT_AllContours},
        {NOTYPE, NOICON, NOCMD, MCMD, OPTIONBUTTONIDX_MajorContoursOnly, NOMASK, ON, ANNOTTXT_MajorContoursOnly},
        }         
    };

/// <summary>
/// </summary>
/// <author>Piotr.Slowinski</author>                            <date>3/2011</date>
DItem_OptionButtonRsc OPTIONBUTTONID_AnnotationCountoursTxtAligment =
    {
    NOSYNONYM, 
    NOHELP, LHELPCMD, 
    /*HOOKITEMID_???*/ NOHOOK, NOARG | OPTNBTNATTR_NEWSTYLE,
    ANNOTTXT_AnnotationCountoursTxtAligment,
    "dtmcommandsInfoP->annotateContoursTxtAlignment",
        {
        {NOTYPE, NOICON, NOCMD, MCMD, OPTIONBUTTONIDX_UpSlope,    NOMASK, ON, ANNOTTXT_UpSlope},
        {NOTYPE, NOICON, NOCMD, MCMD, OPTIONBUTTONIDX_FollowLine, NOMASK, ON, ANNOTTXT_FollowLine},
        }         
    };


DItem_IconCmdRsc        ICONCMDID_TerrainModelImportLandXML =
    {
    NOHELP, OHELPTASKIDCMD, 0,
    CMD_TERRAINMODEL_IMPORT_LANDXML, OTASKID, "", ""
    }
    extendedAttributes
    {
    {
    {EXTATTR_FLYTEXT, TXT_Flyover_ImportLandXML},
    {EXTATTR_BALLOON, TXT_Balloon_ImportLandXML},
    }
    };

IconCmdSmallRsc ICONCMDID_TerrainModelImportLandXML =
    {
    24, 24, ICONFORMAT_WINDOWS, BLACK_INDEX, "Meas Area El", "TMImport"
    };

IconCmdLargeRsc ICONCMDID_TerrainModelImportLandXML = 
    {
    32, 32, ICONFORMAT_WINDOWS, BLACK_INDEX, "Meas Area El", "TMImport"
    };
    
DItem_IconCmdRsc        ICONCMDID_TerrainModelLabelContours =
    {
    NOHELP, OHELPTASKIDCMD, 0,
    CMD_TERRAINMODEL_LABEL_CONTOURS, OTASKID, "", ""
    }
    extendedAttributes
    {
    {
    {EXTATTR_FLYTEXT, TXT_Flyover_LabelContours},
    {EXTATTR_BALLOON, TXT_Balloon_LabelContours},
    }
    };

IconCmdSmallRsc ICONCMDID_TerrainModelLabelContours =
    {
    24, 24, ICONFORMAT_WINDOWS, BLACK_INDEX, "Label Contours", "TMAnnotateContours"
    };

IconCmdLargeRsc ICONCMDID_TerrainModelLabelContours = 
    {
    32, 32, ICONFORMAT_WINDOWS, BLACK_INDEX, "Label Contours", "TMAnnotateContours"
    };
    
DItem_IconCmdRsc        ICONCMDID_TerrainModelLabelSpots =
    {
    NOHELP, OHELPTASKIDCMD, 0,
    CMD_TERRAINMODEL_LABEL_SPOTS, OTASKID, "", """"
    }
    extendedAttributes
    {
    {
    {EXTATTR_FLYTEXT, TXT_Flyover_LabelSpots},
    {EXTATTR_BALLOON, TXT_Balloon_LabelSpots},
    }
    };

IconCmdSmallRsc ICONCMDID_TerrainModelLabelSpots =
    {
    24, 24, ICONFORMAT_WINDOWS, BLACK_INDEX, "Label Spots", "TMAnnotateSpots"
    };

IconCmdLargeRsc ICONCMDID_TerrainModelLabelSpots = 
    {
    32, 32, ICONFORMAT_WINDOWS, BLACK_INDEX, "Label Spots", "TMAnnotateSpots"
    };

DItem_GenericRsc GENERICID_GenericLock =
    {
    NOHELP, MHELPTOPIC,
    HOOKITEMID_CommonLock, NOARG
    };

DItem_TextRsc TEXTID_GenericDistanceProp =
    {
    NOCMD, LCMD, NOSYNONYM, NOHELP, LHELP, 
    HOOKITEMID_CommonOverriddenDistanceProp, NOARG,
    64, "%w", "%w", "", "", NOMASK, TEXT_NOCONCAT,
    "", ""
    }
extendedAttributes
    {
        {
            {EXTATTR_ITEMNAME, "GenericOverriddenDistanceProp" }
        }
    };


XCommandListRsc         XCMDLISTID_TerrainModelCommands = 
{
    {
    { "TerrainModel.Import.LandXML",
        { CMD_TERRAINMODEL_IMPORT_LANDXML, "TERRAINMODELCOMMANDS", "" },
        "TMImport",
        TXT_Balloon_ImportLandXML,
        TXT_Flyover_ImportLandXML,
        "53df5da6-aafe-11e4-b0ff-6894231a2840", "", "[Session]Session.TreatActiveModelAs3D()", "", "", "SystemEvent.ActiveModelChanged", NOACCEL,
        PP_ID_FeatureAspects_TerrainModel_Import },              // @fadoc  TerrainModel.ImportLandXML XCommand
    {"TerrainModel.Label.Contours",
        {CMD_TERRAINMODEL_LABEL_CONTOURS, "TERRAINMODELCOMMANDS" , ""},
        "TMAnnotateContours",
        TXT_Balloon_LabelContours,
        TXT_Flyover_LabelContours,
        "53dfaba3-aafe-11e4-b83c-6894231a2840", "", "", "", "", "", NOACCEL,
        PP_ID_FeatureAspects_TerrainModel_Annotate_Contours},   // @fadoc  TerrainModel.Label.Contours XCommand
    {"TerrainModel.Label.Spots",
        {CMD_TERRAINMODEL_LABEL_SPOTS, "TERRAINMODELCOMMANDS" , ""},
        "TMAnnotateSpots",
        TXT_Balloon_LabelSpots,
        TXT_Flyover_LabelSpots,
        "53dfabaa-aafe-11e4-a4ad-6894231a2840", "", "", "", "", "", NOACCEL,
        PP_ID_FeatureAspects_TerrainModel_Annotate_Spots},      // @fadoc  TerrainModel.Label.Spots XCommand
    }
};

