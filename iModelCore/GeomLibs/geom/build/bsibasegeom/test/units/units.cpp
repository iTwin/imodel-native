//
// test1.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
//#include <msgeomstructs.h>
//#include <printfuncs.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static double sKILOGRAM_per_POUNDMASS = 0.45359237;   // This is exact !!!
static double sMETER_per_INCH = 0.0254;         // This is exact !!!
static double sMETER_per_FOOT = 0.3048;         // 12 * sMETER_per_INCH, but explicit constant gets last-bit-match with string conversions ?

    // http://www1.bipm.org/en/CGPM/db/3/2/
    // http://en.wikipedia.org/wiki/Pound-force
static double sNEWTON_per_POUNDFORCE = 4.4482216152605;
static double sGRAVITY_SI = 9.80665;



int sNumDefaultOK    = 0;
int sNumRelaxedTolOK  = 0;
int sNumFail  = 0;
int sNumSkip  = 0;
int sNumTiny  = 0;
int sNumExact = 0;

static int sOutputFormat = 0;

void ShowCount (char *pName, int count)
    {
    printf ("<%s>%d</%s>\n", pName, count, pName);
    }

void PrintTag (char *pName, char *pString = 0)
    {
    if (pString != 0)
        printf ("\n        <%s>%s</%s>", pName, pString, pName);
    else
        printf ("\n        <%s/>", pName);
    }

void PrintTag (char *pName, double value)
    {
    printf ("\n        <%s>%.17lg</%s>", pName, value, pName);
    }


// Output function -- pack everything on a single line for text search.
void DoUnit_quick
(
char *pDimension,
char *pUnit,
double toRefNumerator,
double toRefDenominator,
double oldFromRef,
double relTol = 0,
char *pRemark = 0
)
    {
    static double sDefaultRelTol = 1.0e-6;

    if (relTol > 0)
        {
        }
    else
        {
        relTol = sDefaultRelTol;
        }

    if (toRefNumerator <= 0.0)
        {
        sNumSkip++;
        printf("<SKIP><Dim>%s</Dim><Unit>%s</Unit> <Old>%.17lg</Old>", pDimension, pUnit, oldFromRef);
        if (pRemark)
            printf ("<Remark>%s</Remark>", pRemark);
        printf ("</SKIP>\n");
        }
    else
        {
        double fromRef = toRefDenominator / toRefNumerator;
        double relerr = fabs (1.0 - fromRef / oldFromRef);
        printf ("<FACTOR>");
        if (relerr == 0.0)
            {
            sNumExact++;
            }
        else if (relerr > relTol)
            {
            sNumFail++;
            printf ("<RELERR>%.3le</RELERR>", relerr);
            }
        else if (relTol == sDefaultRelTol)
            {
            sNumDefaultOK++;
            }
        else
            {
            sNumRelaxedTolOK++;
            }

        printf ("<Dim>%s</Dim><Unit>%s</Unit> <FromRef>%.17lg</FromRef><Old>%.17lg</Old>",
                    pDimension, pUnit, fromRef, oldFromRef);
        if (pRemark)
            printf ("<Remark>%s</Remark>", pRemark);
        printf ("</FACTOR>\n");
        }
    }

// Output function -- verbose multi-line xml element.
void DoUnit_as_Unit_Attributes
(
char *pDimension,
char *pUnit,
double toRefNumerator,
double toRefDenominator,
double oldFromRef,
double relTolIn = 0,
char *pRemark = 0
)
    {
    static double sDefaultRelTol = 1.0e-6;
    static double sTinyTol = 1.0e-15;
    double relTol = sDefaultRelTol;
    if (relTolIn > 0)
        {
        relTol = relTolIn;
        }

    if (toRefNumerator <= 0.0)
        {
        sNumSkip++;
        printf("<Unit_Attributes_NoChange>");
        PrintTag ("Name", pUnit);
        PrintTag ("Dimension", pDimension);
        PrintTag ("OldConversionFactor", oldFromRef);
        if (pRemark)
            PrintTag ("Remark", pRemark);
        printf ("\n<Unit_Attributes_NoChange>\n");
        }
    else
        {
        double fromRef = toRefDenominator / toRefNumerator;
        double relerr = 1.0 - ( oldFromRef > fromRef ? fromRef/ oldFromRef : oldFromRef / fromRef);

        printf("<Unit_Attributes_WithChange>");
        PrintTag ("Name", pUnit);
        PrintTag ("Dimension", pDimension);
        PrintTag ("OldConversionFactor", oldFromRef);
        PrintTag ("NewConversionFactor", fromRef);

        if (relerr == 0)
            {
            PrintTag ("PASS_EXACT");
            sNumExact++;
            }
        else if (relerr < sTinyTol)
            {
            PrintTag ("PASS_TINY_TOL");
            sNumTiny++;
            }
        else if (relerr > relTol)
            {
            PrintTag ("FAIL");
            sNumFail++;
            }
        else if (relTolIn <= 0)
            {
            PrintTag ("PASS_defaultTol", relTol);
            sNumDefaultOK++;
            }
        else
            {
            PrintTag ("PASS_specialTol", relTol);
            sNumRelaxedTolOK++;
            }
        if (relerr != 0.0)
            PrintTag ("RelErr", relerr);
        if (pRemark)
            PrintTag ("Remark", pRemark);
        printf ("\n</Unit_Attributes_WithChange>\n");
        }
    }

