/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

#define QUANTITYTAKEOFFSASPECTS_NAMESPACE_NAME  QuantityTakeoffsAspects
#define BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace QUANTITYTAKEOFFSASPECTS_NAMESPACE_NAME {
#define END_QUANTITYTAKEOFFSASPECTS_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_QUANTITYTAKEOFFSASPECTS using namespace BENTLEY_NAMESPACE_NAME::QUANTITYTAKEOFFSASPECTS_NAMESPACE_NAME;

#define QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME                         "QuantityTakeoffsAspects"

#define QUANTITYTAKEOFFSASPECTS_SCHEMA(className)                   QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME "." className

#if defined (__QUANTITYTAKEOFFSASPECTS_BUILD__)
#define QUANTITYTAKEOFFSASPECTS_EXPORT EXPORT_ATTRIBUTE
#else
#define QUANTITYTAKEOFFSASPECTS_EXPORT IMPORT_ATTRIBUTE
#endif

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_QUANTITYTAKEOFFSASPECTS_NAMESPACE


#define QUANTITYTAKEOFFSASPECTS_CLASS_DimensionsAspect              "DimensionsAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_EnergyPerformanceAspect       "EnergyPerformanceAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_MaterialAspect                "MaterialAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_PerimeterAspect               "PerimeterAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_PileAspect                    "PileAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_PipeAspect                    "PipeAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SideAreasAspect               "SideAreasAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect                    "SlabAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SlopeAspect                   "SlopeAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_StairsAspect                  "StairsAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect  "StructuralLinearMemberAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect             "SurfaceAreaAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_ThicknessAspect               "ThicknessAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_VolumeAspect                  "VolumeAspect"


#define QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Height             "Height"
#define QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Length             "Length"
#define QUANTITYTAKEOFFSASPECTS_DIMENSIONSASPECT_Width              "Width"

#define QUANTITYTAKEOFFSASPECTS_ENERGYPERFORMANCEASPECT_Rating      "Rating"

#define QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Material             "Material"
#define QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_MaterialDensity      "MaterialDensity"
#define QUANTITYTAKEOFFSASPECTS_MATERIALASPECT_Weight               "Weight"

#define QUANTITYTAKEOFFSASPECTS_PERIMETERASPECT_Perimeter           "Perimeter"

#define QUANTITYTAKEOFFSASPECTS_PILEASPECT_EmbedmentDepth           "EmbedmentDepth"

#define QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Diameter                 "Diameter"
#define QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Length                   "Length"
#define QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Schedule                 "Schedule"
#define QUANTITYTAKEOFFSASPECTS_PIPEASPECT_Thickness                "Thickness"

#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomGrossArea     "BottomGrossArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_BottomNetArea       "BottomNetArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideGrossArea   "LeftSideGrossArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_LeftSideNetArea     "LeftSideNetArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideGrossArea  "RightSideGrossArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_RightSideNetArea    "RightSideNetArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopGrossArea        "TopGrossArea"
#define QUANTITYTAKEOFFSASPECTS_SIDEAREASASPECT_TopNetArea          "TopNetArea"

#define QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection            "SlabDirection"
#define QUANTITYTAKEOFFSASPECTS_SLABASPECT_Type                     "Type"

#define QUANTITYTAKEOFFSASPECTS_SLOPEASPECT_Slope                   "Slope"

#define QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_NumberOfRisers         "NumberOfRisers"
#define QUANTITYTAKEOFFSASPECTS_STAIRSASPECT_RiserHeight            "RiserHeight"

#define QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_CrossSectionalArea   "CrossSectionalArea"
#define QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_SectionName          "SectionName"
#define QUANTITYTAKEOFFSASPECTS_STRUCTURALLINEARMEMBERASPECT_Type                 "Type"

#define QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_GrossSurfaceArea  "GrossSurfaceArea"
#define QUANTITYTAKEOFFSASPECTS_SURFACEAREAASPECT_NetSurfaceArea    "NetSurfaceArea"

#define QUANTITYTAKEOFFSASPECTS_THICKNESSASPECT_Thickness           "Thickness"

#define QUANTITYTAKEOFFSASPECTS_VOLUMEASPECT_GrossVolume            "GrossVolume"
#define QUANTITYTAKEOFFSASPECTS_VOLUMEASPECT_NetVolume              "NetVolume"
