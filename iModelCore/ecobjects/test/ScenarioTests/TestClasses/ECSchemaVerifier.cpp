/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECSchemaVerifier.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
using namespace Bentley::EC;
using namespace TestHelpers;
using namespace std;

/*---------------------------------------------------------------------------------**//**
Create a new ECSchema when name is passed
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaVerifier::CreateSchema_Success (ECSchemaP& schemaOut, bwstring const& schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner)
    {
    ECObjectsStatus ecobjStatus = ECSchema::CreateSchema(schemaOut, schemaName, versionMajor, versionMinor, schemaOwner);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreateSchema method returned error";
        return ECOBJECTS_STATUS_InvalidName;
        }
    else
        wcout<<"Schema created successfully"<<endl;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
Do not create schema in case of invalid name parameter
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaVerifier::CreateSchema_Failure (ECSchemaP& schemaOut, bwstring const& schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner)
    {
    ECObjectsStatus ecobjStatus = ECSchema::CreateSchema(schemaOut, schemaName, versionMajor, versionMinor, schemaOwner);
    if (ecobjStatus == ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreateSchema method passed while it should fail";
        return ECOBJECTS_STATUS_InvalidName;
        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
Create ECRelationshipClass when name is passed
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaVerifier::CreateRelationshipClass_Success (ECRelationshipClassP& pClass, bwstring const& relationshipName)
    {
    ECObjectsStatus ecobjStatus = m_ECSchemaP->CreateRelationshipClass(pClass, relationshipName);
    if (ecobjStatus != ECOBJECTS_STATUS_Success)
        {
        EXPECT_TRUE (false) << "CreateRelationshipClass method returned error";
        return ERROR;
        }
    else
        wcout<<"Relationship class created successfully"<<endl;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
Do not create ECRelationshipClass in case of invalid name parameter
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECSchemaVerifier::CreateRelationshipClass_Failure (ECObjectsStatus expectedStatus, bwstring const& relationshipName)
    {
    ECRelationshipClassP relClass;
    ECObjectsStatus ecobjStatus = m_ECSchemaP->CreateRelationshipClass(relClass, relationshipName);
    if (ecobjStatus != expectedStatus)
        {
        EXPECT_TRUE (false) << "CreateRelationshipClass method returned an error status that was not expected.";
        return ERROR;
        }

    if (relClass != NULL)
        {
        EXPECT_TRUE (false) << "CreateRelationshipClass method must return NULL";
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
Read XML file from a given location and deserialize as an ECXML schema
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchemaVerifier::ReadXmlFromFile (ECSchemaP & schemaOut, const wchar_t * ecSchemaXmlFile, const bvector< IECSchemaLocatorP > * schemaLocators, const bvector< const wchar_t * > * schemaPaths)
    {
    schemaOwner = ECSchemaOwner::CreateOwner();
    ECSchemaDeserializationContextPtr   schemaContext = ECSchemaDeserializationContext::CreateContext(*schemaOwner);
    SchemaDeserializationStatus schemaDesrializationStatus = ECSchema::ReadXmlFromFile(schemaOut, ecSchemaXmlFile, *schemaContext);
    if (schemaDesrializationStatus != SCHEMA_DESERIALIZATION_STATUS_Success)
        {
        EXPECT_TRUE(false) << "ReadXmlFromFile method returned error";

        switch(schemaDesrializationStatus)
            {
             case SCHEMA_DESERIALIZATION_STATUS_Success:
             cout << "SCHEMA_DESERIALIZATION_STATUS_Success\n";
             break;
             case SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl:
             cout << "SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl\n";
             break;
             case SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml:
             cout << "SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml\n";
             break;
             case SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml:
             cout << "SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml\n";
             break;
             case SCHEMA_DESERIALIZATION_STATUS_ReferencedSchemaNotFound:
             cout << "SCHEMA_DESERIALIZATION_STATUS_ReferencedSchemaNotFound\n";
             break;
            }

        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }
    else
        wcout<<"Xml file read successfully"<<endl;
        return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchemaVerifier::WriteXmlToFile (const wchar_t * ecSchemaXmlFile, ECSchemaP schema)
    {
    SchemaSerializationStatus schemaSerializationStatus = schema->WriteXmlToFile(ecSchemaXmlFile);
    if (schemaSerializationStatus != SCHEMA_SERIALIZATION_STATUS_Success)
        {
        EXPECT_TRUE(false) << "WriteXmlToFile method returned error";
        return SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml;
        }
    else
        wcout<<"Xml file written successfully"<<endl;
        return SCHEMA_SERIALIZATION_STATUS_Success;
    }