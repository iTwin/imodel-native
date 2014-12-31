/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaImportContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECSchemaValidator.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct SchemaImportContext
    {
private:
    std::map<ECN::ECSchemaCP, ECSchemaValidationResult> m_schemaValidationResultMap; //cannot use bmap as value type is not copyable
    ECDbSchemaManager::IImportIssueListener const& m_importIssueListener;

public:
    explicit SchemaImportContext (ECDbSchemaManager::IImportIssueListener const* importIssueListener);

    ECSchemaValidationResult const* GetSchemaValidationResult (ECN::ECSchemaCR schema) const;
    ECSchemaValidationResult& GetSchemaValidationResultR (ECN::ECSchemaCR schema);
    std::map<ECN::ECSchemaCP, ECSchemaValidationResult> const& GetSchemaValidationResultMap () const { return m_schemaValidationResultMap; }

    ECDbSchemaManager::IImportIssueListener const& GetIssueListener () const { return m_importIssueListener; }
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2014
//+===============+===============+===============+===============+===============+======
struct NoopImportIssueListener : ECDbSchemaManager::IImportIssueListener
    {
private:
    static NoopImportIssueListener s_singleton;

    NoopImportIssueListener ()
        : ECDbSchemaManager::IImportIssueListener ()
        {}

    virtual void _OnIssueReported (Severity severity, Utf8CP message) const override {}
public:
    ~NoopImportIssueListener () {}

    static IImportIssueListener const& Singleton () { return s_singleton; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE