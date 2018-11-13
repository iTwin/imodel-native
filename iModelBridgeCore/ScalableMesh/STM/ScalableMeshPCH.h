/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshPCH.h $
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#if _WIN32
#include <amp.h> 
#include <amp_math.h>
#endif

#ifdef __APPLE__
	#define _VA_LIST
#endif	
#include <thread>
#include <atomic>
#include <mutex> 
#if _WIN32
#include <excpt.h> 
#endif
#include <map>

#ifdef __APPLE__
    #define _RWSTD_NO_IMPLICIT_INCLUSION
	#define TARGET_CPU_ARM64 1
#endif	
#include <BeSQLite/BeSQLite.h>
#include <ScalableMesh/Foundations/Definitions.h>
#include <ScalableMesh/GeoCoords/Definitions.h>
#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/ScalableMeshDefs.h>

#if _WIN32
#ifndef VANCOUVER_API    
    #include <STMInternal/Foundations/FoundationsPrivateTools.h>
#endif 
#endif

#include <Bentley/Bentley.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/bcdtminlines.h>
#ifndef LINUX_SCALABLEMESH_BUILD
#include <TerrainModel\Formats\InroadsImporter.h>
#endif

USING_NAMESPACE_BENTLEY_TERRAINMODEL


#include <Bentley/BeStringUtilities.h>
#include <DgnPlatform/DgnPlatform.h>
#include <Geom/GeomApi.h>
#include <Mtg/MtgStructs.h>
#include <GeoCoord/BaseGeoCoord.h>



#ifdef VANCOUVER_API
#undef static_assert
#include <DgnGeoCoord/DgnGeoCoord.h>
#include <DgnPlatform\DgnAttachment.h>
#include <DgnPlatform\DgnPlatform.h>
#include <DgnPlatform\DgnFileIO\ModelInfo.h>
#include <DgnPlatform\DgnFileIO\UnitDefinition.h>
#include <DgnPlatform\ElementHandlerManager.h>
#include <DgnPlatform\ScanCriteria.h>
#endif

#ifndef VANCOUVER_API
//#include <DgnPlatform\ImageUtilities.h>
#endif


#include <DgnPlatform/DgnPlatform.r.h>


#ifdef VANCOUVER_API
#include <DgnPlatform\ElementHandle.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <RmgrTools\Tools\DataExternalizer.h>

#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <DgnPlatform\DelegatedElementECEnabler.h>

#include <DgnPlatform\ExtendedElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>
#endif

#ifdef VANCOUVER_API
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#else
USING_NAMESPACE_BENTLEY_DGN
#endif

#ifdef VANCOUVER_API
#include <DgnPlatform\PointCloudHandler.h>

#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudChannel.h>
#include <PointCloud\PointCloudFileEdit.h>
#include <DgnPlatform\PointCloudClipHandler.h>    
USING_NAMESPACE_BENTLEY_POINTCLOUD
#include <PointCloud\PointCloudDataQuery.h>
#endif


/*----------------------------------------------------------------------+
| Include TerrainModel general header files                             |
+----------------------------------------------------------------------*/

#include <ScalableMesh/Foundations/Definitions.h>
#include <ScalableMesh/GeoCoords/Definitions.h>
#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#if _WIN32
#ifndef VANCOUVER_API    
#include <STMInternal/Foundations/FoundationsPrivateTools.h>
#endif 
#endif

#include <Logging/bentleylogging.h>


//Useful for detecting memory leak
//#define _DEBUG
//#include <C:\Program Files (x86)\Visual Leak Detector\include\vld.h>

#ifndef BEGIN_UNNAMED_NAMESPACE
#define BEGIN_UNNAMED_NAMESPACE namespace {
#define END_UNNAMED_NAMESPACE }
#endif //!BEGIN_UNNAMED_NAMESPACE

//TM API changes
//#ifdef VANCOUVER_API
#define GET_POINT_AT_INDEX(drapedLineP, pt, dist, code, sample)\
                    drapedLineP->GetPointByIndex(pt, dist, code, sample)
//#else
//#define GET_POINT_AT_INDEX(drapedLineP, pt, dist, code, sample)\
//                        drapedLineP->GetPointByIndex(&pt, dist, code, sample)
//#endif

#define scmInterface struct __declspec(novtable)
