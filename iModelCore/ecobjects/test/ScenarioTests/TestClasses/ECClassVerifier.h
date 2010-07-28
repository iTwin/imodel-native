/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECClassVerifier.h $
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
class ECClassVerifier
{
public:
    ECSchemaP m_ECSchemaP;
    ECClassVerifier():m_ECSchemaP(NULL)
    {}

    ECClassVerifier(ECSchemaP m_schema):m_ECSchemaP(m_schema)
    {}

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus CreateClass_Success (ECClassP &classP, bwstring className);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus CreateClass_Failure (ECObjectsStatus expectedStatus, bwstring className);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus AddBaseClass_Success (ECClassP baseClass, ECClassP childClass);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus AddBaseClass_Failure (ECClassP baseClassP, ECClassP childClass);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus RemoveBaseClass(ECClassP baseClass, ECClassP childClass);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  07/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus ModifyECClass(ECClassP ecClass, bwstring fieldValue, int flag);
};
};