/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/Unit.h $
|    $RCSfile: Unit.h,v $
|   $Revision: 1.2 $
|       $Date: 2006/02/14 15:15:03 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define Femtometers     _TEXT("Femtometers")
#define Picometers      _TEXT("Picometers")
#define Nanometers      _TEXT("Nanometers")
#define Micrometers     _TEXT("Micrometers")
#define Millimeters     _TEXT("Millimeters")
#define Centimeters     _TEXT("Centimeters")
#define Decimeters      _TEXT("Decimeters")
#define Meters          _TEXT("Meters")
#define Dekameters      _TEXT("Dekameters")
#define Hectometers     _TEXT("Hectometers")
#define Kilometers      _TEXT("Kilometers")
#define Megameters      _TEXT("Megameters")
#define Gigameters      _TEXT("Gigameters")
#define Terameters      _TEXT("Terameters")
#define Petameters      _TEXT("Petameters")
#define MicroInches     _TEXT("MicroInches")
#define Mils            _TEXT("Mils")
#define Points          _TEXT("Points")
#define Picas           _TEXT("Picas")
#define Inches          _TEXT("Inches")
#define Feet            _TEXT("Feet")
#define Yards           _TEXT("Yards")
#define Miles           _TEXT("Miles")
#define SurveyFeet      _TEXT("Survey Feet")
#define Fathoms         _TEXT("Fathoms")
#define Rods            _TEXT("Rods")
#define Chains          _TEXT("Chains")
#define Furlongs        _TEXT("Furlongs")
#define SurveyMiles     _TEXT("Survey Miles")
#define Angstroms       _TEXT("Angstroms")
#define NauticalMiles   _TEXT("Nautical Miles")
#define AstromomicalUnits      _TEXT("Astromomical Units")
#define LightYears      _TEXT("Light Years")
#define Parsecs         _TEXT("Parsecs")

class Unit
    {


    public:

        Unit(void)
            {
            }

        ~Unit(void)
            {
            }

        void SetInfo(const TCHAR* pLabel, double numerator, double denominator)
                        {_tcscpy(m_label, pLabel);
                         m_numerator = numerator;
                         m_denominator = denominator;};
        const TCHAR* GetLabel ()
                        {return m_label;};
        double   GetNumerator ()
                        {return m_numerator;};
        double   GetDenominator ()
                        {return m_denominator;};
    
    private:
    TCHAR m_label[256];
    double  m_numerator;
    double  m_denominator;

    };