void DoUnit
(
char *pDimension,
char *pUnit,
double toRefNumerator,
double toRefDenominator,
double oldFromRef,
double relTol = 0,
char *pRemark = 0
)
    {
    if (sOutputFormat == 1)
        DoUnit_as_Unit_Attributes (pDimension, pUnit, toRefNumerator, toRefDenominator, oldFromRef, relTol, pRemark);
    else
        DoUnit_quick (pDimension, pUnit, toRefNumerator, toRefDenominator, oldFromRef, relTol, pRemark);
    }





void DoMassRates ()
    {
    // mass rates use milligram as mass unit ...

    double MILLIGRAM = 1.0;
    double GRAM      = 1000 * MILLIGRAM;
    double KILOGRAM  = 1000 * GRAM;
    double MICROGRAM = 0.001 * MILLIGRAM;
    double invMICROGRAM = 1000;
    double sec = 1;
    double min = 60 * sec;
    double hr  = 60 * min;
    double day = 24 * hr;

    double MILLION = 1e6;
    DoUnit("MASS_RATE", "GRAM_PER_DAY", GRAM, day, 86.4);
    DoUnit("MASS_RATE", "GRAM_PER_HOUR", GRAM,  hr, 3.6);
    DoUnit("MASS_RATE", "GRAM_PER_MINUTE", GRAM,  min, 0.06);
    DoUnit("MASS_RATE", "GRAM_PER_SECOND", GRAM,  sec, 0.001);
    DoUnit("MASS_RATE", "KILOGRAM_PER_DAY", KILOGRAM, day, 0.0864);
    DoUnit("MASS_RATE", "KILOGRAM_PER_HOUR", KILOGRAM, hr, 0.0036);
    DoUnit("MASS_RATE", "KILOGRAM_PER_MINUTE", KILOGRAM, min, 6E-05);
    DoUnit("MASS_RATE", "KILOGRAM_PER_SECOND", KILOGRAM, sec, 1E-06);

    DoUnit("MASS_RATE", "MICROGRAM_PER_DAY", 1, invMICROGRAM * day, 86400000);
    DoUnit("MASS_RATE", "MICROGRAM_PER_HOUR", 1, invMICROGRAM * hr, 3600000);
    DoUnit("MASS_RATE", "MICROGRAM_PER_MINUTE", 1, invMICROGRAM * min, 60000);
    DoUnit("MASS_RATE", "MICROGRAM_PER_SECOND", 1, invMICROGRAM * sec, 1000);

    DoUnit("MASS_RATE", "MILLIGRAM_PER_DAY", MILLIGRAM, day, 86400);
    DoUnit("MASS_RATE", "MILLIGRAM_PER_HOUR", MILLIGRAM, hr, 3600);
    DoUnit("MASS_RATE", "MILLIGRAM_PER_MINUTE", MILLIGRAM, min, 60);
    DoUnit("MASS_RATE", "MILLIGRAM_PER_SECOND", MILLIGRAM, sec, 1);

    DoUnit("MASS_RATE", "POUND_PER_DAY",    sKILOGRAM_per_POUNDMASS * MILLION, day, 0.1904793926);
    DoUnit("MASS_RATE", "POUND_PER_HOUR",   sKILOGRAM_per_POUNDMASS * MILLION, hr, 0.00793664136);
    DoUnit("MASS_RATE", "POUND_PER_MINUTE", sKILOGRAM_per_POUNDMASS * MILLION, min, 0.000132277356);
    DoUnit("MASS_RATE", "POUND_PER_SECOND", sKILOGRAM_per_POUNDMASS * MILLION, sec, 2.2046226E-06);
    }

void DoForce ()
    {
    char comment[] = "Is double 8 in Haastead 4.448822 a typo?";
    DoUnit("FORCE", "KILONEWTON", 1000 , sNEWTON_per_POUNDFORCE, 0.0044488222, 2e-4, comment);
    DoUnit("FORCE", "KILOPOUND_FORCE", 1000, 1, 0.001);
    // 4.4488222 has doubled 8 -- as found in original Haastead data.
    DoUnit("FORCE", "NEWTON", 1, sNEWTON_per_POUNDFORCE, 4.4488222, 2e-4, comment);
    DoUnit("FORCE", "POUND_FORCE", 1, 1, 1);
    }

