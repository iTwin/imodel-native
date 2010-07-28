/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECClassVerifier.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
using namespace TestHelpers;
using namespace std;

/*---------------------------------------------------------------------------------**//**
Create an ECClass when its name is passed.
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::CreateClass_Success (ECClassP &classP, bwstring className)
    {
    ECObjectsStatus ecobjStatus = m_ECSchemaP->CreateClass (classP, className);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreateClass method returned error";
        return ERROR;
        }
    if (classP == NULL)
        {
        EXPECT_TRUE (false) << "CreateClass method returned NULL object";
        return ERROR;
        }
    return SUCCESS;
    }

    /*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::CreateClass_Failure (ECObjectsStatus expectedStatus, bwstring className)
    {
    ECClassP classP;
    ECObjectsStatus ecobjStatus = m_ECSchemaP->CreateClass (classP, className);

    cout << "Status: " << ecobjStatus << endl;

    if (ecobjStatus != expectedStatus)
        {
        EXPECT_TRUE (false) << "CreateClass method returned an error status that was not expected.";
        return ERROR;
        }

    if (classP != NULL)
        {
        EXPECT_TRUE (false) << "CreateClass method must return NULL";
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::AddBaseClass_Success (ECClassP baseClass, ECClassP childClass)
{
    ECObjectsStatus ecobjStatus = childClass->AddBaseClass(*baseClass);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
    {
        EXPECT_TRUE (false) << "AddBaseClass method returned error";
        return ERROR;
        
    }
     else
        return SUCCESS;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::AddBaseClass_Failure (ECClassP baseClass, ECClassP childClass)
    {
    ECObjectsStatus ecobjStatus = childClass->AddBaseClass(*baseClass);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
            if (ecobjStatus == ECOBJECTS_STATUS_BaseClassUnacceptable)
            {
                return SUCCESS;
            }
        
            EXPECT_TRUE (false) << "AddBaseClass method returned error";
            return ERROR;
        }
    else
        return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::RemoveBaseClass(ECClassP baseClass, ECClassP childClass)
    {
    ECObjectsStatus ecobjStatus = childClass->AddBaseClass(*baseClass);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
    {
        EXPECT_TRUE (false) << "AddBaseClass method returned error";
        return ERROR;
        
    }
     else
        return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECClassVerifier::ModifyECClass(ECClassP ecClass, bwstring fieldValue, int flag)
    {
    switch (flag)
        {
            case 1:
            {
                ecClass->Description = fieldValue;
                if (ecClass->Description != fieldValue)
                {
                    EXPECT_TRUE (false) << "Class description could not be set properly";
                    return ERROR;
                }
                else
                    return SUCCESS;
            }

            case 0:
            {
                ecClass->DisplayLabel = fieldValue;
                if (ecClass->DisplayLabel != fieldValue)
                {
                    EXPECT_TRUE (false) << "Class display label could not be set properly";
                    return ERROR;
                }
                else
                    return SUCCESS;
            }

            default:
                EXPECT_TRUE (false) << "Class display label could not be set properly";
                return ERROR;
        }
    }