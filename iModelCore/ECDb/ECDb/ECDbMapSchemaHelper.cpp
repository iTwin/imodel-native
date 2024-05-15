/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBMAP_SCHEMANAME "ECDbMap"

//*****************************************************************
//ECDbMapCustomAttributeHelper
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetSchemaMap(SchemaMapCustomAttribute& schemaMap, ECSchemaCR schema)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(schema, ECDBMAP_SCHEMANAME, "SchemaMap");
    if (!ca.IsValid())
        return false;

    schemaMap = SchemaMapCustomAttribute(schema, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetClassMap(ClassMapCustomAttribute& classMap, ECClassCR ecClass)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "ClassMap");
    if (!ca.IsValid())
        return false;

    classMap = ClassMapCustomAttribute(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetShareColumns(ShareColumnsCustomAttribute& shareColumns, ECClassCR ecClass)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "ShareColumns");
    if (!ca.IsValid())
        return false;

    shareColumns = ShareColumnsCustomAttribute(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(ECEntityClassCR ecClass)
    {
    return ecClass.GetCustomAttributeLocal(ECDBMAP_SCHEMANAME, "JoinedTablePerDirectSubclass") != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetDbIndexList(DbIndexListCustomAttribute& dbIndexList, ECClassCR ecClass)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "DbIndexList");
    if (!ca.IsValid())
        return false;

    dbIndexList = DbIndexListCustomAttribute(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetPropertyMap(PropertyMapCustomAttribute& propertyMap, PrimitiveECPropertyCR ecProperty)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(ecProperty, ECDBMAP_SCHEMANAME, "PropertyMap");
    if (!ca.IsValid())
        return false;

    propertyMap = PropertyMapCustomAttribute(ecProperty, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(LinkTableRelationshipMapCustomAttribute& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMANAME, "LinkTableRelationshipMap");
    if (!ca.IsValid())
        return false;

    linkTableRelationshipMap = LinkTableRelationshipMapCustomAttribute(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(ForeignKeyConstraintCustomAttribute& foreignKeyTableRelationshipMap, NavigationECPropertyCR navProp)
    {
    IECInstancePtr ca = CustomAttributeReader::Read(navProp, ECDBMAP_SCHEMANAME, "ForeignKeyConstraint");
    if (!ca.IsValid())
        return false;

    foreignKeyTableRelationshipMap = ForeignKeyConstraintCustomAttribute(navProp, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetImportRequiresVersion(ImportRequiresVersionCustomAttribute& ca, ECN::ECSchemaCR schema)
    {
    IECInstancePtr inst = CustomAttributeReader::Read(schema, ECDBMAP_SCHEMANAME, "ImportRequiresVersion");
    if (!inst.IsValid())
        return false;

    ca = ImportRequiresVersionCustomAttribute(schema, inst);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetUseRequiresVersion(UseRequiresVersionCustomAttribute& ca, ECN::ECClassCR ecClass)
    {
    IECInstancePtr inst = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "UseRequiresVersion");
    if (!inst.IsValid())
        return false;

    ca = UseRequiresVersionCustomAttribute(ecClass, inst);
    return true;
    }

//static
bool ECDbMapCustomAttributeHelper::IsForeignKeyView(ECN::ECRelationshipClassCR ecClass)
    {
    IECInstancePtr inst = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "ForeignKeyView");
    if (!inst.IsValid())
        return false;

    return true;
    }

//*****************************************************************
//SchemaMapCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaMapCustomAttribute::TryGetTablePrefix(Nullable<Utf8String>& tablePrefix) const
    {
    if (m_ca == nullptr)
        return ERROR;

    if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(tablePrefix, *m_ca, "TablePrefix"))
        return ERROR;

    if (tablePrefix.IsNull() || tablePrefix.Value().empty() || ECNameValidation::Validate(tablePrefix.Value().c_str()) == ECNameValidation::RESULT_Valid)
        return SUCCESS;

    LOG.errorv("Custom attribute '%s' on the ECSchema '%s' has an invalid value for the property 'TablePrefix': %s. "
               "The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.",
               m_ca->GetClass().GetName().c_str(),
               m_schema->GetName().c_str(), tablePrefix.Value().c_str());
    return ERROR;
    }


//*****************************************************************
//ClassMapCustomAttribute
//*****************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapCustomAttribute::TryGetMapStrategy(Nullable<Utf8String>& mapStrategy) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(mapStrategy, *m_ca, "MapStrategy");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapCustomAttribute::TryGetTableName(Nullable<Utf8String>& tableName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(tableName, *m_ca, "TableName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ClassMapCustomAttribute::TryGetECInstanceIdColumn(Nullable<Utf8String>& ecInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, *m_ca, "ECInstanceIdColumn");
    }

//*****************************************************************
//DbIndexListCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbIndexListCustomAttribute::GetIndexes(bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return ERROR;

    uint32_t propIx;
    ECObjectsStatus stat = m_ca->GetEnablerR().GetPropertyIndex(propIx, "Indexes");
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Failed to get property index for property 'List' of custom attribute '%s' on ECClass '%s'.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return ERROR;
        }

    ECValue indexesVal;
    stat = m_ca->GetValue(indexesVal, propIx);
    if (ECObjectsStatus::Success != stat)
        return ERROR;

    indices.clear();
    const uint32_t indexCount = indexesVal.IsNull() ? 0 : indexesVal.GetArrayInfo().GetCount();
    if (indexCount == 0)
        {
        LOG.errorv("Failed to read %s custom attribute' on ECClass '%s'. Its property 'Indexes' must be defined and at contain at least one 'DbIndex' element.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return ERROR;
        }

    for (uint32_t i = 0; i < indexCount; i++)
        {
        ECValue indexVal;
        if (ECObjectsStatus::Success != m_ca->GetValue(indexVal, propIx, i))
            {
            LOG.errorv("DbIndex #%d on ECClass %s could not be retrieved.", i, m_class->GetFullName());
            return ERROR;
            }

        IECInstancePtr dbIndexCA = indexVal.GetStruct();
        if (dbIndexCA == nullptr)
            continue;

        Nullable<Utf8String> indexName;
        if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(indexName, *dbIndexCA, "Name"))
            return ERROR;

        if (indexName.IsNull() || indexName.Value().empty())
            {
            LOG.errorv("Invalid DbIndex #%d on ECClass %s. A Name must always be specified for a DbIndex.", i, m_class->GetFullName());
            return ERROR;
            }

        Nullable<Utf8String> whereClause;
        if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(whereClause, *dbIndexCA, "Where"))
            return ERROR;

        Nullable<bool> isUnique;
        if (SUCCESS != CustomAttributeReader::TryGetBooleanValue(isUnique, *dbIndexCA, "IsUnique"))
            return ERROR;

        //Properties are mandatory for the index to be valid, so fail if there are none
        uint32_t propertiesPropIdx;
        if (ECObjectsStatus::Success != dbIndexCA->GetEnablerR().GetPropertyIndex(propertiesPropIdx, "Properties"))
            {
            LOG.errorv("Failed to get property index for property 'Indexes.Properties' of custom attribute '%s' on ECClass '%s'.",
                       m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ERROR;
            }

        ECValue propertiesVal;
        uint32_t propertiesCount = 0;
        if (ECObjectsStatus::Success != dbIndexCA->GetValue(propertiesVal, propertiesPropIdx))
            return ERROR;

        if (propertiesVal.IsNull() || (propertiesCount = propertiesVal.GetArrayInfo().GetCount()) == 0)
            {
            LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. At least one property must be specified.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ERROR;
            }

        DbIndex dbIndex(indexName.Value(), isUnique.IsNull() ? false : isUnique.Value(), whereClause);
        for (uint32_t j = 0; j < propertiesCount; j++)
            {
            Nullable<Utf8String> propName;
            if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(propName, *dbIndexCA, propertiesPropIdx, j) ||
                propName.IsNull())
                {
                LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. Array element of the Properties property is an empty string.",
                           i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
                return ERROR;
                }

            BeAssert(!propName.Value().empty());
            dbIndex.m_properties.push_back(propName.Value());
            }

        indices.push_back(std::move(dbIndex));
        }

    BeAssert(indices.size() == indexCount);
    return SUCCESS;
    }


