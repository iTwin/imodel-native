/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/app/handlerApp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Mstn\MdlApi\mdl.h>
#include <Mstn\basetype.h>
   
#include <Mstn\MdlApi\msrmgr.h>
#include <Mstn\MdlApi\ditemlib.fdf>
#include <Mstn\MdlApi\msoutput.fdf>
#include <Mstn\MdlApi\msparse.fdf>
#include <Mstn\MdlApi\msritem.fdf>
#include <Mstn\MdlApi\msstate.fdf>
#include <Mstn\MdlApi\mssystem.fdf>

#include <RmgrTools\Tools\RscFileManager.h>

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/idtm.h>
#include <TerrainModel/Core/bcdtmclass.h>
#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void DTMRegisterDisplayHandlers();

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

void registerManagedElementHandler();
void registerSelectionListener ();

#include "handlerApp.h"
#include "handlerAppDefs.h"
#include    <Mstn\MdlApi\mstagdat.fdf> 
//
// ToDo copied from internal
enum SharedCellSearch
    {
    SCFLAG_SearchDestModel     = (1<<0),    //*< Allowed to search preferred model
    SCFLAG_SearchReferences    = (1<<1),    //*< Allowed to search reference files
    SCFLAG_SearchLibraries     = (1<<2),    //*< Allowed to search attached libs and MS_CELLLIST for non-shared cells to turn into shared cells
    SCFLAG_SearchActiveLibary  = (1<<3),    //*< Allowed to search attached library for shared cells
    SCFLAG_SearchActiveModel   = (1<<4),    //*< Allowed to search active model
    SCFLAG_SearchActiveRefs    = (1<<5),    //*< Allowed to search references of active model
    };

#define SCFLAG_TrueScale            (1<<8)    /* => Scale the cell to match */
#define SCFLAG_NoApplyPointCellSymb (1<<9)

#define SCFLAG_DefaultSCFlags       (SCFLAG_TrueScale|SCFLAG_SearchDestModel|SCFLAG_SearchReferences|SCFLAG_SearchActiveLibary|SCFLAG_SearchLibraries)
USING_NAMESPACE_BENTLEY_DGNPLATFORM

  
BEGIN_EXTERN_C
MSCORE_EXPORT StatusInt        mdlSharedCellDef_getOrCreate
(
MSElementDescr      **scDefEdPP,            /* <= shared cell definition element; can be NULL to just ensure existance */
const DgnModelRefP  destModelRef,           /* => modelRef to use for styles, etc */
const uint32_t        flags,
PFAddTagPreWrite    preWriteFunc,           /* => Used to modify tag values before writing */
WCharCP             cellName               /* => name of cell */
);
END_EXTERN_C

USING_NAMESPACE_BENTLEY_MSTNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL

USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT;

struct DTMElementPowerPlatformExtension : public DTMElementHandlerManager::IDTMElementPowerPlatformExtension
    {
    virtual DgnPlatform::DgnTextStylePtr GetActiveStyle (DgnFileR file) override
        {
        return DgnTextStyle::GetActiveStyle (file);
        }
    virtual void EnsureSharedCellDefinitionExists (WCharCP cellName, DgnModelRefP modelRef)
        {
        mdlSharedCellDef_getOrCreate (nullptr, modelRef, SCFLAG_DefaultSCFlags, NULL, cellName);
        }
    };

static DTMElementPowerPlatformExtension s_DTMElementPowerPlatformExtension;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" __declspec(dllexport) void MdlMain (int , WCharCP  argv[])
    {
    RscFileHandle   rscFileH;

    mdlResource_openFile (&rscFileH, NULL, 0);

//    mdlState_registerStringIds (STRINGLISTID_Commands, STRINGLISTID_Prompts);

    mdlSystem_setMdlAppClass (NULL, MdlApplicationClass::MSRequired); // Don't allow handler app to unload!

    BENTLEY_NAMESPACE_NAME::TerrainModel::Element::DTMElementHandlerManager::Initialize (&s_DTMElementPowerPlatformExtension);
    registerManagedElementHandler();
    registerSelectionListener ();
    }
