/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/AecUnits/AecUnitsDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
#define AEC_UNIT_DEG                                        "ARC_DEG"
#define AEC_UNIT_RAD                                        "RAD"

// Length Units
#define AEC_UNIT_MM                                         "MM"
#define AEC_UNIT_IN                                         "IN"
#define AEC_UNIT_M                                          "M"
#define AEC_UNIT_FT                                         "FT"

// Area Units
#define AEC_UNIT_SQ_MM                                      "SQ.MM"
#define AEC_UNIT_SQ_IN                                      "SQ.IN"
#define AEC_UNIT_SQ_M                                       "SQ.M"
#define AEC_UNIT_SQ_FT                                      "SQ.FT"

// Volume Units
#define AEC_UNIT_CUB_MM                                     "CUB.MM"
#define AEC_UNIT_CUB_IN                                     "CUB.IN"
#define AEC_UNIT_CUB_M                                      "CUB.M"
#define AEC_UNIT_CUB_FT                                     "CUB.FT"
#define AEC_UNIT_CUB_YRD                                    "CUB.YRD"
#define AEC_UNIT_LITRE                                      "LITRE"
//#define AEC_UNIT_ML                                       "LITRE/1000"
#define AEC_UNIT_GAL                                        "GALLON"

// Pressure Units
#define AEC_UNIT_PSI                                        "PSI"
#define AEC_UNIT_PSIG                                       "PSIG"
#define AEC_UNIT_PA                                         "PA"
#define AEC_UNIT_PAG                                        "PA_GAUGE"
#define AEC_UNIT_KGF_PER_SQ_M                               "KGF/SQ.M"


// Temperature Units
#define AEC_UNIT_CELSIUS                                    "CELSIUS"
#define AEC_UNIT_FAHRENHEIT                                 "FAHRENHEIT"
#define AEC_UNIT_KELVIN                                     "K"

// Weight Units
#define AEC_UNIT_G                                          "G"
#define AEC_UNIT_KG                                         "KG"
#define AEC_UNIT_LBM                                        "LBM"

// Current Units
#define AEC_UNIT_AMP                                        "A"

// Force Units
#define AEC_UNIT_LBF                                        "LBF"
#define AEC_UNIT_N                                          "N"
#define AEC_UNIT_KN                                         "KN"
#define AEC_UNIT_KIP                                        "KIPF"


// Power Units
#define AEC_UNIT_W                                          "W"
#define AEC_UNIT_KW                                         "KW"
#define AEC_UNIT_MEGAW                                      "MEGAW"
#define AEC_UNIT_BTU_PER_HR                                 "BTU/HR"
#define AEC_UNIT_HP                                         "HP"

// Flow Units
#define AEC_UNIT_CUB_M_PER_SEC                              "CUB.M/SEC"
#define AEC_UNIT_LITRE_PER_MIN                              "LITRE/MIN"
#define AEC_UNIT_CUB_FT_PER_MIN                             "CUB.FT/MIN"
#define AEC_UNIT_GALLON_PER_MIN                             "GALLON/MIN"
#define AEC_UNIT_LITRE_PER_SEC                              "LITRE/SEC"


// Electrical Potential Units
#define AEC_UNIT_VOLT                                       "VOLT"
#define AEC_UNIT_KILOVOLT                                   "KILOVOLT"
#define AEC_UNIT_MEGAVOLT                                   "MEGAVOLT"

//  Heat Transfer Units
#define AEC_UNIT_WATT_PER_SQ_M_KELVIN                       "W/(SQ.M*K)"
#define AEC_UNIT_WATT_PER_SQ_M_CELSIUS                      "W/(SQ.M*CELSIUS)"
#define AEC_UNIT_BTU_PER_SQ_FT_HR_FAHRENHEIT                "BTU/(SQ.FT*HR*FAHRENHEIT)"

//  Luminous Flux Units
#define AEC_UNIT_LUMEN                                      "LUMEN"

// Illuminance Units
#define AEC_UNIT_LUX                                        "LUX"
#define AEC_UNIT_LUMEN_PER_SQ_FT                            "LUMEN/SQ.FT"

// Luminous Intensity Units
#define AEC_UNIT_CANDELA                                    "CD"

// Velocity Units
#define AEC_UNIT_M_PER_SEC                                  "M/SEC"
#define AEC_UNIT_FT_PER_SEC                                 "FT/SEC"
#define AEC_UNIT_MPH                                        "MPH"
#define AEC_UNIT_KMPH                                       "KM/HR"

// Frequency Units
#define AEC_UNIT_HZ                                         "HZ"
#define AEC_UNIT_KHZ                                        "KHZ"
#define AEC_UNIT_MHZ                                        "MHZ"

// Thermal Resistance Units
#define AEC_UNIT_SQ_M_KELVIN_PER_WATT                       "(SQ.M*KELVIN)/WATT"
#define AEC_UNIT_SQ_M_CELSIUS_PER_WATT                      "(SQ.M*CELSIUS)/WATT"
#define AEC_UNIT_SQ_FT_HR_FAHRENHEIT_PER_BTU                "(SQ.FT*HR*FAHRENHEIT)/BTU"

