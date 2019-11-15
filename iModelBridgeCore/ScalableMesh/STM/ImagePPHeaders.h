/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../mki/StaticAnalysisWarningsPush.h"
#include <ImagePP/h/ExportMacros.h>
#include <ImagePP/all/h/HFCAccessMode.h>
#include <ImagePP/h/ImageppAPI.h>

#ifdef VANCOUVER_API
    #include <ImagePP/h/hstdcpp.h>
#endif

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
#include <ImagePP/all/h/HGF2DSegment.h>
#include <ImagePP/all/h/HVE2DSegment.h>
#include <ImagePP/all/h/HVE2DVoidShape.h>
#include <ImagePP/all/h/HVE3DPolyLine.h>
//#include <ImagePP/all/h/HVEDTMLinearFeature.h>
#include <ImagePP/all/h/HVE2DComplexShape.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HCDPacket.h>

#include "SMSQLiteFile.h"

#include <ImagePP/all/h/HGF2DTemplateExtent.h>
#include <ImagePP/all/h/HGF3DCoord.h>
#include <ImagePP/all/h/HGF2DExtent.h>

#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HCPGCoordModel.h>

#ifdef VANCOUVER_API
#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ImagePP/all/h/IDTMFeatureArray.h>
#include <ImagePP/all/h/HPUArray.h>
#endif

#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFPngFile.h>
#include <ImagePP/all/h/HVEClipShape.h>
#include <ImagePP/all/h/HUTDEMRasterXYZPointsExtractor.h>

/*#ifdef VANCOUVER_API
#include <RasterCore/DgnRaster.h>
#include <RasterCore/RasterDEMFilters.h>
#include <RasterCore/msrastercore.h>
#endif*/
#include "../mki/StaticAnalysisWarningsPop.h"

#ifndef VANCOUVER_API
#define HVE2DSHAPE ImagePP::HVE2DShape
#define HVESHAPE ImagePP::HVEShape
#define HFCPTR ImagePP::HFCPtr
#define HGF2DCOORDSYS ImagePP::HGF2DCoordSys
#define HGF3DCOORD ImagePP::HGF3DCoord
#else
#define HFCPTR HFCPtr
#define HVE2DSHAPE HVE2DShape
#define HGF2DCOORDSYS HGF2DCoordSys
#define HVESHAPE HVEShape
#define HGF3DCOORD HGF3DCoord
#endif