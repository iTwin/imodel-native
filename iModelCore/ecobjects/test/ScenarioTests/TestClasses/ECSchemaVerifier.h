/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECSchemaVerifier.h $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
using namespace Bentley::EC;

/*=================================================================================**\\**
* @bsiclass                                                        Farrukh Latif 06/10
+===============+===============+===============+===============+===============+======*/
namespace TestHelpers
{
class ECSchemaVerifier
    {
    public: 
        ECSchemaP m_ECSchemaP;
        ECSchemaOwnerPtr                    schemaOwner;
        ECClassP m_ECClassP;
        ECSchemaVerifier(): m_ECSchemaP (NULL)
            {}

        ECSchemaVerifier(ECSchemaP m_schema): m_ECSchemaP (m_schema)
            {}

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
//public: ECObjectsStatus CreateSchema_Success (ECSchemaP& schemaOut, bwstring const& schemaName);
public: ECObjectsStatus CreateSchema_Success (ECSchemaP& schemaOut, bwstring const& schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
//public: ECObjectsStatus CreateSchema_Failure (ECSchemaP& schemaOut, bwstring const& schemaName);
public: ECObjectsStatus CreateSchema_Failure (ECSchemaP& schemaOut, bwstring const& schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus CreateRelationshipClass_Success (ECRelationshipClassP& pClass, bwstring const& relationshipName);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus CreateRelationshipClass_Failure (ECObjectsStatus expectedStatus, bwstring const& relationshipName);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: static SchemaDeserializationStatus ReadXmlFromFile (ECSchemaOwnerPtr &schemaOwner, ECSchemaP & schemaOut, const wchar_t * ecSchemaXmlFile, const bvector< IECSchemaLocatorP > * schemaLocators, const bvector< const wchar_t * > * schemaPaths);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: SchemaSerializationStatus WriteXmlToFile (const wchar_t * ecSchemaXmlFile, ECSchemaP schema);
    };
};
