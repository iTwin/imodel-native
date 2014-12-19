/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaImportContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// SchemaImportContext
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
SchemaImportContext::SchemaImportContext (ECDbSchemaManager::IImportIssueListener const* importIssueListener)
: m_importIssueListener (importIssueListener != nullptr ? *importIssueListener : NoopImportIssueListener::Singleton ())
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
ECSchemaValidationResult const* SchemaImportContext::GetSchemaValidationResult (ECN::ECSchemaCR schema) const
    {
    auto schemaIt = m_schemaValidationResultMap.find (&schema);
    if (schemaIt == m_schemaValidationResultMap.end ())
        return nullptr;

    return &schemaIt->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
ECSchemaValidationResult& SchemaImportContext::GetSchemaValidationResultR (ECN::ECSchemaCR schema)
    {
    return m_schemaValidationResultMap[&schema];
    }


//*************************************************************************************
// NoopImportIssueListener
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    04/2014
//---------------------------------------------------------------------------------------
//static member initialization
NoopImportIssueListener NoopImportIssueListener::s_singleton;

END_BENTLEY_SQLITE_EC_NAMESPACE
