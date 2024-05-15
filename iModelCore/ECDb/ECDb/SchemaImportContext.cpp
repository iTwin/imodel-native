/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************************************************************************************
// SchemaImportContext
//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SchemaImportContext::AllowDataTransform() {
    constexpr auto kSchemaUpgrade = SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade;
    if ((kSchemaUpgrade & GetOptions()) == kSchemaUpgrade) {
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
MainSchemaManager const& SchemaImportContext::GetSchemaManager() const { return m_ecdb.Schemas().Main(); }


//*********************************************************************************
// SchemaPolicies
//*********************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPolicies::ReadPolicies(ECDbCR ecdb)
    {
    std::vector<ECN::ECSchemaId> systemSchemaExceptions = GetSystemSchemaExceptions(ecdb);
    for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
        {
        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalForeignKeyConstraints, systemSchemaExceptions))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalLinkTables, systemSchemaExceptions))
            return ERROR;

        if (SUCCESS != ReadPolicy(ecdb, *schema, SchemaPolicy::Type::NoAdditionalRootEntityClasses, systemSchemaExceptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaPolicies::ReadPolicy(ECDbCR ecdb, ECN::ECSchemaCR schema, SchemaPolicy::Type policyType, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    IECInstancePtr policyCA = schema.GetCustomAttributeLocal("ECDbSchemaPolicies", SchemaPolicy::TypeToString(policyType));
    if (policyCA == nullptr)
        return SUCCESS;

    auto it = m_optedInPolicies.find(policyType);
    if (it != m_optedInPolicies.end())
        {
        ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0265,
            "Failed to import schemas. Schema '%s' opts in policy '%s' although it is already opted in by schema '%s'. A schema policy can only be opted in by one schema.",
            schema.GetName().c_str(),
            SchemaPolicy::TypeToString(policyType),
            SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), it->second->GetOptingInSchemaId()).c_str()
        );
        return ERROR;
        }

    std::unique_ptr<SchemaPolicy> policy = nullptr;
    switch (policyType)
        {
            case SchemaPolicy::Type::NoAdditionalForeignKeyConstraints:
                policy = NoAdditionalForeignKeyConstraintsPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            case SchemaPolicy::Type::NoAdditionalLinkTables:
                policy = NoAdditionalLinkTablesPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            case SchemaPolicy::Type::NoAdditionalRootEntityClasses:
                policy = NoAdditionalRootEntityClassesPolicy::Create(ecdb, schema.GetId(), *policyCA, systemSchemaExceptions);
                break;

            default:
                BeAssert(false && "New SchemaPolicy type was added. Adjust this method!");
                return ERROR;
        }

    if (policy == nullptr)
        return ERROR;

    m_optedInPolicies[policyType] = std::move(policy);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaPolicies::IsOptedIn(SchemaPolicy const*& policy, SchemaPolicy::Type policyType) const
    {
    auto it = m_optedInPolicies.find(policyType);
    if (it == m_optedInPolicies.end())
        return false;

    policy = it->second.get();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<ECSchemaId> SchemaPolicies::GetSystemSchemaExceptions(ECDbCR ecdb)
    {
    std::vector<Utf8CP> ecdbSchemaNames = ecdb.Schemas().GetDispatcher().GetECDbSchemaNames();
    std::set<Utf8CP, CompareIUtf8Ascii> systemSchemaExceptions(ecdbSchemaNames.begin(), ecdbSchemaNames.end());
    return SchemaPersistenceHelper::GetSchemaIds(ecdb, DbTableSpace::Main(), Utf8StringVirtualSet([&systemSchemaExceptions] (Utf8CP name) { return systemSchemaExceptions.find(name) != systemSchemaExceptions.end(); }));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool SchemaPolicy::IsException(ECN::ECClassCR candidateClass) const
    {
    if (m_schemaExceptions.find(candidateClass.GetSchema().GetId()) != m_schemaExceptions.end())
        return true;

    return m_classExceptions.find(candidateClass.GetId()) != m_classExceptions.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus SchemaPolicy::RetrieveExceptions(bvector<bvector<Utf8String>>& tokenizedExceptions, ECN::IECInstanceCR policyCA, Utf8CP exceptionsPropName)
    {
    ECValue exceptionsVal;
    if (ECObjectsStatus::Success != policyCA.GetValue(exceptionsVal, exceptionsPropName))
        return ERROR;

    if (exceptionsVal.IsNull())
        return SUCCESS;

    const uint32_t arraySize = exceptionsVal.GetArrayInfo().GetCount();
    for (uint32_t i = 0; i < arraySize; i++)
        {
        ECValue exceptionVal;
        if (ECObjectsStatus::Success != policyCA.GetValue(exceptionVal, exceptionsPropName, i))
            return ERROR;

        if (exceptionVal.IsNull() || Utf8String::IsNullOrEmpty(exceptionVal.GetUtf8CP()))
            continue;

        tokenizedExceptions.push_back(bvector<Utf8String>());
        BeStringUtilities::Split(exceptionVal.GetUtf8CP(), ":.", tokenizedExceptions.back());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
Utf8CP SchemaPolicy::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::NoAdditionalForeignKeyConstraints:
                return "NoAdditionalForeignKeyConstraints";
            case Type::NoAdditionalLinkTables:
                return "NoAdditionalLinkTables";
            case Type::NoAdditionalRootEntityClasses:
                return "NoAdditionalRootEntityClasses";

            default:
                BeAssert(false && "SchemaPolicy::Type enum has a new value. This method needs to be adjusted");
                return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalRootEntityClassesPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalRootEntityClassesPolicy> policy(new NoAdditionalRootEntityClassesPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(optingInSchemaId); // the opting-in schema is always an exception
    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0266,
            "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
            policyCA.GetClass().GetName().c_str(),
            SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), optingInSchemaId).c_str()
        );
        return nullptr;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 2)
            {
            ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0266,
                "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                policyCA.GetClass().GetName().c_str(), SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), optingInSchemaId).c_str());
            return nullptr;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = SchemaPersistenceHelper::GetSchemaId(ecdb, DbTableSpace::Main(), tokenizedException[0].c_str(), SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                policy->m_schemaExceptions.insert(exceptionSchemaId);
            }
        else
            {
            ECClassId exceptionClassId = ecdb.Schemas().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);

            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                policy->m_classExceptions.insert(exceptionClassId);
            }
        }

    return std::move(policy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalRootEntityClassesPolicy::Evaluate(ECDbCR ecdb, ECN::ECEntityClassCR ecClass) const
    {
    if (ecClass.HasBaseClasses() || ecClass.IsMixin() || ClassViews::IsViewClass(ecClass) || IsException(ecClass))
        return SUCCESS;

    ecdb.GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECDbIssue,
        ECDbIssueId::ECDb_0268,
        "Failed to import ECClass '%s'. It violates against the 'No additional root entity classes' policy which means that all entity classes must subclass from classes defined in the ECSchema %s",
        ecClass.GetFullName(),
        SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), GetOptingInSchemaId()).c_str()
    );

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalLinkTablesPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalLinkTablesPolicy> policy(new NoAdditionalLinkTablesPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(optingInSchemaId); // the opting-in schema is always an exception
    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0266,
            "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
            policyCA.GetClass().GetName().c_str(),
            SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), optingInSchemaId).c_str()
        );
        return nullptr;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 2)
            {
            ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue,
                ECDbIssueId::ECDb_0266,
                "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                policyCA.GetClass().GetName().c_str(),
                SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), optingInSchemaId).c_str()
            );
            return nullptr;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = SchemaPersistenceHelper::GetSchemaId(ecdb, DbTableSpace::Main(), tokenizedException[0].c_str(), SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                policy->m_schemaExceptions.insert(exceptionSchemaId);
            }
        else
            {
            ECClassId exceptionClassId = ecdb.Schemas().Main().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);

            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                policy->m_classExceptions.insert(exceptionClassId);
            }
        }

    return std::move(policy);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalLinkTablesPolicy::Evaluate(ECDbCR ecdb, ECN::ECRelationshipClassCR relClass) const
    {
    if (relClass.HasBaseClasses() || IsException(relClass))
        return SUCCESS;

    ecdb.GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECDbIssue,
        ECDbIssueId::ECDb_0271,
        "Failed to import ECRelationshipClass '%s'. It violates against the 'No additional link tables' policy which means that relationship classes with 'Link table' mapping must subclass from relationship classes defined in the ECSchema %s",
        relClass.GetFullName(),
        SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), GetOptingInSchemaId()).c_str()
    );

    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<SchemaPolicy> NoAdditionalForeignKeyConstraintsPolicy::Create(ECDbCR ecdb, ECN::ECSchemaId optingInSchemaId, ECN::IECInstanceCR policyCA, std::vector<ECN::ECSchemaId> const& systemSchemaExceptions)
    {
    std::unique_ptr<NoAdditionalForeignKeyConstraintsPolicy> policy(new NoAdditionalForeignKeyConstraintsPolicy(optingInSchemaId));

    policy->m_schemaExceptions.insert(systemSchemaExceptions.begin(), systemSchemaExceptions.end());

    if (SUCCESS != policy->ReadExceptionsFromCA(ecdb, policyCA))
        {
        ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0266,
            "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
            policyCA.GetClass().GetName().c_str(),
            SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), optingInSchemaId).c_str()
        );
        return nullptr;
        }

    return std::move(policy);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalForeignKeyConstraintsPolicy::ReadExceptionsFromCA(ECDbCR ecdb, ECN::IECInstanceCR policyCA)
    {
    m_schemaExceptions.insert(m_optingInSchemaId); // the opting-in schema is always an exception

    bvector<bvector<Utf8String>> tokenedExceptions;
    if (SUCCESS != RetrieveExceptions(tokenedExceptions, policyCA, "Exceptions"))
        {
        ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0266,
            "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
            policyCA.GetClass().GetName().c_str(),
            SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), m_optingInSchemaId).c_str()
        );
        return ERROR;
        }

    for (bvector<Utf8String> const& tokenizedException : tokenedExceptions)
        {
        const size_t tokenCount = tokenizedException.size();
        if (tokenCount == 0 || tokenCount > 3)
            {
            ecdb.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue,
                ECDbIssueId::ECDb_0266,
                "Failed to read the %s custom attribute from schema %s because it has invalid exceptions. Make sure they are formatted correctly.",
                policyCA.GetClass().GetName().c_str(),
                SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), m_optingInSchemaId).c_str()
            );
            return ERROR;
            }

        if (tokenCount == 1 || tokenizedException[1].EqualsIAscii("*"))
            {
            ECSchemaId exceptionSchemaId = SchemaPersistenceHelper::GetSchemaId(ecdb, DbTableSpace::Main(), tokenizedException[0].c_str(), SchemaLookupMode::AutoDetect);
            // If schema id doesn't exist, the schema is not yet imported by this schema import. Exception can be ignored
            if (exceptionSchemaId.IsValid())
                m_schemaExceptions.insert(exceptionSchemaId);
            }
        else if (tokenCount == 2 || tokenizedException[2].EqualsIAscii("*"))
            {
            ECClassId exceptionClassId = ecdb.Schemas().Main().GetClassId(tokenizedException[0], tokenizedException[1], SchemaLookupMode::AutoDetect);

            // If class id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionClassId.IsValid())
                m_classExceptions.insert(exceptionClassId);
            }
        else
            {
            ECPropertyId exceptionPropId = SchemaPersistenceHelper::GetPropertyId(ecdb, DbTableSpace::Main(), tokenizedException[0].c_str(), tokenizedException[1].c_str(), tokenizedException[2].c_str(), SchemaLookupMode::AutoDetect);
            // If property id doesn't exist, the class is not yet imported by this schema import. Exception can be ignored
            if (exceptionPropId.IsValid())
                m_propertyExceptions.insert(exceptionPropId);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus NoAdditionalForeignKeyConstraintsPolicy::Evaluate(ECDbCR ecdb, ECN::NavigationECPropertyCR navPropWithFkConstraintCA) const
    {
    if (IsException(navPropWithFkConstraintCA))
        return SUCCESS;

    ecdb.GetImpl().Issues().ReportV(
        IssueSeverity::Error,
        IssueCategory::BusinessProperties,
        IssueType::ECDbIssue,
        ECDbIssueId::ECDb_0275,
        "Failed to import ECClass '%s'. Its navigation property '%s' violates against the 'No additional foreign key constraints' policy which means that navigation properties may not define the 'ForeignKeyConstraint' custom attribute other than in the ECSchema %s",
        navPropWithFkConstraintCA.GetClass().GetFullName(),
        navPropWithFkConstraintCA.GetName().c_str(),
        SchemaPersistenceHelper::GetSchemaName(ecdb, DbTableSpace::Main(), GetOptingInSchemaId()).c_str()
    );

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool NoAdditionalForeignKeyConstraintsPolicy::IsException(ECN::NavigationECPropertyCR navProp) const
    {
    if (IsException(navProp.GetClass()))
        return true;

    return m_propertyExceptions.find(navProp.GetId()) != m_propertyExceptions.end();
}

//***************************************************************************************
// SqlTypeDetector
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
const std::vector<std::string> SqlTypeDetector::GetDataTables(DbCR conn) {
    std::vector<std::string> tables;
    auto sql = R"sql(
        SELECT [tbl_name]
        FROM   [sqlite_master]
        WHERE   [type] = 'table'
                AND NOT [tbl_name] LIKE 'ec\_%'     ESCAPE '\'
                AND NOT [tbl_name] LIKE 'dgn\_%'    ESCAPE '\'
                AND NOT [tbl_name] LIKE 'be\_%'     ESCAPE '\'
                AND NOT [tbl_name] LIKE 'sqlite\_%' ESCAPE '\')sql";
    auto stmt = conn.GetCachedStatement(sql);
    while (stmt->Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt->GetValueText(0));
    }
    std::sort(tables.begin(), tables.end());
    return tables;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
const std::vector<std::string> SqlTypeDetector::GetSystemTables(DbCR conn) {
    std::vector<std::string> tables;
    auto sql = R"sql(
        SELECT tbl_name
        FROM   [sqlite_master]
        WHERE  [type] = 'table'
                AND     [tbl_name] LIKE 'ec\_%' ESCAPE '\')sql";
    auto stmt = conn.GetCachedStatement(sql);
    while (stmt->Step() == BE_SQLITE_ROW) {
        tables.push_back(stmt->GetValueText(0));
    }
    std::sort(tables.begin(), tables.end());
    return tables;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string SqlTypeDetector::Join(std::vector<std::string> const& v, const std::string sep) {
    if (v.empty()) {
        return "";
    }
    std::string init = v.front();
    return std::accumulate(v.begin() + 1, v.end(), init,
                           [&](std::string s, const std::string& piece) -> decltype(auto) {
                               return s.append(sep).append(piece);
                           });
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqlTypeDetector::SetupRegex(DbCR conn, bool useDataCRUD) {
    RE2::Options opts;
    opts.set_never_capture(true);
    opts.set_case_sensitive(false);
    opts.set_one_line(false);
    opts.set_log_errors(true);

    m_alterTableRegEx = std::make_unique<RE2>(
        R"(^\s*ALTER\s+TABLE\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_dropTableRegEx = std::make_unique<RE2>(
        R"(^\s*DROP\s+TABLE\s+(IF\s+EXISTS)?\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_dropIndexRegEx = std::make_unique<RE2>(
        R"(^\s*DROP\s+INDEX\s+(IF\s+EXISTS)?\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createTableRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((TEMP|TEMPORARY)\s*)?TABLE\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createIndexRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((UNIQUE)\s*)?INDEX\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_createViewRegEx = std::make_unique<RE2>(
        R"(^\s*CREATE\s+((TEMP|TEMPORARY)\s*)?VIEW\s+(IF\s+NOT\s+EXISTS)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_pragmaRegEx = std::make_unique<RE2>(
        R"(^\s*PRAGMA\s+(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_insertRegEx = std::make_unique<RE2>(
        R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_deleteRegEx = std::make_unique<RE2>(
        R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);
    m_updateRegEx = std::make_unique<RE2>(
        R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?\w+\]?))", opts);

    const auto dataTables = Join(GetDataTables(conn), "|");
    m_dataInsertRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);
    m_dataDeleteRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);
    m_dataUpdateRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", dataTables.c_str()).GetUtf8CP(), opts);

    const auto sysTables = Join(GetSystemTables(conn), "|");
    m_sysInsertRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*INSERT\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);
    m_sysDeleteRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*DELETE\s+(FROM)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);
    m_sysUpdateRegEx = std::make_unique<RE2>(
        SqlPrintfString(R"(^\s*UPDATE\s+(INTO)?\s*(\[?\w+\]?\s*\.\s*)?(\[?(%s)\]?))", sysTables.c_str()).GetUtf8CP(), opts);

    BeAssert(m_alterTableRegEx->ok());
    BeAssert(m_dropTableRegEx->ok());
    BeAssert(m_dropIndexRegEx->ok());
    BeAssert(m_createTableRegEx->ok());
    BeAssert(m_createIndexRegEx->ok());
    BeAssert(m_createViewRegEx->ok());
    BeAssert(m_pragmaRegEx->ok());
    BeAssert(m_insertRegEx->ok());
    BeAssert(m_deleteRegEx->ok());
    BeAssert(m_updateRegEx->ok());
    BeAssert(m_dataInsertRegEx->ok());
    BeAssert(m_dataDeleteRegEx->ok());
    BeAssert(m_dataUpdateRegEx->ok());
    BeAssert(m_sysInsertRegEx->ok());
    BeAssert(m_sysDeleteRegEx->ok());
    BeAssert(m_sysUpdateRegEx->ok());
}

