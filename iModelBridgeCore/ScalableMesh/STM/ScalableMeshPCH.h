/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshPCH.h $
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <amp.h> 
#include <amp_math.h>
#include <thread>
#include <atomic>
#include <mutex> 
#include <excpt.h> 

using namespace std;

#include <Bentley/Bentley.h>
#include <Bentley\BeStringUtilities.h>
#include <DgnPlatform\DgnPlatform.h>
#include <Geom/GeomApi.h>
#include <Mtg/MtgStructs.h>
#include <GeoCoord/BaseGeoCoord.h>

#include <DgnGeoCoord\DgnGeoCoord.h>
#include <DgnPlatform\DgnAttachment.h>
#include <DgnPlatform\DgnPlatform.h>
#include <DgnPlatform\DgnFileIO\ModelInfo.h>
#include <DgnPlatform\DgnFileIO\UnitDefinition.h>
#include <DgnPlatform\ElementHandlerManager.h>
#include <DgnPlatform\ScanCriteria.h>

#include <DgnPlatform\DgnPlatform.r.h>
#include <DgnPlatform\ElementHandle.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <RmgrTools\Tools\DataExternalizer.h>

#include <DgnPlatform\IPointCloud.h>
#include <DgnPlatform\ITransactionHandler.h>
#include <DgnPlatform\XAttributeHandler.h>
#include <DgnPlatform\DelegatedElementECEnabler.h>

#include <DgnPlatform\ExtendedElementHandler.h>
#include <DgnPlatform\TerrainModel\TMElementHandler.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <DgnPlatform\PointCloudHandler.h>

#include <PointCloud\PointCloud.h>
#include <PointCloud\PointCloudChannel.h>
#include <PointCloud\PointCloudFileEdit.h>
#include <DgnPlatform\PointCloudClipHandler.h>    
USING_NAMESPACE_BENTLEY_POINTCLOUD
#include <PointCloud\PointCloudDataQuery.h>

/*----------------------------------------------------------------------+
| Include ImagePP headers
+----------------------------------------------------------------------*/
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/h/HStlStuff.h>
#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>
#include <ImagePP/h/HNumeric.h>


#include <ImagePP/all/h/HVEShape.h>
#include <ImagePP/all/h/HVE2DPolygonOfSegments.h>
#include <ImagePP/all/h/HVE2DShape.h>
#include <ImagePP/all/h/HVE2DRectangle.h>
#include <ImagePP/all/h/HVE2DHoledShape.h>
#include <ImagePP/all/h/HVE2DComplexShape.h>
#include <ImagePP/all/h/HVE2DPolySegment.h>
#include <ImagePP/all/h/HVE2DPolygonOfSegments.h>
#include <ImagePP/all/h/HVE2DSegment.h>
#include <ImagePP/all/h/HVE2DVoidShape.h>
#include <ImagePP/all/h/HVE3DPolyLine.h>
#include <ImagePP/all/h/HVEDTMLinearFeature.h>
#include <ImagePP/all/h/HVE2DComplexShape.h>
#include <ImagePP/all/h/HFCException.h>

#ifdef SCALABLE_MESH_DGN
#include <ImagePP/all/h/DgnTileStore.h>
#endif
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DExtent.h>

#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HPMPool.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/IDTMFeatureArray.h>
#include <ImagePP/all/h/HPUArray.h>

#include <ImagePP\all\h\HFCURLFile.h>
#include <ImagePP\all\h\HRFRasterFileFactory.h>
#include <ImagePP\all\h\HVEClipShape.h>
#include <ImagePP\all\h\HUTDEMRasterXYZPointsExtractor.h>

#include <RasterCore\DgnRaster.h>
#include <RasterCore\RasterDEMFilters.h>
#include <RasterCore\msrastercore.h>
#include "SMPointTileStore.h"
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

#ifndef BEGIN_UNNAMED_NAMESPACE
#define BEGIN_UNNAMED_NAMESPACE namespace {
#define END_UNNAMED_NAMESPACE }
#endif //!BEGIN_UNNAMED_NAMESPACE


#define scmInterface struct __declspec(novtable)
