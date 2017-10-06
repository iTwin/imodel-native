/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/AecUnits/AecUnitsDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


#ifdef __AEC_UNITS_DOMAIN_BUILD__
#define AEC_UNITS_EXPORT EXPORT_ATTRIBUTE
#else
#define AEC_UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::AecModelingUnits %AecUnits data types */
#define BEGIN_BENTLEY_AEC_UNITS_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace AecUnits {
#define END_BENTLEY_AEC_UNITS_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_AEC_UNITS        using namespace BentleyApi::AecUnits;

#define AEC_UNITS_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_AEC_UNITS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_AEC_UNITS_NAMESPACE

#define AEC_UNITS_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_AEC_UNITS_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_AEC_UNITS_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_AEC_UNITS_SCHEMA_NAME                        "AecUnits"
#define BENTLEY_AEC_UNITS_SCHEMA_PATH                        L"ECSchemas/Domain/AecUnits.ecschema.xml"
#define BENTLEY_AEC_UNITS_SCHEMA(className)                  BENTLEY_AEC_UNITS_SCHEMA_NAME "." className
#define BENTLEY_AEC_UNITS_SCHEMA_CODE                        BENTLEY_AEC_UNITS_SCHEMA_NAME 
#define BENTLEY_AEC_UNITS_AUTHORITY                          BENTLEY_AEC_UNITS_SCHEMA_NAME

#define AEC_KOQ_ANGLE                         "ANGLE"
#define AEC_KOQ_AREA                          "AREA"
#define AEC_KOQ_AREA_SMALL                    "AREA_SMALL"
#define AEC_KOQ_AREA_LARGE                    "AREA_LARGE"
#define AEC_KOQ_LENGTH                        "LENGTH"
#define AEC_KOQ_LENGTH_LONG                   "LENGTH_LONG"
#define AEC_KOQ_LENGTH_SHORT                  "LENGTH_SHORT"
#define AEC_KOQ_VOLUME                        "VOLUME"
#define AEC_KOQ_VOLUME_SMALL                  "VOLUME_SMALL"
#define AEC_KOQ_VOLUME_LARGE                  "VOLUME_LARGE"
#define AEC_KOQ_LIQUID_VOLUME                 "LIQUID_VOLUME"
#define AEC_KOQ_LIQUID_VOLUME_SMALL           "LIQUID_VOLUME_SMALL"
#define AEC_KOQ_LIQUID_VOLUME_LARGE           "LIQUID_VOLUME_LARGE"
#define AEC_KOQ_PROCESS_PIPING_PRESSURE       "PROCESS_PIPING_PRESSURE"
#define AEC_KOQ_PROCESS_PIPING_TEMPERATURE    "PROCESS_PIPING_TEMPERATURE"
#define AEC_KOQ_PROCESS_PIPING_FLOW           "PROCESS_PIPING_FLOW"
#define AEC_KOQ_WEIGHT                        "WEIGHT"
#define AEC_KOQ_CURRENT                       "CURRENT"
#define AEC_KOQ_FORCE                         "FORCE"
#define AEC_KOQ_POWER                         "POWER"



#define AEC_UNIT_DEG                           "ARC_DEG"
#define AEC_UNIT_RAD                           "RAD"

#define AEC_UNIT_MM                            "MM"
#define AEC_UNIT_IN                            "IN"
#define AEC_UNIT_M                             "M"
#define AEC_UNIT_FT                            "FT"

#define AEC_UNIT_SQ_MM                         "SQ.MM"
#define AEC_UNIT_SQ_IN                         "SQ.IN"
#define AEC_UNIT_SQ_M                          "SQ.M"
#define AEC_UNIT_SQ_FT                         "SQ.FT"

#define AEC_UNIT_CUB_MM                        "CUB.MM"
#define AEC_UNIT_CUB_IN                        "CUB.IN"
#define AEC_UNIT_CUB_M                         "CUB.M"
#define AEC_UNIT_CUB_FT                        "CUB.FT"
#define AEC_UNIT_CUB_YRD                       "CUB.YRD"
#define AEC_UNIT_LITRE                         "LITRE"
//#define AEC_UNIT_ML                            "LITRE/1000"
#define AEC_UNIT_GAL                           "GALLON"

#define AEC_UNIT_PSI                           "PSI"
#define AEC_UNIT_PSIG                          "PSIG"
#define AEC_UNIT_PA                            "PA"
#define AEC_UNIT_PAG                           "PA_GAUGE"

#define AEC_UNIT_CELSIUS                       "CELSIUS"
#define AEC_UNIT_FAHRENHEIT                    "FAHRENHEIT"

#define AEC_UNIT_G                             "G"
#define AEC_UNIT_KG                            "KG"
#define AEC_UNIT_LBM                           "LBM"

#define AEC_UNIT_AMP                           "A"

#define AEC_UNIT_LBF                           "LBF"
#define AEC_UNIT_N                             "N"
#define AEC_UNIT_KN                            "KN"
#define AEC_UNIT_KIP                           "KIPF"


#define AEC_UNIT_W                             "W"
#define AEC_UNIT_KW                            "KW"
#define AEC_UNIT_MEGAW                         "MEGAW"
#define AEC_UNIT_BTU_PER_HOUR                  "BTU/HR"
#define AEC_UNIT_HP                            "HP"

#define AEC_UNIT_CUB_M_PER_SEC                 "CUB.M/SEC"
#define AEC_UNIT_LITRE_PER_MIN                 "LITRE/MIN"
#define AEC_UNIT_CUB_FT_PER_MIN                "CUB.FT/MIN"
#define AEC_UNIT_GALLON_PER_MIN                "GALLON/MIN"




