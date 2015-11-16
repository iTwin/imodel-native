/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/ECDbDebugInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECDbDebugInfo.h>

#include "../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbDebugInfo::ECDbDebugInfo(ObservableECDb& ecDb) :
m_dbAdapter(ecDb)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ShouldSkipSchemaClasses(ECSchemaCP schema)
    {
    return
        schema->GetName().Equals(L"Bentley_Standard_Classes") ||
        schema->GetName().Equals(L"ECDbSystem");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECDbDebugInfo::GetDataDebugInfo(ECSchemaCP schema)
    {
    Utf8String message;

    if (nullptr == schema)
        {
        message = "Error! Null schema\n";
        return message;
        }

    message += "\tSchema: " + Utf8String(schema->GetName()) + ":\n";

    if (ShouldSkipSchemaClasses(schema))
        {
        return message;
        }

    for (ECClassCP ecClass : schema->GetClasses())
        {
        if (!ecClass->GetIsDomainClass() ||
            ecClass->GetIsCustomAttributeClass() ||
            ecClass->GetIsStruct() ||
            ecClass->GetRelationshipClassCP() != nullptr)
            {
            continue;
            }

        int instanceCount = m_dbAdapter.CountClassInstances(ecClass);

        Utf8String classDebug;
        classDebug.Sprintf("\tObjects:%-3d - class: \"%s\" ", instanceCount, Utf8String(ecClass->GetName()).c_str());

        message += classDebug + "\n";
        }

    for (ECClassCP ecClass : schema->GetClasses())
        {
        if (!ecClass->GetIsDomainClass() ||
            ecClass->GetIsCustomAttributeClass() ||
            ecClass->GetIsStruct() ||
            ecClass->GetRelationshipClassCP() == nullptr)
            {
            continue;
            }

        int instanceCount = m_dbAdapter.CountClassInstances(ecClass);

        Utf8String relationshipDebug;
        relationshipDebug.Sprintf("\tRelationships:%-3d - class: \"%s\"", instanceCount, Utf8String(ecClass->GetName()).c_str());

        message += relationshipDebug + "\n";
        }
    message += "\n";
    return message;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbDebugInfoHolder::ECDbDebugInfoHolder(ObservableECDb& ecDb, const bvector<ECSchemaCP>& schemas, Utf8StringCR message, Utf8StringCR context) :
m_info(ecDb),
m_schemas(schemas),
m_message(message),
m_context(context)
    {
    Log("Before " + m_context);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbDebugInfoHolder::Log(Utf8StringCR context)
    {
    Utf8PrintfString message("\n%s (%s)\n", m_message.c_str(), context.c_str());
    for (ECSchemaCP schema : m_schemas)
        {
        message += m_info.GetDataDebugInfo(schema);
        }
    LOG.trace(message);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECDbDebugInfoHolder::~ECDbDebugInfoHolder()
    {
    Log("After " + m_context);
    }
