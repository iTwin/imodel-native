/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECSchemaTestFixture.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
using namespace Bentley::EC;

#include "..\..\Published\TestFixture.h"


/*--------------------------------------------------------------------------------------+
|
| This file contains fixture classes for basic schema operations.
|
+--------------------------------------------------------------------------------------*/

/*=================================================================================**//**
* @bsiclass                                                        Farrukh Latif 7/10
+===============+===============+===============+===============+===============+======*/
namespace TestHelpers
{
/*=================================================================================**//**
* @bsiclass                                                        Farrukh Latif 7/10
+===============+===============+===============+===============+===============+======*/
class ECSchemaTestFixture : public ::testing::Test
    {
    ECSchemaCachePtr schemaOwner;
    public: ECSchemaP m_schema;
    public: ECClassContainer::const_iterator iter;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif 7/10
+---------------+---------------+---------------+---------------+---------------+------*/
    public: virtual void SetUp() override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif 7/10
+---------------+---------------+---------------+---------------+---------------+------*/
    public: virtual void TearDown () override {}

    };

};
