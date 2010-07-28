/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECPropertyVerifier.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
using namespace TestHelpers;
using namespace std;

/*---------------------------------------------------------------------------------**//**
Create a new PrimitiveECProperty when its name and type is passed.
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECPropertyVerifier::CreatePrimitiveProperty (PrimitiveECPropertyP & ecProperty, bwstring propertyName, PrimitiveType type, ECClassP classToAddProperty)
    {
    ECObjectsStatus ecobjStatus = classToAddProperty->CreatePrimitiveProperty (ecProperty, propertyName, type);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreatePrimitiveProperty method returned error";
        return ERROR;
        }
    if (ecProperty == NULL)
        {
        EXPECT_TRUE (false) << "CreatePrimitiveProperty method returned NULL object";
        return ERROR;
        }
    else
        wcout<<"Test Adding Property"<<endl;

        //TODO: Remove this.
        ecProperty->DisplayLabel = L"New Label";
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
Create a new StructECProperty when its name and type is passed.
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECPropertyVerifier::CreateStructProperty(StructECPropertyP& ecProperty, bwstring const& name, ECClassCR structType, ECClassR classToAddProperty)
    {
    ECObjectsStatus ecobjStatus = classToAddProperty.CreateStructProperty (ecProperty, name, structType);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreateStructProperty method returned error";
        return ECOBJECTS_STATUS_Error;
        }
    if (ecProperty == NULL)
        {
        EXPECT_TRUE (false) << "CreateStructProperty method returned NULL object";
        return ECOBJECTS_STATUS_Error;
        }
    else
        wcout<<"Test Adding Property"<<endl;
    return ECOBJECTS_STATUS_Success;
    }
