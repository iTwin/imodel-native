/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/ScalableTerrainModelPCH.h $
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley\BeStringUtilities.h>
#include <Geom/GeomApi.h>
#include <GeoCoord/BaseGeoCoord.h>

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
#include <ImagePP/all/h/HGFPointTileStore.h>
#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DExtent.h>
#include <ImagePP/all/h/HGFPointIndex.h>
#include <ImagePP/all/h/HGFFeatureIndex.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HPMPool.h>
#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/IDTMFeatureArray.h>
#include <ImagePP/all/h/HPUArray.h>


/*----------------------------------------------------------------------+
| Include TerrainModel general header files                             |
+----------------------------------------------------------------------*/
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <TerrainModel\Formats\InroadsImporter.h>

#include <ScalableTerrainModel/Foundations/Definitions.h>
#include <ScalableTerrainModel/GeoCoords/Definitions.h>
#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/MrDTMDefs.h>


#include <STMInternal/Foundations/FoundationsPrivateTools.h>

#ifndef BEGIN_UNNAMED_NAMESPACE
#define BEGIN_UNNAMED_NAMESPACE namespace {
#define END_UNNAMED_NAMESPACE }
#endif //!BEGIN_UNNAMED_NAMESPACE