//*****************************************************************
//PropertyMapCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyMapCustomAttribute::TryGetColumnName(Nullable<Utf8String>& columnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(columnName, *m_ca, "ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyMapCustomAttribute::TryGetIsNullable(Nullable<bool>& isNullable) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(isNullable, *m_ca, "IsNullable");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyMapCustomAttribute::TryGetIsUnique(Nullable<bool>& isUnique) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(isUnique, *m_ca, "IsUnique");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyMapCustomAttribute::TryGetCollation(Nullable<Utf8String>& collation) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(collation, *m_ca, "Collation");
    }

//*****************************************************************
//ForeignKeyConstraintCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ForeignKeyConstraintCustomAttribute::TryGetOnDeleteAction(Nullable<Utf8String>& onDeleteAction) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(onDeleteAction, *m_ca, "OnDeleteAction");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ForeignKeyConstraintCustomAttribute::TryGetOnUpdateAction(Nullable<Utf8String>& onUpdateAction) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(onUpdateAction, *m_ca, "OnUpdateAction");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ForeignKeyConstraintCustomAttribute::operator==(ForeignKeyConstraintCustomAttribute const& rhs) const
    {
    if (IsValid() != rhs.IsValid())
        return false;

    if (!IsValid())
        return true;

    Nullable<Utf8String> onDeleteStr, rhsOnDeleteStr;
    TryGetOnDeleteAction(onDeleteStr);
    rhs.TryGetOnDeleteAction(rhsOnDeleteStr);

    if (onDeleteStr != rhsOnDeleteStr)
        return false;

    Nullable<Utf8String> onUpdateStr, rhsOnUpdateStr;
    TryGetOnUpdateAction(onUpdateStr);
    rhs.TryGetOnUpdateAction(rhsOnUpdateStr);
    return onUpdateStr == rhsOnUpdateStr;
    }



