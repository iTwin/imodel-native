/*--------------------------------------------------------------------------------------+
|
|     $Source: Localization/xliffs/Units.xliff.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(UnitsL10N,units_msg)
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

    L10N_STRING(DecimalPrecision_0)                                     // =="0"==
    L10N_STRING(DecimalPrecision_1)                                     // =="0.0"==
    L10N_STRING(DecimalPrecision_2)                                     // =="0.00"==
    L10N_STRING(DecimalPrecision_3)                                     // =="0.000"==
    L10N_STRING(DecimalPrecision_4)                                     // =="0.0000"==
    L10N_STRING(DecimalPrecision_5)                                     // =="0.00000"==
    L10N_STRING(DecimalPrecision_6)                                     // =="0.000000"==
    L10N_STRING(DecimalPrecision_7)                                     // =="0.0000000"==
    L10N_STRING(DecimalPrecision_8)                                     // =="0.00000000"==
    L10N_STRING(DecimalPrecision_9)                                     // =="0.000000000"==
    L10N_STRING(DecimalPrecision_10)                                    // =="0.0000000000"==
    L10N_STRING(DecimalPrecision_11)                                    // =="0.00000000000"==
    L10N_STRING(DecimalPrecision_12)                                    // =="0.000000000000"==

    L10N_STRING(FractionalPrecision_Whole)                               // =="Whole"==
    L10N_STRING(FractionalPrecision_Half)                                // =="1/2"==
    L10N_STRING(FractionalPrecision_Quarter)                             // =="1/4"==
    L10N_STRING(FractionalPrecision_Eighth)                              // =="1/8"==
    L10N_STRING(FractionalPrecision_Sixteenth)                           // =="1/16"==
    L10N_STRING(FractionalPrecision_Over_32)                             // =="1/32"==
    L10N_STRING(FractionalPrecision_Over_64)                             // =="1/64"==
    L10N_STRING(FractionalPrecision_Over_128)                            // =="1/128"==
    L10N_STRING(FractionalPrecision_Over_256)                            // =="1/256"==

    L10N_STRING(ShowSignOption_NoSign)                                   // =="Never"==
    L10N_STRING(ShowSignOption_OnlyNegative)                             // =="Only Negative Values"==
    L10N_STRING(ShowSignOption_SignAlways)                               // =="All Values"==
    L10N_STRING(ShowSignOption_NegativeParentheses)                      // =="Use Parentheses for Negative Values"==

    L10N_STRING(PresentationType_Decimal)                                // =="Decimal"==
    L10N_STRING(PresentationType_Fractional)                             // =="Fractional"==
    L10N_STRING(PresentationType_Scientific)                             // =="Scientific"==
    L10N_STRING(PresentationType_ScientificNorm)                         // =="Normalized Scientific"==
    L10N_STRING(PresentationType_Stop100)                                // =="Stationing (Imperial)"==
    L10N_STRING(PresentationType_Stop1000)                               // =="Stationing (Metric)"==

BENTLEY_TRANSLATABLE_STRINGS_END

//m_displayLabel = BeSQLite::L10N::GetString(UnitsL10N::GetNameSpace(), BeSQLite::L10N::StringId(stringId.c_str()));

#define UNITSL10N_GETSTRING(K)  BeSQLite::L10N::GetString(UnitsL10N::GetNameSpace(), UnitsL10N::K())

END_BENTLEY_UNITS_NAMESPACE
