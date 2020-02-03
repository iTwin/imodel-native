/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/UnitComboBox.cpp $
|    $RCSfile: UnitComboBox.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2006/02/20 16:00:20 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include ".\unitcombobox.h"

#define LAST_METRIC 14
#define LAST_ENGLISH 28

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
UnitComboBox::UnitComboBox(void)
    {                    //Label                //Numerator           //Denominator
    m_Units[0].SetInfo  (Femtometers,   1000000000000000.0,                   1.0);
    m_Units[1].SetInfo  (Picometers,       1000000000000.0,                   1.0);
    m_Units[2].SetInfo  (Nanometers,          1000000000.0,                   1.0);
    m_Units[3].SetInfo  (Micrometers,            1000000.0,                   1.0);
    m_Units[4].SetInfo  (Millimeters,               1000.0,                   1.0);
    m_Units[5].SetInfo  (Centimeters,                100.0,                   1.0);
    m_Units[6].SetInfo  (Decimeters,                  10.0,                   1.0);
    m_Units[7].SetInfo  (Meters,                       1.0,                   1.0);
    m_Units[8].SetInfo  (Dekameters,                   1.0,                  10.0);
    m_Units[9].SetInfo  (Hectometers,                  1.0,                 100.0);
    m_Units[10].SetInfo (Kilometers,                   1.0,                1000.0);
    m_Units[11].SetInfo (Megameters,                   1.0,             1000000.0);
    m_Units[12].SetInfo (Gigameters,                   1.0,          1000000000.0);
    m_Units[13].SetInfo (Terameters,                   1.0,       1000000000000.0);
    m_Units[14].SetInfo (Petameters,                   1.0,    1000000000000000.0);
    // LAST_METRIC
    m_Units[15].SetInfo (MicroInches,        10000000000.0,                 254.0);  
    m_Units[16].SetInfo (Mils,                  10000000.0,                 254.0);  
    m_Units[17].SetInfo (Points,                  720000.0,                 254.0);   
    m_Units[18].SetInfo (Picas,                    60000.0,                 254.0);   
    m_Units[19].SetInfo (Inches,                   10000.0,                 254.0);    
    m_Units[20].SetInfo (Feet,                     10000.0,                3048.0);   
    m_Units[21].SetInfo (Yards,                    10000.0,                9144.0);   
    m_Units[22].SetInfo (Miles,                    10000.0,            16093440.0);   
    m_Units[23].SetInfo (SurveyFeet,               39370.0,               12000.0); 
    m_Units[24].SetInfo (Fathoms,                  39370.0,               72000.0);   
    m_Units[25].SetInfo (Rods,                     39370.0,              198000.0);   
    m_Units[26].SetInfo (Chains,                   39370.0,              792000.0);   
    m_Units[27].SetInfo (Furlongs,                 39370.0,             7920000.0);   
    m_Units[28].SetInfo (SurveyMiles,              39370.0,           633660000.0); 
    //LAST_ENGLISH
    m_Units[29].SetInfo (Angstroms,          10000000000.0,                   1.0);   
    m_Units[30].SetInfo (NauticalMiles,                1.0,                1852.0);   
    m_Units[31].SetInfo (AstromomicalUnits,            1.0,        149597900000.0);
    m_Units[32].SetInfo (LightYears,                   1.0,    9460730000000000.0);   
    m_Units[33].SetInfo (Parsecs,                      1.0,   30856780000000000.0);  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
UnitComboBox::~UnitComboBox(void)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitComboBox::Fill (const TCHAR* pDefaultUnit)
    {
    SetSeparator(LAST_METRIC);
    SetSeparator(LAST_ENGLISH);

    SetSepLineStyle(PS_SOLID);
    SetSepLineColor(0);
    SetHorizontalMargin(1);   


    // Now fill the combox box
    for (int i = 0; i < NUMBERUNITS; ++i)
        {
        AddString (m_Units[i].GetLabel ());
        }

    if (pDefaultUnit)
        {
        SelectString (-1, pDefaultUnit);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  2/2006
+---------------+---------------+---------------+---------------+---------------+------*/
Unit* const UnitComboBox::GetUnit ()
    {
    return &m_Units[GetCurSel ()];
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(UnitComboBox, SepComboBox)
    //{{AFX_MSG_MAP(UnitComboBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()