
#include <Imagepp/all/h/HFCUnit.h>

#include <Imagepp/all/h/HFCStandardUnits.h>

#include <Imagepp/all/h/HFCStandardQuantities.h>


int TestQuantities();


typedef HFCInverseUnit<HFCDistanceUnit> HFCInvDistanceUnit;

HFC_DEFINE_QUANTITY_FROM_INVERSE_UNIT(InversedDistwww, HFCInvDistanceUnit);

int main(int argc, char** argv)
    {
    // Definition of custom base unit type
    typedef HFCBaseUnit<HFCBaseUnitType<1> > HFCAppleUnit;
    typedef HFCBaseUnit<HFCBaseUnitType<2> > HFCRaisinUnit;

    // Definition of multiplied homogenous units
    typedef HFCMultipliedUnit<HFCTimeUnit, HFCTimeUnit> HFCTimeSquaredUnit;
    typedef HFCMultipliedUnit<HFCTemperatureUnit, HFCTemperatureUnit> HFCTemperatureSquaredUnit;

    // Definition of multiplied heterogenous units
    typedef HFCMultipliedUnit<HFCAmountOfSubstanceUnit, HFCTimeUnit> HFCDesintegrationUnit;

    // Definition of divided units
    typedef HFCDividedUnit<HFCAppleUnit, HFCTimeUnit> HFCAppleFallSpeed;
    typedef HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> HFCAngularSpeed;


    // Definition of inverse unit
    typedef HFCInverseUnit<HFCDistanceUnit> HFCInverseOfDistanceUnit;



    // Declaration of base units
    HFCDistanceUnit             ADistUnit;
    HFCTimeUnit                 ATimeUnit;
    HFCElectricCurrentUnit      ACurrentUnit;
    HFCAngularUnit              AnQAngularUnit;
    HFCMassUnit                 AMassUnit;
    HFCAmountOfSubstanceUnit    AnAmountUnit;
    HFCLuminousIntensityUnit    ALuminousIntensityUnit;
    HFCTemperatureUnit          ATemperatureUnit;
    HFCSolidAngularUnit         ASolidAngularUnit;
    HFCAppleUnit                AnAppleUnit;
    HFCRaisinUnit               ARaisinUnit;

    // Definition of base unit (other constructors)
    HFCDistanceUnit MyDistUnit1;
    HFCDistanceUnit MyDistUnit2(3.0);
    HFCDistanceUnit MyDistUnit3(0.3048, MyDistUnit1);
    HFCDistanceUnit MyDistUnit4(MyDistUnit2);

    HFCTimeUnit MyTimeUnit1;
    HFCTimeUnit MyTimeUnit2(60);
    HFCTimeUnit MyTimeUnit3(3600, MyTimeUnit1);
    HFCTimeUnit MyTimeUnit4(MyTimeUnit2);

    HFCElectricCurrentUnit MyElectricalCurrentUnit1;
    HFCElectricCurrentUnit MyElectricalCurrentUnit2(3.0);
    HFCElectricCurrentUnit MyElectricalCurrentUnit3(0.3048, MyElectricalCurrentUnit1);
    HFCElectricCurrentUnit MyElectricalCurrentUnit4(MyElectricalCurrentUnit2);

    HFCAngularUnit MyAngularUnit1;
    HFCAngularUnit MyAngularUnit2(3.0);
    HFCAngularUnit MyAngularUnit3(0.3048, MyAngularUnit1);
    HFCAngularUnit MyAngularUnit4(MyAngularUnit2);

    HFCTemperatureUnit MyTemperatureUnit1;
    HFCTemperatureUnit MyTemperatureUnit2(3.0);
    HFCTemperatureUnit MyTemperatureUnit3(0.3048, MyTemperatureUnit1);
    HFCTemperatureUnit MyTemperatureUnit4(MyTemperatureUnit2);

    HFCAmountOfSubstanceUnit MyAmountOfSubstanceUnit1;
    HFCAmountOfSubstanceUnit MyAmountOfSubstanceUnit2(3.0);
    HFCAmountOfSubstanceUnit MyAmountOfSubstanceUnit3(0.3048, MyAmountOfSubstanceUnit1);
    HFCAmountOfSubstanceUnit MyAmountOfSubstanceUnit4(MyAmountOfSubstanceUnit2);

    HFCLuminousIntensityUnit MyLuminousIntensityUnit1;
    HFCLuminousIntensityUnit MyLuminousIntensityUnit2(3.0);
    HFCLuminousIntensityUnit MyLuminousIntensityUnit3(0.3048, MyLuminousIntensityUnit1);
    HFCLuminousIntensityUnit MyLuminousIntensityUnit4(MyLuminousIntensityUnit2);



    // Definition of a multiplied unit
    HFCAreaUnit     MyAreaUnit1;
    HFCTimeSquaredUnit MyTimeSquaredUnit1;

    // Definition of a multiplied unit from two unit of same type
    HFCAreaUnit     MyAreaUnit2(MyDistUnit1 * MyDistUnit2);

    // More complex definition of area
    HFCAreaUnit     MyAreaUnit3((MyDistUnit1 * MyDistUnit1 * MyDistUnit1) / MyDistUnit2);
    HFCAreaUnit     MyAreaUnit4(MyAreaUnit1 * (MyDistUnit1 / MyDistUnit2));
    HFCAreaUnit     MyAreaUnit5(((MyLuminousIntensityUnit1 * MyDistUnit1) /
                                 (MyLuminousIntensityUnit2)) * MyDistUnit3);
    HFCAreaUnit     MyAreaUnit6(MyDistUnit3 * ((MyLuminousIntensityUnit1 * MyDistUnit1) /
                                               (MyLuminousIntensityUnit2)));
//    HFCAreaUnit     MyAreaUnit7(MyDistUnit3 * ((MyDistUnit1 * MyLuminousIntensityUnit1) /
//                                (MyLuminousIntensityUnit2)));

    // Definition of an inverse unit
    HFCFrequencyUnit  MyFrequencyUnit1;

    // Definition of a divided unit
    HFCAngularAccelerationUnit  MyAngularAccelerationUnit1;

    // Systematic check of grammar rules
    // Rule G1
    HFCInverseUnit<HFCDistanceUnit>  Toto1;
    HFCPureNumber Toto2(MyDistUnit1 * Toto1);

    HFCInverseUnit<HFCAreaUnit>  Toto3;
    HFCPureNumber Toto4(MyAreaUnit1 * Toto3);


    // Rule G2
    HFCPureNumber Toto5(Toto1 * MyDistUnit1);

    HFCPureNumber Toto6(Toto3 * MyAreaUnit1);


    // Rule G3
    HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit>  Toto7;
// PROBLEM Ambiguous Probably (G3 with 2.6)
// SOLVED THROUGH CAST
    HFCDistanceUnit Toto8(MyAngularUnit1 * Toto7);

    HFCDividedUnit<HFCTemperatureUnit, HFCAreaUnit>  Toto9;
// PROBLEM Ambiguous Probably (G3 with 4.6)
// SOLVED THROUGH CAST
    HFCTemperatureUnit Toto10(MyAreaUnit1 * Toto9);


    // Rule G4
    HFCDistanceUnit Toto8A(Toto7 * MyAngularUnit1);
    HFCTemperatureUnit Toto10A(Toto9 * MyAreaUnit1);

    // Rule G5
    HFCDistanceUnit Toto11(MyAreaUnit1 / MyDistUnit1);

    // Rule G6
    HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit>  Toto12;
    HFCInverseUnit<HFCAngularUnit> Toto13(Toto12 / MyDistUnit1);


    // Rule 1.1
    HFCPureNumber   MyPureNumber;
    HFCInverseUnit<HFCDistanceUnit>  Toto14(MyPureNumber / MyDistUnit1);
    // Rule 1.2
    HFCInverseUnit<HFCAreaUnit>  Toto15(MyPureNumber / MyAreaUnit1);
    // Rule 1.3
    HFCDistanceUnit Toto16(MyPureNumber / Toto14);
    // Rule 1.4
    HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> Toto17(MyPureNumber / MyAngularAccelerationUnit1);


    // Rule 2.1
    HFCMultipliedUnit<HFCDistanceUnit, HFCAppleUnit> Toto18(MyDistUnit1 * AnAppleUnit);
    // Rule 2.2
    HFCInverseUnit<HFCDistanceUnit> Toto19;
    HFCPureNumber Toto20(MyDistUnit1 * Toto19);
    // Rule 2.3
    HFCFrequencyUnit  Toto21;
    HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> Toto22(MyDistUnit1 * MyFrequencyUnit1);
    // Rule 2.4
    HFCMultipliedUnit<HFCDistanceUnit, HFCAreaUnit> Toto23(MyDistUnit1 * MyAreaUnit1);

    // Rule 2.5
// PROBLEM Conflict G3 or 2.6 with 2.5
// SOLVED THROUGH CAST
    HFCAngularUnit Toto24(MyDistUnit1 * MyAngularAccelerationUnit1);

    // Rule 2.6
    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCAngularUnit>, HFCDistanceUnit> Toto25(MyAngularUnit1 * MyAngularAccelerationUnit1);

    // Rule 2.7
    HFCPureNumber Toto26(MyDistUnit1 / MyDistUnit2);

    // Rule 2.8
    HFCAngularAccelerationUnit Toto27(MyAngularUnit1 / MyDistUnit1);

    // Rule 2.9
    HFCTimeSquaredUnit Toto28(MyTimeUnit1 / MyFrequencyUnit1);

    // Rule 2.10
// PROBLEM rule 2.10 with 2.12 ???
    HFCInverseUnit<HFCDistanceUnit> Toto29(MyDistUnit1 / MyAreaUnit1);

    // Rule 2.11 Not implemented since it conflicts with 2.10

    // Rule 2.12
    HFCDividedUnit<HFCDistanceUnit, HFCTimeSquaredUnit> Toto31(MyDistUnit1 / MyTimeSquaredUnit1);

    // Rule 2.13
// PROBLEM Conflict 2.13 with 2.14
    HFCDistanceUnit Toto32(MyAngularUnit1 / MyAngularAccelerationUnit1);

    // Rule 2.14
    HFCDividedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit>, HFCAngularUnit> Toto33(MyTimeUnit1 / MyAngularAccelerationUnit1);

    // Rule 3.1
    HFCSpeedUnit    Toto34(MyFrequencyUnit1 * MyDistUnit1);

    // Rule 3.2
    HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTimeUnit> > Toto35(MyFrequencyUnit1 * MyFrequencyUnit1);

    // Rule 3.3
    HFCInverseUnit<HFCAngularUnit> Toto36;
// Problem ... conflict with 3.4
// SOLVED THROUGH CAST
    HFCInverseUnit<HFCDistanceUnit> Toto37(Toto36 * MyAngularAccelerationUnit1);

    // Rule 3.4
    HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> > Toto38(MyFrequencyUnit1 * MyAngularAccelerationUnit1);

    // Rule 3.5
    HFCInverseUnit<HFCDistanceUnit> Toto39;
// PROBLEM ... conflict 3.5 with 3.7
    HFCDistanceUnit Toto40(Toto39 * MyAreaUnit1);


    // Rule 3.6 not implemented due to conflicts with 3.5

    // Rule 3.7
    HFCDividedUnit<HFCAreaUnit, HFCTimeUnit> Toto41(MyFrequencyUnit1 * MyAreaUnit1);


    // Rule 3.8
    HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> > Toto42(MyFrequencyUnit1 / MyDistUnit1);

    // Rule 3.9
    HFCPureNumber Toto43(MyFrequencyUnit1 / MyFrequencyUnit1);


    // Rule 3.10
    HFCInverseUnit<HFCDistanceUnit> Toto44;
    HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> Toto45(MyFrequencyUnit1 / Toto44);

    // Rule 3.11
    HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCAreaUnit> > Toto46(MyFrequencyUnit1 / MyAreaUnit1);

    // Rule 3.12
    HFCInverseUnit<HFCDistanceUnit> Toto47;
// Problem ... conflict with 3.13
// SOLVED THROUGH CAST
    HFCInverseUnit<HFCAngularUnit> Toto48(Toto47 / MyAngularAccelerationUnit1);

    //Rule 3.13
    HFCDividedUnit<HFCDistanceUnit, HFCMultipliedUnit<HFCTimeUnit, HFCAngularUnit> > Toto49(MyFrequencyUnit1 / MyAngularAccelerationUnit1);

    // Rule 4.1
    HFCMultipliedUnit<HFCAreaUnit, HFCTimeUnit> Toto50(MyAreaUnit1 * MyTimeUnit1);

    // Rule 4.2
    HFCInverseUnit<HFCDistanceUnit> Toto51;
    HFCDistanceUnit Toto52(MyAreaUnit1 * Toto51);

    // Rule 4.3 not implemented since conflicts with 4.2

    // Rule 4.4
    HFCDividedUnit<HFCAreaUnit, HFCTimeUnit> Toto53(MyAreaUnit1 * MyFrequencyUnit1);

    // Rule 4,5
    HFCMultipliedUnit<HFCAreaUnit, HFCAreaUnit> Toto54(MyAreaUnit1 * MyAreaUnit2);

    // Rule 4.6
// Problem ... conflict with 4.8
//  UNABLE TO SOLVE THROUGH CAST
//    HFCMultipliedUnit<HFCDistanceUnit, HFCAngularUnit> Toto55(MyAreaUnit1 * MyAngularAccelerationUnit1);

    // Rule 4.7 ... not implemented ... conflicts with 4.6

    // Rule 4.8
    HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto56;
    HFCDividedUnit<HFCMultipliedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit>, HFCAngularUnit>, HFCDistanceUnit> toto57(Toto56 * MyAngularAccelerationUnit1);

    // Rule 4.9
    HFCMultipliedUnit<HFCAreaUnit, HFCTimeUnit> Toto58(MyAreaUnit1 / MyFrequencyUnit1);

    // Rule 4.10
    HFCPureNumber Toto59(MyAreaUnit1 / MyAreaUnit2);

    // Rule 4.11
    HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> Toto60;
// PROBLEM OPERATION NOT DEFINED conflict wit 4.13 and 4.12
//    HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> Toto61(MyAreaUnit1 / Toto60);

    // Rule 4.12
    HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> Toto62;
// PROBLEM OPERATION NOT DEFINED conflict wit 4.13 and 4.11
//    HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> Toto63(MyAreaUnit1 / Toto62);

    // Rule 4.13
    HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto64;
    HFCDividedUnit<HFCAreaUnit, HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto65(MyAreaUnit1 / Toto64);

    // Rule 4.14
    HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit> Toto66;
// PROBLEM ... Conflict probably with 4.16
//    HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> Toto67(Toto66 / MyAngularAccelerationUnit1);

    // Rule 4.15
    HFCMultipliedUnit<HFCTimeUnit, HFCAngularUnit> Toto68;
// PROBLEM ... Result inapropriate due to 4.16
//    HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> Toto69(Toto68 / MyAngularAccelerationUnit1);

    // Rule 4.16
    HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto70;
    HFCDividedUnit<HFCMultipliedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit>, HFCDistanceUnit>, HFCAngularUnit> Toto71(Toto70 / MyAngularAccelerationUnit1);

    // Rule 5.1
    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>, HFCDistanceUnit> Toto72(MyAngularAccelerationUnit1 * MyTimeUnit1);

    // Rule 5.2
    HFCInverseUnit<HFCAngularUnit> Toto73;
    HFCInverseUnit<HFCDistanceUnit> Toto74(MyAngularAccelerationUnit1 * Toto73);

    // Rule 5.3
    HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto75(MyAngularAccelerationUnit1 * MyFrequencyUnit1);

    // Rule 5.4
// PROBLEM conflict with 5.6
//    HFCMultipliedUnit<HFCAngularUnit, HFCDistanceUnit> Toto76(MyAngularAccelerationUnit1 * MyAreaUnit1);

    // Rule 5.5 not implemented due to conflict with 5.4

    // Rule 5.6
    HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto77;
    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> >, HFCDistanceUnit> Toto78(MyAngularAccelerationUnit1 * Toto77);

    // Rule 5.7
    HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> Toto79;
    HFCPureNumber Toto80(MyAngularAccelerationUnit1 * Toto79);

    // Rule 5.8
    HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> Toto81;
// PROBLEM ...
//    HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> Toto82(MyAngularAccelerationUnit1 * Toto81);

    // Rule 5.9
    HFCDividedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto83;
// PROBLEM ... operator not defined
//    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>,HFCMultipliedUnit<HFCDistanceUnit, HFCTemperatureUnit> > Toto84(MyAngularAccelerationUnit1 * Toto83);

    // Rule 5.10
    HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto85(MyAngularAccelerationUnit1 / MyTimeUnit1);

    // Rule 5.11
    HFCInverseUnit<HFCDistanceUnit> Toto86;
    HFCAngularUnit Toto87(MyAngularAccelerationUnit1 / Toto86);

    // Rule 5.12
    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>, HFCDistanceUnit> Toto88(MyAngularAccelerationUnit1 / MyFrequencyUnit1);

    // Rule 5.13
    HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit> Toto89;
// PROBLEM Conflict with 5.15
//    HFCInverseUnit<HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto90(MyAngularAccelerationUnit1 / Toto89);

    // Rule 5.14 Not implemented due to conflict with 5.13

    // Rule 5.15
    HFCMultipliedUnit<HFCTemperatureUnit, HFCTimeUnit> Toto91;
    HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCMultipliedUnit<HFCTemperatureUnit, HFCTimeUnit> > > Toto92(MyAngularAccelerationUnit1 / Toto91);

    // Rule 5.16
    HFCPureNumber Toto93(MyAngularAccelerationUnit1 / MyAngularAccelerationUnit1);

    // Rule 5.17
    HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> Toto94;
//    HFCDividedUnit<HFCTimeUnit, HFCDistanceUnit> Toto95(MyAngularAccelerationUnit1 / Toto94);

    // Rule 5.18
    HFCDividedUnit<HFCTimeUnit, HFCDistanceUnit> Toto96;
// PROBLEM ... Opertation not defined
//    HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> Toto97(MyAngularAccelerationUnit1 / Toto96);


    // Rule 5.19
    HFCDividedUnit<HFCTimeUnit, HFCTemperatureUnit> Toto98;
// Problem ... no operation defined
    HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTemperatureUnit>,
                   HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto99(MyAngularAccelerationUnit1 / Toto98);


    TestQuantities();

    return 0;
    }