void DoPressure ()
    {
    double IN = sMETER_per_INCH;
    double IN2 = IN * IN;
    double FT = sMETER_per_FOOT;
    double FT2 = FT * FT;
    double ATM_per_KILOPASCAL = 9.8692326671601283000246730816679e-3;
    double KILOPASCAL_per_ATM = 101.325;
    // http://physics.nist.gov/cgi-bin/cuu/Value?stdatm|search_for=atmosphere
    // http://en.wikipedia.org/wiki/Atmospheric_pressure
    // standard atmosphere = 101325 P (!Exact)
    // inverts to 0.0000098692326671601278

    DoUnit("PRESSURE", "ATMOSPHERE", KILOPASCAL_per_ATM, 1.0, 0.00986923267, 0, "This is exact");
    //DoUnit("PRESSURE", "ATMOSPHERE", -1, ATM_per_KILOPASCAL, 0.00986923267, 0, "Need atm/KP factor");
    DoUnit("PRESSURE", "BAR", 100, 1, 0.01);
    DoUnit("PRESSURE", "FOOT_OF_H2O", -1, 1, 0.33455255, 0, "Need H2O pressure data");
    // 101.972 appears commonly as pressure due to a metre of water.
    // OK, weighing water accounts for slightly-away-from-one.  But why are the pure KG/M^2 numbers not 1?
    // (and why do these not have sec^2?  Jargon for meter of water?
    // http://en.wikipedia.org/wiki/Pressure
    // The following correspond to what the wiki entry calls a "technical atmosphere"
    DoUnit("PRESSURE", "KILOGRAM_PER_CENTIMETE_SQUARED", -1, 1, 0.01019716, 0, "Need H2O pressure constant");
    DoUnit("PRESSURE", "KILOGRAM_PER_METRE_SQUARED", -1, 1, 101.971621, 0, "Need H2O pressure constant");
    DoUnit("PRESSURE", "KILOPASCAL", 1, 1, 1);
    DoUnit("PRESSURE", "METER_OF_H2O", -1, 1, 0.101972, 0, "Need H2O pressure constant");
    DoUnit("PRESSURE", "MILLIMETER_OF_H2O", -1, 1, 101.972, 0, "Need H2O pressure constant");
    DoUnit("PRESSURE", "NEWTON_PER_METRE_SQUARED", 1, 1000, 1000);
    DoUnit("PRESSURE", "POUND_PER_SQUARE_FOOT", sNEWTON_per_POUNDFORCE, FT2 * 1000, 20.8854, 2e-6, "Consistent Newtons/pf");
    DoUnit("PRESSURE", "POUND_PER_SQUARE_INCH", sNEWTON_per_POUNDFORCE, IN2 * 1000, 0.145037738);
    DoUnit("PRESSURE", "PSI", sNEWTON_per_POUNDFORCE, IN2 * 1000, 0.145037738);
    }

void doTime ()
    {
    double sec_per_hr = 3600;
    double min_per_hr = 60;
    double THOUSAND = 1000;
    DoUnit("TIME", "DAY", 24, 1, 0.0416666666666667);
    DoUnit("TIME", "HOUR", 1, 1, 1);
    DoUnit("TIME", "MILLISECOND", 1, sec_per_hr * THOUSAND, 3600000);
    DoUnit("TIME", "MINUTE", 1, min_per_hr, 60);
    DoUnit("TIME", "SECOND", 1, sec_per_hr, 3600);
    DoUnit("TIME", "YEAR", 365 * 24, 1, 0.000114155251141553);
    }

