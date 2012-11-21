/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/ECSchemaUnitsManagerTests.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "StopWatch.h"
#include "TestFixture.h"
#include <ECUnits/Units.h>
#include "UnitsTestBase.h"

using namespace Bentley::ECN;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*====================================================================================**/
/// <summary>Tests for Bentley.Units and Bentley.ECObjects.Units.</summary>
/// <author>Colin.Kerr</author>                             <date>3/2008</date>
/*==============+===============+===============+===============+===============+======*/
//[TestFixture]
 class ECSchemaUnitsManagerTests : public UnitsTestBase
{
public: ECSchemaUnitsManagerTests()
	{}


//public: void TearDownTestCase()
//	{}

};

TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaWhereSupplementalSchemaOverridesDomainSchemaAtPropertyLevel)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_trainingWheelDiameterProp));
        VerifyDefaultUnit (L"MILLIFOOT", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaOverrideOfBaseClass)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_wcWeightProp));
        VerifyDefaultUnit (L"POUND", defaultUnit);
    }
TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyDirectlyFromPropertyOverridesSupplementalSchema)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_seatPostAngleProp));
        VerifyDefaultUnit (L"POUND", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyDirectlyFromProperty)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_AreaProp));
        VerifyDefaultUnit (L"INCH_SQUARED", defaultUnit);
    }


TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromClassUsingKOQFromProperty)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_spokeLengthProp));
        VerifyDefaultUnit (L"CENTIMETRE", defaultUnit);
    }


TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromDomainSchemaUsingKOQFromPropertyOnBaseClass)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_wcDiameterProp));
        VerifyDefaultUnit (L"CENTIMETRE", defaultUnit);
    }


TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromProperty)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_headSetAngleProp));
        VerifyDefaultUnit (L"RADIAN", defaultUnit);
    }


TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromDomainSchema)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_rearWheelDiameterProp));
        VerifyDefaultUnit (L"CENTIMETRE", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnitForPropertyFromSupplementalSchemaUsingKOQFromSupplementalSchema)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_trainingWheelDiameterProp));
        VerifyDefaultUnit (L"MILLIFOOT", defaultUnit);
    }



TEST_F (ECSchemaUnitsManagerTests, GetUnitFromExternalSchemaUsingKOQFromProperty)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_frontWheelPressureProp));
        VerifyDefaultUnit (L"ATMOSPHERE", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnitFromDomainSchemaOverridingExternalSchemaUsingKOQFromProperty)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_rearWheelPressureProp));
        VerifyDefaultUnit (L"FOOT_OF_H2O_CONVENTIONAL", defaultUnit);
    }

/////////////////////////////


TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults1)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucAreaProp));
        VerifyDefaultUnit (L"FOOT_SQUARED", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults2)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucVolumeProp));
        VerifyDefaultUnit (L"FOOT_CUBED", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromUnitSystemDefaults3)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucTemperatureProp));
        VerifyDefaultUnit (L"DEGREE_FAHRENHEIT", defaultUnit);
    }

TEST_F (ECSchemaUnitsManagerTests, GetUnit_FromKOQ_OverriddingUnitSystemDefaults)
    {
		Unit defaultUnit;
		EXPECT_TRUE (Unit::GetUnitForECProperty (defaultUnit, *m_sucWidthProp));
        VerifyDefaultUnit (L"MILLIMETRE", defaultUnit);
    }





END_BENTLEY_ECOBJECT_NAMESPACE