int TestQuantities()
    {
    // Definition of custom base unit type
    typedef HFCBaseUnit<HFCBaseUnitType<1> > HFCAppleUnit;
    typedef HFCBaseUnit<HFCBaseUnitType<2> > HFCRaisinUnit;
    typedef HFCMultipliedUnit<HFCTimeUnit, HFCTimeUnit> HFCTimeSquaredUnit;

    // Define distance units
    HFCDistanceUnit   Meter;
    HFCDistanceUnit   Foot(0.3048, Meter);

    // Define distances
    HFCDistance Dist1(2.4, Meter);
    HFCDistance Dist2(2.3, Foot);

    // Test distance operations
    HFCDistance Dist3 = Dist1 + Dist2;
    HFCDistance Dist4 = Dist1 - Dist2;
    HFCDistance Dist5 = -Dist1;
    Dist5 -= Dist1;
    Dist5 += Dist1;
    HFCDistance Dist8 = 3 * Dist1;
    HFCDistance Dist9 = Dist1 * 3;
    HFCDistance Dist10 = Dist1 / 3;
    HFCDistance Dist11 = Dist1;
    Dist11 *= 3;
    Dist11 /= 4;
    double Tata = Dist1 / Dist2;

    // Test operation with related quantity
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > InvDist1(3.4);

    double TestDouble1 = Dist11 * InvDist1;

    // Define AreaUnit
    HFCAreaUnit MeterSquared;
    HFCAreaUnit FootSquared(Foot * Foot);

    // Define Area
    HFCArea Area1(12.3, HFCAreaUnit(1.0));
    HFCArea Area2(12.3, HFCAreaUnit(1.0));

    // Test area operations
    HFCArea Area3 = Area1 + Area2;
    HFCArea Area4 = Area1 - Area2;
    HFCArea Area5 = -Area1;
    Area5 -= Area1;
    Area5 += Area1;
    HFCArea Area8 = 3 * Area1;
    HFCArea Area9 = Area1 * 3;
    HFCArea Area10 = Area1 / 3;
    HFCArea Area11 = Area1;
    Area11 *= 3;
    Area11 /= 4;
    double Tota1 = Area1 / Area2;

    // Test operation with related quantity
    HFCQuantity<HFCInverseUnit<HFCAreaUnit> > InvArea1(3.4);

    double TestDouble2 = Area11 * InvArea1;

    // Test OLD Rules
    // Area divided by distance
    HFCDistance TestDist1 = Area1 / Dist1;
    // Square root of area
    HFCDistance TestDist2 = sqrt(Area1);
    // Creation of area by multiplication
    HFCArea     Area12 = Dist1 * Dist1;


    // Define Angle
    HFCAngle Angle1(12.3, HFCAngularUnit(1.0));
    HFCAngle Angle2(12.3, HFCAngularUnit(1.0));

    // Test Angle operations
    HFCAngle Angle3 = Angle1 + Angle2;
    HFCAngle Angle4 = Angle1 - Angle2;
    HFCAngle Angle5 = -Angle1;
    Angle5 -= Angle1;
    Angle5 += Angle1;
    HFCAngle Angle8 = 3 * Angle1;
    HFCAngle Angle9 = Angle1 * 3;
    HFCAngle Angle10 = Angle1 / 3;
    HFCAngle Angle11 = Angle1;
    Angle11 *= 3;
    Angle11 /= 4;
    double Tota3 = Angle1 / Angle2;


    // Test operation with related quantity
    HFCQuantity<HFCInverseUnit<HFCAngularUnit> > InvAngle1(3.4);

    double TestDouble3 = Angle11 * InvAngle1;

    // Define Angular acceleration
    HFCAngularAcceleration AngleAcc1(12.3, HFCAngularAccelerationUnit(1.0));
    HFCAngularAcceleration AngleAcc2(12.3, HFCAngularAccelerationUnit(1.0));

    // Test AngleAcc operations
    HFCAngularAcceleration AngleAcc3 = AngleAcc1 + AngleAcc2;
    HFCAngularAcceleration AngleAcc4 = AngleAcc1 - AngleAcc2;
    HFCAngularAcceleration AngleAcc5 = -AngleAcc1;
    AngleAcc5 -= AngleAcc1;
    AngleAcc5 += AngleAcc1;
    HFCAngularAcceleration AngleAcc8 = 3 * AngleAcc1;
    HFCAngularAcceleration AngleAcc9 = AngleAcc1 * 3;
    HFCAngularAcceleration AngleAcc10 = AngleAcc1 / 3;
    HFCAngularAcceleration AngleAcc11 = AngleAcc1;
    AngleAcc11 *= 3;
    AngleAcc11 /= 4;
    double Tota4 = AngleAcc1 / AngleAcc2;

    // Test OLD Rules
    // Test creation by division
    HFCAngle AnAngle2;
    HFCDistance ADist2;
    HFCAngularAcceleration AnAngleAcc12 = AnAngle2 / ADist2;

    // Test operation with related quantity
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> >InvAngleAcc1(3.4);

    // Even with specific Definition of operator ... this does not work properly
    double TestDouble4 = AngleAcc11 * InvAngleAcc1;


    // Divide angular acceleration by angle
    HFCAngle   AnAngle1;
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Tota14 = AngleAcc11 / AnAngle1;

    // Multiply angular acceleration by distance
    // Old rule
    HFCDistance ADist1;
    HFCAngle Tota15 = AngleAcc11 * ADist1;


    //==================================================================
    // At this point, all OLD defintions are working properly
    //==================================================================

    // Check rules systematicaly


    HFCQuantity<HFCAppleUnit>   AnApple;
    HFCQuantity<HFCRaisinUnit>  ARaisin;

    // Definition of base unit (other constructors)
    HFCQuantity<HFCDistanceUnit> MyDist1;
    HFCQuantity<HFCDistanceUnit> MyDist2(3.0);
    HFCQuantity<HFCDistanceUnit> MyDist3(0.3048 * MyDist1);
    HFCQuantity<HFCDistanceUnit> MyDist4(MyDist2);

    HFCQuantity<HFCTimeUnit> MyTime1;
    HFCQuantity<HFCTimeUnit> MyTime2(60);
    HFCQuantity<HFCTimeUnit> MyTime3(3600 * MyTime1);
    HFCQuantity<HFCTimeUnit> MyTime4(MyTime2);

    HFCQuantity<HFCElectricCurrentUnit> MyElectricalCurrent1;
    HFCQuantity<HFCElectricCurrentUnit> MyElectricalCurrent2(3.0);
    HFCQuantity<HFCElectricCurrentUnit> MyElectricalCurrent3(0.3048 * MyElectricalCurrent1);
    HFCQuantity<HFCElectricCurrentUnit> MyElectricalCurrent4(MyElectricalCurrent2);

    HFCQuantity<HFCAngularUnit> MyAngle1;
    HFCQuantity<HFCAngularUnit> MyAngle2(3.0);
    HFCQuantity<HFCAngularUnit> MyAngle3(0.3048 * MyAngle1);
    HFCQuantity<HFCAngularUnit> MyAngle4(MyAngle2);

    HFCQuantity<HFCTemperatureUnit> MyTemperature1;
    HFCQuantity<HFCTemperatureUnit> MyTemperature2(3.0);
    HFCQuantity<HFCTemperatureUnit> MyTemperature3(0.3048 * MyTemperature1);
    HFCQuantity<HFCTemperatureUnit> MyTemperature4(MyTemperature2);

    HFCQuantity<HFCAmountOfSubstanceUnit> MyAmountOfSubstance1;
    HFCQuantity<HFCAmountOfSubstanceUnit> MyAmountOfSubstance2(3.0);
    HFCQuantity<HFCAmountOfSubstanceUnit> MyAmountOfSubstance3(0.3048 * MyAmountOfSubstance1);
    HFCQuantity<HFCAmountOfSubstanceUnit> MyAmountOfSubstance4(MyAmountOfSubstance2);

    HFCQuantity<HFCLuminousIntensityUnit> MyLuminousIntensity1;
    HFCQuantity<HFCLuminousIntensityUnit> MyLuminousIntensity2(3.0);
    HFCQuantity<HFCLuminousIntensityUnit> MyLuminousIntensity3(0.3048 * MyLuminousIntensity1);
    HFCQuantity<HFCLuminousIntensityUnit> MyLuminousIntensity4(MyLuminousIntensity2);



    // Definition of a multiplied unit
    HFCQuantity<HFCAreaUnit>     MyArea1;
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCTimeUnit> > MyTimeSquared1;

    // Definition of a multiplied unit from two unit of same type
    HFCQuantity<HFCAreaUnit>     MyArea2(MyDist1 * MyDist2);

    // More complex definition of area
    HFCQuantity<HFCAreaUnit>     MyArea3((MyDist1 * MyDist1 * MyDist1) / MyDist2);
    HFCQuantity<HFCAreaUnit>     MyArea4(MyArea1 * (MyDist1 / MyDist2));
    HFCQuantity<HFCAreaUnit>     MyArea5(((MyLuminousIntensity1 * MyDist1) /
                                          (MyLuminousIntensity2)) * MyDist3);
    HFCQuantity<HFCAreaUnit>     MyArea6(MyDist3 * ((MyLuminousIntensity1 * MyDist1) /
                                                    (MyLuminousIntensity2)));

    // Definition of an inverse unit
    HFCQuantity<HFCFrequencyUnit>  MyFrequency1;

    // Definition of a divided unit
    HFCQuantity<HFCAngularAccelerationUnit>  MyAngularAcceleration1;


    // Systematic check of grammar rules
    // Rule G1
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> >  Toto1;
    double Toto2(MyDist1 * Toto1);

    HFCQuantity<HFCInverseUnit<HFCAreaUnit> > Toto3;
    double Toto4(MyArea1 * Toto3);


    // Rule G2
    double Toto5(Toto1 * MyDist1);

    double Toto6(Toto3 * MyArea1);


    // Rule G3
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> >  Toto7;
    HFCDistance Toto8(MyAngle1 * Toto7);

    HFCQuantity<HFCDividedUnit<HFCTemperatureUnit, HFCAreaUnit> >  Toto9;
    HFCQuantity<HFCTemperatureUnit> Toto10(MyArea1 * Toto9);


    // Rule G4
    HFCQuantity<HFCDistanceUnit> Toto8A(Toto7 * MyAngle1);
    HFCQuantity<HFCTemperatureUnit> Toto10A(Toto9 * MyArea1);

    // Rule G5
    HFCQuantity<HFCDistanceUnit> Toto11(MyArea1 / MyDist1);

    // Rule G6
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> >  Toto12;
    HFCQuantity<HFCInverseUnit<HFCAngularUnit> > Toto13(Toto12 / MyDist1);


    // Rule 1.1
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> >  Toto14(3.0 / MyDist1);
    // Rule 1.2
    HFCQuantity<HFCInverseUnit<HFCAreaUnit> >  Toto15(3.0 / MyArea1);
    // Rule 1.3
    HFCQuantity<HFCDistanceUnit> Toto16(3.0 / Toto14);
    // Rule 1.4
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> > Toto17(3.0 / MyAngularAcceleration1);


    // Rule 2.1
    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCAppleUnit> > Toto18(MyDist1 * AnApple);
    // Rule 2.2
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto19;
    double Toto20(MyDist1 * Toto19);
    // Rule 2.3
    HFCQuantity<HFCFrequencyUnit>  Toto21;
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto22(MyDist1 * MyFrequency1);
    // Rule 2.4
    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCAreaUnit> > Toto23(MyDist1 * MyArea1);

    // Rule 2.5
    HFCQuantity<HFCAngularUnit> Toto24(MyDist1 * MyAngularAcceleration1);

    // Rule 2.6
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCAngularUnit>, HFCDistanceUnit> > Toto25(MyAngle1 * MyAngularAcceleration1);

    // Rule 2.7
    double Toto26(MyDist1 / MyDist2);

    // Rule 2.8
    HFCQuantity<HFCAngularAccelerationUnit> Toto27(MyAngle1 / MyDist1);

    // Rule 2.9
    HFCQuantity<HFCTimeSquaredUnit> Toto28(MyTime1 / MyFrequency1);

    // Rule 2.10
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto29(MyDist1 / MyArea1);

    // Rule 2.11 Not implemented since it conflicts with 2.10

    // Rule 2.12
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeSquaredUnit> > Toto31(MyDist1 / MyTimeSquared1);

    // Rule 2.13
    HFCQuantity<HFCDistanceUnit> Toto32(MyAngle1 / MyAngularAcceleration1);

    // Rule 2.14
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit>, HFCAngularUnit> > Toto33(MyTime1 / MyAngularAcceleration1);

    // Rule 3.1
    HFCQuantity<HFCSpeedUnit>    Toto34(MyFrequency1 * MyDist1);

    // Rule 3.2
    HFCQuantity<HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTimeUnit> > > Toto35(MyFrequency1 * MyFrequency1);

    // Rule 3.3
    HFCQuantity<HFCInverseUnit<HFCAngularUnit> > Toto36;
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto37(Toto36 * MyAngularAcceleration1);

    // Rule 3.4
    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> > > Toto38(MyFrequency1 * MyAngularAcceleration1);

    // Rule 3.5
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto39;
    HFCQuantity<HFCDistanceUnit> Toto40(Toto39 * MyArea1);


    // Rule 3.6 not implemented due to conflicts with 3.5

    // Rule 3.7
    HFCQuantity<HFCDividedUnit<HFCAreaUnit, HFCTimeUnit> > Toto41(MyFrequency1 * MyArea1);


    // Rule 3.8
    HFCQuantity<HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> > > Toto42(MyFrequency1 / MyDist1);

    // Rule 3.9
    double Toto43(MyFrequency1 / MyFrequency1);


    // Rule 3.10
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto44;
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto45(MyFrequency1 / Toto44);

    // Rule 3.11
    HFCQuantity<HFCInverseUnit<HFCMultipliedUnit<HFCTimeUnit, HFCAreaUnit> > > Toto46(MyFrequency1 / MyArea1);

    // Rule 3.12
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto47;
    HFCQuantity<HFCInverseUnit<HFCAngularUnit> > Toto48(Toto47 / MyAngularAcceleration1);

    //Rule 3.13
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCMultipliedUnit<HFCTimeUnit, HFCAngularUnit> > > Toto49(MyFrequency1 / MyAngularAcceleration1);

    // Rule 4.1
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCAreaUnit> > Toto50(MyArea1 * MyTime1);

    // Rule 4.2
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto51;
    HFCQuantity<HFCDistanceUnit> Toto52(MyArea1 * Toto51);

    // Rule 4.3 not implemented since conflicts with 4.2

    // Rule 4.4
    HFCQuantity<HFCDividedUnit<HFCAreaUnit, HFCTimeUnit> > Toto53(MyArea1 * MyFrequency1);

    // Rule 4,5
    HFCQuantity<HFCMultipliedUnit<HFCAreaUnit, HFCAreaUnit> > Toto54(MyArea1 * MyArea2);

    // Rule 4.6
// Problem ... conflict with 4.8
//  UNABLE TO SOLVE THROUGH CAST
//    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCAngularUnit> > Toto55(MyArea1 * MyAngularAcceleration1);

    // Rule 4.7 ... not implemented ... conflicts with 4.6

    // Rule 4.8
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto56;
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit>, HFCAngularUnit>, HFCDistanceUnit> > toto57(Toto56 * MyAngularAcceleration1);

    // Rule 4.9
    HFCQuantity<HFCMultipliedUnit<HFCAreaUnit, HFCTimeUnit> > Toto58(MyArea1 / MyFrequency1);

    // Rule 4.10
    double Toto59(MyArea1 / MyArea2);

    // Rule 4.11
    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto60;
// PROBLEM OPERATION NOT DEFINED conflict wit 4.13 and 4.12
//    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto61(MyArea1 / Toto60);

    // Rule 4.12
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCDistanceUnit> > Toto62;
// PROBLEM OPERATION NOT DEFINED conflict wit 4.13 and 4.11
//    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto63(MyArea1 / Toto62);

    // Rule 4.13
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto64;
    HFCQuantity<HFCDividedUnit<HFCAreaUnit, HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > > Toto65(MyArea1 / Toto64);

    // Rule 4.14
    HFCQuantity<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit> > Toto66;
// PROBLEM ... Conflict probably with 4.16
//    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto67(Toto66 / MyAngularAcceleration1);

    // Rule 4.15
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCAngularUnit> > Toto68;
// PROBLEM ... Result inapropriate due to 4.16
//    HFCQuantity<HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto69(Toto68 / MyAngularAcceleration1);

    // Rule 4.16
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto70;
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit>, HFCDistanceUnit>, HFCAngularUnit> > Toto71(Toto70 / MyAngularAcceleration1);

    // Rule 5.1
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>, HFCDistanceUnit> > Toto72(MyAngularAcceleration1 * MyTime1);

    // Rule 5.2
    HFCQuantity<HFCInverseUnit<HFCAngularUnit> > Toto73;
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto74(MyAngularAcceleration1 * Toto73);

    // Rule 5.3
    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > > Toto75(MyAngularAcceleration1 * MyFrequency1);

    // Rule 5.4
// PROBLEM conflict with 5.6
//    HFCQuantity<HFCMultipliedUnit<HFCAngularUnit, HFCDistanceUnit> > Toto76(MyAngularAcceleration1 * MyArea1);

    // Rule 5.5 not implemented due to conflict with 5.4

    // Rule 5.6
    HFCQuantity<HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto77;
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCTimeUnit, HFCTemperatureUnit> >, HFCDistanceUnit> > Toto78(MyAngularAcceleration1 * Toto77);

    // Rule 5.7
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCAngularUnit> > Toto79;
    double Toto80(MyAngularAcceleration1 * Toto79);

    // Rule 5.8
    HFCQuantity<HFCDividedUnit<HFCDistanceUnit, HFCTimeUnit> > Toto81;
// PROBLEM ...
//    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> > Toto82(MyAngularAcceleration1 * Toto81);

    // Rule 5.9
    HFCQuantity<HFCDividedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto83;
// PROBLEM ... operator not defined
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>,HFCMultipliedUnit<HFCDistanceUnit, HFCTemperatureUnit> > > Toto84(MyAngularAcceleration1 * Toto83);

    // Rule 5.10
    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > > Toto85(MyAngularAcceleration1 / MyTime1);

    // Rule 5.11
    HFCQuantity<HFCInverseUnit<HFCDistanceUnit> > Toto86;
    HFCQuantity<HFCAngularUnit> Toto87(MyAngularAcceleration1 / Toto86);

    // Rule 5.12
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit>, HFCDistanceUnit> > Toto88(MyAngularAcceleration1 / MyFrequency1);

    // Rule 5.13
    HFCQuantity<HFCMultipliedUnit<HFCAngularUnit, HFCTimeUnit> > Toto89;
// PROBLEM Conflict with 5.15
//    HFCQuantity<HFCInverseUnit<HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > > Toto90(MyAngularAcceleration1 / Toto89);

    // Rule 5.14 Not implemented due to conflict with 5.13

    // Rule 5.15
    HFCQuantity<HFCMultipliedUnit<HFCTemperatureUnit, HFCTimeUnit> > Toto91;
    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCMultipliedUnit<HFCDistanceUnit, HFCMultipliedUnit<HFCTemperatureUnit, HFCTimeUnit> > > > Toto92(MyAngularAcceleration1 / Toto91);

    // Rule 5.16
    double Toto93(MyAngularAcceleration1 / MyAngularAcceleration1);

    // Rule 5.17
    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> > Toto94;
//    HFCQuantity<HFCDividedUnit<HFCTimeUnit, HFCDistanceUnit> > Toto95(MyAngularAcceleration1 / Toto94);

    // Rule 5.18
    HFCQuantity<HFCDividedUnit<HFCTimeUnit, HFCDistanceUnit> > Toto96;
// PROBLEM ... Opertation not defined
//    HFCQuantity<HFCDividedUnit<HFCAngularUnit, HFCTimeUnit> > Toto97(MyAngularAcceleration1 / Toto96);


    // Rule 5.19
    HFCQuantity<HFCDividedUnit<HFCTimeUnit, HFCTemperatureUnit> > Toto98;
// Problem ... no operation defined
    HFCQuantity<HFCDividedUnit<HFCMultipliedUnit<HFCAngularUnit, HFCTemperatureUnit>,
                HFCMultipliedUnit<HFCDistanceUnit, HFCTimeUnit> > > Toto99(MyAngularAcceleration1 / Toto98);




    //==================================================================
    // At this most rules work properly
    //==================================================================

    // Define speed
    HFCSpeed Speed1(12.3, HFCSpeedUnit(1.0));
    HFCSpeed Speed2(12.3, HFCSpeedUnit(1.0));

    // Test Speed operations
    HFCSpeed Speed3 = Speed1 + Speed2;
    HFCSpeed Speed4 = Speed1 - Speed2;
    HFCSpeed Speed5 = -Speed1;
    Speed5 -= Speed1;
    Speed5 += Speed1;
    HFCSpeed Speed8 = 3 * Speed1;
    HFCSpeed Speed9 = Speed1 * 3;
    HFCSpeed Speed10 = Speed1 / 3;
    HFCSpeed Speed11 = Speed1;
    Speed11 *= 3;
    Speed11 /= 4;
    double Toto116 = Speed1 / Speed2;

    // Test creation by division
    HFCTime ATime2;
    HFCDistance ADist3;
    HFCSpeed ASpeed12 = ADist3 / ATime2;

    // Test operation with related quantity
    HFCQuantity<HFCDividedUnit<HFCTimeUnit, HFCDistanceUnit> >InvSpeed1(3.4);

    // Even with specific Definition of operator ... this does not work properly
    double TestDouble5 = Speed11 * InvSpeed1;

    // Divide speed by distance
    HFCDistance   ADist4;
    HFCQuantity<HFCInverseUnit<HFCTimeUnit> > Toto117 = Speed11 / ADist4;

    // Multiply speed by time
    // Old rule
    HFCTime ATime1;
    HFCDistance Toto118 = Speed11 * ATime1;

    return (0);
    }