int main(int argc, char * argv[])
    {
    for (int i = 0; i < argc; i++)
        {
        if (0 == strcmp (argv[i], "verbose"))
            sOutputFormat = 1;
        }

    double M  = 1.0;
    double IN = sMETER_per_INCH;
    double FT = sMETER_per_FOOT;
    double YD = 36.0 * IN;
    double MI = 5280.0 * FT;
    double CM = 0.01 * M;
    double KM = 1000 * M;
    double MM = 0.001 * M;
    double invCM = 100;

    double IN2 = IN * IN;
    double CM2 = CM * CM;
    double M2  = M * M;
    double FT2 = FT * FT;
    double YD2 = YD * YD;
    double MI2 = MI * MI;
    double KM2 = KM * KM;
    double MM2 = MM * MM;
    double HECTARE = 10000 * M2;

    double IN3 = IN2 * IN;
    double CM3 = CM2 * CM;
    double M3  = M2 * M;
    double FT3 = FT2 * FT;
    double YD3 = YD2 * YD;
    double MI3 = MI2 * MI;
    double KM3 = KM2 * KM;
    double MM3 = MM2 * MM;
    double GAL = 231 * IN3;
    double LI  = 1000 * CM3;
    double invLI = 1000;
    double IMPGAL = 4.54609 * LI;

    double ACRE = 43560 * FT2;

    double THOUSAND = 1000;
    double MILLION = THOUSAND * THOUSAND;

    DoUnit("AREA", "ACRE", ACRE, 1.0, 0.000247105381);
    DoUnit("AREA", "CENTIMETRE_SQUARED", CM2, 1.0, 10000);
    DoUnit("AREA", "FOOT_SQUARED", FT2, 1.0, 10.7639104);
    DoUnit("AREA", "HECTARE", HECTARE, 1.0, 0.0001);
    DoUnit("AREA", "INCH_SQUARED", IN2, 1.0, 1550.0031);
    DoUnit("AREA", "KILOMETRE_SQUARED", KM2, 1.0, 1E-06);
    DoUnit("AREA", "METRE_SQUARED", M2, 1.0, 1);
    DoUnit("AREA", "MILE_SQUARED", MI2, 1.0, 3.86102159E-07);
    DoUnit("AREA", "MILLIMETRE_SQUARED", MM2, 1.0, 1000000);
    DoUnit("AREA", "THOUSAND_SQUARE_FEET", 1000 * FT2, 1.0, 0.01076391);
    DoUnit("AREA", "YARD_SQUARED", YD2, 1.0, 1.19599005);


    // Time in Seconds -- do not confuse with HR based raw times ...
    double sec = 1.0;
    double min = 60.0 * sec;
    double hr  = 60 * min;
    double day = 24 * hr;

    DoUnit("FLOW", "ACRE_FEET_PER_DAY", ACRE * FT, day, 70.04561962);
    DoUnit("FLOW", "ACRE_FEET_PER_HOUR", ACRE * FT, hr, 2.918567484);
    DoUnit("FLOW", "ACRE_FEET_PER_MINUTE", ACRE * FT, min, 0.0486427914);
    DoUnit("FLOW", "ACRE_INCH_PER_HOUR", ACRE * IN, hr, 35.022809808);
    DoUnit("FLOW", "ACRE_INCH_PER_MINUTE", ACRE * IN, min, 0.5837134968);
    DoUnit("FLOW", "CFM", FT3, min, 2118.88002);
    DoUnit("FLOW", "CFS", FT3, sec, 35.314667);
    DoUnit("FLOW", "CUBIC_FOOT_PER_DAY", FT3, day, 3051187.229);
    DoUnit("FLOW", "CUBIC_FOOT_PER_MINUTE", FT3, min, 2118.88002);
    DoUnit("FLOW", "CUBIC_FOOT_PER_SECOND", FT3, sec, 35.314667);
    DoUnit("FLOW", "GALLON_IMPERIAL_PER_DAY", IMPGAL, day, 19005356.16);
    DoUnit("FLOW", "GALLON_IMPERIAL_PER_MINUTE", IMPGAL, min, 13198.164);
    DoUnit("FLOW", "GALLON_IMPERIAL_PER_SECOND", IMPGAL, sec, 219.9694);
    DoUnit("FLOW", "GALLON_PER_DAY", GAL, day, 22824465.12);
    DoUnit("FLOW", "GALLON_PER_MINUTE", GAL, min, 15850.323);
    DoUnit("FLOW", "GALLON_PER_SECOND", GAL, sec, 264.17205);
    DoUnit("FLOW", "GPM", GAL, min, 15850.323);
    DoUnit("FLOW", "LITRE_PER_DAY", 1, invLI * day, 86400000);
    DoUnit("FLOW", "LITRE_PER_MINUTE", 1, invLI * min, 60000);
    DoUnit("FLOW", "LITRE_PER_SECOND", 1, invLI * sec, 1000);
    DoUnit("FLOW", "MEGA_LITRE_PER_DAY", 1e6, invLI * day, 86.4);

    DoUnit("FLOW", "METRE_CUBED_PER_DAY", M3, day, 86400);
    DoUnit("FLOW", "METRE_CUBED_PER_MINUTE", M3, min, 60);
    DoUnit("FLOW", "METRE_CUBED_PER_SECOND", M3, 1, 1);
    DoUnit("FLOW", "MGD", 1e6 * GAL, day, 22.82446512);
    DoUnit("FLOW", "MGD_IMPERIAL", 1e6 * IMPGAL, day, 19.00535616);
    DoUnit("FLOW", "MILLION_LITRE_PER_DAY", 1e6, invLI * day, 86.4);

    DoUnit("VOLUME", "ACRE_FOOT", ACRE * FT, 1.0, 0.00081071319);
    DoUnit("VOLUME", "ACRE_INCHES", ACRE * IN, 1.0, 0.009728558);
    DoUnit("VOLUME", "CENTIMETRE_CUBED", 1, 1000000, 1000000);
    DoUnit("VOLUME", "CUBIC_FOOT", FT3, 1.0, 35.314667);
    DoUnit("VOLUME", "CUBIC_INCH", IN3, 1.0, 61023.74);
    DoUnit("VOLUME", "CUBIC_YARD", YD3, 1.0, 1.3079506);
    DoUnit("VOLUME", "GALLON", GAL, 1.0, 264.17205);
    DoUnit("VOLUME", "GALLON_IMPERIAL", IMPGAL, 1.0, 219.9694);
    DoUnit("VOLUME", "LITRE", 1, invLI, 1000);
    DoUnit("VOLUME", "METRE_CUBED", M3, 1.0, 1);
    DoUnit("VOLUME", "MILLION_GALLON", 1e6 * GAL, 1.0, 0.00026417205);
    DoUnit("VOLUME", "MILLION_LITRE", 1000, 1.0, 0.001);
    DoUnit("VOLUME", "THOUSAND_GALLON", 1e3 * GAL, 1.0, 0.26417205);
    DoUnit("VOLUME", "THOUSAND_LITRE", 1000, invLI, 1);

    double atan1 = atan (1.0);
    double degreesPerRadian = 45.0 / atan (1.0);
    double radiansPerDegree  = atan (1.0) / 45.0;
    double piOver2 = atan (1.0) * 2;
    DoUnit("ANGLE", "ANGLE_MINUTES", atan1, 60.0 * 45.0, 3437.7468);
    DoUnit("ANGLE", "ANGLE_QUADRANTS", 2 * atan1, 1, 0.63661977);
    DoUnit("ANGLE", "ANGLE_SECONDS", atan1, 45 * 3600, 206264.81);
    DoUnit("ANGLE", "DEGREE", atan1, 45, 57.295779);
    DoUnit("ANGLE", "RADIAN", 1, 1, 1);
    DoUnit("ANGLE", "REVOLUTION", 8 * atan1, 1, 0.15915494);

    DoPressure ();

    DoUnit("POPULATION", "CAPITA", 1, 1, 1);
    DoUnit("POPULATION", "CUSTOMER", 1, 1, 1);
    DoUnit("POPULATION", "EMPLOYEE", 1, 1, 1);
    DoUnit("POPULATION", "GUEST", 1, 1, 1);
    DoUnit("POPULATION", "HUNDRED_CAPITA", 100, 1, 0.01);
    DoUnit("POPULATION", "PASSENGER", 1, 1, 1);
    DoUnit("POPULATION", "PERSON", 1, 1, 1);
    DoUnit("POPULATION", "RESIDENT", 1, 1, 1);
    DoUnit("POPULATION", "STUDENT", 1, 1, 1);
    DoUnit("POPULATION", "THOUSAND_CAPITA", 1000, 1, 0.001);

    DoUnit("VELOCITY", "CENTIMETER_PER_HOUR", 1, hr * invCM, 360000);
    DoUnit("VELOCITY", "CENTIMETER_PER_MINUTE", 1, min * invCM, 6000);
    DoUnit("VELOCITY", "CENTIMETER_PER_SECOND", 1, invCM, 100);
    DoUnit("VELOCITY", "FOOT_PER_HOUR", FT, hr, 11811.0234);
    DoUnit("VELOCITY", "FOOT_PER_MINUTE", FT, min, 196.85039);
    DoUnit("VELOCITY", "FOOT_PER_SECOND", FT, 1, 3.2808399);
    DoUnit("VELOCITY", "INCH_PER_HOUR", IN, hr, 141732.3);
    DoUnit("VELOCITY", "INCH_PER_MINUTE", IN, min, 2362.20474);
    DoUnit("VELOCITY", "INCH_PER_SECOND", IN, 1, 39.370079);
    DoUnit("VELOCITY", "KILOMETER_PER_HOUR", KM, hr, 3.6);

    //DoUnit("VELOCITY", "KNOT_ADMIRALTY", 1853.184 * M, hr, 1.942606763);
    DoUnit("VELOCITY", "KNOT", 6080 * FT, hr, 1.942606763, 3e-6, "Admiralty Knot 6080 FT");
    // DoUnit("VELOCITY", "KNOT_HAASTEAD", 1853.18 * M, hr, 1.942606763, 3e-6);
    // http://www.unit2unit.eu/knot_admiralty.html
    // DoUnit("VELOCITY", "KNOT_ADMIRALTY", 1853.183 * M, hr, 1.942606763, 3e-6);
    DoUnit("VELOCITY", "KNOT_INTERNATIONAL", 1852 * M, hr, 1.94384448);

    DoUnit("VELOCITY", "METRE_PER_HOUR", M, hr, 3600);
    DoUnit("VELOCITY", "METRE_PER_MINUTE", M, min, 60);
    DoUnit("VELOCITY", "METRE_PER_SECOND", M, sec, 1);
    DoUnit("VELOCITY", "MILE_PER_HOUR", 5280 * FT, hr, 2.2369363);

    DoUnit("SLOPE", "CENTIMETER_PER_METER", 1, 100, 100);
    DoUnit("SLOPE", "FOOT_PER_1000_FEET", 1, 1000, 1000);
    DoUnit("SLOPE", "FOOT_PER_MILE", 1, 5280, 5280);
    DoUnit("SLOPE", "INCH_PER_FOOT", 1, 12, 12);
    DoUnit("SLOPE", "METER_PER_KILOMETER", M, KM, 1000);
    DoUnit("SLOPE", "MILLIMETER_HORIZONTAL_PER_METER_VERTICAL", -1, 1, 100000, 0, "Why is this not 1000? implied percentage?");
    DoUnit("SLOPE", "MILLIMETER_PER_METER", 1, 1000, 1000);
    DoUnit("SLOPE", "MILLIMETER_VERTICAL_PER_METER_HORIZONTAL", 1, 1000, 1000);
    DoUnit("SLOPE", "ONE_OVER_SLOPE", 1, 1, 1);
    DoUnit("SLOPE", "PERCENT_SLOPE", 1, 100, 100);

    double invDM = 10.0;
    DoUnit("LENGTH", "CENTIMETRE", 1, invCM, 100);
    DoUnit("LENGTH", "DECIMETRE", 1, invDM, 10);
    DoUnit("LENGTH", "FOOT", FT, 1, 3.280839895);
    DoUnit("LENGTH", "INCH", IN, 1, 39.37007874);
    DoUnit("LENGTH", "KILOMETRE", KM, 1, 0.001);
    DoUnit("LENGTH", "METRE", M, 1, 1);
    DoUnit("LENGTH", "MFEET", FT, 1000, 3280.839895);
    DoUnit("LENGTH", "MILE", 5280 * FT, 1, 0.0006213711922);
    DoUnit("LENGTH", "MILLIFEET", FT, 1000, 3280.839895);
    DoUnit("LENGTH", "MILLIMETRE", M, 1000, 1000);
    DoUnit("LENGTH", "YARD", YD, 1, 1.0936133);

    DoUnit("DIFFUSIVITY", "CENTISTOKE", 1, 1e6, 1000000);
    DoUnit("DIFFUSIVITY", "FOOT_SQUARED_PER_SECOND", FT2, sec, 10.76391);
    DoUnit("DIFFUSIVITY", "METRE_SQUARED_PER_SECOND", M2, sec, 1);
    DoUnit("DIFFUSIVITY", "STOKES", 1, 1e4, 10000);




    DoUnit("FLOW_DENSITY_PER_AREA", "CFS_PER_ACRES", invLI * FT3 * day, ACRE, 0.0016540967, 4e-6, "2 incorrect digits in OldConversionFactor");
    DoUnit("FLOW_DENSITY_PER_AREA", "CFS_PER_SQUARE_FOOT", invLI * FT3 * day, FT2, 3.797265E-08);
    DoUnit("FLOW_DENSITY_PER_AREA", "CFS_PER_SQUARE_MILES", invLI * FT3 * day, MI2, 1.05861767);
    DoUnit("FLOW_DENSITY_PER_AREA", "CUBIC_METERS_PER_HECTARES_PER_DAY", invLI, HECTARE, 10);
    DoUnit("FLOW_DENSITY_PER_AREA", "CUBIC_METERS_PER_SQUARE_METER_PER_DAY", invLI, M2, 0.001);
    DoUnit("FLOW_DENSITY_PER_AREA", "GPD_PER_ACRE", invLI * GAL, ACRE, 1069.070643, 4.1e-6, "4 incorrect digits in OldConversionFactor");
    DoUnit("FLOW_DENSITY_PER_AREA", "GPD_PER_SQUARE_FEET", invLI * GAL, FT2, 0.0245423867);
    DoUnit("FLOW_DENSITY_PER_AREA", "GPD_PER_SQUARE_MILE", invLI * GAL, MI2, 684202.474691);

    DoUnit("FLOW_DENSITY_PER_AREA", "GPM_PER_ACRE", invLI * GAL * day, ACRE * min, 0.742410169, 4.1e-6, "4 incorrect digits in OldConversionFactor");
    DoUnit("FLOW_DENSITY_PER_AREA", "GPM_PER_FOOT_SQUARED", invLI * GAL * day, FT2 * min, 1.7043324E-05);
    DoUnit("FLOW_DENSITY_PER_AREA", "GPM_PER_MILE_SQUARED", invLI * GAL * day, MI2 * min, 475.1406074);

    DoUnit("FLOW_DENSITY_PER_AREA", "LITRE_PER_HECTARE_PER_DAY", 1, HECTARE, 10000);
    DoUnit("FLOW_DENSITY_PER_AREA", "LITRE_PER_KILOMETER_SQUARED_PER_DAY", 1, KM2, 1000000);
    DoUnit("FLOW_DENSITY_PER_AREA", "LITRE_PER_METER_SQUARED_PER_DAY", 1, M2, 1);
    DoUnit("FLOW_DENSITY_PER_AREA", "METERS_CUBED_PER_KILOMETER_SQUARED_PER_DAY", invLI, KM2, 1000);

    // meter to ft is exact, so we can relate teh FOOT_H2O and METER_H2O without putting
    // a number on either PSI or KPA per foot or meeter
    DoUnit("DISCHARGE_PER_PRESSURE_DROP", "CFS_PER_SQURE_ROOT_FOOT_H20", FT3, sqrt(FT), 19.5, 2e-4, "Only 3 digits in OldConversionFactor");
    DoUnit("DISCHARGE_PER_PRESSURE_DROP", "CMS_PER_SQUARE_ROOT_METER_H20", 1, 1, 1);
    // but these need the water data ...
    DoUnit("DISCHARGE_PER_PRESSURE_DROP", "GPM_PER_SQUARE_ROOT_PSI", -1, 1, 13290.37762, 0, "Need water data?");
    DoUnit("DISCHARGE_PER_PRESSURE_DROP", "lPER_SEC_PER_SQUARE_ROOT_KPA", -1, 1, 319.3305497, 0, "Need water data?");

    DoUnit("FLOW_RATE", "CUBIC_METRE_PER_HOUR", 1, 3600, 3600);

    doTime ();

    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_ACRE_FOOT", 1, ACRE * FT, 1233.50191192796, 2e-5, "OldConversionFactor converts to 0.2540013 M/in");
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_ACRE_INCH", 1, ACRE * IN, 102.789712805542, 5e-6, "OldConversionFactor converts to 0.25399963 M/in");
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_CENTIMETER_CUBED", invCM * invCM * invCM, 1, 1E-06);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_FOOT_CUBED", 1, FT3, 0.0283168463686774);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_GALLON", 1, GAL, 0.00378541181779072);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_IMPERIAL_GALLON", 1, IMPGAL, 0.00454608686480938);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_INCH_CUBED", 1, IN3, 1.63870650995826E-05);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_LITRE", invLI, 1, 0.001);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_METER_CUBED", 1, M3, 1);
    // Compare old value to DOLLAR_PER_GALLON -- extra 0 inserted??
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_MILLION_GALLON", 1, MILLION * GAL, 3785.01135503406, 2e-4, "OldConversion 0.253991 M/in");
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_MILLION_LITRE", invLI, MILLION, 1000);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_THOUSAND_GALLON", 1, THOUSAND * GAL, 3.78541110132372);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_THOUSAND_LITRE", invLI, THOUSAND, 1);
    DoUnit("COST_PER_UNIT_VOLUME", "DOLLAR_PER_YARD_CUBED", 1, YD3, 0.764554869274115);

    DoUnit("CURRENCY_PER_LENGTH", "DOLLAR_PER_FOOT", 1, FT, 0.3048);
    DoUnit("CURRENCY_PER_LENGTH", "DOLLAR_PER_METER", 1, 1, 1);

    double W_per_HORSEPOWER = 550 * sNEWTON_per_POUNDFORCE * FT;
    DoUnit("CURRENCY_PER_POWER", "DOLLAR_PER_HORSEPOWER", THOUSAND, W_per_HORSEPOWER, 0.745699872);
    DoUnit("CURRENCY_PER_POWER", "DOLLAR_PER_KILOWATT", 1, 1, 1);

    DoUnit("CURRENCY_PER_ENERGY", "DOLLAR_PER_KILOWATT_HOUR", 1, 1, 1);

    DoUnit("CURRENCY", "DOLLARS", 1, 1, 1);

    // ARG ... metric side is 100:1 as reference ...
    DoUnit("SCALE_DIMENSION", "FEET_PER_INCH", 12, 100, 8.333333333);
    DoUnit("SCALE_DIMENSION", "METER_PER_CENTIMETER", 1, 1, 1);

    DoUnit("DIAMETER_LENGTH", "FOOT_FEET", 12, 5280, 1, 440);
    DoUnit("DIAMETER_LENGTH", "FOOT_MILE", 12, 1, 0.0833333333333333);
    DoUnit("DIAMETER_LENGTH", "INCH_FEET",  1, 5280, 5280);
    DoUnit("DIAMETER_LENGTH", "INCH_METER", M, MI, 1609.344);

    DoUnit("DIAMETER_LENGTH", "INCH_MILE", 1, 1, 1);

    DoUnit("DIAMETER_LENGTH", "METER_KILOMETER", M* KM, IN * MI, 0.040877338);
    DoUnit("DIAMETER_LENGTH", "METER_METERS", M * M , IN * MI, 40.8773376);
    DoUnit("DIAMETER_LENGTH", "MILLIMETER_KILOMETER", MM * KM, IN * MI, 40.8773376);
    DoUnit("DIAMETER_LENGTH", "MILLIMETER_METER", MM * M, IN * MI, 40877.3376);
    DoUnit("DIAMETER_LENGTH", "MILLIMETER_MILE", MM, IN, 25.4);

    double JOULE = 1.0;
    double KJOULE = 1000 * JOULE;
    DoUnit("ENERGY", "FOOT_POUNDAL",    FT * sNEWTON_per_POUNDFORCE * FT, sGRAVITY_SI, 23.73, 2e-5, "Consistent Newton/pf, gravity");
    DoUnit("ENERGY", "JOULE",           JOULE, 1, 1);
    DoUnit("ENERGY", "KILOJOULE",       KJOULE, 1, 0.001);
    DoUnit("ENERGY", "KILOWATT_HOUR",   KJOULE * hr, sec, 2.77777777777778E-07);

    DoUnit("FLOW_DENSITY_PER_CAPITA", "GPD_PER_CAPITA", invLI * GAL, 1, 0.264172);
    DoUnit("FLOW_DENSITY_PER_CAPITA", "LITRE_PER_CAPITA_PER_DAY", 1, 1, 1);

    double KILOGRAM = 1;
    double GRAM = 0.001 * KILOGRAM;
    double invGRAM = 1000;
    double invMILLIGRAM = 1000 * 1000;

    DoUnit("MASS", "GRAM", 1, invGRAM, 1000);
    DoUnit("MASS", "KILOGRAM", KILOGRAM, 1, 1);
    DoUnit("MASS", "MILLIGRAM", 1, invMILLIGRAM, 1E06);
    DoUnit("MASS", "POUND", sKILOGRAM_per_POUNDMASS, 1, 2.2046226);

    DoMassRates ();

    DoUnit("ELECTRICAL_FREQUENCY", "HERTZ", 1, 1, 1);

    DoUnit("POWER", "HORSEPOWER", W_per_HORSEPOWER, 1000, 1.34124, 4e-4, "Consistent Newton / pf");
    DoUnit("POWER", "KILOWATT", 1, 1, 1);
    DoUnit("POWER", "WATT", 1, 1000, 1000);

    DoUnit("MASS_PER_AREA", "KILOGRAM_PER_HECTARE", ACRE, HECTARE * sKILOGRAM_per_POUNDMASS, 1.12085116518377);
    DoUnit("MASS_PER_AREA", "POUND_PER_ACRE", 1, 1, 1);

    double POUNDMASS_PER_SLUG = sGRAVITY_SI / FT;

    DoUnit("DENSITY", "KILOGRAM_PER_METRE_CUBED", FT3, POUNDMASS_PER_SLUG * sKILOGRAM_per_POUNDMASS, 515.3788);
    DoUnit("DENSITY", "POUND_PER_CUBIC_FOOT", 1, POUNDMASS_PER_SLUG, 32.174054);
    DoUnit("DENSITY", "SLUG_PER_CUBIC_FOOT", 1, 1, 1);

    DoForce ();

    DoUnit("ENERGY_PER_UNIT_VOLUME", "KILONEWTON_PER_FOOT_CUBED", 1, FT3, 0.0283168463686774);

    DoUnit("ENERGY_PER_UNIT_VOLUME", "KILOWATT_HOUR_PER_METRE_CUBED", 1, 1, 1);
                // Haastead has only 4 digits -- 1/0.0002642
    DoUnit("ENERGY_PER_UNIT_VOLUME", "KILOWATT_HOUR_PER_MILLION_GALLON", 1, MILLION * GAL, 3785.01135503406, 2e-4, "OldConversionFactor looks like 1/0.0002642 -- only 4 digits");
    DoUnit("ENERGY_PER_UNIT_VOLUME", "KILOWATT_HOUR_PER_MILLION_LITRE", invLI, MILLION, 1000);

    DoUnit("SPECIFIC_WEIGHT", "KILONEWTON_PER_METER_CUBED", THOUSAND, 1, 0.001);
    DoUnit("SPECIFIC_WEIGHT", "NEWTON_PER_METRE_CUBED", 1, 1, 1);
    DoUnit("SPECIFIC_WEIGHT", "POUND_FORCE_PER_CUBIC_FOOT", sNEWTON_per_POUNDFORCE, FT3, 0.006365882, 3e-5, "Consistent Newton/pf");

    DoUnit("CONCENTRATION", "MICROGRAM_PER_LITRE", 1, 1000, 1000);
    DoUnit("CONCENTRATION", "MILLIGRAM_PER_LITRE", 1, 1, 1);
    DoUnit("CONCENTRATION", "PARTS_PER_BILLION", 1, 1000, 1000);
    DoUnit("CONCENTRATION", "PARTS_PER_MILLION", 1, 1, 1);

    DoUnit("CONCENTRATION", "POUND_PER_FOOT_CUBED", sKILOGRAM_per_POUNDMASS * THOUSAND, FT3, 6.242621E-05, 3e-5, "Consistent kg/pm");
    DoUnit("CONCENTRATION", "POUND_PER_MILLION_GALLON", sKILOGRAM_per_POUNDMASS, invLI * GAL, 8.3452, 3e-5, "Consistent kg/pm");

    DoUnit("INERTIA", "NEWTON_METER_SQUARED", 1, sNEWTON_per_POUNDFORCE * FT2, 0.41325322);
    DoUnit("INERTIA", "POUND_SQUARE_FEET", 1, 1, 1);

    DoUnit("TORQUE", "NEWTON_METRE", 1, 1, 1);
    DoUnit("TORQUE", "POUND_FOOT", FT * sNEWTON_per_POUNDFORCE, 1, 0.737562);

    DoUnit("SPRING_CONSTANT", "NEWTON_PER_MILLIMETER", IN, sNEWTON_per_POUNDFORCE * MM, 0.175126797);
    DoUnit("SPRING_CONSTANT", "POUND_PER_INCH", 1, 1, 1);

    DoUnit("NONE", "NONE", 1, 1, 1);

    DoUnit("PERCENT", "PERCENT_PERCENT", 1, 1, 1);
    DoUnit("PERCENT", "UNITLESS_PERCENT", 100, 1, 0.01);

    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_ACRE", 1, ACRE, 4046.8564464278);
    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_FOOT_SQUARED", 1, FT2, 0.09290304);
    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_HECTARE", 1, HECTARE, 10000);
    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_KILOMETER_SQUARED", 1, KM2, 1000000);
    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_METER_SQUARED", 1, M2, 1);
    DoUnit("POPULATION_DENSITY_PER_AREA", "PERSON_PER_MILE_SQUARED", 1, MI2, 2589988.10055867);

    DoUnit("ROTATIONAL_FREQUENCY", "REVOLUTION_PER_MINUTE", 1, 1, 1);

    DoUnit("UNITLESS", "UNITLESS_UNIT", 1, 1, 1);

    ShowCount("numExact", sNumExact);
    ShowCount("numTiny", sNumTiny);
    ShowCount("numDefaultOK", sNumDefaultOK);
    ShowCount("numRelaxedTolOK", sNumRelaxedTolOK);
    ShowCount("numFail", sNumFail);
    ShowCount("numSkip", sNumSkip);
    }
