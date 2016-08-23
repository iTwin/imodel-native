/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshPCH.h $
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <amp.h> 
#include <amp_math.h>
#include <thread>
#include <atomic>
#include <mutex> 
#include <excpt.h> 
#include <map>



#include <Bentley\BeStringUtilities.h>
#include <DgnPlatform\DgnPlatform.h>
#include <Geom/GeomApi.h>
#include <Mtg/MtgStructs.h>
#include <GeoCoord/BaseGeoCoord.h>

/*#include <DgnGeoCoord\DgnGeoCoord.h>
#include <DgnPlatform\DgnAttachment.h>
#include <DgnPlatform\DgnPlatform.h>
#include <DgnPlatform\DgnFileIO\ModelInfo.h>
#include <DgnPlatform\DgnFileIO\UnitDefinition.h>
#include <DgnPlatform\ElementHandlerManager.h>
#include <DgnPlatform\ScanCriteria.h>*/

#include <DgnPlatform\DgnPlatform.r.h>
/*#include <DgnPlatform\ElementHandle.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <RmgrTools\Tools\DataExternalizer.h>

#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <DgnPlatform\DelegatedElementECEnabler.h>

#include <DgnPlatform\ExtendedElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>*/

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*#include <DgnPlatform\PointCloudHandler.h>

#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudChannel.h>
#include <PointCloud\PointCloudFileEdit.h>
#include <DgnPlatform\PointCloudClipHandler.h>    
USING_NAMESPACE_BENTLEY_POINTCLOUD
#include <PointCloud\PointCloudDataQuery.h>*/


/*----------------------------------------------------------------------+
| Include TerrainModel general header files                             |
+----------------------------------------------------------------------*/
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <TerrainModel\Formats\InroadsImporter.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

#include <ScalableMesh/Foundations/Definitions.h>
#include <ScalableMesh/GeoCoords/Definitions.h>
#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/ScalableMeshDefs.h>
    
#include <STMInternal/Foundations/FoundationsPrivateTools.h>

//Useful for detecting memory leak
//#define _DEBUG
//#include <C:\Program Files (x86)\Visual Leak Detector\include\vld.h>

#ifndef BEGIN_UNNAMED_NAMESPACE
#define BEGIN_UNNAMED_NAMESPACE namespace {
#define END_UNNAMED_NAMESPACE }
#endif //!BEGIN_UNNAMED_NAMESPACE

//TM API changes
#ifdef VANCOUVER_API
#define GET_POINT_AT_INDEX(drapedLineP, pt, dist, code, sample)\
                    drapedLineP->GetPointByIndex(pt, dist, code, sample)
#else
#define GET_POINT_AT_INDEX(drapedLineP, pt, dist, code, sample)\
                        drapedLineP->GetPointByIndex(&pt, dist, code, sample)
#endif

#define scmInterface struct __declspec(novtable)
