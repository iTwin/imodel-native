/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/Commands/Stdafx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#pragma unmanaged
#define _USE_MATH_DEFINES
#include <math.h>
#include <list>

#include <DgnPlatform\DgnPlatformAPI.h>
#include <DgnPlatform\Note.h>
#include <DgnView\DgnViewAPI.h>
#include <DgnView\IViewManager.h>

#include <TerrainModel\ElementHandler\TerrainModelElementHandler.h>
#include <TerrainModel\Core\bcDTMClass.h>
#include <TerrainModel\Core\IDTM.h>
#include <TerrainModel\Core\bcDTMElement.h>

//#include <Mstn\MstnPlatformAPI.h>
#include <Mstn\MdlApi\mdl.h>
#include <Mstn\basetype.h>
#include <Mstn\MdlApi\workmode.fdf>
#include <Mstn\MdlApi\dloadlib.h>
#include <Mstn\MdlApi\msrmgr.h>
#include <Mstn\MdlApi\ditemlib.fdf>
#include <Mstn\MdlApi\msoutput.fdf>
#include <Mstn\MdlApi\msparse.fdf>
#include <Mstn\MdlApi\msritem.fdf>
#include <Mstn\MdlApi\msstate.fdf>
#include <Mstn\MdlApi\mssystem.fdf>
#include <Mstn\MdlApi\filelist.h>
#include <Mstn\MdlApi\dlogids.r.h>
#include <Mstn\MdlApi\msdialog.fdf>
#include <Mstn\MdlApi\msinput.fdf>
#include <Mstn\XDataTree\MstnElementTemplate.h>
#include <Mstn\MdlApi\keys.r.h>
#include <Mstn\MdlApi\msview.fdf>
#include <Mstn\MdlApi\msdisplaypath.h>
#include <Mstn\MdlApi\msmodel.fdf>
#include <Mstn\MdlApi\msrmatrx.fdf>
#include <Mstn\MdlApi\mstmatrx.fdf>
#include <Mstn\MdlApi\mstextstyle.fdf>
#include <Mstn\MdlApi\msdgnmodelref.fdf>
#include <Mstn\MdlApi\msreffil.fdf>
#include <Mstn\MdlApi\mselmdsc.fdf>
#include <Mstn\MdlApi\mscexpr.fdf>
#include <Mstn\MdlApi\msdim.fdf>
#include <Mstn\MdlApi\msdimstyle.h>
#include <Mstn\MdlApi\msstring.fdf>
#include <Mstn\MdlApi\msassoc.fdf>
#include <Mstn\MdlApi\msvec.fdf>
#include <Mstn\MdlApi\mselemen.fdf>
#include <Mstn\MdlApi\msmisc.fdf>
#include <Mstn\MdlApi\mscurrtr.fdf>
#include <Mstn\MdlApi\leveltable.fdf>
#include <Mstn\ISessionMgr.h>
#include <Mstn\ElementPropertyUtils.h>
#include <Mstn\MstnResourceUtils.h>
#include <Mstn\MdlApi\mslocate.fdf>
#include <TerrainModel\Formats\Formats.h>
#include <TerrainModel\Formats\LandXMLImporter.h>
#include <TerrainModel\ElementHandler\DTMElementHandlerManager.h>

#include <DgnPlatform\TerrainModel\TMElementSubHandler.h>
#include <TerrainModel\ElementHandler\DTMDataRef.h>

#include "refunitsconverter.h"
#include <TerrainModel\ElementHandler\TMElementDisplayHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_TERRAINMODEL_ELEMENT

#include <commandsCmd.h>
#include "commandsDefs.h"

#define ADDIN_NAME L"TMCOMMANDS"
#define ADDIN_ASSEMBLYNAME L"TerrainModelCommands"

#if !defined (BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE)
#define BEGIN_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE namespace Bentley { namespace TerrainModel { namespace Commands {
#define END_BENTLEY_TERRAINMODEL_COMMANDS_NAMESPACE }}}
#define USING_NAMESPACE_BENTLEY_TERRAINMODEL_COMMANDS using namespace BENTLEY_NAMESPACE_NAME::TerrainModel::Commands;
#endif

#pragma managed

#include "Bentley.DTM.Commands.h"