//*****************************************************************
//ShareColumnsCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ShareColumnsCustomAttribute::TryGetApplyToSubclassesOnly(Nullable<bool>& applyToSubclassesOnly) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(applyToSubclassesOnly, *m_ca, "ApplyToSubclassesOnly");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ShareColumnsCustomAttribute::TryGetMaxSharedColumnsBeforeOverflow(Nullable<uint32_t>& maxSharedColumnsBeforeOverflow) const
    {
    if (m_ca == nullptr)
        return ERROR;
    Nullable<int> intVal;
    if (SUCCESS != CustomAttributeReader::TryGetIntegerValue(intVal, *m_ca, "MaxSharedColumnsBeforeOverflow"))
        return ERROR;

    if (intVal.IsNull())
        {
        maxSharedColumnsBeforeOverflow = Nullable<uint32_t>();
        return SUCCESS;
        }

    if (intVal.Value() < 0)
        return ERROR;

    maxSharedColumnsBeforeOverflow = Nullable<uint32_t>((uint32_t) intVal.Value());
    return SUCCESS;
    }


//*****************************************************************
//LinkTableRelationshipMapCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinkTableRelationshipMapCustomAttribute::TryGetSourceECInstanceIdColumn(Nullable<Utf8String>& sourceECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(sourceECInstanceIdColumnName, *m_ca, "SourceECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinkTableRelationshipMapCustomAttribute::TryGetTargetECInstanceIdColumn(Nullable<Utf8String>& targetECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(targetECInstanceIdColumnName, *m_ca, "TargetECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinkTableRelationshipMapCustomAttribute::TryGetCreateForeignKeyConstraints(Nullable<bool>& createForeignKeyConstraintsFlag) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(createForeignKeyConstraintsFlag, *m_ca, "CreateForeignKeyConstraints");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LinkTableRelationshipMapCustomAttribute::TryGetAllowDuplicateRelationships(Nullable<bool>& allowDuplicateRelationshipsFlag) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(allowDuplicateRelationshipsFlag, *m_ca, "AllowDuplicateRelationships");
    }


