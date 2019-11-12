/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__


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

#define AEC_KOQ_ANGLE                               "ANGLE"
#define AEC_KOQ_AREA                                "AREA"
#define AEC_KOQ_AREA_SMALL                          "AREA_SMALL"
#define AEC_KOQ_AREA_LARGE                          "AREA_LARGE"
#define AEC_KOQ_LENGTH                              "LENGTH"
#define AEC_KOQ_LENGTH_LONG                         "LENGTH_LONG"
#define AEC_KOQ_LENGTH_SHORT                        "LENGTH_SHORT"
#define AEC_KOQ_VOLUME                              "VOLUME"
#define AEC_KOQ_VOLUME_SMALL                        "VOLUME_SMALL"
#define AEC_KOQ_VOLUME_LARGE                        "VOLUME_LARGE"
#define AEC_KOQ_LIQUID_VOLUME                       "LIQUID_VOLUME"
#define AEC_KOQ_LIQUID_VOLUME_SMALL                 "LIQUID_VOLUME_SMALL"
#define AEC_KOQ_LIQUID_VOLUME_LARGE                 "LIQUID_VOLUME_LARGE"
#define AEC_KOQ_PROCESS_PIPING_PRESSURE             "PROCESS_PIPING_PRESSURE"
#define AEC_KOQ_PRESSURE                            "PRESSURE"
#define AEC_KOQ_PROCESS_PIPING_TEMPERATURE          "PROCESS_PIPING_TEMPERATURE"
#define AEC_KOQ_TEMPERATURE                         "TEMPERATURE"
#define AEC_KOQ_PROCESS_PIPING_FLOW                 "PROCESS_PIPING_FLOW"
#define AEC_KOQ_FLOW                                "FLOW"
#define AEC_KOQ_WEIGHT                              "WEIGHT"
#define AEC_KOQ_CURRENT                             "CURRENT"
#define AEC_KOQ_FORCE                               "FORCE"
#define AEC_KOQ_POWER                               "POWER"
#define AEC_KOQ_ELECTRIC_POTENTIAL                  "ELECTRIC_POTENTIAL"
#define AEC_KOQ_HEAT_TRANSFER                       "HEAT_TRANSFER"
#define AEC_KOQ_LUMINOUS_FLUX                       "LUMINOUS_FLUX"
#define AEC_KOQ_ILLUMINANCE                         "ILLUMINANCE"
#define AEC_KOQ_LUMINOUS_INTENSITY                  "LUMINOUS_INTENSITY"
#define AEC_KOQ_VELOCITY                            "VELOCITY"
#define AEC_KOQ_FREQUENCY                           "FREQUENCY"
#define AEC_KOQ_THERMAL_RESISTANCE                  "THERMAL_RESISTANCE"
#define AEC_KOQ_PRESSURE_GRADIENT                   "PRESSURE_GRADIENT"
#define AEC_KOQ_ENERGY                              "ENERGY"
#define AEC_KOQ_DYNAMIC_VISCOSITY                   "DYNAMIC_VISCOSITY"
#define AEC_KOQ_TIME                                "TIME"
#define AEC_KOQ_ACCELERATION                        "ACCELERATION"
#define AEC_KOQ_LINEAR_FORCE                        "LINEAR_FORCE"
#define AEC_KOQ_MOMENT_OF_INERTIA                   "MOMENT_OF_INERTIA"
#define AEC_KOQ_DENSITY                             "DENSITY"
#define AEC_KOQ_THERMAL_EXPANSION_COEFFICIENT       "THERMAL_EXPANSION_COEFFICIENT"
#define AEC_KOQ_SPECIFIC_HEAT_OF_VAPORIZATION       "SPECIFIC_HEAT_OF_VAPORIZATION"
#define AEC_KOQ_SPECIFIC_HEAT_CAPACITY              "SPECIFIC_HEAT_CAPACITY"
#define AEC_KOQ_LINEAR_DENSITY                      "LINEAR_DENSITY"
#define AEC_KOQ_FORCE_DENSITY                       "FORCE_DENSITY"
#define AEC_KOQ_LINEAR_ROTATIONAL_SPRING_CONSTANT   "LINEAR_ROTATIONAL_SPRING_CONSTANT"
#define AEC_KOQ_WARPING_CONSTANT                    "WARPING_CONSTANT"
#define AEC_KOQ_ANGULAR_VELOCITY                    "ANGULAR_VELOCITY"
#define AEC_KOQ_THERMAL_CONDUCTIVITY                "THERMAL_CONDUCTIVITY"



