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


#define QUANTITYTAKEOFFSASPECTS_CLASS_PerimeterAspect               "PerimeterAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_PileAspect                    "PileAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect                    "SlabAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_SlopeAspect                   "SlopeAspect"
#define QUANTITYTAKEOFFSASPECTS_CLASS_ThicknessAspect               "ThicknessAspect"

#define QUANTITYTAKEOFFSASPECTS_PERIMETERASPECT_Perimeter           "Perimeter"
#define QUANTITYTAKEOFFSASPECTS_PILEASPECT_EmbedmentDepth           "EmbedmentDepth"
#define QUANTITYTAKEOFFSASPECTS_SLABASPECT_SlabDirection            "SlabDirection"
#define QUANTITYTAKEOFFSASPECTS_SLOPEASPECT_Slope                   "Slope"
#define QUANTITYTAKEOFFSASPECTS_THICKNESSASPECT_Thickness           "Thickness"