//***************************************************************************************
// DataChangeSqlListener
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult DataChangeSqlListener::Start(bool emptyCaptureBuffer) const {
    if (m_cancelCb != nullptr) {
        return BE_SQLITE_OK;
    }
    if (emptyCaptureBuffer) {
        m_dataChangeSql.clear();
        m_unknownSql.clear();
    }
    m_cancelCb = m_conn.GetTraceStmtEvent().AddListener(
        [&](TraceContext const& ctx, Utf8CP sql) {
            //! ignore readonly statement
            const auto isStmtReadonly = ctx.IsReadonly();
            if (isStmtReadonly) {
                return;
            }
            if (m_sqlDetector.IsDdl(sql) || m_sqlDetector.IsPragma(sql)) {
                return;
            }
            if (m_sqlDetector.IsSysDml(sql)) {
                return;
            }
            if (m_sqlDetector.IsDataDml(sql)) {
                m_dataChangeSql.push_back(ctx.GetExpandedSql());
                return;
            }
            m_unknownSql.push_back(ctx.GetExpandedSql());
        });
    m_conn.ConfigTraceEvents(DbTrace::Stmt, true);
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DataChangeSqlListener::Stop() const {
    if (m_cancelCb == nullptr) {
        return;
    }
    m_cancelCb();
    m_cancelCb = nullptr;
    if (m_dataChangeSql.empty() && m_unknownSql.empty()) {
        return;
    }
    for (auto& sql : m_dataChangeSql) {
        LOG.warningv("DATA CHANGE> %s\n", sql.c_str());
    }
    for (auto& sql : m_unknownSql) {
        LOG.warningv("UNKNOWN CHANGE> %s\n", sql.c_str());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DataChangeSqlListener::DataChangeSqlListener(ECDbCR conn)
    : m_conn(conn), m_sqlDetector(conn, true) {
    Start();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DataChangeSqlListener::~DataChangeSqlListener() {
    Stop();
}

//***************************************************************************************
// SqlTransformStep
//***************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TransformData::Append(Utf8StringCR description, Utf8String sql) {
    m_transforms.emplace_back(description, sql);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TransformData::ForEach(std::function<bool(Task const&)> cb) {
    for(auto& task : m_transforms) {
        if (!cb(task)) {
            return;
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult TransformData::Execute(ECDbCR conn) const{
    for(auto& transform : m_transforms) {
        if (transform.IsExecuted()) {
            BeAssert(false);
            return BE_SQLITE_ERROR;
        }
        auto rc = transform.Execute(conn);
        if (rc != BE_SQLITE_OK) {
            return rc;
        }
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TransformData::Validate(ECDbCR conn)  const{
    for(auto& transform : m_transforms) {
        if (!transform.Validate(conn)) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TransformData::Task& TransformData::Task::operator = (TransformData::Task&& rhs) {
    if (this != &rhs) {
        m_sql = std::move(rhs.m_sql);
        m_executed = std::move(rhs.m_executed);
        m_description = std::move(rhs.m_description);
    }
    return *this;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TransformData::Task& TransformData::Task::operator = (TransformData::Task& rhs) {
    if (this != &rhs) {
        m_sql = rhs.m_sql;
        m_executed = rhs.m_executed;
        m_description = rhs.m_description;
    }
    return *this;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult TransformData::Task::Execute(ECDbCR conn) const {
    LOG.infov("Executing data transform step: %s", m_description.c_str());
    auto rc = conn.TryExecuteSql(m_sql.c_str());
    m_executed = true;
    if (rc != BE_SQLITE_OK) {
        Utf8String msg = SqlPrintfString("[TransformData] Failed: %s - %s: %s",
                                        m_description.c_str(), m_sql.c_str(), BeSQLiteLib::GetLogError(rc).c_str())
                            .GetUtf8CP();
        conn.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0605, msg.c_str());
        LOG.error(msg.c_str());
        BeAssert(false && "transform query failed");
        return rc;
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool TransformData::Task::Validate(ECDbCR conn) const {
    Statement stmt;
    const auto rc = stmt.Prepare(conn, m_sql.c_str());
    if (rc != BE_SQLITE_OK) {
        conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0606,
            "[TransformData] Failed: %s. SQLite Error: %s", m_description.c_str(), BeSQLiteLib::GetLogError(rc).c_str());
        return false;
    }
    if (stmt.GetParameterCount() > 0) {
        conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0607,
            "[TransformData] Failed: %s. Transform sql should not be parameterized.", m_description.c_str());
        return false;
    }
    if (stmt.IsReadonly()) {
        conn.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0608,
            "[TransformData] Failed: %s. Transform sql must be a DML statement.", m_description.c_str());
        return false;
    }
    return true;
}


END_BENTLEY_SQLITE_EC_NAMESPACE
