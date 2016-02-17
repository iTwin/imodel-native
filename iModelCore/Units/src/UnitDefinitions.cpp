/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitDefinitions.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "StandardNames.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultUnits ()
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    //reg.AddUnit(sysName, phName, unitName, definition, factor, offset);
    reg.AddUnit(LENGTH, SI, "M", "M"); // BISQPrimUom)->AddSynonym("METRE");
    reg.AddUnit(LENGTH, SI, "MM", "[MILLI]*M");// BISQSecUom)->AddSynonym("MILLIMETRE");
    reg.AddUnit(LENGTH, SI, "CM", "[CENTI]*M");// BISQSecUom)->AddSynonym("CENTIMETRE");
    reg.AddUnit(LENGTH, SI, "DM", "[DECI]*M");// BISQSecUom)->AddSynonym("DECIMETRE");
    reg.AddUnit(LENGTH, SI, "KM", "[KILO]*M");// BISQSecUom)->AddSynonym("KILOMETRE");
    reg.AddUnit(LENGTH, SI, "MU", "[MICRO]*M");// BISQFactOne, BISQZeroE10, BISQNoDescript, BISQSecUom)->AddSynonyms("MICRON", "MICROMETRE");
    reg.AddUnit(LENGTH, SI, "ANGSTROM", "M");// BISQFactOne, -10.0, BISQNoDescript, BISQSecUom);
    reg.AddUnit(LENGTH, SI, "FERMI", "[FEMTO]*M");// BISQSecUom)->AddSynonym("FEMTOMETRE");
    reg.AddUnit(LENGTH, IMPERIAL, "IN", "MM", 25.4);// , BISQSecUom)->AddSynonym("INCH");
    reg.AddUnit(LENGTH, IMPERIAL, "FT", "IN", 12.0);// , BISQSecUom)->AddSynonym("FOOT");
    reg.AddUnit(LENGTH, USCUSTOM, "MILLIINCH", "[MILLI]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "MICROINCH", "[MICRO]*IN");// BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILLIFOOT", "[MILLI]*FT");// BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "YRD", "FT", 3.0);// , BISQSecUom)->AddSynonym("YARD");
    reg.AddUnit(LENGTH, SURVEYOR, "CHAIN", "FT", 66.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "MILE", "YRD", 1760.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, IMPERIAL, "NAUT_MILE", "M", 1852.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_INCH", "CM", 10000.0 / 3937.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_FOOT", "US_SURVEY_INCH", 12.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_YARD", "US_SURVEY_FOOT", 3.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, USCUSTOM, "US_SURVEY_MILE", "US_SURVEY_YARD", 1760.0);// , BISQSecUom);

    reg.AddUnit(LENGTH, IMPERIAL, "BARLEYCORN", "IN", (1.0 / 3.0));// , BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "CUBIT", "IN", 18.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "ELL", "IN", 45.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, HISTORICAL, "FATHOM", "FT", 6.0);// , BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_SEC", "[C]*SEC");// , BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_MIN", "[C]*MIN");// , BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_HOUR", "[C]*HR");// , BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "LIGHT_YEAR", "[C]*YR");// , BISQSecUom);
    reg.AddUnit(LENGTH, ASTRONOMY, "AU", "M", 1.495978707, 11.0);// , BISQNoDescript, BISQSecUom);
    }