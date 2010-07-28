/*--------------------------------------------------------------------------------------+
|
|  $Source: ecobjects/nativeatp/ScenarioTests/TestClasses/ECPropertyVerifier.h $
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
class ECPropertyVerifier
    {
    public: 
        ECSchemaP m_ECSchemaP;
        ECClassP m_ECClassP;
        ECPropertyVerifier(): m_ECSchemaP (NULL)
            {}

        ECPropertyVerifier(ECSchemaP m_schema): m_ECSchemaP (m_schema)
            {}

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: BentleyStatus CreatePrimitiveProperty (PrimitiveECPropertyP & ecProperty, bwstring propertyName, PrimitiveType type,ECClassP classToAddProperty);

/*---------------------------------------------------------------------------------**\\**
* @bsimethod                                                    Farrukh Latif  06/10
+---------------+---------------+---------------+---------------+---------------+------*/
public: ECObjectsStatus CreateStructProperty(StructECPropertyP& ecProperty, bwstring const& name, ECClassCR structType, ECClassR classToAddProperty);

    };
};
