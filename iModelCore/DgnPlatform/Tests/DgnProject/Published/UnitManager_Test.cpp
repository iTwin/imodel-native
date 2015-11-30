/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/UnitManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

USING_NAMESPACE_BENTLEY_DGN

#define LOCALIZED_STR(str) str

char const* UNIT_ITERATOR_OPTIONS_ERROR_STRING = "Either a unit was returned that shouldn't have, or one was inserted after this test was created.";

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct UnitManagerTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UnitDefinition
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TEST_F (UnitManagerTest, GetStandardUnit)
    {
    UnitDefinition def;
    def = UnitDefinition::GetStandardUnit (StandardUnit::MetricMeters);
    
    ASSERT_TRUE (1.0 == def.GetDenominator()&& 
           1.0 == def.GetNumerator() && 
           UnitBase::Meter == def.GetBase() && 
           UnitSystem::Metric == def.GetSystem());
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DgnCore has a local lookup table to retrieve the different units. 
// Should I create my own table and compare the results? 
// Not sure because I should test all paths of execution (if possible) but then again, I'd be coupled
// with the table then. - If it were to change then I'd have to add that change to my table. (DRY)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardUnit_NULL)
    {
    UnitDefinition def;
    def = UnitDefinition::GetStandardUnit (StandardUnit::None);
    
    ASSERT_TRUE (false == def.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardUnitByName)
    {
    UnitDefinition def;
    
    WChar const* buffer = LOCALIZED_STR(L"meter");
    def = UnitDefinition::GetStandardUnitByName (buffer);
    
    ASSERT_TRUE (1.0 == def.GetDenominator()&& 
           1.0 == def.GetNumerator() && 
           UnitBase::Meter == def.GetBase() && 
           UnitSystem::Metric == def.GetSystem());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardUnitByName_None)
    {
    UnitDefinition def;
    
    WChar const* buffer = LOCALIZED_STR(L"none");
    def = UnitDefinition::GetStandardUnitByName (buffer);
    
    ASSERT_TRUE (false == def.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, IsStandardUnit)
    {
    UnitDefinition def(UnitBase::Meter, UnitSystem::Metric, 100.0, 1.0, LOCALIZED_STR(L"def"));

    ASSERT_TRUE (StandardUnit::MetricCentimeters == def.IsStandardUnit());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardLabel)
    {
    WString label = UnitDefinition::GetStandardLabel (StandardUnit::AngleGrads);

    ASSERT_TRUE (label == WString(LOCALIZED_STR(L"grad")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardName)
    {
    WString name = UnitDefinition::GetStandardName (StandardUnit::NoSystemLightYears, false);

    ASSERT_TRUE (name == WString(LOCALIZED_STR(L"Light Years")));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, GetStandardName_None)
    {
    WString name = UnitDefinition::GetStandardName (StandardUnit::Custom, false);

    ASSERT_TRUE (name == WString(L""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitManagerTest, CreateStandardUnitIterator)
    {
    UnitIteratorOptions     options;
    StandardUnitCollection  collection (options);
    
    StandardUnitCollection::Entry           entry = *collection.begin();

    UnitDefinition def = entry.GetUnitDef();
    UnitDefinition cmp = UnitDefinition::GetStandardUnit (StandardUnit::MetricFemtometers);
    
    ASSERT_TRUE (cmp.GetDenominator() == def.GetDenominator()&& 
           cmp.GetNumerator() == def.GetNumerator() && 
           cmp.GetBase() == def.GetBase() && 
           cmp.GetSystem() == def.GetSystem());
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UnitIterator
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TODO: not sure if these are good examples since they are coupled what is in the table (if stuff is added/removed these tests might fail.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitIterator, ToNext_Ascending)
    {
    ScopedDgnHost autoDgnHost;
    UnitIteratorOptions     options;
    StandardUnitCollection  collection (options);
    
    StandardUnitCollection::const_iterator  iter = collection.begin();
    StandardUnitCollection::Entry           firstEntry = *iter;

    ++iter;

    StandardUnitCollection::Entry           secondEntry = *iter;

    ASSERT_TRUE ( StandardUnit::MetricFemtometers == firstEntry.GetNumber() && 
                  StandardUnit::MetricPicometers  == secondEntry.GetNumber() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitIterator, ToNext_Descending)
    {
    ScopedDgnHost autoDgnHost;
    UnitIteratorOptions options;
    options.SetOrderDescending();
    
    StandardUnitCollection  collection (options);

    StandardUnitCollection::const_iterator  iter = collection.begin();
    StandardUnitCollection::Entry           firstEntry = *iter;

    ++iter;

    StandardUnitCollection::Entry           secondEntry = *iter;

    ASSERT_TRUE ( StandardUnit::UnitlessWhole == firstEntry.GetNumber() && 
                  StandardUnit::AngleRadians  == secondEntry.GetNumber() );
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// UnitIteratorOptions
// TODO: seems like all functions are public setters: Need to think of better tests.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StandardUnitCollection::Entry EEE;
bool VerifyExpectedStandardUnits (UnitIteratorOptionsCR options, bvector<StandardUnit> searchArray)
    {
    bvector<StandardUnit> foundEntries;

    StandardUnitCollection collection (options);

    FOR_EACH(EEE const& standardUnit, collection)
        foundEntries.push_back (standardUnit.GetNumber());

    std::sort(foundEntries.begin(), foundEntries.end());
    std::sort(searchArray.begin(), searchArray.end());
    return foundEntries == searchArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
void InsertArrayIntoVector (StandardUnit const in[], size_t const sz, bvector<StandardUnit> & out)
    {
    for (size_t i = 0; i < sz; ++i)
        out.push_back (in[i]);
    }

//=======================================================================================
// @bsiclass                                                    Umar.Hayat     11/2015
//=======================================================================================
struct UnitIteratorOptionsTest : public ::testing::Test
{
    ScopedDgnHost autoDgnHost;
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitIteratorOptionsTest, SetAllowSingleBase)
    {
    UnitIteratorOptions options;
    options.SetAllowSingleBase(UnitBase::Degree);

    StandardUnitCollection collection (options);

    StandardUnitCollection::Entry           firstEntry = *collection.begin();

    ASSERT_TRUE ( StandardUnit::AngleSeconds == firstEntry.GetNumber() );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitIteratorOptionsTest, SetAllowAdditionalBase)
    {
    UnitIteratorOptions options;
    options.SetAllowSingleBase(UnitBase::Degree);
    options.SetAllowAdditionalBase(UnitBase::None);

    const int NUM_VALID_ENTRIES = 6;
    StandardUnit validArray[NUM_VALID_ENTRIES] = { StandardUnit::UnitlessWhole, 
                                     StandardUnit::AngleRadians, 
                                     StandardUnit::AngleDegrees, 
                                     StandardUnit::AngleGrads, 
                                     StandardUnit::AngleMinutes, 
                                     StandardUnit::AngleSeconds };

    bvector<StandardUnit> validVector;
    InsertArrayIntoVector (validArray, NUM_VALID_ENTRIES, validVector);
    
    if ( ! VerifyExpectedStandardUnits (options, validVector))
        ASSERT_FALSE(UNIT_ITERATOR_OPTIONS_ERROR_STRING);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitIteratorOptionsTest, SetAllowAdditionalSystem)
    {
    UnitIteratorOptions options;
    options.SetAllowSingleBase (UnitBase::Meter);
    options.SetAllowSingleSystem (UnitSystem::USSurvey);
    options.SetAllowAdditionalSystem (UnitSystem::Undefined);

    const int NUM_VALID_ENTRIES = 12;
    StandardUnit validArray[NUM_VALID_ENTRIES] = { StandardUnit::EnglishSurveyMiles, 
                                     StandardUnit::EnglishFurlongs, 
                                     StandardUnit::EnglishChains, 
                                     StandardUnit::EnglishRods, 
                                     StandardUnit::EnglishFathoms, 
                                     StandardUnit::EnglishSurveyFeet,
                                     StandardUnit::EnglishSurveyInches,
                                     StandardUnit::NoSystemAngstroms,
                                     StandardUnit::NoSystemNauticalMiles,
                                     StandardUnit::NoSystemAstronomicalUnits,
                                     StandardUnit::NoSystemLightYears,
                                     StandardUnit::NoSystemParsecs };


    bvector<StandardUnit> validVector;
    InsertArrayIntoVector (validArray, NUM_VALID_ENTRIES, validVector);
    
    if ( ! VerifyExpectedStandardUnits (options, validVector))
        ASSERT_FALSE(UNIT_ITERATOR_OPTIONS_ERROR_STRING);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (UnitIteratorOptionsTest, SetSizeCriteria)
    {
    UnitDefinition def = UnitDefinition::GetStandardUnit (StandardUnit::MetricNanometers);
    
    UnitIteratorOptions options;
    options.SetSizeCriteria (def, UnitIteratorOptions::SIZECOMPARE_AllowSmallerOrEqual);
    options.SetAllowSingleBase (UnitBase::Meter);
    options.SetAllowSingleSystem (UnitSystem::Metric);
    
    const int NUM_VALID_ENTRIES = 3;
    StandardUnit validArray[NUM_VALID_ENTRIES] = { StandardUnit::MetricFemtometers, 
                                          StandardUnit::MetricPicometers, 
                                          StandardUnit::MetricNanometers };

    bvector<StandardUnit> validVector;
    InsertArrayIntoVector (validArray, NUM_VALID_ENTRIES, validVector);
    
    if ( ! VerifyExpectedStandardUnits (options, validVector))
        ASSERT_FALSE(UNIT_ITERATOR_OPTIONS_ERROR_STRING);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
* TODO: same test as:
*    TEST_F (UnitManagerTest, CreateStandardUnitIterator)
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitIterator, GetUnitDef)
    {
    UnitIteratorOptions     options;
    StandardUnitCollection  collection (options);

    StandardUnitCollection::Entry   unitEntry = *collection.begin();

    UnitDefinition def = unitEntry.GetUnitDef();
    UnitDefinition cmp = UnitDefinition::GetStandardUnit (StandardUnit::MetricFemtometers);

    ASSERT_TRUE (cmp.GetDenominator() == def.GetDenominator()&& 
           cmp.GetNumerator() == def.GetNumerator() && 
           cmp.GetBase() == def.GetBase() && 
           cmp.GetSystem() == def.GetSystem());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitIterator, GetNumber)
    {
    UnitIteratorOptions     options;
    StandardUnitCollection  collection (options);

    StandardUnitCollection::Entry   unitEntry = *collection.begin();
    
    ASSERT_TRUE (StandardUnit::MetricFemtometers == unitEntry.GetNumber());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (UnitIterator, GetName)
    {
    UnitIteratorOptions     options;
    StandardUnitCollection  collection (options);

    StandardUnitCollection::Entry   unitEntry = *collection.begin();
    
    ASSERT_TRUE (unitEntry.GetName() == WString(LOCALIZED_STR(L"Femtometers")));
    }