// Angular Units
#define AEC_UNIT_DEG                                        "Units:ARC_DEG"
#define AEC_UNIT_RAD                                        "Units:RAD"

// Length Units
#define AEC_UNIT_MM                                         "Units:MM"
#define AEC_UNIT_IN                                         "Units:IN"
#define AEC_UNIT_M                                          "Units:M"
#define AEC_UNIT_FT                                         "Units:FT"

// Area Units
#define AEC_UNIT_SQ_MM                                      "Units:SQ_MM"
#define AEC_UNIT_SQ_IN                                      "Units:SQ_IN"
#define AEC_UNIT_SQ_M                                       "Units:SQ_M"
#define AEC_UNIT_SQ_FT                                      "Units:SQ_FT"

// Volume Units
#define AEC_UNIT_CUB_MM                                     "Units:CUB_MM"
#define AEC_UNIT_CUB_IN                                     "Units:CUB_IN"
#define AEC_UNIT_CUB_M                                      "Units:CUB_M"
#define AEC_UNIT_CUB_FT                                     "Units:CUB_FT"
#define AEC_UNIT_CUB_YRD                                    "Units:CUB_YRD"
#define AEC_UNIT_LITRE                                      "Units:LITRE"
//#define AEC_UNIT_ML                                       "LITRE/1000"
#define AEC_UNIT_GAL                                        "Units:GALLON"

// Pressure Units
#define AEC_UNIT_PSI                                        "Units:PSI"
#define AEC_UNIT_PSIG                                       "Units:PSIG"
#define AEC_UNIT_PA                                         "Units:PA"
#define AEC_UNIT_PAG                                        "Units:PA_GAUGE"
#define AEC_UNIT_KGF_PER_SQ_M                               "Units:KGF_PER_SQ_M"


// Temperature Units
#define AEC_UNIT_CELSIUS                                    "Units:CELSIUS"
#define AEC_UNIT_FAHRENHEIT                                 "Units:FAHRENHEIT"
#define AEC_UNIT_KELVIN                                     "Units:K"

// Weight Units
#define AEC_UNIT_G                                          "Units:G"
#define AEC_UNIT_KG                                         "Units:KG"
#define AEC_UNIT_LBM                                        "Units:LBM"

// Current Units
#define AEC_UNIT_AMP                                        "Units:A"

// Force Units
#define AEC_UNIT_LBF                                        "Units:LBF"
#define AEC_UNIT_N                                          "Units:N"
#define AEC_UNIT_KN                                         "Units:KN"
#define AEC_UNIT_KIP                                        "Units:KPF"


// Power Units
#define AEC_UNIT_W                                          "Units:W"
#define AEC_UNIT_KW                                         "Units:KW"
#define AEC_UNIT_MEGAW                                      "Units:MEGAW"
#define AEC_UNIT_BTU_PER_HR                                 "Units:BTU_PER_HR"
#define AEC_UNIT_HP                                         "Units:HP"

// Flow Units
#define AEC_UNIT_CUB_M_PER_SEC                              "Units:CUB_M_PER_SEC"
#define AEC_UNIT_LITRE_PER_MIN                              "Units:LITRE_PER_MIN"
#define AEC_UNIT_CUB_FT_PER_MIN                             "Units:CUB_FT_PER_MIN"
#define AEC_UNIT_GALLON_PER_MIN                             "Units:GALLON_PER_MIN"
#define AEC_UNIT_LITRE_PER_SEC                              "Units:LITRE_PER_SEC"


