/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/ECSchemaUnitsManagerTests.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECUnits/Units.h>
#include "UnitsTestBase.h"

using namespace BentleyApi::ECN;


BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*====================================================================================**/
/// <summary>Tests for Bentley.Units and Bentley.ECObjects.Units.</summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
 class ECSchemaUnitsManagerTests : public UnitsTestBase
{
public:
    ECSchemaUnitsManagerTests() { }
};
 
 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaWhereSupplementalSchemaOverridesDomainSchemaAtPropertyLevel)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_trainingWheelDiameterProp));
    VerifyDefaultUnit ("MILLIFOOT", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaOverrideOfBaseClass)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_wcWeightProp));
    VerifyDefaultUnit ("POUND", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyDirectlyFromPropertyOverridesSupplementalSchema)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_seatPostAngleProp));
    VerifyDefaultUnit ("ANGLE_SECOND", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyDirectlyFromProperty)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_AreaProp));
    VerifyDefaultUnit ("INCH_SQUARED", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromClassUsingKOQFromProperty)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_spokeLengthProp));
    VerifyDefaultUnit ("CENTIMETRE", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromDomainSchemaUsingKOQFromPropertyOnBaseClass)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_wcDiameterProp));
    VerifyDefaultUnit ("CENTIMETRE", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromProperty)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_headSetAngleProp));
    VerifyDefaultUnit ("RADIAN", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromDomainSchema)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_rearWheelDiameterProp));
    VerifyDefaultUnit ("CENTIMETRE", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromSupplementalSchema)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_trainingWheelDiameterProp));
    VerifyDefaultUnit ("MILLIFOOT", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitFromExternalSchemaUsingKOQFromProperty)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_frontWheelPressureProp));
    VerifyDefaultUnit ("ATMOSPHERE", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnitFromDomainSchemaOverridingExternalSchemaUsingKOQFromProperty)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_rearWheelPressureProp));
    VerifyDefaultUnit ("FOOT_OF_H2O_CONVENTIONAL", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults1)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucAreaProp));
    VerifyDefaultUnit ("FOOT_SQUARED", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults2)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucVolumeProp));
    VerifyDefaultUnit ("FOOT_CUBED", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults3)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucTemperatureProp));
    VerifyDefaultUnit ("DEGREE_FAHRENHEIT", defaultUnit);
    }

 /*====================================================================================**/
/// <summary></summary>
/// <author>Muhammad.Zaighum</author>                             <date>12/12</date>
/*==============+===============+===============+===============+===============+======*/
TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromKOQ_OverriddingUnitSystemDefaults)
    {
    Unit defaultUnit;
    EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucWidthProp));
    VerifyDefaultUnit ("MILLIMETRE", defaultUnit);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