// Pressure Gradients Units
#define AEC_UNIT_PA_PER_M                                   "PA/M"
#define AEC_UNIT_BAR_PER_KM                                 "BAR/KM"

// Energy Units
#define AEC_UNIT_JOULES                                     "J"
#define AEC_UNIT_KILOJOULES                                 "KJ"
#define AEC_UNIT_BTU                                        "BTU"
#define AEC_UNIT_KW_HR                                      "KWH"

// Dynamic Viscosity Units
#define AEC_UNIT_PA_SEC                                      "PA-S"
#define AEC_UNIT_POISE                                       "POISE"
#define AEC_UNIT_CENTIPOISE                                  "CENTIPOISE"
#define AEC_UNIT_LBM_PER_FT_PER_SEC                          "LBM/(FT*S)"

// Time Units
#define AEC_UNIT_SEC                                         "S"
#define AEC_UNIT_MIN                                         "MIN"
#define AEC_UNIT_HR                                          "HR"
#define AEC_UNIT_DAY                                         "DAY"
#define AEC_UNIT_MILLISEC                                    "MS"

// Time Units
#define AEC_UNIT_M_PER_SEC_SQ                                "M/SEC.SQ"
#define AEC_UNIT_CM_PER_SEC_SQ                               "CM/SEC.SQ"
#define AEC_UNIT_FT_PER_SEC_SQ                               "FT/SEC.SQ"

// Linear Force Units
#define AEC_UNIT_N_PER_M                                     "N/M"
#define AEC_UNIT_N_PER_MM                                    "N/MM"
#define AEC_UNIT_LBF_PER_IN                                  "LBF/IN"

// Moment Of Inertia Units
#define AEC_UNIT_M_FOURTH                                     "M^4"
#define AEC_UNIT_MM_FOURTH                                    "MM^4"
#define AEC_UNIT_CM_FOURTH                                    "CM^4"
#define AEC_UNIT_IN_FOURTH                                    "IN^4"
#define AEC_UNIT_FT_FOURTH                                    "FT^4"

// Moment Of Inertia Units
#define AEC_UNIT_KG_PER_CUB_M                                  "KG/CUB.M"
#define AEC_UNIT_G_PER_CUB_CM                                  "G/CUB.CM"
#define AEC_UNIT_LBM_PER_CUB_FT                                "LBM/CUB.FT"
#define AEC_UNIT_LBM_PER_CUB_IN                                "LBM/CUB.IN"
#define AEC_UNIT_KIP_PER_CUB_FT                                "KIP/CUB.FT"

// Thermal Expansion Coefficient Units
#define AEC_UNIT_STRAIN_PER_KELVIN                             "STRAIN/KELVIN"
#define AEC_UNIT_STRAIN_PER_CELSIUS                            "STRAIN/CELSIUS"
#define AEC_UNIT_STRAIN_PER_FAHRENHEIT                         "STRAIN/FAHRENHEIT"

// Specific Heat Of Vaporization Units
#define AEC_UNIT_J_PER_KG                                       "J/KG"
#define AEC_UNIT_KJ_PER_KG                                      "KJ/KG"
#define AEC_UNIT_BTU_PER_LBM                                    "BTU/LBM"

// Specific Heat Capacity Units
#define AEC_UNIT_J_PER_KG_KELVIN                                "J/(KG*K)"
#define AEC_UNIT_BTU_PER_LBM_RANKINE                            "BTU/(LBM*RANKINE)"

// Linear Density Units
#define AEC_UNIT_KG_PER_M                                       "KG/M"
#define AEC_UNIT_KG_PER_MM                                      "KG/MM"
#define AEC_UNIT_LBM_PER_FT                                     "LBM/FT"

// Force Density Units
#define AEC_UNIT_N_PER_CUB_M                                    "N/CUB.M"
#define AEC_UNIT_KN_PER_CUB_M                                   "KN/CUB.M"
#define AEC_UNIT_N_PER_CUB_FT                                   "N/CUB.FT"
#define AEC_UNIT_KN_PER_CUB_FT                                  "KN/CUB.FT"

// Rotational Spring Constant Units
#define AEC_UNIT_N_PER_RAD                                      "N/RAD"

// Warping Constant Units
#define AEC_UNIT_M_SIXTH                                        "M^6"
#define AEC_UNIT_FT_SIXTH                                       "FT^6"

// Anguler Velocity Units
#define AEC_UNIT_RAD_PER_SEC                                    "RAD/SEC"
#define AEC_UNIT_DEG_PER_SEC                                    "DEG/SEC"
#define AEC_UNIT_RPM                                            "RPM"

// Thermal Conductivity Units
#define AEC_UNIT_W_PER_M_KELVIN                                 "W/(M*K)"
#define AEC_UNIT_W_PER_M_CELSIUS                                "W/(M*C)"
#define AEC_UNIT_BTU_IN_PER_SQ_FT_HR_FAHRENHEIT                 "(BTU*IN)/(SQ.FT*HR*FAHRENHEIT)"