// Electrical Potential Units
#define AEC_UNIT_VOLT                                       "Units:VOLT"
#define AEC_UNIT_KILOVOLT                                   "Units:KILOVOLT"
#define AEC_UNIT_MEGAVOLT                                   "Units:MEGAVOLT"

//  Heat Transfer Units
#define AEC_UNIT_WATT_PER_SQ_M_KELVIN                       "Units:W_PER_SQ_M_K"
#define AEC_UNIT_WATT_PER_SQ_M_CELSIUS                      "Units:W_PER_SQ_M_CELSIUS"
#define AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT                "Units:BTU_PER_SQ_FT_HR_FAHRENHEIT"

//  Luminous Flux Units
#define AEC_UNIT_LUMEN                                      "Units:LUMEN"

// Illuminance Units
#define AEC_UNIT_LUX                                        "Units:LUX"
#define AEC_UNIT_LUMEN_PER_SQ_FT                            "Units:LUMEN_PER_SQ_FT"

// Luminous Intensity Units
#define AEC_UNIT_CANDELA                                    "Units:CD"

// Velocity Units
#define AEC_UNIT_M_PER_SEC                                  "Units:M_PER_SEC"
#define AEC_UNIT_FT_PER_SEC                                 "Units:FT_PER_SEC"
#define AEC_UNIT_MPH                                        "Units:MPH"
#define AEC_UNIT_KMPH                                       "Units:KM_PER_HR"

// Frequency Units
#define AEC_UNIT_HZ                                         "Units:HZ"
#define AEC_UNIT_KHZ                                        "Units:KHZ"
#define AEC_UNIT_MHZ                                        "Units:MHZ"

// Thermal Resistance Units
#define AEC_UNIT_SQ_M_KELVIN_PER_WATT                       "Units:SQ_M_KELVIN_PER_WATT"
#define AEC_UNIT_SQ_M_CELSIUS_PER_WATT                      "Units:SQ_M_CELSIUS_PER_WATT"
#define AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU                "Units:SQ_FT_HR_FAHRENHEIT_PER_BTU"

// Pressure Gradients Units
#define AEC_UNIT_PA_PER_M                                   "Units:PA_PER_M"
#define AEC_UNIT_BAR_PER_KM                                 "Units:BAR_PER_KM"

// Energy Units
#define AEC_UNIT_JOULES                                     "Units:J"
#define AEC_UNIT_KILOJOULES                                 "Units:KJ"
#define AEC_UNIT_BTU                                        "Units:BTU"
#define AEC_UNIT_KW_HR                                      "Units:KWH"

// Dynamic Viscosity Units
#define AEC_UNIT_PA_SEC                                      "Units:PA_S"
#define AEC_UNIT_POISE                                       "Units:POISE"
#define AEC_UNIT_CENTIPOISE                                  "Units:CENTIPOISE"
#define AEC_UNIT_LBM_PER_FT_PER_SEC                          "Units:LBM_PER_FT_S"

// Time Units
#define AEC_UNIT_SEC                                         "Units:S"
#define AEC_UNIT_MIN                                         "Units:MIN"
#define AEC_UNIT_HR                                          "Units:HR"
#define AEC_UNIT_DAY                                         "Units:DAY"
#define AEC_UNIT_MILLISEC                                    "Units:MS"

// Time Units
#define AEC_UNIT_M_PER_SEC_SQ                                "Units:M_PER_SEC_SQ"
#define AEC_UNIT_CM_PER_SEC_SQ                               "Units:CM_PER_SEC_SQ"
#define AEC_UNIT_FT_PER_SEC_SQ                               "Units:FT_PER_SEC_SQ"

// Linear Force Units
#define AEC_UNIT_N_PER_M                                     "Units:N_PER_M"
#define AEC_UNIT_N_PER_MM                                    "Units:N_PER_MM"
#define AEC_UNIT_LBF_PER_IN                                  "Units:LBF_PER_IN"

