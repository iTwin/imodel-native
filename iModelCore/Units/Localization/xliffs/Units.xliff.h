/*--------------------------------------------------------------------------------------+
|
|     $Source: Localization/xliffs/Units.xliff.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(UnitsL10N,units_msg)
    L10N_STRING(UNIT_LABEL_SurveyMiles)                         // =="sm"==
    L10N_STRING(UNIT_LABEL_Miles)                               // =="mi"==
    L10N_STRING(UNIT_LABEL_Furlongs)                            // =="fur"==
    L10N_STRING(UNIT_LABEL_Chains)                              // =="ch"==
    L10N_STRING(UNIT_LABEL_Rods)                                // =="rd"==
    L10N_STRING(UNIT_LABEL_Fathoms)                             // =="fat"==
    L10N_STRING(UNIT_LABEL_Yards)                               // =="yd"==
    L10N_STRING(UNIT_LABEL_SurveyFeet)                          // =="sf"==
    L10N_STRING(UNIT_LABEL_Feet)                                // =="'"==
    L10N_STRING(UNIT_LABEL_SurveyInches)                        // =="si"==
    L10N_STRING(UNIT_LABEL_Inches)                              // =="""==
    L10N_STRING(UNIT_LABEL_Picas)                               // =="pica"==
    L10N_STRING(UNIT_LABEL_Points)                              // =="pt"==
    L10N_STRING(UNIT_LABEL_Mils)                                // =="mil"==
    L10N_STRING(UNIT_LABEL_MicroInches)                         // =="ui"==
    L10N_STRING(UNIT_LABEL_Petameters)                          // =="Pm"==
    L10N_STRING(UNIT_LABEL_Terameters)                          // =="Tm"==
    L10N_STRING(UNIT_LABEL_Gigameters)                          // =="Gm"==
    L10N_STRING(UNIT_LABEL_Megameters)                          // =="Mm"==
    L10N_STRING(UNIT_LABEL_Kilometers)                          // =="km"==
    L10N_STRING(UNIT_LABEL_Hectometers)                         // =="hm"==
    L10N_STRING(UNIT_LABEL_Dekameters)                          // =="dam"==
    L10N_STRING(UNIT_LABEL_Meters)                              // =="m"==
    L10N_STRING(UNIT_LABEL_Decimeters)                          // =="dm"==
    L10N_STRING(UNIT_LABEL_Centimeters)                         // =="cm"==
    L10N_STRING(UNIT_LABEL_Millimeters)                         // =="mm"==
    L10N_STRING(UNIT_LABEL_Micrometers)                         // =="um"==
    L10N_STRING(UNIT_LABEL_Nanometers)                          // =="nm"==
    L10N_STRING(UNIT_LABEL_Picometers)                          // =="pm"==
    L10N_STRING(UNIT_LABEL_Femtometers)                         // =="fm"==
    L10N_STRING(UNIT_LABEL_Parsecs)                             // =="pc"==
    L10N_STRING(UNIT_LABEL_LightYears)                          // =="l.y."==
    L10N_STRING(UNIT_LABEL_AstronomicalUnits)                   // =="AU"==
    L10N_STRING(UNIT_LABEL_NauticalMiles)                       // =="nm"==
    L10N_STRING(UNIT_LABEL_Angstroms)                           // =="A"==
    L10N_STRING(UNIT_LABEL_Radians)                             // =="rad"==
    L10N_STRING(UNIT_LABEL_Degrees)                             // =="deg"==
    L10N_STRING(UNIT_LABEL_Grads)                               // =="grad"==
    L10N_STRING(UNIT_LABEL_Minutes)                             // =="min"==
    L10N_STRING(UNIT_LABEL_Seconds)                             // =="sec"==
    L10N_STRING(UNIT_LABEL_UnitlessWhole)                       // =="uu"==
    L10N_STRING(UNIT_LABEL_CUSTOM)                              // =="??"==
    L10N_STRING(UNIT_LABEL_SUFFIX_Area)                         // =="2"==
    L10N_STRING(UNIT_LABEL_SUFFIX_Volume)                       // =="3"==
    L10N_STRING(UNIT_LABEL_PREFIX_Area)                         // =="Sq."==
    L10N_STRING(UNIT_LABEL_PREFIX_Volume)                       // =="Cu."==
    L10N_STRING(UNIT_SINGULAR_NAME_None)                        // =="Unitless"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyMiles)                 // =="US Survey Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Miles)                       // =="Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Furlongs)                    // =="Furlong"==
    L10N_STRING(UNIT_SINGULAR_NAME_Chains)                      // =="Chain"==
    L10N_STRING(UNIT_SINGULAR_NAME_Rods)                        // =="Rod"==
    L10N_STRING(UNIT_SINGULAR_NAME_Fathoms)                     // =="Fathom"==
    L10N_STRING(UNIT_SINGULAR_NAME_Yards)                       // =="Yard"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyFeet)                  // =="US Survey Foot"==
    L10N_STRING(UNIT_SINGULAR_NAME_Feet)                        // =="Foot"==
    L10N_STRING(UNIT_SINGULAR_NAME_SurveyInches)                // =="US Survey Inch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Inches)                      // =="Inch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Picas)                       // =="Pica"==
    L10N_STRING(UNIT_SINGULAR_NAME_Points)                      // =="Point"==
    L10N_STRING(UNIT_SINGULAR_NAME_Mils)                        // =="Mil"==
    L10N_STRING(UNIT_SINGULAR_NAME_MicroInches)                 // =="MicroInch"==
    L10N_STRING(UNIT_SINGULAR_NAME_Petameters)                  // =="Petameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Terameters)                  // =="Terameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Gigameters)                  // =="Gigameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Megameters)                  // =="Megameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Kilometers)                  // =="Kilometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Hectometers)                 // =="Hectometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Dekameters)                  // =="Dekameter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Meters)                      // =="Meter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Decimeters)                  // =="Decimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Centimeters)                 // =="Centimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Millimeters)                 // =="Millimeter"==
    L10N_STRING(UNIT_SINGULAR_NAME_Micrometers)                 // =="Micrometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Nanometers)                  // =="Nanometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Picometers)                  // =="Picometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Femtometers)                 // =="Femtometer"==
    L10N_STRING(UNIT_SINGULAR_NAME_Parsecs)                     // =="Parsec"==
    L10N_STRING(UNIT_SINGULAR_NAME_LightYears)                  // =="Light Year"==
    L10N_STRING(UNIT_SINGULAR_NAME_AstronomicalUnits)           // =="Astronomical Unit"==
    L10N_STRING(UNIT_SINGULAR_NAME_NauticalMiles)               // =="Nautical Mile"==
    L10N_STRING(UNIT_SINGULAR_NAME_Angstroms)                   // =="Angstrom"==
    L10N_STRING(UNIT_SINGULAR_NAME_Radians)                     // =="Radian"==
    L10N_STRING(UNIT_SINGULAR_NAME_Degrees)                     // =="Degree"==
    L10N_STRING(UNIT_SINGULAR_NAME_Grads)                       // =="Grad"==
    L10N_STRING(UNIT_SINGULAR_NAME_Minutes)                     // =="Minute"==
    L10N_STRING(UNIT_SINGULAR_NAME_Seconds)                     // =="Second"==
    L10N_STRING(UNIT_SINGULAR_NAME_UnitlessWhole)               // =="Unitless Unit"==
    L10N_STRING(UNIT_SINGULAR_NAME_CUSTOM)                      // =="Custom"==
    L10N_STRING(UNIT_PLURAL_NAME_None)                          // =="Unitless"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyMiles)                   // =="US Survey Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Miles)                         // =="Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Furlongs)                      // =="Furlongs"==
    L10N_STRING(UNIT_PLURAL_NAME_Chains)                        // =="Chains"==
    L10N_STRING(UNIT_PLURAL_NAME_Rods)                          // =="Rods"==
    L10N_STRING(UNIT_PLURAL_NAME_Fathoms)                       // =="Fathoms"==
    L10N_STRING(UNIT_PLURAL_NAME_Yards)                         // =="Yards"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyFeet)                    // =="US Survey Feet"==
    L10N_STRING(UNIT_PLURAL_NAME_Feet)                          // =="Feet"==
    L10N_STRING(UNIT_PLURAL_NAME_SurveyInches)                  // =="US Survey Inches"==
    L10N_STRING(UNIT_PLURAL_NAME_Inches)                        // =="Inches"==
    L10N_STRING(UNIT_PLURAL_NAME_Picas)                         // =="Picas"==
    L10N_STRING(UNIT_PLURAL_NAME_Points)                        // =="Points"==
    L10N_STRING(UNIT_PLURAL_NAME_Mils)                          // =="Mils"==
    L10N_STRING(UNIT_PLURAL_NAME_MicroInches)                   // =="MicroInches"==
    L10N_STRING(UNIT_PLURAL_NAME_Petameters)                    // =="Petameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Terameters)                    // =="Terameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Gigameters)                    // =="Gigameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Megameters)                    // =="Megameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Kilometers)                    // =="Kilometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Hectometers)                   // =="Hectometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Dekameters)                    // =="Dekameters"==
    L10N_STRING(UNIT_PLURAL_NAME_Meters)                        // =="Meters"==
    L10N_STRING(UNIT_PLURAL_NAME_Decimeters)                    // =="Decimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Centimeters)                   // =="Centimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Millimeters)                   // =="Millimeters"==
    L10N_STRING(UNIT_PLURAL_NAME_Micrometers)                   // =="Micrometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Nanometers)                    // =="Nanometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Picometers)                    // =="Picometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Femtometers)                   // =="Femtometers"==
    L10N_STRING(UNIT_PLURAL_NAME_Parsecs)                       // =="Parsecs"==
    L10N_STRING(UNIT_PLURAL_NAME_LightYears)                    // =="Light Years"==
    L10N_STRING(UNIT_PLURAL_NAME_AstronomicalUnits)             // =="Astronomical Units"==
    L10N_STRING(UNIT_PLURAL_NAME_NauticalMiles)                 // =="Nautical Miles"==
    L10N_STRING(UNIT_PLURAL_NAME_Angstroms)                     // =="Angstroms"==
    L10N_STRING(UNIT_PLURAL_NAME_Radians)                       // =="Radians"==
    L10N_STRING(UNIT_PLURAL_NAME_Degrees)                       // =="Degrees"==
    L10N_STRING(UNIT_PLURAL_NAME_Grads)                         // =="Grads"==
    L10N_STRING(UNIT_PLURAL_NAME_Minutes)                       // =="Minutes"==
    L10N_STRING(UNIT_PLURAL_NAME_Seconds)                       // =="Seconds"==
    L10N_STRING(UNIT_PLURAL_NAME_UnitlessWhole)                 // =="Unitless Units"==
    L10N_STRING(UNIT_PLURAL_NAME_CUSTOM)                        // =="Custom"==

    L10N_STRING(PHENOMENON_LENGTH)                                      // =="Length"==                                      
    L10N_STRING(PHENOMENON_MASS)                                        // =="Mass"==                                        
    L10N_STRING(PHENOMENON_TIME)                                        // =="Time"==                                        
    L10N_STRING(PHENOMENON_TEMPERATURE)                                 // =="Temperature"==                                 
    L10N_STRING(PHENOMENON_TEMPERATURE_CHANGE)                          // =="Temperature Change"==                          
    L10N_STRING(PHENOMENON_CURRENT)                                     // =="Current"==                                     
    L10N_STRING(PHENOMENON_MOLE)                                        // =="Mole"==                                        
    L10N_STRING(PHENOMENON_LUMINOSITY)                                  // =="Luminosity"==                                  
    L10N_STRING(PHENOMENON_ONE)                                         // =="One"==                                         
    L10N_STRING(PHENOMENON_ANGLE)                                       // =="Angle"==                                       
    L10N_STRING(PHENOMENON_SOLIDANGLE)                                  // =="Solid Angle"==                                  
    L10N_STRING(PHENOMENON_APPARENT_POWER)                              // =="Apparent Power"==                              
    L10N_STRING(PHENOMENON_AREA)                                        // =="Area"==                                        
    L10N_STRING(PHENOMENON_VOLUME)                                      // =="Volume"==                                      
    L10N_STRING(PHENOMENON_VELOCITY)                                    // =="Velocity"==                                    
    L10N_STRING(PHENOMENON_MOMENTUM)                                    // =="Momentum"==                                    
    L10N_STRING(PHENOMENON_ANGULAR_VELOCITY)                            // =="Angular Velocity"==                            
    L10N_STRING(PHENOMENON_ACCELERATION)                                // =="Acceleration"==                                
    L10N_STRING(PHENOMENON_ANGULAR_ACCELERATION)                        // =="Angular_acceleration"==                        
    L10N_STRING(PHENOMENON_FORCE)                                       // =="Force"==                                       
    L10N_STRING(PHENOMENON_PRESSURE)                                    // =="Pressure"==                                    
    L10N_STRING(PHENOMENON_FORCE_DENSITY)                               // =="Force Density"==                               
    L10N_STRING(PHENOMENON_PRESSURE_GRADIENT)                           // =="Pressure Gradient"==                           
    L10N_STRING(PHENOMENON_TORQUE)                                      // =="Torque"==                                      
    L10N_STRING(PHENOMENON_MOMENT_INERTIA)                              // =="Moment Inertia"==                              
    L10N_STRING(PHENOMENON_AREA_MOMENT_INERTIA)                         // =="Area Moment Inertia"==                         
    L10N_STRING(PHENOMENON_MASS_RATIO)                                  // =="Mass Ratio"==                                  
    L10N_STRING(PHENOMENON_DENSITY)                                     // =="Density"==                                     
    L10N_STRING(PHENOMENON_SPECIFIC_VOLUME)                             // =="Specific Volume"==                             
    L10N_STRING(PHENOMENON_LINEAR_DENSITY)                              // =="Linear Density"==                              
    L10N_STRING(PHENOMENON_SURFACE_DENSITY)                             // =="Surface Density"==                             
    L10N_STRING(PHENOMENON_WORK)                                        // =="Work"==                                        
    L10N_STRING(PHENOMENON_POWER)                                       // =="Power"==                                       
    L10N_STRING(PHENOMENON_FLOW)                                        // =="Flow"==                                        
    L10N_STRING(PHENOMENON_SURFACE_FLOW_RATE)                           // =="Surface Flow Rate"==                           
    L10N_STRING(PHENOMENON_SURFACE_POWER_DENSITY)                       // =="Surface Power Density"==                       
    L10N_STRING(PHENOMENON_MASS_FLOW)                                   // =="Mass Flow"==                                   
    L10N_STRING(PHENOMENON_PARTICLE_FLOW)                               // =="Particle Flow"==                               
    L10N_STRING(PHENOMENON_DYNAMIC_VISCOSITY)                           // =="Dynamic Viscosity"==                           
    L10N_STRING(PHENOMENON_KINEMATIC_VISCOSITY)                         // =="Kinematic Viscosity"==                         
    L10N_STRING(PHENOMENON_ELECTRIC_CHARGE)                             // =="Electric Charge"==                             
    L10N_STRING(PHENOMENON_ELECTRIC_POTENTIAL)                          // =="Electric Potential"==                          
    L10N_STRING(PHENOMENON_ELECTRIC_RESISTANCE)                         // =="Electric Resistance"==                         
    L10N_STRING(PHENOMENON_CAPACITANCE)                                 // =="Capacitance"==                                 
    L10N_STRING(PHENOMENON_MAGNETIC_FLUX)                               // =="Magnetic Flux"==                               
    L10N_STRING(PHENOMENON_MAGNETIC_FLUX_DENSITY)                       // =="Magnetic Flux Density"==                       
    L10N_STRING(PHENOMENON_INDUCTANCE)                                  // =="Inductance"==                                  
    L10N_STRING(PHENOMENON_LUMINOUS_FLUX)                               // =="Luminous Flux"==                               
    L10N_STRING(PHENOMENON_ILLUMINANCE)                                 // =="Illuminance"==                                 
    L10N_STRING(PHENOMENON_RADIATION)                                   // =="Radiation"==                                   
    L10N_STRING(PHENOMENON_RADEXPOSURE)                                 // =="Radexposure"==                                 
    L10N_STRING(PHENOMENON_RADABSORBDOSE)                               // =="Radabsorbdose"==                               
    L10N_STRING(PHENOMENON_RADEQUDOSE)                                  // =="Radequdose"==                                  
    L10N_STRING(PHENOMENON_SIZE_LENGTH_RATE)                            // =="Size Length Rate"==                            
    L10N_STRING(PHENOMENON_CAPITA)                                      // =="Capita"==                                      
    L10N_STRING(PHENOMENON_THERMAL_CONDUCTIVITY)                        // =="Thermal Conductivity"==                        
    L10N_STRING(PHENOMENON_THERMAL_RESISTANCE)                          // =="Thermal Resistance"==                          
    L10N_STRING(PHENOMENON_TEMPERATURE_GRADIENT)                        // =="Temperature Gradient"==                        
    L10N_STRING(PHENOMENON_MOLAR_VOLUME)                                // =="Molar Volume"==                                
    L10N_STRING(PHENOMENON_MOLAR_CONCENTRATION)                         // =="Molar Concentration"==                         
    L10N_STRING(PHENOMENON_SLOPE)                                       // =="Slope"==                                       
    L10N_STRING(PHENOMENON_GRAVCONSTANT)                                // =="Gravconstant"==                                
    L10N_STRING(PHENOMENON_THREAD_PITCH)                                // =="Thread Pitch"==                                
    L10N_STRING(PHENOMENON_HEAT_TRANSFER)                               // =="Heat Transfer"==                               
    L10N_STRING(PHENOMENON_HEAT_FLUX_DENSITY)                           // =="Heat Flux Density"==                           
    L10N_STRING(PHENOMENON_TORSIONAL_WARPING_CONSTANT)                  // =="Torsional Warping Constant"==                  
    L10N_STRING(PHENOMENON_POPULATION_DENSITY)                          // =="Population Density"==                          
    L10N_STRING(PHENOMENON_FREQUENCY)                                   // =="Frequency"==                                   
    L10N_STRING(PHENOMENON_LINEAR_LOAD)                                 // =="Linear Load"==                                 
    L10N_STRING(PHENOMENON_AREA_LOAD)                                   // =="Area Load"==                                   
    L10N_STRING(PHENOMENON_ENERGY_DENSITY)                              // =="Energy Density"==                              
    L10N_STRING(PHENOMENON_SPECIFIC_ENERGY)                             // =="Specific Energy"==                             
    L10N_STRING(PHENOMENON_HEATING_VALUE_MOLE)                          // =="Heating Value Mole"==                          
    L10N_STRING(PHENOMENON_SPECIFIC_HEAT_CAPACITY)                      // =="Specific_heat Capacity"==                      
    L10N_STRING(PHENOMENON_SPECIFIC_HEAT_CAPACITY_MOLAR)                // =="Specific_heat Capacity Molar"==                
    L10N_STRING(PHENOMENON_ROTATIONAL_SPRING_CONSTANT)                  // =="Rotational Spring Constant"==                  
    L10N_STRING(PHENOMENON_LINEAR_ROTATIONAL_SPRING_CONSTANT)           // =="Linear Rotational Spring Constant"==           
    L10N_STRING(PHENOMENON_PERCENTAGE)                                  // =="Percentage"==                                  
    L10N_STRING(PHENOMENON_LINEAR_COST)                                 // =="Linear Cost"==                                 
    L10N_STRING(PHENOMENON_LINEAR_RATE)                                 // =="Linear Rate"==                                 
    L10N_STRING(PHENOMENON_LINEAR_COEFFICIENT_OF_THERMAL_EXPANSION)     // =="Linear Coefficient Of Thermal Expansion"==     
    L10N_STRING(PHENOMENON_AREA_COEFFICIENT_OF_THERMAL_EXPANSION)       // =="Area Coefficient Of Thermal Expansion"==       
    L10N_STRING(PHENOMENON_VOLUMETRIC_COEFFICIENT_OF_THERMAL_EXPANSION) // =="Volumetric Coefficient Of Thermal Expansion"== 
    L10N_STRING(PHENOMENON_VOLUME_RATIO)                                // =="Volume Ratio"==                                
BENTLEY_TRANSLATABLE_STRINGS_END

END_BENTLEY_UNITS_NAMESPACE