//*****************************************************************
//ImportRequiresVersionCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ImportRequiresVersionCustomAttribute::TryGetECDbRuntimeVersion(Nullable<Utf8String>& version) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(version, *m_ca, "ECDbRuntimeVersion");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ImportRequiresVersionCustomAttribute::Verify(IssueDataSource const& issues, Utf8CP fullSchemaName) const
    {
    Nullable<Utf8String> version;
    if (TryGetECDbRuntimeVersion(version) != BentleyStatus::SUCCESS)
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0654, "ECSchema %s has an invalid ImportRequiresVersion custom attribute.", fullSchemaName);
        return ERROR;
        }

    auto profileVersion = ECDb::CurrentECDbProfileVersion();
    ProfileVersion requiredProfileVersion(0, 0, 0, 0);
    if (version.IsNull() || requiredProfileVersion.FromString(version.ValueR().c_str()) != BentleyStatus::SUCCESS)
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0655, "ECSchema %s has a ImportRequiresVersion custom attribute with a missing or invalid ECDbRuntimeVersion property.", fullSchemaName);
        return ERROR;
        }

    if(requiredProfileVersion > profileVersion)
        {
        issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0656, "ECSchema %s requires ECDb version %s, but the current runtime version is only %s.", fullSchemaName, requiredProfileVersion.ToString().c_str(), profileVersion.ToString().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//*****************************************************************
//UseRequiresVersionCustomAttribute
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UseRequiresVersionCustomAttribute::TryGetECDbRuntimeVersion(Nullable<Utf8String>& version) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(version, *m_ca, "ECDbRuntimeVersion");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UseRequiresVersionCustomAttribute::TryGetECSqlVersion(Nullable<Utf8String>& version) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(version, *m_ca, "ECSqlVersion");
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UseRequiresVersionCustomAttribute::Verify(IssueDataSource const& issues, Utf8CP context) const
    {
    Nullable<Utf8String> version;
    if (TryGetECDbRuntimeVersion(version) == BentleyStatus::SUCCESS && !version.IsNull())
        {
        auto profileVersion = ECDb::CurrentECDbProfileVersion();
        ProfileVersion requiredProfileVersion(0, 0, 0, 0);
        if (requiredProfileVersion.FromString(version.ValueR().c_str()) != BentleyStatus::SUCCESS)
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue,ECDbIssueId::ECDb_0657,  "%s has a UseRequiresVersion custom attribute with an invalid ECDbRuntimeVersion property.", context);
            return ERROR;
            }

        if(requiredProfileVersion > profileVersion)
            {
            issues.ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECDbIssue,ECDbIssueId::ECDb_0658,  "%s requires ECDb version %s, but the current runtime version is only %s.", context, requiredProfileVersion.ToString().c_str(), profileVersion.ToString().c_str());
            return ERROR;
            }
        }

    Nullable<Utf8String> sqlVersion;
    if (TryGetECSqlVersion(sqlVersion) == BentleyStatus::SUCCESS && !sqlVersion.IsNull())
        {
        auto ecSqlVersion = ECDb::GetECSqlVersion();
        BeVersion requiredECSqlVersion(0, 0, 0, 0);
        if (requiredECSqlVersion.FromString(sqlVersion.ValueR().c_str()) != BentleyStatus::SUCCESS)
            {
            issues.ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue,ECDbIssueId::ECDb_0659,  "%s has a UseRequiresVersion custom attribute with an invalid ECSqlVersion property.", context);
            return ERROR;
            }

        if(requiredECSqlVersion > ecSqlVersion)
            {
            issues.ReportV(IssueSeverity::Warning, IssueCategory::BusinessProperties, IssueType::ECDbIssue,ECDbIssueId::ECDb_0660,  "%s requires ECSql version %s, but the current version is only %s.", context, requiredECSqlVersion.ToString().c_str(), ecSqlVersion.ToString().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//*****************************************************************
//CustomAttributeReader
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    return TryGetTrimmedValue(val, v);
    }


//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex)
    {
    ECValue v;
    if (ECObjectsStatus::Success != ca.GetValue(v, propIndex, arrayIndex))
        {
        LOG.errorv("Could not retrieve array element #%d value of ECProperty with property index %d of the '%s' custom attribute ECInstance.",
                   arrayIndex, propIndex, ca.GetClass().GetFullName());
        return ERROR;
        }

    return TryGetTrimmedValue(val, v);
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& strVal, ECValueCR val)
    {
    if (!val.IsNull())
        {
        strVal = Nullable<Utf8String>(val.GetUtf8CP());
        strVal.ValueR().Trim();

        //if the string property was specified but without a string, we consider this null as well
        if (strVal.Value().empty())
            strVal = Nullable<Utf8String>();
        }
    else
        strVal = Nullable<Utf8String>();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetIntegerValue(Nullable<int>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    if (ECObjectsStatus::Success != ca.GetValue(v, ecPropertyAccessString))
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    if (!v.IsNull())
        val = Nullable<int>(v.GetInteger());
    else
        val = Nullable<int>();


    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetBooleanValue(Nullable<bool>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    if (!v.IsNull())
        val = Nullable<bool>(v.GetBoolean());
    else
        val = Nullable<bool>();

    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