// Moment Of Inertia Units
#define AEC_UNIT_M_FOURTH                                     "Units:M_TO_THE_FOURTH"
#define AEC_UNIT_MM_FOURTH                                    "Units:MM_TO_THE_FOURTH"
#define AEC_UNIT_CM_FOURTH                                    "Units:CM_TO_THE_FOURTH"
#define AEC_UNIT_IN_FOURTH                                    "Units:IN_TO_THE_FOURTH"
#define AEC_UNIT_FT_FOURTH                                    "Units:FT_TO_THE_FOURTH"

// Moment Of Inertia Units
#define AEC_UNIT_KG_PER_CUB_M                                  "Units:KG_PER_CUB_M"
#define AEC_UNIT_G_PER_CUB_CM                                  "Units:G_PER_CUB_CM"
#define AEC_UNIT_LBM_PER_CUB_FT                                "Units:LBM_PER_CUB_FT"
#define AEC_UNIT_LBM_PER_CUB_IN                                "Units:LBM_PER_CUB_IN"
#define AEC_UNIT_KIP_PER_CUB_FT                                "Units:KIP_PER_CUB_FT"

// Thermal Expansion Coefficient Units
#define AEC_UNIT_STRAIN_PER_KELVIN                             "Units:STRAIN_PER_KELVIN"
#define AEC_UNIT_STRAIN_PER_CELSIUS                            "Units:STRAIN_PER_CELSIUS"
#define AEC_UNIT_STRAIN_PER_FAHRENHEIT                         "Units:STRAIN_PER_FAHRENHEIT"

// Specific Heat Of Vaporization Units
#define AEC_UNIT_J_PER_KG                                       "Units:J_PER_KG"
#define AEC_UNIT_KJ_PER_KG                                      "Units:KJ_PER_KG"
#define AEC_UNIT_BTU_PER_LBM                                    "Units:BTU_PER_LBM"

// Specific Heat Capacity Units
#define AEC_UNIT_J_PER_KG_KELVIN                                "Units:J_PER_KG_K"
#define AEC_UNIT_BTU_PER_LBM_RANKINE                            "Units:BTU_PER_LBM_RANKINE"

// Linear Density Units
#define AEC_UNIT_KG_PER_M                                       "Units:KG_PER_M"
#define AEC_UNIT_KG_PER_MM                                      "Units:KG_PER_MM"
#define AEC_UNIT_LBM_PER_FT                                     "Units:LBM_PER_FT"

// Force Density Units
#define AEC_UNIT_N_PER_CUB_M                                    "Units:N_PER_CUB_M"
#define AEC_UNIT_KN_PER_CUB_M                                   "Units:KN_PER_CUB_M"
#define AEC_UNIT_N_PER_CUB_FT                                   "Units:N_PER_CUB_FT"
#define AEC_UNIT_KN_PER_CUB_FT                                  "Units:KN_PER_CUB_FT"

// Rotational Spring Constant Units
#define AEC_UNIT_N_PER_RAD                                      "Units:N_PER_RAD"

// Warping Constant Units
#define AEC_UNIT_M_SIXTH                                        "Units:M_TO_THE_SIXTH"
#define AEC_UNIT_FT_SIXTH                                       "Units:FT_TO_THE_SIXTH"

// Anguler Velocity Units
#define AEC_UNIT_RAD_PER_SEC                                    "Units:RAD_PER_SEC"
#define AEC_UNIT_DEG_PER_SEC                                    "Units:DEG_PER_SEC"
#define AEC_UNIT_RPM                                            "Units:RPM"

// Thermal Conductivity Units
#define AEC_UNIT_W_PER_M_KELVIN                                 "Units:W_PER_M_K"
#define AEC_UNIT_W_PER_M_CELSIUS                                "Units:W_PER_M_C"
#define AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT                 "Units:BTU_IN_PER_SQ_FT_HR_FAHRENHEIT"

